#include <winternl.h>
#include <ntstatus.h>
#include <windows.h>
#include <iostream>
#include "link.h"
#include "in_memory_init.hpp"

// I'd recommend https://github.com/processhacker/phnt for these definitions
NTSTATUS NtAllocateVirtualMemory(HANDLE ProcessHandle, PVOID *BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);
NTSTATUS NtFreeVirtualMemory(
  HANDLE  ProcessHandle,
  PVOID   *BaseAddress,
  PSIZE_T RegionSize,
  ULONG   FreeType
);

HANDLE NtGetCurrentProcess();

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateThreadEx(
    _Out_ PHANDLE ThreadHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
    _In_ HANDLE ProcessHandle,
    _In_ PVOID StartRoutine, // PUSER_THREAD_START_ROUTINE
    _In_opt_ PVOID Argument,
    _In_ ULONG CreateFlags, // THREAD_CREATE_FLAGS_*
    _In_ SIZE_T ZeroBits,
    _In_ SIZE_T StackSize,
    _In_ SIZE_T MaximumStackSize,
    _In_opt_ PPS_ATTRIBUTE_LIST AttributeList
    );

NTSYSAPI
NTSTATUS
NTAPI
NtDelayExecution(
  IN BOOLEAN              Alertable,
  IN PLARGE_INTEGER       DelayInterval OPTIONAL  );

NTSTATUS NtSuspendProcess(HANDLE handle);

template<std::uint32_t Hash>
JM_INLINE_SYSCALL_FORCEINLINE u32 syscall_present_impl() {
    ::jm::detail::exports_directory exports(static_cast<const char*>(::jm::detail::ntdll_base()));
    for(auto i = exports.size();; --i) {
        if(::jm::hash(exports.name(i)) == Hash)
            return *reinterpret_cast<const std::int32_t*>(exports.address(i) + 4);
    }
    return 0;
}

#define GET_SYSCALL_ID(x) (syscall_present_impl<::jm::hash(#x)>())
#define SYSCALL(x) INLINE_SYSCALL_MANUAL(x, GET_SYSCALL_ID(x))

NTSTATUS test(void* p){
  printf("[0] SUCCESS\n");
  return STATUS_SUCCESS;
}

bool run_test(bool exp){
  static i32 test_num = 1;

  printf("TEST %i: %s\n", test_num, exp ? "OK" : "FAIL");

  test_num++;

  return exp;
}

int main(int argc, const char *argv[]){
    jm::init_syscalls_list();

    // Syscall tests
    run_test(GET_SYSCALL_ID(NtCreateThreadEx));
    run_test(GET_SYSCALL_ID(NtQuerySystemInformation));
    run_test(GET_SYSCALL_ID(NtAllocateVirtualMemory));
    run_test(GET_SYSCALL_ID(NtFreeVirtualMemory));
    run_test(GET_SYSCALL_ID(NtDelayExecution));

    // Create thread test
    HANDLE handle;
    run_test(SYSCALL(NtCreateThreadEx)(&handle, MAXIMUM_ALLOWED, nullptr, (HANDLE)-1, (void*)test, (void*)1, 0x40, 0, 0, 0, 0) == STATUS_SUCCESS);

    system("pause");
    return 0;
}