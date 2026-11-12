#pragma once
#include <cstdint>
#include "task007_policy.h"
namespace boots {
constexpr uint32_t ringVariant=0x54425247,ringGroup=0x54425247;
inline bool ringState(uint32_t state) {
    state=task007::classify(state);
    switch(state) {
    case 0x15914F6D:
    case 0x164E8B07:
    case 0x2DCECD55:
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
inline bool privateRing(uint32_t key,uint32_t group) {
    return group==ringGroup && (key==1997301 || key==1997302);
}
inline uint32_t ringSelection(bool ready,bool scoped,uint32_t state,uint32_t current) {
    return ready && scoped && ringState(state) && current!=0x54424C44 && current!=0x54424340 ? ringVariant : current;
}
}
