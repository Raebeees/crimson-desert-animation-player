#include "idle_shared_dispatch.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
using namespace idle_shared;
struct Errors {unsigned value=789;unsigned get(){return value;}void set(unsigned x){value=x;}};
struct Memory {
 std::uintptr_t descriptor=0x11000;Descriptor d{{234,76,1292328098,0,0}};
 bool read(std::uintptr_t a,void* out,std::size_t n){
  if(a==0x10000 && n==8){std::memcpy(out,&descriptor,8);return true;}
  if(a>=descriptor && a+n<=descriptor+20){std::memcpy(out,reinterpret_cast<char*>(&d)+a-descriptor,n);return true;}
  return false;
 }
};
int calls=0;int behavior=0;Registry* current=nullptr;
bool callback(const Invocation* in,std::uint8_t* low){
 ++calls;assert(in->abi==4 && in->args[1]==0x10000 && in->nativeResult==0xabc01);
 if(behavior==1)throw std::runtime_error("fixture");
 if(behavior==2){*low=7;return true;}
 if(behavior==3){current->disable();*low=0;return true;}
 *low=0;return behavior!=4;
}
int main(){
 Memory m;Errors e;boots204::ConditionArguments a{0x12000,0x10000,3,4,5,6,7};
 Registration r{};r.size=sizeof(r);r.abi=4;r.count=1;r.bindings[0]=m.d;r.evaluate=callback;
 auto pin=[](Evaluate){return true;};
 Registry none;assert(none.forward(0xabc01,a,m,e,1,2,true)==0xabc01 && calls==0);
 Registry x;assert(!x.install(r,[](Evaluate){return false;}));assert(x.install(r,pin));assert(!x.install(r,pin));
 assert(x.forward(0xabc01,a,m,e,1,2,true)==0xabc00 && calls==1 && e.value==789);
 assert(x.forward(0xabc07,a,m,e,1,2,true)==0xabc07 && calls==1);
 assert(x.forward(0xabc01,a,m,e,1,2,false)==0xabc01 && calls==1);
 for(unsigned i=0;i<5;++i){++m.d.words[i];assert(x.forward(0xabc01,a,m,e,1,2,true)==0xabc01 && calls==1);--m.d.words[i];}
 for(int mode=1;mode<=2;++mode){behavior=mode;assert(x.forward(0xabc01,a,m,e,1,2,true)==0xabc01 && e.value==789);}
 behavior=4;assert(x.forward(0xabc01,a,m,e,1,2,true)==0xabc01);
 behavior=3;current=&x;assert(x.forward(0xabc01,a,m,e,1,2,true)==0xabc01);assert(!x.install(r,pin));
 auto option=r;option.bindings[0].words[1]=84;option.bindings[0].words[2]=3562184783u;assert(valid(option));
 auto bad=r;bad.bindings[0].words[2]=4024466150u;Registry y;assert(!y.install(bad,pin)); // flight's real key
 bad=r;bad.count=2;bad.bindings[1]=bad.bindings[0];assert(!y.install(bad,pin));
 bad=r;bad.abi=1;assert(!y.install(bad,pin));bad=r;bad.count=19;assert(!y.install(bad,pin));
 std::cout<<"Shared bridge: private isolation, flight-key exclusion, lifetime, errors and disable gates passed\n";
}
