#include "effect_wrapper204.h"
#include "gate_fixture.h"
#include <cassert>
#include <vector>
#include <functional>
using namespace boots204;
std::vector<unsigned> stopped;std::function<void()> onStop;
void stopCrow(std::uintptr_t,const std::uint32_t* key,std::uint32_t group,bool immediate){
 assert(group==0&&immediate);stopped.push_back(*key);if(onStop)onStop();
}
struct Runtime {bool ready=true;bool assetsReady(){return ready;}bool slotsOwned(){return ready;}};
int main(){
 auto m=flight_test::fixture();Runtime runtime;
 constexpr std::uintptr_t effect=0x620000,active=0x621000,pending=0x625000;
 std::uintptr_t components=0;m.read(memoryfixture::actor+0x68,&components,8);
 m.ptr(components+0x60,effect);m.ptr(effect+8,memoryfixture::actor);
 m.ptr(effect+0x90,active);m.u32(effect+0x98,4);m.u32(effect+0x9c,4);
 m.ptr(effect+0xb0,pending);m.u32(effect+0xb8,1);m.u32(effect+0xbc,1);
 for(unsigned i=0;i<4;++i){m.u32(active+i*0x250,i<2?1000309+i:i==2?1997301:1000186);m.u32(active+i*0x250+4,i==2?0x54425247:0);}
 m.u32(pending,1000311);m.u32(pending+4,0);
 auto invoke=[&](){stopped.clear();return suppressCrowEffects204(m,runtime,&stopCrow,memoryfixture::base,memoryfixture::cc,effect);};
 assert(invoke()==3);assert((stopped==std::vector<unsigned>{1000309,1000310,1000311}));
 commaEnabled204.store(false);assert(invoke()==0);commaEnabled204.store(true);
 m.u32(memoryfixture::itemRecord+8,99);assert(invoke()==0);m.u32(memoryfixture::itemRecord+8,1987201);
 runtime.ready=false;assert(invoke()==0);runtime.ready=true;
 m.ptr(effect+8,memoryfixture::actor+8);assert(invoke()==0);m.ptr(effect+8,memoryfixture::actor);
 m.u32(active+4,1);assert(invoke()==2);m.u32(active+4,0);
 m.u32(effect+0x98,5);assert(invoke()==0);m.u32(effect+0x98,4);
 onStop=[&](){commaEnabled204.store(false);};assert(invoke()==1);commaEnabled204.store(true);onStop={};
 onStop=[&](){m.ptr(effect+8,memoryfixture::actor+8);};assert(invoke()==1);m.ptr(effect+8,memoryfixture::actor);onStop={};
 onStop=[&](){runtime.ready=false;};assert(invoke()==1);runtime.ready=true;onStop={};
 m.u32(effect+0x98,0);m.u32(effect+0xb8,0);assert(invoke()==0);
 std::cout<<"Crow suppression: exact keys, active/pending, boots/mode/owner gates, no rings/boost trails, bounded revalidation passed\n";
}
