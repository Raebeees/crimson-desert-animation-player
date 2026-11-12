#pragma once
#include <cstdint>
namespace boots204 {
// One attempt per process, only after uninterrupted eligible gameplay.
struct DeferredCompat204 {
    std::uint64_t actor=0, since=0, last=0;
    bool tracking=false, attempted=false;
    bool update(bool eligible,std::uint64_t identity,std::uint64_t now) noexcept {
        if(attempted)return false;
        if(!eligible || !identity){tracking=false;return false;}
        if(!tracking || actor!=identity || now<last || now-last>100){
            actor=identity;since=now;tracking=true;
        }
        last=now;
        if(now-since<2000)return false;
        attempted=true;return true;
    }
};
}
