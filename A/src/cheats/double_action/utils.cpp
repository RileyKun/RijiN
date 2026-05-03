#include "link.h"
void utils::lock_mouse(bool lock){
  if(global->focus_overlay_panel_id == 0)
    return;

  global->panel->set_mouse_input_enabled(global->focus_overlay_panel_id, lock);
}

void* utils::get_interface(std::wstring module, u32 name_hash, bool find_list){
  if(module.empty() || name_hash == 0)
    return nullptr;

  uptr create_interface_ptr = (uptr)utils::get_proc_address(utils::get_module_handle(WHASH_RT(module.c_str())), HASH("CreateInterface"));

  if(create_interface_ptr == 0){
    DBG("[-] %ls is invalid\n", module.c_str());
    return nullptr;
  }

  DBG("[!] %ls create interface addr 0x%p\n", module.c_str(), create_interface_ptr);
  uptr jmp               = 0;

  if(find_list){
    jmp = create_interface_ptr + XOR32(5);   // PUSH EBP, MOV EBP, ESP, POP EBP, JMP
    jmp = (jmp + *(i32*)jmp) + XOR32(4);     // jmp eip + jmp rel + IMM32
  }
  else
    jmp = create_interface_ptr;

  s_interface_list* list = **(s_interface_list***)(jmp + XOR32(6)); // PUSH EBP, MOV EBP, ESP, PUSH ESI, MOV ESI, DWORD_00000000
  do{
    if(name_hash == HASH_RT(list->name) && list->init_interface != nullptr){
      DBG("[+] interface: %ls %s\n", module.c_str(), list->name);
      return utils::call_stdcall<void*>(list->init_interface);
    }

  } while(list = list->next);

  DBG("[-] ERROR: Couldn't find interface address for %ls %p\n", module.c_str(), name_hash);
  assert(false);

  return nullptr;
}

ALWAYSINLINE c_base_player* utils::localplayer(){
  return global->entity_list->get_entity(global->engine->get_local_player());
}

void utils::store_cmd_data(s_user_cmd* cmd){
  if(cmd == nullptr)
    return;

  static i32 last_tickcount = 0;
  global->speedhack_tick = last_tickcount == globaldata->tick_count;
  last_tickcount = globaldata->tick_count;

  global->current_cmd = cmd;
  memcpy(&global->last_cmd, &global->original_cmd, sizeof(s_user_cmd));
  memcpy(&global->untouched_cmd, cmd, sizeof(s_user_cmd));
  memcpy(&global->original_cmd, cmd, sizeof(s_user_cmd));
}

c_player_data* utils::get_player_data(i32 index){
  index--;
  if(index < 0 || index >= (sizeof(global->player_data) / sizeof(c_player_data)))
    return nullptr;

  return &global->player_data[index];
}

c_player_esp_settings* utils::get_player_esp_settings(i32 type){
  if(type < 0 || type > 3)
    return nullptr;

  return &config->player_esp[type];
}

c_player_esp_settings* utils::get_player_esp_settings(c_base_player* player){
  if(player == nullptr)
    return nullptr;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return nullptr;

  i32 type = PLAYER_ESP_SETTING_ENEMY;
  if(localplayer->is_steam_friend(player) || player == localplayer)
    type = PLAYER_ESP_SETTING_FRIEND;

  return utils::get_player_esp_settings(type);
}

c_player_chams_settings* utils::get_player_chams_settings(i32 type){
  if(type < 0 || type > 3)
    return nullptr;

  return &config->player_chams[type];
}

c_player_chams_settings* utils::get_player_chams_settings(c_base_player* player){
  if(player == nullptr)
    return nullptr;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return nullptr;

  i32 type = PLAYER_ESP_SETTING_ENEMY;
  if(localplayer->is_steam_friend(player) || player == localplayer)
    type =  PLAYER_ESP_SETTING_FRIEND;

  return utils::get_player_chams_settings(type);
}

bool utils::reset_lag_comp_data(i32 index){
  index--;

  if(index < 0 || index >= LC_MAX_ENTRIES)
    return false;

  memset(&lag_compensation->lc_table[index], 0, sizeof(s_lc_data));
  return true;
}

void utils::reset_all_lag_comp_data(){
  memset(lag_compensation->lc_table, 0, sizeof(s_lc_data) * LC_MAX_ENTRIES);

  DBG("[!] utils::reset_all_lag_comp_data\n");
}

bool utils::reset_player_data(i32 index){
  index--;

  if(index < 0 || index >= (sizeof(global->player_data) / sizeof(c_player_data)))
    return false;

  memset(&global->player_data[index], 0, sizeof(c_player_data));
  return true;
}

void utils::reset_all_player_data(){
  memset(&global->player_data, 0, sizeof(global->player_data));

  DBG("[!] utils::reset_all_player_data\n");
}

void utils::check_to_reset_data(i32 index){
  if(index <= 0)
    return;

  c_player_data* data = utils::get_player_data(index);
  if(data == nullptr)
    return;

  s_player_info info;
  if(!global->engine->get_player_info(index, &info))
    return;

  if(info.friends_id != data->friends_id){
    DBG("[!] friends id mismatch on ID Stored: %i - ID: %i\n", data->friends_id, info.friends_id);
    utils::reset_player_data(index);
    data->friends_id = info.friends_id;
  }
}

void utils::verify_player_data(){
  if(global->verified_player_data)
    return;

  DBG("[!] Verifying player data...\n");
  global->verified_player_data = true;
  for(i32 i = 1; i <= globaldata->max_clients; i++)
    utils::check_to_reset_data(i);
}

void utils::check_for_steam_friends(){
  float time = math::time();
  for(i32 i = 1; i <= globaldata->max_clients; i++){
    if(i == global->engine->get_local_player())
      continue;

    c_base_player* player = global->entity_list->get_entity(i);
    if(player == nullptr)
      continue;

    c_player_data* data = player->data();
    if(data == nullptr)
      continue;

    if(data->next_steam_friend_check > time)
      continue;

    s_player_info info = player->info();
    if(info.fake_player)
      continue;

    data->next_steam_friend_check = time + 5.0f;
    data->steam_friend = global->steamcontext->steamfriends->has_friend(utils::get_user_steamid(info.friends_id));
    DBG("[+] Checking to see if %s is a friend: (result: %i)\n", info.name, data->steam_friend);
  }
}

c_steamid utils::get_user_steamid(u32 friends_id){
  c_steamid steamid;
  steamid.m_steamid.comp.account_id = 0;
  if(friends_id == 0)
    return steamid;

   steamid.m_steamid.comp.account_id       = friends_id;
   steamid.m_steamid.comp.universe         = 1;
   steamid.m_steamid.comp.account_type     = 1;
   steamid.m_steamid.comp.account_instance = 1;
   steamid.m_steamid.bits                  = steamid.m_steamid.comp.account_id + 76561197960265728;

  return steamid;
}

void utils::send_con_notification(i8* txt){
  i8 buf[512];
  wsprintfA(buf, XOR("[RijiN] %s"), txt);

  // Should we do colours at some point? -Rud
  typedef void(__cdecl* func)(const i8*, ...);
  static uptr msg_ptr = (uptr)utils::get_proc_address(utils::get_module_handle(HASH("tier0.dll")), HASH("Msg"));
  static func msg     = (func)msg_ptr;

  msg(buf);
}

bool utils::wait_lag_fix(c_base_player* player){
  if(!config->aimbot.position_adjust_accurate_unlag || player == nullptr /*|| double_tap->is_shift_tick()*/)
    return false;

  s_lc_data* lc_data = lag_compensation->get_data(player);

  if(lc_data == nullptr)
    return false;

  s_lc_record* record1 = lc_data->get_record(0);
  s_lc_record* record2 = lc_data->get_record(1);

  if(record1 == nullptr || record2 == nullptr)
    return false;

  // NOTE: better to use ticks_to_time(23)
  //if(global->engine->get_latency(true) >= 0.070f && !record2->broken)
  //  return false;

  float len = player->velocity().length_2d();

  if(len < 0.1f)
    return false;

  i32 ticks_required_to_break_lc = math::clamp((i32)(((64.f / globaldata->interval_per_tick) / len) + 0.5f), 0, math::time_to_ticks(1.f));
  i32 update_delta               = math::clamp(global->client_state->server_tick - record1->server_tick, 0, math::time_to_ticks(1.f)) + 1 + global->engine->get_latency(true);

  return update_delta > ticks_required_to_break_lc;
}