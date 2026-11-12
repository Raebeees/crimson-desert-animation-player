#pragma once
#include "effect_cleanup204.h"
namespace boots204 {
using EffectUpdateOriginal204=std::uintptr_t(*)(std::uintptr_t);
using StopPrivateEffect204=void(*)(std::uintptr_t,const std::uint32_t*,std::uint32_t,bool);
struct CleanupResult204 {Cleanup204 status=Cleanup204::unknown;unsigned stopCalls=0;};
template<class Reader,class Runtime>
CleanupResult204 cleanupPrivateEffects204(Reader& reader,Runtime& runtime,StopPrivateEffect204 stop,
 std::uintptr_t base,std::uintptr_t cc,std::uintptr_t effect) {
 CleanupResult204 result;
 if(!stop)return result;
 // At most one request per private key per native update. Deferred cleanup
 // is rechecked on the next update; no cross-frame identity cache.
 for(unsigned i=0;i<2;++i){
  if(!runtime.assetsReady() || !runtime.slotsOwned()){result.status=Cleanup204::unknown;return result;}
  const auto o=inspectEffects204(reader,base,cc,effect);
  result.status=cleanupDecision204(o,runtime.assetsReady()&&runtime.slotsOwned());
  if(result.status!=Cleanup204::request_native_stop)return result;
  const std::uint32_t key=(i%2)?1997302u:1997301u;
  if(!runtime.assetsReady() || !runtime.slotsOwned()){result.status=Cleanup204::unknown;return result;}
  stop(effect,&key,boots::ringGroup,true);++result.stopCalls;
 }
 const auto o=inspectEffects204(reader,base,cc,effect);
 result.status=cleanupDecision204(o,runtime.assetsReady()&&runtime.slotsOwned());
 return result;
}
// Exact effectinfo keys identified in the current mounted table. Do not suppress
// boost trails or private rings. The central identity gate includes comma mode.
template<class Reader> bool observedCrowEffect204(Reader& reader,std::uintptr_t base,std::uintptr_t cc,
 std::uintptr_t effect,std::uint32_t wanted) {
 if(wanted!=1000309 && wanted!=1000310 && wanted!=1000311)return false;
 Snapshot<Reader> s(reader);IdentityEquipment identity;std::uintptr_t components=0,actual=0,owner=0;
 if(effect<0x10000 || effect>=0x800000000000ULL-0xc0 || effect%8 ||
    !s.pointer(effect+8,owner) || !readIdentityEquipment(s,base,cc,identity,owner) ||
    identity.boots!=EquipmentPresence::present || !s.pointer(identity.actor+0x68,components) ||
    !s.pointer(components+0x60,actual) || actual!=effect || owner!=identity.actor)return false;
 bool found=false;
 for(unsigned list=0;list<2;++list){
  std::uint32_t count=0,capacity=0;std::uintptr_t data=0;
  const auto offset=list?0xb0u:0x90u,stride=list?0x208u:0x250u;
  if(!s.read(effect+offset+8,count)||!s.read(effect+offset+12,capacity)||count>capacity||capacity>4096||count>64)return false;
  if(count && (!s.pointer(effect+offset,data)||data>=0x800000000000ULL-static_cast<std::uintptr_t>(count)*stride))return false;
  for(unsigned i=0;i<count;++i){std::uint32_t key=0,group=0;
   if(!s.read(data+i*stride,key)||!s.read(data+i*stride+4,group))return false;
   if(key==wanted && group==0)found=true;
  }
 }
 return found && s.stable();
}
template<class Reader,class Runtime>
unsigned suppressCrowEffects204(Reader& reader,Runtime& runtime,StopPrivateEffect204 stop,
 std::uintptr_t base,std::uintptr_t cc,std::uintptr_t effect) {
 unsigned crowStops=0;if(!stop)return crowStops;
 for(const std::uint32_t key:{1000309u,1000310u,1000311u}){
  if(!runtime.assetsReady()||!runtime.slotsOwned())break;
  if(observedCrowEffect204(reader,base,cc,effect,key) && runtime.assetsReady() && runtime.slotsOwned()){
   stop(effect,&key,0,true);++crowStops;
  }
 }
 return crowStops;
}
// Execute only as the hooked native update on its own thread. Never invoke
// this to poll or create an update. Unknown ownership is forwarded, not guessed.
template<class Reader,class Runtime,class Errors>
std::uintptr_t forwardEffectUpdate204(EffectUpdateOriginal204 original,StopPrivateEffect204 stop,std::uintptr_t effect,
 Reader& reader,Runtime& runtime,Errors& errors,std::uintptr_t base,std::uintptr_t cc,CleanupResult204& cleanup) {
 const auto native=original(effect);const auto saved=errors.get();cleanup={};
 try{cleanup=cleanupPrivateEffects204(reader,runtime,stop,base,cc,effect);
     suppressCrowEffects204(reader,runtime,stop,base,cc,effect);}catch(...){cleanup.status=Cleanup204::unknown;}
 errors.set(saved);return native;
}
} // namespace boots204


