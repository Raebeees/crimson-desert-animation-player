#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>

// Read-only eligibility, not a flight-readiness or lifetime guarantee.
namespace legacy204 {
enum class Failure { none, unreadable, invalid, changed, budget, ineligible };
struct Result {
    bool eligible=false;
    Failure failure=Failure::none;
    std::uintptr_t actor=0, control=0;
    unsigned reads=0;
};
template<class Reader> class Snapshot {
    struct Entry { std::uintptr_t address; std::uint64_t value; std::size_t size; };
    Entry entries[384]{};
    unsigned used=0, calls=0;
    Reader& reader;
public:
    Failure failure=Failure::none;
    explicit Snapshot(Reader& r):reader(r){}
    unsigned reads() const { return calls; }
    bool raw(std::uintptr_t address, void* out, std::size_t size) {
        if(size==0 || size>8 || address<0x10000 || address>0x7fffffffffffULL-size) {
            failure=Failure::invalid; return false;
        }
        if(calls>=768) { failure=Failure::budget; return false; }
        ++calls;
        if(!reader.read(address,out,size)) { failure=Failure::unreadable; return false; }
        return true;
    }
    template<class T> bool read(std::uintptr_t address,T& out) {
        static_assert(sizeof(T)<=8,"scalar only");
        std::uint64_t value=0;
        if(!raw(address,&value,sizeof(T)))return false;
        for(unsigned i=0;i<used;++i)if(entries[i].address==address && entries[i].size==sizeof(T)) {
            if(entries[i].value!=value) { failure=Failure::changed; return false; }
            std::memcpy(&out,&value,sizeof(T));return true;
        }
        if(used==384) { failure=Failure::budget; return false; }
        entries[used++]={address,value,sizeof(T)};
        std::memcpy(&out,&value,sizeof(T));return true;
    }
    bool pointer(std::uintptr_t address,std::uintptr_t& out) {
        if(!read(address,out))return false;
        if(out<0x10000 || out>=0x800000000000ULL || out%8) { failure=Failure::invalid; return false; }
        return true;
    }
    bool stable() {
        for(unsigned i=0;i<used;++i) {
            std::uint64_t value=0;
            if(!raw(entries[i].address,&value,entries[i].size))return false;
            if(value!=entries[i].value) { failure=Failure::changed;return false; }
        }
        return true;
    }
};
template<class S> bool recordKey(S& s,std::uintptr_t base,std::uintptr_t rva,
                                std::uint16_t handle,std::uint32_t& key) {
    if(handle==0xffff) { s.failure=Failure::invalid;return false; }
    std::uintptr_t cache=0,array=0,record=0;std::uint32_t count=0;
    if(!s.pointer(base+rva,cache) || !s.read(cache+8,count))return false;
    if(!count || count>1000000 || handle>=count) { s.failure=Failure::invalid;return false; }
    return s.pointer(cache+0x58,array) && s.pointer(array+handle*8ULL,record) && s.read(record+8,key);
}
template<class Reader> Result resolve(Reader& reader,std::uintptr_t base,std::uintptr_t cc,
                    std::uintptr_t expectedActor=0,std::uintptr_t expectedControl=0) {
    Snapshot<Reader> s(reader);Result result;
    auto finish=[&](bool eligible) {
        result.eligible=eligible;
        result.failure=s.failure==Failure::none && !eligible?Failure::ineligible:s.failure;
        result.reads=s.reads();return result;
    };
    if(base<0x10000 || base>=0x800000000000ULL-0x71c1088 ||
       cc<0x10000 || cc>=0x800000000000ULL-0x193510)return finish(false);
    std::uintptr_t world=0,manager=0,user=0,actor=0,userLink=0,other=0,components=0;
    if(!s.pointer(base+0x71c0100,world) || !s.pointer(world+0x30,manager) ||
       !s.pointer(manager+0x58,user) || !s.pointer(user+0xd8,actor) ||
       (expectedActor && expectedActor!=actor) || !s.pointer(user+8,userLink) ||
       !s.pointer(userLink+0x38,other) || other!=actor || !s.pointer(actor+0x68,components))return finish(false);
    std::uintptr_t character=0,control=0,ownerLink=0,owner=0,scene=0,appearance=0,actual=0,expected=0;
    std::uint16_t handle=0;std::uint32_t characterKey=0;
    if(!s.pointer(components+0x20,character) || !s.read(character+0x30,handle) ||
       !recordKey(s,base,0x71c1020,handle,characterKey) || characterKey!=1 ||
       !s.pointer(components+0x40,control) || (expectedControl && expectedControl!=control) ||
       !s.pointer(control+0x58,ownerLink) || !s.pointer(ownerLink,owner) || owner!=actor ||
       !s.pointer(control+0xb8,scene) || !s.pointer(scene+0x20,appearance) ||
       !s.pointer(appearance,actual) || !s.pointer(cc+0x193508,expected) || actual!=expected)return finish(false);
    std::uintptr_t equipment=0,slots=0,rows=0;std::uint32_t count=0;
    if(!s.pointer(components+0x38,equipment) || !s.pointer(equipment+0xa0,slots) ||
       !s.read(slots+0x10,count) || count==0 || count>40 || !s.pointer(slots+8,rows))return finish(false);
    bool found=false;
    for(std::uint32_t i=0;i<count;++i) {
        std::uint16_t itemHandle=0;std::uint32_t itemKey=0;
        if(!s.read(rows+i*0xd0ULL+8,itemHandle) || !recordKey(s,base,0x71c1080,itemHandle,itemKey))return finish(false);
        found=found || itemKey==1987201;
    }
    if(!found || !s.stable())return finish(false);
    result.actor=actor;result.control=control;return finish(true);
}
} // namespace boots204

