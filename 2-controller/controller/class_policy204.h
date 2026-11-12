#pragma once
#include "negative_filter204.h"
#include "flight_gate204.h"
#include "condition_calls204.h"
#include "class_state_samples204.h"
namespace boots204 {
template<class Reader> bool readClassLane204(Snapshot<Reader>& s,Word control,bool upper,LowerState& output) {
 output={};
 if(control<0x10000 || control>=0x800000000000ULL-0x98 || control%8)return false;
 Word holder=0,node=0;std::uint64_t a=0,b=0;std::uint8_t active=0;
 if(!s.pointer(control+(upper?0x88:0x90),holder) || !s.read(holder,a) || !s.read(holder+8,b))return false;
 if(!a || !b)return true;
 if(!s.read(holder+0x10,node) || node<0x10000 || node>=0x800000000000ULL-0x112 || !s.read(node+0x111,active))return false;
 if(!active)return true;
 if(!s.read(node+0x18,output.id))return false;
 output.active=true;return true;
}
template<class Reader> ConditionScope inspectClassState204(Reader& reader,Word base,Word cc,const ConditionArguments& args) {
 ConditionScope result{};Snapshot<Reader> s(reader);IdentityEquipment owner;
 if(!readIdentityEquipment(s,base,cc,owner) || owner.boots!=EquipmentPresence::present)return result;
 if(args.a<0x10000 || args.a>=0x800000000000ULL-16 || args.a%8 || args.b<0x10000 || args.b>=0x800000000000ULL-8 || args.b%8)return result;
 Word actor=0,d=0;
 if(!s.pointer(args.a+8,actor) || actor!=owner.actor || !s.read(args.b,d) || d<0x10000 || d>0x800000000000ULL-20 || d<0x10020 || d>=0x800000000000ULL-64)return result;
 std::array<std::uint32_t,5> words{};
 for(unsigned i=0;i<5;++i)if(!s.read(d+4*i,words[i]))return result;
 if(words[0]!=235 || words[1]!=0x0b494c51 || words[3] || words[4] ||
   (words[2]!=0x3f800000 && words[2]!=0x40000000 && words[2]!=0x40400000))return result;
 std::array<std::uint8_t,96> window{};
 for(unsigned i=0;i<96;++i)if(!s.read(d-32+i,window[i]))return result;
 bool matched=false;
 for(const auto& sample:classStateSamples204) {
  if(sample.bytes!=window)continue;
  LowerState state{};if(!readClassLane204(s,owner.control,sample.upper,state) || !state.active)return result;
  for(unsigned i=0;i<sample.count;++i)if((state.id==sample.states[i] || (state.id==0x54424635 && sample.states[i]==0x54424634)))matched=true;
 }
 if(!matched || !s.stable())return result;
 result.owned=true;result.descriptor_exact=true;result.words=words;return result;
}
template<class Reader,class Readiness> class ClassPolicy204 {
 Reader& reader;Readiness& readiness;Word base,cc;
public:
 ClassPolicy204(Reader& r,Readiness& s,Word b,Word c):reader(r),readiness(s),base(b),cc(c){}
 ConditionScope inspect(const ConditionArguments& args) {
  std::array<std::uint32_t,5> prefix{};if(!descriptorPrefix204(reader,args.b,235,prefix))return {};if(prefix[1]!=0x0b494c51||prefix[3]||prefix[4]||(prefix[2]!=0x3f800000&&prefix[2]!=0x40000000&&prefix[2]!=0x40400000))return {};
  if(!readiness.slotsOwned() || !readiness.assetsReady())return {};
  auto scope=inspectClassState204(reader,base,cc,args);
  scope.ready=scope.owned && readiness.slotsOwned() && readiness.assetsReady() && readiness.slotsOwned();return scope;
 }
 ConditionDecision before(const ConditionScope&,const ConditionArguments&){return {};}
 ConditionDecision after(const ConditionScope& scope,const ConditionArguments&,Word native) {
  if(!matching(scope,ConditionKind::character_class) || (native&255)>1 || !readiness.slotsOwned() || !readiness.assetsReady() || !readiness.slotsOwned())return {};
  return {true,static_cast<std::uint8_t>(scope.words[2]==0x40400000?0:1)};
 }
};
template<class Reader,class Readiness,class Errors> Word forwardClass204(ConditionOriginal original,const ConditionArguments& args,Reader& r,Readiness& ready,Errors& errors,Word base,Word cc) {
 ClassPolicy204<Reader,Readiness> policy(r,ready,base,cc);
 return forwardCondition(ConditionKind::character_class,original,args,policy,errors);
}
} // namespace boots204

