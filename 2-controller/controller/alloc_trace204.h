#pragma once
#include <atomic>
#include <cstring>
struct AllocTrace204 {
    std::atomic<unsigned> kind{0},winError{0},freeRegions{0};
    void observe(const char* msg,unsigned a,unsigned b) noexcept {
        if(!strcmp(msg,"boots allocator: hook bookkeeping allocation failed"))kind.store(1);
        else if(!strcmp(msg,"boots allocator: executable trampoline allocation failed"))kind.store(2);
        else if(!strcmp(msg,"boots allocator: VirtualAlloc2 failed Win32 error") ||
                !strcmp(msg,"boots allocator: VirtualAlloc2 unavailable") ||
                !strcmp(msg,"boots allocator: invalid bounds") ||
                !strcmp(msg,"boots allocator: rejected out-of-range allocation"))winError.store(a);
        else if(!strcmp(msg,"snapshot regions/free regions"))freeRegions.store(b);
    }
};
