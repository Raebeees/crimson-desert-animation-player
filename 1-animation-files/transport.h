#pragma once
#include <cstdint>
#include "playback.h"
// The menu owns audio. A verified game-thread adapter must supply this interface.
// Requests must be queued, not call game functions from the window thread.
// stop must fence pending starts; snapshot must include actor lifetime generation.
// No provider exists yet. An unbound ASI refuses playback.
namespace lowcortisol {
struct Observation {
 std::uint32_t size=sizeof(Observation),version=2;
 idle::Context context{};
 idle::Token token=0;
 double clipSeconds=0;
 bool started=false,stopped=false,failed=false;
};
struct Transport {
 std::uint32_t size=sizeof(Transport),version=2;
 bool (*start)(idle::Token,idle::Token,std::uint32_t)=nullptr;
 bool (*stop)(idle::Token)=nullptr;
 bool (*observe)(Observation*)=nullptr;
};
}
