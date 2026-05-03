#pragma once

namespace rtp_methods{

  ALWAYSINLINE inline bool has_no_internet(){
    ul64 flags;
    I(InternetGetConnectedState)(&flags, 0);
    return !(flags & XOR32(INTERNET_CONNECTION_LAN | INTERNET_CONNECTION_MODEM));
  }

  ALWAYSINLINE inline bool is_test_signing_enabled(){
    SYSTEM_CODEINTEGRITY_INFORMATION sci;
    sci.Length = XOR32(sizeof(sci));

    if(SYSCALL(NtQuerySystemInformation)(XOR32(SystemCodeIntegrityInformation), (void*)&sci, XOR32(sizeof(sci)), nullptr) != XOR32(STATUS_SUCCESS))
      return true;

    return sci.CodeIntegrityOptions & XOR32(0x00000002) || sci.CodeIntegrityOptions & XOR32(0x00000080);
  }

  ALWAYSINLINE inline void free_memory(void* mem){
    SIZE_T size = 0;
    SYSCALL(NtFreeVirtualMemory)((HANDLE)-1, (void**)&mem, &size, XOR32(MEM_RELEASE));
  }

  ALWAYSINLINE inline bool flash_system_module(const i8* module_name){
    i8 file_path[MAX_PATH];
    I(GetWindowsDirectoryA)(file_path, MAX_PATH);
    I(wsprintfA)(file_path, XOR("%s\\System32\\%s"), file_path, module_name);

    HANDLE file = I(CreateFileA)(file_path, XOR32(GENERIC_READ), XOR32(FILE_SHARE_READ), nullptr, XOR32(OPEN_EXISTING), 0, nullptr);
    if(!IS_HANDLE_VALID(file))
      return false;

    HANDLE mapping = I(CreateFileMappingA)(file, nullptr, PAGE_READONLY | SEC_IMAGE, 0, 0, nullptr);
    if (!IS_HANDLE_VALID(mapping)){
      I(CloseHandle)(file);
      return false;
    }

    i8* clean_ntdll = (i8*)I(MapViewOfFile)(mapping, FILE_MAP_READ, 0, 0, 0);
    if (!clean_ntdll)
      return false;

    i8* loaded_ntdll = (i8*)utils::get_module_handle(HASH_RT(module_name));
    if(loaded_ntdll == nullptr){
      DBG("[!] %s is not loaded.\n", module_name);
      return false;
    }

    IMAGE_DOS_HEADER*     dos = (IMAGE_DOS_HEADER*)clean_ntdll;
    IMAGE_NT_HEADERS*     nt  = (IMAGE_NT_HEADERS*)(clean_ntdll + dos->e_lfanew);
    IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);

    for (i32 i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec) {
      if (memcmp(sec->Name, XOR(".text"), XOR32(5)) == 0) {
        ul64  size = sec->Misc.VirtualSize;
        void* src  = clean_ntdll + sec->VirtualAddress;
        void* dst  = loaded_ntdll + sec->VirtualAddress;

        ul64 old = 0;
        if(I(VirtualProtect)(dst, size, XOR32(PAGE_EXECUTE_READWRITE), &old)){
          memcpy(dst, src, size);
          I(VirtualProtect)(dst, size, old, &old);
          DBG("[!] Flashing %s's text section from file\n", module_name);
        }

        break;
      }
    }

    I(UnmapViewOfFile)(clean_ntdll);
    I(CloseHandle)(mapping);
    I(CloseHandle)(file);
    return true;
  }

  ALWAYSINLINE inline bool is_string_present(void* base_addr, uptr size, const wchar_t* str);
  ALWAYSINLINE inline bool is_string_present(void* base_addr, uptr size, const i8* str){
    u32 len = 0;
    while(str[len] != 0)
      len++;

    uptr region_start = (uptr)base_addr;
    uptr region_end   = region_start + size;

    for(uptr scan = (uptr)base_addr; scan <= region_end - len; scan++){
      if(memcmp((void*)scan, str, len) == 0)
        return true;
    }

    wchar_t wstr[1024];
    convert::str2wstr(str, wstr, sizeof(wstr));
    return is_string_present(base_addr, size, wstr);
  }

  ALWAYSINLINE inline bool is_string_present(void* base_addr, uptr size, const wchar_t* str){
    u32 len = 0;
    while(str[len] != 0)
      len++;

    len *= 2;
    uptr region_start = (uptr)base_addr;
    uptr region_end   = region_start + size;

    for(uptr scan = (uptr)base_addr; scan <= region_end - len; scan++){
      if(memcmp((void*)scan, str, len) == 0)
        return true;
    }

    return false;
  }

  ALWAYSINLINE inline bool detect_scylla_hide(void* base_addr, uptr size){
    return is_string_present(base_addr, size, XOR("HookLibraryx64.dll"))          ||
           is_string_present(base_addr, size, XOR("HookLibraryx86.dll"))           ||
           is_string_present(base_addr, size, XOR("Malware called ResumeThread")) ||
           is_string_present(base_addr, size, XOR("HookDllData")) ||
           is_string_present(base_addr, size, XOR("HookedGetLocalTime")) ||
           is_string_present(base_addr, size, XOR("HookedGetSystemTime")) ||
           is_string_present(base_addr, size, XOR("HookedGetTickCount64")) ||
           is_string_present(base_addr, size, XOR("HookedNtQueryInformationProcess")) ||
           is_string_present(base_addr, size, XOR("HookedNtCreateThreadEx")) ||
           is_string_present(base_addr, size, XOR("HookedNtCreateThread")) ||
           is_string_present(base_addr, size, XOR("HookedNtSetInformationProcess")) ||
           is_string_present(base_addr, size, XOR("HookedOutputDebugStringA")) ||
           is_string_present(base_addr, size, XOR("Failed to load ScyllaHide")) ||
           is_string_present(base_addr, size, XOR("SCYLLA_HIDE")) ||
           is_string_present(base_addr, size, XOR("HookedNativeCallInternal"));
  }

  ALWAYSINLINE inline bool detect_titan_hide(void* base_addr, uptr size){
    return is_string_present(base_addr, size, XOR("D:\a\\TitanHide\\TitanHide\\x64\\Release\\plugins\\TitanHide")) ||
           is_string_present(base_addr, size, XOR("TitanHideTE.dll")) ||
           is_string_present(base_addr, size, XOR("TitanDebuggingCallBack"));
  }

  ALWAYSINLINE inline i32 patch_function(u32 module_name, u32 func_name){
    DBG("[!] Patching: %X, %X\n", module_name, func_name);
    void* addr = utils::get_proc_address(utils::get_module_handle(module_name), func_name);
    if(addr == nullptr)
      return 0;

    if(*(u8*)addr == XOR32(0xC3))
      return 2;

    ul64 old = 0;
    if(utils::virtual_protect(addr, XOR32(1), XOR32(PAGE_EXECUTE_READWRITE), &old)){
      *(u8*)addr = XOR32(0xC3);
      DBG("[!] Patched: %X - %X\n", module_name, func_name);
      utils::virtual_protect(addr, XOR32(1), old, &old);
      return 1;
    }

    return 0;
  }
}