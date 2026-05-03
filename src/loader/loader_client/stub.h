#pragma once

#include "global.h"
#include "Shlwapi.h"
#include "accctrl.h"
#include "aclapi.h"

class c_stub{
public:
  CFLAG_O0 bool load(c_net_receive_license* license, u32 selected_object){
    VM_MUTATE_START();
    if(license == nullptr)
      return false;

    WANTS_TIME_CHECKS();
    global->show_loading_page(XOR(L"Loading agent (1/10)..."));

    bool result = false;
    rtp::screenshot(&result);

    if(!result){
      SHOW_ERROR(ERR_LOADER_FAILED_SCREENSHOT_CAPTURE, true);
      return false;
    }

    LOADER_STATUS("STUB_START_EXECUTE");

    if(selected_object >= license->objects_size){
      SHOW_ERROR_STR(ERR_STUB_SELECTED_OBJECT_BIGGER_THAN_MAX_OBJECTS, "Selected object is invalid", true);
      return false;
    }

    {
      SYSTEM_CODEINTEGRITY_INFORMATION sci;
      sci.Length = XOR32(sizeof(sci));

      // Its common for titan-hide to make this return 0xC0000005 which makes no sense in our cases.
      // No AV is going to block a process trying to see if test-signing is enabled. it has no use in evil deeds.
      NTSTATUS status = SYSCALL(NtQuerySystemInformation)(XOR32(SystemCodeIntegrityInformation), (void*)&sci, XOR32(sizeof(sci)), nullptr);
      if(status != 0 && status == XOR32(0xC0000005))
        TRIP_SECURITY_REASON(RTP_REASON_NTQUERYSYSTEMINFO_TAMPERED);
      else if(status == 0){
        if(sci.CodeIntegrityOptions & XOR32(0x00000002) || sci.CodeIntegrityOptions & XOR32(0x00000080))
          TRIP_SECURITY_REASON(RTP_REASON_TEST_SIGNING_CHECK_BYPASSED);
      }
    }

    rtp::send_challenge();


    // Create the buffer name
    std::string buffer_name = utils::format(XOR("Global\\%X"), HASH_RT(global->up.username));

    // 1. map the stub data into pagefile
    DBG("[!] Creating file mapping\n");
    global->show_loading_page(XOR(L"Loading agent (2/10)..."));

    TIME_CHECK_START();
    HANDLE map_file = I(CreateFileMappingA)(INVALID_HANDLE_VALUE, nullptr, XOR32(PAGE_READWRITE), 0, XOR32(sizeof(c_stub_data)), buffer_name.c_str());
    TIME_CHECK_END(1.f);

    if(!IS_HANDLE_VALID(map_file)){
      if(I(GetLastError)() == XOR32(ERROR_ACCESS_DENIED))
        SHOW_ERROR_STR(ERR_STUB_FAILED_TO_CREATE_FILE_MAPPING, "1. Make sure the loader is running as admin.\n2. Disable all anti-virus software and try again.", true);
      else
        SHOW_ERROR(ERR_STUB_FAILED_TO_CREATE_FILE_MAPPING, true);

      return false;
    }

    
    DBG("[!] Mapping view of file to created mapping\n");
    global->show_loading_page(XOR(L"Loading agent (3/10)..."));

    TIME_CHECK_START();
    c_stub_data* buffer_map = I(MapViewOfFile)(map_file, XOR32(FILE_MAP_ALL_ACCESS), 0, 0, XOR32(sizeof(c_stub_data)));
    TIME_CHECK_END(2.f);

    if(buffer_map == nullptr){
      if(I(GetLastError)() == XOR32(ERROR_ACCESS_DENIED))
        SHOW_ERROR_STR(ERR_STUB_FAILED_TO_MAP_VIEW_OF_FILE, "1. Make sure the loader is running as admin.\n2. Disable all anti-virus software and try again.", true);
      else
        SHOW_ERROR(ERR_STUB_FAILED_TO_MAP_VIEW_OF_FILE, true);

      return false;
    }
    
    DBG("[+] Buffer was successfully mapped as %s\n", buffer_name.c_str());
    DBG("[!] Allocating %i bytes into mapped buffer\n", sizeof(c_stub_data));
    global->show_loading_page(XOR(L"Loading agent (4/10)..."));

    // 2. Create the new buffer to be mapped
    c_stub_data* new_stub = I(malloc)(XOR32(sizeof(c_stub_data)));
    
    I(memset)(new_stub, 0, XOR32(sizeof(c_stub_data)));
    I(memcpy)(new_stub->key, license->key, XOR32(sizeof(c_stub_data::key)));
    new_stub->license_expire_seconds = license->license_expire_seconds;
    I(memcpy)(&new_stub->object, &license->objects[selected_object], XOR32(sizeof(license_object)));
    
    cookie_xor((u8*)&new_stub->object, XOR32(sizeof(license_object)));
    cookie_xor(new_stub->key, XOR32(sizeof(c_stub_data::key)));
    global->show_loading_page(XOR(L"Loading agent (5/10)..."));
    // 3. Copy new buffer into the mapped buffer
    {

      I(memcpy)(buffer_map, new_stub, XOR32(sizeof(c_stub_data)));
      
      I(free)(new_stub);
      new_stub = nullptr;
      
      DBG("[+] Memory copied into buffer map\n");
    }

    // Kill all existing processes of rundll32
    utils::terminate_process_by_name(HASH("rundll32.exe"));
    
    global->show_loading_page(XOR(L"Loading agent (6/10)..."));
    rtp::debugger_check();

    // 4. Request stub from server
    c_net_receive_stub* stub = server->request_stub(license); 
     
    if(stub == nullptr){
      I(UnmapViewOfFile)(buffer_map);
      I(CloseHandle)(map_file);
      SHOW_ERROR(ERR_STUB_REQUEST_FAILED, true);
      return false;
    }
    
    DBG("[+] Received stub\n");

    // 5. Now load the stub
    {
      global->show_loading_page(XOR(L"Loading agent (7/10)..."));

      /* Added a little randomness on top of the username hash. That way we can ensure we always save and use the latest version.
         Avoid some magic tricky of it somehow using the previous version. Assuming it somehow wasn't deleted at loader start, and the check below.
      */

      i8 buf[1024];
      u64 username_hash =  HASH_RT64(global->up.username) + (uptr)&global->up.username + (uptr)utils::get_cur_process_base_addr() ^ HASH("BASE_ADDR_XOR");
      std::string dll_name = utils::format(XOR("%p%X.tmp"), username_hash, *(volatile u32*)(XOR32_IMPORTANT_ONLY(0x7FFE0330)) % XOR32(8912));

      // 10.13.2024: If we get lucky with a file that already exists. Lets just delete it and then recreate it.
      // Make sure we delete the old version of the stub to ensure that the user always uses the most up to date version.
      // Even though we try and delete all .tmp files in the loader directory on launch.
      if(file::does_file_exist(dll_name)){
        DBG("[!] deleting old stub: %s\n", dll_name.c_str());
        for(u32 i = 1; i <= XOR32(10); i++){
          DBG("[!] trying to delete: %s\n\n", dll_name.c_str());
          if(file::delete_file(dll_name))
            break;

          DBG("[!] Failed to delete: %s\n", dll_name.c_str());
          I(Sleep)(XOR32(1000));
        }
      }

      // If it fails, we can have people possibly use old versions of the stub.
      if(file::does_file_exist(dll_name)){
        DBG("[-] Failed to cleanup stub\n");
        SHOW_ERROR_STR(ERR_STUB_FAIL_TO_CLEAN_STUB, "Prerun cleanup failed.\n- Do not remove your USB/External drive until the loader window closes.\n-Delete the .tmp files from your loader directory.\nTry again once you've read the instructions.", true);
        return false;
      }

      HANDLE handle = INVALID_HANDLE_VALUE;
      for(u32 i = 1; i <= XOR32(3); i++){
        handle = I(CreateFileA)(dll_name.c_str(), XOR32(GENERIC_READ) | XOR32(GENERIC_WRITE), 0, nullptr, XOR32(CREATE_ALWAYS), XOR32(FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_HIDDEN), nullptr);
        if(IS_HANDLE_VALID(handle))
          break;

        DBG("[!] CreateFileA is not valid, trying again.\n");
        I(Sleep)(XOR32(1000));
      }

      if(!IS_HANDLE_VALID(handle)){
        uptr last_error_code = I(GetLastError)();
        if(last_error_code == ERROR_NOT_READY)
          SHOW_ERROR_STATUS_STR(ERR_STUB_CREATE_FILE_FAILED, last_error_code, "- Make sure the loader is extracted from the ZIP file.\n- Do not remove your USB/External drive until the loader window closes.\nOnce you've read these instructions try again.", true);    
        else if(last_error_code == ERROR_ACCESS_DENIED)
          SHOW_ERROR_STATUS_STR(ERR_STUB_CREATE_FILE_FAILED, last_error_code, "- Exclude the folder the loader is in from your anti-virus\n-If it's exclude then disable your anti-virus entirely.\nOnce you've read these instructions try again.", true);
        else
          SHOW_ERROR_STATUS_STR(ERR_STUB_CREATE_FILE_FAILED, last_error_code, "An active anti-virus is tampering with the loader.\nPlease disable it and try again.\nMake sure the loader is extracted from the zip file.", true);    
        
        return false;
      }
      
      global->show_loading_page(XOR(L"Loading agent (8/10)..."));
      
      OVERLAPPED overlapped;
      ul64       wrote_bytes;
      uptr       last_error_code = 0;

      memset(&overlapped, 0, sizeof(OVERLAPPED));

      bool success = I(WriteFile)(handle, &stub->data, stub->len, (ul64*)&wrote_bytes, &overlapped);
      if(!success){
        last_error_code = I(GetLastError)();
        if(last_error_code == XOR32(ERROR_IO_PENDING)){
          DBG("[!] Writing stub is on ERROR_IO_PENDING\n");
          success         = I(GetOverlappedResult)(handle, &overlapped, &wrote_bytes, true);
          last_error_code = I(GetLastError)();
        }
      }

      if(!success || wrote_bytes != stub->len){
        I(UnmapViewOfFile)(buffer_map);
        I(CloseHandle)(map_file);
        I(CloseHandle)(handle);
        SHOW_ERROR_STATUS(ERR_STUB_WRITE_BINARY_FAILED, last_error_code, true);
        return false;
      }

      global->show_loading_page(XOR(L"Loading agent (8.5/10)..."));

      for(u32 i = 1; i <= XOR32(1000); i++){
        DBG("[+] making sure %s exists\n", dll_name.c_str());
        if(file::does_file_exist(dll_name.c_str()))
          break;

        I(Sleep)(10);
      }
      
      global->show_loading_page(XOR(L"Loading agent (9/10)..."));

      I(CloseHandle)(handle);

      i8 arg_buffer[MAX_PATH];
      i8 windows_dir[MAX_PATH] = {};
      formatA(arg_buffer, XOR("%s, __stub %s"), dll_name.c_str(), buffer_name.c_str());

      if(!GetWindowsDirectoryA(windows_dir, MAX_PATH)){
        SHOW_ERROR(ERR_FAILED_TO_GET_WINDOW_PATH, true);
        return false;
      }

      rtp::debugger_check();

      i8 sub_directory[MAX_PATH];
      formatA(sub_directory, XOR("%s\\System32\\rundll32.exe"), windows_dir);

      if(!utils::create_process(sub_directory, arg_buffer)){
        I(UnmapViewOfFile)(buffer_map);
        I(CloseHandle)(map_file);
        SHOW_ERROR(ERR_CREATE_PROCESS_FAILED, true);
        return false;
      }

      for(auto i = 0; i < XOR32(10000); i++){
        if(buffer_map->loaded)
          break;
        
        DBG("[!] Waiting for stub to execute\n");
        I(Sleep)(XOR32(1));
      }
      
      bool loaded = buffer_map->loaded;
      
      I(UnmapViewOfFile)(buffer_map);
      I(CloseHandle)(map_file); 
      
      if(loaded){
        DBG("[+] Stub has loaded!\n");
      }
      else
        SHOW_ERROR_STR(ERR_STUB_FAILED_TO_LOAD_TIMEOUT, "Failed to load stub-loader.\nPlease make sure directory is whitelisted and try again.", true);

    }

    DBG("[+] All done here, exit.");
    LOADER_STATUS("STUB_LOADED");

    global->show_loading_page(XOR(L"Product is loading, this window will close.\nPlease wait at least 5 minutes before trying again."));
    I(Sleep)(XOR32(2500));
    global->show_loading_page(XOR(L"Closing..."));
    I(Sleep)(XOR32(1500));
    VM_MUTATE_STOP();

    return true;
  }
};

CLASS_EXTERN(c_stub, stub);