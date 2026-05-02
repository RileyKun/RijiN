#include <winternl.h>
#include <ntstatus.h>
#include <windows.h>
#include <iostream>

// This header contains the initialization function.
// If you already initialized, inline_syscall.hpp contains all you need.
#include "in_memory_init.hpp"
#include "black.h"

// I'd recommend https://github.com/processhacker/phnt for these definitions
NTSTATUS NtAllocateVirtualMemory(HANDLE ProcessHandle, PVOID *BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);

int main() {
    // Needs to be called once at startup before INLINE_SYSCALL is used.
    jm::init_syscalls_list();

    // Usage of the main macro INLINE_SYSCALL
    void* allocation = nullptr;
    SIZE_T size      = 0x1000;
    NTSTATUS status  = INLINE_SYSCALL(NtAllocateVirtualMemory)((HANDLE)-1, &allocation, 0, &size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    return 0;
}