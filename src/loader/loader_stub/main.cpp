#include "link.h"

i32         error_code        = 0;
i32         error_line        = 0;
ul64        error_status_code = 0;
std::string error_msg         = "";

// We do this to get the compiler to create a import entry
// This function in reality will never be called
void import_trick(){
  // USER32.dll
  MessageBoxA(nullptr, nullptr, nullptr, 0);
  
  // KERNEL32.dll
  CloseHandle(nullptr);

  // Sheel32.dll
  ShellExecuteExA(nullptr);

  // Advapi32.dll
  ObjectDeleteAuditAlarmA(nullptr, nullptr, false);

  RtlCopyMemory(nullptr, nullptr, 0);
}

static u32 __stdcall license_timeout_worker_thread(c_stub_data* p){
  assert(p != nullptr);

  while(p->license_expire_seconds-- > 0){
#if defined(DEV_MODE)
    if(p->license_expire_seconds % 10 == 0 || p->license_expire_seconds <= 10)
      DBG("[!] License expires in %i seconds...\n", p->license_expire_seconds);
#endif

    I(Sleep)(1000);
  }

  I(exit)(0);

  return 0;
}

EXPORT void __stub(HWND hwnd, void* inst, i8* cmd_line, i32 cmd_show){
  VM_FAST_START()
  jm::init_syscalls_list();
  if(cmd_line == nullptr)
    return 0;

  VM_FAST_STOP()

  LISTEN_FOR_ERRORS();

#if defined(DEV_MODE)
  _iobuf* data;
  
  AllocConsole( );
  freopen_s( &data, XOR( "CONIN$" ), XOR( "r" ), stdout );
  freopen_s( &data, XOR( "CONOUT$" ), XOR( "w" ), __acrt_iob_func( 1 ) );
    
  DBG("[!] stub - build %s\n", __DATE__);

  //system("color 1f");
#endif

  //rtp::send_challenge();

  WANTS_TIME_CHECKS();
  TIME_CHECK_START();

  // 1. Collect and check for the stub data
  // If it has not been passed to the stub then we will silently exit
  {
    global->stub_data = stub->get_data(cmd_line);
    if(global->stub_data == nullptr)
      return 0;
  }
  TIME_CHECK_END(2.f);

  // Create the timeout worker
  if(!utils::create_worker(license_timeout_worker_thread, global->stub_data)){
  }

  rtp::debugger_check();

  c_net_receive_info* info = nullptr;
  {
    for(u32 i = 0; i < XOR32(5); i++){
      if((info = server->get_serverinfo()) != nullptr)
        break;

      I(Sleep)(XOR32(1000));
    }

    if(info == nullptr)
      SHOW_ERROR_STR(ERR_TIMEOUT, "A connection to the master server cannot be established.", true);
  }

  file_map->create(HASH("RIJIN_STUB"));

  // 2. Print out some information about the cheat we are planning to load
  {
    DBG("[!] POS:\t %i\n", global->stub_data->object.pos);
    DBG("[!] NAME:\t \"%s\"\n", global->stub_data->object.name);
    DBG("[!] EXPIRE:\t %i\n", global->stub_data->object.expire);
    DBG("[!] DISABLED:\t %i\n", global->stub_data->object.disabled ? 1 : 0);
    DBG("[!] HYPERVISOR:\t %i\n", global->stub_data->object.disabled ? 1 : 0);
    DBG("[!] SPOOFER:\t %i\n", global->stub_data->object.spoofer ? 1 : 0);
    DBG("[!] VAC_MODE:\t %i\n", global->stub_data->object.vac_mode ? 1 : 0);
    DBG("[!] VAC_POS:\t %i\n", global->stub_data->object.vac_pos);
    DBG("[!] APPID:\t %i\n", global->stub_data->object.appid);
    DBG("[!] D3DX:\t %i\n", global->stub_data->object.d3dx);
  }

  VM_FAST_START();
  rtp::runtime_protection();

  inject->run_vac_bypass();
  inject->run();
  VM_FAST_STOP();

  return 0;
}
