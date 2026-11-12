#pragma once
#include "policy.h"
namespace boots {
inline bool controlledFlightState(uint32_t state) {
    state=task007::classify(state);
    // Launch, landing and native vertical-launch actions retain their own motion.
    switch(state) {
    case 0xEB340761:case 0x9C0C7E5B:case 0x15914F6D:
    case 0x4ABEBF9D:case 0x164E8B07:case 0x8ED159F1:
    case 0x01EBBDBD:case 0x2DCECD55:case 0x815B8344:
    case 0xB3BC3663:case 0x4266B3CA:case 0xEF25D4BD:
    case 0x3AB7188E:case 0xCD285377:return true;
    default:return false;
    }
}
inline bool solverCaller(uintptr_t caller) {
    return caller==0x36a025d || caller==0x36a49b3;
}
}
