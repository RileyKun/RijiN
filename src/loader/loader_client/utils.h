#pragma once

#include <dwmapi.h>

#define LOGIN_FILE_NAME WXOR(L"UP.bin")
#define LOGIN_ROTATOR_KEY XOR32(1038875)

#define LOGIN_MIN_USERNAME_SIZE 3
#define LOGIN_MIN_PASSWORD_SIZE 5
#define LOGIN_MAX_USERNAME_SIZE 32
#define LOGIN_MAX_PASSWORD_SIZE 1024

// TODO: move me to structs.h
struct up_data{
  u32 username_len;
  i8 username[CREDUI_MAX_USERNAME_LENGTH];
  u32 password_len;
  i8 password[CREDUI_MAX_PASSWORD_LENGTH];
};

namespace utils{
  inline u64 get_login_encryption_key(){
    i8* machine_guid = reg::read_key_str(XOR("SOFTWARE\\Microsoft\\Cryptography"), XOR("MachineGuid"), HKEY_LOCAL_MACHINE);
    u64 key = 1;
    if(machine_guid != nullptr){
      for(i32 i = 0; i < strlen(machine_guid); i++)
        machine_guid[i] ^= LOGIN_ROTATOR_KEY;
    }

    return FNV1A_STR_RT_64(machine_guid);
  }

  void save_up();
  bool check_up();
  bool load_up();
  bool delete_up();
  bool prompt_up(bool* should_save_credentials = nullptr, uptr* _result = nullptr);

  inline void xor_up(u8* buffer, i32 size){
    if(buffer == nullptr)
      return;

    if(size != sizeof(up_data))
      return;

    for(i32 i = 0; i < size; i++){
      buffer[i] ^= (i % XOR32(0x64)) + get_login_encryption_key();
    }
  }

  inline void* create_window(WNDPROC wnd_proc, colour background, vec3i size){
    i8* class_name = ("IME");

    WNDCLASSA wc{};
    {
      wc.lpfnWndProc   = wnd_proc;
      wc.hCursor       = I(LoadCursorA)( nullptr, IDC_ARROW );
      wc.lpszClassName = class_name;
      wc.hInstance     = utils::get_cur_process_base_addr();
    }

    I(RegisterClassA)(&wc);

    void* handle = I(CreateWindowExA)(0, class_name, XOR(""), XOR32(WS_OVERLAPPED) | XOR32(WS_CAPTION) | XOR32(WS_SYSMENU) | XOR32(WS_MINIMIZEBOX), XOR32(CW_USEDEFAULT), XOR32(CW_USEDEFAULT), size.x, size.y, nullptr, nullptr, wc.hInstance, nullptr);

    if(handle == nullptr)
      return nullptr;

    // Enable dark mode for window
    u32 use_dark_mode = true;
    DwmSetWindowAttribute(handle, XOR32(20)/*DWMWA_WINDOW_CORNER_PREFERENCE*/, &use_dark_mode, sizeof(use_dark_mode));

    I(UpdateWindow)(handle);
    I(ShowWindow)(handle, SW_SHOW);

    DBG("[+] Created window %X\n", handle);

    return handle;
  }
}