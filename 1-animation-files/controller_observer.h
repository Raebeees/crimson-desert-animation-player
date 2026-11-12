#pragma once
#include <atomic>
#include <fstream>
#include <intrin.h>
namespace controller_observer {
using Word=std::uintptr_t;
using Update=void(*)(Word,float,std::uint8_t);
inline Update original=nullptr;
inline std::atomic<unsigned> state{0},hookStatus{0};
inline std::atomic<Word> target{0};
inline Word game=0;
struct Sample {Word control=0,caller=0;DWORD thread=0;float dt=0;unsigned mode=0,tls=0,immediate=0;};
inline Sample sample{};
inline ULONGLONG deadline=0;
// Read TEB TLS only. No actor dereferences, allocations, blocking locks,
// file IO, ReadProcessMemory, or engine calls inside this observation.
inline void tlsSample(Sample* out) noexcept {
 __try {
  const auto array=__readgsqword(0x58);
  if(array){const auto tls=*reinterpret_cast<const Word*>(array);
   if(tls){out->immediate=*reinterpret_cast<const unsigned char*>(tls+0x1eb);out->tls=1;}}
 }__except(EXCEPTION_EXECUTE_HANDLER){out->tls=0;}
}
inline void update(Word control,float dt,std::uint8_t mode){
 const auto caller=reinterpret_cast<Word>(_ReturnAddress());
 // Original update completes while the native caller still retains the actor.
 original(control,dt,mode);
 const auto outgoing=GetLastError();
 if(caller==game+0x99fd33&&control&&control==target.load(std::memory_order_relaxed)){
  unsigned expected=1;
  if(state.compare_exchange_strong(expected,2,std::memory_order_acq_rel)){
   Sample s;s.control=control;s.caller=caller-game;s.dt=dt;s.mode=mode;s.thread=GetCurrentThreadId();
   tlsSample(&s);sample=s;state.store(3,std::memory_order_release);
  }
 }
 SetLastError(outgoing);
}
// Invoked only by the existing file-verified bridge worker after startup.
inline bool install(Word base){
 game=base;
 constexpr unsigned char prefix[]={0x48,0x89,0x5c,0x24,0x10,0x55,0x56,0x57,0x41,0x54,0x41,0x55,
  0x41,0x56,0x41,0x57,0x48,0x8b,0xec,0x48,0x83,0xec,0x60,0xc5};
 unsigned char actual[sizeof prefix]{};boots204::ProcessReader204 reader;
 const auto entry=base+0x9451e0;
 if(!reader.read(entry,actual,sizeof actual)||memcmp(actual,prefix,sizeof actual)){hookStatus.store(2);return false;}
 void* captured=nullptr;
 if(MH_CreateHook(reinterpret_cast<void*>(entry),reinterpret_cast<void*>(&update),&captured)!=MH_OK){hookStatus.store(3);return false;}
 original=reinterpret_cast<Update>(captured);
 if(MH_EnableHook(reinterpret_cast<void*>(entry))!=MH_OK){hookStatus.store(4);return false;}
 hookStatus.store(1);return true;
}
inline void arm(){
 if(hookStatus.load()!=1)return;
 unsigned expected=0;deadline=GetTickCount64()+5000;
 state.compare_exchange_strong(expected,1,std::memory_order_release);
}
inline void flush(const std::filesystem::path& directory){
 const auto current=state.load(std::memory_order_acquire);
 if(current!=3&&!(current==1&&GetTickCount64()>=deadline))return;
 if(current==1){unsigned expected=1;if(!state.compare_exchange_strong(expected,0))return;}
 std::ofstream out(directory/L"LowCortisol-controller-observer.log",std::ios::app);
 if(current==3){const auto s=sample;
  out<<"OBSERVE_ONLY controller="<<std::hex<<s.control<<" caller_rva="<<s.caller<<std::dec
   <<" timestep="<<s.dt<<" mode="<<s.mode<<" thread="<<s.thread<<" tls_present="<<s.tls
   <<" immediate_dispatch="<<s.immediate<<" native_submit_called=0\n";
  state.store(0,std::memory_order_release);
 }else out<<"OBSERVE_ONLY no_local_controller_sample_within_5s native_submit_called=0\n";
}
}
