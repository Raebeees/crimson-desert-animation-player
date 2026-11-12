#pragma once
#include "production_condition_host204.h"
#include "selector_adapter204.h"
#include "movement_wrapper204.h"
#include "velocity_writer204.h"
#include "ring_retention204.h"
#include "effect_wrapper204.h"
#include <intrin.h>
#include "ce_variant_compat.h"
namespace boots204 {
// Bind the immutable originals and retain their trampolines before enabling any hook.
struct FlightEntries204 {
    using Runtime = ProductionConditionHost204::Runtime;
    Runtime& runtime;
    ProcessReader204 reader;
    VelocityWriter204 writer;
    Word game, cc;
    SolverOriginal204 solver;
    SelectorOriginal204 select;
    EffectUpdateOriginal204 update;
    RetireOriginal204 retire;
    StopPrivateEffect204 stop;
    inline static std::atomic<CeVariantOriginal204> ceSelect{nullptr};
    inline static std::atomic<unsigned> cePreserved{0},ceForwarded{0};
    inline static std::atomic<FlightEntries204*> bound{nullptr};
    bool bind() noexcept {
        if (!game || !cc || !solver || !select || !update || !retire || !stop) return false;
        FlightEntries204* empty=nullptr;
        return bound.compare_exchange_strong(empty,this,std::memory_order_release,std::memory_order_relaxed);
    }
    static FlightEntries204& get() noexcept {
        auto p=bound.load(std::memory_order_acquire);
        if(!p)std::terminate();
        return *p;
    }
    static Word movement(Word a,Word b,Word c,Word d,Word e,Word f,Word g){
        auto& x=get();
        if(!x.runtime.assetsReady()) return x.solver(a,b,c,d,e,f,g);
        thread_local MovementAdapter204 adapter;
        WindowsErrors204 errors;
        const auto incoming=errors.get();const auto now=GetTickCount64();errors.set(incoming);
        return forwardMovement204(x.solver,{a,b,c,d,e,f,g},reinterpret_cast<Word>(_ReturnAddress()),
            x.reader,x.writer,x.runtime,errors,&foregroundProcess204,adapter,x.game,x.cc,now);
    }
    static std::uint32_t selector(Word control,Word holder){
        auto& x=get();
        if(!x.runtime.assetsReady())return x.select(control,holder);
        WindowsErrors204 errors;
        return forwardSelector204(x.select,control,holder,x.reader,x.runtime,errors,x.game,x.cc);
    }
    static std::uint64_t ceVariant(std::uint32_t state,std::uint32_t incoming){
        auto& x=get();
        const auto saved=GetLastError();bool preserve=false;
        try {
            const bool ready=x.runtime.assetsReady() && x.runtime.slotsOwned();
            if(preserveBootsVariant204(ready,true,state,incoming)){
                Snapshot<ProcessReader204> journal(x.reader);IdentityEquipment owner;
                const bool equipped=readIdentityEquipment(journal,x.game,x.cc,owner)
                    && owner.boots==EquipmentPresence::present && journal.stable();
                preserve=preserveBootsVariant204(x.runtime.assetsReady()&&x.runtime.slotsOwned(),equipped,state,incoming);
            }
        }catch(...){preserve=false;}
        SetLastError(saved);
        if(preserve){cePreserved.fetch_add(1,std::memory_order_relaxed);return retainedBootsVariant204(incoming);}
        ceForwarded.fetch_add(1,std::memory_order_relaxed);
        return ceSelect.load(std::memory_order_acquire)(state,incoming);
    }
    static Word effectUpdate(Word effect){
        auto& x=get();
        if(!x.runtime.assetsReady()) return x.update(effect);
        WindowsErrors204 errors;CleanupResult204 cleanup;
        return forwardEffectUpdate204(x.update,x.stop,effect,x.reader,x.runtime,errors,x.game,x.cc,cleanup);
    }
    static void retirement(Word effect,std::uint32_t index,bool immediate){
        auto& x=get();
        if(!x.runtime.assetsReady()){x.retire(effect,index,immediate);return;}
        WindowsErrors204 errors;
        forwardRetirement204(x.retire,effect,index,immediate,reinterpret_cast<Word>(_ReturnAddress()),
            x.reader,x.runtime,errors,x.game,x.cc);
    }
};
}
