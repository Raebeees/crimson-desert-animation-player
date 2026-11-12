#pragma once
#include "resolver204.h"
namespace boots204 {
struct LowerState {std::uint32_t id=0;bool active=false;};
// Native current consumer242467A..24246A8. Must be validated with all other
// observations in the caller's journal. No native functions are called.
template<class Reader> bool readLowerState(Snapshot<Reader>& s,std::uintptr_t control,LowerState& output) {
 output={};
 if(control<0x10000 || control>=0x800000000000ULL-0x98 || control%8)return false;
 std::uintptr_t holder=0,node=0;std::uint64_t valid0=0,valid8=0;std::uint8_t active=0;
 if(!s.pointer(control+0x90,holder) || !s.read(holder,valid0) || !s.read(holder+8,valid8))return false;
 if(!valid0 || !valid8)return true;
 // Live corroboration: native active nodes can be unaligned. Only this
 // byte-addressed structure relaxes alignment; keep range and read checks.
 if(!s.read(holder+0x10,node) || node<0x10000 || node>=0x800000000000ULL-0x112 ||
    !s.read(node+0x111,active))return false;
 if(!active)return true;
 if(!s.read(node+0x18,output.id))return false;
 output.active=true;return true;
}
inline bool privateFlightState(std::uint32_t id) {
 return (id>=0x54424631 && id<=0x54424635) || (id>=0x54424431 && id<=0x54424438);
}
inline bool entryContext(std::uint32_t id) {
 switch(id) {
 case 0x9c0c7e5b:case 0x4abebf9d:case 0x164e8b07:case 0x8ed159f1:
 case 0x54424631:case 0x54424632:case 0x54424633:case 0x54424634:case 0x54424635:
 case 0x54424438:case 0xeb340761:return true;
 default:return false;
 }
}
inline bool exitContext204(std::uint32_t id) {
 if(entryContext(id) || privateFlightState(id))return true;
 switch(id) {
 case 0x6ecd9c21:case 0x64f500eb:case 0x3df1d45f:case 0x1d49e726:
 case 0xd28abcec:case 0x9bef84e4:case 0xbf31394a:return true;
 default:return false;
 }
}
} // namespace boots204
