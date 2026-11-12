#pragma once
#include <cstdint>
#include <array>

namespace boots204 {
using Word=std::uintptr_t;
struct ConditionArguments {
    Word a,b,c,d;
    std::uint8_t e; // native fifth stack argument is byte-sized
    Word f,g;
};
using ConditionOriginal=Word(*)(Word,Word,Word,Word,std::uint8_t,Word,Word);
enum class ConditionKind:std::uint32_t { key=234, character_class=235, skill=238 };
struct ConditionScope {
    bool ready=false, owned=false, descriptor_exact=false;
    std::array<std::uint32_t,5> words{};
};
struct ConditionDecision { bool handled=false;std::uint8_t value=0; };
inline bool matching(const ConditionScope& s,ConditionKind kind) {
    return s.ready && s.owned && s.descriptor_exact && s.words[0]==static_cast<std::uint32_t>(kind);
}
inline bool privateGate(const ConditionScope& s,ConditionKind kind) {
    return kind==ConditionKind::skill && matching(s,kind) &&
        (s.words[1]==1997601 || s.words[1]==1997602) &&
        s.words[2]==1 && s.words[3]==0 && s.words[4]==0;
}
// Policy interface: inspect(args), before(scope,args), after(scope,args,result).
// inspect must independently establish exact descriptor and fresh actor/equipment ownership.
// This helper neither reads a descriptor nor certifies those facts.
// ErrorState supplies get()/set() to preserve caller/native thread error state.
template<class Policy,class ErrorState>
Word forwardCondition(ConditionKind kind,ConditionOriginal original,const ConditionArguments& args,
                      Policy& policy,ErrorState& errors) {
    // A null original is a setup failure; never invent a fallback native result.
    // Full adapter must reject activation and never expose such a wrapper.
    const auto callerError=errors.get();
    ConditionScope beforeScope{};
    try {
        beforeScope=policy.inspect(args);
        if(privateGate(beforeScope,kind)) {
            const auto decision=policy.before(beforeScope,args);
            if(decision.handled && decision.value<=1) {
                errors.set(callerError);
                return decision.value;
            }
        }
    } catch(...) { /* Observation/policy failure falls back to original. */ }
    errors.set(callerError);
    // Original is outside policy exception handlers: never retry native code on an exception.
    const Word result=original(args.a,args.b,args.c,args.d,args.e,args.f,args.g);
    const auto nativeError=errors.get();
    Word finalResult=result;
    if((result&0xff)<=1) {
        try {
            const auto afterScope=policy.inspect(args); // ownership may change in original
            if(matching(afterScope,kind)) {
                const auto decision=policy.after(afterScope,args,result);
                if(decision.handled && decision.value<=1)
                    finalResult=(result&~Word(0xff))|decision.value;
            }
        } catch(...) { finalResult=result; }
    }
    errors.set(nativeError);
    return finalResult;
}
} // namespace boots204
