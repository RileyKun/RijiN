#define STB_SPRINTF_IMPLEMENTATION
#include "link.h"

bool utils::get_module_info_from_address_space(uptr addr, MODULEENTRY32* entry){
  if(entry == nullptr)
    return false;

#if defined(INTERNAL_CHEAT)
  uptr base_address_start = gen_internal->base_address_start;
  uptr base_address_end   = gen_internal->base_address_end;

  #if defined(__x86_64__)
    base_address_start = gen_internal->base_address_start64;
    base_address_end   = gen_internal->base_address_end64;
  #endif
  
  // is address in our cheat page
  if(addr >= base_address_start && addr <= base_address_end){
    strcpy(entry->szModule, XOR("this"));
    entry->modBaseAddr = base_address_start;
    entry->modBaseSize = base_address_end - base_address_start;

    return true;
  }
#endif

  HANDLE handle = I(CreateToolhelp32Snapshot)(XOR32(0x00000008) | XOR32(0x00000010), I(GetCurrentProcessId)());
  if(!IS_HANDLE_VALID(handle))
    return false;

  MODULEENTRY32 module_entry = MODULEENTRY32{sizeof(MODULEENTRY32)};
  if(I(Module32First)(handle, &module_entry)){
    do{
      if(addr >= module_entry.modBaseAddr && addr <= module_entry.modBaseAddr + module_entry.modBaseSize){
        memcpy(entry, &module_entry, sizeof(MODULEENTRY32));
        return true;
      }

    }while(I(Module32Next)(handle, &module_entry));
  }

  I(CloseHandle)(handle);
  return false;
}

bool utils::get_process_modules(MODULEENTRY32* entry, u32 size, u32* c, uptr pid){
  u32 count = 0;
  HANDLE handle = I(CreateToolhelp32Snapshot)(XOR32(0x00000008) | XOR32(0x00000010), pid);
  if(!IS_HANDLE_VALID(handle))
    return false;

  MODULEENTRY32 module_entry = MODULEENTRY32{sizeof(MODULEENTRY32)};
  if(I(Module32First)(handle, &module_entry)){
    do{
      memcpy(&entry[count], &module_entry, sizeof(MODULEENTRY32));
      count++;
      if(count >= size)
        break;
    }
    while(I(Module32Next)(handle, &module_entry));
  }

  I(CloseHandle)(handle);
  *c = count;
  return count > 0;
}

#if defined(IMGUI_API)
i32 utils::format_string(i8* buf, u32 buf_size, const i8* fmt, ...){
    va_list args;
    va_start(args, fmt);
    i32 w = stbsp_vsnprintf(buf, (i32)buf_size, fmt, args);
    va_end(args);

    if (buf == 0)
        return w;

    if (w == -1 || w >= (i32)buf_size)
        w = (i32)buf_size - 1;

    buf[w] = 0;
    return w;
}
#endif

#if defined(HDE_DISASM) && !defined(NO_SECURITY)
ALWAYSINLINE bool utils::has_breakpoint(uptr* addr){
  if(addr == nullptr)
    return false;

  uptr op8 = *(u8*)((uptr)addr);
  uptr op16 = *(u16*)((uptr)addr);

  // BREAKPOINTS
  // 0xCC  = int 3
  // 0xCD  = long int 3
  // 0xBOF = UD2

  // Returns (Can be used to disable function calls)
  // 0xC2 = ret
  // 0xC3 = ret
  // 0x90 = nop

  //DBG("(%p): op8: %X, op16: %X\n", addr, op8, op16);
  return op8 == XOR32(0x90) || op8 == XOR32(0xCC) || op8 == XOR32(0xCD) || op16 == XOR32(0xB0F) || op8 == XOR32(0xC2) || op8 == XOR32(0xC3);
}

static bool utils::is_hooked(uptr* addr, bool never_possible){
  if(addr == nullptr)
    return false;

  // never_possible means that the function should never start with an E9 of FF25

  uptr op8 = *(u8*)((uptr)addr);
  uptr op16 = *(u16*)((uptr)addr);
  if(op8 == XOR32(0xE9) || op8 == XOR32(0xE8)){
    if(never_possible)
      return true;

    // UNTESTED
    hde32s h;
    hde32_disasm(addr, &h);
    u32 found_ptr = 0;
    if(h.flags & HDE32_F_DISP32)
      found_ptr += h.disp.disp32 + h.len;
    else if(h.flags & HDE32_F_IMM32 && h.imm.imm32 != 0)
      found_ptr += h.imm.imm32 + h.len;
    else if(h.flags & HDE32_F_IMM16 && h.imm.imm16 != 0)
      found_ptr += h.imm.imm16 + h.len;
    else if(h.flags & HDE32_F_IMM8 && h.imm.imm8 != 0)
      found_ptr += h.imm.imm8 + h.len;

    uptr new_addr = ((uptr)addr + (uptr)found_ptr);
    MODULEENTRY32 entry;
    if(!utils::get_module_info_from_address_space(new_addr, &entry))
      return true;

    if(!entry.szModule[0] || !entry.szExePath[0])
      return true;

    i8* module_name = str_utils::tolower(entry.szModule);
    i8* path_name   = str_utils::tolower(entry.szExePath);
    if(module_name == nullptr)
      return true;

    // Well this is clearly a debug module.
    if(str_utils::contains(module_name, XOR("dbg"))
      || str_utils::contains(module_name, XOR("debug"))
      || str_utils::contains(module_name, XOR("titanengine"))
      || str_utils::contains(module_name, XOR("ida"))
      || str_utils::contains(module_name, XOR("hook"))
      || str_utils::contains(module_name, XOR("scylla")))
      return true;

    if(str_utils::contains(path_name, XOR("download"))
      || str_utils::contains(path_name, XOR("desktop"))
      || str_utils::contains(path_name, XOR("dbg"))
      || str_utils::contains(path_name, XOR("debug"))
      || str_utils::contains(path_name, XOR("ida"))
      || str_utils::contains(path_name, XOR("dump")))
      return true;


    DBG("(Hook32) (%p): op8: %X, op16: %X, rel addr: %X\n", addr, op8, op16, found_ptr);
  }
  else if(op16 == XOR32(0x25FF)){
    if(never_possible)
      return true;

    // TESTED
    hde64s h;
    hde64_disasm(addr, &h);
    u64 found_ptr = 0;
    if(h.flags & HDE64_F_DISP32)
      found_ptr += h.disp.disp32 + h.len;
    else if(h.flags & HDE64_F_IMM32 && h.imm.imm32 != 0)
      found_ptr += (u64)h.imm.imm32 + h.len;
    else if(h.flags & HDE64_F_IMM16 && h.imm.imm16 != 0)
      found_ptr += (u64)h.imm.imm16 + h.len;
    else if(h.flags & HDE64_F_IMM8 && h.imm.imm8 != 0)
      found_ptr += (u64)h.imm.imm8 + h.len;

    uptr new_addr = ((uptr)addr + (uptr)found_ptr);
    MODULEENTRY32 entry;
    if(!utils::get_module_info_from_address_space(new_addr, &entry))
      return true;

    if(!entry.szModule[0] || !entry.szExePath[0])
      return true;

    i8* module_name = str_utils::tolower(entry.szModule);
    i8* path_name   = str_utils::tolower(entry.szExePath);
    if(module_name == nullptr)
      return true;

    // Well this is clearly a debug module.
    if(str_utils::contains(module_name, XOR("dbg"))
      || str_utils::contains(module_name, XOR("debug"))
      || str_utils::contains(module_name, XOR("titanengine"))
      || str_utils::contains(module_name, XOR("ida"))
      || str_utils::contains(module_name, XOR("hook"))
      || str_utils::contains(module_name, XOR("scylla")))
      return true;

    if(str_utils::contains(path_name, XOR("download"))
      || str_utils::contains(path_name, XOR("desktop"))
      || str_utils::contains(path_name, XOR("dbg"))
      || str_utils::contains(path_name, XOR("debug"))
      || str_utils::contains(path_name, XOR("ida"))
      || str_utils::contains(path_name, XOR("dump")))
      return true;

    DBG("(Hook64) (%p): op8: %X, op16: %X, rel addr: %p\n", addr, op8, op16, found_ptr);
    return false;
  }

  return false;
}
#endif

#if !defined(DISABLE_NETWORKING)
static bool utils::submit_error_code(u32 error_code, u32 status_code){
  if(!error_code)
    return false;

  c_net_submit_error_code submit_error_code{};
  submit_error_code.error_code  = error_code;
  submit_error_code.status_code = status_code;

  c_net_submit_error_code_response* response = (c_net_submit_error_code_response*)submit_error_code.transmit(true);
  if(response == nullptr)
    return false;

  return response->ok;
}
#endif

#if defined(INTERNAL_CHEAT)
static std::string utils::bytes_to_hex(u8* data, u32 len){
  assert(data != nullptr);
  static i8 hex_map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

  std::string s(len * 2, ' ');
  for(i32 i = 0; i < len; ++i){
    s[2 * i]     = hex_map[(data[i] & 0xF0) >> 4];
    s[2 * i + 1] = hex_map[data[i] & 0x0F];
  }
  return s;
}

static u8* utils::hex_to_bytes(i8* str){
  if(str == nullptr)
    return nullptr;

  u32 len  = strlen(str);
  u8* data = (u8*)malloc(len);
  memset(data, 0, len);

  for(u32 index = 0; index < len; index++){
    u8 c   = str[index];
    u8 val = 0;

    if(c >= '0' && c <= '9')
      val = (c - '0');
    else if(c >= 'A' && c <= 'F')
      val = (10 + (c - 'A'));
    else if(c >= 'a' && c <= 'f')
      val = (10 + (c - 'a'));
    else{
      free(data);
      return nullptr;
    }

    data[(index / 2)] += val << ((( index + 1) % 2) * 4);
  }

  return data;
}
#endif
static bool utils::get_process_hash_list(u32* table, u32* max_len){
  if(table == nullptr)
    return false;

  static nt_query_system_information_fn nt_query_system_information = (nt_query_system_information_fn)(utils::get_proc_address(utils::get_module_handle(HASH("ntdll.dll")), HASH("NtQuerySystemInformation")));

  ULONG len = 0;
  nt_query_system_information(SystemProcessInformation, nullptr, 0, &len);
  if(!len)
    return false;

  void* buffer = I(malloc)(len);
  memset(buffer, 0, len);

  if(buffer == nullptr){
    assert(false && "out of memory");
    return false;
  }

  NTSTATUS status = nt_query_system_information(SystemProcessInformation, buffer, len, nullptr);
  if(status != STATUS_SUCCESS){
    free(buffer);
    return false;
  }
  
  u32 i = 0;

  SYSTEM_PROCESS_INFORMATION* entry = (SYSTEM_PROCESS_INFORMATION*)buffer;
  while(entry->NextEntryOffset){
    if(entry->ImageName.Buffer == nullptr){
      entry = (SYSTEM_PROCESS_INFORMATION*)((u8*)entry + entry->NextEntryOffset);
      continue;
    }

    table[i] = WHASH_RTL(entry->ImageName.Buffer);
    i++;
    if(i >= *max_len){
      free(buffer);
      return true;
    }

    entry = (SYSTEM_PROCESS_INFORMATION*)((u8*)entry + entry->NextEntryOffset);
  }

  *max_len = i;

  free(buffer);
  return true;
}
