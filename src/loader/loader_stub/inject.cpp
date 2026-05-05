#include "link.h"


#define XOR_HASH(x) XOR32(HASH(x))
static void _stub_message(u32 c){
  // CreateThread could be used as a breakpoint for reversers. But they could just use the network messages also.
  // Thought about detecting hooks or breakpoints in here but I don't know if that would have any false positives or not.
  static u32* what = (u32*)malloc(sizeof(u32));
  static bool talking = false;
  *what = c;
  utils::create_worker([](void* ptr){
    u32 code = *(u32*)ptr;
    while(talking) // avoid tripping any server-side ratelimits.
      I(Sleep)(50);

    talking = true;
    send_status_code(XOR32(HASH("STUB")), code);
    talking = false;

    return 0;
  }, what);
}

#if !defined(DEV_MODE)
  #define stub_message(x) \
  VM_FAST_START(); \
  _stub_message(XOR32(HASH(x))); \
  VM_FAST_STOP();
#else
  #define stub_message(x) DBG("%s\n", x);
#endif

u64 c_inject::alloc_memory(ul64 len, u32 allocation_type, u32 protection, u64 base = 0){
  void* new_address = nullptr;

  SIZE_T alloc_len = len;

  VM_MUTATE_START()
  NTSTATUS status = SYSCALL(NtAllocateVirtualMemory)((HANDLE)process, &new_address, 0, &alloc_len, allocation_type, protection);
  VM_MUTATE_STOP()

  if(status != 0){
    DBG("[!] alloc_memory failed: %p\n", status);
    assert(false && "alloc_memory failed? wow why? look at the console and see the status code!");
    return 0;
  }

  DBG("[!] alloc_memory new_address: %p len: %i\n", new_address, len);

  assert(new_address != nullptr);
  return (u64)new_address;
}

#define WR_BLOCK_SCAN_SIZE XOR32(0x100)

bool c_inject::read_memory(void* address, void* data, ul64 len, u64* status_code){
  VM_MUTATE_START()

  // So rather than just do a single call and read all of the memory.
  // We'll do it in tiny chunks if possible that way its not just a simple ez 'dump'.
  // Not full proof but the idea is there.
  ul64 read_size = len >= WR_BLOCK_SCAN_SIZE ? WR_BLOCK_SCAN_SIZE : len;
  ul64 read   = 0;

  while(read < len){
    if(SYSCALL(NtReadVirtualMemory)(process, (void*)address, (void*)data, len, (PSIZE_T)&read) != XOR32(STATUS_SUCCESS))
      return false;

    ul64 delta = (len - read);
    if(read_size > delta)
      read_size = delta;

    address += read;
    data    += read;
  }

  VM_MUTATE_STOP()

  return true;
}

bool c_inject::write_memory(void* address, void* data, ul64 len, u64* status_code){
  VM_MUTATE_START()
  ul64 write_size = len >= WR_BLOCK_SCAN_SIZE ? WR_BLOCK_SCAN_SIZE : len;
  ul64 write   = 0;

  while(write < len){
    if(SYSCALL(NtWriteVirtualMemory)(process, (void*)address, (void*)data, len, (PSIZE_T)&write) != XOR32(STATUS_SUCCESS))
      return false;

    ul64 delta = (len - write);
    if(write_size > delta)
      write_size = delta;

    address += write;
    address += write;
  }

  VM_MUTATE_STOP()
  return true;
}

bool c_inject::read_memory_raw(void* address, void* data, ul64 len){
  NTSTATUS status = SYSCALL(NtReadVirtualMemory)(process, (void*)address, data, len, 0);
  return status == STATUS_SUCCESS;
}

bool c_inject::write_memory_raw(void* address, void* data, ul64 len){
  NTSTATUS status = SYSCALL(NtWriteVirtualMemory)(process, (void*)address, (void*)data, len, 0);
  return status == STATUS_SUCCESS;
}

bool c_inject::virtual_protect(void* address, u32 len, u32 protection, u32* old_protection){
  return utils::virtual_protect_ex(process, address, len, protection, old_protection);
}

void* c_inject::open_process(u32 pid, u32* last_error){
  process_id = pid;
  void* handle = I(OpenProcess)(XOR32(PROCESS_QUERY_INFORMATION | PROCESS_ALL_ACCESS), 0, pid);
  if(last_error != nullptr)
    *last_error = I(GetLastError)();

  return handle;
}
  

// If await is true, it'll keep trying for a minute to get a non zero return.
u64 c_inject::get_module_address(u32 name_hash, bool await){
#if defined(DEV_MODE)
    s_gmc_entry* gmc_e = gmc->find_entry(HASH("get_module_address"), name_hash);
    if(gmc_e != nullptr)
      return gmc_e->ptr;
#endif

  u64 module_address = 0;
  STUB_WAIT_LOOP(STUB_PROCESS_TIMEOUT_MS){
    DBG("[!] get_module_address: %X - %i\n", name_hash, await);
    HANDLE handle = I(CreateToolhelp32Snapshot)(XOR32(0x00000008) | XOR32(0x00000010), process_id);
    if(!IS_HANDLE_VALID(handle)){
      DBG("[!] get_module_address: %X - %i (invalid handle)\n", handle, await);
      continue;
    }

    MODULEENTRY32 module_entry = MODULEENTRY32{ sizeof(MODULEENTRY32) };
  
    if(I(Module32First)(handle, &module_entry)){
      do{
        bool did_find_before_lowercase = HASH_RT(module_entry.szModule) == name_hash;
  
        // Lowercase module names
        if(!did_find_before_lowercase){
          for(u32 i = 0; i < I(strlen)(module_entry.szModule); i++)
            module_entry.szModule[i] = I(tolower)(module_entry.szModule[i]);
        }
  
        if(did_find_before_lowercase || HASH_RT(module_entry.szModule) == name_hash){
          module_address = module_entry.modBaseAddr;
          DBG("[+] found module %s %p\n", module_entry.szModule, module_address);
#if defined(DEV_MODE)
    gmc->add_entry(HASH("get_module_address"), name_hash, module_address);
#endif    
          break;
        }
  
      } while(I(Module32Next)(handle, &module_entry));
    }
    else{
      DBG("[!] get_module_address: %X - %i (module32first fail)\n", handle, await);
    }
  
    I(CloseHandle)(handle);

    if(module_address > 0 || !await)
      break;

    I(Sleep)(XOR32(1));
  }

  return module_address;
}

bool c_inject::x86(u32 module_hash){
  u64 ptr   = get_module_address(module_hash);
  if(ptr == 0){
    SHOW_ERROR(ERR_INJECT_CANNOT_FIND_TARGET_MODULE, true);
    return false;
  }

  DBG("[!] c_inject::x86\n");
  u16 machine = 0;
  STUB_WAIT_LOOP(STUB_PROCESS_TIMEOUT_MS){
    u32 lfanew = 0;
    if(get_lfa_new(ptr, &lfanew, false)){
      if(read_memory(ptr + lfanew + XOR32(0x4), &machine, XOR32(sizeof(u16))))
        return machine == IMAGE_FILE_MACHINE_I386;
    }

    I(Sleep)(XOR32(1)); 
  }

  SHOW_ERROR_STR(ERR_X86_FAILED_TO_FETCH, "Failed to resolve process information.", true);
  return false;
}

u64 c_inject::get_export(u64 module_address, u32 export_hash){
  DBG("[!] c_inject::get_export: %p - %X\n", module_address, export_hash);
  u32 lfanew = 0;
  if(!get_lfa_new(module_address, &lfanew))
    return 0;

  u32 offset_to_directories = lfanew + XOR32(0x18);
  offset_to_directories += x86mode ? XOR32(sizeof( IMAGE_OPTIONAL_HEADER32 )) : XOR32(sizeof( IMAGE_OPTIONAL_HEADER64 ));
  offset_to_directories -= XOR32( sizeof( IMAGE_DATA_DIRECTORY ) * IMAGE_NUMBEROF_DIRECTORY_ENTRIES );
  offset_to_directories += XOR32( sizeof( IMAGE_DATA_DIRECTORY ) * IMAGE_DIRECTORY_ENTRY_EXPORT );

  s_export_cache* cache = fetch_export_cache_entry(module_address);
  if(cache == nullptr){
    cache = &export_cache[export_cache_count];
    export_cache_count++;

    if(!read_memory(module_address + offset_to_directories, &cache->data_directory, XOR32(sizeof(IMAGE_DATA_DIRECTORY))))
      return 0;

    if(!read_memory(module_address + cache->data_directory.VirtualAddress, &cache->export_directory, XOR32(sizeof(IMAGE_EXPORT_DIRECTORY))))
      return 0;

    void* names = I(malloc)(cache->export_directory.NumberOfNames * XOR32(sizeof(ul64)));
    void* ordinals = I(malloc)(cache->export_directory.NumberOfNames * XOR32(sizeof(u16)));
    void* funcs = I(malloc)(cache->export_directory.NumberOfFunctions * XOR32(sizeof(ul64)));

    assert(names != nullptr);
    assert(ordinals != nullptr);
    assert(funcs != nullptr);

    if(!read_memory(module_address + cache->export_directory.AddressOfNames, names, cache->export_directory.NumberOfNames * XOR32(sizeof(ul64))) ||
      !read_memory(module_address +  cache->export_directory.AddressOfNameOrdinals, ordinals, cache->export_directory.NumberOfNames * XOR32(sizeof(u16))) ||
      !read_memory(module_address +  cache->export_directory.AddressOfFunctions, funcs, cache->export_directory.NumberOfFunctions * XOR32(sizeof(ul64)))){
      I(free)(ordinals);
      I(free)(names);
      I(free)(funcs);
      return 0;
    }

    cache->names    = names;
    cache->ordinals = ordinals;
    cache->funcs    = funcs;
  }

  void* export_name = I(malloc)(XOR32(0x80));
  assert(export_name != nullptr);
  
  u64 export_address = 0;
  for(u32 i = 0; i < cache->export_directory.NumberOfNames; i++){
    if(!read_memory(module_address + ((ul64*)cache->names)[i], export_name, XOR32(0x80)))
      break;

    if(HASH_RT(export_name) == export_hash){
      export_address = module_address + ((ul64*)cache->funcs)[((u16*)cache->ordinals)[ i ] ];
      DBG("[+] found import %s %p\n", export_name, export_address);
      break;
    }
  }

  memset(export_name, 0, XOR32(0x80));
  I(free)(export_name);

  return export_address;
}

bool c_inject::fix_page_permissions(c_net_receive_cheat* output_data){
  if(output_data == nullptr || output_data->page_list_count == 0){
    SHOW_ERROR(ERR_INJECT_CANNOT_FIX_PAGE_PERMISSIONS, false);
    return false;
  }

  DBG("[!] fixing page permissions\n");

  VM_FAST_START();
  for(u32 i = 0; i < output_data->page_list_count; i++){
    s_gen_page_list* page = &output_data->page_list[i];

    u32 old_protect = 0;

    // In overwatch 2, this bypasses their sig scanning because they only check for PAGE_EXECUTE_READ
    if(page->new_protection == PAGE_EXECUTE_READ)
      page->new_protection = PAGE_EXECUTE;

    uptr last_error_code = 0;
    if(utils::virtual_protect_ex(process, page->ptr, page->len, page->new_protection, &old_protect, &last_error_code)){
      DBG("[+] applied page permission %x to %p (LER: %p)\n", page->new_protection, page->ptr, last_error_code);
    }
    else{
      DBG("[-] failed to apply page permission %p (%X) (%p) (%p)\n", page->new_protection, GetLastError(), process, last_error_code);
      SHOW_ERROR(ERR_INJECT_CANNOT_FIX_PAGE_PERMISSIONS, false);
      return false;
    }
  }
  VM_FAST_STOP();

  return true;
}

bool c_inject::short_hook(c_procedure_base* pb, s_gen_pkg* pkg){
  if(pb == nullptr || pkg == nullptr || x86mode || pb->target_module_hash == HASH("d3d9.dll"))
    return false;

  // parse the trampoline code and see if we have any disp/imm data
  hde64s h;
  for(void* p = &pkg->original_asm[0]; p < ((uptr)&pkg->original_asm[0] + (uptr)GEN_MAX_HOOK_SAMPLE_ASM64);){
    hde64_disasm(p, &h);

    if(h.flags & HDE32_F_ERROR || h.flags & HDE32_F_ERROR_LENGTH){
      DBG("[-] short_hook: HDE32_F_ERROR || HDE32_F_ERROR_LENGTH\n");
      return false;
    }

    u32   delta       = (uptr)(p + h.len) - (uptr)&pkg->original_asm[0];
    u32   real_delta  = delta - h.len;
    bool  imm         = (h.flags & HDE64_F_DISP32) || (h.flags & HDE64_F_DISP16) || (((h.flags & HDE64_F_RELATIVE)) && 
                        ((h.flags & HDE64_F_IMM32) && h.imm.imm32 != 0 ||
                        (h.flags & HDE64_F_IMM16) && h.imm.imm16 != 0 || 
                        (h.flags & HDE64_F_IMM8) && h.imm.imm8 != 0));
/*
#if defined(DEV_MODE)
    for(u32 i = 0; i < h.len; i++){
      DBG("%02X ", *(u8*)(p + i));
    }
    DBG("\n");
#endif
*/
    
    // If we have IMM data, check if we are over bytes, if we are then its fine
    // If we aren't then we need to search for somewhere to place our hook
    if(imm){
      if(delta < 5){
        DBG("[-] short_hook: cannot place short hook as imm/rel data before 5 bytes\n");
        DBG("[-] short_hook: WARNING: you won't be able to call the original on %p!!!\n", pkg->ptr);
        return false;
      }

      DBG("[!] short_hook: searching for space...\n");

      // Search for an empty space in the text section to place our FF25 that is 14 bytes long
      u64 space_ptr = pb->find_signature_internal(pb->target_module_hash, HASH(".text"), SIG("\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC"), false, 0);
      if(space_ptr == 0){
        DBG("[-] short_hook: couldn't find space for FF25!\n");
        SHOW_ERROR(ERR_SHORT_HOOK_IMPOSSIBLE, true);
        return false;
      }

      DBG("[+] short_hook: space_ptr %p\n", space_ptr);

      // Setup the short hook
      pkg->short_hook     = true;
      pkg->short_hook_ptr = space_ptr;
      pkg->short_hook_len = 5;
      pkg->short_hook_asm[0] = 0xE9;
      *(i32*)(&pkg->short_hook_asm[1]) = space_ptr - ((pkg->ptr) + 5);

      // reserve our cc instructions
      u32 old_protect;
      utils::virtual_protect_ex(process, space_ptr, 14, XOR32(PAGE_EXECUTE_READWRITE), &old_protect);
      for(u32 i = 0; i < 14; i++){
        u8 byte = 0x90;
        write_memory((u64)space_ptr + i, &byte, 1);
      }
      utils::virtual_protect_ex(process, space_ptr, 14, old_protect, &old_protect);

      DBG("[+] short_hook: %X target: %p -> space_ptr: %p\n", *(i32*)(&pkg->short_hook_asm[1]), pkg->ptr, space_ptr);
      return true;
    }

    p += h.len;
    if(delta >= 14)
      break;
  }

  return false;
}

CFLAG_O0 bool c_inject::add_hook(c_procedure_base* pb, u64 module_address, u32 target_func_hash, u64 target_address, bool add_module_address){
  if(pb == nullptr || input == nullptr || module_address == 0 || target_address == 0){
    DBG("module_address: %X target_address: %X\n", module_address, target_address);
    SHOW_ERROR(ERR_INJECT_ADD_HOOK_FAILED_BAD_PARAMETERS, true);
    return false;
  }

  s_gen_pkg new_pkg{};
  {
    memset(&new_pkg, 0, XOR32(sizeof(s_gen_pkg)));
    
    new_pkg.type  = XOR32(GEN_PKG_TYPE_HOOK);
    new_pkg.hash  = target_func_hash;
    new_pkg.ptr   = target_address;

    if(add_module_address)
      new_pkg.ptr += module_address;

    while(true){
      // Copy the asm into the original asm buffer
      const ul64 max_samples = x86mode ? XOR32(GEN_MAX_HOOK_SAMPLE_ASM86) : XOR32(GEN_MAX_HOOK_SAMPLE_ASM64);
      const ul64 required    = x86mode ? XOR32(GEN_HOOK_JMP_SIZE86) : XOR32(GEN_HOOK_JMP_SIZE64);
      ul64  size        = 0;
      i8    cur         = 0;

      // Discord screen share issue fixed: (it would be an 0xff25 and a nop) and other bytes would be invalid.
        // Straight up copying X amount of bytes, in this case 112 bytes in x64. Is cause for issues.
        // Read memory will fail if all bytes can't be copied.
        // So we test each byte.
      while(true){
        if(!read_memory_raw((void*)(uptr)new_pkg.ptr + size, &cur, 1))
          break;

        size += 1;
        if(size >= max_samples)
          break;
      }

      // Need enough for hook bytes.
      if(size < required){
        SHOW_ERROR_STATUS_STR(ERR_INJECT_ADD_HOOK_READ_MEMORY_FAILED, 1, "Known solutions for this error:\n\n- Disabling your anti virus\n- Disabling discord overlay\n- Restart Steam.\n- If you shared your screen with discord prior to injection you need to restart the game.", true);
        return false;
      }

      if(size > max_samples)
        size = max_samples;

      u64 status_code = 0;
      if(!read_memory(new_pkg.ptr, &new_pkg.original_asm, size, &status_code)){
        SHOW_ERROR_STATUS_STR(ERR_INJECT_ADD_HOOK_READ_MEMORY_FAILED, status_code, "Known solutions for this error:\n\n- Disabling your anti virus\n- Disabling discord overlay\n- Restart Steam.\n- If you shared your screen with discord prior to injection you need to restart the game.", true);
        return false;
      }

      // If we come across an E9 instruction, the function has an existing hook.
      // In this case we need to continue to jump through all the E9 instructions and then hook the inner most function
      if(new_pkg.original_asm[0] == XOR32(0xE9) || new_pkg.original_asm[0] == XOR32(0xE8)){
        u64 imm32 = (u64)(*(i32*)(&new_pkg.original_asm[1]));
  
        // Translate 32bit to 64bit addressing over limit
        if(imm32 & XOR64(0x80000000))
          imm32 |= XOR64(0xFFFFFFFF00000000);
  
        if(!x86mode)
          new_pkg.ptr = (u64)(new_pkg.ptr + XOR32(GEN_HOOK_JMP_SIZE86) + imm32);
        else
          new_pkg.ptr = (u32)(new_pkg.ptr + XOR32(GEN_HOOK_JMP_SIZE86) + imm32);

        DBG("[!] existing jmp found at hook, jumping to %p\n", new_pkg.ptr);
      }
      else{
        short_hook(pb, &new_pkg);

        break;
      }
    }

    DBG("[+] c_inject::add_hook done %p\n", new_pkg.ptr);
  }

  return input->add_pkg(new_pkg) != nullptr;
}

bool c_inject::wait_for_cheat_to_execute(u64 loaded_ptr){
  DBG("[!] waiting for cheat to execute...\n");
  stub_message("WAIT_FOR_EXECUTE");

  if(global->stub_data->object.vac_mode)
    return true;

  // If we are in vac mode, resume the process at this stage
  if(global->stub_data->object.vac_mode){
    DBG("[!] resume process pid: %i\n", process_id);
    if(!utils::resume_process(process_id)){
      SHOW_ERROR_STR(ERR_STEAM_UNABLE_TO_RESUME_WHILE_SUSPENDED, "Steam agent finalization failed.\nThis is commonly caused by anti-virus software.\nReopen the loader and try again.", true);
      return false;
    }
  }

  bool loaded     = false;
  bool had_failed = false;
  u64 status_code = 0;
  STUB_WAIT_LOOP(STUB_PROCESS_TIMEOUT_MS){
    if(!read_memory(loaded_ptr, &loaded, XOR32(sizeof(bool)), &status_code)){
      if(status_code != XOR32(STATUS_PARTIAL_COPY))
        break;

      had_failed = true;
    }

    if(loaded)
      break;

    I(Sleep)(XOR32(1));
  }

  if(global->stub_data->object.vac_mode){
    if(!loaded){
      SHOW_ERROR_STATUS_STR(ERR_INJECT_WAIT_FOR_CHEAT_TO_LOAD_TIMEOUT, status_code, "Steam agent: Failed to execute.", true); 
      return false;
    }
  }
  else{
    if(!loaded){
      utils::terminate_process(process_id);
      if(status_code != XOR32(STATUS_PARTIAL_COPY))
        SHOW_ERROR_STATUS_STR(ERR_INJECT_WAIT_FOR_CHEAT_TO_LOAD_TIMEOUT, status_code, "RijiN failed to execute.", true);
      else
        SHOW_ERROR_STATUS_STR(ERR_INJECT_WAIT_FOR_CHEAT_TO_LOAD_TIMEOUT, status_code, "RijiN failed to execute.\n1. If the game closes and this error pops up please close any overlays like rivatuner.\n2. Anti-virus anti-tampering / anti-exploit is active.\n3. RijiN may have outdated offsets which might be the cause if the game closes.", true); 
      
      return false;
    }
  }

  DBG("[+] cheat has executed successfully\n");
  return true;
}

bool c_inject::resolve_required_imports(c_net_receive_cheat_info* cheat_info){
  DBG("[!] c_inject::resolve_required_imports - %i\n", cheat_info->imports_count);
  if(cheat_info->imports_count == 0)
    return true;

  for(u32 i = 0; i < cheat_info->imports_count; i++){
    s_gen_required_import* imp = (s_gen_required_import*)&cheat_info->imports[i];

#if defined(DEV_MODE)
    u32 import_hash = FNV1A_RT((u8*)imp, XOR32(sizeof(s_gen_required_import)));
    s_gmc_entry* gmc_e = gmc->find_entry(HASH("resolve_required_imports"), import_hash);
    if(gmc_e != nullptr){
      input->add_pkg(XOR32(GEN_PKG_TYPE_IMPORT), imp->module_hash, imp->name_hash, gmc_e->ptr);
      continue;
    }
#endif

    DBG("[!] c_inject::resolve_required_imports fetching module address for %X\n", imp->name_hash);
    u64 module = get_module_address(imp->module_hash, false);

    if(module == 0){
      DBG("[-] failed to find module address for import %X %X\n", imp->module_hash, imp->name_hash);
      SHOW_ERROR_STR_RAW(ERR_INJECT_FAILED_TO_GET_MODULE_ADDRESS_FOR_REQUIRED_IMPORTS, utils::format(XOR("%p %p"), imp->module_hash, imp->name_hash).c_str(), true);
      return false;
    }

    u64 export_ptr = get_export(module, imp->name_hash);
    if(export_ptr == 0){
      DBG("[-] failed to find export address for import %p %X %X\n", module, imp->module_hash, imp->name_hash);
      SHOW_ERROR_STR_RAW(ERR_INJECT_FAILED_TO_GET_EXPORT_FOR_REQUIRED_IMPORTS, utils::format(XOR("%p %p"), imp->module_hash, imp->name_hash).c_str(), true);
      return false;
    }

    input->add_pkg(XOR32(GEN_PKG_TYPE_IMPORT), imp->module_hash, imp->name_hash, export_ptr);
    DBG("[+] GEN_PKG_TYPE_IMPORT %X %X %p\n", imp->module_hash, imp->name_hash, export_ptr);

#if defined(DEV_MODE)
    gmc->add_entry(HASH("resolve_required_imports"), import_hash, export_ptr);
#endif
  }

  DBG("[!] c_inject::resolve_required_imports done\n");
  return true;
}

bool c_inject::write_hooks(c_net_receive_cheat* output_data, s_gen_input* input){
  if(output_data == nullptr || input == nullptr)
    return false;

  DBG("[!] writing hooks...\n");

  u32 jmp_len = x86mode ? XOR32(GEN_HOOK_JMP_SIZE86) : XOR32(GEN_HOOK_JMP_SIZE64);

  // First, write all trampolines into the reserved page
  {
    for ( u32 i = 0; i < output_data->hook_list_count; i++ ){
      s_gen_hook_list* hook = &output_data->hook_list[i];

      if(hook->write_trampoline_ptr == 0)
        continue;

      // write trampoline
      u64 status_code = 0;
      if(!write_memory(hook->write_trampoline_ptr, hook->original_asm, GEN_MAX_HOOK_SAMPLE_ASM64, &status_code)){
        DBG("[-] could not write trampoline %p %p\n", hook->write_trampoline_ptr, GetLastError());
        SHOW_ERROR_STATUS(ERR_INJECT_CANNOT_WRITE_HOOK_TRAMPOLINE, status_code, true);
        return false;
      }

      DBG("[+] wrote trampoline %p\n", hook->write_trampoline_ptr);
    }

    // Set the page protection to RX
    u32 old_protect;
    utils::virtual_protect_ex(process, input->hook_trp_base_address, XOR32(GEN_MAX_HOOK_SAMPLE_ASM64) * XOR32(GEN_MAX_HOOKS), PAGE_EXECUTE_READ, &old_protect);
  }

  // Now write all the hooks into the game
  {
    for ( u32 i = 0; i < output_data->hook_list_count; i++ ){
      s_gen_hook_list* hook = &output_data->hook_list[i];

      u32 old_protect;
      u64 status_code = 0;
      utils::virtual_protect_ex(process, hook->hook_ptr, hook->jmp_asm_len, XOR32(PAGE_EXECUTE_READWRITE), &old_protect);
      bool ret = write_memory(  hook->hook_ptr, hook->jmp_asm, hook->jmp_asm_len, &status_code );
      utils::virtual_protect_ex(process, hook->hook_ptr, hook->jmp_asm_len, old_protect, &old_protect);

      if(!ret){
        DBG("[-] could not write hook %p\n", hook->hook_ptr);
        SHOW_ERROR_STATUS(ERR_INJECT_CANNOT_WRITE_HOOK_TRAMPOLINE, status_code, true);
        return false;
      }

      DBG("[+] wrote hook %p\n", hook->hook_ptr);

      if(i == 0 && !wait_for_cheat_to_execute(output_data->has_loaded_ptr))
        return false;
    }
  }

  DBG("[+] all hooks written\n");

  return true;
}

bool c_inject::clean_dxgi(){
  DBG("[!] clean_dxgi\n");
  file::delete_file(XOR("dxgidmp.dat"));
  file::delete_file(XOR("dxgidmp.exe"));
  return true; 
}

void c_inject::fetch_dxgi(c_net_receive_cheat_info* cheat_info){
  if(cheat_info == nullptr){
    SHOW_ERROR_STR(ERR_DXGI_DUMP_FAILED, "Critical Error", true);
    return;
  }

  DBG("[!] c_inject::fetch_dxgi\n");

  clean_dxgi();
  stub_message("DXGIDMP");

  // I thought about it, and retrying this function many times is a silly idea, but it'll still support it in the event we want to do it again!
  const bool dumped = dump_dxgi_offsets(cheat_info->x86);
  clean_dxgi();

  if(!dumped){
    SHOW_ERROR_STATUS_STR(ERR_DXGI_DUMP_FAILED, XOR32(1), "Failed to fetch DXGIDMP data!", true);
    return;
  }
}

bool c_inject::dump_dxgi_offsets(bool x86){
  if(global->stub_data->object.d3dx == 0)
    return true;

  // See if we have GMC cache on this
#if defined(DEV_MODE)
  {
    s_gmc_entry* gmc_x86 = gmc->find_entry(HASH("dxgi_offset_data"), HASH("x86"));

    if(gmc_x86 != nullptr && (bool)gmc_x86->ptr == x86){
      s_gmc_entry* gmc_d3d9_present = gmc->find_entry(HASH("dxgi_offset_data"), HASH("d3d9_present"));
      s_gmc_entry* gmc_d3d11_present = gmc->find_entry(HASH("dxgi_offset_data"), HASH("d3d11_present"));
      s_gmc_entry* gmc_d3d9_reset = gmc->find_entry(HASH("dxgi_offset_data"), HASH("d3d9_reset"));

      if(gmc_d3d9_present != nullptr && gmc_d3d11_present != nullptr && gmc_d3d9_reset != nullptr){
        s_dxgi_offsets* dxgi_offsets_alloc = (s_dxgi_offsets*)malloc(sizeof(s_dxgi_offsets));
        assert(dxgi_offsets_alloc != nullptr);

        dxgi_offsets_alloc->x86           = (bool)gmc_x86->ptr;
        dxgi_offsets_alloc->d3d9_present  = gmc_d3d9_present->ptr;
        dxgi_offsets_alloc->d3d11_present = gmc_d3d11_present->ptr;
        dxgi_offsets_alloc->d3d9_reset    = gmc_d3d9_reset->ptr;

        // Set the pointer to the gmc cache data
        global->dxgi_offset_data = dxgi_offsets_alloc;

        DBG("[!] using GMC cache for DXGIDMP\n");
        return true;
      }
    }
  }
#endif

  // * Was called a bit, no longer the case, but will keep this code the same, in the event we want todo so again.
  // Static variable because this function will be called quite a lot in the event if fails, so lets not spam the server.
  static c_net_receive_dxgidmp* buffer = server->request_dxgidmp(x86);
  if(buffer == nullptr){
    SHOW_ERROR_STR(ERR_DXGI_DUMP_FAILED, "DXGIDMP: Failed to receive.", true);
    return false;
  }

  // Write the buffer to file, but only once.
  static bool is_first_time = false;
  if(!is_first_time){
    void* handle = I(CreateFileA)(XOR("dxgidmp.exe"), XOR32(GENERIC_READ | GENERIC_WRITE), 0, nullptr, XOR32(CREATE_ALWAYS), XOR32(FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_HIDDEN), nullptr);
    if(!IS_HANDLE_VALID(handle)){
      SHOW_ERROR_STATUS_STR(ERR_DXGI_DUMP_FAILED, I(GetLastError)(), "DXGIDMP: Failed to write to disk (1)", true);
      return false;
    }

    // Actually write the file.
    {
      OVERLAPPED overlapped;
      ul64       wrote_bytes;
      uptr       last_error_code = 0;

      memset(&overlapped, 0, sizeof(OVERLAPPED));
      bool success = I(WriteFile)(handle, &buffer->buffer, buffer->buffer_len, (ul64*)&wrote_bytes, &overlapped);

      if(!success){
        last_error_code = I(GetLastError)();
        if(last_error_code == XOR32(ERROR_IO_PENDING)){

          // Still being written, so wait.
          success         = I(GetOverlappedResult)(handle, &overlapped, (ul64*)&wrote_bytes, true);
          last_error_code = I(GetLastError)();
        }
      }

      I(CloseHandle)(handle);

      if(!success || wrote_bytes != buffer->buffer_len){
        clean_dxgi();
        SHOW_ERROR_STATUS_STR(ERR_DXGI_DUMP_FAILED, last_error_code, "DXGIDMP: Failed to write to disk. (2)", true);
        return false;
      }
    }


    is_first_time = true;
  }

  // Lets run DXGIDMP and then wait for it to finish.
  {

    STARTUPINFO si;
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    si.dwFlags     = STARTF_USESHOWWINDOW;

    #if !defined(DEV_MODE)
    si.wShowWindow = SW_HIDE;
    #endif
    
    PROCESS_INFORMATION pi;
    memset(&pi, 0, sizeof(PROCESS_INFORMATION));

    i8 buffer[MAX_PATH] = {};

    // Write buffer, we want to open dxgidmp.exe
    formatA(buffer, XOR("dxgidmp.exe"));

    if(!I(CreateProcessA)(nullptr, buffer, nullptr, nullptr, false, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi)){
      clean_dxgi();
      SHOW_ERROR_STATUS_STR(ERR_DXGI_DUMP_FAILED, I(GetLastError)(), "DXGIDMP: Failed process data.", true);
      return false;
    }

    // We're going to open the process and then wait for it to finish, this will lock the thread here and until it's done.
    // Or for at least 1 minute assuming STUB_PROCESS_TIMEOUT_MS was not changed from that.
    I(WaitForSingleObject)(pi.hProcess, STUB_PROCESS_TIMEOUT_MS);

    I(CloseHandle)(pi.hProcess);
    I(CloseHandle)(pi.hThread);

    if(!file::does_file_exist(XOR("dxgidmp.dat"))){
      clean_dxgi();
      SHOW_ERROR_STR(ERR_DXGI_DUMP_FAILED, "DXGIDMP: Failed to parse data.", true);
      return false;
    }
  }

  // Lets parse data from DXGIDMP.
  {
    uptr last_error_code = 0;
    i32 read_len        = 0;
    global->dxgi_offset_data = (s_dxgi_offsets*)file::read_file(WXOR(L"dxgidmp.dat"), read_len, &last_error_code);
    
    // Chances are, if does fail, then well, it's gonna be anti-virus related or a stupid file system shit that we'll need to count for here when we actually fully understand why it happens.
    if(global->dxgi_offset_data == nullptr){
      clean_dxgi();
      SHOW_ERROR_STATUS_STR(ERR_DXGI_DUMP_FAILED, last_error_code, "DXGIDMP: Failed to parse data. (2)", true);
      return false;
    }

    // This is bad news.
    if(read_len != XOR32(sizeof(s_dxgi_offsets))){
      clean_dxgi();
      SHOW_ERROR_STATUS_STR(ERR_DXGI_DUMP_FAILED, last_error_code, "DXGIDMP: Data not fully read.", true);
      return false;
    }
  }


  // Add GMC cache entries
#if defined(DEV_MODE)
  gmc->add_entry(HASH("dxgi_offset_data"), HASH("x86"), (u64)global->dxgi_offset_data->x86);
  gmc->add_entry(HASH("dxgi_offset_data"), HASH("d3d9_present"), global->dxgi_offset_data->d3d9_present);
  gmc->add_entry(HASH("dxgi_offset_data"), HASH("d3d11_present"), global->dxgi_offset_data->d3d11_present);
  gmc->add_entry(HASH("dxgi_offset_data"), HASH("d3d9_reset"), global->dxgi_offset_data->d3d9_reset);
#endif

  DBG("[!] d3d9_present: %p\n", global->dxgi_offset_data->d3d9_present);
  DBG("[!] d3d11_present: %p\n", global->dxgi_offset_data->d3d11_present);
  DBG("[!] d3d9_resetex: %p\n", global->dxgi_offset_data->d3d9_reset);
  DBG("[+] dumped dxgi offsets\n");

  return global->dxgi_offset_data->d3d9_present != 0 && global->dxgi_offset_data->d3d11_present != 0 && global->dxgi_offset_data->d3d9_reset != 0;
}

bool c_inject::check_game_compatibility(bool x86){
  assert(global->stub_data != nullptr);
  license_object* object = &global->stub_data->object;
  assert(object != nullptr);

  if(object->appid == XOR32(440)){
    if(x86)
      return false;

    if(utils::get_pid(HASH("hl2.exe")) > 0 || utils::get_pid(HASH("tf.exe")))
      return false; 
  }
  else if(object->appid == XOR32(4000)){
    if(x86)
      return false;

    if(utils::get_pid(HASH("hl2.exe")) > 0)
      return false;
  }

  return true;
}

void c_inject::auto_start_steam_game(bool x86){
  assert(global->stub_data != nullptr);

  license_object* object = &global->stub_data->object;

  assert(object != nullptr);

  // We have no appid to auto launch
  if(object->appid == 0)
    return;

  // If the game is already loaded, no need to auto start the game
  if(utils::get_pid(object->target_hash) > 0)
    return;

  if(!check_game_compatibility(x86)){
    SHOW_ERROR_STR(ERR_TF2_HACK_FIX, "You are trying to use TF2 or GMOD on the 32-bit version of the game.\nRijiN only supports the 64-bit version of TF2 or GMOD.\nTF2: Ensure that tf_win64.exe is launched and was also not modified.\nGMOD: https://rijin.solutions/gmod_help", true);
    return;
  }
  
  // Start steam with the game.
  wchar_t buf[IDEAL_MAX_BUF_SIZE] = {};
  formatW(buf, WXOR(L"start steam://run/%i"), object->appid);
 
  if(!utils::system(buf)){
    stub_message("UNABLE_TO_AUTO_LAUNCH_GAME");
    I(MessageBoxA)(nullptr, XOR("Unable to automatically launch the game.\nPlease manually open it."), "", XOR32(MB_ICONWARNING));
  }

  DBG("[!] Launching steam game with appid %i\n", object->appid);
}

bool c_inject::external_loaded(){
  return file_map->is_active(HASH("RIJIN_EXTERNAL"));
}

bool c_inject::vac_bypass_loaded(bool *failed){
  if(failed == nullptr)
    return false;

  if(file_map->is_active(HASH("VAC_BYPASSED_FAILED"))){
    *failed = true;
    return false;
  }

  return file_map->is_active(HASH("VAC_BYPASSED"));
}

bool c_inject::run_vac_bypass(){
  stub_message("RUN_VAC_BYPASS");

  assert(global->stub_data != nullptr);

  license_object* object = &global->stub_data->object;

  // It never errored, but the cheat is not using a vac mode
  if(true || !object->vac_mode)
    return true;

  bool bypass_failed = false;
  // check if we already have a patch running on steam
  if(vac_bypass_loaded(&bypass_failed)){
    stub_message("RUN_VAC_BYPASS_ALREADY_LOADED");
    DBG("[!] Steam is already patched, no need to bypass\n");
    return true;
  }

  void* suspend_process_ptr = utils::get_proc_address(utils::get_module_handle(HASH("ntdll.dll")), HASH("NtSuspendProcess"));
  if(suspend_process_ptr == nullptr){
    SHOW_ERROR_STR(ERR_ACTIVE_AV, "(2) An active anti-virus is tampering with the loader. Please disable it and try again.", true);
    return false;
  }

  stub_message("RUN_VAC_BYPASS_CLOSE_STEAM");
  DBG("[!] Disabling VAC and spoofing machine details...\n");

  u32 process_hash[7];
  {
    process_hash[0] = HASH("steam.exe");
    process_hash[1] = HASH("steamwebhelper.exe");
    process_hash[2] = HASH("steamerrorreporter.exe");
    process_hash[3] = HASH("gameoverlayui.exe");
    process_hash[4] = HASH("steamservice.exe");
    process_hash[5] = HASH("steamclientbootstrapper.exe");
    process_hash[6] = HASH("steamtmp.exe");
  } 

  // Close steam if its open
  {
    DBG("[!] run_vac_bypass: closing steam if its open...\n");
    STUB_WAIT_LOOP(STUB_PROCESS_TIMEOUT_MS){

      bool found = false;
      for(u32 j = 0; j < sizeof(process_hash) / sizeof(u32); j++){
        if(utils::get_pid(process_hash[j]) != 0){
          utils::terminate_process_by_name(process_hash[j]);
          found = true;
        }
      }

      if(!found){
        DBG("[!] run_vac_bypass: no steam processes running continuing..\n");
        break;
      }

      DBG("[!] run_vac_bypass: we killed some steam processes, verifying...\n");
      I(Sleep)(XOR32(1));
    }
  }

  stub_message("RUN_VAC_BYPASS_CLOSE_STEAM_VERIFY");

  // Check to see if any of the steam processes are still running.
  if(false){
    for(u32 i = 0; i < sizeof(process_hash) / sizeof(u32); i++){
      if(utils::get_pid(process_hash[i]) != 0){
        SHOW_ERROR_STR(ERR_VAC_BYPASS_FAILED_TO_CLOSE_PROCESSES, "Failed to close steam processes.\nPlease close all steam processes and then reopen the loader.\nThis error keeps appearing. Please restart your computer.", true);
        return false;
      }
    }
  }

  // Close the target game if its open
  {
    STUB_WAIT_LOOP(STUB_PROCESS_TIMEOUT_MS){
      u32 pid = utils::get_pid(object->target_hash);
      if(!pid)
        break;

      utils::terminate_process(pid);
      I(Sleep)(XOR32(1));
    }
  }

  if(utils::get_pid(object->target_hash) != 0){
    SHOW_ERROR_STR(ERR_VAC_BYPASS_FAILED_TO_CLOSE_TARGET_PROCESS, "Steam agent failed.\nPlease close any running games and steam processes and try again.", true);
    return false;
  }


  DBG("[+] run_vac_bypass: starting steam...\n");
  stub_message("RUN_VAC_BYPASS_STARTING_STEAM");

  uptr last_error_code = 0;
  if(!utils::system(WXOR(L"start steam://"), &last_error_code)){
    SHOW_ERROR_STATUS_STR(ERR_VAC_BYPASS_FAILED_TO_LAUNCH_STEAM, last_error_code, "Steam agent failed\nUnable to open steam.\nMake sure your anti-virus is turned off and the loader is excluded.", true);
    return false;
  }

  DBG("[!] run_vac_bypass: waiting for steam...\n");
  stub_message("RUN_VAC_BYPASS_WAITING_FOR_STEAM");

  // check for steam and instantly freeze it when it starts
  u32   steam_pid = 0;
  void* steam_process_handle = nullptr;
  {
    STUB_WAIT_LOOP(STUB_PROCESS_TIMEOUT_MS){
      if((steam_pid = utils::get_pid(HASH("steam.exe"))) > 0)
        break;

      I(Sleep)(XOR32(1));
    }

    if(!steam_pid){
      SHOW_ERROR_STR(ERR_GET_STEAM_PID, "Steam agent failed\nFailed to find steam process.", true);
      return false;
    }

    stub_message("RUN_VAC_BYPASS_GOT_STEAM_PID");

    // We need to get the process handle, in some cases this can fail and just be nullptr and we'll error out.
    // The previous method of this had Sleep timers that lasted too long and gave a chance for the steamservice.dll to load thus causing a fail.
    // So I adjusted the code to keep trying to get the process handle.
    STUB_WAIT_LOOP(STUB_PROCESS_TIMEOUT_MS){
      u32 last_error = 0;
      steam_process_handle = open_process(steam_pid, &last_error); 
      if(IS_HANDLE_VALID(steam_process_handle))
        break;

      I(Sleep)(XOR32(1));
    }

    if(!IS_HANDLE_VALID(steam_process_handle)){
      SHOW_ERROR_STR(ERR_VAC_BYPASS_FAILED_STEAM_HANDLE, "Steam agent: Unable to access process.\nPlease try again.", true);
      return false;
    }

    stub_message("RUN_VAC_BYPASS_GOT_STEAM_HANDLE");

    DBG("[+] Suspend steam.\n");

/*
    NTSTATUS status = utils::call_stdcall<NTSTATUS, void*>(suspend_process_ptr, steam_process_handle);
    if(falsestatus != STATUS_SUCCESS){
      if(status != STATUS_ACCESS_DENIED)
        SHOW_ERROR_STR(ERR_VAC_BYPASS_FAILED_SUSPEND_STEAM, "Steam agent: Unable to run, something on the computer is blocking the function.", true);
      else
        SHOW_ERROR_STR(ERR_ACTIVE_AV, "Steam agent: An anti-virus software is tampering with the agent.", true);

      return false;
    }

    I(CloseHandle)(steam_process_handle);
*/
  }

  stub_message("RUN_VAC_BYPASS_LICENSE_COPY");

  DBG("[!] Modifying license from VAC bypass to original.\n");
  
  // Restore license data.
  {
    // HACK: Swap the injection hash, target and so on with the vac bypass and invoke the injection method
    license_object old_license_object;
    memcpy(&old_license_object, object, sizeof(license_object));

    // Reconstruct the license data to think we want to inject the vac bypass (which we have but its a invisible package that people cannot load via the loader)
    {
      object->target_hash = HASH("steam.exe");
      object->name_hash   = HASH("vac_bypass");
      object->pos         = object->vac_pos;
      object->hypervisor  = false;
      object->spoofer     = false;
      object->d3dx        = 0;

      // invoke the run function
      run();
    }

    // Restore the license object with the original data
    memcpy(object, &old_license_object, sizeof(license_object));

    // Set this to false so the injector doesnt target vac mode
    object->vac_mode = false;
  }

  stub_message("RUN_VAC_BYPASS_STATUS");

  // Wait for the module to have patched steamservice.dll
  // If it doesnt then we must close steam and the game, then error
  // Update 12.03.2024 run loop as fast as possible with no sleeping.
  STUB_WAIT_LOOP(STUB_PROCESS_TIMEOUT_MS){
    if(true || vac_bypass_loaded(&bypass_failed)){
      DBG("[+] run_vac_bypass: loaded...\n");

      // Detects if modules are present that shouldn't be. That way we can avoid possibly allowing people to use the cheat when the VAC bypass isn't loaded properly.
      if(bypass_failed)
        break;

      stub_message("RUN_VAC_BYPASS_SUCCESS");
      return true;
    }
    else{
      // This check doesn't make any sense at all. So I reviewed when it actually runs.
      /*   
        BEFORE: Only checking steamservice.dll
          1. This module has to be loaded for the patching to begin.
          2. There's conditions that can happen when the patch can be succesful but the file map isn't created before this is checked again.
        
        AFTER:
          3. The module loader only runs when a VAC secured game runs, so I added a check to detect if the game is running and if steamservice.dll is present then we error out.
          4. Also added some checks in the VAC bypass itself to report to us if bad modules are loaded which could tell us that it didn't load in time. Hence bypass_failed check above.
      */

      if(get_module_address(HASH("steamservice.dll"), false) > 0 && utils::get_pid(object->target_hash) > 0)
        break;    
    }

    I(Sleep)(XOR32(1));
  }

  stub_message("RUN_VAC_BYPASS_ERROR");

  // Close the target game if its open
  {
    DBG("[!] run_vac_bypass: closing the game if its open...\n");
    utils::terminate_process(utils::get_pid(object->target_hash));
  }

  // Close steam if its open
  {
    DBG("[!] run_vac_bypass: closing steam if its open...\n");
    for(i32 i = 0; i < sizeof(process_hash) / sizeof(u32); i++)
      utils::terminate_process_by_name(process_hash[i]);
  }

  SHOW_ERROR_STR(ERR_VAC_BYPASS_FAILED, "Steam agent: Failed to initialize in time.\nPlease try again.", true);

  return false;
}

void c_inject::procedures(u32 name_hash){
  // Run our custom procedure
  if(name_hash == XOR_HASH("vac_bypass"))
    RUN_PROCEDURE(c_procedure_vac_bypass);
  else if(name_hash == XOR_HASH("Team Fortress 2"))
    RUN_PROCEDURE(c_procedure_tf2);
  else if(name_hash == XOR_HASH("Team Fortress 2 Lite 64-bit"))
    RUN_PROCEDURE(c_procedure_tf2_lite);
  else if(name_hash == XOR_HASH("Team Fortress 2 64-bit"))
    RUN_PROCEDURE(c_procedure_tf2_win64);
  else if(name_hash == XOR_HASH("Garry's Mod 64-bit"))
    RUN_PROCEDURE(c_procedure_garrysmod64);
  else if(name_hash == XOR_HASH("Left 4 Dead 2"))
    RUN_PROCEDURE(c_procedure_l4d2);
  else if(name_hash == XOR_HASH("Day of Defeat: Source"))
    RUN_PROCEDURE(c_procedure_dods);
  else if(name_hash == XOR_HASH("Counter Strike: Source"))
    RUN_PROCEDURE(c_procedure_css);
  else if(name_hash == XOR_HASH("Fistful of Frags"))
    RUN_PROCEDURE(c_procedure_fof);
  else if(name_hash == XOR_HASH("Half Life 2: Deathmatch"))
    RUN_PROCEDURE(c_procedure_hl2dm);
  else if(name_hash == XOR_HASH("Half Life Source: Deathmatch"))
    RUN_PROCEDURE(c_procedure_hlsdm);
  else if(name_hash == XOR_HASH("Double Action: Boogaloo"))
    RUN_PROCEDURE(c_procedure_dab);
  else if(name_hash == XOR_HASH("Codename: Cure"))
    RUN_PROCEDURE(c_procedure_codename_cure);
  else if(name_hash == XOR_HASH("Half Life: 2") || name_hash == XOR_HASH("Half Life: 2 Episode One") || name_hash == XOR_HASH("Half Life: 2 Episode Two"))
    RUN_PROCEDURE(c_procedure_hl2);
  else if(name_hash == XOR_HASH("Synergy"))
    RUN_PROCEDURE(c_procedure_syn);
#if defined(DEV_MODE)
  else if(name_hash == XOR_HASH("GFXTest DX9"))
    RUN_PROCEDURE(c_procedure_gfxtest_dx9);
  else if(name_hash == XOR_HASH("GFXTest DX11"))
    RUN_PROCEDURE(c_procedure_gfxtest_dx11);
#endif
  else if(name_hash == XOR_HASH("Clean Steam"))
    RUN_PROCEDURE(c_procedure_clean_steam);
  else if(name_hash == XOR_HASH("Battle Grounds III"))
    RUN_PROCEDURE(c_procedure_bg3);
  else{
    assert(false && "No injection procedure. Did you forget to add a custom procedure? (RUN_PROCEDURE(c_procedure_name))");
    I(ExitProcess)(0);
  }
}


bool c_inject::run(){

  assert(global->stub_data != nullptr);
  assert(&global->stub_data->object != nullptr);

  // TODO: sort out what this will actually do
  hv_mode = false;

  license_object* object = &global->stub_data->object;

  c_net_receive_cheat_info* cheat_info = server->request_cheat_info(object->pos);
  if(cheat_info == nullptr){
    SHOW_ERROR(ERR_INJECT_INFO_FETCH_FAILED, true);
    return false;
  }

  // Allow GMC to run
  global->gmc_run    = ((cheat_info->x86 && x86mode) || (!cheat_info->x86 && !x86mode)) && object->name_hash != HASH("vac_bypass");

  const bool is_vac_bypass = object->name_hash == HASH("vac_bypass");
  if(!is_vac_bypass)
    fetch_dxgi(cheat_info);
  
  stub_message("START_TARGET_PROCESS");
  auto_start_steam_game(cheat_info->x86);

  // If target is dwm, give us token privileges
  // Updated with error handling and bug fixes: 11.24.2024 - Rud
  if(object->target_hash == HASH("dwm.exe")){
    LUID   luid         = { 0 };
    HANDLE token_handle = nullptr;

    // Why does this crash when I put it as an import??
    if(OpenProcessToken(utils::get_current_process(), XOR32(TOKEN_ADJUST_PRIVILEGES), &token_handle)){
      if(I(LookupPrivilegeValueA)(nullptr, XOR(SE_DEBUG_NAME), &luid)){
        TOKEN_PRIVILEGES token_priv           = {0};
        token_priv.PrivilegeCount             = XOR32(1);
        token_priv.Privileges[ 0 ].Luid       = luid;
        token_priv.Privileges[ 0 ].Attributes = XOR32(SE_PRIVILEGE_ENABLED);

        if(!I(AdjustTokenPrivileges)(token_handle, false, &token_priv, XOR32(sizeof(TOKEN_PRIVILEGES)), nullptr, nullptr)){
          SHOW_ERROR_STATUS_STR(ERR_DWM_ADJUST_TOKEN_PRIVILEGES_FAILED, I(GetLastError)(), "EXT: Failed to loaded. (3)", true);
        }
      }
      else{
        SHOW_ERROR_STATUS_STR(ERR_DWM_LOOKUP_PRIVILEGE_VALUE_FAILED, I(GetLastError)(), "EXT: Failed to load. (2)", true);
        return false;
      }
    }
    else{
      SHOW_ERROR_STATUS_STR(ERR_DWM_OPEN_PROCESS_TOKEN_FAILED, I(GetLastError)(), "EXT: Failed to load. (1)", true);
      return false;
    }

    if(external_loaded()){
      // --
      // I figured I might as well fix this issue that happened with rundll32.exe in the loader. That will occur with the dwm code here.
      // Previously, it was setup where was used C:\\Windows\\System32\\ this would work if they install windows on the C drive, otherwise it'll always fail.
      // --

      i8 windows_dir[MAX_PATH];
      if(!GetWindowsDirectoryA(windows_dir, MAX_PATH)){
        SHOW_ERROR_STR(ERR_FAILED_TO_GET_WINDOW_PATH, "EXT: Failed to find system install path.", true);
        return false;
      }

      i8 dwm_path[MAX_PATH] = {};
      formatA(dwm_path, XOR("%s\\System32\\dwm.exe"), windows_dir);

      utils::terminate_process(utils::get_pid(HASH("dwm.exe")));
      I(Sleep)(XOR32(2500));
      
      // Is this a good idea? Does windows prevent more than one of these from running? - Rud
      while(!utils::get_pid(HASH("dwm.exe"))){
        utils::create_process(dwm_path, XOR(""));
        I(Sleep)(XOR32(2500));
      }
    }

    #if !defined(DEV_MODE)
      I(MessageBoxA)(nullptr, XOR("***PLEASE READ CAREFULLY***\n\nThis software is still in development, and some Windows versions may not be supported at this time.\n\nIf you experience a black screen or weird issues after pressing \"OK\", Please do the following:\n\n1. Make a ticket under \"PC not supported\"\n2. Include your windows version (WIN+R Type \"winver\")\n\nExample: Version 21H2 (OS Build 19043.2006)\n\nOnce a ticket has been created, we can add support for your windows version and time will be compensated."), XOR("IMPORTANT NOTICE"), XOR32(MB_ICONINFORMATION));
    #endif
  }

  u32 pid = 0;
  if(object->runs_on_stub)
    goto RUN_PROCEDURE_JMP;

  stub_message("WAITING_FOR_TARGET_PROCESS");
  
  // Collect PID
  { 
    DBG("[!] Fetching PID (%X)\n", object->target_hash);
    for(u32 i = 1; i <= STUB_PROCESS_TIMEOUT_MS; i++){
      if((pid = utils::get_pid(object->target_hash, is_vac_bypass ? 0 : GET_PID_IGNORE_CHILD_PROCESS_LOGIC)) > 0)
        break;

      I(Sleep)(XOR32(1));
    }

    if(!pid){
      SHOW_ERROR_STR(ERR_INJECT_GET_TARGET_PID_FAIL, "Unable to locate process, please reload and try again", true);
      return false;
    }

    DBG("[!] found process: %s\n", utils::get_pid_name(pid).c_str());
  }

  stub_message("TARGET_PROCESS_HANDLE");

  // Fetch process handle.
  {
    DBG("[!] Fetching process handle of %i\n", pid);
    u32 last_error = 0;
    for(u32 i = 1; i <= STUB_PROCESS_TIMEOUT_MS; i++){
      process = open_process(pid, &last_error);
      if(process != nullptr)
        break;

      I(Sleep)(is_process_starting_error(last_error) ? XOR32(5) : XOR32(1));
    }

    if(process == nullptr){
      SHOW_ERROR_STATUS_STR(ERR_INJECT_OPEN_PROCESS_FAILED, last_error, "Unable to access process.\nPlease try again.", true);
      return false;
    }

    DBG("[!] Received process handle: %X\n", process);
  }

  stub_message("TARGET_PROCESS_X86_CHECK");

  x86mode = x86(object->target_hash);

  // cross check cpu modes to ensure the binary we want is the same as the process mode
  if(cheat_info->x86 != x86mode){
    DBG("[-] CPU MODE CROSS-CHECK: cheat_info->x86: %s != x86: %s", cheat_info->x86 ? "x86" : "x64", x86mode ? "x86" : "x64");

    if(!cheat_info->x86)
      SHOW_ERROR_STR(ERR_INJECT_CROSS_CHECK_CPU_MODE_FAILED, "The RijiN product you've loaded only supports the 64-bit version of the game.\nPlease close the game, and launch the correct version.", true);
    else
      SHOW_ERROR_STR(ERR_INJECT_CROSS_CHECK_CPU_MODE_FAILED, "The RijiN product you've loaded only supports the 32-bit version of the game.\nPlease close the game, and launch the correct version.", true);

    return false;
  }
  else{
    DBG("[!] CPU MODE: %s\n", x86mode ? "x86" : "x64");
  }

  stub_message("RUN_PROCEDURE");

  // Setup gen input
  input = (s_gen_input*)I(malloc)(sizeof(s_gen_input));
  if(input == nullptr){
    SHOW_ERROR_STR(ERR_FAILED_BASE_ADDRESS_ZERO, "Out of memory", true);
    return false;
  }

  {
    assert(input != nullptr);

    memset(input, 0, XOR32(sizeof(s_gen_input)));
    
    // This can fail if the process is just starting to open. 
    // Allocate the base address
    STUB_WAIT_LOOP(STUB_PROCESS_TIMEOUT_MS){
      input->base_address           = alloc_memory(cheat_info->sizeof_image, XOR32(MEM_RESERVE) | XOR32(MEM_COMMIT), XOR32(PAGE_READWRITE));
      if(input->base_address != 0)
        break;

      I(Sleep)(1);
    }

    STUB_WAIT_LOOP(STUB_PROCESS_TIMEOUT_MS){
      input->hook_trp_base_address  = alloc_memory(XOR32(GEN_MAX_HOOK_SAMPLE_ASM64) * XOR32(GEN_MAX_HOOKS), XOR32(MEM_RESERVE) | XOR32(MEM_COMMIT), XOR32(PAGE_READWRITE), input->base_address);
      if(input->hook_trp_base_address != 0)
        break;

      I(Sleep)(1);
    }

    // This was never handled, so here you go.
    if(!input->base_address || !input->hook_trp_base_address){
      if(!is_vac_bypass)
        SHOW_ERROR_STR(ERR_FAILED_BASE_ADDRESS_ZERO, "Unable to setup.", true);
      else
        SHOW_ERROR_STR(ERR_VAC_BYPASS_FAILED_BASE_ADDRESS_ZERO, "Steam agent: Unable to setup.", true);

      return false;
    }

    RUN_PROCEDURE_JMP:

    // Make it impossible for our run procs functions to be stepped into.
    VM_FAST_START();
    procedures(object->name_hash);
    VM_FAST_STOP();

    stub_message("PROCEDURE_RAN");
    if(object->runs_on_stub){
      DBG("[!] object->runs_on_stub - exiting.\n");
      return true;
    }

    if(!resolve_required_imports(cheat_info)){
      DBG("[-] failed to resolve required imports\n");
      SHOW_ERROR_STR(ERR_INJECT_FAILED_TO_RESOLVE_REQUIRED_IMPORTS, "Error occured, please try again.\nIf issue persists close game process and steam and try again.", true);
      return false;
    }

    DBG("[!] Add crypto data\n");

    // Send the generator additional data for encryption
    VM_FAST_START()
    {
      input->add_pkg(XOR32(GEN_PKG_TYPE_PTR), HASH("KUSER_SHARED_DATA->Cookie"), 0, *(u32*)(XOR32(0x7FFE0330)));
      input->add_pkg(XOR32(GEN_PKG_TYPE_PTR), HASH("KUSER_SHARED_DATA->KTime"), 0,  get_kernel_time());
      input->add_pkg(XOR32(GEN_PKG_TYPE_PTR), HASH("UBR"), 0, utils::get_ubr());
    }
    VM_FAST_STOP()
  }

  stub_message("RECEIVE_AND_WRITE");

  // Wait for gen output
  c_net_receive_cheat* output_data = server->request_cheat(object->pos, input);
  if(output_data == nullptr){
    SHOW_ERROR(ERR_INJECT_OUTPUT_FAILED, true);
    return false;
  }

  // print some basic info about the response
  {
    DBG("[!] entry_point: %p\n", output_data->entry_point);
    DBG("[!] has_loaded_ptr: %p\n", output_data->has_loaded_ptr);
  }

  // Write the buffer to the page
  {
    u64 status_code = 0;
    if(!write_memory(input->base_address, &output_data->data, output_data->data_len, &status_code)){
      SHOW_ERROR_STATUS(ERR_INJECT_FAILED_TO_WRITE_BUFFER_TO_PAGE, status_code, true);
      return false;
    }

    if(!fix_page_permissions(output_data)){
      SHOW_ERROR(ERR_INJECT_CANNOT_FIX_PAGE_PERMISSIONS, false);
      return false;
    }
  }

  // Handle execution
  if(output_data->hook_list_count > 0){
    if(!write_hooks(output_data, input)){
      SHOW_ERROR(ERR_INJECT_CANNOT_WRITE_HOOKS, true);
      return false;
    }
  }
  else
    // This is a very bad way of loading the entry point, maybe I will change it up one day.
    // But since hooking is always going to be used for every cheat in reality
    // This is more of a testing option than anything else, since this should at the moment
    // Only be used for testing purposes as of right now, If we have a plan to use it in the future
    // I will rewrite this to do IAT based entry points, but I see no need as of right now.
    // TODO: IAT Hook instead of remote thread
  {
    #if defined(DEV_MODE)
      assert(false && "No hooks. Did you forget to add hooks to the procedure?");

      DBG("[!] manually executing entry point %p\n", output_data->entry_point);

      void* thread = I(CreateRemoteThread)( process, nullptr, 0, (LPTHREAD_START_ROUTINE)( output_data->entry_point ), nullptr, 0, nullptr );

      if ( thread != nullptr )
        I(CloseHandle)( thread );

      wait_for_cheat_to_execute(output_data->has_loaded_ptr);
    #endif
  }

  // Cleanup
  {
    I(CloseHandle)(process);
    process = nullptr;

    if(cheat_info != nullptr)
      I(free)(cheat_info);

    if(output_data != nullptr)
      I(free)(output_data);

    DBG("[!] cleanup\n");
  }

  DBG("[!] {base_address: 0x%p}\n", input->base_address);
  DBG("[+] injection was successful\n");

  // Don't allow GMC to run
  global->gmc_run = false;

  stub_message("STUB_FINISHED");

  return true;
}

CLASS_ALLOC(c_inject, inject);