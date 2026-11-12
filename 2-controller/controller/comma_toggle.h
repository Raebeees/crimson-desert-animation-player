#pragma once
#include <atomic>
#include <cstdint>
namespace boots204 {
// Session preference only. Never modifies real equipment or native input data.
inline std::atomic<bool> commaEnabled204{true};
class CommaEdge204 {
    std::uintptr_t actor_=0;
    std::uint64_t last_=0;
    bool armed_=false,down_=false;
public:
    bool update(bool eligible,std::uintptr_t actor,bool down,std::uint64_t now){
        if(!eligible || !actor){actor_=0;armed_=false;down_=down;last_=now;return false;}
        if(actor_!=actor || now<last_ || now-last_>250){armed_=false;down_=down;actor_=actor;}
        last_=now;
        if(!armed_){armed_=!down;down_=down;return false;}
        const bool edge=down&&!down_;down_=down;return edge;
    }
};
}
