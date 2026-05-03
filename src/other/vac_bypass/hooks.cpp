#include "link.h"

static void _status_message(u32 c){
  // CreateThread could be used as a breakpoint for reversers. But they could just use the network messages also.
  // Thought about detecting hooks or breakpoints in here but I don't know if that would have any false positives or not.
  static u32* what = (u32*)malloc(sizeof(u32));
  if(what == nullptr){
    send_status_code(HASH("VAC BYPASS"), c);
    return;
  }

  static bool talking = false;
  *what = c;
  utils::create_worker([](void* ptr){
    u32 code = *(u32*)ptr;
    while(talking) // avoid tripping any server-side ratelimits.
      I(Sleep)(50);

    talking = true;
    send_status_code(XOR32(HASH("VAC BYPASS")), code);
    talking = false;

    return 0;
  }, what);
}

#define status_message(x) _status_message(XOR32(HASH(x)));


bool setup(){
  static bool is_setup = false;
  if(is_setup)
    return true;

  status_message("VAC_BYPASS_INIT");

  // Setup trampoline addresses
  {
    global->get_system_info_hook_trp        = gen_internal->get_pkg(HASH("get_system_info_hook"));
    //global->dll_main_hook_trp               = gen_internal->get_pkg(HASH("dll_main_hook"));
  }

  // Tell the loader we are loaded
  gen_internal->loaded = true;

  is_setup = true;

  return true;
}

// Essentially when VAC scans pages, it will query GetSysteminfo.
// If GetSystemInfo does not return a page size thats aligned in array of 0x1000 bytes
// It does not run and instead returns false and the sig scan is not started
EXPORT void __fastcall get_system_info_hook(SYSTEM_INFO* info, void* rdx){
  setup();

  Beep(1000,1000);
  utils::call_fastcall64<void, SYSTEM_INFO*>(gen_internal->decrypt_asset(global->get_system_info_hook_trp), info, rdx);
  return;

  assert(info != nullptr);

  setup();

  DBG("[!] get_system_info_hook(0x%X)\n", info);

  utils::call_fastcall64<void, SYSTEM_INFO*>(gen_internal->decrypt_asset(global->get_system_info_hook_trp), info, rdx);

  // bypass pagesize == 0x1000 check
  if(info != nullptr)
    info->dwPageSize = 1;

  static bool once = false;
  if(!once){
    once = true;
    status_message("VAC_BYPASS_GET_SYSTEM_INFO_WORKS");
  }
}

EXPORT bool __stdcall dll_main_hook(void* instance, u32 reason, void* reserved){
  setup();

  // When the steam service becomes loaded
  // We will obtain a callback that the dll is about to have its entry point executed
  // Quickly perform a byte patch that will always make the code jump over the vac module loader
  // And then making it so that no vac modules load as steam starts
  VM_MUTATE_START()
  if(false){
    static bool has_patched = false;
    uptr steamservice_ptr = utils::get_module_handle(HASH("steamservice.dll"));

    if(!has_patched && steamservice_ptr != 0){
      status_message("VAC_BYPASS_PATCHING");
      // clear debugview buffer (kind of ghetto)
#if defined(DEV_MODE)
      for(u32 i = 0; i < 10; i++)
        DBG("\n");
#endif
      // Search for "Assertion Failed: pModule->m_pModule == NULL" and that is the function to loadvacmodule
      // Look for some code like "if ( (a2 & 2) != 0 )"
      // You want this code to always jump rather than on zero flag
      uptr jz_ptr = utils::find_signature(steamservice_ptr, SIG("\x74\x47\x6A\x01\x6A"), XOR32(0xA0000));

      assert(jz_ptr != 0);

      u32 old_protect;
      VirtualProtect(jz_ptr, XOR32(1), PAGE_EXECUTE_READWRITE, &old_protect);
      *(u8*)jz_ptr = XOR32(0xEB); // EB 00 | jmp imm8
      VirtualProtect(jz_ptr, XOR32(1), old_protect, &old_protect);

      has_patched = true;

      // Create the file mapping object to instruct the loader that vac was patched
      if(utils::get_module_handle(HASH("steamclient.dll")) == nullptr){
        status_message("VAC_BYPASS_PATCHED");
        file_map->create(HASH("VAC_BYPASSED"));
      }
      else{
        status_message("VAC_BYPASS_PATCH_TOO_LATE");
        file_map->create(HASH("VAC_BYPASSED_FAILED"));
      }

      DBG("[+] Patched LoadVacModule JZ to JMP at %p\n", jz_ptr);
    }
  }
  VM_MUTATE_STOP()

  return utils::call_stdcall<bool, void*, u32, void*>(gen_internal->decrypt_asset(global->dll_main_hook_trp), instance, reason, reserved);
}