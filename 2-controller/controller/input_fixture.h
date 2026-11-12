#pragma once
#include "input_identity204.h"
#include "gate_fixture.h"
namespace identity_test {
using namespace boots204;using M=memoryfixture::Memory;
constexpr std::uintptr_t manager=0x500000,container=0x501000,array=0x502000,input=0x503000,buckets=0x504000,entries=0x505000,entry=0x506000,action=0x507000,suppression=0x508000;
// Read fixture's controlled world/component pointers via its reader, not guessed aliases.
std::uintptr_t pointer(M& m,std::uintptr_t address){std::uintptr_t v=0;if(!m.read(address,&v,8))throw std::runtime_error("fixture pointer");return v;}
M fixture(){auto m=flight_test::fixture();auto world=pointer(m,memoryfixture::base+0x71c0100);auto components=pointer(m,memoryfixture::actor+0x68);
 m.ptr(world+0xd8,manager);m.ptr(manager+0x20,container);m.u32(container+0x28,1);m.ptr(container+0x20,array);m.ptr(array+0x10,input);m.ptr(input,memoryfixture::base+0x5f1e070);m.ptr(memoryfixture::base+0x5f1e0b0,memoryfixture::base+0x30cbb40);
 m.ptr(components+0x1b0,suppression);m.put<std::uint8_t>(suppression+0x179,0);m.u32(input+0x1b0,1);m.u32(input+0x1b4,4);m.ptr(input+0x1c0,buckets);m.ptr(input+0x1c8,entries);m.u32(buckets,4);
 for(unsigned i=0;i<4;++i){m.u32(buckets+8+i*8,movementActions204[i]);m.u32(buckets+12+i*8,i);m.ptr(entries+i*8,entry+i*0x100);m.u32(entry+i*0x100+4,movementActions204[i]);m.ptr(entry+i*0x100+8,action+i*0x100);m.put<std::uint8_t>(action+i*0x100+0x5e,i==0?1:0);m.put<std::uint8_t>(action+i*0x100+0x78,0);}
 return m;}
void check(bool yes,const char* why){if(!yes)throw std::runtime_error(why);}
auto observe(M& m){return inspectOwnedActions204(m,memoryfixture::base,memoryfixture::cc);}
}
