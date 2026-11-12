#include "movement_wrapper204.h"
#include "velocity_writer204.h"
#include "input_fixture.h"
#include <functional>
using namespace boots204;
unsigned calls=0,n=0,error=17;SolverArguments204 received{};
std::uintptr_t native(std::uintptr_t a,std::uintptr_t b,std::uintptr_t c,std::uintptr_t d,std::uintptr_t e,std::uintptr_t f,std::uintptr_t g){++calls;received={a,b,c,d,e,f,g};if(error!=17)throw 7;error=29;return 0x123456789abcdef0ULL;}
struct Errors{unsigned get(){return error;}void set(unsigned v){error=v;}};
struct Runtime{bool assets=true,slots=true;bool assetsReady(){return assets;}bool slotsOwned(){return slots;}};
void check(bool b){++n;if(!b)throw n;}
int main(){
auto m=identity_test::fixture();constexpr std::uintptr_t controller=0x610000,solver=0x611000,time=0x612000;
m.ptr(memoryfixture::control+0x140,controller);m.ptr(controller+0x2c0,solver);m.put<float>(time,0.016f);m.put<float>(solver+0xc4,0.f);m.put<float>(solver+0x180,0.f);m.put<float>(solver+0x184,1.f);m.put<float>(solver+0x188,0.f);

struct Writer{memoryfixture::Memory& memory;unsigned writes=0;bool fail=false;bool compareWrite(std::uintptr_t address,float old,float value){error=99;if(fail)return false;float actual=0;if(!memory.read(address,&actual,4)||actual!=old)return false;memory.put<float>(address,value);++writes;return true;}}writer{m};
Runtime runtime;Errors errors;MovementAdapter204 adapter;bool focus=true;std::function<void()> during;
auto foreground=[&]{if(during){auto run=during;during={};run();}return focus;};
auto call=[&](std::uint64_t now,std::uintptr_t caller=memoryfixture::base+0x3979e2d){calls=0;error=17;const auto result=forwardMovement204(&native,{solver,time,3,4,5,6,7},caller,m,writer,runtime,errors,foreground,adapter,memoryfixture::base,memoryfixture::cc,now);check(result==0x123456789abcdef0ULL&&calls==1&&error==29);check(received.a==solver&&received.b==time&&received.c==3&&received.d==4&&received.e==5&&received.f==6&&received.g==7);};
call(1000);check(writer.writes==1);m.put<std::uint8_t>(identity_test::action+0x5e,0);call(1016);m.put<std::uint8_t>(identity_test::action+0x5e,1);call(1032);float v=0;m.read(solver+0xc4,&v,4);check(v>0&&v<=3);
auto previous=writer.writes;runtime.assets=false;call(1048);check(writer.writes==previous);runtime.assets=true;
focus=false;call(1064);check(writer.writes==previous);focus=true;
m.u32(memoryfixture::itemRecord+8,99);call(1080);check(writer.writes==previous);m.u32(memoryfixture::itemRecord+8,1987201);
call(1096,1);check(writer.writes==previous);writer.fail=true;call(1112);check(writer.writes==previous);writer.fail=false;
// Verify the concrete writer only on this test process's private memory.
VelocityWriter204 concrete;alignas(4) float own=1.f;auto address=reinterpret_cast<std::uintptr_t>(&own);
check(concrete.compareWrite(address,1.f,2.f)&&own==2.f);check(!concrete.compareWrite(address,1.f,0.f)&&own==2.f);check(!concrete.compareWrite(address,2.f,4.f));check(!concrete.compareWrite(1,0.f,0.f));
std::cout<<"movement wrapper checks passed: "<<n<<" assertions\n";
}
