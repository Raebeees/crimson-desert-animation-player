#pragma once
#include "lifecycle.h"
#include "controller/input_identity204.h"
#include "controller/production_condition_host204.h"
#include <mutex>
namespace lowcortisol_engine {
inline double now(){return double(GetTickCount64())/1000.;}
inline Pair inspect(boots204::ProcessReader204& reader,std::uintptr_t base,std::uintptr_t cc,bool ready){
 Pair p;boots204::Snapshot<boots204::ProcessReader204> s(reader);boots204::IdentityEquipment identity;
 std::uintptr_t components=0,input=0;bool suppressed=true;
 if(!boots204::readIdentityEquipment(s,base,cc,identity)||!s.read(identity.actor+8,p.identity)||!p.identity||
    !readLayer(s,identity.control,0,p.upper)||!readLayer(s,identity.control,1,p.lower)||
    !s.pointer(identity.actor+0x68,components)||!boots204::inputSuppressed204(s,components,suppressed)||
    !boots204::readInputObject204(s,base,input))return p;
 bool moving=false;
 constexpr std::uint32_t inputs[]={0xf623030b,0xe644ce16,0x9d910fc4,0x6bef3613};
 for(auto hash:inputs){
  const auto state=boots204::readAction204(s,input,hash);
  if(state==boots204::Action204::unavailable)return p;
  moving=moving||state==boots204::Action204::held;
 }
 if(!s.stable())return p;
 p.actor=identity.actor;p.verified=true;
 p.eligible=ready&&!suppressed&&!moving&&boots204::foregroundProcess204()&&
  p.lower.active&&(chartEntry(1,p.lower.action)||part(p.lower.action)>=0)&&
  p.upper.active&&(chartEntry(0,p.upper.action)||part(p.upper.action)>=0);
 return p;
}
class Adapter {
 Lifecycle life;std::atomic_flag guard=ATOMIC_FLAG_INIT;
 std::atomic<std::uint64_t> cancel{0};
 std::mutex outputMutex;lowcortisol::Observation published{};double publishedAt=0;
 std::uintptr_t base=0,cc=0;std::atomic<bool> ready{false};
 double lastVerified=-1;
 boots204::ProcessReader204 reader;
 void applyCancel(double t){const auto token=cancel.load();if(token)life.stop(token,t);}
public:
 bool configure(std::uintptr_t game,std::uintptr_t creator){if(base||cc)return false;base=game;cc=creator;return true;}
 bool request(std::uint64_t token,std::uint64_t actor,std::uint32_t action){
  if(guard.test_and_set(std::memory_order_acquire))return false;
  bool ok=life.start(token,actor,action,now());if(ok)cancel.store(0);
  guard.clear(std::memory_order_release);return ok;
 }
 bool stop(std::uint64_t token){if(!token)return false;cancel.store(token);return true;}
 bool observe(lowcortisol::Observation* output){
  if(!output||output->size!=sizeof(*output)||output->version!=2)return false;
  std::lock_guard<std::mutex> lock(outputMutex);*output=published;
  if(now()-publishedAt>.1)output->context.ready=false;
  return true;
 }
 void service(bool hostReady){
  ready.store(hostReady);const auto sample=inspect(reader,base,cc,hostReady);const double t=now();
  if(guard.test_and_set(std::memory_order_acquire))return;
  applyCancel(t);
  if(sample.verified){lastVerified=t;life.sample(sample,t);}
  else if(t-lastVerified>.100)life.sample(sample,t);
  auto output=life.observation(t);
  guard.clear(std::memory_order_release);
  std::lock_guard<std::mutex> lock(outputMutex);published=output;publishedAt=t;
 }
 bool evaluate(const idle_shared::Invocation* in,std::uint8_t* value){
  if(!in||!value||in->size!=sizeof(*in)||in->abi!=idle_shared::version||in->gameBase!=base||in->characterCreatorBase!=cc)return false;
  const auto& w=in->descriptor.words;
  unsigned layer=2;bool isCancel=false;
  if(w[0]!=234||w[3]||w[4])return false;
  if(w[1]>=76&&w[1]<=84)layer=1;
  else if(w[1]>=125&&w[1]<=133)layer=0;
  else return false;
  const auto offset=w[1]-(layer?76u:125u);
  const std::uint32_t hashes[]={1292328098u,1247619153u,784376820u,3359083252u,3752803957u,3138324348u,1655964569u,1560144286u,3562184783u};
  if(w[2]!=hashes[offset])return false;isCancel=offset==2;
  auto sample=inspect(reader,base,cc,ready.load());
  std::uintptr_t actor=0;
  // Match the actual condition actor, not merely the current local actor.
  if(!sample.verified||in->args[0]<0x10000||in->args[0]>0x7fffffffffffULL-16||
     !reader.read(in->args[0]+8,&actor,sizeof(actor))||actor!=sample.actor)return false;
  if(guard.test_and_set(std::memory_order_acquire))return false;
  const double t=now();applyCancel(t);lastVerified=t;life.sample(sample,t);
  bool active=life.condition(layer,offset,isCancel,t);
  // A concurrently submitted stop may revoke an armed permission. The lifecycle
  // still records issued permission and conservatively requires ownership/exit.
  if(!isCancel&&cancel.load())active=false;
  guard.clear(std::memory_order_release);
  *value=active?0:1;return true;
 }
};
inline Adapter adapter;
inline bool request(std::uint64_t token,std::uint64_t actor,std::uint32_t action){return adapter.request(token,actor,action);}
inline bool stop(std::uint64_t token){return adapter.stop(token);}
inline bool observe(lowcortisol::Observation* output){return adapter.observe(output);}
inline bool evaluate(const idle_shared::Invocation* in,std::uint8_t* value){return adapter.evaluate(in,value);}
inline idle_shared::Registration registration(){
 idle_shared::Registration r{};r.size=sizeof(r);r.abi=idle_shared::version;r.count=18;r.evaluate=evaluate;
 const std::uint32_t hashes[]={1292328098u,1247619153u,784376820u,3359083252u,3752803957u,3138324348u,1655964569u,1560144286u,3562184783u};
 for(unsigned layer=0;layer<2;++layer)for(unsigned i=0;i<9;++i)r.bindings[layer*9+i]={{234,(layer?76u:125u)+i,hashes[i],0,0}};
 return r;
}
}
