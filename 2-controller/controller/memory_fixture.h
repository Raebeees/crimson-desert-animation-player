#pragma once
#include "resolver204.h"
#include <map>
#include <functional>
#include <iostream>
#include <stdexcept>

namespace memoryfixture {
using P=std::uintptr_t;
constexpr P base=0x140000000ULL, cc=0x180000000ULL;
constexpr P world=0x100000,manager=0x101000,user=0x102000,actor=0x103000,
 userLink=0x104000,components=0x105000,character=0x106000,control=0x107000,
 ownerLink=0x108000,scene=0x109000,appearance=0x10a000,vtable=0x10b000,
 equipment=0x10c000,slots=0x10d000,rows=0x10e000,
 charCache=0x120000,charArray=0x121000,charRecord=0x122000,
 itemCache=0x130000,itemArray=0x131000,itemRecord=0x132000;
struct Memory {
 std::map<P,unsigned char> bytes;
 std::map<P,unsigned> visits;
 P change=0;unsigned changeVisit=2;
 template<class T> void put(P at,T value) {
  const auto* b=reinterpret_cast<const unsigned char*>(&value);
  for(std::size_t i=0;i<sizeof(T);++i)bytes[at+i]=b[i];
 }
 void ptr(P at,P value){put<P>(at,value);}
 void u32(P at,std::uint32_t value){put(at,value);}
 void u16(P at,std::uint16_t value){put(at,value);}
 bool read(P at,void* out,std::size_t n) {
  const auto visit=++visits[at];
  auto* b=static_cast<unsigned char*>(out);
  for(std::size_t i=0;i<n;++i) {
   auto it=bytes.find(at+i);if(it==bytes.end())return false;b[i]=it->second;
  }
  if(at==change && visit>=changeVisit)b[0]^=1;
  return true;
 }
 Memory() {
  ptr(base+0x71c0100,world);ptr(world+0x30,manager);ptr(manager+0x58,user);
  ptr(user+0xd8,actor);ptr(user+8,userLink);ptr(userLink+0x38,actor);ptr(actor+0x68,components);
  ptr(components+0x20,character);u16(character+0x30,0);
  ptr(base+0x71c1020,charCache);u32(charCache+8,1);ptr(charCache+0x58,charArray);
  ptr(charArray,charRecord);u32(charRecord+8,1);
  ptr(components+0x40,control);ptr(control+0x58,ownerLink);ptr(ownerLink,actor);
  ptr(control+0xb8,scene);ptr(scene+0x20,appearance);ptr(appearance,vtable);ptr(cc+0x193508,vtable);
  ptr(components+0x38,equipment);ptr(equipment+0xa0,slots);ptr(slots+8,rows);u32(slots+0x10,2);
  ptr(base+0x71c1080,itemCache);u32(itemCache+8,2);ptr(itemCache+0x58,itemArray);
  ptr(itemArray,itemRecord);ptr(itemArray+8,itemRecord+0x100);
  u32(itemRecord+8,1987201);u32(itemRecord+0x108,220004);
  u16(rows+8,0);u16(rows+0xd0+8,1);
 }
};

} // namespace memoryfixture
