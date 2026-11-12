#pragma once
#include <Windows.h>
#include <atomic>
#include <cstdint>
#include "launch_states.h"
namespace boots204 {
inline std::atomic<std::uintptr_t> launchActor204{0};
inline std::atomic<std::uint64_t> launchDeadline204{0};
inline void clearLaunch204(){launchDeadline204.store(0,std::memory_order_release);launchActor204.store(0,std::memory_order_release);}
inline void requestLaunch204(std::uintptr_t actor,std::uint64_t now){
 launchDeadline204.store(0,std::memory_order_release);
 launchActor204.store(actor,std::memory_order_release);
 launchDeadline204.store(now+750,std::memory_order_release);
}
inline bool requestedLaunch204(std::uintptr_t actor,std::uint32_t state,std::uint64_t now){
 const auto end=launchDeadline204.load(std::memory_order_acquire);
 return actor && launchSource204(state) && end>=now && end-now<=750 &&
   launchActor204.load(std::memory_order_acquire)==actor && end==launchDeadline204.load(std::memory_order_acquire);
}
}
