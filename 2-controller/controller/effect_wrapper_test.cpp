#include "effect_wrapper204.h"
#include "gate_fixture.h"
#include <functional>
using namespace boots204;
unsigned n=0,calls=0,error=0;std::function<void(std::uint32_t)> onStop;
void check(bool b){++n;if(!b)throw n;}
std::uintptr_t native(std::uintptr_t){++calls;error=71;return 0x123456789abcdef0ULL;}
void stop(std::uintptr_t,const std::uint32_t* key,std::uint32_t group,bool immediate){check(group==0x54425247&&immediate&&(*key==1997301||*key==1997302));error=99;if(onStop)onStop(*key);}
struct Errors{unsigned get(){return error;}void set(unsigned v){error=v;}};
struct Runtime{bool ready=true;bool assetsReady(){return ready;}bool slotsOwned(){return ready;}};
int main(){
auto m=flight_test::fixture();constexpr std::uintptr_t effect=0x620000,active=0x621000,pending=0x622000;std::uintptr_t components=0;m.read(memoryfixture::actor+0x68,&components,8);
m.ptr(components+0x60,effect);m.ptr(effect+8,memoryfixture::actor);m.ptr(effect+0x90,active);m.u32(effect+0x98,1);m.u32(effect+0x9c,1);m.ptr(effect+0xb0,pending);m.u32(effect+0xb8,1);m.u32(effect+0xbc,1);
m.u32(active,1997301);m.u32(active+4,0x54425247);m.u32(pending,1997302);m.u32(pending+4,0x54425247);

Runtime runtime;Errors errors;CleanupResult204 result;
auto invoke=[&]{calls=0;auto v=forwardEffectUpdate204(&native,&stop,effect,m,runtime,errors,memoryfixture::base,memoryfixture::cc,result);check(calls==1&&v==0x123456789abcdef0ULL&&error==71);};
invoke();check(result.status==Cleanup204::keep&&result.stopCalls==0);
m.u32(memoryfixture::itemRecord+8,99);
onStop=[&](std::uint32_t key){if(key==1997301)m.u32(effect+0x98,0);else m.u32(effect+0xb8,0);};invoke();check(result.status==Cleanup204::confirmed_clear&&result.stopCalls==2);
m.u32(effect+0x98,1);m.u32(effect+0xb8,1);onStop={};invoke();check(result.status==Cleanup204::request_native_stop&&result.stopCalls==2);
// Deferred native stop: every future update stays bounded, then detects completion.
for(unsigned frame=0;frame<100;++frame){invoke();check(result.status==Cleanup204::request_native_stop&&result.stopCalls==2);}
onStop=[&](std::uint32_t key){if(key==1997301)m.u32(effect+0x98,0);else m.u32(effect+0xb8,0);};invoke();check(result.status==Cleanup204::confirmed_clear&&result.stopCalls==2);
onStop={};invoke();check(result.status==Cleanup204::confirmed_clear&&result.stopCalls==0);
m.u32(effect+0x98,1);m.u32(effect+0xb8,1);
m.ptr(effect+8,memoryfixture::actor+8);invoke();check(result.status==Cleanup204::unknown&&result.stopCalls==0);m.ptr(effect+8,memoryfixture::actor);
onStop=[&](std::uint32_t){m.ptr(effect+8,memoryfixture::actor+8);};invoke();check(result.status==Cleanup204::unknown&&result.stopCalls==1);m.ptr(effect+8,memoryfixture::actor);
m.u32(memoryfixture::itemRecord+8,1987201);runtime.ready=false;onStop=[&](std::uint32_t key){if(key==1997301)m.u32(effect+0x98,0);else m.u32(effect+0xb8,0);};invoke();check(result.status==Cleanup204::unknown&&result.stopCalls==0);
runtime.ready=true;m.u32(memoryfixture::itemRecord+8,99);onStop=[&](std::uint32_t){runtime.ready=false;};invoke();check(result.status==Cleanup204::unknown&&result.stopCalls==1);
runtime.ready=true;
m.u32(effect+0x98,2);m.u32(effect+0x9c,1);invoke();check(result.status==Cleanup204::unknown&&result.stopCalls==0);
std::cout<<"effect wrapper checks passed: "<<n<<" assertions\n";
}

