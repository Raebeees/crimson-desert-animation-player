#include "gate_fixture.h"
int main() {
    using namespace flight_test;
    for(unsigned stage=0;stage<9;++stage) {
        auto m=fixture();
        auto context=ctx;
        if(stage==0)context=1;
        if(stage==1)m.bytes.erase(ctx+8);
        if(stage==2)m.ptr(ctx+8,memoryfixture::actor+8);
        if(stage==3)m.bytes.erase(holder);
        if(stage==4)m.ptr(holder,0x800000000000ULL-16);
        if(stage==5)m.u32(desc,237);
        if(stage==6)m.u32(desc+4,15029);
        if(stage==7)m.u32(desc+8,0);
        boots204::Snapshot<Memory> journal(m);
        boots204::PrivateDescriptor result;
        const bool valid=boots204::readPrivateDescriptor(journal,context,holder,memoryfixture::actor,result);
        check(valid==(stage==8),"descriptor acceptance preserved");
        check(boots204::descriptorReject204[stage].load()==1,"descriptor reason");
        check(valid || result.address==0,"partial result not published");
    }
    std::cout<<"Nine descriptor outcomes and partial-output rejection passed\n";
}
