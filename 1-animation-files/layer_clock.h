#pragma once
#include "controller/resolver204.h"
#include <cmath>
#include "selection_v2.h"
#include <array>
#include <algorithm>
namespace lowcortisol_engine {
constexpr std::uint32_t idleAction=0x26107d3c;
inline int part(std::uint32_t id){return dance_v2::known(id)?0:-1;}
inline bool chartEntry(unsigned layer,std::uint32_t id){
 if(layer==0)return id==0x25dc7fb9||id==0x26107d3c||id==0x45cdcad0||id==0x4e37c32f||id==0x7728ba59||id==0x874f689b||id==0x9070ea87||id==0xc2a1f123||id==0xecf9b7d2||id==0xfad22852||id==0xfbd56b20;
 if(layer==1)return id==0x26107d3c;
 return false;
}
struct Layer {std::uint32_t action=0;bool active=false;float previous=0,current=0;};
// Current-build static evidence: 251EA20 copies holder+20/+28/+30 and passes
// holder+20's two floats to 251ED90 -> 251E5A0 -> 251FBE0 timed-window tests.
// Both layers are selected as control+88+8*layer. No native functions called.
// Requires hash-pinned host and a stable shared observation journal.
template<class Reader> bool readLayer(boots204::Snapshot<Reader>& s,std::uintptr_t control,unsigned layer,Layer& out){
 out={};if(layer>1||control<0x10000||control%8||control>0x7fffffffffffULL-0x98)return false;
 std::uintptr_t holder=0,node=0;std::uint64_t valid0=0,valid8=0;std::uint8_t active=0;
 if(!s.pointer(control+0x88+8*layer,holder)||!s.read(holder,valid0)||!s.read(holder+8,valid8))return false;
 if(!valid0||!valid8)return true;
 if(!s.read(holder+0x10,node)||node<0x10000||node>0x7fffffffffffULL-0x112||!s.read(node+0x111,active))return false;
 if(!active)return true;
 if(!s.read(node+0x18,out.action)||!s.read(holder+0x20,out.previous)||!s.read(holder+0x24,out.current))return false;
 if(!std::isfinite(out.previous)||!std::isfinite(out.current)||out.previous<0||out.current<0)return false;
 out.active=true;return true;
}
inline double position(const Layer& l){const int i=part(l.action);return l.active&&i>=0?l.current+(l.action==dance_v2::vervePartTwo?33.5:0.):-1.;}
struct Pair {
 std::uint64_t actor=0,identity=0;
 Layer upper{},lower{};
 bool verified=false,eligible=false;
 unsigned ownedMask()const{return (upper.active&&part(upper.action)>=0?1u:0u)|(lower.active&&part(lower.action)>=0?2u:0u);}
 bool standing()const{return upper.active&&lower.active&&chartEntry(0,upper.action)&&chartEntry(1,lower.action);}
 bool clock(double& seconds,std::uint32_t selected)const{
  if(ownedMask()!=3||dance_v2::canonical(upper.action)!=selected||dance_v2::canonical(lower.action)!=selected)return false;
  const double a=position(upper),b=position(lower);
  // Diagnostic refusal bound, not a claim about the game's permitted blend skew.
  if(std::abs(a-b)>.100||a>dance_v2::duration(selected)+.100||b>dance_v2::duration(selected)+.100)return false;
  seconds=(std::min)(a,b);return true;
 }
};
}
