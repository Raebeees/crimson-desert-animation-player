#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "condition_slots204.h"

namespace boots204 {
// Mechanism only. Address/module/CE detour identity verification is a separate
// mandatory prerequisite. Does not discover slots, change page protection,
// initialize native tables, patch instructions or load/unload any module.
class WindowsSlots204 {
    const std::array<Target,3> slots;
    bool configured=false;
    bool accessible(std::size_t index,bool write) const noexcept {
        if(!configured || index>=slots.size())return false;
        const auto address=slots[index];MEMORY_BASIC_INFORMATION info{};
        if(VirtualQuery(reinterpret_cast<const void*>(address),&info,sizeof(info))!=sizeof(info))return false;
        if(info.State!=MEM_COMMIT || (info.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;
        const auto protection=info.Protect&0xff;
        const bool writable=protection==PAGE_READWRITE || protection==PAGE_EXECUTE_READWRITE;
        const bool readable=writable || protection==PAGE_READONLY || protection==PAGE_EXECUTE_READ;
        if(write?!writable:!readable)return false;
        const auto start=reinterpret_cast<Target>(info.BaseAddress);
        return address>=start && address-start<=info.RegionSize &&
            sizeof(Target)<=info.RegionSize-(address-start);
    }
    static Exchange exchangeAt(Target address,Target expected,Target replacement) noexcept {
        __try {
            const auto previous=InterlockedCompareExchangePointer(
                reinterpret_cast<void* volatile*>(address),reinterpret_cast<void*>(replacement),
                reinterpret_cast<void*>(expected));
            return reinterpret_cast<Target>(previous)==expected?Exchange::swapped:Exchange::mismatch;
        } __except(EXCEPTION_EXECUTE_HANDLER) {return Exchange::unavailable;}
    }
public:
    explicit WindowsSlots204(std::array<Target,3> addresses):slots(addresses) {
        static_assert(sizeof(Target)==8,"x64 only");
        configured=true;
        for(std::size_t i=0;i<slots.size();++i) {
            if(slots[i]<0x10000 || slots[i]>=0x800000000000ULL-sizeof(Target) || slots[i]%alignof(Target))configured=false;
            for(std::size_t j=0;j<i;++j)if(slots[j]==slots[i])configured=false;
        }
    }
    bool read(std::size_t index,Target& output) const noexcept {
        output=0;if(!accessible(index,false))return false;
        Target value=0;SIZE_T count=0;
        if(!ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(slots[index]),&value,sizeof(value),&count)
           || count!=sizeof(value))return false;
        output=value;return true;
    }
    Exchange compare_exchange(std::size_t index,Target expected,Target replacement) const noexcept {
        if(!expected || !replacement || !accessible(index,true))return Exchange::unavailable;
        return exchangeAt(slots[index],expected,replacement);
    }
};
} // namespace boots204
