#pragma once
#include <Rpc.h>
#include <cpuid.h>
#include <Winsock2.h>
#include <Winsock.h>

struct s_mac_address_info{
  u8  data[6];
  u32 hash;
};

struct s_cpu_brand_info{
  u32 data[12];
  u32 hash;
};

struct s_arp_mac_address_info{
  u8  data[6];
  u32 hash;
};

struct s_steam_id_list{
  u32 size;
  u32 id[1024];
};

class c_hwid{
public:

  // This is important so we can avoid temp drivers.
  bool is_invalid_network_driver(i8* desc);
  ALWAYSINLINE bool is_invalid_network_driver_type(u32 type, u32 len){
    if(len != XOR32(6))
      return true;

    switch(type){
      default: break;
      case IF_TYPE_TUNNEL:
      case IF_TYPE_PPP:
        return true;
      case MIB_IF_TYPE_ETHERNET:
      case IF_TYPE_IEEE80211:
      case IF_TYPE_GIGABITETHERNET:
      case IF_TYPE_FASTETHER:
      case IF_TYPE_FASTETHER_FX:
        return false;
    }

    return false;
  }

  ALWAYSINLINE bool is_bad_mac_address(IP_ADAPTER_INFO* adapter){
    if(adapter == nullptr)
      return true;

    if(adapter->AddressLength == 0)
      return true;

    for(i32 i = 0; i < adapter->AddressLength; i++){
      if(adapter->Address[i] != 0)
        return false;
    }

    return true;
  }

  bool get_mac_address(s_mac_address_info& info);

  ALWAYSINLINE bool get_cpu_brand_name(s_cpu_brand_info& info){
    if(!__get_cpuid_max(XOR32(0x80000000), nullptr)){
      SHOW_ERROR(ERR_GET_CPU_BRAND_NAME_NO_SUPPORT, true);
      return false;
    }

    memset(&info.data[0], 0x00, XOR32(sizeof(info.data)));
    __get_cpuid(XOR32(0x80000002), info.data+XOR32(0x0), info.data+XOR32(0x1), info.data+XOR32(0x2), info.data+XOR32(0x3));
    __get_cpuid(XOR32(0x80000003), info.data+XOR32(0x4), info.data+XOR32(0x5), info.data+XOR32(0x6), info.data+XOR32(0x7));
    __get_cpuid(XOR32(0x80000004), info.data+XOR32(0x8), info.data+XOR32(0x9), info.data+XOR32(0xA), info.data+XOR32(0xB));
    
    str_utils::trim((i8*)&info.data[0]);
    info.hash = HASH_RT((i8*)&info.data[0]);

    DBG("[!] get_cpu_brand_name: \"%s\" (0x%08X)\n", info.data, info.hash);
    return true;
  }

  bool get_arp_mac_address(s_arp_mac_address_info& info);

  ALWAYSINLINE bool get_steam_ids(s_steam_id_list& steam_id_list){
    /*
    HKEY key;
    if(I(RegOpenKeyExW)((HKEY_CURRENT_USER), WXOR(L"Software\\Valve\\Steam\\Users"), 0, XOR32(KEY_QUERY_VALUE) | XOR32(KEY_ENUMERATE_SUB_KEYS), &key) != XOR32(ERROR_SUCCESS))
      return false;

    u32 number_of_folders = 0;
    if(I(RegQueryInfoKeyW)(key, nullptr, nullptr, nullptr, &number_of_folders, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) != XOR32(ERROR_SUCCESS) || number_of_folders < XOR32(1))
      return false;

    memset(&steam_id_list, 0, XOR32(sizeof(s_steam_id_list)));
    i8 folder_name[255]{};
    for(i32 i = 0; i < number_of_folders; i++){
      u32 name_len = XOR32(sizeof(folder_name));

      if(I(RegEnumKeyExA)(key, i, folder_name, &name_len, nullptr, nullptr, nullptr, nullptr) != XOR32(ERROR_SUCCESS))
        continue;

      if(!convert::is_number(folder_name)){
        DBG("[-] reg steamid %s is not a number\n", folder_name);
        continue;
      }

      if(I(strlen)(folder_name) > 10){
        DBG("[-] folder %s too large\n", folder_name);
        continue;
      }

      u32 sid = (u32)std::stoul(folder_name);
      if(!add_sid_to_list(steam_id_list, sid))
        break;
    }
    I(RegCloseKey)(key);

    return steam_id_list.size >= XOR32(1);*/
    return true;
  }

  ALWAYSINLINE bool get_steamids_from_activeuser_key(s_steam_id_list& steam_id_list){
    if(steam_id_list.size >= (XOR32(sizeof(steam_id_list.id)) / XOR32(sizeof(u32))))
      return true;

    if(steam_id_list.size < (XOR32(sizeof(steam_id_list.id)) / XOR32(sizeof(u32)))){
      i32 id = reg::read_key_i32(WXOR(L"Software\\Valve\\Steam\\ActiveProcess"), WXOR(L"ActiveUser"));
      if(id > 0){
        add_sid_to_list(steam_id_list, id);
      }
    }

    return steam_id_list.size >= XOR32(1);
  }

  ALWAYSINLINE bool get_steamids_from_userdata_folder(s_steam_id_list& steam_id_list){
    if(steam_id_list.size >= (XOR32(sizeof(steam_id_list.id)) / XOR32(sizeof(u32))))
      return true;

    // Get steamids from userdata folder.
    {
      wchar_t* dir = reg::read_key_str(WXOR(L"Software\\Valve\\Steam"), WXOR(L"SteamPath"));
      if(dir == nullptr)
        return steam_id_list.size >= XOR32(1);

      wchar_t userdata_dir[MAX_PATH];
      formatW(userdata_dir, WXOR(L"%ls/userdata/*.*"), dir);
      I(free)(dir);

      WIN32_FIND_DATAW data;
      void* file_handle = I(FindFirstFileW)(userdata_dir, &data);
      if(file_handle == INVALID_HANDLE_VALUE)
        return steam_id_list.size >= XOR32(1);

      while(I(FindNextFileW)(file_handle, &data) != 0){
        if(data.cFileName[0] == L'.')
          continue;

        if(!convert::is_number(data.cFileName)){
          DBG("[-] dir steamid %s is not a number\n", data.cFileName);
          continue;
        }

        if(I(wcslen)(data.cFileName) > 10){
          DBG("[-] folder %s too large\n", data.cFileName);
          continue;
        }

        u32 sid = (u32)std::stoul(data.cFileName);
        if(!add_sid_to_list(steam_id_list, sid))
          break;
      }
    }

    return steam_id_list.size >= XOR32(1);

    return true;
  }

  ALWAYSINLINE bool get_steamids_from_config_folder(s_steam_id_list& steam_id_list){
    if(steam_id_list.size >= (XOR32(sizeof(steam_id_list.id)) / XOR32(sizeof(u32))))
      return true;

    // Get steamids from config folder.
    {
      wchar_t* dir = reg::read_key_str(WXOR(L"Software\\Valve\\Steam"), WXOR(L"SteamPath"));
      if(dir == nullptr)
        return steam_id_list.size >= XOR32(1);

      wchar_t config_dir[MAX_PATH];
      formatW(config_dir, WXOR(L"%ls/config/*.*"), dir);
      I(free)(dir);

      WIN32_FIND_DATAW data;
      void* file_handle = I(FindFirstFileW)(config_dir, &data);
      if(file_handle == INVALID_HANDLE_VALUE)
        return steam_id_list.size >= XOR32(1);

      while(I(FindNextFileW)(file_handle, &data) != 0){
        if(data.cFileName[0] == L'.')
          continue;

        // The file name must contain the following text.
        if(!str_utils::contains(data.cFileName, WXOR(L"coplay_7656")))
          continue;

        // The file name is too big.
        if(I(wcslen)(data.cFileName) > XOR32(32)){
          DBG("[-] config steamid %ls is too big\n", data.cFileName);
          continue;
        }

        // Get the number inside of the file.
        std::wstring new_string;
        for(u32 i = 0; i < I(wcslen)(data.cFileName); i++){
          if(!convert::is_number(data.cFileName[i]))
            continue;

          new_string += data.cFileName[i];
        }

        // We somehow didn't get a steamid from this?
        if(new_string.empty()){
          DBG("[-] config steamid %ls doesn't have any numbers.\n", data.cFileName);
          continue;
        }

        // Is the SteamID64 too large?
        if(new_string.length() > XOR32(18)){
          DBG("[-] config steamid %ls is too large.\n", data.cFileName);
          continue;
        }

        u32 sid = (u32)std::stoull(new_string.c_str());
        if(!add_sid_to_list(steam_id_list, sid))
          break;
      }
    }

    return steam_id_list.size >= XOR32(1);
  }

  ALWAYSINLINE bool get_steamids_from_loginusers_file(s_steam_id_list& steam_id_list){
    if(steam_id_list.size >= (XOR32(sizeof(steam_id_list.id)) / XOR32(sizeof(u32))))
      return true;

    // Get steamids from the loginusers.vdf file located in the config folder.
    {
      wchar_t* dir = reg::read_key_str(WXOR(L"Software\\Valve\\Steam"), WXOR(L"SteamPath"));
      if(dir == nullptr)
        return steam_id_list.size >= XOR32(1);

      wchar_t loginu_file[MAX_PATH];

      formatW(loginu_file, WXOR(L"%ls/config/loginusers.vdf"), dir);
      I(free)(dir);

      {
        i32 len = 0;
        u8* buf = file::read_file(loginu_file, len);
        if(buf == nullptr)
          return steam_id_list.size >= XOR32(1);

        i32 index = utils::index_of(buf, XOR("765611"));
        while(index != -1){

          std::wstring new_string;
          for(i32 i = index; i < len; i++){
            if(!convert::is_number((i8)buf[i]))
              break;

            new_string += buf[i];
          }

          if(new_string.length() > XOR32(18)){
            DBG("[-] loginusers steamid %ls is too large.\n", new_string.c_str());
            index = utils::index_of(buf, XOR("765611"), index + 1);
            continue;
          }

          u32 sid = (u32)std::stoull(new_string.c_str());
          new_string = L"";

          if(!add_sid_to_list(steam_id_list, sid))
            break;

          index = utils::index_of(buf, XOR("765611"), index + 1);
        }

        I(free)(buf);
      }
    }

    return steam_id_list.size >= XOR32(1);
  }

  ALWAYSINLINE bool get_steamids_from_appmanifest_files(s_steam_id_list& steam_id_list){
    if(steam_id_list.size >= (XOR32(sizeof(steam_id_list.id)) / XOR32(sizeof(u32))))
      return true;

    // Get steamids from appmanifest files.
    {
      wchar_t* dir = reg::read_key_str(WXOR(L"Software\\Valve\\Steam"), WXOR(L"SteamPath"));
      if(dir == nullptr)
        return steam_id_list.size >= XOR32(1);

      wchar_t steamapps_dir[MAX_PATH];
      formatW(steamapps_dir, WXOR(L"%ls/steamapps/*.acf"), dir);

      WIN32_FIND_DATAW data;
      void* file_handle = I(FindFirstFileW)(steamapps_dir, &data);
      if(file_handle == INVALID_HANDLE_VALUE)
        return steam_id_list.size >= XOR32(1);

      while(I(FindNextFileW)(file_handle, &data) != 0){
        if(data.cFileName[0] == L'.')
          continue;

        wchar_t file_path[MAX_PATH];
        formatW(file_path, WXOR(L"%ls/steamapps/%ls"), dir, data.cFileName);

        i32 len = 0;
        u8* buf = file::read_file(file_path, len);
        if(buf == nullptr)
          continue;

        i32 index = utils::index_of(buf, XOR("765611"));
        while(index != -1){
          std::wstring new_string;
          for(i32 i = index; i < len; i++){
            if(!convert::is_number((i8)buf[i]))
              break;

            new_string += buf[i];
          }

          if(new_string.length() > XOR32(18)){
            DBG("[-] appman steamid %ls is too large.\n", new_string.c_str());
            index = utils::index_of(buf, XOR("765611"), index + 1);
            continue;
          }

          u32 sid = (u32)std::stoull(new_string.c_str());
          new_string = L"";

          if(!add_sid_to_list(steam_id_list, sid))
            break;

          index = utils::index_of(buf, XOR("765611"), index + 1);
        }

        I(free)(buf);
      }

      I(free)(dir);
    }

    return steam_id_list.size >= XOR32(1);
  }

  ALWAYSINLINE bool add_sid_to_list(s_steam_id_list& steam_id_list, u32 id){
    // We have reached the sid limit.
    if(steam_id_list.size >= (XOR32(sizeof(steam_id_list.id)) / XOR32(sizeof(u32))))
      return false;

    if(steam_id_list.size > 0){
      // Don't add the same id again.
      for(i32 i = 0; i < steam_id_list.size; i++){
        if(steam_id_list.id[i] == id)
          return true;
      }
    }

    DBG("[+] Added sid %i\n", id);
    steam_id_list.id[steam_id_list.size] = id;
    steam_id_list.size++;
    return true;
  }
};

CLASS_EXTERN(c_hwid, hwid);