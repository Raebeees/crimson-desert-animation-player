#include "ring_retention204.h"
#include "gate_fixture.h"
using namespace boots204;
unsigned n=0,calls=0,error=13;void check(bool b){++n;if(!b)throw n;}
void native(std::uintptr_t c,std::uint32_t i,bool flag){check(c==0x620000&&i==0&&flag);++calls;check(error==13);error=71;}
struct Errors{unsigned get(){return error;}void set(unsigned e){error=e;}};
struct Runtime{bool ready=true;bool assetsReady(){return ready;}bool slotsOwned(){return ready;}};
int main(){auto m=flight_test::fixture();constexpr std::uintptr_t effect=0x620000,record=0x621000,array=0x622000,ref=0x623000,object=0x624000,groups=0x625000,objects=0x626000;std::uintptr_t components=0;m.read(memoryfixture::actor+0x68,&components,8);
m.ptr(components+0x60,effect);m.ptr(effect+8,memoryfixture::actor);m.ptr(effect+0x90,record);m.u32(effect+0x98,1);m.u32(effect+0x9c,1);m.u32(record,1997301);m.u32(record+4,0x54425247);m.put<std::uint8_t>(record+0x1f0,0);m.put<std::int8_t>(record+0x1f1,0);
m.ptr(record+0x1f8,array);m.u32(record+0x200,1);m.u32(record+0x204,1);m.ptr(array+16,ref);m.ptr(ref+8,object);m.put<std::uint8_t>(object+0x15,0);m.u32(record+0x210,0);m.u32(record+0x214,0);
Runtime runtime;Errors errors;auto call=[&](bool want,std::uintptr_t caller=memoryfixture::base+0x563a38){calls=0;error=13;forwardRetirement204(&native,effect,0,true,caller,m,runtime,errors,memoryfixture::base,memoryfixture::cc);check(calls==(want?0u:1u));check(error==(want?13u:71u));};
call(true);m.u32(flight_test::node+0x18,task008::dispatcher);call(true);EffectObservation204 dispatch;dispatch.verified=true;dispatch.identity.boots=EquipmentPresence::present;dispatch.state={task008::dispatcher,true};dispatch.active=1;check(cleanupDecision204(dispatch,true)==Cleanup204::keep);dispatch.state.id=0x01ebbdbd;check(cleanupDecision204(dispatch,true)==Cleanup204::request_native_stop);m.u32(flight_test::node+0x18,0x54424631);call(false,memoryfixture::base+0x55f741);call(false,memoryfixture::base+0x8c58ae0);runtime.ready=false;call(false);runtime.ready=true;
m.u32(memoryfixture::itemRecord+8,99);call(false);m.u32(memoryfixture::itemRecord+8,1987201);
m.put<std::uint8_t>(object+0x15,1);call(false);m.u32(record+0x200,0);m.u32(record+0x204,0);
m.ptr(record+0x208,groups);m.u32(record+0x210,1);m.u32(record+0x214,1);m.ptr(groups,objects);m.u32(groups+8,1);m.u32(groups+12,1);m.ptr(objects,object);
call(false);m.put<std::uint8_t>(object+0x15,2);call(true);m.put<std::uint8_t>(object+0x15,0);call(true);
m.put<std::int8_t>(record+0x1f1,1);call(false);m.put<std::int8_t>(record+0x1f1,0);
m.u32(groups+8,2);call(false);m.u32(groups+8,1);m.u32(record,99);call(false);m.u32(record,1997301);
m.u32(flight_test::node+0x18,0x01ebbdbd);call(false);m.ptr(effect+8,memoryfixture::actor+8);call(false);
std::cout<<"ring retention checks passed: "<<n<<" assertions\n";
}
