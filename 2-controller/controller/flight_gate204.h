#pragma once
#include "condition_descriptor204.h"
#include "flight_state204.h"
#include <atomic>
#include "comma_toggle.h"
#include "launch_request.h"
namespace boots204 {
enum class EquipmentPresence { unknown, absent, present };
struct IdentityEquipment {
    std::uintptr_t actor=0,control=0;
    EquipmentPresence boots=EquipmentPresence::unknown;
};
// Internal uncommitted observation. Only publish after the shared journal passes.
template<class Reader> bool readIdentityEquipment(Snapshot<Reader>& s,
    std::uintptr_t base,std::uintptr_t cc,IdentityEquipment& output,
    std::uintptr_t expectedActor=0,std::uintptr_t expectedControl=0,bool physicalEquipment=false) {
    output={};
    if(base<0x10000 || base>=0x800000000000ULL-0x71c1088 ||
       cc<0x10000 || cc>=0x800000000000ULL-0x193510)return false;
    std::uintptr_t world=0,manager=0,user=0,actor=0,userLink=0,other=0,components=0;
    if(!s.pointer(base+0x71c0100,world) || !s.pointer(world+0x30,manager) ||
       !s.pointer(manager+0x58,user) || !s.pointer(user+0xd8,actor) ||
       (expectedActor && expectedActor!=actor) || !s.pointer(user+8,userLink) ||
       !s.pointer(userLink+0x38,other) || other!=actor || !s.pointer(actor+0x68,components))return false;
    std::uintptr_t character=0,control=0,ownerLink=0,owner=0,scene=0,appearance=0,actual=0,expected=0;
    std::uint16_t handle=0;std::uint32_t characterKey=0;
    if(!s.pointer(components+0x20,character) || !s.read(character+0x30,handle) ||
       !recordKey(s,base,0x71c1020,handle,characterKey) || characterKey!=1 ||
       !s.pointer(components+0x40,control) || (expectedControl && expectedControl!=control) ||
       !s.pointer(control+0x58,ownerLink) || !s.pointer(ownerLink,owner) || owner!=actor ||
       !s.pointer(control+0xb8,scene) || !s.pointer(scene+0x20,appearance) ||
       !s.pointer(appearance,actual) || !s.pointer(cc+0x193508,expected) || actual!=expected)return false;

    std::uintptr_t equipment=0,slots=0,rows=0;std::uint32_t count=0;
    if(!s.pointer(components+0x38,equipment) || !s.pointer(equipment+0xa0,slots) ||
       !s.read(slots+0x10,count) || count>40)return false;
    // A valid empty list has no rows to dereference. A missing list is unknown.
    if(count && !s.pointer(slots+8,rows))return false;
    bool found=false;
    for(std::uint32_t i=0;i<count;++i) {
        std::uint16_t itemHandle=0;std::uint32_t itemKey=0;
        if(!s.read(rows+i*0xd0ULL+8,itemHandle) || !recordKey(s,base,0x71c1080,itemHandle,itemKey))return false;
        found=found || itemKey==1987201;
    }
    output.actor=actor;output.control=control;
    output.boots=(found && (physicalEquipment || commaEnabled204.load(std::memory_order_acquire)))?EquipmentPresence::present:EquipmentPresence::absent;
    return true;
}
inline std::atomic<unsigned> gateStages204[5]{};
inline std::atomic<unsigned> gateDecisions204[4]{};
struct FlightGateObservation {
    bool verified=false;
    IdentityEquipment identity{};
    PrivateDescriptor descriptor{};
    LowerState lower{};
    unsigned reads=0;
};
template<class Reader> FlightGateObservation inspectFlightGate(Reader& reader,
    std::uintptr_t base,std::uintptr_t cc,std::uintptr_t context,std::uintptr_t holder) {
    Snapshot<Reader> journal(reader);IdentityEquipment identity;PrivateDescriptor descriptor;LowerState lower;
    FlightGateObservation result;
    unsigned stage=0;
    if(readIdentityEquipment(journal,base,cc,identity)) {
        stage=1;
        if(readPrivateDescriptor(journal,context,holder,identity.actor,descriptor)) {
            stage=2;
            if(readLowerState(journal,identity.control,lower)) {
                stage=3;
                if(journal.stable()) {
                    stage=4;
                    result.verified=true;result.identity=identity;result.descriptor=descriptor;result.lower=lower;
                }
            }
        }
    }
    gateStages204[stage].fetch_add(1,std::memory_order_relaxed);
    result.reads=journal.reads();return result;
}
struct GateDecision204 {bool handled=false;std::uint8_t value=1;};
// runtimeReady must reflect assets and owned hook slots; this pure function
// cannot certify those. Native dispatcher prechecks are never bypassed.
inline GateDecision204 decideFlightGate(const FlightGateObservation& o,bool runtimeReady) {
 if(!o.verified)return {};
 const bool continuation=runtimeReady && o.identity.boots==EquipmentPresence::present;
 if(!o.lower.active)return {true,1};
 if(o.descriptor.words[1]==1997601 && launchSource204(o.lower.id))
     return {true,static_cast<std::uint8_t>(continuation && requestedLaunch204(o.identity.actor,o.lower.id,GetTickCount64())?0:1)};
 if(o.descriptor.words[1]==1997601)
     return {true,static_cast<std::uint8_t>(entryContext(o.lower.id) && continuation?0:1)};
 if(o.descriptor.words[1]==1997602)
     return {true,static_cast<std::uint8_t>(exitContext204(o.lower.id) && !continuation?0:1)};
 return {};
}
} // namespace boots204
