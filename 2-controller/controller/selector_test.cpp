#include "selector_adapter204.h"
#include "gate_fixture.h"
#include <functional>
using namespace boots204;
unsigned calls=0;std::function<void()> change;std::uint32_t native(std::uintptr_t,std::uintptr_t){++calls;if(change)change();return 123;}
struct Errors{unsigned get(){return 71;}void set(unsigned v){if(v!=71)throw 1;}};
struct Runtime{bool assets=true,slots=true;bool assetsReady(){return assets;}bool slotsOwned(){return slots;}};
int main(){unsigned n=0;auto check=[&](bool b){++n;if(!b)throw n;};auto m=flight_test::fixture();m.ptr(memoryfixture::control+0x88,flight_test::stateHolder);Runtime rt;Errors e;
auto invoke=[&](std::uintptr_t control=memoryfixture::control,std::uintptr_t holder=flight_test::stateHolder){calls=0;auto v=forwardSelector204(&native,control,holder,m,rt,e,memoryfixture::base,memoryfixture::cc);check(calls==1);return v;};
commaEnabled204.store(false);check(invoke()==123);commaEnabled204.store(true);check(invoke()==boots::ringVariant);check(selectorCounts204[0]==2 && selectorCounts204[1]==1);check(selectorCounts204[2]==1 && selectorCounts204[4]==1);check(selectorLastState204==0x54424631 && selectorLastNative204==123 && selectorLastResult204==boots::ringVariant);rt.assets=false;check(invoke()==123);rt.assets=true;rt.slots=false;check(invoke()==123);rt.slots=true;
check(invoke(memoryfixture::control+8)==123);check(invoke(memoryfixture::control,flight_test::holder)==123);
m.u32(memoryfixture::itemRecord+8,99);check(invoke()==123);m.u32(memoryfixture::itemRecord+8,1987201);
for(auto state:{0x12345678u,task008::dispatcher}){m.u32(flight_test::node+0x18,state);check(invoke()==123);}
m.u32(flight_test::node+0x18,0x54424631);change=[&]{m.u32(memoryfixture::itemRecord+8,99);};check(invoke()==123);change={};
m.u32(memoryfixture::itemRecord+8,1987201);m.put<std::uint8_t>(flight_test::node+0x111,0);check(invoke()==123);
auto rejectCase=[&](unsigned stage,const std::function<void()>& mutate,std::uintptr_t holder=flight_test::stateHolder){m=flight_test::fixture();m.ptr(memoryfixture::control+0x88,flight_test::stateHolder);rt.assets=rt.slots=true;mutate();const auto before=selectorReject204[stage].load();check(invoke(memoryfixture::control,holder)==123);check(selectorReject204[stage]==before+1);};
rejectCase(0,[&]{rt.assets=false;});
rejectCase(1,[&]{m.u32(memoryfixture::charRecord+8,99);});
rejectCase(2,[&]{m.u32(memoryfixture::itemRecord+8,99);});
rejectCase(3,[&]{m.bytes.erase(memoryfixture::control+0x88);});
rejectCase(4,[]{},flight_test::holder);
rejectCase(5,[&]{m.ptr(memoryfixture::control+0x88,flight_test::stateHolder+1);},flight_test::stateHolder+1);
rejectCase(6,[&]{m.ptr(flight_test::stateHolder,0);});
rejectCase(7,[&]{m.ptr(flight_test::stateHolder+0x10,0);});
rejectCase(8,[&]{m.put<std::uint8_t>(flight_test::node+0x111,0);});
rejectCase(9,[&]{m.bytes.erase(flight_test::node+0x18);});
rejectCase(10,[&]{m.change=memoryfixture::control+0x88;m.changeVisit=2;});
constexpr auto candidate=0x300000ULL,candidateNode=0x301000ULL;
auto prepare=[&]{m=flight_test::fixture();m.ptr(memoryfixture::control+0x88,flight_test::stateHolder);m.ptr(candidate,11);m.ptr(candidate+8,22);m.ptr(candidate+16,candidateNode);m.u32(candidateNode+24,0x54424631);m.put<std::uint8_t>(candidateNode+0x111,1);rt.assets=rt.slots=true;};
// R8: copied holder is authorized by the validated control's exact table record.
constexpr auto table=0x410000ULL,array=0x420000ULL,vt=0x430000ULL;
auto membershipFixture=[&]{prepare();m.ptr(memoryfixture::control,vt);m.ptr(vt+0x330,memoryfixture::base+0x948980);m.ptr(flight_test::stateHolder+0x118,table);m.u32(table+0x20,1);m.ptr(table+0x18,array);m.ptr(array,11);m.ptr(array+8,22);m.ptr(array+16,candidateNode);};
membershipFixture();check(invoke(memoryfixture::control,candidate)==boots::ringVariant);
// Candidate may precede the active animation state.
membershipFixture();m.u32(flight_test::node+24,0x26107d3c);check(invoke(memoryfixture::control,candidate)==boots::ringVariant);
auto rejectCopy=[&](const std::function<void()>& mutate){membershipFixture();mutate();check(invoke(memoryfixture::control,candidate)==123);};
rejectCopy([&]{m.u32(memoryfixture::itemRecord+8,99);});
rejectCopy([&]{m.ptr(memoryfixture::ownerLink,memoryfixture::actor+8);});
rejectCopy([&]{m.ptr(vt+0x330,memoryfixture::base+0x948981);});
rejectCopy([&]{rt.assets=false;});rejectCopy([&]{rt.slots=false;});
rejectCopy([&]{m.ptr(array,12);});rejectCopy([&]{m.ptr(array+8,23);});
rejectCopy([&]{m.ptr(array+16,flight_test::node);});
rejectCopy([&]{m.u32(candidateNode+24,0x12345678);});
rejectCopy([&]{m.put<std::uint8_t>(candidateNode+0x111,0);});
rejectCopy([&]{m.u32(table+0x20,65537);});rejectCopy([&]{m.u32(table+0x20,0);});
rejectCopy([&]{m.ptr(table+0x18,0xfffffffffffffff8ULL);});
rejectCopy([&]{m.bytes.erase(array+8);});
rejectCopy([&]{m.change=table+0x20;m.changeVisit=2;});
rejectCopy([&]{m.change=array+16;m.changeVisit=2;});
rejectCopy([&]{m.change=memoryfixture::itemRecord+8;m.changeVisit=2;});
membershipFixture();check(invoke(memoryfixture::control+8,candidate)==123);
// Shared state ID in upper table must not mask the exact lower-table tuple.
constexpr auto upperHolder=0x440000ULL,upperTable=0x450000ULL,upperArray=0x460000ULL;
membershipFixture();m.ptr(memoryfixture::control+0x88,upperHolder);m.ptr(upperHolder+0x118,upperTable);m.u32(upperTable+0x20,1);m.ptr(upperTable+0x18,upperArray);m.ptr(upperArray,1);m.ptr(upperArray+8,2);m.ptr(upperArray+16,flight_test::node);check(invoke(memoryfixture::control,candidate)==boots::ringVariant);
// Missing optional upper lookup can fall through; malformed data cannot.
m.ptr(upperHolder+0x118,0);check(invoke(memoryfixture::control,candidate)==boots::ringVariant);
m.ptr(upperHolder+0x118,1);check(invoke(memoryfixture::control,candidate)==123);
// Lookup respects snapshot read budget.
membershipFixture();Snapshot<memoryfixture::Memory> limited(m);std::uint64_t discard=0;for(unsigned i=0;i<768;++i)check(limited.raw(candidate,&discard,8));check(!copiedFlightHolder204(limited,memoryfixture::base,memoryfixture::control,candidate,flight_test::stateHolder,flight_test::stateHolder));
// Exercise a full-size bounded lookup at the lowest state key: 17 iterations.
membershipFixture();m.u32(table+0x20,65536);
unsigned lo=0,hi=65536;
while(lo<hi){const auto mid=lo+(hi-lo)/2;const auto at=array+static_cast<std::uintptr_t>(mid)*24;if(mid==0){m.ptr(at,11);m.ptr(at+8,22);m.ptr(at+16,candidateNode);break;}const auto extraNode=0x10000000ULL+static_cast<std::uintptr_t>(mid)*0x200;m.ptr(at,11);m.ptr(at+8,22);m.ptr(at+16,extraNode);m.u32(extraNode+24,0x54424631+mid);m.put<std::uint8_t>(extraNode+0x111,1);hi=mid;}
check(invoke(memoryfixture::control,candidate)==boots::ringVariant);
for(auto state:{0x15914f6du,0x269521ceu,0x61b7853eu,0xe4ef0545u,0x3ac4a94eu}){
 membershipFixture();m.u32(candidateNode+24,state);
 check(invoke(memoryfixture::control,candidate)==0x54424340);
 commaEnabled204.store(false);check(invoke(memoryfixture::control,candidate)==123);
 commaEnabled204.store(true);m.u32(memoryfixture::itemRecord+8,99);
 check(invoke(memoryfixture::control,candidate)==123);
}
std::cout<<"selector adapter checks passed: "<<n<<" assertions\n";
}





