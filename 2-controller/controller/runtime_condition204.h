#pragma once
#include "condition_slots204.h"
#include "gate_forwarding204.h"
#include "class_policy204.h"
#include "key_policy204.h"
namespace boots204 {
// Lifetime: keep this object and its backend alive for process lifetime after
// publication. No automatic destruction/unloading/rollback from a callback.
template<class Backend> class RuntimeCondition204 {
    Backend& backend;
    ConditionSlots<Backend> transaction;
    std::array<Target,3> wrappers{};
    std::atomic<bool> assets{false},lost{false};
    std::mutex activation;
    bool attempted=false;
public:
    explicit RuntimeCondition204(Backend& b):backend(b),transaction(b){}
    RuntimeCondition204(const RuntimeCondition204&)=delete;
    RuntimeCondition204& operator=(const RuntimeCondition204&)=delete;
    SetupReport activate(const SlotPlan& verifiedPlan) {
        std::lock_guard<std::mutex> lock(activation);
        if(attempted)return {Setup::reused,{}};
        attempted=true;wrappers=verifiedPlan.wrappers;
        // Originals are published by transaction before any table replacement.
        // Wrappers are immutable before active() publishes readiness.
        return transaction.activate(verifiedPlan);
    }
    void setAssetsReady(bool value) {assets.store(value,std::memory_order_release);}
    bool assetsReady()const{return assets.load(std::memory_order_acquire);}
    bool ownershipLost()const{return lost.load(std::memory_order_acquire);}
    bool slotsOwned() {
        if(lost.load(std::memory_order_acquire) || !transaction.active())return false;
        for(std::size_t i=0;i<3;++i) {
            Target actual=0;
            if(!backend.read(i,actual) || actual!=wrappers[i]) {
                lost.store(true,std::memory_order_release);return false;
            }
        }
        return transaction.active() && !lost.load(std::memory_order_acquire);
    }
    Target original(std::size_t index)const{return transaction.original(index);}
    RestoreReport deactivate() {
        lost.store(true,std::memory_order_release);
        return transaction.deactivate();
    }
    bool requiresProcessLifetime()const{return transaction.requires_process_lifetime();}
    template<class Reader,class Errors>
    Word key(const ConditionArguments& args,Reader& reader,Errors& errors,Word base,Word cc,Foreground204 foreground) {
        auto native=reinterpret_cast<ConditionOriginal>(transaction.original(0));
        return forwardGlideKey204(native,args,reader,*this,errors,base,cc,foreground);
    }
    template<class Reader,class Errors>
    Word characterClass(const ConditionArguments& args,Reader& reader,Errors& errors,Word base,Word cc) {
        auto native=reinterpret_cast<ConditionOriginal>(transaction.original(1));
        return forwardClass204(native,args,reader,*this,errors,base,cc);
    }
    template<class Reader,class Errors>
    Word skill(const ConditionArguments& args,Reader& reader,Errors& errors,Word base,Word cc) {
        // Only expose wrapper after saved original exists. Setup/caller invariant;
        // there is deliberately no fabricated native result for a null original.
        auto native=reinterpret_cast<ConditionOriginal>(transaction.original(2));
        return forwardPrivateGate204(native,args,reader,*this,errors,base,cc);
    }
};
} // namespace boots204
