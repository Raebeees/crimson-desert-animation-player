#include <windows.h>
#include "MinHook.h"
#include "ce_variant_compat.h"
#include <cassert>
#include <cstring>
#include <iostream>
using namespace boots204;
CeVariantOriginal204 original=nullptr;
bool enabled=false;
extern "C" void BootsAllocationTrace(const char*,DWORD,DWORD){}
std::uint64_t replacement(std::uint32_t state,std::uint32_t incoming){
    if(preserveBootsVariant204(true,enabled,state,incoming))return retainedBootsVariant204(incoming);
    return original(state,incoming);
}
int main(){
    // Exact CE entry prologue, then fixture body/epilogue. No game loaded.
    const unsigned char code[]={0x89,0x4c,0x24,0x08,0x55,0x56,0x57,0x41,0x56,0x41,0x57,0x48,0x83,0xec,0x20,0x8b,0xf9,
        0x8b,0xc2,0xff,0xc0,0x48,0x83,0xc4,0x20,0x41,0x5f,0x41,0x5e,0x5f,0x5e,0x5d,0xc3};
    void* target=VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(target);
    memcpy(target,code,sizeof code);DWORD old=0;
    assert(VirtualProtect(target,4096,PAGE_EXECUTE_READ,&old));
    assert(FlushInstructionCache(GetCurrentProcess(),target,sizeof code));
    auto call=reinterpret_cast<CeVariantOriginal204>(target);
    assert(call(0x15914f6d,123)==124);
    assert(MH_Initialize()==MH_OK);
    void* captured=nullptr;
    assert(MH_CreateHook(target,reinterpret_cast<void*>(&replacement),&captured)==MH_OK);
    original=reinterpret_cast<CeVariantOriginal204>(captured);
    assert(MH_EnableHook(target)==MH_OK);
    for(unsigned i=0;i<10000;++i){
        enabled=(i&1)!=0;
        assert(call(0x15914f6d,0x54424340)==(enabled?0x54424340u:0x54424341u));
        assert(call(0x15914f6d,123)==124);
        assert(call(0x3ac4a94e,0x54424340)==0x54424341);
        assert(call(0x12345678,0x54424340)==0x54424341);
    }
    assert(MH_DisableHook(target)==MH_OK);assert(MH_RemoveHook(target)==MH_OK);
    assert(MH_Uninitialize()==MH_OK);assert(VirtualFree(target,0,MEM_RELEASE));
    std::cout<<"Exact CE prologue trampoline: 40000 hooked calls and cleanup passed (fixture only)\n";
}
