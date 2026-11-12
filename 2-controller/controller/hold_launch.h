#pragma once
#include <cstdint>
namespace tidebreaker35 {
enum class Phase { unavailable, ground, jumpStart, airborne, launching, flight };
struct Input {
    std::uintptr_t actor=0;
    std::uint64_t now=0;
    bool equipped=false,enabled=false,gameplay=false,focused=false,held=false;
    Phase phase=Phase::unavailable;
};
// Pure request policy. Does not suppress, simulate or consume native jump input.
// An adapter must validate the native route before acting on the request.
class HoldLaunch {
    std::uintptr_t actor_=0;
    std::uint64_t last_=0,start_=0;
    bool neutral_=false,down_=false,tracking_=false,fired_=false;
public:
    static constexpr std::uint64_t holdMs=350;
    void reset(){*this=HoldLaunch{};}
    bool update(const Input& in){
        if(!in.actor || !in.equipped || !in.enabled || !in.gameplay || !in.focused || in.phase==Phase::unavailable){reset();return false;}
        if(actor_!=in.actor || in.now<last_ || in.now-last_>100){reset();actor_=in.actor;}
        last_=in.now;
        if(!neutral_){neutral_=!in.held;down_=in.held;return false;}
        if(!in.held){down_=tracking_=fired_=false;return false;}
        if(in.phase==Phase::launching || in.phase==Phase::flight){tracking_=false;down_=true;return false;}
        if(!down_){
            down_=true;start_=in.now;fired_=false;
            // A polling tick can see the native jump's first state after press.
            // jumpStart must be a validated native jump-start state, not a guess.
            tracking_=in.phase==Phase::ground || in.phase==Phase::jumpStart;
        }
        if(!tracking_ || fired_)return false;
        if(in.now-start_>=holdMs){fired_=true;tracking_=false;return true;}
        return false;
    }
};
}
