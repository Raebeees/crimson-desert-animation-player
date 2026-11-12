#include "gate_fixture.h"
#include "effect_cleanup204.h"
#include <cassert>
int main(){
 using namespace boots204;
 CommaEdge204 k;
 assert(!k.update(true,1,true,100));assert(!k.update(true,1,false,116));
 assert(k.update(true,1,true,132));assert(!k.update(true,1,true,148));
 assert(!k.update(false,1,false,164));assert(!k.update(true,1,true,180));
 assert(!k.update(true,1,false,196));assert(k.update(true,1,true,212));
 assert(!k.update(true,2,true,228));assert(!k.update(true,2,false,244));
 assert(!k.update(true,2,true,1000));
 for(bool on:{false,true}){
  commaEnabled204.store(on);auto m=flight_test::fixture();
  auto o=flight_test::inspect(m);assert(o.verified);
  assert((o.identity.boots==EquipmentPresence::present)==on);
  assert(decideFlightGate(o,true).value==(on?1:0)); // Exit.
  m.u32(flight_test::desc+4,1997601);o=flight_test::inspect(m);
  assert(decideFlightGate(o,true).value==(on?0:1)); // Entry.
  Snapshot<memoryfixture::Memory> s(m);IdentityEquipment real;
  assert(readIdentityEquipment(s,memoryfixture::base,memoryfixture::cc,real,0,0,true));
  assert(real.boots==EquipmentPresence::present && s.stable());
  EffectObservation204 fx;fx.verified=true;fx.active=1;fx.identity=o.identity;fx.state=o.lower;
  assert(cleanupDecision204(fx,true)==(on?Cleanup204::keep:Cleanup204::request_native_stop));
 }
 commaEnabled204.store(true);
 std::cout<<"Comma edges, physical equipment preservation, entry/exit gates and effect cleanup passed\n";
}
