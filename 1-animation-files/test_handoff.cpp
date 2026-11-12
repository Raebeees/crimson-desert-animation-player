#include <windows.h>
#include <cassert>
#include <cstdint>
#include "idle_shared_api.h"
static unsigned got=0;
bool capture(const idle_shared::Registration* p){assert(p&&p->abi==4&&p->count==18&&p->evaluate);got=p->count;return true;}
int main(){HMODULE h=LoadLibraryW(L"DanceModule.asi");assert(h);auto init=reinterpret_cast<bool(*)(std::uintptr_t,std::uintptr_t,idle_shared::Register)>(GetProcAddress(h,"DanceInitialize1"));auto service=GetProcAddress(h,"DanceService1");auto menu=GetProcAddress(h,"LowCortisolOpenMenu1");assert(init&&service&&menu);assert(init(0x10000,0x20000,capture));assert(got==18);assert(!init(0x10000,0x20000,capture));FreeLibrary(h);}
