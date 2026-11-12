#pragma once
#include "condition_slots204.h"
#include <cstring>
namespace boots204 {
struct CePin204 {
    Target gameRva, callbackRva;
    unsigned char suffix[11], callback[16];
};
inline constexpr CePin204 cePins204[] = {
    {0x3b0a30,0xe890,{0x90,0x90,0x8b,0x50,8,0x48,0x8b,0x41,8,0x48,0x8b},{0x48,0x89,0x5c,0x24,0x10,0x48,0x89,0x6c,0x24,0x18,0x56,0x57,0x41,0x55,0x41,0x56}},
    {0x3b0a90,0x10060,{0x48,0x89,0x6c,0x24,0x18,0x48,0x89,0x74,0x24,0x20,0x57},{0x40,0x55,0x53,0x56,0x57,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,0x48,0x8d,0x6c}},
    {0x3acbb0,0x12f00,{0x48,0x89,0x6c,0x24,0x18,0x48,0x89,0x74,0x24,0x20,0x57},{0x48,0x89,0x5c,0x24,8,0x4c,0x89,0x4c,0x24,0x20,0x4c,0x89,0x44,0x24,0x18,0x48}}
};
// Module bases and file identities must be independently verified and retained.
template<class Reader> struct CeChains204 {
    Reader& reader;
    Target game, ce;
    bool valid(const std::array<Target,3>& originals) const noexcept {
        constexpr Target limit=0x7fffffffffffULL;
        if(game<0x10000 || game>limit-0x3c0000 || ce<0x10000 || ce>limit-0x13000 || game==ce) return false;
        for(unsigned i=0;i<3;++i){
            const auto& pin=cePins204[i];
            const Target entry=game+pin.gameRva, expected=ce+pin.callbackRva;
            unsigned char bytes[16], relay[14];
            if(originals[i]!=entry || !reader.read(entry,bytes,sizeof bytes) || bytes[0]!=0xe9 ||
               std::memcmp(bytes+5,pin.suffix,sizeof pin.suffix)) return false;
            std::int32_t relative;
            std::memcpy(&relative,bytes+1,4);
            const auto address=static_cast<std::int64_t>(entry+5)+relative;
            if(address<0x10000 || address>static_cast<std::int64_t>(limit-14) ||
               !reader.read(static_cast<Target>(address),relay,sizeof relay)) return false;
            constexpr unsigned char jump[]={0xff,0x25,0,0,0,0};
            Target destination;
            static_assert(sizeof(destination)==8);
            std::memcpy(&destination,relay+6,sizeof destination);
            if(std::memcmp(relay,jump,sizeof jump) || destination!=expected ||
               !reader.read(expected,bytes,sizeof bytes) || std::memcmp(bytes,pin.callback,sizeof bytes)) return false;
        }
        return true;
    }
};
}
