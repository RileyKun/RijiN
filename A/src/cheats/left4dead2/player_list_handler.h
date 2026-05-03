#pragma once

#define PLAYER_LIST_CONFIG_VERSION 2
#define PLAYER_LIST_CONFIG_ID 7
#define PLAYER_LIST_DEFAULT_ENTRY_SIZE 8

class c_cloud_player_list_data{
public:
  u32   version;
  i32   item_count;
  uptr get_offset(){
    return ((uptr)sizeof(version) + (uptr)(sizeof(item_count)));
  }

  void* get_data(){
    return (void*)((uptr)this + (uptr)get_offset());
  }
};

class c_player_list_cache_data{
public:
  u32 sid;
  c_player_list_data data;
};

#define TEMP_DOWNLOAD_BUFFER_SIZE 0x4C4B40
class c_player_list_handler{
private:
  c_dynamic_indexed_memory<c_player_list_cache_data> cfg;
  bool                  downloaded_locked      = false;
  bool                  needs_to_save          = false;
  bool                  is_allowed_to_save     = false;
  float                 next_allowed_save_time = -1.f;
  bool                  needs_list_generated   = false;
  bool                  list_generated         = true;
  bool                  download_failed        = false;

  c_cloud_player_list_data* temp_download_buffer   = nullptr;
  u32                       download_buffer_len    = 0;
public:
  void* raw_cfg_data      = nullptr;
  u32   raw_cfg_data_size = 0;

  void alloc(){
    
    // CALM DOWN SENATOR THIS IS SIMPLY USED AS A MEMORY CATCH ALL FOR DOWNLOADING CONFIGS.
    temp_download_buffer = malloc(TEMP_DOWNLOAD_BUFFER_SIZE /* 5 MB */);
    download_buffer_len  = 0;
  }

  void dealloc(){
    if(raw_cfg_data == nullptr)
      return;

    cfg.dealloc();
    raw_cfg_data = nullptr;
  }

  void think(bool force_update = false);
  void verify_selected_player();
  c_player_list_cache_data* get_entry(u32 sid);
  bool remove_entry(u32 id);
  bool add_entry(u32 sid, c_player_list_data data);
  bool copy_cache_to_player_data(c_base_player* player);
  bool copy_player_data_to_cache(c_base_player* player);
  u32 get_default_settings_hash();
  void force_update_to_cache();
  void process_cfg_download();
  bool fetch_player_data();
  bool save_player_data();
  void update_cfg_information(){
    if(cfg.get_data() == nullptr)
      return;
    
    raw_cfg_data      = cfg.get_data();
    raw_cfg_data_size = cfg.get_data_size();
    DBG("[!] Updated cfg information\n");
  }

  void destroy_temp_buffers(){
    free(temp_download_buffer);
    temp_download_buffer = nullptr;
  }

  void on_entry_transfer(c_base_player* player, c_entity_data* data);
};

CLASS_EXTERN(c_player_list_handler, player_list_handler);