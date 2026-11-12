#pragma once
#include <cstdint>
namespace task008 {
constexpr uint32_t skill=1997605, hover=0x54424438, nativeHover=0xEB340761, dispatcher=0x54424437;
inline bool dodgeRecovery(uint32_t s){return s>=0x54424431 && s<=0x54424436;}
inline bool privateExit(uint32_t s){return dodgeRecovery(s)||s==dispatcher||s==hover;}
inline bool exitContext(uint32_t s){
 if(privateExit(s)||s==nativeHover)return true;
 switch(s){case 0x6ECD9C21:case 0x64F500EB:case 0x3DF1D45F:case 0x1D49E726:case 0xD28ABCEC:case 0x9BEF84E4:case 0xBF31394A:return true;default:return false;}
}
inline uint32_t select(bool scoped,bool ready,uint32_t raw,uint32_t original){
 if(!scoped||!dodgeRecovery(raw))return original;
 return ready?0x54425247:0x54424854;
}
// Semantic flight context for existing class/key overrides, not solver/variant selection.
inline bool flightContext(uint32_t raw,bool oldResult){return oldResult||dodgeRecovery(raw)||raw==dispatcher;}
inline bool retain(uint32_t raw,bool oldResult){return oldResult||dodgeRecovery(raw)||raw==dispatcher;}
}

