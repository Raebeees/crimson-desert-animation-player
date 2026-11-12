#include "gate_fixture.h"
#include "gate_forwarding204.h"
int main() {
    using namespace flight_test;
    unsigned cases=0;
    for(unsigned offset=0;offset<8;++offset) {
        for(unsigned skill : {1997601u,1997602u}) {
            auto m=fixture();
            m.ptr(holder,desc+offset);
            const unsigned words[]={238,skill,1,0,0};
            for(unsigned i=0;i<5;++i)m.u32(desc+offset+i*4,words[i]);
            auto observed=inspect(m);
            check(observed.verified,"packed descriptor accepted");
            check(observed.descriptor.words[1]==skill,"skill preserved");
            observed.identity.boots=P::absent;
            const auto off=boots204::decideFlightGate(observed,true);
            check(off.handled && off.value==(skill==1997601?1:0),"boots-off entry/exit");
            observed.identity.boots=P::present;
            const auto on=boots204::decideFlightGate(observed,true);
            check(on.handled && on.value==(skill==1997601?0:1),"boots-on entry/exit");
            ++cases;
        }
    }
    for(unsigned defect=0;defect<5;++defect) {
        auto m=fixture();
        constexpr auto address=desc+4;
        m.ptr(holder,address);
        const unsigned words[]={238,1997601,1,0,0};
        for(unsigned i=0;i<5;++i)m.u32(address+i*4,words[i]);
        if(defect==0)m.bytes.erase(address+19);
        if(defect==1)m.u32(address,237);
        if(defect==2)m.u32(address+4,15029);
        if(defect==3)m.ptr(ctx+8,memoryfixture::actor+8);
        if(defect==4)m.change=address;
        check(!inspect(m).verified,"invalid packed descriptor rejected");
        ++cases;
    }
    std::cout<<cases<<" packed-descriptor and rejection cases passed\n";
}
