#include "lifecycle.h"
#include <cassert>
using namespace lowcortisol_engine;
Pair normal(){Pair p;p.actor=0x10000;p.identity=7;p.verified=p.eligible=true;p.upper={0x25dc7fb9,true,0,0};p.lower={idleAction,true,0,0};return p;}
int main(){Lifecycle life;auto p=normal();double t=0;life.sample(p,t);auto actor=life.observation(t).context.actor;assert(!life.start(99,actor,dance_v2::vervePartTwo,.001));
for(unsigned i=0;i<8;++i){auto id=dance_v2::ids[i];t+=.01;assert(life.start(i+1,actor,id,t));for(unsigned off=0;off<9;++off)if(off!=2)assert(life.condition(0,off,false,t+.001)==(off==dance_v2::route(id)));assert(life.condition(1,dance_v2::route(id),false,t+.001));p.upper=p.lower={id,true,0,.01f};t+=.02;life.sample(p,t);assert(life.observation(t).started);
 if(i==5){p.upper.current=p.lower.current=33.49f;t+=33.48;life.sample(p,t);p.upper={dance_v2::vervePartTwo,true,0,.01f};t+=.01;life.sample(p,t);assert(life.observation(t).started);p.lower=p.upper;p.upper.current=p.lower.current=.02f;t+=.01;life.sample(p,t);assert(life.observation(t).started);p.upper.current=p.lower.current=33.46f;t+=33.44;}
 else {p.upper.current=p.lower.current=static_cast<float>(dance_v2::duration(id)-.04);t+=dance_v2::duration(id)-.05;}
 life.sample(p,t);p=normal();t+=.02;life.sample(p,t);auto o=life.observation(t);assert(o.stopped&&!o.failed&&!o.context.interrupted);assert(o.clipSeconds==dance_v2::duration(id));}}
