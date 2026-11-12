#pragma once
#include <cstdint>
#include "task007_policy.h"
namespace boots {
constexpr uint32_t item=1987201, rocket=0xEDBDEF3D;
inline bool transitionVisualState(uint32_t s){return s==0x15914f6d||s==0x269521ce||s==0x61b7853e||s==0xe4ef0545||s==0x3ac4a94e||s==0x36d65fb4||s==0xe906b0ea;}
inline bool flightState(uint32_t s) {
    s=task007::classify(s);
    if(transitionVisualState(s))return true;
    // Only states with an Oongka variant observed in the loaded native charts.
    switch(s) {
    case 0x9C0C7E5B:case 0x15914F6D:case 0xEB340761:case 0x766AA1E9:
    case 0x164E8B07:case 0x8ED159F1:case 0x4ABEBF9D:case 0x01EBBDBD:
    case 0x2DCECD55:case 0x815B8344:case 0xB3BC3663:case 0x36D65FB4:
    // Combat Evolved's additional native rocket boost states.
    case 0x4266B3CA:case 0xEF25D4BD:case 0x3AB7188E:case 0xCD285377:return true;
    default:return false;
    }
}
inline bool hatSafeState(uint32_t s) {
    s=task007::classify(s);
    switch(s) {
    case 0x15914F6D:
    case 0x164E8B07:
    case 0x36D65FB4:
    case 0x3AB7188E:
    case 0x4ABEBF9D:
    case 0x815B8344:
    case 0x8ED159F1:
    case 0x9C0C7E5B:
    case 0xB3BC3663:
    case 0xCD285377:
    case 0xEB340761:
    case 0xEF25D4BD:
        return true;
    default:return false;
    }
}
inline uint32_t variant(bool scoped,uint32_t state,uint32_t original) {
    state=task007::classify(state);
    if(scoped && transitionVisualState(state))return 0x54424340;
    // Private file-based landing variant. Existing scope checks remain unchanged.
    if(scoped && state==0x01EBBDBD)return 0x54424C44;
    if(scoped && hatSafeState(state))return 0x54424854;
    return scoped && flightState(state)?rocket:original;
}
inline unsigned char skill(bool scoped,uint32_t key,unsigned char original) {
    if(!scoped || original>1)return original;
    // Exact flight gate set recovered from the reference, including its
    // separate double-jump prerequisite (0x27fb). Does not edit learned saves.
    switch(key) {
    case 10235:case 10244:case 10245:case 10246:case 15066:return 0;
    // Native superdash spends Mp/Spirit. Keep it unavailable for the boots
    // until a boots-only stamina version exists; ordinary boost stays enabled.
    case 15048:return 1;
    case 15029:case 15045:case 10264:case 15046:return 1;
    default:return original;
    }
}
}
