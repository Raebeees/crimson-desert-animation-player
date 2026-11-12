#pragma once
#include "idle_shared_api.h"
#include "negative_filter204.h"
#include "condition_calls204.h"
#include <atomic>
#include <mutex>
#include <cstring>
namespace idle_shared {
// Hashes come from the private lower-chart descriptor generation, not stock inputs.
inline bool privateHash(std::uint32_t h) {
 return h==1292328098u || h==1247619153u || h==784376820u || h==3359083252u || h==3752803957u || h==3138324348u || h==1655964569u || h==1560144286u || h==3562184783u;
}
inline bool valid(const Registration& r) {
 if(r.size!=sizeof(Registration)||r.abi!=version||r.reserved||!r.evaluate||!r.count||r.count>maxBindings)return false;
 for(std::uint32_t i=0;i<r.count;++i){
  auto& w=r.bindings[i].words;
  if(w[0]!=234 || w[1]>65535 || !privateHash(w[2]) || w[3] || w[4])return false;
  for(std::uint32_t j=0;j<i;++j)if(!std::memcmp(w,r.bindings[j].words,sizeof(w)))return false;
 }
 return true;
}
class Registry {
 Registration registration{};
 std::mutex publish;
 std::atomic<bool> enabled{false};
 bool used=false;
public:
 // Pin must retain the client module for process lifetime BEFORE publication.
 template<class Pin> bool install(const Registration& r,Pin pin) {
  if(!valid(r))return false;
  std::lock_guard<std::mutex> lock(publish);
  if(used||!pin(r.evaluate))return false;
  registration=r;used=true;enabled.store(true,std::memory_order_release);return true;
 }
 void disable() noexcept {enabled.store(false,std::memory_order_release);}
 bool active()const noexcept{return enabled.load(std::memory_order_acquire);}
 template<class Reader,class Errors> std::uintptr_t forward(std::uintptr_t native,
  const boots204::ConditionArguments& a,Reader& reader,Errors& errors,
  std::uintptr_t game,std::uintptr_t cc,bool hostReady) noexcept {
  if(!hostReady || !active() || (native&255)>1)return native;
  const auto saved=errors.get();auto answer=native;
  try {
   std::array<std::uint32_t,5> words{};
   if(boots204::descriptorPrefix204(reader,a.b,234,words)) {
    bool exact=false;
    for(std::uint32_t i=0;i<registration.count;++i)
     if(!std::memcmp(words.data(),registration.bindings[i].words,sizeof(registration.bindings[i].words)))exact=true;
    if(exact && active()) {
     Invocation in{};in.size=sizeof(in);in.abi=version;
     const std::uintptr_t args[7]={a.a,a.b,a.c,a.d,a.e,a.f,a.g};
     std::memcpy(in.args,args,sizeof(args));std::memcpy(in.descriptor.words,words.data(),sizeof(in.descriptor.words));
     in.nativeResult=native;in.gameBase=game;in.characterCreatorBase=cc;
     std::uint8_t low=255;
     if(registration.evaluate(&in,&low) && low<=1 && active())answer=(native&~std::uintptr_t(255))|low;
    }
   }
  }catch(...){answer=native;}
  errors.set(saved);return answer;
 }
};
inline Registry registry;
}
