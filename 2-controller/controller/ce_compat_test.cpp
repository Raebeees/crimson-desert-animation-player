#include "ce_variant_compat.h"
#include "hook_batch204.h"
#include "deferred_compat.h"
#include <cassert>
#include <iostream>
unsigned enables=0,disables=0,applies=0,fail=0;
MH_STATUS WINAPI MH_QueueEnableHook(LPVOID){++enables;return enables==fail?MH_ERROR_NOT_CREATED:MH_OK;}
MH_STATUS WINAPI MH_QueueDisableHook(LPVOID){++disables;return MH_OK;}
MH_STATUS WINAPI MH_ApplyQueued(){++applies;return MH_OK;}
int main(){
 using namespace boots204;
 for(auto state:{0x15914f6du,0x36d65fb4u,0xeb340761u}){
  auto v=boots::ringSelection(true,true,state,boots::variant(true,state,0));
  assert(preserveBootsVariant204(true,true,state,v));
  assert(!preserveBootsVariant204(false,true,state,v));assert(!preserveBootsVariant204(true,false,state,v));
  assert(retainedBootsVariant204(v)==v);
  for(auto native:{0u,0xedbdef3du,0x4cb714a1u,123u})assert(!preserveBootsVariant204(true,true,state,native));
 }
 for(auto state:{0x269521ceu,0x61b7853eu,0xe906b0eau,0xe4ef0545u,0x3ac4a94eu,0x01ebbdbdu})
  for(auto incoming:{0u,0xedbdef3du,0x4cb714a1u,0x54424340u,0x54424c44u,0x54424854u,0x54425247u})
   assert(!preserveBootsVariant204(true,true,state,incoming));
 assert(!preserveBootsVariant204(true,true,0x12345678,0x54424340));
 assert(!preserveBootsVariant204(true,true,0x15914f6d,0x54424c44));
 void* targets[4];for(unsigned i=0;i<4;++i)targets[i]=reinterpret_cast<void*>(static_cast<uintptr_t>(i+1));
 auto ok=boots_enable_batch(targets);assert(ok.activation==MH_OK && enables==4 && applies==1);
 enables=disables=applies=0;fail=4;
 auto bad=boots_enable_batch(targets);assert(bad.activation==MH_ERROR_NOT_CREATED && bad.rollback==MH_OK && enables==4 && disables==4 && applies==1);
 DeferredCompat204 arm;
 for(unsigned t=0;t<5000;t+=16)assert(!arm.update(false,1,t));
 for(unsigned t=5000;t<6900;t+=16)assert(!arm.update(true,1,t));
 assert(!arm.update(true,2,6900)); // actor change resets
 for(unsigned t=6916;t<8800;t+=16)assert(!arm.update(true,2,t));
 assert(!arm.update(true,2,9000)); // missed polls reset
 for(unsigned t=9016;t<11000;t+=16)assert(!arm.update(true,2,t));
 assert(arm.update(true,2,11000));
 for(unsigned t=11016;t<20000;t+=16)assert(!arm.update(true,2,t));
 assert(!arm.update(false,0,20000));assert(!arm.update(true,3,25000));
 DeferredCompat204 interrupted;
 assert(!interrupted.update(true,1,0));assert(!interrupted.update(false,1,16));
 assert(!interrupted.update(true,1,2000));
 std::cout<<"CE compatibility: private-only scope, native pass-through, zero reason and four-hook rollback passed\n";
}
