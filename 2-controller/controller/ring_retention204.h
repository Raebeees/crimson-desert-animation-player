#pragma once
#include "effect_cleanup204.h"
namespace boots204 {
template<class Reader> bool inspectRetention204(Reader& reader,std::uintptr_t base,std::uintptr_t cc,std::uintptr_t effect,std::uint32_t index,std::uintptr_t caller) {
 if(base>0x7ffff0000000ULL||caller!=base+0x563a38||effect<0x10000||effect>=0x800000000000ULL-0xa0||effect%8)return false;
 Snapshot<Reader> s(reader);IdentityEquipment identity;LowerState state;
 std::uintptr_t components=0,actual=0,owner=0,data=0;std::uint32_t count=0,capacity=0;
 if(!readIdentityEquipment(s,base,cc,identity)||identity.boots!=EquipmentPresence::present||!readLowerState(s,identity.control,state)||!state.active||
 !(boots::ringState(state.id)||task008::dodgeRecovery(state.id)||state.id==task008::dispatcher)||
 !s.pointer(identity.actor+0x68,components)||!s.pointer(components+0x60,actual)||actual!=effect||!s.pointer(effect+8,owner)||owner!=identity.actor||
 !s.read(effect+0x98,count)||!s.read(effect+0x9c,capacity)||index>=count||count>capacity||capacity>4096||!s.pointer(effect+0x90,data))return false;
 if(data>=0x800000000000ULL-static_cast<std::uintptr_t>(count)*0x250)return false;
 const auto record=data+static_cast<std::uintptr_t>(index)*0x250;std::uint32_t key=0,group=0;std::uint8_t refs0=0;std::int8_t refs1=0;
 if(!s.read(record,key)||!s.read(record+4,group)||!boots::privateRing(key,group)||!s.read(record+0x1f0,refs0)||!s.read(record+0x1f1,refs1)||refs0||refs1>0)return false;
 bool live=false;
 std::uint32_t children=0,childCap=0;std::uintptr_t array=0;
 if(!s.read(record+0x200,children)||!s.read(record+0x204,childCap)||children>childCap||childCap>64)return false;
 if(children&&!s.pointer(record+0x1f8,array))return false;
 for(unsigned i=0;i<children;++i){std::uintptr_t ref=0,object=0;std::uint8_t flag=0;
  if(!s.read(array+i*24ULL+16,ref))return false;if(!ref)continue;
  if(ref<0x10000||ref>=0x800000000000ULL-16||ref%8||!s.read(ref+8,object))return false;if(!object)continue;
  if(object<0x10000||object>=0x800000000000ULL-0x16||!s.read(object+0x15,flag))return false;if(flag==0)live=true;
 }
 // Current second collection: stride32 groups containing pointer/count/capacity,
 // then contiguous object pointers. Its live predicate is flag!=1, not flag==0.
 std::uint32_t groups=0,groupCap=0;std::uintptr_t groupArray=0;
 if(!s.read(record+0x210,groups)||!s.read(record+0x214,groupCap)||groups>groupCap||groupCap>16)return false;
 if(groups&&!s.pointer(record+0x208,groupArray))return false;
 for(unsigned i=0;i<groups;++i){const auto g=groupArray+i*32ULL;std::uintptr_t objects=0;std::uint32_t size=0,cap=0;
  if(!s.read(g+8,size)||!s.read(g+12,cap)||size>cap||cap>64)return false;if(size&&!s.pointer(g,objects))return false;
  for(unsigned j=0;j<size;++j){std::uintptr_t object=0;std::uint8_t flag=0;if(!s.read(objects+j*8ULL,object))return false;if(!object)continue;
   if(object<0x10000||object>=0x800000000000ULL-0x16||!s.read(object+0x15,flag))return false;if(flag!=1)live=true;}
 }
 return live&&s.stable();
}
using RetireOriginal204=void(*)(std::uintptr_t,std::uint32_t,bool);
template<class Reader,class Runtime,class Errors>
void forwardRetirement204(RetireOriginal204 original,std::uintptr_t effect,std::uint32_t index,bool immediate,std::uintptr_t caller,
 Reader& reader,Runtime& runtime,Errors& errors,std::uintptr_t base,std::uintptr_t cc) {
 const auto incoming=errors.get();bool retain=false;
 try{retain=runtime.assetsReady()&&runtime.slotsOwned()&&inspectRetention204(reader,base,cc,effect,index,caller)&&runtime.assetsReady()&&runtime.slotsOwned();}catch(...){retain=false;}
 errors.set(incoming);if(!retain)original(effect,index,immediate);
}
} // namespace boots204
