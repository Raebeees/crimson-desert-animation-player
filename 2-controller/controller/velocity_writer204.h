#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <cmath>
namespace boots204 {
struct VelocityWriter204 {
 static bool exchange(std::uintptr_t address,LONG expected,LONG desired) noexcept {
  __try{return InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(address),desired,expected)==expected;}
  __except(EXCEPTION_EXECUTE_HANDLER){return false;}
 }
 bool compareWrite(std::uintptr_t address,float expected,float value) const noexcept {
  if(address<0x10000||address>=0x800000000000ULL-4||address%4||!std::isfinite(expected)||!std::isfinite(value)||std::fabs(value)>3.f)return false;
  MEMORY_BASIC_INFORMATION info{};
  if(VirtualQuery(reinterpret_cast<void*>(address),&info,sizeof(info))!=sizeof(info)||info.State!=MEM_COMMIT||
    (info.Protect&(PAGE_GUARD|PAGE_NOACCESS))||(info.Protect&0xff)!=PAGE_READWRITE)return false;
  const auto start=reinterpret_cast<std::uintptr_t>(info.BaseAddress);
  if(address<start||address-start>info.RegionSize||info.RegionSize-(address-start)<4)return false;
  LONG old=0,next=0;static_assert(sizeof(float)==sizeof(LONG));std::memcpy(&old,&expected,4);std::memcpy(&next,&value,4);
  return exchange(address,old,next);
 }
};
} // namespace boots204
