#pragma once
#include "condition_slots204.h"
#include "hook_batch204.h"
namespace boots204 {
// Single startup worker. Host, lease, callbacks and trampolines must remain resident.
// Hooks must forward to their originals while host.runtime.assetsReady() is false.
enum class StartupStep { reused, preflight, hooks, slots, ownership, ready };
struct StartupResult {
    StartupStep step = StartupStep::preflight;
    BootsHookBatchResult hooks{MH_OK, MH_OK};
    SetupReport slots{};
    bool hookRollbackWithheld = false;
};
template<class Host, class Lease, class Verifier, class Chains>
class CoordinatedStartup204 {
    Host& host;
    const Lease& lease;
    Verifier& verifier;
    const Chains& chains;
    bool attempted = false;
public:
    CoordinatedStartup204(Host& h, const Lease& l, Verifier& v, const Chains& c):host(h),lease(l),verifier(v),chains(c){}
    CoordinatedStartup204(const CoordinatedStartup204&) = delete;
    StartupResult start(void *const targets[4], const SlotPlan& plan) {
        StartupResult result;
        if (attempted) { result.step = StartupStep::reused; return result; }
        attempted = true;
        host.runtime.setAssetsReady(false);
        if (!plan.prerequisites_verified || !lease.ready() || !chains.valid(plan.originals) || !verifier.before(targets)) return result;
        result.step = StartupStep::hooks;
        result.hooks = boots_enable_batch(targets);
        if (result.hooks.activation != MH_OK) return result;
        result.step = StartupStep::ownership;
        if (verifier.owned(targets)) {
            result.step = StartupStep::slots;
            result.slots = host.activate(plan);
            if (result.slots.status == Setup::active) {
                result.step = StartupStep::ownership;
                if (lease.ready() && chains.valid(plan.originals) && host.runtime.slotsOwned() && verifier.owned(targets)) {
                    host.runtime.setAssetsReady(true);
                    result.step = StartupStep::ready;
                    return result;
                }
                result.slots.rollback = host.runtime.deactivate();
            }
        }
        if (verifier.owned(targets)) result.hooks.rollback = boots_disable_batch(targets);
        else result.hookRollbackWithheld = true;
        return result;
    }
};
}

