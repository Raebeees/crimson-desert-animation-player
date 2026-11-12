#pragma once
#include <cstdint>
#include <cstddef>
namespace idle_shared {
constexpr std::uint32_t version=4,maxBindings=18;
struct Descriptor {std::uint32_t words[5];};
struct Invocation {
 std::uint32_t size,abi;
 std::uintptr_t args[7];
 std::uintptr_t nativeResult,gameBase,characterCreatorBase;
 Descriptor descriptor;
};
// Synchronous, bounded, engine-thread callback. Never retain native pointers,
// acquire UI locks, play audio, or call back into native conditions here.
// Return false to pass through; true with resultLowByte 0/1 for the exact
// registered private descriptor only. Client validates actor/generation/state.
using Evaluate=bool(*)(const Invocation*,std::uint8_t* resultLowByte);
struct Registration {
 std::uint32_t size,abi,count,reserved;
 Descriptor bindings[maxBindings];
 Evaluate evaluate;
};
using Register=bool(*)(const Registration*);
using Disable=void(*)();
using QueryVersion=std::uint32_t(*)();
// Exports on the shared flight candidate:
// TidebreakerIdleBridgeVersion1, TidebreakerRegisterIdleBridge1,
// TidebreakerDisableIdleBridge1. Disabled by default. One registration/process.
}
