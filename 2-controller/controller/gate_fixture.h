#pragma once
#include "flight_gate204.h"
#include "memory_fixture.h"
namespace flight_test {
using memoryfixture::Memory;using P=boots204::EquipmentPresence;
constexpr std::uintptr_t ctx=0x200000,holder=0x201000,desc=0x202000,stateHolder=0x203000,node=0x204000;
Memory fixture(){Memory m;m.ptr(ctx+8,memoryfixture::actor);m.ptr(holder,desc);m.u32(desc,238);m.u32(desc+4,1997602);m.u32(desc+8,1);m.u32(desc+12,0);m.u32(desc+16,0);
 m.ptr(memoryfixture::control+0x90,stateHolder);m.ptr(stateHolder,1);m.ptr(stateHolder+8,2);m.ptr(stateHolder+0x10,node);m.put<std::uint8_t>(node+0x111,1);m.u32(node+0x18,0x54424631);return m;}
auto inspect(Memory& m){return boots204::inspectFlightGate(m,memoryfixture::base,memoryfixture::cc,ctx,holder);}
void check(bool yes,const char* label){if(!yes)throw std::runtime_error(label);}
}
