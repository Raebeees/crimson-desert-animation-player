#pragma once
#include "input_identity204.h"
#include "vertical_control.h"
#include "solver_policy.h"
namespace boots204 {
struct MovementObservation204 {
 bool valid=false,foreign=false;IdentityEquipment identity{};LowerState state{};
 std::uintptr_t solver=0;float dt=0,velocity=0;
 std::array<Action204,4> actions{};
};
// Read only: callers are current-build instruction-boundary-verified sites.
// All ownership/input/state/solver reads share one bounded stability journal.
template<class Reader> MovementObservation204 inspectMovement204(Reader& reader,std::uintptr_t base,std::uintptr_t cc,
 std::uintptr_t solver,std::uintptr_t time,std::uintptr_t caller,bool ready,bool focused) {
 MovementObservation204 o;Snapshot<Reader> s(reader);
 if(base<=0x7ffff0000000ULL&&caller!=base+0x3979e2d&&caller!=base+0x397e633){o.foreign=true;return o;}
 if(!ready||!focused||base>0x7ffff0000000ULL||solver<0x10000||solver>=0x800000000000ULL-0x190||solver%8||time<0x10000)return {};
 std::uintptr_t components=0,controller=0,owned=0,input=0;bool suppressed=true;float up[3]{};
 if(!readIdentityEquipment(s,base,cc,o.identity)||o.identity.boots!=EquipmentPresence::present||
 !readLowerState(s,o.identity.control,o.state)||!o.state.active||!boots::controlledFlightState(o.state.id)||
 !s.pointer(o.identity.actor+0x68,components)||!s.pointer(o.identity.control+0x140,controller)||
 !s.pointer(controller+0x2c0,owned))return {};
 if(owned!=solver){if(s.stable())o.foreign=true;return o;}
 if(!inputSuppressed204(s,components,suppressed)||suppressed||
 !readInputObject204(s,base,input)||!s.read(time,o.dt)||!std::isfinite(o.dt)||o.dt<=0||o.dt>0.1f||
 !s.read(solver+0xc4,o.velocity)||!std::isfinite(o.velocity))return {};
 for(unsigned i=0;i<3;++i)if(!s.read(solver+0x180+i*4,up[i])||!std::isfinite(up[i]))return {};
 if(std::fabs(up[0])>0.001f||std::fabs(up[1]-1.f)>0.001f||std::fabs(up[2])>0.001f)return {};
 for(unsigned i=0;i<4;++i){o.actions[i]=readAction204(s,input,movementActions204[i]);if(o.actions[i]==Action204::unavailable)return {};}
 if(!s.stable())return {};o.valid=true;o.solver=solver;return o;
}
struct MovementProposal204 {std::uintptr_t address=0;float expected=0,value=0;bool write=false;};
// Pure proposal stage. No game writes or native calls. Production must recheck
// identity/readiness immediately before applying, then call the original solver.
class MovementAdapter204 {
 boots::VerticalControl vertical;boots::VerticalOutput command{};
 std::uintptr_t actor=0,solver=0;std::uint64_t last=0;
public:
 void reset(){vertical.reset();command={};actor=solver=0;last=0;}
 MovementProposal204 prepare(const MovementObservation204& o,std::uint64_t now) {
  if(o.foreign)return {}; // Foreign physics callbacks must not reset our ramp.
  if(!o.valid){reset();return {};}
  if(o.identity.actor!=actor||o.solver!=solver||now<last||now-last>100){reset();actor=o.identity.actor;solver=o.solver;last=now;}
  if(o.actions[3]==Action204::held){reset();return {};}
  if(now!=last||!command.overrideVertical){boots::VerticalInput in;in.actor=actor;in.eligible=true;
   in.up=o.actions[0]==Action204::held;in.down=o.actions[1]==Action204::held||o.actions[2]==Action204::held;
   in.dt=now==last?0.001f:float(now-last)*0.001f;command=vertical.update(in);last=now;}
  return {solver+0xc4,o.velocity,command.speed,command.overrideVertical};
 }
};
} // namespace boots204
