#include "gate_fixture.h"
int main() {
    using namespace flight_test;
    for(unsigned stage=0;stage<5;++stage) {
        auto m=fixture();
        if(stage==0)m.bytes.erase(memoryfixture::base+0x71c0100);
        if(stage==1)m.bytes.erase(desc);
        if(stage==2)m.bytes.erase(stateHolder);
        if(stage==3)m.change=memoryfixture::base+0x71c0100;
        const auto result=inspect(m);
        check(result.verified==(stage==4),"verification result");
        check(boots204::gateStages204[stage].load()==1,"correct failure counter");
    }
    auto m=fixture();
    const auto o=inspect(m);
    auto entry=o;
    entry.descriptor.words[1]=1997601;
    entry.identity.boots=P::absent;
    check(boots204::decideFlightGate(entry,true).value==1,"boots-off entry rejected");
    entry.identity.boots=P::present;
    check(boots204::decideFlightGate(entry,true).value==0,"boots-on entry accepted");
    std::cout<<"Five observation stages and boots entry decisions passed\n";
}
