#include <windows.h>
#include <cstdio>
int main(int argc,char** argv){
    if(argc!=2)return 1;
    HMODULE m=LoadLibraryW(L".\\TidebreakerFlight204.asi");
    if(!m)return 2;
    auto query=reinterpret_cast<unsigned(*)()>(GetProcAddress(m,"TidebreakerStartupStatus204"));
    if(!query)return 3;
    const auto end=GetTickCount64()+5000;
    unsigned result;
    while(!(result=query()) && GetTickCount64()<end)Sleep(10);
    printf("Expected startup status %s, observed %u\n",argv[1],result);
    return result==strtoul(argv[1],nullptr,10)?0:4;
}
