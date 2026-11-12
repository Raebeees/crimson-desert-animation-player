#include <windows.h>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include "MinHook.h"
extern "C" void BootsAllocationTrace(const char*,DWORD,DWORD) {}
using Fn=int(*)();
static int replacement(){return 17;}
int main(int argc,char**){
    const bool originalAllocator=argc==2;
    const SIZE_T size=0xA0000000ULL;
    auto block=static_cast<unsigned char*>(VirtualAlloc(nullptr,size,MEM_RESERVE,PAGE_NOACCESS));
    if(!block)return 1;
    auto target=static_cast<unsigned char*>(VirtualAlloc(block+size/2,4096,MEM_COMMIT,PAGE_EXECUTE_READWRITE));
    if(!target)return 2;
    unsigned char body[]={0xb8,8,0,0,0,0xc3};memcpy(target,body,sizeof body);
    FlushInstructionCache(GetCurrentProcess(),target,sizeof body);
    Fn native=nullptr;auto call=reinterpret_cast<Fn>(target);
    if(MH_Initialize()!=MH_OK||call()!=8)return 3;
    auto status=MH_CreateHook(target,reinterpret_cast<void*>(&replacement),reinterpret_cast<void**>(&native));
    if(originalAllocator){printf("Original allocator status %d (expect 9)\n",int(status));return status==MH_ERROR_MEMORY_ALLOC?0:4;}
    if(status!=MH_OK||!native)return 5;
    auto delta=reinterpret_cast<intptr_t>(native)-reinterpret_cast<intptr_t>(target);
    if(delta>-0x40000000LL&&delta<0x40000000LL)return 6;
    if(MH_EnableHook(target)!=MH_OK||call()!=17||native()!=8)return 7;
    if(MH_DisableHook(target)!=MH_OK||call()!=8||MH_RemoveHook(target)!=MH_OK)return 8;
    printf("Far trampoline delta %lld; hook, original and restore passed\n",(long long)delta);
    MH_Uninitialize();VirtualFree(block,0,MEM_RELEASE);return 0;
}
