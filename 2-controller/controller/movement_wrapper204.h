#pragma once
#include "movement_adapter204.h"
namespace boots204 {
struct SolverArguments204 {std::uintptr_t a,b,c,d,e,f,g;};
using SolverOriginal204=std::uintptr_t(*)(std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t);
inline bool sameMovement204(const MovementObservation204& a,const MovementObservation204& b) {
 return a.valid&&b.valid&&a.identity.actor==b.identity.actor&&a.identity.control==b.identity.control&&
 a.identity.boots==b.identity.boots&&a.solver==b.solver&&a.state.id==b.state.id&&a.state.active==b.state.active&&
 a.actions==b.actions&&a.dt==b.dt&&a.velocity==b.velocity;
}
// Caller supplies a captured original, verified module/caller ABI and the native
// simulation thread. Never call this from a worker or invoke the solver for testing.
template<class Reader,class Writer,class Runtime,class Errors,class Foreground>
std::uintptr_t forwardMovement204(SolverOriginal204 original,const SolverArguments204& args,std::uintptr_t caller,
 Reader& reader,Writer& writer,Runtime& runtime,Errors& errors,Foreground foreground,MovementAdapter204& adapter,
 std::uintptr_t base,std::uintptr_t cc,std::uint64_t now) {
 const auto incomingError=errors.get();
 try {
  const auto before=inspectMovement204(reader,base,cc,args.a,args.b,caller,runtime.assetsReady()&&runtime.slotsOwned(),foreground());
  auto proposed=adapter.prepare(before,now);
  if(proposed.write){
   const auto after=inspectMovement204(reader,base,cc,args.a,args.b,caller,runtime.assetsReady()&&runtime.slotsOwned(),foreground());
   if(!sameMovement204(before,after)||!runtime.assetsReady()||!runtime.slotsOwned()||!foreground()||
      !writer.compareWrite(proposed.address,proposed.expected,proposed.value))adapter.reset();
  }
 }catch(...){adapter.reset();}
 errors.set(incomingError);
 return original(args.a,args.b,args.c,args.d,args.e,args.f,args.g);
}
} // namespace boots204
