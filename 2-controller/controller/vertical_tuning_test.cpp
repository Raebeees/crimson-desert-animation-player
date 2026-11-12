#include "vertical_control.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
using namespace boots;
struct Legacy {
 uintptr_t actor=0; bool armed=false; float speed=0;
 VerticalOutput update(VerticalInput in) {
  if(!in.actor || !in.eligible || !std::isfinite(in.dt) || in.dt<=0 || in.dt>0.1f){actor=0;armed=false;speed=0;return {};}
  if(actor!=in.actor){actor=in.actor;armed=false;speed=0;}
  if(!armed){armed=!in.up&&!in.down&&!in.cancel;return {!in.cancel,0};}
  if(in.cancel){actor=0;armed=false;speed=0;return {};}
  float target=in.up!=in.down?(in.up?3.f:-3.f):0.f,step=8.f*in.dt;
  if(speed<target)speed=std::fmin(speed+step,target);else speed=std::fmax(speed-step,target);
  return {true,speed};
 }
};
int main(){
 assert(parseVerticalRate(L"8.0")==8 && parseVerticalRate(L" 4.5 ")==4.5f);
 for(auto s:{L"",L"nan",L"inf",L"0",L"30.1",L"4oops",L"1e999"})assert(parseVerticalRate(s)==8);
 assert(parseVerticalRate(L"0.5")==0.5f && parseVerticalRate(L"30")==30.f);
 VerticalControl a; Legacy b; std::uint32_t rng=17;
 for(int i=0;i<200000;i++){
  rng=rng*1664525u+1013904223u;
  VerticalInput in{(rng%71==0)?0u:((rng%89==0)?2u:1u),rng%53!=0,(rng&16)!=0,(rng&32)!=0,(rng%79)==0,(rng%97==0)?0.f:float((rng%100)+1)*0.001f};
  auto x=a.update(in),y=b.update(in);assert(x.overrideVertical==y.overrideVertical && x.speed==y.speed);
 }
 verticalTuning={4,10}; VerticalControl c;
 VerticalInput in{1,true,false,false,false,0.1f}; c.update(in);
 in.up=true; auto r=c.update(in); assert(std::fabs(r.speed-0.4f)<0.00001f);
 in.up=false;r=c.update(in); assert(r.speed==0);
 in.up=true;c.update(in);in.up=false;in.down=true;r=c.update(in);
 assert(std::fabs(r.speed+0.24f)<0.00001f);
 in.down=false;in.up=true;in.dt=0.01f;r=c.update(in);assert(std::fabs(r.speed+0.14f)<0.00001f);
 in.cancel=true;r=c.update(in);assert(!r.overrideVertical);
 verticalTuning={};std::cout<<"200000 default differential steps and tuning checks passed\n";
}
