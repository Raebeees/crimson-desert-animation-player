#pragma once
#include "flight_gate204.h"
#include "condition_calls204.h"
#include "skill_eligibility204.h"
#include "negative_filter204.h"
namespace boots204 {
// Construct on the wrapper's stack for EACH invocation (including nested calls).
// Readiness supplies slotsOwned() and assetsReady(); production implementations
// must verify the initialized adapter and exact slot ownership, not just flags.
template<class Reader,class Readiness> class GatePolicy204 {
    Reader& reader;Readiness& readiness;Word base,cc;
    FlightGateObservation observed{};
public:
    GatePolicy204(Reader& r,Readiness& status,Word game,Word creator):reader(r),readiness(status),base(game),cc(creator){}
    ConditionScope inspect(const ConditionArguments& args) {
        observed={};ConditionScope scope{};
        std::array<std::uint32_t,5> prefix{};
        if(!descriptorPrefix204(reader,args.b,238,prefix))return scope;
        bool relevant=(prefix[1]==1997601||prefix[1]==1997602)&&prefix[2]==1&&!prefix[3]&&!prefix[4];
        for(const auto& sample:skillSamples204)if(sample==prefix)relevant=true;
        if(!relevant)return scope;
        if(!readiness.slotsOwned())return scope;
        observed=inspectFlightGate(reader,base,cc,args.a,args.b);
        if(!observed.verified) {
            scope=inspectExistingSkill204(reader,base,cc,args);
            scope.ready=scope.owned && readiness.slotsOwned() && readiness.assetsReady() && readiness.slotsOwned();
            return scope;
        }
        if(!readiness.slotsOwned())return scope;
        scope.ready=true;scope.owned=true;scope.descriptor_exact=true;
        scope.words=observed.descriptor.words;return scope;
    }
    ConditionDecision before(const ConditionScope& scope,const ConditionArguments&) {
        if(!privateGate(scope,ConditionKind::skill) || !readiness.slotsOwned())return {};
        const bool assets=readiness.assetsReady();
        if(!readiness.slotsOwned())return {};
        const auto decision=decideFlightGate(observed,assets);
        if(decision.handled && decision.value<=1) {
            const unsigned index=(observed.descriptor.words[1]==1997602?2u:0u)+decision.value;
            gateDecisions204[index].fetch_add(1,std::memory_order_relaxed);
        }
        return {decision.handled,decision.value};
    }
    ConditionDecision after(const ConditionScope& scope,const ConditionArguments&,Word native) {
        if(!scope.ready || !scope.owned || !scope.descriptor_exact || !readiness.slotsOwned() || !readiness.assetsReady() || !readiness.slotsOwned())return {};
        return existingSkillDecision204(true,scope.words,native);
    }
};
// Original must be the immutable captured non-null native entry, including its
// existing CE detour. No exception swallowing or repeated native calls here.
template<class Reader,class Readiness,class Errors>
Word forwardPrivateGate204(ConditionOriginal original,const ConditionArguments& args,
        Reader& reader,Readiness& readiness,Errors& errors,Word base,Word cc) {
    GatePolicy204<Reader,Readiness> policy(reader,readiness,base,cc);
    return forwardCondition(ConditionKind::skill,original,args,policy,errors);
}
} // namespace boots204
