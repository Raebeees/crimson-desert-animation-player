#pragma once
#include "flight_gate204.h"
#include "ring_policy.h"
namespace boots204 {
struct EffectObservation204 {bool verified=false;IdentityEquipment identity{};LowerState state{};std::uint32_t active=0,pending=0;std::uintptr_t component=0;};
template<class Reader> EffectObservation204 inspectEffects204(Reader& reader,std::uintptr_t base,std::uintptr_t cc,std::uintptr_t effect) {
 Snapshot<Reader> s(reader);EffectObservation204 o;std::uintptr_t components=0,actual=0,owner=0;
 if(effect<0x10000||effect>=0x800000000000ULL-0xc0||effect%8||
 !s.pointer(effect+8,owner)||!readIdentityEquipment(s,base,cc,o.identity,owner)||!s.pointer(o.identity.actor+0x68,components)||
 !s.pointer(components+0x60,actual)||actual!=effect||!s.pointer(effect+8,owner)||owner!=o.identity.actor||
 !readLowerState(s,o.identity.control,o.state))return {};
 for(unsigned list=0;list<2;++list){std::uint32_t count=0,capacity=0;std::uintptr_t data=0;const auto offset=list?0xb0u:0x90u,stride=list?0x208u:0x250u;
  if(!s.read(effect+offset+8,count)||!s.read(effect+offset+12,capacity)||count>capacity||capacity>4096||count>64)return {}; // bounded observation, unknown if exceeded
  if(count&&!s.pointer(effect+offset,data))return {};
  if(count&&data>=0x800000000000ULL-static_cast<std::uintptr_t>(count)*stride)return {};
  for(std::uint32_t i=0;i<count;++i){std::uint32_t key=0,group=0;
   if(!s.read(data+i*stride,key)||!s.read(data+i*stride+4,group))return {};
   if(boots::privateRing(key,group)){if(list)++o.pending;else ++o.active;}}
 }
 if(!s.stable())return {};o.verified=true;o.component=effect;return o;
}
enum class Cleanup204 {unknown,keep,request_native_stop,confirmed_clear};
inline Cleanup204 cleanupDecision204(const EffectObservation204& o,bool runtimeReady) {
 if(!o.verified)return Cleanup204::unknown;
 if(!o.active&&!o.pending)return Cleanup204::confirmed_clear;
 const bool flight=o.state.active&&(boots::ringState(o.state.id)||task008::dodgeRecovery(o.state.id)||o.state.id==task008::dispatcher);
 if(runtimeReady&&o.identity.boots==EquipmentPresence::present&&flight)return Cleanup204::keep;
 return Cleanup204::request_native_stop;
}
// A request is not proof of removal. Only a fresh verified snapshot with both
// lists clear permits confirmed_clear. No list erasure, native calls or writes.
} // namespace boots204
