#include "production_condition_host204.h"
#include "condition_callbacks204.h"
#include "input_fixture.h"
#include <functional>
#include <type_traits>
namespace integrated {
bool focused=true;bool foreground(){return focused;}
using namespace boots204;
struct Errors{DWORD get(){return GetLastError();}void set(DWORD v){SetLastError(v);}};
struct Tag{};using Host=ConditionHost204<memoryfixture::Memory,Tag>;using Callbacks=Host::Callbacks;
static_assert(std::is_same_v<decltype(&Callbacks::characterClass),ConditionOriginal>);
unsigned calls=0,checks=0;Word result=0xfeed01;std::function<void()> change;
ConditionArguments received{};DWORD callerError=0;
Word native(Word a,Word b,Word c,Word d,std::uint8_t e,Word f,Word g){++calls;received={a,b,c,d,e,f,g};callerError=GetLastError();if(change)change();SetLastError(912);return result;}
void check(bool b,const char* label){++checks;if(!b)throw std::runtime_error(label);}
Word invoke(Target target){calls=0;SetLastError(713);return reinterpret_cast<ConditionOriginal>(target)(flight_test::ctx,flight_test::holder,3,4,0xa5,6,7);}
void sample(memoryfixture::Memory& m,unsigned index){m=flight_test::fixture();m.ptr(memoryfixture::control+0x88,flight_test::stateHolder);const auto& s=classStateSamples204[index];for(unsigned i=0;i<96;++i)m.put<std::uint8_t>(flight_test::desc-32+i,s.bytes[i]);m.u32(flight_test::node+0x18,s.states[0]);}
void packedTests(){
 const ConditionArguments args{flight_test::ctx,flight_test::holder,3,4,0,6,7};
 auto relocate=[](memoryfixture::Memory& m,unsigned offset){const auto copy=m.bytes; const Word target=flight_test::desc+0x800+offset;for(int i=-32;i<64;++i){auto it=copy.find(flight_test::desc+i);if(it!=copy.end())m.bytes[target+i]=it->second;}m.ptr(flight_test::holder,target);};
 for(unsigned offset=0;offset<8;++offset){
  for(const auto& words:skillSamples204){auto m=flight_test::fixture();for(unsigned i=0;i<5;++i)m.u32(flight_test::desc+4*i,words[i]);relocate(m,offset);check(inspectExistingSkill204(m,memoryfixture::base,memoryfixture::cc,args).owned,"packed skill recognized");m.u32(memoryfixture::itemRecord+8,220004);check(!inspectExistingSkill204(m,memoryfixture::base,memoryfixture::cc,args).owned,"boots off skill untouched");}
  for(unsigned i=0;i<classStateSamples204.size();++i){memoryfixture::Memory m;sample(m,i);relocate(m,offset);check(inspectClassState204(m,memoryfixture::base,memoryfixture::cc,args).owned,"packed class recognized");m.ptr(flight_test::ctx+8,memoryfixture::actor+8);check(!inspectClassState204(m,memoryfixture::base,memoryfixture::cc,args).owned,"foreign class untouched");}
  auto m=identity_test::fixture();m.u32(flight_test::node+0x18,keyFlightStates204[0]);for(unsigned i=0;i<5;++i)m.u32(flight_test::desc+4*i,glideEndSamples204[0][i]);relocate(m,offset);check(inspectGlideKey204(m,memoryfixture::base,memoryfixture::cc,args).scope.owned,"packed key recognized");m.u32(memoryfixture::itemRecord+8,220004);check(!inspectGlideKey204(m,memoryfixture::base,memoryfixture::cc,args).scope.owned,"boots off key untouched");
 }
}
void run(){packedTests();ProcessReader204 reader;Word scratch=123,copy=0;check(reader.read(reinterpret_cast<Word>(&scratch),&copy,sizeof(copy))&&copy==scratch,"production reader private stack");check(!reader.read(1,&copy,sizeof(copy)),"invalid read rejected");check(!reader.read(reinterpret_cast<Word>(&scratch),&copy,4097),"oversized read rejected");auto* page=static_cast<Target*>(VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));check(page!=nullptr,"private allocation");auto original=reinterpret_cast<Target>(&native);for(unsigned i=0;i<3;++i)page[i]=original;
 auto* backend=new WindowsSlots204({reinterpret_cast<Target>(page),reinterpret_cast<Target>(page+1),reinterpret_cast<Target>(page+2)});auto* m=new memoryfixture::Memory(flight_test::fixture());auto* host=new Host(*backend,*m,memoryfixture::base,memoryfixture::cc,&foreground);auto* runtime=&host->runtime;
 check(!runtime->assetsReady(),"production readiness starts false");check(host->activate({{original,original,original},Callbacks::addresses(),true}).status==Setup::active,"activate host");check(host->activate({}).status==Setup::reused,"host one activation");runtime->setAssetsReady(true);
 for(unsigned i=0;i<classStateSamples204.size();++i){sample(*m,i);const auto& s=classStateSamples204[i];const unsigned desired=s.bytes[43]==0x40&&s.bytes[42]==0x40?0:1;
  for(unsigned v=0;v<3;++v){result=0xfeed00|v;check(invoke(page[1])==(v==2?result:0xfeed00|desired)&&calls==1,"real class callback result");check(callerError==713&&GetLastError()==912,"native error preserved");}
  check(received.a==flight_test::ctx&&received.b==flight_test::holder&&received.c==3&&received.d==4&&received.e==0xa5&&received.f==6&&received.g==7,"ABI intact");
  result=0xfeed01;check(invoke(page[0])==result&&calls==1,"key remains native");
  runtime->setAssetsReady(false);check(invoke(page[1])==result&&calls==1,"asset loss");runtime->setAssetsReady(true);
 }
 sample(*m,3);m->u32(flight_test::node+0x18,0x9b76958cu);check(invoke(page[1])==result&&calls==1,"foreign consumer state excluded");
sample(*m,4);m->u32(flight_test::node+0x18,0x414b6e75u);check(invoke(page[1])==result&&calls==1,"foreign consumer state excluded");
sample(*m,4);m->u32(flight_test::node+0x18,0x9846765bu);check(invoke(page[1])==result&&calls==1,"foreign consumer state excluded");
sample(*m,8);m->u32(flight_test::node+0x18,0x9846765bu);check(invoke(page[1])==result&&calls==1,"foreign consumer state excluded");
 *m=identity_test::fixture();m->u32(flight_test::node+0x18,keyFlightStates204[0]);for(unsigned i=0;i<5;++i)m->u32(flight_test::desc+i*4,glideEndSamples204[0][i]);
 result=0xfeed00;check(invoke(page[0])==0xfeed01&&calls==1,"Space glide-end held for ascent");
 m->put<std::uint8_t>(identity_test::action+0x300+0x5e,1);result=0xfeed01;check(invoke(page[0])==0xfeed00&&calls==1,"R cancel through real callback");
 focused=false;check(invoke(page[0])==result&&calls==1,"background key unchanged");focused=true;
 m->put<std::uint8_t>(identity_test::suppression+0x179,1);check(invoke(page[0])==result&&calls==1,"menu key unchanged");m->put<std::uint8_t>(identity_test::suppression+0x179,0);
 result=0xfeed02;check(invoke(page[0])==result&&calls==1,"key native exceptional result");result=0xfeed01;
 change=[]{focused=false;};check(invoke(page[0])==result&&calls==1,"postnative focus recheck");change={};focused=true;
 *m=flight_test::fixture();check(invoke(page[2])==1&&calls==0,"private equipped exit");m->u32(memoryfixture::itemRecord+8,220004);check(invoke(page[2])==0&&calls==0,"private unequip exit");
 *m=flight_test::fixture();m->u32(flight_test::desc+4,15066);check(invoke(page[2])==0xfeed00&&calls==1,"native skill eligibility retained");
 for(auto id:{1997603u,1997604u,1997605u}){m->u32(flight_test::desc+4,id);check(invoke(page[2])==result&&calls==1,"stamina records untouched");}
 sample(*m,0);change=[m]{m->u32(memoryfixture::itemRecord+8,220004);};check(invoke(page[1])==result&&calls==1,"unequip inside native call");change={};
 sample(*m,0);const auto retainedClass=page[1],retainedSkill=page[2];auto restored=runtime->deactivate();check(restored.restored==3&&runtime->requiresProcessLifetime(),"restore all slots");check(invoke(retainedClass)==result&&calls==1,"retained class after rollback");check(invoke(retainedSkill)==result&&calls==1,"retained skill after rollback");check(invoke(page[1])==result&&calls==1,"restored native");
}
}
int main(){try{integrated::run();std::cout<<"all condition callback checks passed: "<<integrated::checks<<" assertions\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

