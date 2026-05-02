#pragma once

typedef NTSTATUS (__stdcall* nt_query_system_information_fn)( i32, PVOID, ULONG, PULONG );
typedef NTSTATUS (__stdcall* nt_write_virtual_memory_fn)( HANDLE, PVOID, PVOID, ULONG, PULONG );
typedef NTSTATUS (__stdcall* nt_read_virtual_memory_fn)( HANDLE, PVOID, PVOID, ULONG, PULONG );
typedef NTSTATUS (__stdcall* nt_free_virtual_memory_fn)( void*, void**, unsigned long*, unsigned long );
typedef NTSTATUS (__stdcall* nt_allocate_virtual_memory_fn)( void*, void**, unsigned long, unsigned long*, unsigned long, unsigned long );
typedef PVOID(WINAPI* rtl_allocate_heap_fn)(PVOID, ULONG, SIZE_T);
typedef BOOLEAN(WINAPI* rtl_free_heap_fn)(PVOID, ULONG, PVOID);

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef enum _SYSTEM_INFORMATION_CLASS {
  SystemBasicInformation = 0,
  SystemPerformanceInformation = 2,
  SystemTimeOfDayInformation = 3,
  SystemProcessInformation = 5,
  SystemProcessorPerformanceInformation = 8,
  SystemInterruptInformation = 23,
  SystemExceptionInformation = 33,
  SystemRegistryQuotaInformation = 37,
  SystemLookasideInformation = 45,
  SystemPolicyInformation = 134,
} SYSTEM_INFORMATION_CLASS;

typedef LONG KPRIORITY;

typedef struct _SYSTEM_PROCESS_INFORMATION {
  ULONG          NextEntryOffset;
  ULONG          NumberOfThreads;
  BYTE           Reserved1[ 48 ];
  UNICODE_STRING ImageName;
  KPRIORITY      BasePriority;
  HANDLE         UniqueProcessId;
  PVOID          Reserved2;
  ULONG          HandleCount;
  ULONG          SessionId;
  PVOID          Reserved3;
  SIZE_T         PeakVirtualSize;
  SIZE_T         VirtualSize;
  ULONG          Reserved4;
  SIZE_T         PeakWorkingSetSize;
  SIZE_T         WorkingSetSize;
  PVOID          Reserved5;
  SIZE_T         QuotaPagedPoolUsage;
  PVOID          Reserved6;
  SIZE_T         QuotaNonPagedPoolUsage;
  SIZE_T         PagefileUsage;
  SIZE_T         PeakPagefileUsage;
  SIZE_T         PrivatePageCount;
  LARGE_INTEGER  Reserved7[ 6 ];
}                SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

struct __PEB_LDR_DATA {
  ULONG      Length;
  UCHAR      Initialized;
  PVOID      SsHandle;
  LIST_ENTRY InLoadOrderModuleList;
  LIST_ENTRY InMemoryOrderModuleList;
  LIST_ENTRY InInitializationOrderModuleList;
};

typedef struct ___PEB_LDR_DATA {
  BYTE       Reserved1[ 8 ];
  PVOID      Reserved2[ 3 ];
  LIST_ENTRY InMemoryOrderModuleList;
}            ___PEB_LDR_DATA, *__PPEB_LDR_DATA;

struct __PEB {
  UCHAR           InheritedAddressSpace;
  UCHAR           ReadImageFileExecOptions;
  UCHAR           BeingDebugged;
  UCHAR           BitField;
  PVOID           Mutant;
  PVOID           ImageBaseAddress;
  __PPEB_LDR_DATA Ldr;
  PVOID           ProcessParameters;
  PVOID           SubSystemData;
  PVOID           ProcessHeap;
  PVOID           FastPebLock;
  PVOID           AtlThunkSListPtr;
  PVOID           IFEOKey;
  PVOID           CrossProcessFlags;
  PVOID           KernelCallbackTable;
  ULONG           SystemReserved;
  ULONG           AtlThunkSListPtr32;
  PVOID           ApiSetMap;
};

struct __LDR_DATA_TABLE_ENTRY {
  LIST_ENTRY     InLoadOrderLinks;
  LIST_ENTRY     InMemoryOrderLinks;
  LIST_ENTRY     InInitializationOrderLinks;
  PVOID          DllBase;
  PVOID          EntryPoint;
  ULONG          SizeOfImage;
  UNICODE_STRING FullDllName;
  UNICODE_STRING BaseDllName;
  ULONG          Flags;
  USHORT         LoadCount;
  USHORT         Tlf_sindex;
  LIST_ENTRY     HashLinks;
  ULONG          TimeDateStamp;
};

typedef struct _KSYSTEM_TIME {
  ULONG LowPart;
  LONG  High1Time;
  LONG  High2Time;
}       KSYSTEM_TIME, *PKSYSTEM_TIME;

typedef enum _NT_PRODUCT_TYPE {
  NtProductWinNt = 1,
  NtProductLanManNt = 2,
  NtProductServer = 3
} NT_PRODUCT_TYPE;

typedef enum _ALTERNATIVE_ARCHITECTURE_TYPE {
  StandardDesign = 0,
  NEC98x86 = 1,
  EndAlternatives = 2
} ALTERNATIVE_ARCHITECTURE_TYPE;

typedef struct _KUSER_SHARED_DATA {
  ULONG                         TickCountLowDeprecated;
  ULONG                         TickCountMultiplier;
  KSYSTEM_TIME                  InterruptTime;
  KSYSTEM_TIME                  SystemTime;
  KSYSTEM_TIME                  TimeZoneBias;
  WORD                          ImageNumberLow;
  WORD                          ImageNumberHigh;
  WCHAR                         NtSystemRoot[ 260 ];
  ULONG                         MaxStackTraceDepth;
  ULONG                         CryptoExponent;
  ULONG                         TimeZoneId;
  ULONG                         LargePageMinimum;
  ULONG                         Reserved2[ 7 ];
  NT_PRODUCT_TYPE               NtProductType;
  UCHAR                         ProductTypeIsValid;
  ULONG                         NtMajorVersion;
  ULONG                         NtMinorVersion;
  UCHAR                         ProcessorFeatures[ 64 ];
  ULONG                         Reserved1;
  ULONG                         Reserved3;
  ULONG                         TimeSlip;
  ALTERNATIVE_ARCHITECTURE_TYPE AlternativeArchitecture;
  LARGE_INTEGER                 SystemExpirationDate;
  ULONG                         SuiteMask;
  UCHAR                         KdDebuggerEnabled;
  UCHAR                         NXSupportPolicy;
  ULONG                         ActiveConsoleId;
  ULONG                         DismountCount;
  ULONG                         ComPlusPackage;
  ULONG                         LastSystemRITEventTickCount;
  ULONG                         NumberOfPhysicalPages;
  UCHAR                         SafeBootMode;
  ULONG                         SharedDataFlags;
  ULONG DbgErrorPortPresent : 1;
  ULONG DbgElevationEnabled : 1;
  ULONG DbgVirtEnabled : 1;
  ULONG DbgInstallerDetectEnabled : 1;
  ULONG SystemDllRelocated : 1;
  ULONG SpareBits : 27;
  UINT64 TestRetInstruction;
  ULONG  SystemCall;
  ULONG  SystemCallReturn;
  UINT64 SystemCallPad[ 3 ];

  union {
    KSYSTEM_TIME TickCount;
    UINT64       TickCountQuad;
  };

  ULONG Cookie;
  INT64 ConsoleSessionForegroundProcessId;
  ULONG Wow64SharedInformation[ 16 ];
  WORD  UserModeGlobalLogger[ 8 ];
  ULONG HeapTracingPid[ 2 ];
  ULONG CritSecTracingPid[ 2 ];
  ULONG ImageFileExecutionOptions;

  union {
    UINT64 AffinityPad;
    ULONG  ActiveProcessorAffinity;
  };

  UINT64 InterruptTimeBias;
}        KUSER_SHARED_DATA, *PKUSER_SHARED_DATA;