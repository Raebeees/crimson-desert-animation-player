#pragma once
#include "input204.h"
#include "flight_gate204.h"
namespace boots204 {
// Native3B0150 selects first input object; constructorEEA0740 assigns table.
// Layout constants require the externally pinned game build, not RTTI guessing.
template<class Reader> bool readInputObject204(Snapshot<Reader>& s,std::uintptr_t base,std::uintptr_t& output){
 output=0;
 if(base<0x10000 || base>=0x800000000000ULL-0x71c0108 || base%8)return false;
 std::uintptr_t world=0,manager=0,container=0,array=0,input=0,table=0,method=0;
 std::uint32_t count=0;
 if(!s.pointer(base+0x71c0100,world)||!s.pointer(world+0xd8,manager)||!s.pointer(manager+0x20,container)||
    !s.read(container+0x28,count)||count==0||count>64||!s.pointer(container+0x20,array)||!s.pointer(array+0x10,input)||
    !s.pointer(input,table)||table!=base+0x5f1e070||!s.pointer(table+0x40,method)||method!=base+0x30cbb40)return false;
 output=input;return true;
}
struct OwnedActions204 {
 bool verified=false,suppressed=true;
 IdentityEquipment identity{};LowerState lower{};
 std::uintptr_t input=0;
 std::array<Action204,4> actions{Action204::unavailable,Action204::unavailable,Action204::unavailable,Action204::unavailable};
 unsigned reads=0;
};
// Preserve native remaps. These are the established ascent/crouch/guard/cancel hashes.
inline constexpr std::array<std::uint32_t,4> movementActions204{0x839b6e98,0x46e961d1,0x5bac9265,0xc81dc84a};
template<class Reader> OwnedActions204 inspectOwnedActions204(Reader& reader,std::uintptr_t base,std::uintptr_t cc){
 Snapshot<Reader> s(reader);OwnedActions204 result,candidate;
 std::uintptr_t components=0;
 if(!readIdentityEquipment(s,base,cc,candidate.identity)||!readLowerState(s,candidate.identity.control,candidate.lower)||
    !s.pointer(candidate.identity.actor+0x68,components)||!inputSuppressed204(s,components,candidate.suppressed)||
    !readInputObject204(s,base,candidate.input)){result.reads=s.reads();return result;}
 for(unsigned i=0;i<4;++i){candidate.actions[i]=readAction204(s,candidate.input,movementActions204[i]);
   if(candidate.actions[i]==Action204::unavailable){result.reads=s.reads();return result;}}
 if(s.stable()){candidate.verified=true;result=candidate;}
 result.reads=s.reads();return result;
}
// Observation only. No input suppression bypass, movement writes, or readiness grant.
} // namespace boots204
