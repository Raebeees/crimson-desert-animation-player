#pragma once
#include <cstdint>
#include <cmath>
namespace dance_v2 {
constexpr std::uint32_t version=2;
constexpr std::uint32_t vervePartTwo=4294960588u;
constexpr std::uint32_t ids[]={0xffdb4dcbu,0xfffba235u,0xffe91bc1u,0xfff930cfu,0xffff5a5du,4294947163u,4294934262u,4294964113u};
constexpr double durations[]={10.944240362811792,18.4,9.733356009070295,42.666666666666664,10.666666666666666,67.0,35.93333333333333,12.066666666666666};
inline std::uint32_t canonical(std::uint32_t id){return id==vervePartTwo?ids[5]:id;}
inline bool known(std::uint32_t id){id=canonical(id);for(auto x:ids)if(x==id)return true;return false;}
inline double duration(std::uint32_t id){id=canonical(id);for(unsigned i=0;i<8;++i)if(ids[i]==id)return durations[i];return 0.;}
inline unsigned route(std::uint32_t id){for(unsigned i=0;i<8;++i)if(ids[i]==id)return i==0?1:i==1?0:i+1;return 99;}
struct Request {std::uint32_t size=sizeof(Request),abi=version;std::uint64_t token=0,actor=0;std::uint32_t action=0,reserved=0;};
inline bool valid(const Request& r){return r.size==sizeof(Request)&&r.abi==version&&r.token&&r.actor&&!r.reserved&&known(r.action)&&canonical(r.action)==r.action;}
class Selection {
 Request active{};bool held=false;
public:
 bool start(const Request& r){if(held||!valid(r))return false;active=r;held=true;return true;}
 bool permits(unsigned offset)const{return held&&offset==route(active.action);}
 bool acknowledge(std::uint64_t token,std::uint32_t upper,std::uint32_t lower)const{return held&&token==active.token&&upper==active.action&&lower==active.action;}
 bool finish(std::uint64_t token,bool leftUpper,bool leftLower){if(!held||active.token!=token||!leftUpper||!leftLower)return false;held=false;active={};return true;}
};
}
