#pragma once

#if defined(INTERNAL_CHEAT)
EXTERN void* __cdecl malloc( uptr size );
EXTERN void  __cdecl free( void* buffer );
EXTERN void* __cdecl realloc( void* buffer, uptr size );
EXTERN void* __cdecl calloc( uptr count, uptr size );

void* __cdecl operator new( uptr size );
void* __cdecl operator new[]( uptr size );
void __cdecl operator delete( void* buffer );
void __cdecl operator delete[]( void* buffer );
void __cdecl operator delete( void* buffer, uptr size );

using atexit_func = void(__cdecl*)();
#endif


// error: attempt to use poisoned "X", I've done this so you can use one of our rebuilt functions that's more internal.
// aka either SYSCALL, Uses the PEB, or just rebuilds how windows does the API call.
#if !defined(ALLOW_INSECURE_WINAPI)
// #pragma GCC poison GetCurrentProcess IsDebuggerPresent WriteProcessMemory ReadProcessMemory GetModuleHandleA GetModuleHandleW GetProcAddress GetCommandLineA GetCommandLineW
#endif

namespace utils{

  static ALWAYSINLINE void* get_module_handle(u32 module_hash){
    // Fetch the peb
#if defined(__x86_64__)
  static __PEB* peb = (__PEB*)__readgsqword(XOR32(0x60));
#else
  static __PEB* peb = (__PEB*)__readfsdword(XOR32(0x30));
#endif

    if(peb == nullptr)
      return nullptr;

    // Get ldr from peb
#if defined(__x86_64__)
    __PPEB_LDR_DATA* ldr = *(__PPEB_LDR_DATA**)((uptr)peb + XOR32(0x18));
#else
    __PPEB_LDR_DATA* ldr = *(__PPEB_LDR_DATA**)((uptr)peb + XOR32(0x0C));
#endif

    if(ldr == nullptr)
      return nullptr;

    // Fetch module list
#if defined(__x86_64__)
    LDR_DATA_TABLE_ENTRY* modules = *(LDR_DATA_TABLE_ENTRY**)((uptr)ldr + XOR32(0x10));
#else
    LDR_DATA_TABLE_ENTRY* modules = *(LDR_DATA_TABLE_ENTRY**)((uptr)ldr + XOR32(0x0C));
#endif

    if(modules == nullptr)
      return nullptr;

    if(!module_hash)
      return peb->ImageBaseAddress;
    
    while(modules->DllBase != nullptr){
      if(FNV1A_WSTR_RT(modules->BaseDllName.Buffer, true) == module_hash)
        return modules->DllBase;

      modules = (LDR_DATA_TABLE_ENTRY*)modules->InLoadOrderLinks.Flink;
    }

    return nullptr;
  }

  static ALWAYSINLINE const wchar_t* get_module_path_from_addr(void* addr) {
#if defined(__x86_64__)
    __PEB* peb = (__PEB*)__readgsqword(XOR32(0x60));
    __PPEB_LDR_DATA* ldr = *(__PPEB_LDR_DATA**)((uptr)peb + XOR32(0x18));
    LIST_ENTRY* list = (LIST_ENTRY*)((uptr)ldr + XOR32(0x20));
#else
    __PEB* peb = (__PEB*)__readfsdword(XOR32(0x30));
    __PPEB_LDR_DATA* ldr = *(__PPEB_LDR_DATA**)((uptr)peb + XOR32(0x0C));
    LIST_ENTRY* list = (LIST_ENTRY*)((uptr)ldr + XOR32(0x14));
#endif

    if (!peb || !ldr || !list)
        return nullptr;

    LIST_ENTRY* head = list;
    LIST_ENTRY* curr = list->Flink;

    while (curr && curr != head) {
        LDR_DATA_TABLE_ENTRY* entry = (LDR_DATA_TABLE_ENTRY*)((uptr)curr - offsetof(LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks));

        uptr base = (uptr)entry->DllBase;
        uptr end  = base + entry->SizeOfImage;

        if(entry->FullDllName.Length != 0){
          if ((uptr)addr >= base && (uptr)addr < end)
              return entry->FullDllName.Buffer;
        }

        curr = curr->Flink;
    }

    return nullptr;
  }

  static ALWAYSINLINE const wchar_t* get_module_name(void* addr) {
#if defined(__x86_64__)
    __PEB* peb = (__PEB*)__readgsqword(XOR32(0x60));
    __PPEB_LDR_DATA* ldr = *(__PPEB_LDR_DATA**)((uptr)peb + XOR32(0x18));
    LIST_ENTRY* list = (LIST_ENTRY*)((uptr)ldr + XOR32(0x20));
#else
    __PEB* peb = (__PEB*)__readfsdword(XOR32(0x30));
    __PPEB_LDR_DATA* ldr = *(__PPEB_LDR_DATA**)((uptr)peb + XOR32(0x0C));
    LIST_ENTRY* list = (LIST_ENTRY*)((uptr)ldr + XOR32(0x14));
#endif

    if (!peb || !ldr || !list)
        return nullptr;

    LIST_ENTRY* head = list;
    LIST_ENTRY* curr = list->Flink;

    while (curr && curr != head) {
        LDR_DATA_TABLE_ENTRY* entry = (LDR_DATA_TABLE_ENTRY*)((uptr)curr - offsetof(LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks));

        uptr base = (uptr)entry->DllBase;
        uptr end  = base + entry->SizeOfImage;

        if(entry->BaseDllName.Length != 0){
          if ((uptr)addr >= base && (uptr)addr < end)
              return entry->BaseDllName.Buffer;
        }

        curr = curr->Flink;
    }

    return nullptr;
  }

  static ALWAYSINLINE void* get_cur_process_base_addr(){
    return get_module_handle(0);
  }

  static ALWAYSINLINE void* get_proc_address(void* module, u32 function_hash){
    IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)module;

    if(dos_header == nullptr || dos_header->e_magic != XOR32(IMAGE_DOS_SIGNATURE))
      return nullptr;

    IMAGE_NT_HEADERS* nt_headers = (IMAGE_NT_HEADERS*)((uptr)dos_header + dos_header->e_lfanew);

    if(nt_headers == nullptr || nt_headers->Signature != XOR32(IMAGE_NT_SIGNATURE))
      return nullptr;

    IMAGE_DATA_DIRECTORY* image_data_directory = &nt_headers->OptionalHeader.DataDirectory[XOR32(IMAGE_DIRECTORY_ENTRY_EXPORT)];

    if(image_data_directory == nullptr || image_data_directory->VirtualAddress == 0)
      return nullptr;

    IMAGE_EXPORT_DIRECTORY* image_export_directory = (IMAGE_EXPORT_DIRECTORY*)((uptr)module + image_data_directory->VirtualAddress);

    if(image_export_directory == nullptr)
      return nullptr;

    u32* names     = (u32*)((uptr)module + image_export_directory->AddressOfNames);
    u32* functions = (u32*)((uptr)module + image_export_directory->AddressOfFunctions);
    u16* ordinals  = (u16*)((uptr)module + image_export_directory->AddressOfNameOrdinals);

    if(names == nullptr || functions == nullptr || ordinals == nullptr)
      return nullptr;

    for(u32 i = 0; i < image_export_directory->NumberOfNames; i++){
      i8* entry_name = (i8*)((uptr)module + names[i]);

      if(entry_name == nullptr)
        continue;

      if(FNV1A_STR_RT(entry_name) == function_hash || FNV1A_STR_RT(entry_name, true) == function_hash)
        return (void*)((uptr)module + functions[ordinals[i]]);
    }

    return nullptr;
  }

  static ALWAYSINLINE bool is_debugger_present(){
    #if defined(__x86_64__)
      __PEB* peb = (__PEB*)__readgsqword(XOR32(0x60));
    #else
      __PEB* peb = (__PEB*)__readfsdword(XOR32(0x30));
    #endif

    return peb->BeingDebugged;
  }

  static ALWAYSINLINE HANDLE get_current_process(){
    return (HANDLE)-1;
  }

  static ALWAYSINLINE wchar_t* get_command_line(){
    #if defined(__x86_64__)
      __PEB* peb = (__PEB*)__readgsqword(XOR32(0x60));
    #else
       __PEB* peb = (__PEB*)__readfsdword(XOR32(0x30));
    #endif

    if(peb->ProcessParameters == nullptr)
      return L"";

    return peb->ProcessParameters->CommandLine.Buffer;
  }

  static ALWAYSINLINE wchar_t* get_image_path_name(){
     #if defined(__x86_64__)
      __PEB* peb = (__PEB*)__readgsqword(XOR32(0x60));
    #else
      __PEB* peb = (__PEB*)__readfsdword(XOR32(0x30));
    #endif

    if(peb->ProcessParameters == nullptr)
      return L"";

    return peb->ProcessParameters->ImagePathName.Buffer;
  }

  static ALWAYSINLINE wchar_t* get_current_directory(){
    #if defined(__x86_64__)
      __PEB* peb = (__PEB*)__readgsqword(XOR32(0x60));
    #else
      __PEB* peb = (__PEB*)__readfsdword(XOR32(0x30));
    #endif

    if(peb->ProcessParameters == nullptr)
      return L"";

    return peb->ProcessParameters->CurrentDirectoryPath.Buffer != nullptr ? peb->ProcessParameters->CurrentDirectoryPath.Buffer : L"";
  }

  static ALWAYSINLINE wchar_t* get_window_title(){
    #if defined(__x86_64__)
       __PEB* peb = (__PEB*)__readgsqword(XOR32(0x60));
    #else
       __PEB* peb = (__PEB*)__readfsdword(XOR32(0x30));
    #endif

    if(peb->ProcessParameters == nullptr)
      return L"";

    return peb->ProcessParameters->WindowTitle.Buffer;
  }

  static ALWAYSINLINE uptr get_window_flags(){
    #if defined(__x86_64__)
      __PEB* peb = (__PEB*)__readgsqword(XOR32(0x60));
    #else
      __PEB* peb = (__PEB*)__readfsdword(XOR32(0x30));
    #endif

    if(peb->ProcessParameters == nullptr)
      return 0;

    return peb->ProcessParameters->WindowFlags;
  }

  static ALWAYSINLINE uptr get_show_window_flags(){
    #if defined(__x86_64__)
      __PEB* peb = (__PEB*)__readgsqword(XOR32(0x60));
    #else
      __PEB* peb = (__PEB*)__readfsdword(XOR32(0x30));
    #endif

    if(peb->ProcessParameters == nullptr)
      return 0;

    return peb->ProcessParameters->ShowWindowFlags;
  }

  static ALWAYSINLINE bool system(std::wstring str, uptr* error = nullptr){
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));

    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    #if !defined(DEV_MODE)
      si.wShowWindow = SW_HIDE;
    #endif

    wchar_t buffer[2048];
    memset(buffer, 0, sizeof(buffer));

    wsprintfW(buffer, WXOR(L"cmd /c %ls"), str.c_str());
    if(!I(CreateProcessW)(nullptr, buffer, nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi)){
      if(error != nullptr)
        *error = I(GetLastError)();

      return false;
    }

    I(WaitForSingleObject)(pi.hProcess, XOR32(30000));

    I(CloseHandle)(pi.hProcess);
    I(CloseHandle)(pi.hThread);

    return true;
  }

  static ALWAYSINLINE bool virtual_protect_ex(HANDLE process, void* address, uptr size, ULONG new_protection, PULONG old_protection, uptr* last_error = nullptr){
    #if defined(SYSCALLS_PRESENT)
    if(last_error != nullptr)
      *last_error = 0;

    // Same thing here.
    if(address == nullptr || old_protection == nullptr || size == 0){
      if(last_error != nullptr)
        *last_error = (uptr)STATUS_INVALID_PARAMETER;

      //assert(false && "Invalid parameters for virtual_protect_ex")
      return false;
    }

    NTSTATUS status = SYSCALL(NtProtectVirtualMemory)(process, &address, (PSIZE_T)&size, new_protection, old_protection);
    if(last_error != nullptr)
      *last_error = (uptr)status;

    return status == STATUS_SUCCESS;
    #else
    assert(false && "do not use virtual_protect in what is classed as INTERNAL_CHEAT!!!")
    return false;
    #endif
  }

  static ALWAYSINLINE bool virtual_protect(void* address, uptr size, ULONG new_protection, PLONG old_protection, uptr* last_error = nullptr){
    return virtual_protect_ex(utils::get_current_process(), address, size, new_protection, old_protection, last_error);
  }

  static ALWAYSINLINE bool write_virtual_memory(HANDLE process, void* address, void* data, ul64 len){
    #if defined(SYSCALLS_PRESENT)
      return SYSCALL(NtWriteVirtualMemory)(process, (void*)address, (void*)data, len, 0) == 0;
    #else
      return I(WriteProcessMemory)(process, address, data, len, 0);
    #endif
  }

  static ALWAYSINLINE bool query_virtual_memory_ex(HANDLE process, void* address, void* mem_info, SIZE_T mem_info_len, NTSTATUS* status = 0){
    #if defined(SYSCALLS_PRESENT)
      NTSTATUS result = SYSCALL(NtQueryVirtualMemory)(
        (void*)process,
        (void*)address,
        (_MEMORY_INFORMATION_CLASS)0,
        (void*)mem_info,
        (SIZE_T)mem_info_len,
        (SIZE_T*)nullptr
      );

      if(status != nullptr)
        *status = result;

      return result == 0;
    #else
      return I(VirtualQueryEx)(process, address, mem_info, mem_info_len) != 0;
    #endif
  }

  static ALWAYSINLINE bool query_virtual_memory(void* address, void* mem_info, ul64 mem_info_len, NTSTATUS* status = 0){
    return query_virtual_memory_ex(utils::get_current_process(), address, mem_info, mem_info_len);
  }
}