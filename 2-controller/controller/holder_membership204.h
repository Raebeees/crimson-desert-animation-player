#pragma once
#include "policy.h"
#include "task008_policy.h"
namespace boots204 {
// Native 2.04.07 lookup2626580 copies a 24-byte record to a temporary holder.
// Membership is checked only after player identity and equipped-boots validation.
template<class S> bool copiedFlightHolder204(S& s,std::uintptr_t base,std::uintptr_t control,
 std::uintptr_t candidate,std::uintptr_t upper,std::uintptr_t lower){
 const auto range=[](std::uintptr_t p,std::size_t n){return p>=0x10000&&p<0x800000000000ULL-n;};
 if(!range(candidate,24)||candidate%8)return false;
 std::uintptr_t vt=0,fn=0;std::uint64_t w0=0,w8=0,node=0;std::uint32_t target=0;std::uint8_t active=0;
 if(!s.pointer(control,vt)||!range(vt,0x338)||!s.read(vt+0x330,fn)||fn!=base+0x948980||
    !s.read(candidate,w0)||!s.read(candidate+8,w8)||!w0||!w8||!s.read(candidate+16,node)||
    !range(node,0x112)||!s.read(node+24,target)||!s.read(node+0x111,active)||!active||
    !(boots::flightState(target)||task008::privateExit(target)))return false;
 for(auto lane:{upper,lower}){
  std::uintptr_t table=0,array=0;std::uint32_t count=0;
  if(!range(lane,0x120)||lane%8||!s.read(lane+0x118,table))return false;
  if(!table)continue;
  if(!range(table,0x24)||table%8||!s.read(table+0x20,count)||count>65536)return false;
  if(!count)continue;
  if(!s.pointer(table+0x18,array)||!range(array,static_cast<std::size_t>(count)*24))return false;
  std::uint32_t lo=0,hi=count;
  for(unsigned step=0;step<17&&lo<hi;++step){
   const auto mid=lo+(hi-lo)/2;const auto at=array+static_cast<std::uintptr_t>(mid)*24;
   std::uint64_t a=0,b=0,n=0;std::uint32_t key=0;std::uint8_t enabled=0;
   if(!s.read(at,a)||!s.read(at+8,b)||!a||!b||!s.read(at+16,n)||!range(n,0x112)||
      !s.read(n+24,key)||!s.read(n+0x111,enabled)||!enabled)return false;
   if(key==target){if(a==w0&&b==w8&&n==node)return true;break;}
   if(key<target)lo=mid+1;else hi=mid;
  }
 }
 return false;
}
}

