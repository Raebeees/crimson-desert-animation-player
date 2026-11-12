#pragma once
#include <cstdint>
#include "task008_policy.h"
namespace task007 {
constexpr uint32_t nativeState=0x9C0C7E5B, privateState=0x54424631;
constexpr uint32_t enterSkill=1997601, exitSkill=1997602;
struct Pair {uint32_t nativeId,privateId;};
inline constexpr Pair pairs[]={{0x9C0C7E5B,0x54424631},{0x4ABEBF9D,0x54424632},{0x164E8B07,0x54424633},{0x8ED159F1,0x54424634},{0x8ED159F1,0x54424635}};
inline bool mappedNative(uint32_t raw){for(auto p:pairs)if(raw==p.nativeId)return true;return false;}
inline bool mappedPrivate(uint32_t raw){for(auto p:pairs)if(raw==p.privateId)return true;return false;}
inline uint32_t classify(uint32_t raw){if(raw==task008::hover)return task008::nativeHover;for(auto p:pairs)if(raw==p.privateId)return p.nativeId;return raw;}
enum class Gate:uint8_t {none,enter,exit};
inline Gate identify(const uint32_t (&words)[5]){
    if(words[0]!=237 || words[2]!=1 || words[3] || words[4])return Gate::none;
    return words[1]==enterSkill?Gate::enter:words[1]==exitSkill?Gate::exit:Gate::none;
}
enum class Reason:uint8_t {not_private,unavailable,wrong_state,entry_match,entry_denied,continue_private,exit_match};
struct Decision {bool handled;uint8_t result;Reason reason;};
inline Decision decide(Gate gate,bool readable,uint32_t raw,bool continuation){
    if(gate==Gate::none)return {false,1,Reason::not_private};
    if(!readable)return {true,1,Reason::unavailable};
    if(gate==Gate::enter){
        if(!(mappedNative(raw)||mappedPrivate(raw)||raw==task008::hover||raw==task008::nativeHover))return {true,1,Reason::wrong_state};
        return {true,uint8_t(continuation?0:1),continuation?Reason::entry_match:Reason::entry_denied};
    }
    if(!(mappedNative(raw)||mappedPrivate(raw)||task008::exitContext(raw)))return {true,1,Reason::wrong_state};
    return {true,uint8_t(continuation?1:0),continuation?Reason::continue_private:Reason::exit_match};
}
struct Budget {
    uint64_t start=0;
    uint32_t rows=0,bytes=0;
    bool started=false,finished=false;
    void begin(uint64_t now){if(!started && !finished){started=true;start=now;}}
    bool expired(uint64_t now)const{return started && now-start>=120000;}
    bool room(uint32_t n)const{return started && !finished && rows<8191 && uint64_t(bytes)+n<=8*1024*1024-1024;}
};
}
