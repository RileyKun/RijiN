#include "link.h"
CLASS_ALLOC(c_player_list_handler, player_list_handler);

void c_player_list_handler::think(bool force_update){
  static float next_think_time = -1.f;
  float time = math::time();

  if(next_think_time > time && !force_update)
    return;

  next_think_time = time + 0.2f;

  if(!downloaded_locked){ 
    utils::create_worker([](void* p){
      player_list_handler->process_cfg_download();    
      return 0;
    }, nullptr);
  }
  else{
    if(needs_list_generated && temp_download_buffer != nullptr){

      // We weren't able to download the playerlist config.
      if(download_failed || !download_buffer_len){ 
        cfg.init(128); // Init memory.

        needs_list_generated = false;
        list_generated       = true;

        update_cfg_information();
        destroy_temp_buffers();
      
        DBG("[!] Failed to download playerlist from cloud. (Generating new config)\n");
        return;
      }

      DBG("[!] Received playerlist v%i from cloud reported to have %i valid entries\n", temp_download_buffer->version, temp_download_buffer->item_count);
      cfg = c_dynamic_indexed_memory<c_player_list_cache_data>(temp_download_buffer->get_data(), sizeof(c_player_list_cache_data), download_buffer_len, temp_download_buffer->item_count);
      
      // Verify entries here?

      update_cfg_information();
      destroy_temp_buffers();

      needs_list_generated = false;
      list_generated       = true;
    }
  }

  if(!is_allowed_to_save || !list_generated || raw_cfg_data == nullptr)
    return;
  
  if(utils::is_in_game()){
    for(i32 i = 1; i <= globaldata->max_clients; i++){
      if(i == global->localplayer_index)
        continue;

      c_base_player* player = global->entity_list->get_entity(i);
      if(player == nullptr)
        continue;

      c_player_data* data = player->data();
      if(data == nullptr)
        continue;

      if(!data->friends_id)
        continue;

      s_player_info info = player->info();
      if(info.fake_player)
        continue;

      // We only wanna store players that have modified playerlist settings.
      u32 default_hash = get_default_settings_hash();
      u32 hash         = FNV1A_RT((i8*)&data->playerlist, sizeof(c_player_list_data));

      bool has_data_setup     = data->playerlist_hash != 0 && data->playerlist_cache_hash != 0;
      bool has_default_config = default_hash == hash;

      c_player_list_cache_data* entry = get_entry(info.friends_id);
      if(entry != nullptr){
        if(default_hash != hash || !has_data_setup)
          copy_cache_to_player_data(player);
        else{
          // Remove unused entries.
          DBG("[+] Removing sid %i (%s) from playerlist config.\n", info.friends_id, info.name);
          remove_entry(info.friends_id);
          needs_to_save = true;
        }
      }
      else{
        if(default_hash != hash || !has_data_setup)
          add_entry(info.friends_id, data->playerlist);
      }
    }

    // Backup data about players.
    for(i32 i = 1; i <= globaldata->max_clients; i++){
      if(i == global->localplayer_index)
        continue;

      c_base_player* player = global->entity_list->get_entity(i);
      if(player == nullptr)
        continue;

      s_player_info info = player->info();
      if(info.fake_player)
        continue;

      c_player_list_cache_data* entry = get_entry(info.friends_id);
      if(entry != nullptr)
        copy_player_data_to_cache(player);
    }
  }

  save_player_data();
}

void c_player_list_handler::verify_selected_player(){
  global->playerlist_has_valid_player = global->playerlist_current_player_index > 0;
  if(!global->playerlist_has_valid_player)
    return;

  c_base_player* player = global->entity_list->get_entity(global->playerlist_current_player_index);
  if(global->playerlist_current_player_index == global->localplayer_index){
    DBG("[!] Invalidated selected player in playerlist (was localplayer)\n");
    global->playerlist_current_player_index = -1;
    return;
  }

  if(player == nullptr){
    DBG("[!] Invalidated selected player in playerlist (entity nullptr)\n");
    global->playerlist_current_player_index = -1;
    return;
  }

  c_player_data* data = player->data();
  if(data == nullptr){
    DBG("[!] Invalidated selected player in playerlist (player data nullptr)\n");
    global->playerlist_current_player_index = -1;
    return;
  }
#if !defined(RENDER_BOTS)
  if(!data->friends_id){
    DBG("[!] Invalidated selected player in playerlist (player data not setup)\n");
    global->playerlist_current_player_index = -1;
    return;
  }

  if(player->info().fake_player){
    DBG("[!] Invalidated selected player in playerlist (bot player)\n");
    global->playerlist_current_player_index = -1;
    return;
  }
  #endif
}

bool c_player_list_handler::add_entry(u32 sid, c_player_list_data data){
  if(raw_cfg_data == nullptr)
    return false;

  // Already has been added into the list.
  if(get_entry(sid) != nullptr){
    DBG("[!] (playerlist) add_entry: %i has already been added\n", sid);
    return false;
  }

  c_player_list_cache_data new_entry;
  memset(&new_entry, 0, sizeof(c_player_list_cache_data));

  // copy data from c_player_list_data.
  {
    new_entry.sid = sid;
    memcpy(&new_entry.data, &data, sizeof(c_player_list_data));
  }

  cfg.add_entry(new_entry);
  update_cfg_information();

  DBG("[+] (playerlist) add_entry: adding %i to config\n", sid);
  needs_to_save = true;
  return true;
}

// Copies playerlist data from playerlist config to c_player_list playerlist.
bool c_player_list_handler::copy_cache_to_player_data(c_base_player* player){
  if(raw_cfg_data == nullptr)
    return false;

  if(player == nullptr)
    return false;

  c_player_data* data = player->data();
  if(data == nullptr)
    return false;

  if(!data->friends_id)
    return false;

  s_player_info info = player->info();
  if(info.fake_player)
    return false;

  c_player_list_cache_data* entry = get_entry(info.friends_id);
  if(entry == nullptr){
    DBG("[-] copy_cache_to_player_data failed: player %s has no entry for their id\n", info.name);
    return false;
  }

  u32 hash = FNV1A_RT((i8*)&entry->data, sizeof(c_player_list_data));
  if(hash == data->playerlist_cache_hash)
    return false;

  DBG("[!] (cache hash) %s: %p\n", info.name, hash);
  data->playerlist_cache_hash = hash;

  DBG("[+] copy_cache_to_player_data: %s moving cached player list data to player data\n", info.name);
  memcpy(&data->playerlist, &entry->data, sizeof(c_player_list_cache_data));
  on_entry_transfer(player, data);

  return true;
}

// Copies playerlist data from c_player_data into player list config.
bool c_player_list_handler::copy_player_data_to_cache(c_base_player* player){
  if(raw_cfg_data == nullptr)
    return false;

  if(player == nullptr)
    return false;

  c_player_data* data = player->data();
  if(data == nullptr)
    return false;

  if(!data->friends_id)
    return false;

  s_player_info info = player->info();
  if(info.fake_player)
    return false;

  c_player_list_cache_data* entry = get_entry(info.friends_id);
  if(entry == nullptr){
    DBG("[-] copy_cache_to_player_data failed: player %s has no entry for their id\n", info.name);
    return false;
  }

  u32 hash = FNV1A_RT((i8*)&data->playerlist, sizeof(c_player_list_data));
  if(hash == data->playerlist_hash)
    return false;

  needs_to_save               = true;
  data->playerlist_hash       = hash;
  data->playerlist_cache_hash = hash;
  DBG("[+] copy_cache_to_player_data: %s moving cached player list data to player data\n", info.name);
  memcpy(&entry->data, &data->playerlist, sizeof(c_player_list_data));

  return true;
}

c_player_list_cache_data* c_player_list_handler::get_entry(u32 sid){
  if(!sid)
    return nullptr;

  for(u32 i = 0; i < cfg.get_item_count(); i++){
    c_player_list_cache_data* entry = cfg.get_entry(i);
    if(entry == nullptr)
      continue;

    if(entry->sid != sid)
      continue;

    return entry;
  }

  return nullptr;
}

bool c_player_list_handler::remove_entry(u32 sid){
  if(!sid)
    return false;

  for(u32 i = 0; i < cfg.get_item_count(); i++){
    c_player_list_cache_data* entry = cfg.get_entry(i);
    if(entry == nullptr)
      continue;

    if(entry->sid != sid)
      continue;

    DBG("[!] (playerlist) remove_entry: Removing %i at pos %i in list\n", sid, i);
    cfg.remove(i);
    return true;
  }

  return false;
}

u32 c_player_list_handler::get_default_settings_hash(){
  static c_player_list_data data;
  static bool first_run = false;
  if(!first_run){
    data.init();
    first_run = true;
  }

  static u32 hash = FNV1A_RT((i8*)&data, sizeof(c_player_list_data));
  return hash;
}

void c_player_list_handler::force_update_to_cache(){
  if(!utils::is_in_game())
    return;

  DBG("[+] Forcing update to playerlist config");
  for(i32 i = 1; i <= globaldata->max_clients; i++){
    if(i == global->localplayer_index)
      continue;

    c_base_player* player = global->entity_list->get_entity(i);
    if(player == nullptr)
      continue;

    c_player_data* data = player->data();
    if(data == nullptr)
      continue;

    data->playerlist_cache_hash = 0;
  }
}

void c_player_list_handler::process_cfg_download(){
  static bool first_run            = false; // First run as in the first time this function runs not the playerlist config.
  static float first_run_wait_time = -1.f;

  // Disallow any redownloading.
  if(downloaded_locked)
    return;

  float time = math::time();
  if(!first_run){
    first_run_wait_time = time + 1.f;
    first_run = true;
    return;
  }

  // Wait!
  if(first_run_wait_time > time)
    return;

  first_run_wait_time = time + 1.f;

  // We need to try at least 3 times.
  // We're trying to avoid a case where people can mistakenly override their playerlist configs.
  // This however should be very unlikely due to an internet connection being required.
  // I'm not gonna bother putting any more effort into this than this.
  // Obviously this will fail 3 times for everyone when loading the cheat for the first time.
  // From what i'm aware of the server doesn't tell us why it failed. Which I suppose would be helpful.
  static i32 tries = 0;

  // Lets try and fetch player data now.
  if(fetch_player_data()){
    is_allowed_to_save     = true;
    next_allowed_save_time = time + 5.f;
    downloaded_locked      = true;
    needs_list_generated   = true;
    download_failed        = false;

    DBG("[+] Successfully loaded playerlist data.\n");
    return;
  }

  tries++;
  if(tries >= 3){
    DBG("[-] Failed to download playerlist cfg from cloud. (No cfg present, cfg too big, or some unknown error)\n")
    downloaded_locked      = true;
    is_allowed_to_save     = true;
    next_allowed_save_time = time + 5.f;
    first_run_wait_time    = time + 1.f;
    download_failed        = true;
    needs_list_generated   = true;
    return;
  }
}

// Call process_cfg_download
bool c_player_list_handler::fetch_player_data(){
  if(temp_download_buffer == nullptr)
    return false;

  return cloud::download(PLAYER_LIST_CONFIG_ID, temp_download_buffer, TEMP_DOWNLOAD_BUFFER_SIZE, PLAYER_LIST_CONFIG_VERSION, true, &download_buffer_len);
}

bool c_player_list_handler::save_player_data(){
  if(!needs_to_save | !is_allowed_to_save)
    return false;


  float time                  = math::time();
  if(next_allowed_save_time > time)
    return true;

  needs_to_save          = false;
  next_allowed_save_time = time + 2.f;

  update_cfg_information();
  if(raw_cfg_data == nullptr)
    return;
  
  utils::create_worker([](void* p){
    if(player_list_handler->raw_cfg_data == nullptr)
      return 0;

    DBG("[!] Performing playerlist data upload to cloud.\n");
    c_cloud_player_list_data* temp_upload_buffer = malloc(player_list_handler->raw_cfg_data_size + (sizeof(u32) + sizeof(u32)));

    temp_upload_buffer->version    = PLAYER_LIST_CONFIG_VERSION;
    temp_upload_buffer->item_count = player_list_handler->cfg.get_item_count();

    memcpy(temp_upload_buffer->get_data(), player_list_handler->raw_cfg_data, player_list_handler->raw_cfg_data_size);
    if(cloud::upload(PLAYER_LIST_CONFIG_ID, temp_upload_buffer, player_list_handler->raw_cfg_data_size + (sizeof(u32) + sizeof(u32)), true)){
      DBG("[+] Successfully uploaded playerlist data\n");
    }
    else{
      DBG("[-] Failed to upload playerlist data\n");
    }

    // Free the temp buffer.
    free(temp_upload_buffer);
    temp_upload_buffer = nullptr;

    DBG("[!] Performed playerlist config upload.\n");
    return 0;
  }, nullptr);

  return true;
}

void c_player_list_handler::on_entry_transfer(c_base_player* player, c_player_data* data){
  if(player == nullptr || data == nullptr)
    return;

 // cheat_detection->on_playerlist_cfg_load(player);
}