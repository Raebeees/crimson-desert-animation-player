#pragma once
#include <atomic>
#include <array>
#include <cstdint>
#include "policy.h"
#include "task008_policy.h"
namespace boots204 {
// Append-only records: committed slots are never rewritten. A remote reader must
// read commit before and after the payload and accept only the same nonzero value.
struct HolderRecord204 {
 std::atomic<unsigned> commit{0};
 unsigned reserved=0;
 std::array<std::uint64_t,19> values{};
};
struct HolderBank204 {
 unsigned magic=0x37484254,version=1,capacity=64,words=19;
 HolderRecord204 records[64]{};
};
inline HolderBank204 holderBank204;
inline std::atomic_flag holderBusy204=ATOMIC_FLAG_INIT;
inline unsigned holderUsed204=0; // accessed only while holding nonblocking flag
inline std::atomic<unsigned> holderSkipped204{0},holderInvalid204{0},holderDuplicate204{0},holderFull204{0};
struct HolderUnlock204 { ~HolderUnlock204(){holderBusy204.clear(std::memory_order_release);} };
template<class S,class Runtime>
void observeHolder204(S& s,Runtime& runtime,std::uintptr_t control,std::uintptr_t candidate,std::uintptr_t upper,std::uintptr_t lower){
 if(holderBusy204.test_and_set(std::memory_order_acquire)){++holderSkipped204;return;}
 HolderUnlock204 unlock;
 if(holderUsed204==64){++holderFull204;return;}
 std::array<std::uint64_t,19> data{};data[0]=control;
 const std::uintptr_t addresses[3]={candidate,upper,lower};
 for(unsigned i=0;i<3;++i){
  const auto h=addresses[i];const unsigned j=1+i*6;data[j]=h;
  std::uintptr_t node=0;std::uint32_t state=0;std::uint8_t active=0;
  if(h<0x10000||h>=0x800000000000ULL-0x18||h%8||
    !s.read(h,data[j+1])||!s.read(h+8,data[j+2])||!s.read(h+0x10,node)||
    node<0x10000||node>=0x800000000000ULL-0x112||
    !s.read(node+0x18,state)||!s.read(node+0x111,active)){++holderInvalid204;return;}
  data[j+3]=node;data[j+4]=state;data[j+5]=active;
 }
 // Observation filter only: retain entry/exit candidates when either active lane
 // or the candidate is in a known flight state. Idle transitions consume no slot.
 const auto flight=[](std::uint64_t state){const auto v=static_cast<std::uint32_t>(state);return boots::flightState(v)||task008::privateExit(v);};
 if(!flight(data[5])&&!flight(data[11])&&!flight(data[17]))return;
 // Includes the caller's identity, equipment and control-lane reads in stability.
 if(!s.stable()||!runtime.assetsReady()||!runtime.slotsOwned()){++holderInvalid204;return;}
 for(unsigned i=0;i<holderUsed204;++i)if(holderBank204.records[i].values==data){++holderDuplicate204;return;}
 auto& out=holderBank204.records[holderUsed204];out.values=data;
 out.commit.store(++holderUsed204,std::memory_order_release);
}
}

