#include "expected_runtime204.h"
#include "flight_entries204.h"
#include "startup204.h"
#include "ce_chains204.h"
#include "wait204.h"
#include "hook_routes204.h"
#include "hook_entry_pins204.h"
#include <memory>
#include "idle_shared_api.h"
extern "C" __declspec(dllexport) std::uint32_t TidebreakerIdleBridgeVersion1(){return idle_shared::version;}
extern "C" __declspec(dllexport) bool TidebreakerRegisterIdleBridge1(const idle_shared::Registration* request){
    if(!request)return false;
    return idle_shared::registry.install(*request,[](idle_shared::Evaluate callback){
        HMODULE retained=nullptr;
        return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
            reinterpret_cast<LPCWSTR>(callback),&retained)!=FALSE;
    });
}
extern "C" __declspec(dllexport) void TidebreakerDisableIdleBridge1(){idle_shared::registry.disable();}
#include "hold_launch.h"
#include "deferred_compat.h"

#include "client_identity.h"
#include "controller_observer.h"
namespace fs=std::filesystem;
using namespace boots204;
namespace {
enum Status : unsigned { waiting, wrongHost, missingModule, identityMismatch, missingAssets,
                         chainMismatch, entryMismatch, hookFailure, gateFailure, active, internalFailure };
std::atomic<unsigned> bridgeStatus{waiting};
std::atomic<unsigned> failedOperation{0},failureCode{0};
Status recordHookFailure(unsigned operation,unsigned code){
    failureCode.store(code);
    failedOperation.store(operation);
    return hookFailure;
}
constexpr char build[]="build\t0200040008000100000006000000646c635f303107000000973ad5e4\t326872984";
constexpr char ceHash[]="b2bbbc6618c64de03774463c95dcdd9937baee5394ecd3bfb1850f1cdeb0cb8b";
constexpr char dmmHash[]="d339d53b1c5741ada5b0f5eb3a6f93c63b828d1553f568507d94dbcfac3d17d9";
struct Module {
    HMODULE image=nullptr;
    HANDLE file=INVALID_HANDLE_VALUE;
    ~Module(){if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);if(image)FreeLibrary(image);}
    Module()=default;
    Module(const Module&)=delete;
    bool retain(HMODULE handle){
        return handle && GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
            reinterpret_cast<LPCWSTR>(handle),&image);
    }
    fs::path path() const {
        wchar_t name[32768];
        DWORD n=GetModuleFileNameW(image,name,32768);
        return n && n<32768 ? fs::path(name,name+n) : fs::path{};
    }
    bool verify(const fs::path& expected,const char* hash){
        auto actual=path();
        if(actual.empty() || _wcsicmp(actual.c_str(),expected.c_str()))return false;
        file=CreateFileW(actual.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
        LARGE_INTEGER size{};
        if(file==INVALID_HANDLE_VALUE || !GetFileSizeEx(file,&size) || size.QuadPart<=0 || size.QuadPart>512LL*1024*1024)return false;
        BCRYPT_ALG_HANDLE alg=nullptr;BCRYPT_HASH_HANDLE h=nullptr;
        bool ok=BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0 &&
                BCryptCreateHash(alg,&h,nullptr,0,nullptr,0,0)>=0;
        unsigned char buffer[65536],digest[32];
        auto start=GetTickCount64();
        for(auto left=size.QuadPart;ok && left>0;){
            DWORD want=static_cast<DWORD>((std::min)(left,65536LL)),got=0;
            ok=GetTickCount64()-start<15000 && ReadFile(file,buffer,want,&got,nullptr) && got==want &&
               BCryptHashData(h,buffer,got,0)>=0;
            left-=got;
        }
        if(ok)ok=BCryptFinishHash(h,digest,sizeof digest,0)>=0;
        if(h)BCryptDestroyHash(h);
        if(alg)BCryptCloseAlgorithmProvider(alg,0);
        return ok && !memcmp(digest,unhex204(hash).data(),sizeof digest);
    }
};
struct Resident {
    Module game,cc,ce,dmm;
    RuntimeReadinessLease204 lease;
    ProcessReader204 reader;
    std::unique_ptr<WindowsSlots204> slots;
    std::unique_ptr<ProductionConditionHost204> host;
    std::unique_ptr<FlightEntries204> entries;
    Module danceClient;
    int (*danceMenu)()=nullptr;
    void (*danceService)(bool)=nullptr;
    HANDLE danceMenuThread=nullptr;
};
Resident* toggleResident=nullptr;
std::atomic<bool> danceMenuRequested{false};
std::atomic<unsigned> danceStatus{0}; // 0 not initialized, 1 active, 2 rejected
DWORD WINAPI danceMenuWorker(void* argument){
    auto& s=*static_cast<Resident*>(argument);
    return static_cast<DWORD>(s.danceMenu());
}
bool initializeDance(Resident& s){
    const auto path=s.game.path().parent_path()/L"LowCortisolClient.asi";
    LockedReader204 identityReader;std::string fileHash;
    if(!identityReader.hash(path.string(),fileHash)||fileHash!=lowCortisolClientHash)return false;
    HMODULE loaded=LoadLibraryExW(path.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    if(!loaded)return false;
    const bool retained=s.danceClient.retain(loaded);FreeLibrary(loaded);
    if(!retained||!s.danceClient.verify(path,lowCortisolClientHash))return false;
    auto init=reinterpret_cast<bool(*)(std::uintptr_t,std::uintptr_t,idle_shared::Register)>(GetProcAddress(s.danceClient.image,"DanceInitialize1"));
    s.danceMenu=reinterpret_cast<int(*)()>(GetProcAddress(s.danceClient.image,"LowCortisolOpenMenu1"));
    s.danceService=reinterpret_cast<void(*)(bool)>(GetProcAddress(s.danceClient.image,"DanceService1"));
    if(!init||!s.danceMenu||!s.danceService)return false;
    return init(reinterpret_cast<Word>(s.game.image),reinterpret_cast<Word>(s.cc.image),TidebreakerRegisterIdleBridge1);
}
void serviceDance(Resident& s){
    if(danceStatus.load()!=1)return;
    s.danceService(s.host->runtime.assetsReady()&&s.host->runtime.slotsOwned());
    const bool requested=danceMenuRequested.exchange(false);
    if(requested)controller_observer::arm();
    controller_observer::flush(s.game.path().parent_path());
    if(s.danceMenuThread&&WaitForSingleObject(s.danceMenuThread,0)==WAIT_OBJECT_0){CloseHandle(s.danceMenuThread);s.danceMenuThread=nullptr;}
    if(requested&&!s.danceMenuThread)s.danceMenuThread=CreateThread(nullptr,0,danceMenuWorker,&s,0,nullptr);
    else if(requested){HWND menu=FindWindowW(L"LowCortisolNativeMenu1",nullptr);if(menu)PostMessageW(menu,WM_APP+20,0,0);}
}
// Independent of the four-hook startup status: 0 waiting, 1 checking,
// 2 prefix mismatch, 3 create failed, 4 enabling, 5 active, 6 enable failed.
std::atomic<unsigned> compatStatus{0},compatCode{0};
void activateCompat(Resident& s){
    compatStatus.store(1);
    constexpr unsigned char prefix[]={0x89,0x4c,0x24,0x08,0x55,0x56,0x57,0x41,
        0x56,0x41,0x57,0x48,0x83,0xec,0x20,0x8b};
    const auto address=reinterpret_cast<Word>(s.ce.image)+0x14820;
    unsigned char observed[sizeof prefix]{};
    if(!s.reader.read(address,observed,sizeof observed) || memcmp(prefix,observed,sizeof prefix)){
        compatStatus.store(2);return;
    }
    auto target=reinterpret_cast<void*>(address);void* original=nullptr;
    auto result=MH_CreateHook(target,reinterpret_cast<void*>(&FlightEntries204::ceVariant),&original);
    if(result!=MH_OK){compatCode.store(static_cast<unsigned>(result));compatStatus.store(3);return;}
    // Publication precedes activation. Never free a published trampoline; all
    // modules and entries are resident. No global disable/uninitialize on failure.
    FlightEntries204::ceSelect.store(reinterpret_cast<CeVariantOriginal204>(original),std::memory_order_release);
    compatStatus.store(4);
    result=MH_EnableHook(target);
    compatCode.store(static_cast<unsigned>(result));
    compatStatus.store(result==MH_OK?5u:6u);
}
Status start(){
    auto state=std::make_unique<Resident>();
    auto& s=*state;
    if(!s.game.retain(GetModuleHandleW(nullptr)))return wrongHost;
    auto exe=s.game.path();
    if(_wcsicmp(exe.filename().c_str(),L"CrimsonDesert.exe"))return wrongHost;
    auto dir=exe.parent_path();
    if(!s.game.verify(dir/L"CrimsonDesert.exe",hookEntryPins204[0].moduleHash))return identityMismatch;
    // Read once before hook activation; no filesystem work on the solver thread.
    const auto tuningPath=dir/L"TidebreakerFlightTuning.ini";
    wchar_t rate[64]{};
    GetPrivateProfileStringW(L"Vertical",L"Acceleration",L"8.0",rate,64,tuningPath.c_str());
    boots::verticalTuning.acceleration=boots::parseVerticalRate(rate);
    GetPrivateProfileStringW(L"Vertical",L"Deceleration",L"8.0",rate,64,tuningPath.c_str());
    boots::verticalTuning.deceleration=boots::parseVerticalRate(rate);
    const auto deadline=GetTickCount64()+30000;
    while(GetTickCount64()<deadline){
        if(!s.cc.image)s.cc.retain(GetModuleHandleW(L"CharacterCreator.asi"));
        if(!s.ce.image)s.ce.retain(GetModuleHandleW(L"CombatEvolved.asi"));
        if(!s.dmm.image)s.dmm.retain(GetModuleHandleW(L"DmmRuntime.asi"));
        if(s.cc.image&&s.ce.image&&s.dmm.image)break;
        Sleep(100);
    }
    if(!s.cc.image||!s.ce.image||!s.dmm.image)return missingModule;
    if(!s.cc.verify(dir/L"CharacterCreator.asi",hookEntryPins204[4].moduleHash) ||
       !s.ce.verify(dir/L"CombatEvolved.asi",ceHash) || !s.dmm.verify(dir/L"DmmRuntime.asi",dmmHash))return identityMismatch;
    if(!s.lease.verify((dir/L"dmm"/L"manifest.tsv").u8string(),build,expectedAssets204(),expectedRecords204()))return missingAssets;
    auto game=reinterpret_cast<Word>(s.game.image),cc=reinterpret_cast<Word>(s.cc.image);
    CeChains204<ProcessReader204> chains{s.reader,game,reinterpret_cast<Word>(s.ce.image)};
    SlotPlan plan{};
    for(unsigned i=0;i<3;++i)plan.originals[i]=game+cePins204[i].gameRva;
    plan.wrappers=ProductionConditionHost204::Callbacks::addresses();

    Word table=game+0x722b470;
    s.slots=std::make_unique<WindowsSlots204>(std::array<Target,3>{table+0x338,table+0x340,table+0x358});
    const auto conditionsReady=[&](){
        if(!chains.valid(plan.originals))return false;
        for(unsigned i=0;i<3;++i){
            Target value=0;
            if(!s.slots->read(i,value)||value!=plan.originals[i])return false;
        }
        return true;
    };
    if(!waitForConditions204(conditionsReady,GetTickCount64,[]{Sleep(250);}))return chainMismatch;
    s.host=std::make_unique<ProductionConditionHost204>(*s.slots,s.reader,game,cc,&foregroundProcess204);
    void* targets[4]{},*orig[4]{};
    void* wrappers[]={reinterpret_cast<void*>(&FlightEntries204::movement),reinterpret_cast<void*>(&FlightEntries204::effectUpdate),
                      reinterpret_cast<void*>(&FlightEntries204::retirement),reinterpret_cast<void*>(&FlightEntries204::selector)};
    HookRoutes204<ProcessReader204> routes{s.reader,{}};
    unsigned idx=0;
    for(unsigned i=0;i<hookEntryPins204.size();++i){
        const auto& pin=hookEntryPins204[i];
        Word address=(i==4?cc:game)+pin.rva;
        auto bytes=unhex204(pin.hex);
        unsigned char observed[32]{};
        if(bytes.size()>sizeof observed || !s.reader.read(address,observed,bytes.size()) ||
           memcmp(bytes.data(),observed,bytes.size()))return entryMismatch;
        if(i==3)continue;
        auto& route=routes.routes[idx];
        route.entry=address;route.wrapper=reinterpret_cast<Word>(wrappers[idx]);route.size=bytes.size();
        memcpy(route.prefix.data(),observed,bytes.size());targets[idx++]=reinterpret_cast<void*>(address);
    }
    const auto initCode=MH_Initialize();
    if(initCode!=MH_OK)return recordHookFailure(1,static_cast<unsigned>(initCode));
    for(unsigned i=0;i<4;++i){
        const auto createCode=MH_CreateHook(targets[i],wrappers[i],&orig[i]);
        if(createCode!=MH_OK){MH_Uninitialize();return recordHookFailure(2+i,static_cast<unsigned>(createCode));}
        routes.routes[i].trampoline=reinterpret_cast<Word>(orig[i]);
    }
    s.entries.reset(new FlightEntries204{s.host->runtime,{},{},game,cc,reinterpret_cast<SolverOriginal204>(orig[0]),
        reinterpret_cast<SelectorOriginal204>(orig[3]),reinterpret_cast<EffectUpdateOriginal204>(orig[1]),
        reinterpret_cast<RetireOriginal204>(orig[2]),reinterpret_cast<StopPrivateEffect204>(game+hookEntryPins204[3].rva)});
    // Worker holds a module reference for its lifetime. Pin before publishing callbacks.
    HMODULE self=nullptr;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
       reinterpret_cast<LPCWSTR>(&start),&self)){const auto code=GetLastError();MH_Uninitialize();return recordHookFailure(6,code);}
    state.release();
    if(!s.entries->bind())return gateFailure;
    plan.prerequisites_verified=true;
    CoordinatedStartup204 startup(*s.host,s.lease,routes,chains);
    if(startup.start(targets,plan).step!=StartupStep::ready)return gateFailure;
    toggleResident=&s; return active;
}
DWORD WINAPI worker(void* module){
    try{
        bridgeStatus.store(start());
        if(bridgeStatus.load()==active&&toggleResident){
            controller_observer::install(reinterpret_cast<Word>(toggleResident->game.image));
            try{danceStatus.store(initializeDance(*toggleResident)?1u:2u);}catch(...){danceStatus.store(2);}
        }
        CommaEdge204 key;
        tidebreaker35::HoldLaunch hold;
        DeferredCompat204 compat;
        while(bridgeStatus.load()==active && toggleResident){
            auto& s=*toggleResident;
            bool eligible=false;IdentityEquipment owner{};LowerState launchState{};
            if(foregroundProcess204() && s.host->runtime.assetsReady() && s.host->runtime.slotsOwned()){
                Snapshot<ProcessReader204> journal(s.reader);Word components=0;bool suppressed=true;
                eligible=readIdentityEquipment(journal,reinterpret_cast<Word>(s.game.image),reinterpret_cast<Word>(s.cc.image),owner,0,0,true)
                    && owner.boots==EquipmentPresence::present
                    && journal.pointer(owner.actor+0x68,components)
                    && inputSuppressed204(journal,components,suppressed) && !suppressed
                    && readLowerState(journal,owner.control,launchState) && launchState.active && journal.stable();
            }
            controller_observer::target.store(eligible?owner.control:0,std::memory_order_relaxed);
            if(key.update(eligible,owner.actor,(GetAsyncKeyState(VK_OEM_COMMA)&0x8000)!=0,GetTickCount64())){
                commaEnabled204.store(!commaEnabled204.load(std::memory_order_acquire),std::memory_order_release);
            }
            const auto now=GetTickCount64();
            const bool enabled=commaEnabled204.load(std::memory_order_acquire);
            auto phase=tidebreaker35::Phase::unavailable;
            if(eligible && launchGround204(launchState.id))phase=tidebreaker35::Phase::ground;
            else if(eligible && launchSource204(launchState.id))phase=tidebreaker35::Phase::jumpStart;
            if(!eligible || !enabled || phase==tidebreaker35::Phase::unavailable)clearLaunch204();
            tidebreaker35::Input in{owner.actor,now,eligible,enabled,eligible,eligible,
                (GetAsyncKeyState(VK_SPACE)&0x8000)!=0,phase};
            if(hold.update(in))requestLaunch204(owner.actor,now);
            if(compat.update(eligible && enabled,owner.actor,now))activateCompat(s);
            try{serviceDance(s);}catch(...){danceStatus.store(2);}
            Sleep(16);
        }
    }catch(...){bridgeStatus.store(internalFailure);}
    FreeLibraryAndExitThread(static_cast<HMODULE>(module),0);
}
}
#include "alloc_trace204.h"
static AllocTrace204 allocatorTrace;
extern "C" void BootsAllocationTrace(const char* msg,DWORD a,DWORD b){
    allocatorTrace.observe(msg,a,b);
}
extern "C" __declspec(dllexport) unsigned TidebreakerAllocKind204(){return allocatorTrace.kind.load();}
extern "C" __declspec(dllexport) unsigned TidebreakerAllocError204(){return allocatorTrace.winError.load();}
extern "C" __declspec(dllexport) unsigned TidebreakerAllocFree204(){return allocatorTrace.freeRegions.load();}
extern "C" __declspec(dllexport) unsigned TidebreakerStartupStatus204(){return bridgeStatus.load();}
extern "C" __declspec(dllexport) unsigned TidebreakerLowCortisolStatus1(){return danceStatus.load();}
extern "C" __declspec(dllexport) unsigned TidebreakerFailureOperation204(){return failedOperation.load();}
extern "C" __declspec(dllexport) unsigned TidebreakerFailureCode204(){return failureCode.load();}
BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID){
    if(reason==DLL_PROCESS_ATTACH){
        HMODULE hold=nullptr;
        if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,reinterpret_cast<LPCWSTR>(module),&hold))return FALSE;
        auto h=CreateThread(nullptr,0,worker,hold,0,nullptr);
        if(h)CloseHandle(h);else bridgeStatus.store(internalFailure);
    }
    return TRUE;
}





extern "C" __declspec(dllexport) unsigned TidebreakerGateStage0204(){return gateStages204[0].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerGateStage1204(){return gateStages204[1].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerGateStage2204(){return gateStages204[2].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerGateStage3204(){return gateStages204[3].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerGateStage4204(){return gateStages204[4].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerGateDecision0204(){return gateDecisions204[0].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerGateDecision1204(){return gateDecisions204[1].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerGateDecision2204(){return gateDecisions204[2].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerGateDecision3204(){return gateDecisions204[3].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerDescriptorStage0204(){return descriptorReject204[0].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerDescriptorStage1204(){return descriptorReject204[1].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerDescriptorStage2204(){return descriptorReject204[2].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerDescriptorStage3204(){return descriptorReject204[3].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerDescriptorStage4204(){return descriptorReject204[4].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerDescriptorStage5204(){return descriptorReject204[5].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerDescriptorStage6204(){return descriptorReject204[6].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerDescriptorStage7204(){return descriptorReject204[7].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerDescriptorStage8204(){return descriptorReject204[8].load();}

extern "C" __declspec(dllexport) unsigned TidebreakerDescriptorLastKind204(){return descriptorLastKind204.load();}

extern "C" __declspec(dllexport) unsigned TidebreakerDescriptorLastSkill204(){return descriptorLastSkill204.load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorCount0204(){return selectorCounts204[0].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorCount1204(){return selectorCounts204[1].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorCount2204(){return selectorCounts204[2].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorCount3204(){return selectorCounts204[3].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorCount4204(){return selectorCounts204[4].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorCount5204(){return selectorCounts204[5].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorLastState204(){return selectorLastState204.load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorLastNative204(){return selectorLastNative204.load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorLastResult204(){return selectorLastResult204.load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorReject0204(){return selectorReject204[0].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorReject1204(){return selectorReject204[1].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorReject2204(){return selectorReject204[2].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorReject3204(){return selectorReject204[3].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorReject4204(){return selectorReject204[4].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorReject5204(){return selectorReject204[5].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorReject6204(){return selectorReject204[6].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorReject7204(){return selectorReject204[7].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorReject8204(){return selectorReject204[8].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorReject9204(){return selectorReject204[9].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorReject10204(){return selectorReject204[10].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorReject11204(){return selectorReject204[11].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorIdentityProbe0204(){return selectorIdentityProbe204[0].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorIdentityProbe1204(){return selectorIdentityProbe204[1].load();}
extern "C" __declspec(dllexport) unsigned TidebreakerSelectorIdentityProbe2204(){return selectorIdentityProbe204[2].load();}
extern "C" __declspec(dllexport) const HolderBank204* TidebreakerHolderBank204(){return &holderBank204;}
extern "C" __declspec(dllexport) unsigned TidebreakerHolderInvalid204(){return holderInvalid204.load();}
extern "C" __declspec(dllexport) unsigned TidebreakerHolderDuplicate204(){return holderDuplicate204.load();}
extern "C" __declspec(dllexport) unsigned TidebreakerHolderFull204(){return holderFull204.load();}
extern "C" __declspec(dllexport) unsigned TidebreakerHolderSkipped204(){return holderSkipped204.load();}

extern "C" __declspec(dllexport) unsigned TidebreakerCommaEnabled204(){return commaEnabled204.load()?1u:0u;}

extern "C" __declspec(dllexport) unsigned TidebreakerLaunchPending204(){return launchDeadline204.load()>GetTickCount64()?1u:0u;}

extern "C" __declspec(dllexport) unsigned TidebreakerCompatStatus204(){return compatStatus.load();}
extern "C" __declspec(dllexport) unsigned TidebreakerCompatCode204(){return compatCode.load();}
extern "C" __declspec(dllexport) unsigned TidebreakerCompatPreserved204(){return FlightEntries204::cePreserved.load();}
extern "C" __declspec(dllexport) unsigned TidebreakerCompatForwarded204(){return FlightEntries204::ceForwarded.load();}

extern "C" __declspec(dllexport) bool TidebreakerOpenAnimations1(){
    if(danceStatus.load()!=1)return false;
    danceMenuRequested.store(true);return true;
}
