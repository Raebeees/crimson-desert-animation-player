#pragma once
#include "layer_clock.h"
#include "transport.h"
namespace lowcortisol_engine {
// All calls serialized by the host's nonblocking try-lock. No native writes.
// A condition returning active is only permission to transition, never an ack.
class Lifecycle {
 enum class Phase {idle,armed,active,stopping,fault};
 Phase phase=Phase::idle;
 Pair last{};std::uint64_t generation=0,request=0,ownerGeneration=0;
 unsigned issued=0,seen=0;double stamp=0,changed=0,clock=0;
 bool finished=false;std::uint32_t selected=0;
public:
 bool start(std::uint64_t token,std::uint64_t actor,std::uint32_t action,double now){
  if(!dance_v2::known(action)||dance_v2::canonical(action)!=action||!token||actor!=generation||phase!=Phase::idle||!last.verified||!last.eligible||now-stamp>.1)return false;
  selected=action;request=token;ownerGeneration=generation;issued=seen=0;clock=0;finished=false;phase=Phase::armed;changed=now;return true;
 }
 bool stop(std::uint64_t token,double now){
  if(token!=request)return false;
  if(phase==Phase::fault)return false;
  if(phase==Phase::idle||phase==Phase::stopping)return true;
  phase=Phase::stopping;changed=now;
  // An unissued request cannot commit later: selection and cancellation share
  // the serialization domain. Issued starts require observed ownership/exit.
  if(!issued)phase=Phase::idle;
  return true;
 }
 void sample(const Pair& p,double now){
  if(!std::isfinite(now))return;
  const bool identityChanged=!p.verified||!last.verified||p.actor!=last.actor||p.identity!=last.identity;
  if(identityChanged){++generation;if(phase!=Phase::idle)phase=Phase::fault;}
  last=p;stamp=now;
  if(phase==Phase::idle||phase==Phase::fault)return;
  const auto owned=p.ownedMask();seen|=owned;
  if(!p.eligible&&phase!=Phase::stopping){phase=Phase::stopping;changed=now;}
  if(phase==Phase::armed){
   double pos=0;
   if(p.clock(pos,selected)){phase=Phase::active;clock=pos;}
   else if(now-changed>2.){phase=Phase::stopping;changed=now;}
  }else if(phase==Phase::active){
   double pos=0;
   if(p.clock(pos,selected)){
    if(pos+.04<clock){phase=Phase::stopping;changed=now;}
    else clock=pos;
   }else if(owned&&owned!=3&&clock>=dance_v2::duration(selected)-.100&&
     (dance_v2::canonical(p.upper.action)==selected||dance_v2::canonical(p.lower.action)==selected)){
    const double remaining=(std::max)(position(p.upper),position(p.lower));
    if(remaining>=dance_v2::duration(selected)-.100&&remaining<=dance_v2::duration(selected)+.100)clock=(std::min)(remaining,dance_v2::duration(selected));
    else{phase=Phase::stopping;changed=now;}
   }else if(!owned&&clock>=0.){
    // Native end seen; the final clock must have reached the audio boundary.
    // An early exit is cancellation, never a successful natural completion.
    finished=clock>=dance_v2::duration(selected)-.100;
    if(finished)clock=dance_v2::duration(selected);
    phase=Phase::stopping;changed=now;
   }else{phase=Phase::stopping;changed=now;}
  }
  if(phase==Phase::stopping){
   if(!owned&&(seen&issued)==issued)phase=Phase::idle;
   else if(now-changed>2.)phase=Phase::fault;
  }
 }
 bool condition(unsigned layer,unsigned offset,bool cancel,double now){
  if(layer>1||!last.verified||now-stamp>.1)return false;
  const unsigned mask=1u<<layer;
  if(cancel)return (last.ownedMask()&mask)&&(phase==Phase::stopping||phase==Phase::fault||!last.eligible);
  if(phase!=Phase::armed||ownerGeneration!=generation||!last.eligible||offset!=dance_v2::route(selected))return false;
  const auto& l=layer?last.lower:last.upper;
  if(!l.active)return false;
  issued|=mask;return true;
 }
 lowcortisol::Observation observation(double now)const{
  lowcortisol::Observation o;o.context.actor=generation;
  o.context.ready=last.verified&&now-stamp<=.1&&phase!=Phase::fault;
  o.context.focused=o.context.grounded=last.eligible;
  o.context.interrupted=phase==Phase::stopping&&!finished;
  o.token=request;o.clipSeconds=clock;
  o.started=phase==Phase::active;
  o.stopped=phase==Phase::idle&&request!=0;
  o.failed=phase==Phase::fault;
  return o;
 }
};
}
