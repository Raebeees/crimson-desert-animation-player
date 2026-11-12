#pragma once
#include "resolver204.h"
namespace boots204 {
enum class Action204 {unavailable,released,held};
// Pinned game method30C5640; native map lookup only. Does not invoke it.
// Caller verifies root/object identity and owns the shared journal/stable check.
template<class Reader> Action204 readAction204(Snapshot<Reader>& s,std::uintptr_t input,std::uint32_t key) {
 if(input<0x10000 || input>=0x800000000000ULL-0x1d0 || input%8)return Action204::unavailable;
 std::uint32_t count=0,buckets=0;
 if(!s.read(input+0x1b4,count)||!s.read(input+0x1b0,buckets)||!count||count>100000||!buckets||buckets>65536)return Action204::unavailable;
 std::uintptr_t bucketArray=0,entryArray=0;
 if(!s.pointer(input+0x1c0,bucketArray)||!s.pointer(input+0x1c8,entryArray))return Action204::unavailable;
 const auto bucket=bucketArray+std::uintptr_t(key%buckets)*0x100;
 std::uint32_t used=0;if(!s.read(bucket,used)||used>31)return Action204::unavailable;
 for(std::uint32_t i=0;i<used;++i){
  std::uint32_t hash=0,slot=0;
  if(!s.read(bucket+8+i*8,hash))return Action204::unavailable;
  if(hash!=key)continue;
  if(!s.read(bucket+12+i*8,slot)||slot>=100000)return Action204::unavailable;
  std::uintptr_t entry=0,action=0;
  if(!s.pointer(entryArray+std::uintptr_t(slot)*8,entry)||!s.read(entry+4,hash))return Action204::unavailable;
  if(hash!=key)continue; // Preserve native stale-entry continuation.
  if(!s.pointer(entry+8,action))return Action204::unavailable;
  std::uint8_t active=0,blocked=0;
  if(!s.read(action+0x5e,active)||!s.read(action+0x78,blocked))return Action204::unavailable;
  return active&&!blocked?Action204::held:Action204::released;
 }
 // Missing binding is unknown, not permission to override normal controls.
 return Action204::unavailable;
}
template<class Reader> bool inputSuppressed204(Snapshot<Reader>& s,std::uintptr_t components,bool& blocked){
 blocked=true;std::uintptr_t input=0;std::uint8_t value=0;
 if(components<0x10000 || components>=0x800000000000ULL-0x1b8 || components%8 ||
    !s.pointer(components+0x1b0,input)||!s.read(input+0x179,value))return false;
 blocked=value!=0;return true;
}
} // namespace boots204
