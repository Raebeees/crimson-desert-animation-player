#pragma once
#include "runtime_condition204.h"
#include "idle_shared_dispatch.h"
#include <exception>
namespace boots204 {
// Reader must support concurrent read-only calls, with no shared observation
// journal. Runtime creates a per-invocation journal. Errors is per invocation.
// Context, runtime, reader and callback code remain alive after publication.
// One specialization per adapter; bind exactly once BEFORE activating slots.
template<class Runtime,class Reader,class Errors,class Tag> struct ConditionCallbacks204 {
 struct Context {Runtime& runtime;Reader& reader;Word base,cc;Foreground204 foreground=nullptr;};
 inline static std::atomic<Context*> bound{nullptr};
 static bool bind(Context& context) noexcept {
  Context* empty=nullptr;
  return bound.compare_exchange_strong(empty,&context,std::memory_order_release,std::memory_order_relaxed);
 }
 static Context& context() noexcept {
  auto* p=bound.load(std::memory_order_acquire);
  if(!p)std::terminate(); // Setup contract violation, never invent a game result.
  return *p;
 }
 static Word key(Word a,Word b,Word c,Word d,std::uint8_t e,Word f,Word g) {
  auto& ctx=context();Errors errors;
  const ConditionArguments args{a,b,c,d,e,f,g};
  const auto result=ctx.runtime.key(args,ctx.reader,errors,ctx.base,ctx.cc,ctx.foreground);
  // Flight/CE execute first, exactly once. No second slot owner or new hook.
  if(!idle_shared::registry.active())return result;
  const auto saved=errors.get();
  const bool ready=ctx.runtime.assetsReady() && ctx.runtime.slotsOwned();
  errors.set(saved);
  return idle_shared::registry.forward(result,args,ctx.reader,errors,ctx.base,ctx.cc,ready);
 }
 static Word characterClass(Word a,Word b,Word c,Word d,std::uint8_t e,Word f,Word g) {
  auto& ctx=context();Errors errors;
  return ctx.runtime.characterClass({a,b,c,d,e,f,g},ctx.reader,errors,ctx.base,ctx.cc);
 }
 static Word skill(Word a,Word b,Word c,Word d,std::uint8_t e,Word f,Word g) {
  auto& ctx=context();Errors errors;
  return ctx.runtime.skill({a,b,c,d,e,f,g},ctx.reader,errors,ctx.base,ctx.cc);
 }
 static std::array<Target,3> addresses() noexcept {
  return {reinterpret_cast<Target>(&key),reinterpret_cast<Target>(&characterClass),reinterpret_cast<Target>(&skill)};
 }
};
} // namespace boots204
