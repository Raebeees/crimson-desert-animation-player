#include "alloc_trace204.h"
int main(){
 AllocTrace204 t;
 t.observe("boots allocator: VirtualAlloc2 failed Win32 error",87,0);
 t.observe("snapshot regions/free regions",44,3);
 t.observe("boots allocator: executable trampoline allocation failed",0,0);
 if(t.kind!=2||t.winError!=87||t.freeRegions!=3)return 1;
 AllocTrace204 other;
 other.observe("boots allocator: hook bookkeeping allocation failed",0,0);
 return other.kind==1 && other.winError==0 ? 0 : 2;
}
