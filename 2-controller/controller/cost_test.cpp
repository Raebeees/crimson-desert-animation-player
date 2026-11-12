#include "gate_fixture.h"
#include "gate_forwarding204.h"
#include "class_policy204.h"
#include "key_policy204.h"
#include "effect_cleanup204.h"
#include "selector_adapter204.h"
#include <iostream>
using namespace boots204;
unsigned calls=0;
Word native(Word,Word,Word,Word,std::uint8_t,Word,Word){++calls;return 1;}
std::uint32_t selectNative(Word,Word){++calls;return 123;}
struct Ready{bool assetsReady(){return true;}bool slotsOwned(){return true;}};
struct Errors{unsigned get(){return 7;}void set(unsigned x){if(x!=7)throw 1;}};
bool focused(){return true;}
unsigned reads(const memoryfixture::Memory& m){unsigned n=0;for(auto& v:m.visits)n+=v.second;return n;}
void check(bool b,const char* label){if(!b)throw std::runtime_error(label);}
int main(){
 Ready rt;Errors e;ConditionArguments args{flight_test::ctx,flight_test::holder,0,0,0,0,0};
 for(unsigned kind:{234u,235u,238u}){
  auto m=flight_test::fixture();m.u32(flight_test::desc,kind);m.u32(flight_test::desc+4,999999);
  calls=0;Word value=0;
  if(kind==234)value=forwardGlideKey204(&native,args,m,rt,e,memoryfixture::base,memoryfixture::cc,&focused);
  if(kind==235)value=forwardClass204(&native,args,m,rt,e,memoryfixture::base,memoryfixture::cc);
  if(kind==238)value=forwardPrivateGate204(&native,args,m,rt,e,memoryfixture::base,memoryfixture::cc);
  check(calls==1&&value==1,"unrelated native forwarding");
  std::cout<<"unrelated kind="<<kind<<" reads="<<reads(m)<<" equipmentRootReads="<<m.visits[memoryfixture::components+0x38]<<"\n";
#ifdef CANDIDATE
  check(reads(m)<=12&&m.visits[memoryfixture::components+0x38]==0,"unrelated read budget");
#endif
 }
 auto m=flight_test::fixture();constexpr Word effect=0x620000;m.ptr(effect+8,memoryfixture::actor+8);
 auto o=inspectEffects204(m,memoryfixture::base,memoryfixture::cc,effect);
 check(!o.verified,"foreign effect rejected");std::cout<<"foreign effect reads="<<reads(m)<<"\n";
#ifdef CANDIDATE
 check(reads(m)<=5,"foreign effect budget");
#endif
 m=flight_test::fixture();m.ptr(memoryfixture::control+0x88,flight_test::stateHolder);calls=0;
 check(forwardSelector204(&selectNative,memoryfixture::control+8,flight_test::stateHolder,m,rt,e,memoryfixture::base,memoryfixture::cc)==123&&calls==1,"foreign selector passthrough");
 std::cout<<"foreign selector reads="<<reads(m)<<"\n";
#ifdef CANDIDATE
 check(selectorIdentityProbe204[0]==0&&holderUsed204==0,"diagnostics disabled");
 // Positive prefilter never grants permission. Changed descriptors and boots off reject.
 m=flight_test::fixture();m.change=flight_test::desc+4;m.changeVisit=2;
 auto denied=inspectFlightGate(m,memoryfixture::base,memoryfixture::cc,flight_test::ctx,flight_test::holder);
 check(!denied.verified,"descriptor mutation rejected");
 m=flight_test::fixture();m.u32(memoryfixture::itemRecord+8,99);
 auto verified=inspectFlightGate(m,memoryfixture::base,memoryfixture::cc,flight_test::ctx,flight_test::holder);
 check(verified.verified&&verified.identity.boots==EquipmentPresence::absent,"boots off retained for exit");
 check(decideFlightGate(verified,true).value==0,"private exit still accepted");
#endif
 std::cout<<"cost regression passed\n";
}
