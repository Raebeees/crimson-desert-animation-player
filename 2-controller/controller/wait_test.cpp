#include "wait204.h"
#include <cstdio>
#include <cstdlib>
int main(){
    for(int test=0;test<5;++test){
        unsigned elapsed=0,reads=0,pauses=0;
        bool pass=waitForConditions204([&]{
            ++reads;
            if(test==4)elapsed+=60000;
            return test==0 || (test==1&&reads>=3) || (test==2&&reads!=2) || test==4;
        },[&]{return elapsed;},[&]{++pauses;elapsed+=250;});
        unsigned counts[]={2,4,4,480,2};
        if(pass!=(test<3)||reads!=counts[test]||pauses>480)return 1;
    }
    puts("5 condition-readiness timing cases passed");
}
