#include "inline_syscall.hpp"

// I'd recommend https://github.com/processhacker/phnt for these definitions
NTSTATUS NtAllocateVirtualMemory(HANDLE ProcessHandle, PVOID *BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);


void test(){
  void* allocation = nullptr;
  SIZE_T size      = 0x1000;
  NTSTATUS status  = INLINE_SYSCALL(NtAllocateVirtualMemory)((HANDLE)-1, &allocation, 0, &size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}