#include "flight_gate204.h"
#include "ring_policy.h"
#include "policy.h"
#include "key_samples204.h"
#include <cassert>
#include <cstdio>
int main(){
 constexpr std::uint32_t stop=0x54424635;
 assert(task007::classify(stop)==0x8ed159f1);
 assert(task007::mappedPrivate(stop));
 assert(boots204::privateFlightState(stop));
 assert(boots204::entryContext(stop));
 assert(boots::flightState(stop));
 assert(boots::ringSelection(true,true,stop,0)==boots::ringVariant);
 assert(boots::ringSelection(false,true,stop,77)==77);
 assert(boots::variant(false,stop,77)==77);
 bool cancel=false;for(auto v:boots204::keyFlightStates204)if(v==stop)cancel=true;assert(cancel);
 boots204::FlightGateObservation o{};o.verified=true;o.lower={stop,true};
 o.identity.boots=boots204::EquipmentPresence::present;o.descriptor.words[1]=1997602;
 assert(boots204::decideFlightGate(o,true).value==1);
 o.identity.boots=boots204::EquipmentPresence::absent;
 assert(boots204::decideFlightGate(o,true).value==0);
 assert(!boots204::privateFlightState(stop+1));
 puts("Braking state: scoped variants, flight continuity, cancel membership and equipment exit checks passed.");
}
