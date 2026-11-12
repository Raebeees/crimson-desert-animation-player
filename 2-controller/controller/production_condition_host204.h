#pragma once
#include "windows_slots204.h"
#include "condition_callbacks204.h"
namespace boots204 {
struct ProcessReader204 {
 bool read(Word address,void* output,std::size_t size) const noexcept {
  if(!output || !size || size>4096 || address<0x10000 || address>=0x800000000000ULL-size)return false;
  SIZE_T count=0;
  return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(address),output,size,&count)!=0 && count==size;
 }
};
struct WindowsErrors204 { DWORD get()const noexcept{return GetLastError();}void set(DWORD v)const noexcept{SetLastError(v);} };
inline bool foregroundProcess204() noexcept {
 const HWND window=GetForegroundWindow();if(!window)return false;
 DWORD pid=0;GetWindowThreadProcessId(window,&pid);return pid==GetCurrentProcessId();
}
// Host must be allocated for process lifetime; do not unload after publication.
// Caller verifies module identities, slot addresses/original chain, complete
// runtime assets and all other hooks BEFORE calling activate. This class neither
// discovers addresses nor authorizes activation. Readiness starts false.
template<class Reader,class Tag> struct ConditionHost204 {
 using Runtime=RuntimeCondition204<WindowsSlots204>;
 using Callbacks=ConditionCallbacks204<Runtime,Reader,WindowsErrors204,Tag>;
 Runtime runtime;
 typename Callbacks::Context context;
 bool attempted=false;
 ConditionHost204(WindowsSlots204& backend,Reader& reader,Word base,Word cc,Foreground204 foreground):
  runtime(backend),context{runtime,reader,base,cc,foreground}{}
 ConditionHost204(const ConditionHost204&)=delete;
 ConditionHost204& operator=(const ConditionHost204&)=delete;
 SetupReport activate(const SlotPlan& plan) {
  if(attempted)return {Setup::reused,{}};
  attempted=true;
  if(!plan.prerequisites_verified || plan.wrappers!=Callbacks::addresses())return {Setup::rejected,{}};
  if(!Callbacks::bind(context))return {Setup::reused,{}};
  return runtime.activate(plan);
 }
};
struct ProductionConditionTag204{};
using ProductionConditionHost204=ConditionHost204<ProcessReader204,ProductionConditionTag204>;
} // namespace boots204
