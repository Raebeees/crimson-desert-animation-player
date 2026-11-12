#pragma once
#include "policy.h"
#include "ring_policy.h"
namespace boots204 {
using CeVariantOriginal204=std::uint64_t(*)(std::uint32_t,std::uint32_t);
inline bool preserveBootsVariant204(bool ready,bool equipped,std::uint32_t state,std::uint32_t incoming){
 if(!ready||!equipped)return false;
 // R47 captured a native event-dispatch null handler after repeated landing.
 // Diagnostic isolation: retain CE's original complete result for deceleration,
 // landing and cancel; do not zero its reason/flags on these paths.
 switch(state){
 case 0x269521ce:case 0x61b7853e:case 0xe906b0ea:
 case 0xe4ef0545:case 0x3ac4a94e:case 0x01ebbdbd:return false;
 default:break;
 }
 if(incoming!=0x54424340 && incoming!=0x54424c44 && incoming!=0x54424854 && incoming!=0x54425247)return false;
 const auto expected=boots::ringSelection(true,true,state,boots::variant(true,state,0));
 return expected==incoming;
}
// Current hash-pinned CE selector returns low DWORD variant, then four reason/flag bytes.
// Zero reason preserves the incoming selection in both audited callback callers.
inline std::uint64_t retainedBootsVariant204(std::uint32_t incoming){return incoming;}
}
