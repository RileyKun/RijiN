#include "link.h"
void utils::lock_mouse(bool lock){
  if(global->focus_overlay_panel_id == 0)
    return;

  global->panel->set_mouse_input_enabled(global->focus_overlay_panel_id, lock);
}

void* utils::get_interface(std::wstring module, std::string name){
  if(module.empty() || name.empty())
    return nullptr;

  void* create_interface_ptr = utils::get_proc_address(utils::get_module_handle(WHASH_RT(module.c_str())), HASH("CreateInterface"));

  if(create_interface_ptr == nullptr){
    DBG("[-] %ls is invalid\n", module.c_str());
    return nullptr;
  }

  DBG("[!] %ls create interface addr 0x%p\n", module.c_str(), create_interface_ptr);

  void* result = utils::call_stdcall<void*, i8*, i32*>(create_interface_ptr, name.c_str(), nullptr);
  if(result != nullptr){
    DBG("[+] interface: %ls %s (%p)\n", module.c_str(), name.c_str(), result);
    return result;
  }

  DBG("[-] ERROR: Couldn't find interface address for %ls %s\n", module.c_str(), name.c_str());
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

void utils::update_local_data(){
  global->localplayer_index = global->engine->get_local_player();
  global->localplayer = utils::localplayer();
  if(global->localplayer != nullptr){
    global->localplayer_team = global->localplayer->team;
    global->localplayer_wep  = global->localplayer->get_weapon();
  }
}

// GCC was being retarded again.
CFLAG_O0 void utils::update_player_information(i32 stage){
  if(stage != FRAME_NET_UPDATE_END)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  c_player_resource* pr = utils::get_player_resource();

  float time = math::time();
  for(i32 i = 1; i <= globaldata->max_clients; i++){
    c_base_player* player = global->entity_list->get_entity(i);
    if(player == nullptr)
      continue;

    c_player_data* data = player->data();
    if(data == nullptr)
      continue;

    if(player->simulation_time > data->grabbed_by_physgun_sim_time)
      data->grabbed_by_physgun = false;

    c_base_weapon* wep = player->get_weapon();

    data->gmod_data.armor = pr->get_armor(i);

    // Handle resetting of fetched lua data.
    if(data->gmod_data.next_lua_fetch <= time){
      data->gmod_data.user_group_hash = 0;
      memset(&data->gmod_data.weapon_name, 0, sizeof(data->gmod_data.weapon_name));
      memset(&data->gmod_data.unique_name, 0, sizeof(data->gmod_data.unique_name));
      memset(&data->gmod_data.user_group_str, 0, sizeof(data->gmod_data.user_group_str));
    }

    // Try and see if we can detect these and then try bypass it.
    ILuaInterface* lua = global->lua_shared->GetLuaInterface(LUAINTERFACE_CLIENT);
    if(lua != nullptr && data->gmod_data.next_lua_fetch <= time){
      
      // GetName
      {
        player->push_entity();
        lua->GetField(-1, XOR("GetName"));
        if(lua->IsType(-1, Type::FUNCTION)){

          lua->Push(-2);
          lua->Call(1, 1);

          i8* value = lua->GetString(-1);
          if(value != nullptr)
            wsprintfW(data->gmod_data.unique_name, WXOR(L"%ls"), convert::str2wstr(value).c_str());

          lua->Pop(2);
        }
        else
          lua->Pop(2);
      }

      // GetUserGroup
      {
        player->push_entity();
        lua->GetField(-1, XOR("GetUserGroup"));
        if(lua->IsType(-1, Type::FUNCTION)){
          lua->Push(-2);
          lua->Call(1, 1);

          i8* value = lua->GetString(-1);
          if(value != nullptr){
            wsprintfW(data->gmod_data.user_group_str, WXOR(L"%ls"), convert::str2wstr(value).c_str());
            data->gmod_data.user_group_hash = HASH_RTL(value);
          }

          lua->Pop(2);
        }
        else
          lua->Pop(2);
      }

      // HasGodMode
      {
        player->push_entity();
        lua->GetField(-1, XOR("HasGodMode"));
        if(lua->IsType(-1, Type::FUNCTION)){
          lua->Push(-2);
          lua->Call(1, 1);

          data->gmod_data.has_god_mode = lua->GetBool(-1);         
          lua->Pop(2);
        }
        else
          lua->Pop(2);
      }

      // IsPolice
      if(config->misc.custom_server_integrations){
        player->push_entity();
        lua->GetField(-1, XOR("IsPolice"));
        if(lua->IsType(-1, Type::FUNCTION)){
          lua->Push(-2);
          lua->Call(1, 1);

          data->gmod_data.is_perpheads_police = lua->GetBool(-1);
          data->gmod_data.is_perpheads        = true;    
          lua->Pop(2);
        }
        else
          lua->Pop(2);
      }
      else{
        data->gmod_data.is_perpheads_police = false;
        data->gmod_data.is_perpheads        = false;
      }

      data->gmod_data.next_lua_fetch = time + 1.f;
    }

    if(wep != nullptr){
      wsprintfW(data->gmod_data.weapon_name, WXOR(L"%ls"), wep->get_weapon_name().c_str());

      i8* class_name = wep->get_name();
      if(class_name != nullptr){
        u32 class_hash = HASH_RT(class_name);
        if(class_hash != data->gmod_data.weapon_class_hash){

          // Fetch weapon types from string checks.
          {
            data->gmod_data.is_melee    =     str_utils::contains(class_name, XOR("knife")) 
                                          || str_utils::contains(class_name, XOR("stick"))
                                          || str_utils::contains(class_name, XOR("fist"))
                                          || str_utils::contains(class_name, XOR("sword"))
                                          || str_utils::contains(class_name, XOR("axe"))
                                          || str_utils::contains(class_name, XOR("baton"))
                                          || str_utils::contains(class_name, XOR("hammer"))
                                          || str_utils::contains(class_name, XOR("mace"))
                                          || str_utils::contains(class_name, XOR("club"))
                                          || str_utils::contains(class_name, XOR("pipe"))
                                          || str_utils::contains(class_name, XOR("wrench"))
                                          || str_utils::contains(class_name, XOR("sword"))
                                          || str_utils::contains(class_name, XOR("katana"))
                                          || str_utils::contains(class_name, XOR("cleaver"))
                                          || str_utils::contains(class_name, XOR("machete"))
                                          || str_utils::contains(class_name, XOR("blade"))
                                          || str_utils::contains(class_name, XOR("brick"))
                                          || str_utils::contains(class_name, XOR("bottle"))
                                          || str_utils::contains(class_name, XOR("shovel"))
                                          || str_utils::contains(class_name, XOR("torch"))
                                          || str_utils::contains(class_name, XOR("pan"))
                                          || str_utils::contains(class_name, XOR("crowbar"));

            data->gmod_data.is_harmless =    str_utils::contains(class_name, XOR("tool")) ||
                                             str_utils::contains(class_name, XOR("physgun")) ||
                                             str_utils::contains(class_name, XOR("gravgun")) ||
                                             str_utils::contains(class_name, XOR("weld")) ||
                                             str_utils::contains(class_name, XOR("duplicator")) ||
                                             str_utils::contains(class_name, XOR("remover")) ||
                                             str_utils::contains(class_name, XOR("spawner")) ||
                                             str_utils::contains(class_name, XOR("keypad")) ||
                                             str_utils::contains(class_name, XOR("cracker")) ||
                                             str_utils::contains(class_name, XOR("lockpick")) ||
                                             str_utils::contains(class_name, XOR("key")) ||
                                             str_utils::contains(class_name, XOR("remote")) ||
                                             str_utils::contains(class_name, XOR("camera")) ||
                                             str_utils::contains(class_name, XOR("medkit")) ||
                                             str_utils::contains(class_name, XOR("defibrillator")) ||
                                             str_utils::contains(class_name, XOR("healer")) ||
                                             str_utils::contains(class_name, XOR("bandage")) ||
                                             str_utils::contains(class_name, XOR("syringe")) ||
                                             str_utils::contains(class_name, XOR("med")) ||
                                             str_utils::contains(class_name, XOR("admin")) ||
                                             str_utils::contains(class_name, XOR("banhammer")) ||
                                             str_utils::contains(class_name, XOR("freezeray")) ||
                                             str_utils::contains(class_name, XOR("cleanup")) ||
                                             str_utils::contains(class_name, XOR("cleaner")) ||
                                             str_utils::contains(class_name, XOR("teleporter")) ||
                                             str_utils::contains(class_name, XOR("scanner")) ||
                                             str_utils::contains(class_name, XOR("binoculars")) ||
                                             str_utils::contains(class_name, XOR("radar")) ||
                                             str_utils::contains(class_name, XOR("finder")) ||
                                             str_utils::contains(class_name, XOR("detector")) ||
                                             str_utils::contains(class_name, XOR("rope")) ||
                                             str_utils::contains(class_name, XOR("thruster")) ||
                                             str_utils::contains(class_name, XOR("hoverball")) ||
                                             str_utils::contains(class_name, XOR("light")) ||
                                             str_utils::contains(class_name, XOR("emitter")) ||
                                             str_utils::contains(class_name, XOR("trails")) ||
                                             str_utils::contains(class_name, XOR("paint")) ||
                                             str_utils::contains(class_name, XOR("fire_extinguisher")) ||
                                             str_utils::contains(class_name, XOR("breaching_charge")) ||
                                             str_utils::contains(class_name, XOR("police_baton")) ||
                                             str_utils::contains(class_name, XOR("traffic_cone")) ||
                                             str_utils::contains(class_name, XOR("traffic_barrier")) ||
                                             str_utils::contains(class_name, XOR("speed_gun")) ||
                                             str_utils::contains(class_name, XOR("handcuffs")) ||
                                             str_utils::contains(class_name, XOR("evidence_bag")) ||
                                             str_utils::contains(class_name, XOR("firehose")) ||
                                             str_utils::contains(class_name, XOR("fireaxe")) ||
                                             str_utils::contains(class_name, XOR("defuse_kit")) ||
                                            str_utils::contains(class_name, XOR("drug_lab")) ||
                                            str_utils::contains(class_name, XOR("plantpot")) ||
                                            str_utils::contains(class_name, XOR("wrench")) ||
                                            str_utils::contains(class_name, XOR("repair_kit")) ||
                                            str_utils::contains(class_name, XOR("jerrycan"));
          }
          
          DBG("[!] Updated info about %s\n[!] is_considered melee: %i\n[!] is_considered_harmless: %i\n[!] is_semi_auto: %i\n", class_name, data->gmod_data.is_melee, data->gmod_data.is_harmless, data->gmod_data.is_semi_auto);
          data->gmod_data.weapon_class_hash = class_hash;                        
        }
      }

      // When you first get the weapon, it'll fail, so we'll just check every player update.
      // Check if the weapon uses the Automatic field.
      {
        if(lua != nullptr){
          data->gmod_data.is_semi_auto = false;
    
          if(wep->is_lua_weapon()){
            wep->push_entity();
            lua->GetField(-1, XOR("Primary"));
            if(lua->IsType(-1, Type::TABLE)){
              
              lua->GetField(-1, XOR("Automatic"));
              if(lua->IsType(-1, Type::BOOL))
                data->gmod_data.is_semi_auto = !lua->GetBool(-1);
      
              lua->Pop(1);
            }      
      
            lua->Pop(2);  
          }
          else
            data->gmod_data.is_semi_auto = false;
        }
      }
  

      c_base_entity* physgun_grab = wep->get_physgun_grabbed_entity();
      if(physgun_grab != nullptr){
        if(physgun_grab->is_player()){
          c_base_player* grabbed_player = (c_base_player*)physgun_grab;

          c_player_data* grabbed_data = grabbed_player->data();
          if(grabbed_data != nullptr){
            grabbed_data->grabbed_by_physgun_sim_time = grabbed_player->simulation_time;
            grabbed_data->grabbed_by_physgun          = true;
          }
        }
      }
    }
    else
      wsprintfW(data->gmod_data.weapon_name, WXOR(L"NO  WEAPON"));
  }

}

c_player_data* utils::get_player_data(i32 index){
  index--;
  if(index < 0 || index >= (sizeof(global->player_data) / sizeof(c_player_data)))
    return nullptr;

  return &global->player_data[index];
}

c_entity_data* utils::get_entity_data(i32 index){
  index--;
  if(index < 0 || index > (sizeof(global->entity_data) / sizeof(c_entity_data))){
    DBG("[!] get_entity_data overflow: %i\n", index);
    return nullptr;
  }

  return &global->entity_data[index];
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

  i32 type = localplayer->is_steam_friend(player) ? PLAYER_ESP_SETTING_TYPE_FRIEND : PLAYER_ESP_SETTING_TYPE_NORMAL;
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

  i32 type = localplayer->is_steam_friend(player) ? PLAYER_ESP_SETTING_TYPE_FRIEND : PLAYER_ESP_SETTING_TYPE_NORMAL;
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

  if(global->playerlist_current_player_index == index + 1){
    global->playerlist_current_player_index = -1;
    global->playerlist_has_valid_player     = false;
    DBG("[!] Resetting playerlist selected index: %i - %i\n", global->playerlist_current_player_index, index + 1);
  }

  memset(&global->player_data[index], 0, sizeof(c_player_data));
  global->player_data[index].playerlist.init();
  return true;
}

bool utils::reset_entity_data(i32 index){
  index--;

  if(index < 0 || index >= (sizeof(global->entity_data) / sizeof(c_entity_data)))
    return false;

  memset(&global->entity_data[index], 0, sizeof(c_entity_data));
  return true;
}

void utils::reset_all_player_data(){
  memset(&global->player_data, 0, sizeof(global->player_data));

  DBG("[!] utils::reset_all_player_data\n");
}

void utils::reset_all_entity_data(){
  memset(&global->entity_data, 0, sizeof(global->entity_data));
  DBG("[!] utils::reset_entity_data\n");
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

void utils::check_entity_data(i32 index){
  c_entity_data* data = utils::get_entity_data(index);
  if(data == nullptr)
    return;

  if(!data->class_name_hash)
    return;

  c_base_entity* entity = global->entity_list->get_entity(index);
  if(entity == nullptr){
    reset_entity_data(index);
    return;
  }

  i8* class_name = entity->get_scripted_class_name(true);
  if(class_name == nullptr){
    reset_entity_data(index);
    return; 
  }

  u32 hash = HASH_RT(class_name);
  if(hash != data->class_name_hash)
    reset_entity_data(index);
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
    if(i == global->localplayer_index)
      data->steam_friend = true;
    else
      data->steam_friend = data->playerlist.mark_as_friend || global->steamcontext->steamfriends->has_friend(utils::get_user_steamid(info.friends_id));
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

  // If not valid then get the latest tick.
  s_lc_record* record  = lc_data->get_record(0);
  if(record == nullptr)
    return false;

  // Hopefully you can understand my concern with this.
  i32 latency_ticks = math::time_to_ticks(global->engine->get_latency(true));

  // Lerp is considered an intentional latency by source engine design.
  //latency_ticks += math::time_to_ticks(math::get_lerp());

  // NOTE: better to use ticks_to_time(23)
  //if(global->engine->get_latency(true) >= 0.070f && !record2->broken)
  //  return false;

  float len = player->velocity().length_2d();

  if(len < 0.1f)
    return false;

  i32 ticks_required_to_break_lc = math::clamp((i32)(((64.f / globaldata->interval_per_tick) / len) + 0.5f), 0, math::time_to_ticks(1.f));
  i32 update_delta               = math::clamp(global->client_state->server_tick - record->server_tick, 0, math::time_to_ticks(1.f)) + latency_ticks;

  return update_delta > ticks_required_to_break_lc;
}

bool utils::is_call_from_rijin(void* addr){
  if(addr == nullptr)
    return false;

  assert(global->base_address_start != nullptr);
  assert(global->base_address_end != nullptr);

  return (uptr)addr >= global->base_address_start && (uptr)addr <= (uptr)global->base_address_end;
}

bool utils::is_screen_shot_taken(){
  if(anti_screengrab->is_screenshot_taken())
    return true;

  if(global->movie_name_addr != nullptr){
    i8* movie = (i8*)(global->movie_name_addr);
    if(movie != nullptr){
      if(movie[0] != 0)
        return true;
    }
  }

  return global->engine->is_taking_screenshot();
}

bool utils::is_in_thirdperson(){
  if(freecam->is_active() || thirdperson->is_active() || thirdperson->is_in_thirdperson_view())
    return true;

  return false;
}

bool utils::is_in_game(){
  return utils::localplayer() != nullptr;
}

void utils::sync_entity_list_data(){

  float time = math::time();

  static i32   last_sync_id   = 0;
  static float next_sync_time = 0.f;

  // This can be very resourceful, so we'll only run it every 250ms if the selected index is the same.
  if(next_sync_time > time && last_sync_id == entity_list_data->selected_index)
    return;

  last_sync_id   = entity_list_data->selected_index;
  next_sync_time = time + 0.25f;

  // Find new entities to show on the entity list.
  {
    for(i32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
      utils::check_entity_data(i);
      c_base_entity* entity = global->entity_list->get_entity(i);
      if(entity == nullptr)
        continue;
  
      c_entity_data* data = entity->data();
      if(data == nullptr)
        continue;
  
      //if(data->class_name_hash != 0){
      //  DBG("[!] %i has class_name_hash: %X\n", i, data->class_name_hash);
      //  continue;
      //}
  
      i8* class_name = entity->get_scripted_class_name(true);
      if(class_name == nullptr)
        continue; 

      u32 hash = HASH_RT(class_name);
        
      static wchar_t w_class_name[1024];
      convert::str2wstr(class_name, w_class_name, sizeof(w_class_name));
    
      std::wstring print_name = entity->get_print_name();
     
      entity_list_data->add_entry(i, w_class_name, print_name.length() > 0 ? print_name.c_str() : nullptr);
      data->class_name_hash = hash;
    }
  }

  // Sync up entries with entities.
  entity_list_data->sync_selected_item_entity_data(); // If there's a selected entity sync it also.
  for(i32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* entity = global->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    c_entity_data* data = entity->data();
    if(data == nullptr)
      continue;

    if(!data->class_name_hash)
      continue;

    i8* class_name = entity->get_scripted_class_name(true);
    if(class_name == nullptr)
      continue;

    u32 hash = HASH_RT(class_name);
    s_entity_list_data* entry = entity_list_data->get_data_from_hash(hash);
    if(entry == nullptr)
      continue;

    //DBG("[!] Copying entry->entity_data to c_entity_data (%ls)\n", entry->class_name);
    memcpy(&data->list_data, &entry->entity_data, sizeof(c_entity_list_data));
  }
}

c_world_esp_settings* utils::get_world_esp_settings(c_base_entity* entity){
  if(entity == nullptr)
    return nullptr;

  c_entity_data* data = entity->data();
  if(data == nullptr)
    return nullptr;

  if(data->list_data.use_custom_settings)
    return &data->list_data.esp_settings;
  
  if(!data->list_data.esp_settings.object_enabled)
    return nullptr;  

  return &config->world_esp;
}

c_world_chams_settings*  utils::get_world_chams_settings(c_base_entity* entity){
  if(entity == nullptr)
    return nullptr;

  c_entity_data* data = entity->data();
  if(data == nullptr)
    return nullptr;

  if(data->list_data.use_custom_settings)
    return &data->list_data.chams_settings;
  
  if(!data->list_data.chams_settings.object_enabled)
    return nullptr;  

  return &config->world_chams;
}

c_world_glow_settings* utils::get_world_glow_settings(c_base_entity* entity){
  if(entity == nullptr)
    return nullptr;

  c_entity_data* data = entity->data();
  if(data == nullptr)
    return nullptr;

  if(data->list_data.use_custom_settings)
    return &data->list_data.glow_settings;
  
  if(!data->list_data.glow_settings.object_enabled)
    return nullptr;  

  return &config->world_glow;
}

void utils::collect_entity_attachments(){
  for(u32 i = 1; i <= global->entity_list->get_highest_index(); i++){
    c_entity_data* entity_data = utils::get_entity_data(i);
    if(entity_data == nullptr)
      continue;

    entity_data->attachment_count = 0;
  }

  for(u32 i = 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* entity = global->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    if(entity->is_dormant())
      continue;

    if(entity->is_prop())
      continue;

    if(entity->get_model() == nullptr)
      continue;

    s_client_class* cc = entity->get_client_class();
    if(cc == nullptr)
      continue;

    // Blacklisted entities.
    if(cc->id == CPredictedViewModel 
      || cc->id == CBaseViewModel
      || cc->id == CSENT_anim 
      || cc->id == CBaseCombatCharacter
      || cc->id == CBaseDoor
      || cc->id == CBaseGrenade
      || cc->id == CCrossbowBolt)
      continue;
    
    c_base_entity* owner = entity->get_owner();
    if(owner == nullptr)
      continue;

    if(owner == entity)
      continue;

    // Ignore non held weapons.
    if(owner->is_player() && entity->is_combat_weapon()){
      c_base_player* player_owner = (c_base_player*)owner;
      c_base_weapon* cur_wep = player_owner->get_weapon();
      if(cur_wep == nullptr)
        continue;

      if(cur_wep->get_index() != i)
        continue;
    }

    c_entity_data* entity_data = owner->data();
    if(entity_data == nullptr)
      continue;

    entity_data->attachment_index_table[entity_data->attachment_count] = i;
    entity_data->attachment_count++;
  }
}

c_player_resource* utils::get_player_resource(){
  if(!utils::is_in_game())
    return nullptr;

  for(u32 i = 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* entity = global->entity_list->get_entity(i);

    if(entity == nullptr)
      continue;

    s_client_class* cc = entity->get_client_class();

    if(cc == nullptr)
      continue;

    if(cc->id != CPlayerResource)
      continue;

    return (c_player_resource*)entity;
  }

  return nullptr;
}

float utils::get_command_latency(){
  assert(global->client_state != nullptr);
  float latency = utils::get_latency();
  if(global->client_state->choked_commands > 0)
    latency += math::ticks_to_time(global->client_state->choked_commands);

  return latency;
}

float utils::get_latency(){
  return global->engine->get_latency(false, true) + global->engine->get_latency(true, true);
}

bool utils::process_game_event(c_game_event* event){
  if(event == nullptr)
    return false;

  u32 name_hash = HASH_RT(event->get_name());
  if(name_hash == HASH("client_disconnect") || name_hash == HASH("server_spawn") || name_hash == HASH("client_beginconnect")){
    antiaim->reset();
    acp->reset();
    global->reset();
    freecam->reset();
    thirdperson->reset();
    entity_list_data->reset();
    global->engine->client_cmd(XOR("mem_compact"));

    if(!global->engine->is_playing_demo()){
      if(config->misc.auto_disable_hvh_on_leave){
        config->hvh.fakelag                = false;
        config->hvh.antiaim                = false;
        config->hvh.fakelag_peek           = false;
        config->hvh.auto_stop              = false;
        config->hvh.peek_assist            = false;
        config->hvh.fakelag_anti_backtrack = false;
  
        config->hvh.break_animations_disabled   = true;
        config->hvh.break_animations_on_jump    = false;
        config->hvh.break_animations_constant   = false;
  
        config->hvh.fakeduck_disabled           = true;
        config->hvh.fakeduck_onduck             = false;
        config->hvh.fakeduck_always             = false;
        config->hvh.resolver                    = false;
        config->hvh.resolver_allow_manual_cycle = false;
  
        notify->create(NOTIFY_TYPE_INFO, WXOR(L"Ensuring HvH features are disabled."));
      }
    }
  }
  else if(name_hash == HASH("player_disconnect")){
    i32 id = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));

    utils::reset_player_data(id);
    utils::reset_lag_comp_data(id);
  }
  else if(name_hash == HASH("player_connect_client")){
    i32 id = global->engine->get_player_for_user_id(event->get_int(XOR("index")));

    utils::reset_player_data(id);
    utils::reset_lag_comp_data(id);
  }
  else if(name_hash == HASH("player_death")){
    i32 attacked = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));
    i32 attacker = global->engine->get_player_for_user_id(event->get_int(XOR("attacker")));

    cheat_detection->on_death(attacker, attacked);
    utils::reset_lag_comp_data(attacked);
  }
  else if(name_hash == HASH("player_spawn")){
    i32 index = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));
    utils::reset_lag_comp_data(index);
  }
  else if(name_hash == HASH("player_hurt")){
    i32 attacker = global->engine->get_player_for_user_id(event->get_int(XOR("attacker")));
    cheat_detection->snap_check(attacker);
    resolver->fire_event(event);
  }

  return false;
}

bool utils::get_cheater_data_from_network(u32 sid, u32 name_hash, c_net_request_tf_cheater_sid32_response*& _response){
  c_net_request_tf_cheater_sid32 msg;
  memcpy(&msg.config_key, gen_internal->cloud_license_key, XOR32(32));
  msg.sid       = sid;
  msg.name_hash = name_hash;
  msg.game      = HASH("GMOD");

  DBG("[!] Submitting [U:1:%i] to network for cheater check\n", sid);

  global->is_talking_to_tfcl = true;
  c_net_request_tf_cheater_sid32_response* response = (c_net_request_tf_cheater_sid32_response*)msg.transmit(true);
  global->is_talking_to_tfcl = false;

  if(response == nullptr)
    return false;

  if(!response->alias_len)
    memset(&response->alias[0], 0, XOR32(sizeof(response->alias)));

  if(!response->group_len)
    memset(&response->group[0], 0, XOR32(sizeof(response->group)));

  // This will never happen but it's good to be safe!
  if(response->alias_len > XOR32(32)){
    memset(&response->alias[0], 0, XOR32(sizeof(response->alias)));
    DBG("[-] Received invalid alias length size from server!\n");
    response->alias_len = 0;
  }

  if(response->group_len > XOR32(32)){
    memset(&response->group[0], 0, XOR32(sizeof(response->group)));
    DBG("[-] Received invalid group length size from server!\n");
    response->group_len = 0;
  }

  _response = response;
  return true;
}

struct s_submit_cheater_data{
  u32 sid;
  u32 reason;
  u32 server_type;
};

void utils::submit_cheater_to_network(u32 sid, u32 reason){
  s_submit_cheater_data* data = malloc(sizeof(s_submit_cheater_data));
  if(data == nullptr)
    return;

  data->sid         = sid;
  data->reason      = reason;
  data->server_type = globaldata->max_clients == 1 ? 1 : 2;

  // Demo recording in gmod can provide false positives? Possibly.
  if(global->engine->is_playing_demo())
    data->server_type = XOR32(4);

  utils::create_worker([](s_submit_cheater_data* data){
    while(global->is_talking_to_tfcl)
      Sleep(XOR32(250));

    c_net_submit_tf_cheater_sid32 msg;
    memcpy(&msg.config_key, gen_internal->cloud_license_key, XOR32(32));
    msg.sid         = data->sid;
    msg.reason      = data->reason;
    msg.server_type = data->server_type;
    msg.game        = HASH("GMOD");

    DBG("[!] Submitting cheater [U:1:%i] (%i) to network\n", data->sid, data->reason);
    global->is_talking_to_tfcl = true;
    msg.transmit(true);
    global->is_talking_to_tfcl = false;
    DBG("[+] [U:1:%i] Submitted to network\n", data->sid);

    free(data);
    data = nullptr;

    return 0;
  }, data);
}