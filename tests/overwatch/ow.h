#pragma once
#include "link.h"

typedef float matrix_t[4][4];

bool OW_Init();
void OW_Unload();
void OW_InitX(float a, matrix_t& m);
void OW_InitY(float a, matrix_t& m);
void OW_Transform(Vector vec, matrix_t m, Vector* out);

extern bool s_ow_init;

typedef struct _CLIENT_ID
{
     PVOID UniqueProcess;
     PVOID UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _SYSTEM_THREAD_INFORMATION
{
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG WaitTime;
    PVOID StartAddress;
    CLIENT_ID ClientId;
    UINT Priority;
    LONG BasePriority;
    ULONG ContextSwitches;
    UINT ThreadState;
    UINT WaitReason;
} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef NTSTATUS (*NTAPI NtOpenThread_fn)(PHANDLE ThreadHandle, ACCESS_MASK AccessMask, POBJECT_ATTRIBUTES ObjectAttributes, PCLIENT_ID ClientId);
typedef NTSTATUS (*NTAPI NtClose_fn)(HANDLE Handle); // doesn't work
typedef NTSTATUS (*NTAPI NtQueryInformationThread_fn)(HANDLE ThreadHandle, LONG ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength, PULONG ReturnLength);
typedef NTSTATUS (*NTAPI NtQuerySystemInformation_fn)(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
typedef NTSTATUS (*NTAPI NtReadVirtualMemory_fn)(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, ULONG NumberOfBytesToRead, PULONG NumberOfBytesRead);
typedef NTSTATUS (*NTAPI NtWriteVirtualMemory_fn)(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, ULONG NumberOfBytesToWrite, PULONG NumberOfBytesWritten);
typedef NTSTATUS (*NTAPI NtProtectVirtualMemory_fn)(HANDLE ProcessHandle, PVOID* BaseAddress, PSIZE_T NumberOfBytesToProtect, ULONG NewAccessProtection, PULONG OldAccessProtection);
typedef NTSTATUS (*NTAPI NtQueryVirtualMemory_fn)(HANDLE ProcessHandle, PVOID BaseAddress, MEMORY_INFORMATION_CLASS MemoryInformationClass, PVOID MemoryInformation, SIZE_T MemoryInformationLength, PSIZE_T ReturnLength);
typedef NTSTATUS (*NTAPI NtContinue_fn)(PCONTEXT ThreadContext, BOOLEAN RaiseAlert);

extern NtOpenThread_fn sys_NtOpenThread;
extern NtQueryInformationThread_fn sys_NtQueryInformationThread;
extern NtQuerySystemInformation_fn sys_NtQuerySystemInformation;
extern NtReadVirtualMemory_fn sys_NtReadVirtualMemory;
extern NtWriteVirtualMemory_fn sys_NtWriteVirtualMemory;
extern NtProtectVirtualMemory_fn sys_NtProtectVirtualMemory;
extern NtQueryVirtualMemory_fn sys_NtQueryVirtualMemory;
extern NtContinue_fn sys_NtContinue;