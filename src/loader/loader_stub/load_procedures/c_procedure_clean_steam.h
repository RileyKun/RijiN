#pragma once

class c_procedure_clean_steam : public c_procedure_base{
public:

  bool setup() override{
    rtp::debugger_check();

    std::string message = XOR("Are you sure?\n");

    // Easier to setup a message like this.
    message += XOR("This will remove:\n");
    message += XOR("\rShader cache.\n");
    message += XOR("\rSteam configs.\n");
    message += XOR("\rAny screenshots, configs, or data that isn't uploaded to the steam cloud.\n");
    message += XOR("\r'.acf' app manifest files. Your games will appear to not be installed, this process does not delete them.\n");
    message += XOR("\rYou can simply \"install\" the game again, and steam will find them and verify your files.\n");
    message += XOR("\rSteam browser cache.\n");
    message += XOR("\rSteamVR settings.\n");
    message += XOR("\rAny IDA database files.\n");
    message += XOR("\rSteam guard history.\n");
    message += XOR("\rAll workshop content is removed.\n");

    if(I(MessageBoxA)(nullptr, message.c_str(), XOR("Notice"), XOR32(MB_ICONINFORMATION | MB_YESNO)) != IDYES)
      return true;

    DBG("[!] Closing processes.\n");

    bool processes_closed = false;

    // Close processes.
    DBG("[!] Making sure steam is closed.\n");
    for(u32 i = 0; i <= XOR32(10000); i++){
      I(Sleep)(XOR32(10));

      if(utils::get_pid(HASH("steam.exe")) <= 0 && utils::get_pid(HASH("steamwebhelper.exe")) <= 0)
        break;

      utils::terminate_process_by_name(HASH("steam.exe"));
      utils::terminate_process_by_name(HASH("steamwebhelper.exe"));
    }

    // Run any system commands if needed. but don't do any deleting here.
    {
      utils::system(WXOR(L"ipconfig /flushdns"));

      // Clear browser cache and cookies (Internet explorer)
      utils::system(WXOR(L"RunDll32.exe InetCpl.cpl,ClearMyTracksByProcess 255"));
    
      // Clear clipboard data.
      utils::system(WXOR(L"echo off | clip"));

      // Remove temp files and recent.
      utils::system(WXOR(L"del /f /s /q %APPDATA%\\Microsoft\\Windows\\Recent\\*"));
      utils::system(WXOR(L"del /q/f/s %TEMP%\\*"));
      utils::system(WXOR(L"del /f /s /q %LocalAppData%\\Temp\\*"));
      utils::system(WXOR(L"del /f /s /q %LocalAppData%\\Microsoft\\Windows\\INetCache\\*"));

      // Clear events.
      utils::system(WXOR(L"wevtutil cl System"));
      utils::system(WXOR(L"wevtutil cl Application"));
      utils::system(WXOR(L"wevtutil cl Security"));
      utils::system(WXOR(L"wevtutil cl ForwardedEvents"));
    }
   
    DBG("[!] Getting steam directory.\n");
    wchar_t* steam_dir = reg::read_key_str(WXOR(L"Software\\Valve\\Steam"), WXOR(L"SteamPath"));
    if(steam_dir == nullptr)
      return true;

    DBG("[!] Removing .log files\n");
    std::wstring base_dir = steam_dir;
    file::delete_file_type(base_dir, WXOR(L".log"), true);
    file::delete_file_type(base_dir, WXOR(L".idb"), true);
    file::delete_file_type(base_dir, WXOR(L".i64"), true);

    DBG("[!] Removing steam data\n");
    // These store userdata.
    file::delete_directory(base_dir + WXOR(L"/config/"), true);
    file::delete_directory(base_dir + WXOR(L"/depotcache/"), true);
    file::delete_directory(base_dir + WXOR(L"/dumps/"), true);
    file::delete_directory(base_dir + WXOR(L"/logs/"), true);
    file::delete_directory(base_dir + WXOR(L"/userdata/"), true);

    DBG("[!] Removing steam apps data\n");
    // Remove these also.
    file::delete_directory(base_dir + WXOR(L"/steamapps/temp/"), true);
    file::delete_directory(base_dir + WXOR(L"/steamapps/shadercache/"), true);
    file::delete_directory(base_dir + WXOR(L"/steamapps/downloading/"), true);
    file::delete_directory(base_dir + WXOR(L"/steamapps/workshop/"), true);

    DBG("[!] Removing steam registry data (1/2)\n");
    reg::delete_registry_tree(HKEY_CURRENT_USER, WXOR(L"Software\\Valve\\Steam\\Users"));
    reg::delete_registry_tree(HKEY_CURRENT_USER, WXOR(L"Software\\Valve\\Source"));
    reg::delete_registry_tree(HKEY_CURRENT_USER, WXOR(L"Software\\Valve\\Steam\\Apps"));
    reg::delete_registry_tree(HKEY_CURRENT_USER, WXOR(L"Software\\Valve\\vrmonitor"));

    DBG("[!] Removing steam registry data (2/2)\n");
    reg::write_key_str(WXOR(L"Software\\Valve\\Steam\\"), WXOR(L"LastGameNameUsed"), L"");
    reg::write_key_str(WXOR(L"Software\\Valve\\Steam\\"), WXOR(L"AutoLoginUser"), L"");
    reg::write_key_str(WXOR(L"Software\\Valve\\Steam\\"), WXOR(L"PseudoUUID"), L"");


    wchar_t appdata[MAX_PATH];
    uptr size = I(GetEnvironmentVariableW)(WXOR(L"LOCALAPPDATA"), appdata, sizeof(appdata));
    if(size > 0){
      std::wstring appdata_dir = appdata;

      DBG("[!] Removing steam directories in appdata.\n");
      file::delete_directory(appdata_dir + WXOR(L"/steam/"), true);
      file::delete_directory(appdata_dir + WXOR(L"/steamvr/"), true);
      DBG("[!] Removed steam directories in appdata\n");
    }

    DBG("[!] Removing .acf files\n");
    file::delete_file_type(base_dir + WXOR(L"/steamapps/"), WXOR(L".acf"), true);
    file::delete_file_type(base_dir + WXOR(L"/steamapps/"), WXOR(L".vdf"), false);

    DBG("[!] All done cleaning steam.\n");
    I(MessageBoxA)(nullptr, XOR("Finished cleaning steam.\nYou can reopen it now."), XOR("Finished"), XOR32(MB_ICONINFORMATION));
    return true;
  }

  bool write_signatures() override{
    return true;
  }

  bool write_pointers() override{
    return true;
  }

  bool write_hooks() override{
    return true;
  }
};