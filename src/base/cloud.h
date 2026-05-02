#pragma once

#if defined(INTERNAL_CHEAT)
#define SETTINGS_DATA_PAD 0x1000
#define RESERVE_SETTINGS(amount) PAD(SETTINGS_DATA_PAD * amount);

#define CONFIG_COMPRESS_COMBO_ONE_FROM XOR("3030303030303030303030303030303030303030303030303030303030303030")
#define CONFIG_COMPRESS_COMBO_ONE_TO XOR("X")

#define CONFIG_COMPRESS_COMBO_TWO_FROM XOR("XXXXXXXXXXXXXXXX")
#define CONFIG_COMPRESS_COMBO_TWO_TO XOR("Y")

#define CONFIG_COMPRESS_COMBO_THREE_FROM XOR("YYYYYYYY")
#define CONFIG_COMPRESS_COMBO_THREE_TO XOR("Z")

#define CONFIG_MAX_CB_SIZE 0x1E8480
#define CONFIG_CB_MAGIC HASH("THIS_IS_MY_RIJIN_CONFIG1")
struct s_config_clipboard_data{
  u32 size_of_me;
  u32 size_of_config;
  u32 size_of_both;
  u32 magic;
  u32 id;
  u32 game_hash;
  u32 version_number;
  i32 patched;
  i32 patched2;
  u32 config_data_hash;
  i8  config_data[1];
};

enum cloud_return_type{
  CLOUD_STATUS_OK = 1,
  CLOUD_STATUS_CHECK = -100,
  CLOUD_STATUS_DISABLED,
  CLOUD_STATUS_ALREADY_DOWNLOADED,
  CLOUD_STATUS_FETCH_FAILED,
  CLOUD_STATUS_RESPONSE_CORRUPTION,
  CLOUD_STATUS_LENGTH_MISMATCH,
  CLOUD_STATUS_VERSION_CHANGE,
};

namespace cloud{
  static u32  last_upload_hash = 0;
  static u32  last_upload_id = 0;
  static u32  last_download_hash = 0;
  static u32  last_download_id = 0;

  inline std::wstring read_response_type(i32 response){
    switch(response){
      default: break;
      case CLOUD_STATUS_DISABLED: return WXOR(L"Cloud config system is disabled.");
      case CLOUD_STATUS_ALREADY_DOWNLOADED: return WXOR(L"Already downloaded.");
      case CLOUD_STATUS_RESPONSE_CORRUPTION:
      case CLOUD_STATUS_FETCH_FAILED: return WXOR(L"Failed to fetch config.");
      case CLOUD_STATUS_LENGTH_MISMATCH: return WXOR(L"Cloud config length mismatch");
      case CLOUD_STATUS_VERSION_CHANGE: return WXOR(L"Config system was reset, please make a new config.");
    }

    return WXOR(L"UNK");
  }

  inline i32 upload(u32 id, void* in_buffer, u32 len, bool ignore_error = false){
    u32 current_data_hash = FNV1A_RT((i8*)in_buffer, len);

    // double upload check
    if(id != 7){
      if(last_upload_id == id && last_upload_hash == current_data_hash){
        DBG("[-] cloud::upload already uploaded this configuration\n");
        return CLOUD_STATUS_ALREADY_DOWNLOADED;
      }
    }

    c_net_submit_cloud_config* request = (c_net_submit_cloud_config*)malloc(XOR32(sizeof(c_net_submit_cloud_config)) + len);
    {
      request->magic  = XOR32(NET_FRAGMENT_MAGIC);
      request->cmd    = XOR32(c_net_submit_cloud_config_cmd);
      request->status = XOR32(NET_FRAGMENT_OK);
      request->len    = (XOR32(sizeof(c_net_submit_cloud_config)) + len) - XOR32(sizeof(c_net_fragment));

      memcpy(request->config_key, gen_internal->cloud_license_key, XOR32(32));

      request->config_id   = id;
      request->buffer_len  = len;
      memcpy(&request->buffer, in_buffer, request->buffer_len);
    }

    // send the fragment and free the request buffer
    c_net_submit_cloud_config_response* response = (c_net_submit_cloud_config_response*)transmit_fragment_internal(request, ignore_error);
    free(request);

    if(response == nullptr){
      DBG("[-] cloud::upload failed response error\n");
      return CLOUD_STATUS_FETCH_FAILED;
    }

    if(response->status != NET_FRAGMENT_OK){
      DBG("[-] cloud::upload status != NET_FRAGMENT_OK (%i)\n", response->status);
      free(response);
      return CLOUD_STATUS_RESPONSE_CORRUPTION;
    }

    if(response->ok != true){
      DBG("[-] cloud::upload ok = false\n");
      free(response);
      return CLOUD_STATUS_RESPONSE_CORRUPTION;
    }

    DBG("[+] cloud::upload success (%i, %X, %i)\n", id, in_buffer, len);

    // set the last upload data
    last_upload_id    = id;
    last_upload_hash  = current_data_hash;

    // we set this to tell the downloader that we already have this exact same configuration for this id
    last_download_id    = id;
    last_download_hash  = current_data_hash;

    return CLOUD_STATUS_OK;
  }

  inline i32 download(u32 id, void* out_buffer, u32 out_max_len, u32 wanted_version_number, bool ignore_error = false, u32* out_response_len = nullptr){
    assert_private(out_max_len > sizeof(u32));
    assert_private(out_buffer != nullptr);

    u32 current_data_hash = FNV1A_RT((i8*)out_buffer, out_max_len);

    // double download check
    if(id != 7){
      if(last_download_id == id && last_download_hash == current_data_hash){
        DBG("[-] cloud::download already downloaded this configuration\n");
        return CLOUD_STATUS_ALREADY_DOWNLOADED;
      }
    }

    c_net_request_cloud_config request{};
    {
      memcpy(&request.config_key, gen_internal->cloud_license_key, XOR32(32));

      request.config_id   = id;
    }

    // send the fragment request
    c_net_receive_cloud_config* response = (c_net_receive_cloud_config*)request.transmit(ignore_error);

    if(response == nullptr){
      DBG("[-] cloud::download failed response error\n");
      return CLOUD_STATUS_FETCH_FAILED;
    }

    if(response->status != NET_FRAGMENT_OK){
      DBG("[-] cloud::download status != NET_FRAGMENT_OK (%i)\n", response->status);
      free(response);
      return CLOUD_STATUS_RESPONSE_CORRUPTION;
    }

    if(response->buffer_len < sizeof(u32) || response->buffer_len > out_max_len){
      DBG("[-] cloud::download buffer_len invalid %i/%i\n", response->buffer_len, out_max_len);
      free(response);
      return CLOUD_STATUS_LENGTH_MISMATCH;
    }

    if(*(u32*)&response->buffer != wanted_version_number){
      DBG("[-] cloud::download version number does not match (%i/%i)\n", *(u32*)&response->buffer, wanted_version_number);
      free(response);
      return CLOUD_STATUS_VERSION_CHANGE;
    }

    if(out_response_len != nullptr)
      *out_response_len = response->buffer_len;

    // Finally, copy the transacted buffer into the output buffer
    memcpy(out_buffer, &response->buffer, response->buffer_len);

    // update the current data hash
    current_data_hash = FNV1A_RT((i8*)out_buffer, response->buffer_len);

    // set the last download data
    last_download_id      = id;
    last_download_hash    = current_data_hash;

    // we set this to tell the uploader that we already have this exact same configuration for this id
    last_upload_id        = id;
    last_upload_hash      = current_data_hash;

    DBG("[+] cloud::download success (%i, %X, %i)\n", id, out_buffer, out_max_len);
    free(response);
    return CLOUD_STATUS_OK;
  }

  inline bool from_clipboard(u32 version_number, u32 game_hash, void* out_buffer, u32 out_max_len){
    if(!game_hash || out_buffer == nullptr)
      return false;

    if(!I(OpenClipboard)(nullptr)){
      DBG("[-] from_clipboard: general failure\n");
      return false;
    }

    void* data = I(GetClipboardData)(CF_TEXT);
    if(data == nullptr){
      DBG("[-] from_clipboard: no text data\n");
      return false;
    }

    i8* str = (i8*)(I(GlobalLock)(data));
    if(str == nullptr){
      DBG("[-] from_clipboard: invalid clipboard string\n");
      return false;
    }

    I(GlobalUnlock)(data);
    I(CloseClipboard)();

    u32 len = strlen(str);
    if(!len || len > CONFIG_MAX_CB_SIZE){
      DBG("[-] from_clipboard: invalid strlen %i\n", len);
      return false;
    }

    std::string config_code = str;

    config_code = str_utils::str_replace_all(config_code, CONFIG_COMPRESS_COMBO_THREE_TO, CONFIG_COMPRESS_COMBO_THREE_FROM);
    config_code = str_utils::str_replace_all(config_code, CONFIG_COMPRESS_COMBO_TWO_TO, CONFIG_COMPRESS_COMBO_TWO_FROM);
    config_code = str_utils::str_replace_all(config_code, CONFIG_COMPRESS_COMBO_ONE_TO, CONFIG_COMPRESS_COMBO_ONE_FROM);


    // De-XOR string here?
    s_config_clipboard_data* header_data = (s_config_clipboard_data*)utils::hex_to_bytes(config_code.c_str());
    if(header_data == nullptr){
      DBG("[-] from_clipboard: parsing of clipboard data failed (invalid)\n");
      return false;
    }

    // maybe we can catch noobs in the act.
    if(header_data->patched2 != 0 || header_data->patched != 0){
      DBG("[-] from_clipboard: i've been messed with sadly\n");
      free(header_data);
      return false;
    }

    // Since some people are going to try and crash people's games lets prevent that from happening.
    if(header_data->size_of_me != sizeof(s_config_clipboard_data)){
      DBG("[-] from_clipboard: size_of_me mismatch %i - %i\n", header_data->size_of_me, sizeof(s_config_clipboard_data));
      free(header_data);
      return false;
    }

    if(header_data->size_of_config >= CONFIG_MAX_CB_SIZE){
      DBG("[-] from_clipboard: size_of_config too large %i\n", header_data->size_of_config);
      free(header_data);
      return false;
    }
    
    // Just another sanity check. 
    if(header_data->magic != CONFIG_CB_MAGIC){
      DBG("[-] from_clipboard: header mismatch %X != %X\n", header_data->magic, CONFIG_CB_MAGIC);
      free(header_data);
      return false;
    }

    // We'll hardcode the game type inside the cheat itself.
    if(header_data->game_hash != game_hash){
      DBG("[-] from_clipboard: game hash mismatch %X != %X\n", header_data->game_hash, game_hash);
      free(header_data);
      return false;
    }

    if(header_data->size_of_both != (header_data->size_of_me + header_data->size_of_config)){
      DBG("[-] from_clipboard: size_of_both is not eql to size_of_me + size_of_config\n");
      free(header_data);
      return false;
    }

    if(!header_data->config_data[0]){
      DBG("[-] from_clipboard: invalid config data str\n");
      free(header_data);
      return false;
    }

    void* config = utils::hex_to_bytes(header_data->config_data);
    if(config == nullptr){
      DBG("[-] from_clipboard: parsing of config data failed (invalid)\n");
      free(header_data);
      return false;
    }

    i32 v = *(u32*)config;
    if(v != header_data->version_number){
      DBG("[-] from_clipboard: config version mismatch! %i != %i\n", v, header_data->version_number);
      free(header_data);
      free(config);
      return false;
    }

    u32 config_hash = HASH_RT(header_data->config_data);
    if(config_hash != header_data->config_data_hash){
      DBG("[-] from_clipboard: config data hash mismatch %X != %X\n", config_hash, header_data->config_data_hash);
      free(header_data);
      free(config);
      return false;
    }

    // Everything at this point has been sanity checked.
    memcpy(out_buffer, config, out_max_len);

    free(header_data);
    free(config);

    return true;
  }

  inline bool to_clipboard(u32 version_number, u32 game_hash, void* in_buffer, u32 in_max_len, std::string* out_str, bool set_clipboard = true){
    if(in_buffer == nullptr || out_str == nullptr){
      DBG("[-] to_clipboard: invalid parameters\n");
      return false;
    }

    if(!game_hash){
      DBG("[-] to_clipboard: invalid game hash\n");
      return false;
    }

    DBG("[!] Generating code: %X %i\n", in_buffer, in_max_len);
    std::string config_code = utils::bytes_to_hex((u8*)in_buffer, in_max_len);
    if(config_code.empty()){
      DBG("[-] to_clipboard: failed to generate code\n");
      return false;
    }

    if(config_code.length() >= 0xF4240){
      DBG("[-] to_clipboard: data size exceeds 1 MB\n");
      return false;
    }

    s_config_clipboard_data* header_data = (s_config_clipboard_data*)malloc(sizeof(s_config_clipboard_data) + config_code.length() + 1);
    if(header_data == nullptr){
      DBG("[-] to_clipboard: out of memory\n");
      return false;
    }

    // Setup the sizes!
    header_data->size_of_me     = XOR32(sizeof(s_config_clipboard_data));

    header_data->magic          = CONFIG_CB_MAGIC;
    header_data->game_hash      = game_hash;
    header_data->version_number = version_number;

    header_data->patched  = 0;
    header_data->patched2 = 0;

    header_data->size_of_config   = config_code.length() + 1;
    header_data->size_of_both     = header_data->size_of_me + header_data->size_of_config;

    header_data->config_data_hash = HASH_RT((i8*)config_code.c_str());

    memcpy(&header_data->config_data, config_code.c_str(), header_data->size_of_config);

    std::string master_code = utils::bytes_to_hex((u8*)header_data, header_data->size_of_both);
    master_code = str_utils::str_replace_all(master_code, CONFIG_COMPRESS_COMBO_ONE_FROM, CONFIG_COMPRESS_COMBO_ONE_TO);
    master_code = str_utils::str_replace_all(master_code, CONFIG_COMPRESS_COMBO_TWO_FROM, CONFIG_COMPRESS_COMBO_TWO_TO);
    master_code = str_utils::str_replace_all(master_code, CONFIG_COMPRESS_COMBO_THREE_FROM, CONFIG_COMPRESS_COMBO_THREE_TO);
    if(master_code.empty()){
      DBG("[-] to_clipboard: master_code was empty\n");
      free(header_data);
      return false;
    }
   
    if(set_clipboard){
      if(!I(OpenClipboard)(nullptr)){
        DBG("[-] to_clipboard: (clipboard) general failure\n");
        free(header_data);
        return false;
      }

      if(!I(EmptyClipboard)()){
        DBG("[-] to_clipboard: (clipboard) empty\n");
        free(header_data);
        return false;
      }

      u32 str_len = master_code.length() + 1;
      i8* handle = (i8*)I(GlobalAlloc)(GMEM_MOVEABLE, str_len);
      if(handle == nullptr){
        DBG("[-] to_clipboard: failed to alloc clipboard data (out of memory)\n");
        free(header_data);
        return false;
      }

      memcpy(I(GlobalLock)(handle), master_code.c_str(), master_code.length());
      I(GlobalUnlock)(handle);
      {
        if(I(SetClipboardData)(CF_TEXT, handle) == nullptr){
          DBG("[-] to_clipboard: failed to set data: %X\n", GetLastError());
          free(header_data);
          return false;
        }
        
        I(CloseClipboard)();
      }

      I(GlobalFree)(handle);
    }

    // Didn't really have to this but, just increase we wanted the code for whatever reason. Or maybe we just wanted to calculate it.
    *out_str = master_code;

    return true;
  }
}
#endif