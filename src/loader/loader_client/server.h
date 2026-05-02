#pragma once

class c_server{
public:
  c_net_request_license request;
  bool                   init_hwid      = false;
  bool                   is_hwid_filled = false;
  s_mac_address_info     mac_address_info;
  s_arp_mac_address_info arp_mac_address_info;
  s_cpu_brand_info       cpu_brand_info;

  ALWAYSINLINE inline c_net_receive_info* get_serverinfo(){
    c_net_fragment request{};
    request.cmd = XOR32(c_net_receive_info_cmd);

    return (c_net_receive_info*)request.transmit(); 
  }
  
  ALWAYSINLINE inline c_net_receive_license* request_license(up_data up, bool* did_time_out = nullptr){
    static i32 max_tries = 0;
    if(max_tries++ > 5)
      return nullptr;

    DBG("[!] contacting license server...\n");

    VM_FAST_START();
    if(!request.client_version){
      DBG("[!] c_net_receive_license init.\n");
      request = {};
      request.client_version = XOR32(BUILD_VERSION);

      I(memcpy)(request.username, &up.username, XOR32(sizeof(request.username)));
      I(memcpy)(request.password, &up.password, XOR32(sizeof(request.password)));
    }
    VM_FAST_STOP();


    if(!init_hwid){
      DBG("[!] Fetching HWID details...\n");
      init_hwid = true;
      if(!hwid->get_mac_address(mac_address_info))
        return nullptr;

      if(!hwid->get_arp_mac_address(arp_mac_address_info))
        return nullptr;

      if(!hwid->get_cpu_brand_name(cpu_brand_info))
        return nullptr;

      hwid->get_steam_ids(request.steam_id_list);
      hwid->get_steamids_from_activeuser_key(request.steam_id_list);
      hwid->get_steamids_from_userdata_folder(request.steam_id_list);
      hwid->get_steamids_from_config_folder(request.steam_id_list);
      hwid->get_steamids_from_loginusers_file(request.steam_id_list);
      hwid->get_steamids_from_appmanifest_files(request.steam_id_list);

      request.hwid_mac_address_hash     = mac_address_info.hash;
      request.hwid_arp_mac_address_hash = arp_mac_address_info.hash;
      I(strcpy)((i8*)&request.hwid_cpu_brand_name[0], (i8*)&cpu_brand_info.data);

      is_hwid_filled = true;
    }

    if(!init_hwid || !is_hwid_filled)
      return nullptr;

    c_net_receive_license* ret = (c_net_receive_license*)request.transmit(false, did_time_out, true);

    if(ret == nullptr)
      return ret;

    DBG("[+] obtained license!\n");

    return ret;
  }

  c_net_receive_image_resource* request_image_resource(c_net_receive_license* license, u32 object_pos){
    assert(license != nullptr);

    c_net_request_image_resource request{};
    request.cmd = XOR32(c_net_request_image_resource_cmd);
    I(memcpy)(request.key, license->key, XOR32(sizeof(request.key)));
    request.object_pos = object_pos;

    return (c_net_receive_image_resource*)request.transmit();
  }
  
  ALWAYSINLINE c_net_receive_stub* request_stub(c_net_receive_license* license){
    assert(license != nullptr);
    
    c_net_request_stub request{};
    request.cmd = XOR32(c_net_request_stub_cmd);
    I(memcpy)(request.key, license->key, XOR32(sizeof(request.key)));

    return (c_net_receive_stub*)request.transmit();
  }
};

CLASS_EXTERN(c_server, server);