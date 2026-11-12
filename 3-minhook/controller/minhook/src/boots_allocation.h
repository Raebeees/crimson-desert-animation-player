/* Boots-local fallback only. Does not change the shared MinHook installation. */
#pragma once
#include <windows.h>
extern void BootsAllocationTrace(const char* message, DWORD a, DWORD b);
typedef PVOID (WINAPI *BootsVirtualAlloc2)(HANDLE,PVOID,SIZE_T,ULONG,ULONG,MEM_EXTENDED_PARAMETER*,ULONG);
static LPVOID BootsAllocateInRange(ULONG_PTR minimum, ULONG_PTR maximumStart, SIZE_T size, SIZE_T alignment)
{
    HMODULE kernel = GetModuleHandleW(L"KernelBase.dll");
    BootsVirtualAlloc2 allocate;
    MEM_ADDRESS_REQUIREMENTS requirements = {0};
    MEM_EXTENDED_PARAMETER parameter = {0};
    LPVOID result;
    ULONG_PTR endExclusive;
    if (!kernel) kernel = GetModuleHandleW(L"kernel32.dll");
    allocate = kernel ? (BootsVirtualAlloc2)GetProcAddress(kernel,"VirtualAlloc2") : NULL;
    if (!allocate) { BootsAllocationTrace("boots allocator: VirtualAlloc2 unavailable",ERROR_PROC_NOT_FOUND,0);return NULL; }
    if (!size || !alignment || (alignment & (alignment-1)) || minimum > maximumStart ||
        maximumStart > ~(ULONG_PTR)0 - size || minimum > ~(ULONG_PTR)0-(alignment-1)) {
        BootsAllocationTrace("boots allocator: invalid bounds",ERROR_INVALID_PARAMETER,0);return NULL;
    }
    minimum = (minimum+alignment-1)&~(alignment-1);
    /* HighestEndingAddress must be one less than allocation granularity.
       Round inward so the existing MinHook reachability limit never expands. */
    endExclusive=(maximumStart+size)&~(alignment-1);
    if (minimum>maximumStart || endExclusive<=minimum || size>endExclusive-minimum) return NULL;
    requirements.LowestStartingAddress=(PVOID)minimum;
    requirements.HighestEndingAddress=(PVOID)(endExclusive-1);
    requirements.Alignment=alignment;
    parameter.Type=MemExtendedParameterAddressRequirements;
    parameter.Pointer=&requirements;
    SetLastError(ERROR_SUCCESS);
    result=allocate(NULL,NULL,size,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE,&parameter,1);
    if (!result) { DWORD error=GetLastError();BootsAllocationTrace("boots allocator: VirtualAlloc2 failed Win32 error",error,0);return NULL; }
    if ((ULONG_PTR)result<minimum || (ULONG_PTR)result>maximumStart || ((ULONG_PTR)result&(alignment-1))) {
        VirtualFree(result,0,MEM_RELEASE);
        BootsAllocationTrace("boots allocator: rejected out-of-range allocation",ERROR_INVALID_ADDRESS,0);return NULL;
    }
    BootsAllocationTrace("boots allocator: bounded VirtualAlloc2 fallback succeeded",0,0);
    return result;
}

/* Read-only, bounded failure snapshot. Never changes allocation policy. */
static void BootsTraceAddress(const char* label, ULONG_PTR value) {
    BootsAllocationTrace(label,(DWORD)(value>>32),(DWORD)value);
}
static void BootsTraceFailedRange(ULONG_PTR origin, ULONG_PTR low, ULONG_PTR high) {
    ULONG_PTR cursor=low;
    DWORD regions=0,freeRegions=0,shown=0;
    BootsTraceAddress("origin high32/low32",origin);
    BootsTraceAddress("minimum high32/low32",low);
    BootsTraceAddress("maximum-start high32/low32",high);
    while(cursor<=high && regions<8192) {
        MEMORY_BASIC_INFORMATION mbi;
        SIZE_T result=VirtualQuery((LPCVOID)cursor,&mbi,sizeof(mbi));
        ULONG_PTR end;
        if(!result){BootsAllocationTrace("snapshot VirtualQuery failed",GetLastError(),regions);break;}
        ++regions;
        end=(ULONG_PTR)mbi.BaseAddress+mbi.RegionSize;
        if(mbi.State==MEM_FREE) {
            ++freeRegions;
            if(shown++<8){BootsTraceAddress("free base high32/low32",(ULONG_PTR)mbi.BaseAddress);BootsTraceAddress("free size high32/low32",mbi.RegionSize);}
        }
        if(end<=cursor){BootsAllocationTrace("snapshot nonprogress",regions,0);break;}
        cursor=end;
    }
    BootsAllocationTrace("snapshot regions/free regions",regions,freeRegions);
}
