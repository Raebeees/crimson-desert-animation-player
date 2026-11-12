#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include <cstddef>
#include <mutex>

namespace boots204 {
using Target=std::uintptr_t;
enum class Exchange { swapped, mismatch, unavailable };
enum class Setup { active, rejected, reused, preflight_failed, acquisition_failed, recheck_failed };
struct SlotPlan {
    std::array<Target,3> originals{};
    std::array<Target,3> wrappers{};
    // Supplied only after separate module/table/entry ownership and asset checks.
    // This container does not perform or replace those checks.
    bool prerequisites_verified=false;
};
struct RestoreReport { unsigned restored=0, not_owned=0, unavailable=0; };
struct SetupReport { Setup status=Setup::rejected;RestoreReport rollback{}; };

// Backend contracts: read(index,out) and compare_exchange(index,expected,new).
// compare_exchange must be atomic; backend methods must not throw or call game handlers.
// This is a transaction policy, NOT a backend for modifying game memory.
template<class Backend> class ConditionSlots {
    Backend& backend;
    std::mutex setup_mutex;
    std::array<std::atomic<Target>,3> saved{};
    std::array<Target,3> wrappers{};
    std::atomic<bool> enabled{false}, resident{false};
    bool attempted=false, prepared=false;
    RestoreReport restore_locked() {
        RestoreReport report;
        enabled.store(false,std::memory_order_release);
        if(!prepared || !resident.load(std::memory_order_acquire))return report;
        for(std::size_t i=0;i<3;++i) {
            auto result=backend.compare_exchange(i,wrappers[i],saved[i].load(std::memory_order_acquire));
            if(result==Exchange::swapped)++report.restored;
            else if(result==Exchange::mismatch)++report.not_owned;
            else ++report.unavailable;
        }
        return report;
    }
public:
    explicit ConditionSlots(Backend& b):backend(b) { for(auto& p:saved)p.store(0); }
    ConditionSlots(const ConditionSlots&)=delete;
    ConditionSlots& operator=(const ConditionSlots&)=delete;
    bool active()const { return enabled.load(std::memory_order_acquire); }
    // Caller MUST keep wrappers, saved originals and this object alive after any exposure.
    // Even clean rollback is not proof that another thread no longer holds a wrapper.
    bool requires_process_lifetime()const { return resident.load(std::memory_order_acquire); }
    Target original(std::size_t i)const { return i<3?saved[i].load(std::memory_order_acquire):0; }
    SetupReport activate(const SlotPlan& plan) {
        std::lock_guard<std::mutex> lock(setup_mutex);
        if(attempted)return {Setup::reused,{}};
        attempted=true;
        if(!plan.prerequisites_verified)return {Setup::rejected,{}};
        for(std::size_t i=0;i<3;++i) {
            if(!plan.originals[i] || !plan.wrappers[i])return {Setup::rejected,{}};
            for(std::size_t j=0;j<3;++j) {
                if(plan.originals[i]==plan.wrappers[j])return {Setup::rejected,{}};
                if(i!=j && plan.wrappers[i]==plan.wrappers[j])return {Setup::rejected,{}};
            }
        }
        for(std::size_t i=0;i<3;++i) {
            Target observed=0;
            if(!backend.read(i,observed) || observed!=plan.originals[i])return {Setup::preflight_failed,{}};
        }
        wrappers=plan.wrappers;
        // Publish ALL originals before exposing ANY wrapper.
        for(std::size_t i=0;i<3;++i)saved[i].store(plan.originals[i],std::memory_order_release);
        prepared=true;
        resident.store(true,std::memory_order_release);
        for(std::size_t i=0;i<3;++i)
            if(backend.compare_exchange(i,plan.originals[i],wrappers[i])!=Exchange::swapped)
                return {Setup::acquisition_failed,restore_locked()};
        for(std::size_t i=0;i<3;++i) {
            Target observed=0;
            if(!backend.read(i,observed) || observed!=wrappers[i])
                return {Setup::recheck_failed,restore_locked()};
        }
        enabled.store(true,std::memory_order_release);
        return {Setup::active,{}};
    }
    RestoreReport deactivate() {
        std::lock_guard<std::mutex> lock(setup_mutex);
        return restore_locked();
    }
};
} // namespace boots204
