#include "../../link.h"
#include "rtp_methods.h"

volatile u32   rtp_tick_count = 1;
volatile float rtp_last_tick_update_time = 0.f;
volatile bool  rtp_transmitted_heartbeat = false;

DWORD __stdcall exec_test(void* param){
  rtp_tick_count += 2;
  return 0;
}

DWORD __stdcall suspended_thread(void* param){
  {
    #if !defined(LOADER_CLIENT)
      TRIP_SECURITY_REASON(RTP_REASON_SUSPENDED_THREAD_RAN);
    #else
       SAFE_EXIT();
    #endif
  }
  return 0;
}

// Purpose: If the process is suspended for too long then:
// A: A debugger was used.
// B: The user intentionally suspended the process.
// C: Which shouldn't occur due to AV detection safe guards in the loader.
DWORD __stdcall suspended_check_thread(void* param){
  float master_time = math::time(false, true);
  while(true){

    // If this occurs then the thread was suspended.
    float cur_time = math::time(false, true);
    if(math::abs(cur_time - master_time) > 3.f){
      #if !defined(LOADER_CLIENT)
        TRIP_SECURITY_REASON(RTP_REASON_PROCESS_PAUSED);
      #else
         SAFE_EXIT();
      #endif
      break;
    }

    master_time = cur_time;
    if(!IS_IMPORT_BREAKPOINT("kernel32.dll", "Sleep"))
      I(Sleep)(1);
    else{
      #if !defined(LOADER_CLIENT)
        TRIP_SECURITY_REASON(RTP_REASON_SLEEP_BP);
      #endif
    }
  }

  return 0;
}

DWORD __stdcall rtp_think(void* param){
  float heart_beat = 0.f;
  while(true){
    float time = math::time(false, true);

    rtp_tick_count++;
    rtp_last_tick_update_time = math::time(false, true);

    if(heart_beat <= time){
      heart_beat = time + 1.2f;
      #if defined(LOADER_CLIENT)
        bool success = send_status_code(HASH("LOADER"), HASH("HEARTBEAT"));
      #elif defined(LOADER_STUB)
        bool success = send_status_code(HASH("STUB"), HASH("HEARTBEAT"));
      #else
        bool success = send_status_code(HASH("PRODUCT"), HASH("HEARTBEAT"));
      #endif

      if(!rtp_transmitted_heartbeat)
        rtp_transmitted_heartbeat = success;

      if(!success)
        heart_beat = time + 0.25f;
    }

    #if defined(LOADER_STUB)
      if(IS_IMPORT_BREAKPOINT("user32.dll",   "MessageBoxA")            ||
         IS_IMPORT_BREAKPOINT("user32.dll",   "MessageBoxW")            ||
         IS_IMPORT_BREAKPOINT("kernel32.dll", "VirtualProtect")         ||
         IS_IMPORT_BREAKPOINT("kernel32.dll", "VirtualProtectEx")       ||
         IS_IMPORT_BREAKPOINT("kernel32.dll", "WriteProcessMemory")     ||
         IS_IMPORT_BREAKPOINT("kernel32.dll", "ReadProcessMemory")      ||
         IS_IMPORT_BREAKPOINT("kernel32.dll", "CreateProcessA")         ||
         IS_IMPORT_BREAKPOINT("kernel32.dll", "CreateProcessW")         ||
         IS_IMPORT_BREAKPOINT("kernel32.dll", "ExitProcess")            ||
         IS_IMPORT_BREAKPOINT("kernel32.dll", "Sleep")                  ||
         IS_IMPORT_BREAKPOINT("kernel32.dll", "CreateRemoteThread")     ||
         IS_IMPORT_BREAKPOINT("kernel32.dll", "CreateThread")           ||
         IS_IMPORT_BREAKPOINT("ntdll.dll",    "NtQueryInformationProcess") ||
         IS_IMPORT_BREAKPOINT("kernel32.dll", "VirtualAlloc")           ||
         IS_IMPORT_BREAKPOINT("kernel32.dll", "VirtualAllocEx")         ||
         IS_IMPORT_BREAKPOINT("kernel32.dll", "GetModuleHandle")        ||
         IS_IMPORT_BREAKPOINT("kernel32.dll", "GetProcAddress"))
        TRIP_SECURITY_REASON(RTP_REASON_COMMON_BP_PLACEMENTS);
    #endif

    if(  IS_IMPORT_HOOKED("kernel32.dll",     "IsDebuggerPresent")
      || IS_IMPORT_HOOKED("kernel32.dll",     "CheckRemoteDebuggerPresent")
      || IS_IMPORT_BREAKPOINT("kernel32.dll", "CheckRemoteDebuggerPresent")
      || IS_IMPORT_BREAKPOINT("kernel32.dll", "IsDebuggerPresent")
      || IS_IMPORT_BREAKPOINT("kernel32.dll", "OutputDebugStringA")
      || IS_IMPORT_BREAKPOINT("kernel32.dll", "OutputDebugStringW")
      || IS_IMPORT_HOOKED("kernel32.dll",     "OutputDebugStringA")
      || IS_IMPORT_HOOKED("kernel32.dll",     "OutputDebugStringW")
      || utils::is_debugger_present()){

      #if !defined(LOADER_CLIENT)
        TRIP_SECURITY_REASON(RTP_REASON_DEBUGGER_ATTACHED);
      #else
         SAFE_EXIT();
      #endif
    }

    // Some people inject a DLL and it'll popup a console where they can view debug information.
    // Instead, we will detect that console window and ban them.
    #if !defined(DEV_MODE)
      if(  !IS_IMPORT_BREAKPOINT("kernel32.dll", "GetConsoleWindow")){
        if(I(GetConsoleWindow)() != NULL){
          TRIP_SECURITY_REASON(RTP_REASON_CONSOLE_WINDOW_PRESENT);
        }
      }
      else{
        TRIP_SECURITY_REASON(RTP_REASON_CONSOLE_WINDOW_CHECK_BP_HOOK_PRESENT);
      }
    #endif

    I(Sleep)(XOR32(1));
  }

  return 0;
}

DWORD __stdcall rtp_page_checker(void* param){
  while(true){
    rtp::page_check();
    I(Sleep)(XOR32(1));
  }
}

void rtp::reload_text_sections(){
  static bool once = false;
  if(once)
    return;

  once = true;
  rtp_methods::flash_system_module(XOR("ntdll.dll"));
  rtp_methods::flash_system_module(XOR("kernel32.dll"));
  rtp_methods::flash_system_module(XOR("user32.dll"));
  rtp_methods::flash_system_module(XOR("wininet.dll"));
}

void rtp::page_check(){
  const u32 our_module = WHASH_RT(utils::get_module_path_from_addr(utils::get_module_handle(0)));

  void*        module           = 0;
  static i32   modules_iter     = 0;
  static u32   last_module_hash = 0;
  while(true){
    MEMORY_BASIC_INFORMATION info;
    ul64 status = 0;
    if(!utils::query_virtual_memory(module, &info, sizeof(info), &status)){
      if(status == XOR32(0xC0000005))
        TRIP_SECURITY_REASON(RTP_REASON_QUERY_VIRUTAL_MEMORY_TAMPERED);

      break;
    }

    if(info.State != XOR32(MEM_COMMIT)){
      module += info.RegionSize;
      continue;
    }

    ul64 protect = info.Protect & XOR32(0xFF);
    if(protect == XOR32(PAGE_READONLY) || protect == XOR32(PAGE_READWRITE)){
      module += info.RegionSize;
      continue;
    }

    // Yea at this point who ever the fuck this is can go and fuck themselves.
    //if(protect == XOR32(PAGE_EXECUTE_READWRITE) || protect == XOR32(PAGE_EXECUTE_WRITECOPY))
    //  TRIP_SECURITY();

    if(protect == XOR32(PAGE_READWRITE) || protect == XOR32(PAGE_EXECUTE_WRITECOPY) || protect == XOR32(PAGE_EXECUTE_READ) || protect == XOR32(PAGE_EXECUTE_READWRITE)){
      if(rtp_methods::detect_scylla_hide(module, info.RegionSize))
        TRIP_SECURITY_REASON(RTP_REASON_SCYLLA_HIDE_DETECTED);

      if(rtp_methods::detect_titan_hide(module, info.RegionSize))
        TRIP_SECURITY_REASON(RTP_REASON_TITAN_HIDE_DETECTED);
    }

    wchar_t* owner = utils::get_module_path_from_addr(module);
    if(owner != nullptr){
      u32 hash = WHASH_RT(owner);
      if(hash == our_module || hash == last_module_hash && rtp_transmitted_heartbeat){
        module += info.RegionSize;
        continue;
      }

      if(str_utils::contains(owner, WXOR(L"\\Desktop")) && !str_utils::contains(owner, WXOR(L".tmp")))
        TRIP_SECURITY_REASON(RTP_REASON_DLL_INJECTION_FROM_DESKTOP);

      if(str_utils::contains(owner, WXOR(L"nvmdi.inf_amd64"))){
        module += info.RegionSize;
        continue;
      }

      switch(hash){
        default: break;
        case HASH("C:\\WINDOWS\\SYSTEM32\\dhcpcsvc.DLL"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\cryptnet.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\VERSION.dll"):
        case HASH("C:\\WINDOWS\\System32\\SHELL32.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\urlmon.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\CRYPTBASE.DLL"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\DWrite.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\IPHLPAPI.DLL"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\SspiCli.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\WINHTTP.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\WININET.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\WINNSI.DLL"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\apphelp.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\credui.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\d2d1.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\dwmapi.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\iertutil.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\kernel.appcore.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\netutils.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\ntdll.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\ondemandconnroutehelper.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\profapi.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\srvcli.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\windows.storage.dll"):
        case HASH("C:\\WINDOWS\\System32\\ADVAPI32.dll"):
        case HASH("C:\\WINDOWS\\System32\\GDI32.dll"):
        case HASH("C:\\WINDOWS\\System32\\IMM32.DLL"):
        case HASH("C:\\WINDOWS\\System32\\KERNEL32.DLL"):
        case HASH("C:\\WINDOWS\\System32\\KERNELBASE.dll"):
        case HASH("C:\\WINDOWS\\System32\\NSI.dll"):
        case HASH("C:\\WINDOWS\\System32\\OLEAUT32.dll"):
        case HASH("C:\\WINDOWS\\System32\\RPCRT4.dll"):
        case HASH("C:\\WINDOWS\\System32\\SHCORE.dll"):
        case HASH("C:\\WINDOWS\\System32\\USER32.dll"):
        case HASH("C:\\WINDOWS\\System32\\WS2_32.dll"):
        case HASH("C:\\WINDOWS\\System32\\bcryptPrimitives.dll"):
        case HASH("C:\\WINDOWS\\System32\\combase.dll"):
        case HASH("C:\\WINDOWS\\System32\\gdi32full.dll"):
        case HASH("C:\\WINDOWS\\System32\\msvcp_win.dll"):
        case HASH("C:\\WINDOWS\\System32\\msvcrt.dll"):
        case HASH("C:\\WINDOWS\\System32\\sechost.dll"):
        case HASH("C:\\WINDOWS\\System32\\shlwapi.dll"):
        case HASH("C:\\WINDOWS\\System32\\ucrtbase.dll"):
        case HASH("C:\\WINDOWS\\System32\\win32u.dll"):
        case HASH("C:\\WINDOWS\\system32\\mswsock.dll"):
        case HASH("C:\\WINDOWS\\System32\\CRYPT32.dll"):
        case HASH("C:\\WINDOWS\\System32\\ole32.dll"):
        case HASH("C:\\Windows\\System32\\windowscodecs.dll"):
        case HASH("C:\\WINDOWS\\system32\\uxtheme.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\dxcore.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\directxdatabasehelper.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\d3d11.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\dxgi.dll"):
        case HASH("C:\\WINDOWS\\system32\\rsaenh.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\ntmarta.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\CRYPTSP.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\msasn1.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\wldp.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\devobj.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\cfgmgr32.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\UMPDC.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\POWRPROF.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\bcrypt.dll"):
        case HASH("C:\\WINDOWS\\System32\\wintypes.dll"):
        case HASH("C:\\WINDOWS\\System32\\wintrust.dll"):
        case HASH("C:\\WINDOWS\\System32\\clbcatq.dll"):
        case HASH("C:\\WINDOWS\\System32\\imagehlp.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\WINMM.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\drvstore.dll"):
        case HASH("C:\\WINDOWS\\System32\\MSCTF.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\WindowsCodecsExt.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\CoreUIComponents.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\CoreMessaging.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\CFGMGR32.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\TextShaping.dll"):
        case HASH("C:\\WINDOWS\\system32\\nvspcap64.dll"):
        case HASH("C:\\Windows\\System32\\msxml6.dll"):
        case HASH("C:\\WINDOWS\\SYSTEM32\\textinputframework.dll"):
        {
          module += info.RegionSize;
          continue;
        }
      }
    }

    module += info.RegionSize;
  }
}

// This stuff will probably be annoying to find for the person. But, it should be enough for what it is.
void rtp::runtime_protection(){
#if !defined(LOADER_CLIENT)
WANTS_TIME_CHECKS();
TIME_CHECK_START()
  reload_text_sections();
  if(IS_IMPORT_BREAKPOINT("kernel32.dll", "CreateThread") || IS_IMPORT_HOOKED("kernel32.dll", "CreateThread")){
    TRIP_SECURITY_REASON(RTP_REASON_CREATE_THREAD_BP_OR_HOOKED);
    return;
  }

  if(utils::get_kuser_shared_data()->KdDebuggerEnabled)
    TRIP_SECURITY_REASON(RTP_REASON_KUSERSHARED_KD_DEBUGGER_ENABLED);
#endif

  I(CreateThread)(NULL, NULL, suspended_thread, NULL, XOR32(CREATE_SUSPENDED), NULL);
  I(CreateThread)(NULL, NULL, suspended_check_thread, NULL, 0, NULL);
  I(CreateThread)(NULL, NULL, rtp_think, NULL, 0, NULL);
  I(CreateThread)(NULL, NULL, rtp_page_checker, NULL, 0, NULL);

#if !defined(LOADER_CLIENT)
TIME_CHECK_END(5.f);
#endif
}

CFLAG_O0 void rtp::pre_launch(){
#if defined(LOADER_CLIENT)
  VM_FAST_START()

  bool av_present = false;
  if(IS_IMPORT_BREAKPOINT("kernel32.dll", "CreateFileA") || IS_IMPORT_BREAKPOINT("kernel32.dll", "WriteFile") || IS_IMPORT_BREAKPOINT("kernel32.dll", "UnmapViewOfFile"))
    av_present = true;

  // This can occur from a lazy hook but also common for anti-viruses to block handles to these functions.
  if(I(FindWindowA)(XOR("Shell_TrayWnd"), nullptr) == nullptr || I(FindWindowW)(WXOR(L"Shell_TrayWnd"), nullptr) == nullptr)
    av_present = true;

  // Really the only reason why we wouldn't be able to create threads is probably because of titan-hide or something related.
  void* handle = I(CreateThread)(NULL, NULL, exec_test, NULL, 0, NULL);
  if(!IS_HANDLE_VALID(handle))
    av_present = true;

  if(av_present){
    I(MessageBoxA)(nullptr, XOR("Error:\nYou've an active anti-virus present.\n - Make sure that the folder the loader is in is excluded.\n - AVs like ESET/BitDefender require certain protections like \"HIPS\" to be disabled."), XOR(""), XOR32(MB_ICONERROR | MB_TOPMOST));
    SAFE_EXIT();
    return;
  }

  bool test_signing = false;

  SYSTEM_CODEINTEGRITY_INFORMATION sci;
  sci.CodeIntegrityOptions = 0;
  sci.Length = XOR32(sizeof(sci));

  // There is literally never a case that an anti-virus would block you from being able to tell if test-signing is enabled.
  // Thus for this exact reason, we'll allow it through on the basis of detecting these idiots.
  NTSTATUS status = SYSCALL(NtQuerySystemInformation)(XOR32(SystemCodeIntegrityInformation), (void*)&sci, XOR32(sizeof(sci)), nullptr);

  if(status != 0 && status != XOR32(0xC0000005)){
    DBG("[!] Yea somehow NtQuerySystemInformation failed from a SYSCALL (%X)\n", status);
    test_signing = true;
  }
  else
    test_signing = sci.CodeIntegrityOptions & XOR32(0x00000002) || sci.CodeIntegrityOptions & XOR32(0x00000080);

  if(test_signing){
    I(MessageBoxA)(nullptr, XOR("Your Windows was booted up in Test-signing mode.\nRijiN does not support the use of test-signing.\n"), XOR(""), XOR32(MB_ICONERROR | MB_TOPMOST));
    SAFE_EXIT();
    return;
  }

  if(utils::get_kuser_shared_data()->KdDebuggerEnabled){
    I(MessageBoxA)(nullptr, XOR("Your Windows has kernel debugging enabled. Open command prompt as admin and type in \"bcdedit /debug off\" and then restart your computer."), XOR(""), XOR32(MB_ICONERROR | MB_TOPMOST));
    SAFE_EXIT();
    return;
  }

#if !defined(DEV_MODE)
  if(I(GetTickCount64)() >= XOR32(86400000)){
    I(MessageBoxA)(nullptr, XOR("Please restart your computer."), "", XOR32(MB_TOPMOST | MB_ICONERROR));
    SAFE_EXIT();
    return;
  }
#endif

  if(rtp_methods::has_no_internet()){
    I(MessageBoxA)(nullptr, XOR("Your computer has no network connection"), "", XOR32(MB_TOPMOST | MB_ICONERROR));
    SAFE_EXIT();
    return;
  }

  rtp::reload_text_sections();

  // Some anti-viruses can use this so lets just abuse it to detect them.
  if ( !rtp_methods::patch_function(HASH("ntdll.dll"), HASH("DbgBreakPoint"))
    || !rtp_methods::patch_function(HASH("ntdll.dll"), HASH("DbgUiRemoteBreakin"))) {

    if(!IS_IMPORT_BREAKPOINT("user32.dll", "MessageBoxA")){
      I(MessageBoxA)(nullptr, XOR("(2) - Error:\nYou've an active anti-virus present.\n - Make sure that the folder the loader is in is excluded.\n - AVs like ESET/BitDefender require certain protections like \"HIPS\" to be disabled."), XOR(""), XOR32(MB_ICONERROR | MB_TOPMOST));
    }
    SAFE_EXIT();
    return;
  }

  rtp_methods::patch_function(XOR32(HASH("ntdll.dll")), XOR32(HASH("DbgBreakPoint")));
  rtp_methods::patch_function(XOR32(HASH("ntdll.dll")), XOR32(HASH("DbgUiRemoteBreakin")));
  VM_FAST_STOP();
  #endif
}

// Very important to call this in area, where the user will have transmitted login information.
void rtp::debugger_check(){
  const float time = math::time(false, true);

  if(  IS_IMPORT_HOOKED("kernel32.dll",     "IsDebuggerPresent")
    || IS_IMPORT_HOOKED("kernel32.dll",     "CheckRemoteDebuggerPresent")
    || IS_IMPORT_BREAKPOINT("kernel32.dll", "CheckRemoteDebuggerPresent")
    || IS_IMPORT_BREAKPOINT("kernel32.dll", "IsDebuggerPresent")
    || IS_IMPORT_BREAKPOINT("kernel32.dll", "OutputDebugStringA")
    || IS_IMPORT_BREAKPOINT("kernel32.dll", "OutputDebugStringW")
    || IS_IMPORT_HOOKED("kernel32.dll",     "OutputDebugStringA")
    || IS_IMPORT_HOOKED("kernel32.dll",     "OutputDebugStringW")
    || utils::is_debugger_present()){
    TRIP_SECURITY_REASON(RTP_REASON_DEBUGGER_ATTACHED);
  }

  if(rtp_last_tick_update_time > 0.f){
    if(math::abs(time - rtp_last_tick_update_time) >= 30.f)
      TRIP_SECURITY_REASON(RTP_REASON_PROTECTION_SYSTEM_TAMPERED);
  }
}

#if defined(LOADER_CLIENT)
struct image_buffer{
  u8* data = nullptr;
  u32 size = 0;
};

static bool capture_screen(u8** out_data, i32* out_width, i32* out_height){
  i32 x = I(GetSystemMetrics)(SM_XVIRTUALSCREEN);
  i32 y = I(GetSystemMetrics)(SM_YVIRTUALSCREEN);
  i32 w = I(GetSystemMetrics)(SM_CXVIRTUALSCREEN);
  i32 h = I(GetSystemMetrics)(SM_CYVIRTUALSCREEN);

  HDC screen_dc   = I(GetDC)(NULL);
  HDC mem_dc      = I(CreateCompatibleDC)(screen_dc);
  HBITMAP bitmap = I(CreateCompatibleBitmap)(screen_dc, w, h);

  I(SelectObject)(mem_dc, bitmap);

  if(!I(BitBlt)(mem_dc, 0, 0, w, h, screen_dc, x, y, SRCCOPY | CAPTUREBLT)){
    I(DeleteObject)(bitmap);
    I(DeleteDC)(mem_dc);
    I(ReleaseDC)(NULL, screen_dc);
    return false;
  }

  BITMAPINFOHEADER bi = {};
  bi.biSize        = XOR32(sizeof(BITMAPINFOHEADER));
  bi.biWidth       = w;
  bi.biHeight      = -h; // Top-down
  bi.biPlanes      = 1;
  bi.biBitCount    = XOR32(24);
  bi.biCompression = BI_RGB;

  i32 row_size = ((w * 3 + 3) & ~3);
  i32 data_size = row_size * h;
  u8* buffer = (u8*)I(malloc)(data_size);
  if(buffer == nullptr)
    return false;

  BITMAPINFO bmi = {};
  bmi.bmiHeader = bi;

  if(!I(GetDIBits)(mem_dc, bitmap, 0, h, buffer, &bmi, DIB_RGB_COLORS)){
    I(free)(buffer);
    I(DeleteObject)(bitmap);
    I(DeleteDC)(mem_dc);
    I(ReleaseDC)(NULL, screen_dc);
    return false;
  }

  *out_data = buffer;
  *out_width = w;
  *out_height = h;

  I(DeleteObject)(bitmap);
  I(DeleteDC)(mem_dc);
  I(ReleaseDC)(NULL, screen_dc);
  return true;
}

static void write_callback(void* context, void* data, i32 size) {
  image_buffer* out = (image_buffer*)context;

  u8* new_data = (u8*)realloc(out->data, out->size + size);
  if(new_data == nullptr)
    return;

  memcpy(new_data + out->size, data, size);
  out->data = new_data;
  out->size += size;
}

CFLAG_O0 void rtp::screenshot(bool* result){
  static bool once = false;
  if(once)
    return;

  if(GET_WINVER >= XOR32(15063)){
    static void* set_process_dpi_awareness_context_ptr = utils::get_proc_address(utils::get_module_handle(HASH("user32.dll")), HASH("SetProcessDpiAwarenessContext"));
    if(set_process_dpi_awareness_context_ptr == nullptr)
      return;

    utils::call_stdcall<BOOL, DPI_AWARENESS_CONTEXT>(set_process_dpi_awareness_context_ptr, DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  }
  else{
    static void* set_process_dpi_aware_ptr = utils::get_proc_address(utils::get_module_handle(HASH("user32.dll")), HASH("SetProcessDPIAware"));
    if(set_process_dpi_aware_ptr == nullptr)
      return;

    utils::call_stdcall<BOOL>(set_process_dpi_aware_ptr);
  }


  u8* rgb    = nullptr;
  i32 width  = 0;
  i32 height = 0;

  *result = false;
  if(!capture_screen(&rgb, &width, &height)){
    DBG("[!] Failed to capture desktop\n");
    return;
  }

  // BGR -> RGB.
  for (i32 i = 0; i < width * height; ++i) {
    u8* p = &rgb[i * 3];
    u8 tmp = p[0];
    p[0] = p[2];
    p[2] = tmp;
  }

  image_buffer jpeg;
  stbi_write_jpg_to_func(write_callback, &jpeg, width, height, 3/*RGB Channel*/, rgb, XOR32(35) /*Quality setting*/);

  c_net_request_screen_shot* msg = (c_net_request_screen_shot*)malloc(sizeof(c_net_request_screen_shot) + jpeg.size);
  if(msg == nullptr){
    SAFE_EXIT();
    return;
  }

  msg->magic  = XOR32(NET_FRAGMENT_MAGIC);
  msg->cmd    = XOR32(c_net_request_screen_shot_cmd);
  msg->status = XOR32(NET_FRAGMENT_OK);
  msg->len    = XOR32(sizeof(c_net_request_screen_shot)) + jpeg.size - XOR32(sizeof(c_net_fragment));

  msg->image_hash = FNV1A_BINARY(jpeg.data, jpeg.size);
  memcpy(msg->buffer, jpeg.data, jpeg.size);
  msg->buffer_len = jpeg.size;

  c_net_submit_cloud_config_response* response = (c_net_submit_cloud_config_response*)msg->transmit(true);
  if(response == nullptr || !response->ok){
    DBG("[!] screenshot bad response.\n");
    SAFE_EXIT();
    return;
  }

  *result = true;

  // Cleanup
  free(rgb);
  free(jpeg.data);
}
#endif