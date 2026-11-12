#pragma once
#include "resolver204.h"
#include <array>
#include <atomic>

namespace boots204 {
inline std::atomic<unsigned> descriptorReject204[9]{};
inline std::atomic<unsigned> descriptorLastKind204{0},descriptorLastSkill204{0};
struct PrivateDescriptor {
    std::uintptr_t actor=0, address=0;
    std::array<std::uint32_t,5> words{};
};
// Append to the caller's observation journal. Caller MUST call stable() after
// completing all ownership/state reads before using this result. No readiness
// or equipment/flight eligibility is granted here.
template<class Reader> bool readPrivateDescriptor(Snapshot<Reader>& snapshot,
        std::uintptr_t context,std::uintptr_t holder,std::uintptr_t expectedActor,
        PrivateDescriptor& output) {
    output={};
    constexpr std::uintptr_t limit=0x800000000000ULL;
    if(context<0x10000 || context>=limit-16 || context%8 ||
       holder<0x10000 || holder>=limit-8 || holder%8 ||
       expectedActor<0x10000 || expectedActor>=limit || expectedActor%8){descriptorReject204[0].fetch_add(1,std::memory_order_relaxed);return false;}
    PrivateDescriptor candidate;
    if(!snapshot.pointer(context+8,candidate.actor)){descriptorReject204[1].fetch_add(1,std::memory_order_relaxed);return false;}
    if(candidate.actor!=expectedActor){descriptorReject204[2].fetch_add(1,std::memory_order_relaxed);return false;}
    if(!snapshot.read(holder,candidate.address)){descriptorReject204[3].fetch_add(1,std::memory_order_relaxed);return false;}
    // Chart condition payloads are packed; descriptor targets need not be pointer-aligned.
    // Context, holder and actor alignment checks remain above.
    if(candidate.address<0x10000 || candidate.address>limit-20){
        snapshot.failure=Failure::invalid;
        descriptorReject204[4].fetch_add(1,std::memory_order_relaxed);return false;
    }
    // Current dispatcher3AA0E2/E6/E9 corroborates context+8, [holder], DWORD kind.
    // Only the private skill schema is accepted. Never read other kinds as20bytes.
    if(!snapshot.read(candidate.address,candidate.words[0])){descriptorReject204[5].fetch_add(1,std::memory_order_relaxed);return false;}
    descriptorLastKind204.store(candidate.words[0],std::memory_order_relaxed);
    if(candidate.words[0]!=238){descriptorReject204[5].fetch_add(1,std::memory_order_relaxed);return false;}
    if(!snapshot.read(candidate.address+4,candidate.words[1])){descriptorReject204[6].fetch_add(1,std::memory_order_relaxed);return false;}
    descriptorLastSkill204.store(candidate.words[1],std::memory_order_relaxed);
    if(candidate.words[1]!=1997601 && candidate.words[1]!=1997602){descriptorReject204[6].fetch_add(1,std::memory_order_relaxed);return false;}
    for(unsigned i=2;i<5;++i)
        if(!snapshot.read(candidate.address+i*4,candidate.words[i])){descriptorReject204[7].fetch_add(1,std::memory_order_relaxed);return false;}
    if(candidate.words[2]!=1 || candidate.words[3]!=0 || candidate.words[4]!=0){descriptorReject204[7].fetch_add(1,std::memory_order_relaxed);return false;}
    descriptorReject204[8].fetch_add(1,std::memory_order_relaxed);
    output=candidate;return true;
}
} // namespace boots204
