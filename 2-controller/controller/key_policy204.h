#pragma once
#include "negative_filter204.h"
#include "input_identity204.h"
#include "condition_calls204.h"
#include "key_samples204.h"
namespace boots204 {
using Foreground204=bool(*)();
struct KeyObservation204{ConditionScope scope{};Action204 cancel=Action204::unavailable;};
template<class Reader> KeyObservation204 inspectGlideKey204(Reader& r,Word base,Word cc,const ConditionArguments& args){
 KeyObservation204 result;Snapshot<Reader> s(r);IdentityEquipment owner;LowerState lower;
 if(!readIdentityEquipment(s,base,cc,owner) || owner.boots!=EquipmentPresence::present || !readLowerState(s,owner.control,lower) || !lower.active)return result;
 bool flight=false;for(auto id:keyFlightStates204)if(id==lower.id)flight=true;if(!flight)return result;
 if(args.a<0x10000||args.a>=0x800000000000ULL-16||args.a%8||args.b<0x10000||args.b>=0x800000000000ULL-8||args.b%8)return result;
 Word actor=0,d=0,components=0,input=0;bool suppressed=true;
 if(!s.pointer(args.a+8,actor)||actor!=owner.actor||!s.read(args.b,d) || d<0x10000 || d>0x800000000000ULL-20)return result;
 std::array<std::uint32_t,5> words{};for(unsigned i=0;i<5;++i)if(!s.read(d+4*i,words[i]))return result;
 bool exact=false;for(const auto& sample:glideEndSamples204)if(words==sample)exact=true;if(!exact)return result;
 if(!s.pointer(actor+0x68,components)||!inputSuppressed204(s,components,suppressed)||suppressed||!readInputObject204(s,base,input))return result;
 const auto cancel=readAction204(s,input,0xc81dc84a);
 if(cancel==Action204::unavailable||!s.stable())return result;
 result.cancel=cancel;result.scope.owned=true;result.scope.descriptor_exact=true;result.scope.words=words;return result;
}
template<class Reader,class Readiness> class KeyPolicy204 {
 Reader& reader;Readiness& ready;Word base,cc;Foreground204 foreground;KeyObservation204 observed{};
 bool available(){return foreground&&foreground()&&ready.slotsOwned()&&ready.assetsReady()&&ready.slotsOwned();}
public:
 KeyPolicy204(Reader& r,Readiness& s,Word b,Word c,Foreground204 f):reader(r),ready(s),base(b),cc(c),foreground(f){}
 ConditionScope inspect(const ConditionArguments& args){observed={};std::array<std::uint32_t,5> prefix{};if(!descriptorPrefix204(reader,args.b,234,prefix))return {};bool relevant=false;for(const auto& sample:glideEndSamples204)if(sample==prefix)relevant=true;if(!relevant)return {};if(!available())return {};observed=inspectGlideKey204(reader,base,cc,args);observed.scope.ready=observed.scope.owned&&available();return observed.scope;}
 ConditionDecision before(const ConditionScope&,const ConditionArguments&){return {};}
 ConditionDecision after(const ConditionScope& s,const ConditionArguments&,Word native){if(!matching(s,ConditionKind::key)||(native&255)>1||!available()||observed.cancel==Action204::unavailable)return {};return {true,static_cast<std::uint8_t>(observed.cancel==Action204::held?0:1)};}
};
template<class Reader,class Readiness,class Errors> Word forwardGlideKey204(ConditionOriginal original,const ConditionArguments& args,Reader& r,Readiness& ready,Errors& errors,Word base,Word cc,Foreground204 foreground){
 KeyPolicy204<Reader,Readiness> policy(r,ready,base,cc,foreground);return forwardCondition(ConditionKind::key,original,args,policy,errors);
}
}

