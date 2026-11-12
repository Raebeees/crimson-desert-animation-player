#pragma once
#include "resolver204.h"
#include <array>
namespace boots204 {
// Rejection only: a positive result never grants ownership or override permission.
// The caller must still run its complete fresh validation/stability journal.
template<class Reader> bool descriptorPrefix204(Reader& r,std::uintptr_t holder,
 std::uint32_t kind,std::array<std::uint32_t,5>& words) {
 Snapshot<Reader> s(r);std::uintptr_t d=0;
 if(holder<0x10000||holder>=0x800000000000ULL-8||holder%8||
    !s.read(holder,d)||d<0x10000||d>0x800000000000ULL-20||
    !s.read(d,words[0])||words[0]!=kind)return false;
 for(unsigned i=1;i<5;++i)if(!s.read(d+4*i,words[i]))return false;
 return true;
}
}
