#pragma once

#if defined(HDE_DISASM) && !defined(NO_SECURITY)
  #define IS_IMPORT_BREAKPOINT(module, name) utils::has_breakpoint((uptr*)utils::get_proc_address(utils::get_module_handle(HASH(module)), HASH(name)))
  #define IS_BREAKPOINTED(addr) utils::has_breakpoint((uptr*)addr)
  
  #define IS_IMPORT_HOOKED(module, name) utils::is_hooked((uptr*)utils::get_proc_address(utils::get_module_handle(HASH(module)), HASH(name)), false)
  #define IS_HOOKED(addr) utils::is_hooked((uptr*)addr, true)
#else
  #define IS_IMPORT_BREAKPOINT(name) assert(false && "im bp check not active here");
  #define IS_BREAKPOINTED(addr) assert(false && "bp check not active here");

  #define IS_IMPORT_HOOKED(name) assert(false && "im hk check not active here");
  #define IS_HOOKED(addr) assert(false && "hk check not active here");
#endif

enum is_memory_flags{
  MEMORY_EXECUTE = 1,
  MEMORY_WRITE   = 2,
  MEMORY_READ    = 3
};

enum get_pid_filters{
  GET_PID_RETURN_HIGHEST_MEMORY = (1 << 1), // If there is multiple processes it'll return the process pid with the highest memory.
  GET_PID_IGNORE_CHILD_PROCESS_LOGIC = (1 << 2), // As of Mar 1st, 2025, this will ignore processes with a size less than 75 MB.
}; 

#define DECLARE_NT_QUERY_SYSTEM_INFO static nt_query_system_information_fn nt_query_system_information = (nt_query_system_information_fn)(utils::get_proc_address(utils::get_module_handle(HASH("ntdll.dll")), HASH("NtQuerySystemInformation")));

#if defined(DEV_MODE)
  #if defined(__x86_64__)
    #define STACK_CHECK_START  \
      uptr rsp_start = 0; \
      uptr rsp_end   = 0; \
      volatile uptr __c = 0xCAFEBABEDEADBEEF; \
      __asm__ volatile("mov %%rsp, %0" : "=r"(rsp_start));

    #define STACK_CHECK_END \
    do { \
      __asm__ volatile("mov %%rsp, %0" : "=r"(rsp_end)); \
      if (rsp_start != rsp_end) { \
        assert(false && "stack corruption RSP mismatch after call! Check parameters and call conv"); \
      } \
      else if (__c != 0xCAFEBABEDEADBEEF) { \
        assert(false && "stack corruption a value prior to call was modified, which suggests improper stack alignment."); \
      } \
    } while (0)
  #else
    #define STACK_CHECK_START  \
      uptr esp_start = 0; \
      uptr esp_end   = 0; \
      volatile uptr __c = 0xDEADBEEF; \
      __asm__ volatile("mov %%esp, %0" : "=r"(esp_start));

    #define STACK_CHECK_END \
    do { \
      __asm__ volatile("mov %%esp, %0" : "=r"(esp_end)); \
      if (esp_start != esp_end) { \
        assert(false && "stack corruption ESP mismatch after call! Check parameters and call conv"); \
        DBG("[!] %X != %X\n", esp_start, esp_end); \
      } \
      else if (__c != 0xDEADBEEF) { \
        assert(false && "stack corruption a value prior to call was modified, which suggests improper stack alignment."); \
        DBG("[!] 0x%X != 0xDEADBEEF\n", __c); \
      } \
    } while (0)
  #endif
#else
  #define STACK_CHECK_START
  #define STACK_CHECK_END
#endif

namespace utils{
  #if defined(HDE_DISASM) && !defined(NO_SECURITY)
    bool has_breakpoint(uptr* addr);
    bool is_hooked(uptr* addr, bool never_possible = false);
  #endif

  static bool is_memory_valid(void* p, u32 memory_type, uword* last_error = nullptr){
    if(p == nullptr)
      return false;

    MEMORY_BASIC_INFORMATION mbi = {0};
    if(!I(VirtualQuery)(p, &mbi, sizeof(mbi))){
      if(last_error != nullptr)
        *last_error = I(GetLastError)();

      return false;
    }

    if(mbi.State != MEM_COMMIT || mbi.State == PAGE_NOACCESS)
      return false;

    if(mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
      return false;

    switch(memory_type){
      default:
      { 
        assert(false && "is_memory_valid received an invalid memory_type");
        return false;
      }
      case MEMORY_EXECUTE: return mbi.Protect & PAGE_EXECUTE || mbi.Protect & PAGE_EXECUTE_READ || mbi.Protect & PAGE_EXECUTE_READWRITE || mbi.Protect & PAGE_EXECUTE_WRITECOPY;
      case MEMORY_WRITE: return mbi.Protect & PAGE_EXECUTE_READWRITE || mbi.Protect & PAGE_EXECUTE_WRITECOPY || mbi.Protect & PAGE_READWRITE || mbi.Protect & PAGE_WRITECOPY;
      case MEMORY_READ: return mbi.Protect & PAGE_READONLY || mbi.Protect & PAGE_READWRITE || mbi.Protect & PAGE_EXECUTE_READWRITE || mbi.Protect & PAGE_EXECUTE_READ;
    }
    
    return false;
  }

  ALWAYSINLINE inline uptr* get_virtual_address(const void* vtable, u32 index){
    if(vtable == nullptr){
      assert(false && "get_virtual_address: got an nullptr table");
      return nullptr;
    }

    if(index > 1024){
      DBG("[-] ERROR: %i\n", index);
      assert(false && "get_virtual_address: index too large");
      return nullptr;
    }

    uptr* p = nullptr;

    #if defined(__x86_64__)
      uptr vt = *(uptr*)(vtable);
      p = *(uptr**)((uptr)vt + (u64)index * (u64)8);
    #else
      p = (*(uptr**)vtable)[index];
    #endif

    // This will check if the address points to valid memory that can be executed, if not we error out.
    #if defined(DEV_MODE)
      #if defined(TEST_MEMORY)
        if(!is_memory_valid((void*)p, MEMORY_EXECUTE)){
          assert(false && "get_virtual_address: Calculated virtual function address does not lead to valid executable memory.");
          return 0;
        }
      #endif
    #endif

    return p;
  }

#if !defined(DISABLE_CALL_CONVENTIONS)
  template<u32 index, typename ret, typename... a>
  ALWAYSINLINE ret virtual_call(void* vtable, a...args){
    #if !defined(__x86_64__)
      return ((ret(__thiscall*)(void*, a...))(get_virtual_address(vtable, index)))(vtable, args...);
    #else
      return ((ret(__fastcall*)(void*, a...))(get_virtual_address(vtable, index)))(vtable, args...);
    #endif
  }

  template<typename ret, typename... a>
  ALWAYSINLINE ret internal_virtual_call(u32 index, void* vtable, a...args){
    #if !defined(__x86_64__)
      return ((ret(__thiscall*)(void*, a...))(get_virtual_address(vtable, index)))(vtable, args...);
    #else
      return ((ret(__fastcall*)(void*, a...))(get_virtual_address(vtable, index)))(vtable, args...);
    #endif
  }

  template<typename ret, typename... a>
  ALWAYSINLINE ret call_stdcall(void* address, a...args){
    #if defined(DEV_MODE)
      #if defined(TEST_MEMORY)
        if(!is_memory_valid(address, MEMORY_EXECUTE)){
          assert(false && "call_fastcall64_raw invalid address");
          return 0xDEADBEEF;
        }
      #endif
    #endif

    assert(address != nullptr);

    return ((ret(__stdcall*)(a...))address)(args...);
  }

  #if defined(__x86_64__)
    template<u32 index, typename ret, typename... a>
    ALWAYSINLINE ret virtual_call64(void* vtable, a...args){ // We don't really need this one, but we'll keep it for now.
      return ((ret(__fastcall*)(void*, a...))(get_virtual_address(vtable, index)))(vtable, args...);
    }
  
    template<typename ret, typename... a>
    ALWAYSINLINE ret call_fastcall64(void* address, void* rcx, a...args){
    #if defined(DEV_MODE)
      #if defined(TEST_MEMORY)
        f(!is_memory_valid(address, MEMORY_EXECUTE)){
         assert(false && "call_fastcall64_raw invalid address");
         return 0xDEADBEEF;
        
      #endif
    #endif
    assert(rcx != nullptr);

    return ((ret(__fastcall*)(void*, a...))address)(rcx, args...);
    }
  
    template<typename ret, typename... a>
    ALWAYSINLINE ret call_fastcall64_raw(void* address, a...args){
    #if defined(DEV_MODE)
      #if defined(TEST_MEMORY)
        if(!is_memory_valid(address, MEMORY_EXECUTE)){
          assert(false && "call_fastcall64_raw invalid address");
          return 0xDEADBEEF;
        }
      #endif
    #endif

    return ((ret(__fastcall*)(a...))address)(args...);
    }
  #else
    template<u32 index, typename ret, typename... a>
    ALWAYSINLINE ret cvirtual_call(void* vtable, a...args){
      return ((ret(__cdecl*)(void*, a...))(get_virtual_address(vtable, index)))(vtable, args...);
    }

    template<typename ret, typename... a>
    ALWAYSINLINE ret call_thiscall(void* address, void* ecx, a...args){
      #if defined(DEV_MODE)
        #if defined(TEST_MEMORY)
        if(!is_memory_valid(address, MEMORY_EXECUTE)){
          assert(false && "call_fastcall64_raw invalid address");
          return 0xDEADBEEF;
        }
        #endif
      #endif

      assert(address != nullptr);
      assert(ecx != nullptr);
  
      return ((ret(__thiscall*)(void*, a...))address)(ecx, args...);
    }
  
    template< typename ret, typename... a >
    ALWAYSINLINE ret call_cdecl(void* address, a...args){
      #if defined(DEV_MODE)
        #if defined(TEST_MEMORY)
        if(!is_memory_valid(address, MEMORY_EXECUTE)){
          assert(false && "call_fastcall64_raw invalid address");
          return 0xDEADBEEF;
        }
        #endif
      #endif
  
      return ((ret(__cdecl*)(a...))address)(args...);
    }

    template<typename ret, typename... a>
    ALWAYSINLINE ret call_fastcall(void* address, void* ecx, void* edx, a...args){
      #if defined(DEV_MODE)
        #if defined(TEST_MEMORY)
        if(!is_memory_valid(address, MEMORY_EXECUTE)){
          assert(false && "call_fastcall64_raw invalid address");
          return 0xDEADBEEF;
        }
        #endif
      #endif
      assert(ecx != nullptr);
  
      return ((ret(__fastcall*)(void*, void*, a...))address)(ecx, edx, args...);
    }
  #endif
#endif 

// For things that aren't meant to be inside cheats should be in here, that way we avoid allowing possibly insecure calls.
#if !defined(INTERNAL_CHEAT)
  // Returns the original pointer of SystemProcessInformation which has to be free'd
  static void* get_system_process_info(SYSTEM_PROCESS_INFORMATION* table, u32 size_in_bytes, u32* count){
    DECLARE_NT_QUERY_SYSTEM_INFO;
    if(table == nullptr || size_in_bytes == 0 || count == nullptr){
      assert(false && "[-] get_system_process_info: parameters are invalid")
      return nullptr;
    }

    ul64 process_size = 0;
    *count            = 0;
    I(memset)(table, 0, size_in_bytes);
    if(nt_query_system_information(SystemProcessInformation, nullptr, 0, &process_size) != 0xC0000004)
      return nullptr;

    void* process_info     = I(malloc)(process_size);
    void* process_info_ptr = process_info;

    // Size can change after we get and thus fail.
    if(nt_query_system_information(SystemProcessInformation, process_info, process_size, &process_size) != 0){
      I(free)(process_info_ptr);
      return nullptr;
    }

    u32       c    = 0;
    const u32 size = (size_in_bytes / sizeof(SYSTEM_PROCESS_INFORMATION));
    while(true){
      SYSTEM_PROCESS_INFORMATION* process = (SYSTEM_PROCESS_INFORMATION*)process_info;
      if(process == nullptr)
        break;

      // Ignore proceses with no valid image names.
      if(process->ImageName.Buffer != nullptr){
        memcpy(&table[c], process, sizeof(SYSTEM_PROCESS_INFORMATION));
        c++;
      } 

      if(c >= size || process->NextEntryOffset == 0)
        break;

      process_info = (SYSTEM_PROCESS_INFORMATION*)((i8*)process_info + process->NextEntryOffset);
    }

    if(c == 0){
      I(free)(process_info_ptr);
      return nullptr;
    }

    assert(c <= size);

    *count = c;
    return process_info_ptr;
  }

  // return_highest_mem_usage is useful if the process has child processes.
  static uptr get_pid(const u32 hash, u32 filters = 0){
    DBG("[!] utils::get_pid %X, filters: %X\n", hash, filters);
    uptr pid = 0;

    SYSTEM_PROCESS_INFORMATION table[1024];
    u32 count = 0;
    void* p = get_system_process_info(table, sizeof(table), &count);
    if(p == nullptr){
      DBG("[-] get_pid failed to get process info\n");
      return false;
    }

    if(!count){
      DBG("[-] get_pid no processes could be found.\n");
      return 0;
    }

    i8*   name_buffer     = (i8*)I(malloc)(260);
    void* name_buffer_ptr = (void*)name_buffer;

    uptr last_working_set_size = 0;
    for(u32 i = 0; i < count; i++){
      SYSTEM_PROCESS_INFORMATION* process = &table[i];
      if(process == nullptr)
        continue;

      memset(name_buffer, 0, 260);

      const i32 unicode_len = I(WideCharToMultiByte)(XOR32(CP_UTF8), 0, process->ImageName.Buffer, process->ImageName.Length / sizeof(WCHAR), nullptr, 0, nullptr, nullptr);
      if(unicode_len < 0 || unicode_len >= 260)
        continue;

      I(WideCharToMultiByte)(XOR32(CP_UTF8), 0, process->ImageName.Buffer, -1, name_buffer, unicode_len, nullptr, nullptr);
      name_buffer[unicode_len] = '\0';

      u32 len = strlen(name_buffer);
      if(len > 0 && FNV1A_RT(name_buffer, len, true) != hash)
        continue;

      uptr mem_usage = (uptr)process->WorkingSetSize;
      if(mem_usage <= 0x47868C0 /* 75 MB */ && (filters & GET_PID_IGNORE_CHILD_PROCESS_LOGIC)){ // This is the easist way to ignore child processes.
        DBG("[-] utils::get_pid %s - GET_PID_IGNORE_CHILD_PROCESS_LOGIC\n", name_buffer);
        continue;
      }

      if((uptr)process->WorkingSetSize < last_working_set_size && (filters & GET_PID_RETURN_HIGHEST_MEMORY)){
        DBG("[!] utis::get_pid %i < %i - %s GET_PID_RETURN_HIGHEST_MEMORY\n", (uptr)process->WorkingSetSize, last_working_set_size, name_buffer);
        continue;
      }

      pid                   = (uptr)process->UniqueProcessId;
      last_working_set_size = process->WorkingSetSize;

      DBG("[!] utils::get_pid %s, %i (%X)\n", name_buffer, pid, last_working_set_size);
    }

    I(free)(p);
    I(free)(name_buffer_ptr);
    return pid;
  }

  // No pids received if false.
  static bool get_pids(const u32 hash, uptr* out, u32 size_in_bytes, u32* pids_count){
    if(out == nullptr || size_in_bytes < sizeof(uptr) || pids_count == nullptr){
      assert(false && "get_pids has bad parameters");
      return false;
    }

    DBG("[+] utils::get_pids: %X, 0x%p, %i, 0x%p\n", hash, out, size_in_bytes, pids_count);

    *pids_count = 0;
    memset(out, 0, size_in_bytes);

    u32       c    = 0;
    const u32 size = (size_in_bytes / sizeof(uptr));

    SYSTEM_PROCESS_INFORMATION* table = (SYSTEM_PROCESS_INFORMATION*)malloc(sizeof(SYSTEM_PROCESS_INFORMATION) * size);
    if(table == nullptr){
      assert(false && "get_pids out of memory");
      return false;
    }

    u32 info_count = 0;
    void* p = get_system_process_info(table, sizeof(SYSTEM_PROCESS_INFORMATION) * size, &info_count);
    if(p == nullptr){
      DBG("[-] utils::get_pids process_info failed\n");
      free(table);
      return false;
    }

    if(!info_count){
      DBG("[-] utils::get_pids info_count is zero\n");
      free(table);
      return false;
    }

    i8*   name_buffer     = (i8*)I(malloc)(260);
    assert(name_buffer != nullptr && "out of memory");
    void* name_buffer_ptr = (void*)name_buffer;

    for(u32 i = 0; i < info_count; i++){
      SYSTEM_PROCESS_INFORMATION* process = &table[i];
      if(process == nullptr)
        continue;

      memset(name_buffer_ptr, 0, 260);

      const i32 unicode_len = I(WideCharToMultiByte)(XOR32(CP_UTF8), 0, process->ImageName.Buffer, process->ImageName.Length / sizeof(wchar_t), nullptr, 0, nullptr, nullptr);
      if(unicode_len < 0 || unicode_len >= 260)
        continue;

      I(WideCharToMultiByte)(XOR32(CP_UTF8), 0, process->ImageName.Buffer, -1, name_buffer, unicode_len, nullptr, nullptr);
      name_buffer[unicode_len] = '\0';


      u32 len = strlen(name_buffer);
      if(len > 0 && FNV1A_RT(name_buffer, len, true) != hash)
        continue;

      DBG("[!] utils::get_pids %s %i\n", name_buffer, (uptr)process->UniqueProcessId);
      out[c] = (uptr)process->UniqueProcessId;
      c++;
      if(c >= size){
        DBG("utils::get_pids not enough entries for table.\n");
        break;
      }
    }

    DBG("[!] the end\n");
    I(free)(p);
    I(free)(name_buffer_ptr);
    I(free)(table);
    
    *pids_count = c;
    return c > 0;
  }

  static std::string get_pid_name(const uptr pid){
    std::string found_name = "";
    if(!pid)
      return found_name;

    SYSTEM_PROCESS_INFORMATION table[1024];
    u32 count = 0;
    void* p = get_system_process_info(table, sizeof(table), &count);
    if(p == nullptr)
      return found_name;

    i8*   name_buffer     = (i8*)I(malloc)(260);
    void* name_buffer_ptr = (void*)name_buffer;

    for(u32 i = 0; i < count; i++){
      SYSTEM_PROCESS_INFORMATION* process = &table[i];
      if(process == nullptr)
        continue;

      memset(name_buffer, 0, 260);

      const i32 unicode_len = I(WideCharToMultiByte)(XOR32(CP_UTF8), 0, process->ImageName.Buffer, process->ImageName.Length / sizeof(WCHAR), nullptr, 0, nullptr, nullptr);
      if(unicode_len < 0 || unicode_len >= 260)
        continue;

      I(WideCharToMultiByte)(XOR32(CP_UTF8), 0, process->ImageName.Buffer, -1, name_buffer, unicode_len, nullptr, nullptr);
      name_buffer[unicode_len] = '\0';

      if((uptr)process->UniqueProcessId != pid)
        continue;

      found_name = (i8*)name_buffer;
      DBG("[!] utils::get_pid_name: %s, %i\n", name_buffer, pid);
      break;
    }

    I(free)(p);
    I(free)(name_buffer_ptr);
    return found_name;
  }

  static bool terminate_process(const uptr pid, ul64* last_error = nullptr){
    static void* nt_terminate_process_ptr = utils::get_proc_address(utils::get_module_handle(HASH("ntdll.dll")), HASH("NtTerminateProcess"));
    assert(nt_terminate_process_ptr != nullptr);

    if(!pid){
      DBG("[!] terminate_process: received a null pid\n");
      return true;
    }

    HANDLE handle = I(OpenProcess)(PROCESS_TERMINATE, false, pid);
    if(last_error != nullptr)
      *last_error = I(GetLastError)();

    if(!IS_HANDLE_VALID(handle)){
      DBG("[-] utils::terminate_process, failed to get process handle for pid: %i\n", pid);
      return false;
    }

    NTSTATUS result = call_stdcall<NTSTATUS, void*, NTSTATUS>(nt_terminate_process_ptr, handle, 0);

    DBG("[!] terminate_process: %i, %p\n", pid, result);
    I(CloseHandle)(handle);
    return result == STATUS_SUCCESS;
  }
  
  // Return true if there is no processes.
  static bool terminate_process_by_name(const u32 name){
    DBG("[!] utils::terminate_process_by_name: %X\n", name);

    bool term = false;
    while(true){
      u32 p_id = utils::get_pid(name, 0);
      DBG("TERM HITLA %x\n", p_id);

      if(p_id == 0)
        return !term;

      term = true;
      terminate_process(p_id);
    }

    return !term; 
  }

  static bool resume_process(const uptr pid, ul64* last_error = nullptr){
    static void* resume_process_ptr = utils::get_proc_address(utils::get_module_handle(HASH("ntdll.dll")), HASH("NtResumeProcess"));
    if(!pid){
      assert(false && "resume_process is trying to resume a null pid");
      return false;
    }

    HANDLE handle    = I(OpenProcess)(XOR32(PROCESS_ALL_ACCESS), 0, pid);
    if(!IS_HANDLE_VALID(handle)){
      if(last_error != nullptr)
        *last_error = I(GetLastError)();

      return false;
    }

    NTSTATUS result = call_stdcall<NTSTATUS, void*>(resume_process_ptr, handle);
    
    DBG("[!] utils::resume_process on pid %i, ntstatus: %p\n", pid, result);
    I(CloseHandle)(handle);
    return result == STATUS_SUCCESS;
  }

  static bool suspend_process(const uptr pid, ul64* last_error = nullptr){
    static void* suspend_process_ptr = utils::get_proc_address(utils::get_module_handle(HASH("ntdll.dll")), HASH("NtSuspendProcess"));
    if(!pid){
      assert(false && "suspend_process is trying to suspend a null pid");
      return false;
    }

    HANDLE handle    = I(OpenProcess)(XOR32(PROCESS_ALL_ACCESS), 0, pid);
    if(!IS_HANDLE_VALID(handle)){
      if(last_error != nullptr)
        *last_error = I(GetLastError)();

      return false;
    }

    NTSTATUS result = call_stdcall<NTSTATUS, void*>(suspend_process_ptr, handle);

    DBG("[!] utils::suspend_process on pid %i, ntstatus: %p\n", pid, result);
    I(CloseHandle)(handle);
    return result == STATUS_SUCCESS;
  }
#endif

  static bool nt_continue(CONTEXT* context_frame, bool raise_alert){
    static void* nt_continue_ptr = utils::get_proc_address(utils::get_module_handle(HASH("ntdll.dll")), HASH("NtContinue"));
    return call_stdcall<NTSTATUS, CONTEXT*, bool>(nt_continue_ptr, context_frame, raise_alert) == STATUS_SUCCESS;
  }

#if !defined(STRING)

#if defined(IMGUI_API)
  i32 format_string(i8* buf, u32 buf_size, const i8* fmt, ...);
#endif
  // Can detect a string with in a string. -Rud
  inline bool find_start_and_end(wchar_t* buf1, wchar_t* buf2, i32* start = nullptr, i32* end = nullptr){
    if(buf1 == nullptr || buf2 == nullptr)
      return false;

    u32 len1 = wcslen(buf1);
    u32 len2 = wcslen(buf2);

    if(len1 < len2)
      return false;

    if(start != nullptr)
      *start = -1;

    if(end != nullptr)
      *end   = -1;

    i32 cur   = 0;
    for(i32 i = 0; i < len1; i++){
      if(buf1[i] == buf2[cur]){

        if(!cur){
          if(start != nullptr)
            *start = i;
        }

        cur++;
        if(cur >= len2){
          if(end != nullptr)
            *end = i;

          // Success!!
          return true;
        }
      }
      else{

        // The character wasn't the same! Reset
        cur = 0;
        if(start != nullptr)
          *start = -1;

        if(end != nullptr)
          *end   = -1;
      }
    }

    // Fail!!
    return false;
  }

  inline i32 index_of(i8* buf1, i8* buf2, i32 start = -1){
    if(buf1 == nullptr || buf2 == nullptr)
      return -1;

    i32 cur  = -1;
    u32 len1 = strlen(buf1);
    u32 len2 = strlen(buf2);
    if(len2 >= len1)
      return -1;

    if(start != -1){
      if(start >= len1)
        return -1;
    }

    u32 count = 0;
    for(i32 i = start == -1 ? 0 : start; i < len1; i++){
      if(buf1[i] == buf2[count]){
        if(!count)
          cur = i;

        // This is the string! Return the start index.
        count++;
        if(count >= len2)
          return cur;
      }
      else{
        cur   = -1;
        count = 0;
      }
    }

    return -1;
  }

  inline i32 index_of(wchar_t* buf1, wchar_t* buf2, i32 start = -1){
    if(buf1 == nullptr || buf2 == nullptr)
      return -1;

    i32 cur  = -1;
    u32 len1 = wcslen(buf1);
    u32 len2 = wcslen(buf2);
    if(len2 >= len1)
      return -1;

    if(start != -1){
      if(start >= len1)
        return -1;
    }

    u32 count = 0;
    for(i32 i = start == -1 ? 0 : start; i < len1; i++){
      if(buf1[i] == buf2[count]){
        if(!count)
          cur = i;

        // This is the string! Return the start index.
        count++;
        if(count >= len2)
          return cur;
      }
      else{
        cur   = -1;
        count = 0;
      }
    }

    return -1;
  }

  inline void remove_character(std::wstring& str, wchar_t c) {
    if (c == L'\0')
      return;

    std::wstring new_str;
    for (i32 i = 0; i < str.length(); i++) {
      if (str[i] == c)
        continue;

      new_str += str[i];
    }

    str = new_str;
  }
#endif

  inline bool create_process(std::wstring path, std::wstring args){
    if(path.length() == 0)
      return false;

    STARTUPINFOW sinfo;
    memset(&sinfo, 0, XOR32(sizeof(STARTUPINFOW)));
    sinfo.cb = XOR32(sizeof(STARTUPINFOW));

    PROCESS_INFORMATION pinfo;
    memset(&pinfo, 0, XOR32(sizeof(PROCESS_INFORMATION)));

    wchar_t arg_buffer[512];
    I(wsprintfW)(arg_buffer, WXOR(L"%S %S"), path.c_str(), args.c_str());

    return I(CreateProcessW)(path.c_str(), arg_buffer, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &sinfo, &pinfo);
  }

  inline bool create_process(std::string path, std::string args){
    if(path.length() == 0)
      return false;

    STARTUPINFOA sinfo;
    memset(&sinfo, 0, XOR32(sizeof(STARTUPINFOA)));
    sinfo.cb = XOR32(sizeof(STARTUPINFOA));

    PROCESS_INFORMATION pinfo;
    memset(&pinfo, 0, XOR32(sizeof(PROCESS_INFORMATION)));

    i8 arg_buffer[512];
    I(wsprintfA)(arg_buffer, XOR("%s %s"), path.c_str(), args.c_str());

    return I(CreateProcessA)(path.c_str(), arg_buffer, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &sinfo, &pinfo);
  }

  inline bool create_worker(LPTHREAD_START_ROUTINE func, void* p){

    // Valve implementation call to bypass VAC checks
    static void* create_simple_thread_ptr = utils::get_proc_address(utils::get_module_handle(HASH("tier0.dll")), HASH("CreateSimpleThread"));

#if defined(GMOD)
    create_simple_thread_ptr = 0;
#endif

    // Use valves tier0 createsimplethread code if it exists
    if(create_simple_thread_ptr != 0){

      #if defined(__x86_64__)
      void* thread = call_fastcall64_raw<void*, LPTHREAD_START_ROUTINE, void*, u32>(create_simple_thread_ptr, func, p, 0);
      #else
      void* thread = call_cdecl<void*, LPTHREAD_START_ROUTINE, void*, u32>(create_simple_thread_ptr, func, p, 0);
      #endif
      

      if (thread == nullptr || (HANDLE)thread == -1){
        DBG("[-] tier0 create_worker: failed\n");
        return false;
      }

      I(CloseHandle)(thread);
    }
    else{
      void* thread = I(CreateThread)( nullptr, 0, (LPTHREAD_START_ROUTINE)( func ), p, 0, nullptr );
      if(!IS_HANDLE_VALID(thread)){
        #if defined(LOADER_CLIENT) || defined(LOADER_STUB)
          I(ExitProcess)(2);
        #endif
        DBG("[-] windows create_worker: failed \n");
        return false;
      }

      I(CloseHandle)(thread);
    }

    return true;
  }

  #if defined(NEED_SIG_SCANNER)
  // THIS SHOULD NEVER BE USED!
  // It is only used on the vac bypasser because of a rare specific situtation and we do not want to use it anywhere else!
  #define SIG(sig) sig, XOR32(sizeof(sig)-1)
  inline uptr find_signature(uptr module_ptr, u8* sig, uptr sig_len, uptr max_len){
    for(uptr i = module_ptr; i < module_ptr + max_len; i++){
      for(uptr ii = 0; ii < sig_len; ii++){
        u8 cur_byte = *(u8*)(i + ii);
        u8 sig_byte = sig[ii];

        if(sig_byte != 0x00 && cur_byte != sig_byte)
          goto next_scan;
      }

      DBG("[+] find_signature: found signature at %p\n", i);
      return i;

      next_scan:
        continue;
    }

    DBG("[-] find_signature: signature scan failed, dumping signature below:\n\n");

    // Dump the signature to string so we can see whats failing
    for(u32 i = 0; i < sig_len; i++)
      DBG("\\x%02X", sig[i]);

    DBG("\n\n");

    assert(false);
    return 0;
  }
  #endif

#if defined(INTERNAL_CHEAT)
  inline std::string get_exception_name(u32 code){
    switch(code){
      case 0xE06D7363: return XOR("C++ exception");
      case 0xC0000374: return XOR("Heap corruption");
      case 0x406D1388: return XOR("MS Thread nameing");
      case EXCEPTION_ACCESS_VIOLATION: return XOR("ACCESS VIOLATION");
      case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return XOR("ARRAY BOUNDS EXCEEDED");
      case EXCEPTION_BREAKPOINT: return XOR("BREAKPOINT");
      case EXCEPTION_DATATYPE_MISALIGNMENT: return XOR("DATATYPE MISALIGNMENT");
      case EXCEPTION_FLT_DENORMAL_OPERAND: return XOR("FLT DENORMAL OPERAND");
      case EXCEPTION_FLT_DIVIDE_BY_ZERO: return XOR("DIVIDE BY ZERO");
      case EXCEPTION_FLT_INEXACT_RESULT: return XOR("FLT INEXACT RESULT");
      case EXCEPTION_FLT_INVALID_OPERATION: return XOR("FLT INVALID OPERATION");
      case EXCEPTION_FLT_OVERFLOW: return XOR("FLT OVERFLOW");
      case EXCEPTION_FLT_STACK_CHECK: return XOR("FLT STACK CHECK");
      case EXCEPTION_FLT_UNDERFLOW: return XOR("FLT UNDERFLOW");
      case EXCEPTION_ILLEGAL_INSTRUCTION: return XOR("ILLEGAL INSTRUCTION");
      case EXCEPTION_IN_PAGE_ERROR: return XOR("IN PAGE ERROR");
      case EXCEPTION_INT_DIVIDE_BY_ZERO: return XOR("INT DIVIDE BY ZERO");
      case EXCEPTION_INT_OVERFLOW: return XOR("INT OVERFLOW");
      case EXCEPTION_INVALID_DISPOSITION: return XOR("INVALID DISPOSITION");
      case EXCEPTION_NONCONTINUABLE_EXCEPTION: return XOR("NONCONTINUABLE EXCEPTION");
      case EXCEPTION_PRIV_INSTRUCTION: return XOR("PRIV INSTRUCTION");
      case EXCEPTION_SINGLE_STEP: return XOR("SINGLE STEP");
      case EXCEPTION_STACK_OVERFLOW: return XOR("STACK OVERFLOW");
    }

    return XOR("UNKNOWN");
  }
#endif

  inline bool parse_str_end_to_char(std::string str, i8 c, std::string* out){
    if(out == nullptr)
      return false;
  
    u32 len = str.length();
    if(str[len] == c){
      *out = str;
      return true;
    }
  
    u32 new_len = 0;
    for(u32 i = len; i > 0; i--){
      if(str[i] == c){
        new_len = i;
        break;
      }
    }

    i32 start_index = 0;
    if(str[0] == '"')
      start_index = 1;

    if(start_index == len)
      return false;
  
    *out = str.substr(start_index, new_len);
  
    return true;
  }

  inline bool parse_str_end_to_char(std::wstring str, wchar_t c, std::wstring* out){
    if(out == nullptr)
      return false;
  
    u32 len = str.length();
    if(str[len] == c){
      *out = str;
      return true;
    }
  
    u32 new_len = 0;
    for(u32 i = len; i > 0; i--){
      if(str[i] == c){
        new_len = i;
        break;
      }
    }


    i32 start_index = 0;
    if(str[0] == L'"')
      start_index = 1;

    if(start_index == len)
      return false;
  
    *out = str.substr(start_index, new_len);
  
    return true;
  }

  inline bool parse_str_start_to_char(std::string str, i8 c, std::string* out){
    if(out == nullptr)
      return false;
  
    u32 len = str.length();
    if(str[len] == c){
      *out = str;
      return true;
    }
  
    u32 new_len = 0;
    for(u32 i = len; i > 0; i--){
      if(str[i] == c){
        new_len = i;
        break;
      }
    }
    
    if(new_len + 1 == len)
      return false;

    *out = str.substr(new_len + 1, len);
  
    return true;
  }

  inline bool parse_str_start_to_char(std::wstring str, wchar_t c, std::wstring* out){
    if(out == nullptr)
      return false;
  
    u32 len = str.length();
    if(str[len] == c){
      *out = str;
      return true;
    }
  
    u32 new_len = 0;
    for(u32 i = len; i > 0; i--){
      if(str[i] == c){
        new_len = i;
        break;
      }
    }

    if(new_len + 1 == len)
      return false;
  
    *out = str.substr(new_len + 1, len);
  
    return true;
  }

  // KUSER_SHARED_DATA is a global kernel object shared between all processes on the system
#if !defined(INTERNAL_CHEAT)
  #define GET_WINVER (utils::get_kuser_shared_data()->NtBuildNumber)
  inline KUSER_SHARED_DATA* get_kuser_shared_data(){
    return (KUSER_SHARED_DATA*)(XOR32(0x7FFE0000));
  }


  inline i32 get_ubr(){
    static i32 ret = reg::read_key_i32(XOR(L"Software\\Microsoft\\Windows NT\\CurrentVersion"), XOR(L"UBR"), HKEY_LOCAL_MACHINE);
    return ret;
  }
#endif

  inline std::wstring get_current_process_directory(){
    std::wstring cmd_line = utils::get_command_line();
    if(cmd_line.empty())
      return cmd_line;

    std::wstring new_str;
    if(parse_str_end_to_char(cmd_line, L'\\', &new_str))
      return new_str;

    return L"";
  }

  inline std::wstring get_current_process_name(){
    std::wstring cmd_line = utils::get_command_line();
    if(cmd_line.empty())
      return cmd_line;

    std::wstring new_str;
    if(parse_str_start_to_char(cmd_line, L'\\', &new_str)){

      std::wstring name; // Produce a clean version of the string.
      for(u32 i = 0; i < new_str.length(); i++){
        if(new_str[i] == '"')
          continue;

        if(new_str[i] == '.')
          break;

        name += new_str[i];
      }

      return name;
    }

    return L"";
  }

  bool get_module_info_from_address_space(uptr addr, MODULEENTRY32* entry);
  bool get_process_modules(MODULEENTRY32* entry, u32 size, u32* c, uptr pid = I(GetCurrentProcessId)());

  inline void* get_current_process_window(){
    static void* window = nullptr;
    EnumWindows([](void* handle, void* param){
      u32 wnd_proc_id = 0;
      GetWindowThreadProcessId(handle, (LPARAM)&wnd_proc_id);

      if(wnd_proc_id != GetCurrentProcessId())
        return true;

      *(void**)param = handle;
      return false;
    }, &window);

    return window;
  }

  // Didn't know where to put this but its gonna be needed for other source games.
  inline void detect_se_invalid_launch_args(i32 dxlevel){
    // Error out if they're using certain game settings.
    wchar_t* cmd_line = utils::get_command_line();
    if(cmd_line != nullptr){
      if(str_utils::contains(cmd_line, WXOR(L"-r_emulate_gl"))){
        I(MessageBoxA)(nullptr, XOR("RijiN\nRemove the launch parameter -r_emulate_gl"), nullptr, XOR32(MB_ICONERROR) | XOR32(MB_TOPMOST));
        I(ExitProcess)(0);
        return;
      }
      else if(str_utils::contains(cmd_line, WXOR(L"-nobreakpad"))){
        I(MessageBoxA)(nullptr, XOR("RijiN\nRemove the launch parameter -nobreakpad"), nullptr, XOR32(MB_ICONERROR) | XOR32(MB_TOPMOST));
        I(ExitProcess)(0);
        return;
      }
      else if(str_utils::contains(cmd_line, WXOR(L"-nocrashdialog"))){
        I(MessageBoxA)(nullptr, XOR("RijiN\nRemove the launch parameter -nocrashdialog"), nullptr, XOR32(MB_ICONERROR) | XOR32(MB_TOPMOST));
        I(ExitProcess)(0);
        return;
      }
    }
  }

  // This function result should be cached if it's needed somewhere where performance is important.
  // Only works with a handle that FindWindow gives you.
  inline bool is_window_minimized(void* handle){
    if(!IS_HANDLE_VALID(handle))
      return false;

    WINDOWPLACEMENT pl;
    memset(&pl, 0, sizeof(pl));
    pl.length = sizeof(WINDOWPLACEMENT);
    I(GetWindowPlacement)((HWND)handle, &pl);
    return pl.showCmd == SW_SHOWMINIMIZED;
  }

  inline bool focus_window(void* handle){
    if(!IS_HANDLE_VALID(handle))
      return false;

    if(!I(ShowWindow)((HWND)handle, SW_RESTORE))
      return false;

    I(SetForegroundWindow)((HANDLE)handle);
    return true;
  }

  inline std::wstring vis_upper_string(wchar_t* str){
    str = str_utils::toupper(str);

    std::wstring rebuilt = L"";
    for(u32 i = 0; i < wcslen(str); i++){
      if(str[i] == L' ')
        rebuilt += str[i];

      rebuilt += str[i];
    }
    return rebuilt;
  }

#if defined(INTERNAL_CHEAT)
  std::string bytes_to_hex(u8* data, u32 len);
  u8* hex_to_bytes(i8* str);
#endif

#if !defined(DISABLE_NETWORKING)
  bool submit_error_code(u32 error_code, u32 status_code = 0);
#endif

  bool get_process_hash_list(u32* table, u32* max_len);

#if defined(SOURCE_CHEAT)
  inline bool is_valve_fake_ip(i8* ip){
    if(ip == nullptr)
      return false;

    if(str_utils::contains(ip, XOR("169.254.")))
      return true;

    return false;
  }
#endif

#if defined(INTERNAL_CHEAT)
  inline bool is_rijin_text_section(void* location){
    uptr addr = (uptr)location;
#if defined(INTERNAL_CHEAT)
    uptr base_address_start = gen_internal->base_address_start;
    uptr base_address_end   = gen_internal->base_address_end;

    #if defined(__x86_64__)
      base_address_start = gen_internal->base_address_start64;
      base_address_end   = gen_internal->base_address_end64;
    #endif

    // is address in our cheat page
    if(addr >= base_address_start && addr <= base_address_end)
      return true;
#endif

    return false;
  }
#endif

  inline void perform_str_xor(std::wstring& name, i32 key){
    if(name.empty())
      return;

    for(u32 i = 0; i < name.length(); i++)
      name[i] ^= key;
  }

  inline std::wstring str_xor(std::wstring name, i32 key){
    for(u32 i = 0; i < name.length(); i++)
      name[i] ^= key;

    return name;
  }

};