#include "gate_fixture.h"
#include "hold_launch.h"
#include <cassert>
#include "capture_replay.h"
int main(){
 using namespace boots204;
 for(bool enabled:{false,true}){
  tidebreaker35::HoldLaunch hold;
  tidebreaker35::Input in{1,1000,true,enabled,true,true,false,tidebreaker35::Phase::ground};
  assert(!hold.update(in));
  for(unsigned t=16;t<1000;t+=16){in.now=1000+t;in.held=t<200;assert(!hold.update(in));}
 }
 {tidebreaker35::HoldLaunch hold;int moving=0;
 for(const auto& r:capture){
 auto phase=launchGround204(r.state)?tidebreaker35::Phase::ground:(launchSource204(r.state)?tidebreaker35::Phase::jumpStart:tidebreaker35::Phase::unavailable);
 tidebreaker35::Input in{1,r.tick,true,true,true,true,r.held,phase};
 if(hold.update(in) && r.state!=0x2bca0570 && r.state!=0x402b6a4e){++moving;}
 }
 assert(moving>=3);std::cout<<"Captured moving launch requests: "<<moving<<"\n";
 }

 // Replay the observed idle -> stationary jump sequence at worker cadence.
 for(auto state:{0x2bca0570u,0x402b6a4eu,0x1ea2423fu,0x9aa9f111u}){
  tidebreaker35::HoldLaunch hold;
  tidebreaker35::Input in{1,1000,true,true,true,true,false,tidebreaker35::Phase::ground};
  assert(!hold.update(in));
  int fired=0;
  for(unsigned t=16;t<=2000;t+=16){
   in.now=1000+t;in.held=true;
   in.phase=t<48?tidebreaker35::Phase::ground:
    (launchSource204(state)?tidebreaker35::Phase::jumpStart:tidebreaker35::Phase::unavailable);
   if(hold.update(in)){assert(t==368);++fired;}
  }
  assert(fired==1);
 }
 auto m=flight_test::fixture();m.u32(flight_test::desc+4,1997601);m.u32(flight_test::node+0x18,0x26107d3c);
 auto observe=[&]{return flight_test::inspect(m);};
 clearLaunch204();assert(decideFlightGate(observe(),true).value==1);
 requestLaunch204(memoryfixture::actor,GetTickCount64());
 assert(decideFlightGate(observe(),true).value==0);
 assert(decideFlightGate(observe(),false).value==1);
 commaEnabled204.store(false);assert(decideFlightGate(observe(),true).value==1);
 commaEnabled204.store(true);requestLaunch204(memoryfixture::actor+8,GetTickCount64());
 assert(decideFlightGate(observe(),true).value==1);
 requestLaunch204(memoryfixture::actor,GetTickCount64());
 assert(!requestedLaunch204(memoryfixture::actor,0x26107d3c,GetTickCount64()+1000));
 assert(!requestedLaunch204(memoryfixture::actor,0x12345678,GetTickCount64()));
 clearLaunch204();assert(decideFlightGate(observe(),true).value==1);
 m.u32(flight_test::node+0x18,0x54424438);assert(decideFlightGate(observe(),true).value==0);
 std::cout<<"PASS launch gate: hold request, timeout, mode OFF, foreign actor, unknown state and existing hover route\n";
}
