#pragma once
#include <array>
#include <cstdint>
#include <cstring>
namespace boots204 {
// Supply the hash-pinned entry prefix, verified module-relative address,
// captured MinHook trampoline, and intended wrapper separately for each hook.
struct HookRoute204 {
    std::uintptr_t entry, trampoline, wrapper;
    std::array<unsigned char, 32> prefix;
    std::size_t size;
};
template<class Reader> struct HookRoutes204 {
    Reader& reader;
    std::array<HookRoute204,4> routes;
    bool inspect(void *const targets[4], bool enabled) const noexcept {
        for(std::size_t i=0;i<routes.size();++i){
            const auto& r=routes[i];
            if(!r.entry || !r.trampoline || !r.wrapper || r.entry!=reinterpret_cast<std::uintptr_t>(targets[i])
               || r.size<5 || r.size>r.prefix.size()) return false;
            decltype(r.prefix) bytes{};
            if(!reader.read(r.entry,bytes.data(),r.size)) return false;
            const std::size_t skip=enabled?5:0;
            if(std::memcmp(bytes.data()+skip,r.prefix.data()+skip,r.size-skip)) return false;
            if(!enabled) continue;
            if(bytes[0]!=0xe9) return false;
            int32_t displacement;
            std::memcpy(&displacement,bytes.data()+1,4);
            const auto relay=r.entry+5+static_cast<std::uintptr_t>(displacement);
            // x64 MinHook emits the 14-byte relay inside the original 64-byte slot.
            if(relay<r.trampoline || relay-r.trampoline>64-14) return false;
            unsigned char jump[14];
            constexpr unsigned char indirect[6]={0xff,0x25,0,0,0,0};
            if(!reader.read(relay,jump,sizeof(jump)) || std::memcmp(jump,indirect,6)) return false;
            std::uintptr_t destination;
            static_assert(sizeof(destination)==8);
            std::memcpy(&destination,jump+6,8);
            if(destination!=r.wrapper) return false;
        }
        return true;
    }
    bool before(void *const targets[4]) const noexcept {return inspect(targets,false);}
    bool owned(void *const targets[4]) const noexcept {return inspect(targets,true);}
};
}
