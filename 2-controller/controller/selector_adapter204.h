#pragma once
#include "flight_gate204.h"
#include "policy.h"
#include "ring_policy.h"
#include <atomic>
#include "holder_observer204.h"
#include "holder_membership204.h"
namespace boots204 {
inline std::atomic<unsigned> selectorCounts204[6]{};
inline std::atomic<unsigned> selectorReject204[12]{},selectorIdentityProbe204[3]{};
inline std::atomic<unsigned> selectorLastState204{0},selectorLastNative204{0},selectorLastResult204{0};
using SelectorOriginal204=std::uint32_t(*)(std::uintptr_t,std::uintptr_t);
// This adapter wraps CC's own selector, never its generated game stub.
// Runtime ownership and complete asset readiness are mandatory inputs.
template<class Reader,class Runtime,class Errors>
std::uint32_t forwardSelector204(SelectorOriginal204 original,std::uintptr_t control,std::uintptr_t holder,
 Reader& reader,Runtime& runtime,Errors& errors,std::uintptr_t base,std::uintptr_t cc) {
 selectorCounts204[0].fetch_add(1,std::memory_order_relaxed);
 const auto native=original(control,holder);const auto lastError=errors.get();
 auto result=native;
 try {
  Snapshot<Reader> s(reader);IdentityEquipment identity;std::uintptr_t upper=0,lower=0,node=0;
  std::uint64_t valid0=0,valid8=0;std::uint8_t active=0;std::uint32_t state=0;
  unsigned rejected=99;
  const auto accept=[&](bool ok,unsigned stage){if(!ok){rejected=stage;selectorReject204[stage].fetch_add(1,std::memory_order_relaxed);}return ok;};
  if(accept(runtime.assetsReady()&&runtime.slotsOwned(),0)&&
    accept(readIdentityEquipment(s,base,cc,identity,0,control),1)&&
    accept(identity.boots==EquipmentPresence::present,2)&&
    accept(s.read(control+0x88,upper)&&s.read(control+0x90,lower),3)&&
    accept(holder==upper||holder==lower||copiedFlightHolder204(s,base,control,holder,upper,lower),4)&&
    accept(holder>=0x10000&&holder<0x800000000000ULL-0x18&&holder%8==0,5)&&
    accept(s.read(holder,valid0)&&s.read(holder+8,valid8)&&valid0&&valid8,6)&&
    accept(s.read(holder+0x10,node)&&node>=0x10000&&node<0x800000000000ULL-0x112,7)&&
    accept(s.read(node+0x111,active)&&active,8)&&
    accept(s.read(node+0x18,state),9)&&accept(s.stable(),10)&&
    accept(runtime.assetsReady()&&runtime.slotsOwned(),11)) {
   selectorCounts204[1].fetch_add(1,std::memory_order_relaxed);
   selectorLastState204.store(state,std::memory_order_relaxed);
   selectorLastNative204.store(native,std::memory_order_relaxed);
   if(state==task008::dispatcher)result=native;
   else if(task008::dodgeRecovery(state))result=task008::select(true,true,state,native);
   else result=boots::ringSelection(true,true,state,boots::variant(true,state,native));
   selectorLastResult204.store(result,std::memory_order_relaxed);
   selectorCounts204[result==boots::ringVariant?2:3].fetch_add(1,std::memory_order_relaxed);
   if(result!=native)selectorCounts204[4].fetch_add(1,std::memory_order_relaxed);
  }
  (void)rejected; // No diagnostic-only rescans in gameplay.

 }catch(...){selectorCounts204[5].fetch_add(1,std::memory_order_relaxed);result=native;}
 errors.set(lastError);return result;
}
} // namespace boots204




