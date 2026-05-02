#include "link.h"

c_player_data* utils::get_player_data(i32 index){
  index--;

  if(index < 0 || index >= (sizeof(global->player_data) / sizeof(c_player_data)))
    return nullptr;

  return &global->player_data[index];
}

c_entity_data* utils::get_entity_data(i32 index){
  return global->memory_manager.get_data(index);
}

bool utils::reset_transform(i32 index){
  c_player_data* data = utils::get_player_data(index);
  if(data == nullptr)
    return false;

  // We won't actually memset the transform data due to it being used in another thread.
  // We'll instead set this to false which will stop it from running ESP until it gets it again.
  // The reason why we do this is hopefully get rid of the ESP being at an old position when they respawn.
  data->has_transform = false;
  return true;
}

bool utils::reset_lag_comp_data(i32 index){
  index--;

  if(index < 0 || index >= LC_MAX_ENTRIES)
    return false;

  assert(lag_compensation->lc_table != nullptr);
  memset(&lag_compensation->lc_table[index], 0, sizeof(s_lc_data));
  return true;
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

void utils::reset_all_lag_comp_data(){
  assert(lag_compensation->lc_table != nullptr && "Did you forget to call lag_compensation->setup ?");
  memset(lag_compensation->lc_table, 0, sizeof(s_lc_data) * LC_MAX_ENTRIES);

  DBG("[!] utils::reset_all_lag_comp_data\n");
}

void utils::reset_all_player_data(){
  memset(&global->player_data, 0, sizeof(global->player_data));

  DBG("[!] utils::reset_all_player_data\n");
}

c_aimbot_settings* utils::get_aimbot_settings(){
  c_base_player* localplayer = utils::localplayer();

  if(localplayer == nullptr)
    return nullptr;

  if(localplayer->player_class() <= 0 || localplayer->player_class() > TF_CLASS_MAX)
    return nullptr;

  // types
  // 0 = hitscan
  // 1 = projectile
  // 2 = medigun
  // 3 = melee
  i32 wep_type = AIMBOT_WEAPON_SETTING_HITSCAN;
  c_base_weapon* wep = localplayer->get_weapon();
  if(wep != nullptr){
    if(wep->is_melee())
      wep_type = AIMBOT_WEAPON_SETTING_MELEE;
    else if(wep->is_projectile_weapon() || wep->is_wrangler())
      wep_type = AIMBOT_WEAPON_SETTING_PROJ;
    else if(wep->is_medigun())
      wep_type = AIMBOT_WEAPON_SETTING_MEDIGUN;
  }

  return &config->aimbot[math::clamp(localplayer->player_class() - 1, 0, TF_CLASS_MAX - 1)][wep_type];
}

c_player_esp_settings* utils::get_player_esp_settings(i32 type){
  if(type < 0 || type > 2)
    return nullptr;

  return &config->player_esp[type];
}

c_player_esp_settings* utils::get_player_esp_settings(c_base_player* player){
  if(player == nullptr)
    return nullptr;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return nullptr;

  i32 type = 0;
  if(player->is_localplayer() || player->is_steam_friend())
    type = PLAYER_ESP_SETTING_TYPE_FRIEND;
  else
    type = (player->team() == localplayer->team()) ? PLAYER_ESP_SETTING_TYPE_BLU_TEAM : PLAYER_ESP_SETTING_TYPE_RED_TEAM;

  return utils::get_player_esp_settings(type);
}

c_world_esp_settings* utils::get_world_esp_settings(i32 type){
  if(type < 0 || type > 2)
    return nullptr;

  return &config->world_esp[type];
}

c_world_esp_settings* utils::get_world_esp_settings(c_base_entity* entity){
  if(entity == nullptr)
    return nullptr;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return nullptr;

  i32 type = (entity->team() == localplayer->team()) ? WORLD_ESP_SETTING_TYPE_BLU_TEAM : WORLD_ESP_SETTING_TYPE_RED_TEAM;
  // The game will make the ammo/health packs inherit the team it was "dropped" from. Hence the checks
  if(entity->is_halloween_gift_pickup() || entity->is_krampus() || entity->is_halloween_pickup() || entity->is_ammo_pack() || entity->is_health_pack() || entity->is_money() || entity->is_lunchable() || entity->is_powerup() || entity->get_spellbook_type() == 1 || entity->get_spellbook_type() == 2 || (entity->is_boss() || entity->is_bomb()) && (entity->team() < 2 || entity->team() > 3) || entity->is_tank_boss())
    type =  WORLD_ESP_SETTING_TYPE_UNASSIGNED_TEAM;

  return utils::get_world_esp_settings(type);
}

c_player_chams_settings* utils::get_player_chams_settings(i32 type){
  if(type < 0 || type > 6)
    return nullptr;

  return &config->player_chams[type];
}

c_player_chams_settings* utils::get_player_chams_settings(c_base_player* player){
  if(player == nullptr)
    return nullptr;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return nullptr;

  i32 type = 0;
  if(player->is_steam_friend() || player->is_localplayer())
    type = PLAYER_ESP_SETTING_TYPE_FRIEND;
  else
    type = (player->team() == localplayer->team()) ? PLAYER_ESP_SETTING_TYPE_BLU_TEAM : PLAYER_ESP_SETTING_TYPE_RED_TEAM;

  return utils::get_player_chams_settings(type);
}

c_world_chams_settings* utils::get_world_chams_settings(i32 type){
  if(type < 0 || type > 2)
    return nullptr;

  return &config->world_chams[type];
}

c_world_chams_settings* utils::get_world_chams_settings(c_base_entity* entity){
  if(entity == nullptr)
    return nullptr;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return nullptr;

  i32 type = (entity->team() == localplayer->team()) ? WORLD_ESP_SETTING_TYPE_BLU_TEAM : WORLD_ESP_SETTING_TYPE_RED_TEAM;
  // The game will make the ammo/health packs inherit the team it was "dropped" from. Hence the checks
  if(entity->is_ammo_pack() || entity->is_health_pack() || entity->is_krampus() || entity->is_money() || entity->is_lunchable() || entity->is_powerup() || entity->get_spellbook_type() == 1 || entity->get_spellbook_type() == 2 || (entity->is_boss() || entity->is_bomb()) && (entity->team() < 2 || entity->team() > 3)  || entity->is_tank_boss())
    type =  WORLD_ESP_SETTING_TYPE_UNASSIGNED_TEAM;

  return utils::get_world_chams_settings(type);
}

c_player_resource* utils::get_player_resource(){
  static i32 index = -1;

  if(!utils::is_in_game()){
    index = -1;
    return nullptr;
  }

  if(index != -1){
    c_base_entity* entity = global->entity_list->get_entity(index);
    if(entity == nullptr || entity->get_client_class()->id != CTFPlayerResource){
      DBG("[!] utils::get_player_resource cached index is no longer valid, getting new one.\n");
      index = -1; // Set to -1 and then try and find new player resource pointer.
    }
    else
      return (c_player_resource*)entity;
  }

  for(u32 i = 1; i < global->entity_list->get_highest_index(); i++){
    c_base_entity* entity = global->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    s_client_class* cc = entity->get_client_class();
    if(cc == nullptr)
      continue;

    if(cc->id != CTFPlayerResource)
      continue;

    DBG("[!] utils::get_player_resource found entity at %i\n", i);
    index = i;
    return (c_player_resource*)entity;
  }

  return nullptr;
}

std::wstring utils::get_player_class_name(u8 class_id){
  switch(class_id){
    case TF_CLASS_SCOUT:
      return WXOR(L"Scout");
    case TF_CLASS_SNIPER:
      return WXOR(L"Sniper");
    case TF_CLASS_SOLDIER:
      return WXOR(L"Soldier");
    case TF_CLASS_DEMOMAN:
      return WXOR(L"Demoman");
    case TF_CLASS_MEDIC:
      return WXOR(L"Medic");
    case TF_CLASS_HEAVY:
      return WXOR(L"Heavy");
    case TF_CLASS_PYRO:
      return WXOR(L"Pyro");
    case TF_CLASS_SPY:
      return WXOR(L"Spy");
    case TF_CLASS_ENGINEER:
      return WXOR(L"Engineer");
    default:
      break;
  };

  return WXOR(L"UNK");
}

vec3 utils::trace_line(vec3 start, vec3 end, u32 mask, u32 custom_trace_filter){
  s_trace trace = global->trace->ray(start, end, mask != 0 ? mask : mask_bullet, TRACE_EVERYTHING, nullptr, custom_trace_filter);
  return trace.end;
}

float utils::attrib_hook_float(float a1, std::string str, void* a3, void* a4, bool a5){
  assert(global->attrib_hook_value_float != nullptr);

  a5 = false;
  return utils::call_fastcall64_raw<float, float, u8*, void*, void*, bool>(global->attrib_hook_value_float, a1, str.c_str(), a3, a4, a5);
}

i32 utils::attrib_hook_int(i32 a1, std::string str, void* a3, void* a4, bool a5){
  assert(global->attrib_hook_value_int != nullptr);

  a5 = false;
  return utils::call_fastcall64_raw<i32, i32, u8*, void*, void*, bool>(global->attrib_hook_value_int, a1, str.c_str(), a3, a4, a5);
}

float utils::get_weapon_hit_chance(c_base_entity* target, vec3 aim_angle){
  if(target == nullptr)
    return 0.f;

  c_base_player* localplayer = utils::localplayer();

  if(localplayer == nullptr)
    return 0.f;

  c_base_weapon* wep = localplayer->get_weapon();

  if(wep == nullptr)
    return 0.f;

  ctf_weapon_info* wep_info = wep->get_weapon_info();

  if(wep_info == nullptr)
    return 0.f;

  if(wep->is_perfect_shot() || wep->get_slot() == 2 || wep->is_sniper_rifle() || wep->is_medigun())
    return 1.f;

  if(double_tap->is_shifting)
    return 1.f;

  //if(double_tap->is_shifting || double_tap->shift_end + math::time_to_ticks(0.5f) >= global->original_cmd.command_number)
    //return 1.f;

  vec3  shoot_pos = localplayer->shoot_pos();
  float spread    = wep->get_spread();

  vec3 fwd, right, up;
  math::angle_2_vector(aim_angle, &fwd, &right, &up);

  i32 hit = 0;
  for(u32 i = 0; i < (INT_MAX-1); i += ((INT_MAX-1) / 256)){
    math::set_random_seed(i);

    float x = math::random_float(-0.5f, 0.5f) + math::random_float(-0.5f, 0.5f);
    float y = math::random_float(-0.5f, 0.5f) + math::random_float(-0.5f, 0.5f);

    vec3 calc_spread = fwd + right * (spread * x) + up * (spread * y);

    s_trace trace = global->trace->ray_to_entity(shoot_pos, shoot_pos + (calc_spread * wep_info->range), mask_bullet, target);

    if(trace.hit_entity == nullptr || target == nullptr)
      continue;

    if(trace.hit_entity == target || trace.hit_entity->team() == target->team())
      hit++;
  }

  return hit == 0 ? 0.f : math::clamp((float)hit / 256.f, 0.f, 1.f);
}

void utils::create_tf2_marker(i32 owner_index, vec3 pos, std::string text, float life_time){
  if(life_time < 0.f || pos == vec3(0, 0, 0))
    return;

  c_game_event* event = global->game_manager->create_event(XOR("show_annotation"));
  if(event != nullptr){
    event->set_float(XOR("worldPosX"), pos.x);
    event->set_float(XOR("worldPosY"), pos.y);
    event->set_float(XOR("worldPosZ"), pos.z);
    event->set_float(XOR("lifetime"), life_time);
    event->set_int(XOR("id"), owner_index);

    event->set_string(XOR("text"), text.c_str());
    event->set_string(XOR("play_sound"), XOR("ui/system_message_alert.wav"));

    event->set_bool(XOR("show_distance"), true);
    event->set_bool(XOR("show_effect"), true);
    global->game_manager->fire_event_client_side(event);
  }
}

void utils::check_to_reset_data(i32 index){
  if(index <= 0)
    return;

  c_player_data* data = utils::get_player_data(index);
  if(data == nullptr)
    return;

  c_player_resource* pr = utils::get_player_resource();
  if(pr == nullptr)
    return;

  i32 id = pr->get_account_id(index);
  if(id != data->friends_id){
    DBG("[!] friends id mismatch on ID Stored: %i - ID: %i\n", data->friends_id, id);
    utils::reset_player_data(index);
    data->friends_id = id;
  }

}

void utils::check_for_party_member(i32 index){
  if(index <= 0)
    return;

  c_player_data* data = utils::get_player_data(index);
  if(data == nullptr)
    return;

  c_player_resource* pr = utils::get_player_resource();
  if(pr == nullptr)
    return;

  u32 id = pr->get_account_id(index);
  if(!data->friends_id || id < 100)
    return;

  float time = math::time();
  if(data->next_party_member_check > time)
    return;

  data->party_member = false;
  ctf_party* party = get_party();
  if(party == nullptr)
    return;

  for(i32 i = 0; i <= party->member_count; i++){
    u32 memberid = party->get_member_id(i);
    if(memberid < 100) // We'll be unable to detect people who work at valve from being in our party. -Rud
      continue;

    if(memberid == id){
      data->party_member = true;
      break;
    }
  }

  data->next_party_member_check = time + 5.f;
}

void utils::verify_player_data(){
  if(global->verified_player_data)
    return;

  DBG("[!] Verifying player data...\n");
  global->verified_player_data = true;
  for(i32 i = 1; i <= globaldata->max_clients; i++)
    utils::check_to_reset_data(i);
}

void utils::verify_entity_data(){
  if(global->verified_entity_data)
    return;

  global->verified_entity_data = true;
  for(i32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* entity = global->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    create_entity_data(entity);
  }
}

bool utils::create_entity_data(c_base_entity* entity){
  if(entity == nullptr)
    return false;

  i32 index = entity->get_index();
  if(index == -1)
    return false;

  assert(index < 2048);
  s_client_class* cc = entity->get_client_class();
  if(cc == nullptr)
    return false;

  // Since the model isn't setup yet we can't use this function to detect these types entities: ammopacks, healthpacks, lunchables, etc...
  // So we'll assume that CBaseAnimating is an important entity we care about it.
  // There will be no issues with world esp or any features as such.
  // However, if you're crashing from this feature somehow. Then make sure the code is sanity checked.
  // With entities we care about. Not here, in your feature code or whatever.
  if(cc->id != CBaseAnimating && !entity->is_arrow(true) && world_esp->get_object_index(entity) == OBJECT_TYPE_INVALID){
    //DBG("[!] %s (%i) not a valid entity we care about\n", cc->name, index);
    return false;
  }

  global->memory_manager.create_data(index, sizeof(c_entity_data));
  //DBG("[+] %s (%i) [%i] was added\n", cc->name, index, global->memory_manager.get_allocated_bytes());
  return true;
}

float utils::get_command_latency(){
  assert(global->client_state != nullptr);
  float latency = utils::get_latency();
  if(global->client_state->choked_commands() > 0)
    latency += math::ticks_to_time(global->client_state->choked_commands());

  return latency;
}

float utils::get_latency(){
  return global->engine->get_latency(false, FL_RAW_LATENCY) + global->engine->get_latency(true, FL_RAW_LATENCY);
}

void utils::update_lookat_trace(){
  if(global->speedhack_tick)
    return;

   c_base_player* localplayer  = utils::localplayer();

   if(localplayer == nullptr)
    return;

  if(global->last_update_lookat_tick != globaldata->tick_count){
    vec3 fwd;
    math::angle_2_vector(global->untouched_cmd.view_angles, &fwd, nullptr, nullptr);
    vec3 pos = (fwd * 8912.f) + localplayer->shoot_pos();

    global->lookat_target       = global->trace->ray(localplayer->shoot_pos(), pos, mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_HIT_TEAM);
    if(global->lookat_target.hit_entity != nullptr)
      global->lookat_target_entindex = global->lookat_target.hit_entity->get_index();
    else
      global->lookat_target_entindex = -1;

    global->last_update_lookat_tick = globaldata->tick_count;
  }
}

bool utils::is_truce_active(){
  if(global->gamerules_addr == nullptr){
    DBG("[-] gamerules addr is nullptr\n");
    return false;
  }

  c_game_rules* game_rules = *(c_game_rules**)global->gamerules_addr;
  if(game_rules == nullptr){
    DBG("[!] game rules is nullptr\n");
    return false;
  }

  cookie_block_check_return_val(true);

  return game_rules->truce_active();
}

bool utils::is_playing_mvm(){
  if(global->gamerules_addr == nullptr){
    DBG("[-] gamerules addr is nullptr\n");
    return false;
  }

  c_game_rules* game_rules = *(c_game_rules**)global->gamerules_addr;
  if(game_rules == nullptr){
    DBG("[!] game rules is nullptr\n");
    return false;
  }

  return game_rules->playing_mvm();
}

bool utils::is_in_setup(){
  if(global->gamerules_addr == nullptr){
    DBG("[-] gamerules addr is nullptr\n");
    return false;
  }

  c_game_rules* game_rules = *(c_game_rules**)global->gamerules_addr;
  if(game_rules == nullptr){
    DBG("[!] game rules is nullptr\n");
    return false;
  }

  return game_rules->in_setup();
}

bool utils::is_player_ready(i32 index){
  if(global->gamerules_addr == nullptr){
    DBG("[-] gamerules addr is nullptr\n");
    return;
  }

  c_game_rules* game_rules = *(c_game_rules**)global->gamerules_addr;
  if(game_rules == nullptr){
    DBG("[!] game rules is nullptr\n");
    return;
  }

  bool* is_ready_ptr = game_rules->is_player_ready();
  assert(index > 0);
  return is_ready_ptr[index];
}

void utils::set_playing_mvm(bool state){
  if(global->gamerules_addr == nullptr){
    DBG("[-] gamerules addr is nullptr\n");
    return;
  }

  c_game_rules* game_rules = *(c_game_rules**)global->gamerules_addr;
  if(game_rules == nullptr){
    DBG("[!] game rules is nullptr\n");
    return;
  }

  game_rules->playing_mvm() = state;
}

bool utils::is_playing_mannpower(){
  if(global->gamerules_addr == nullptr){
    DBG("[-] gamerules addr is nullptr\n");
    return false;
  }

  c_game_rules* game_rules = *(c_game_rules**)global->gamerules_addr;
  if(game_rules == nullptr){
    DBG("[!] game rules is nullptr\n");
    return false;
  }

  return game_rules->playing_mannpower();
}

ctf_party* utils::get_party(){
  assert(global->shared_object_cache_addr != nullptr);
  assert(global->find_base_type_cache_addr != nullptr);

  void* shared_object = *(void**)global->shared_object_cache_addr;
  if(shared_object == nullptr)
    return nullptr;
  
  void* cache = utils::call_fastcall64<void*, i64>(global->find_base_type_cache_addr, shared_object, 2003);
  if(cache == nullptr)
    return nullptr;
  
  i32 count = *(i32*)(cache + 0x28);
  if(count <= 0)
    return nullptr;
  
  ctf_party* addr = *(ctf_party**)(*(uptr*)(cache + 0x8) + 0x8 * (u32)(count - 1));
  return addr;
}

bool utils::medic_mode(c_base_entity* entity, bool& out){
  if(entity == nullptr)
    return false;

  if(entity->is_dormant())
    return false;

  if(entity->get_index() == global->engine->get_local_player())
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(localplayer->team() != entity->team())
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(!(weapon->is_medigun() || weapon->is_crossbow() || weapon->weapon_id() == WPN_Amputator))
    return false;

  if(entity->is_player()){
    c_base_player* p = (c_base_player*)entity;
    if(!p->is_valid())
      return false;

    if(p->is_cloaked())
      return false;

    if(p->is_steam_friend())
      return false;

    out = (p->health() < p->max_health());
    return true;
  }
  else if(entity->is_revive_marker()){
    out = (entity->revive_marker_health() < entity->revive_marker_max_health());
    return true;
  }

  out = false;
  return false;
}

bool utils::engineer_mode(c_base_entity* entity, bool& out){
  if(entity == nullptr)
    return false;

  if(!(entity->is_sentry() || entity->is_dispenser() || entity->is_teleporter()))
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(localplayer->team() != entity->team())
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(!weapon->can_weapon_repair_objects(entity->sapped()))
    return false;

  out = (entity->object_health() < entity->object_max_health()) && !entity->placing() && !entity->building() && !entity->disposable_building() || entity->sapped();
  return true;
}

bool utils::wait_lag_fix(c_base_player* player){
  if(!global->aimbot_settings->position_adjust_accurate_unlag || player == nullptr || double_tap->is_shift_tick())
    return false;

  s_lc_data* lc_data = lag_compensation->get_data(player);
  if(lc_data == nullptr)
    return false;

  // If not valid then get the latest tick.
  s_lc_record* record  = lc_data->get_record(0);
  s_lc_record* record2 = lc_data->get_record(1);
  if(record == nullptr || record2 == nullptr)
    return false;

  // Hopefully you can understand my concern with this.
  i32 latency_ticks = math::time_to_ticks(global->engine->get_latency(true));

  // With extreme latency this will prevent us from shooting in most cases.
  if(latency_ticks >= math::time_to_ticks(0.2f) && !record2->broken){
    DBG("[!!] %s record2 not broken (high latency unlag mode) active\n", player->info().name);
    return false;
  }

  float len = player->velocity().length_2d();

  if(len < 0.1f)
    return false;

  i32 ticks_required_to_break_lc = math::clamp((i32)(((64.f / globaldata->interval_per_tick) / len) + 0.5f), 0, math::time_to_ticks(1.f));
  i32 update_delta               = math::clamp(global->client_state->server_tick() - record->server_tick, 0, math::time_to_ticks(1.f)) + latency_ticks;

  return update_delta > ticks_required_to_break_lc;
}

float utils::get_damage(c_base_player* player, vec3 attacker_pos, vec3 victim_pos){
  if(player == nullptr)
    return 0.f;

  if(!player->is_valid())
    return 0.f;

  c_base_weapon* weapon = player->get_weapon();
  if(weapon == nullptr)
    return 0.f;

  ctf_weapon_info* wep_info = weapon->get_weapon_info();
  if(wep_info == nullptr)
    return 0.f;

  // How can we do this?
  /*
    bool bDoShortRangeDistanceIncrease = !bCrit || info.GetCritType() == CTakeDamageInfo::CRIT_MINI ;
    bool bDoLongRangeDistanceDecrease = !bIgnoreLongRangeDmgEffects && ( bForceCritFalloff || ( !bCrit && info.GetCritType() != CTakeDamageInfo::CRIT_MINI  ) );
  */

  // Apply penalties or bonuses to the damage.
  float damage = wep_info->damage;
  damage       = utils::attrib_hook_float(damage, XOR("mult_dmg"), weapon, 0, 1);
  if(player->is_disguised())
    damage = utils::attrib_hook_float(damage, XOR("mult_dmg_disguised"), weapon, 0, 1);

  float random_damage        = damage * 0.5f; // tf_damage_range.
  float random_damage_spread = 0.10f;

  float min_dmg = (0.5f - random_damage_spread);
  float max_dmg = (0.5f + random_damage_spread);

  float dist   = math::biggest(1.0f, (victim_pos - attacker_pos).length());
  float center = math::remap_val_clamped(dist / 512.f, 0.0f, 2.0f, 1.0f, 0.0f);
  //if(center > 0.5f && bDoShortRangeDistanceIncrease || center <= 0.5f)
  {
    min_dmg = math::biggest(0.0f, center - random_damage_spread);
    max_dmg = math::smallest(1.0f, center + random_damage_spread);
  }

  // tf_damage_disablespread is enabled by default which does this min_dmg + random_damage_spread otherwise it's random.
  float random_range_val = min_dmg + random_damage_spread;

  switch(weapon->get_id()){
    default: break;
    case TF_WEAPON_SCATTERGUN:
    case TF_WEAPON_SODA_POPPER:
    case TF_WEAPON_PEP_BRAWLER_BLASTER:
    {
      if(random_range_val > 0.5f)
        random_damage *= 1.5f;

      break;
    }
    case TF_WEAPON_ROCKETLAUNCHER:
    case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
    case TF_WEAPON_PARTICLE_CANNON:
    {
      if(random_range_val > 0.5f)
        random_damage *= 0.5f;

      break;
    }
    case TF_WEAPON_PIPEBOMBLAUNCHER:
    case TF_WEAPON_GRENADELAUNCHER:
    case TF_WEAPON_CANNON:
    case TF_WEAPON_STICKBOMB:
    {
      if(random_range_val > 0.5f)
        random_damage *= 0.2f;

      break;
    }
  }

  float dmg_variance = math::simple_spline_remap_val_clamped(random_range_val, 0, 1, -random_damage, random_damage);
  //if ( ( bDoShortRangeDistanceIncrease && flDmgVariance > 0.f ) || bDoLongRangeDistanceDecrease )
  damage = damage + dmg_variance;

  return math::round(damage);
}

float utils::get_heal_bolt_amount(c_base_player* player, vec3 attacker_pos, vec3 victim_pos, c_base_player* victim){
  if(player == nullptr)
    return 0.f;

  if(!player->is_valid())
    return 0.f;

  if(!player->is_player_class(TF_CLASS_MEDIC))
    return 0.f;

  c_base_weapon* crossbow = player->get_weapon_from_belt(0, false);
  if(crossbow == nullptr)
    return 0.f;
  
  if(!crossbow->is_crossbow())
    return 0.f;
  
  ctf_weapon_info* wep_info = crossbow->get_weapon_info();
  if(wep_info == nullptr)
    return 0.f;
  
  if(player->has_condition(TF_COND_NOHEALINGDAMAGEBUFF))
    return 0.f;
     
  float damage          = wep_info->damage;
  float travel_time     = math::ticks_to_time(math::time_to_ticks(attacker_pos.distance(victim_pos) / 2400.f));
  float life_time_scale = math::remap_val_clamped(travel_time, 0.0f, 0.6f, 0.5f, 1.f);
  damage *= life_time_scale;
  damage *= 2.f;

  float health = damage;
  if(victim != nullptr){
    health = utils::attrib_hook_float(health, XOR("mult_healing_from_medics"), victim, 0, 1);

    c_base_weapon* weapon = victim->get_weapon();
    if(weapon != nullptr){
      float healing_bonus = utils::attrib_hook_float(1.f, XOR("mult_healing_received"), weapon, 0, 1);
      health *= healing_bonus;
    }
  }

  //float value = math::floor(health) - 10.f;

  return math::floor(health);
}

void utils::calculate_sniper_damage(u32 stage){
  if(!utils::is_in_game())
    return;

  if(stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
    return;

  // Reset sniper damage.
  for(i32 i = 1; i <= globaldata->max_clients; i++){
    c_base_player* player = global->entity_list->get_entity(i);
    if(player == nullptr)
      continue;

    c_player_data* data = player->data();
    if(data == nullptr)
      continue;

    if(!player->is_valid() || !player->has_condition(TF_COND_AIMING))
      data->sniper_charge_damage = 0.f;
  }

  for(i32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* entity = global->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    if(entity->is_dormant())
      continue;

    if(entity->get_client_class()->id != CSniperDot)
      continue;

    c_base_entity* owner = entity->get_owner();
    if(owner == nullptr)
      continue;

    if(!owner->is_player())
      continue;

    c_base_player* player = (c_base_player*)owner;
    if(!player->is_valid())
      continue;

    c_player_data* data = player->data();
    if(data == nullptr)
      continue;

    float time   = math::biggest(owner->simulation_time() - entity->dot_charge_start_time(), 0.f);
    data->sniper_charge_damage = math::clamp(time * 50.f, 0.f, 150.f);
  }
}

void utils::draw_hitbox(i32 index, colour clr, float duration){
  c_base_player* player = global->entity_list->get_entity(index);
  if(player == nullptr)
    return;

  if(!player->is_valid())
    return;

  if(duration < 0.0f)
    return;

  s_lc_data* lc = lag_compensation->get_data(player);
  if(lc == nullptr)
    return;

  s_lc_record* record = lc->get_record(0);
  if(record == nullptr)
    return;

  draw_hitbox_custom_matrix(index, record->bone_matrix, clr, duration);
}

void utils::draw_server_hitbox(i32 index, float duration = 4.0f){
#if defined(DEV_MODE)
  //c_base_player* player = global->entity_list->get_entity(index);
  //if(player == nullptr)
  //  return;
//
  //if(!player->is_valid())
  //  return;
//
  //void* server_animating = utils::call_cdecl<void*, i32>(global->get_server_animating_addr, index);
  //if(server_animating != nullptr)
  //  utils::call_thiscall<void, float, bool>(global->draw_server_hitbox_addr, server_animating, duration, true);
#endif
}

void utils::draw_bounding_box(i32 index, colour clr, float duration){
  c_base_entity* entity = global->entity_list->get_entity(index);
  if(entity == nullptr)
    return;

  vec3 mins = entity->obb_mins();
  vec3 maxs = entity->obb_maxs();
  render_debug->draw_3dbox(entity->origin(), mins, maxs, vec3(0.f, entity->get_abs_angles().y, 0.f), clr, colour(clr.x, clr.y, clr.z, 255), duration);
}

// Creates a custom bone matrix with its animations recomputed to the destination angle
// NOTE: It should be noted that the animations for the specific player will perminately change to whats done in here
// Its advised to either back them up or call this function again afterwards to restore to the original angle.
bool utils::create_custom_matrix(c_base_player* player, vec3 angles, matrix3x4* bone_matrix){
  if(player == nullptr || bone_matrix == nullptr)
    return false;

  if(!player->is_valid())
    return false;

  s_animation_state* anim_state = player->get_animation_state();
  if(anim_state == nullptr)
    return false;

  // Backup the animation and pose parameter buffers
  static i8 anim_state_backup[0x180];    // CTFPlayerAnimState
  static i8 pose_parameter_backup[0x5C]; // m_flPoseParameter
  memcpy(anim_state_backup, anim_state, sizeof(anim_state_backup));
  memcpy(pose_parameter_backup, player->pose_parameters(), sizeof(pose_parameter_backup));

  // Backup and then set our v_angle to our goal angle
  vec3 old_last_viewangle  = player->last_viewangle();
  player->last_viewangle() = angles;

  bool result = player->setup_bones(bone_matrix, true, true);

  // Restore the animation and pose parameter buffers
  memcpy(anim_state, anim_state_backup, sizeof(anim_state_backup));
  memcpy(player->pose_parameters(), pose_parameter_backup, sizeof(pose_parameter_backup));

  // Restore our v_angle to the original angle
  player->last_viewangle() = old_last_viewangle;

  return result;
}

bool utils::dispatch_particle_effect(i8* particle_name, vec3 origin, vec3 angle){
  if(particle_name == nullptr)
    return false;

  utils::call_fastcall64_raw<void, i8*, vec3, vec3, c_base_entity*>(global->dispatch_particle_effect, particle_name, origin, angle, nullptr);

  return true;
}

void utils::update_player_information(u32 stage){
  if(stage != FRAME_NET_UPDATE_START && stage != FRAME_NET_UPDATE_END)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;
  assert(global->is_crit_boosted != nullptr);
  assert(global->get_max_buffed_health_addr != nullptr);
    
  for(u32 index = 1; index <= globaldata->max_clients; index++){
    c_base_player* player = global->entity_list->get_entity(index);
    if(player == nullptr)
      continue;

    c_player_data* data = player->data();
    if(data == nullptr)
      continue;

    s_shared* shared = player->shared();
    if(shared == nullptr){
      DBG("[-] s_shared was nullptr for %s in update_player_information\n", player->info().name);
      memset(&data->tfdata, 0, XOR32(sizeof(s_tf2_data)));
      continue;
    }

    for(i32 i = 0; i < XOR32(TF_COND_COUNT); i++)
      data->tfdata.condition_active[i] = utils::call_fastcall64<bool, u32>(gen_internal->decrypt_asset(global->ctfplayershared_incondition_trp), shared, i);

    c_base_weapon* wep = player->get_weapon(); // Do a nullcheck when you wanna read or write memory.

    data->tfdata.is_crit_boosted     = utils::call_fastcall64<bool>(global->is_crit_boosted, shared);
    data->tfdata.is_dueling          = player->is_dueling(false);
    data->tfdata.heal_bolt_amount    = (i32)utils::get_heal_bolt_amount(localplayer, localplayer->shoot_pos(), player->obb_center(), player);

    if(player->team() != global->localplayer_team)
      data->tfdata.rta_target_flags = raytrace_aimbot->get_target_flags(player);
    else
      data->tfdata.rta_target_flags = 0;
    

    // This crashed inside the function it calls (C_TFPlayer::GetMaxHealthForBuffing) - 8.05.2023 (rud)
    // get_max_buffed_health: Is a thiscall, has 2 boolean parameters.
    // Either the player resource inside C_TFPlayer::GetMaxHealthForBuffing is nullptr for some reason or the outer poiner of m_shared is invalid for some reason.
    data->tfdata.max_buffed_health   = utils::call_fastcall64<i32, bool, bool>(global->get_max_buffed_health_addr, shared, false, true);

    data->tfdata.weapon_index         = -1;
    data->tfdata.use_rapid_fire_crits = false;
    if(wep != nullptr){
      ctf_weapon_info* wep_info = wep->get_weapon_info();
      if(wep_info != nullptr)
        data->tfdata.use_rapid_fire_crits = wep_info->use_rapid_fire_crits;

      formatW(data->tfdata.weapon_name, WXOR(L"%ls"), wep->get_weapon_name().c_str());
      data->tfdata.weapon_index = wep->get_index();
    }

   
    vec3 pos      = player->origin();
    s_trace trace = global->trace->ray(pos, vec3(pos.x, pos.y, pos.z - 2.f), mask_bullet, player);

    data->tfdata.ground_entity_index = trace.hit_entity != nullptr ? trace.hit_entity->get_index() : -1;
  }
}

bool utils::switch_to_weapon(c_base_weapon* weapon){
  if(weapon == nullptr)
    return false;

  c_base_player* owner = weapon->get_owner();
  if(owner == nullptr){
    DBG("[-] switch_to_weapon: weapon has no owner\n");
    return false;
  }

  if(owner->get_index() != global->localplayer_index){
    DBG("[-] switch_to_weapon: weapon owner didn't belong to the localplayer entity index: %i\n", owner->get_index());
    return false;
  }

  c_base_weapon* cur = owner->get_weapon();
  if(cur == nullptr){
    DBG("[-] switch_to_weapon: cur held weapon was nullptr.\n");
    return false;
  }

  if(cur->get_index() == weapon->get_index())
    return true;

  // Make sure we predict this weapon change?
  global->current_cmd->weapon_select  = weapon->get_index();
  global->current_cmd->weapon_subtype = 0;

  packet_manager->force_send_packet(false);
  packet_manager->force_send_packet(true);
  return false;
}

bool utils::switch_to_weapon_from_slot(i32 slot){
  if(slot < 0 || slot > 6){
    DBG("[-] switch_to_weapon_from_slot: received invalid slot %i\n", slot);
    return false;
  }

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* wep = localplayer->get_weapon_from_belt(slot, false);
  if(wep == nullptr){
    DBG("[-] switch_to_weapon_from_slot: slot %i doesn't have a valid weapon\n");
    return false;
  }

  return switch_to_weapon(wep);
}

bool utils::choose_class(u8 i){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(localplayer->team() == 0 || localplayer->team() > 3)
    return;

  if(i < 1 || i > 9)
    return false;

  i8 buf[64];
  formatA(buf, XOR("join_class %ls"), i == TF_CLASS_HEAVY ? WXOR(L"heavyweapons") : get_player_class_name(i).c_str());
  global->engine->client_cmd(buf);

  // Hide the class selection menu and also fix the invisible health hud.
  show_panel(localplayer->team() == 2 ? XOR("class_red") : XOR("class_blue"), false);

  return true;
}

#define SHOOT_POS_HEIGHT 16.f
bool utils::get_headshot_threats(s_active_headshot_threat* threat_data){
  if(threat_data == nullptr)
    return false;

  memset(threat_data, 0, sizeof(s_active_headshot_threat));
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return;

  vec3 shoot_pos[] = {
    localplayer->shoot_pos(),
    localplayer->shoot_pos() + vec3(0.f, 0.f, SHOOT_POS_HEIGHT),
    utils::trace_line(localplayer->shoot_pos(), math::vec_point_from_angle_len(localplayer->shoot_pos(), vec3(0.f, global->untouched_cmd.view_angles.y + 90.f, 0.f), 64.f), mask_bullet, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS),
    utils::trace_line(localplayer->shoot_pos(), math::vec_point_from_angle_len(localplayer->shoot_pos(), vec3(0.f, global->untouched_cmd.view_angles.y - 90.f, 0.f), 64.f), mask_bullet, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS),
    utils::trace_line(localplayer->shoot_pos(), math::vec_point_from_angle_len(localplayer->shoot_pos(), vec3(0.f, global->untouched_cmd.view_angles.y + 180.f, 0.f), 64.f), mask_bullet, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS),
    utils::trace_line(localplayer->shoot_pos(), math::vec_point_from_angle_len(localplayer->shoot_pos(), vec3(0.f, global->untouched_cmd.view_angles.y, 0.f), 64.f), mask_bullet, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS),

    // So in the event we're peeking above a ramp or somebody else it'll turn on in time.
    utils::trace_line(localplayer->shoot_pos() + vec3(0.f, 0.f, SHOOT_POS_HEIGHT), math::vec_point_from_angle_len(localplayer->shoot_pos(), vec3(0.f, global->untouched_cmd.view_angles.y + 90.f, 0.f), 64.f), mask_bullet, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS),
    utils::trace_line(localplayer->shoot_pos() + vec3(0.f, 0.f, SHOOT_POS_HEIGHT), math::vec_point_from_angle_len(localplayer->shoot_pos(), vec3(0.f, global->untouched_cmd.view_angles.y - 90.f, 0.f), 64.f), mask_bullet, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS),
    utils::trace_line(localplayer->shoot_pos() + vec3(0.f, 0.f, SHOOT_POS_HEIGHT), math::vec_point_from_angle_len(localplayer->shoot_pos(), vec3(0.f, global->untouched_cmd.view_angles.y + 180.f, 0.f), 64.f), mask_bullet, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS),
    utils::trace_line(localplayer->shoot_pos() + vec3(0.f, 0.f, SHOOT_POS_HEIGHT), math::vec_point_from_angle_len(localplayer->shoot_pos(), vec3(0.f, global->untouched_cmd.view_angles.y, 0.f), 64.f), mask_bullet, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS)
  };

  for(i32 i = 1; i <= globaldata->max_clients; i++){
    c_base_player* player = global->entity_list->get_entity(i);
    if(player == nullptr)
      continue;

    if(!player->is_alive())
      continue;

    if(player->is_dormant())
      continue;

    if(player->is_taunting() || player->is_bonked())
      continue;

    if(!localplayer->is_considered_enemy(player))
      continue;

    c_player_data* data = player->data();
    if(data == nullptr)
      continue;

    bool is_cheater = cheat_detection->is_cheating(i);

    c_base_weapon* wep = player->get_weapon();
    if(wep == nullptr)
      continue;

    if(!(wep->is_sniper_rifle() || player->can_spy_headshot()))
      continue;

    {
      bool seen_point = false;
      float dist = localplayer->origin().distance(player->origin());
      if(dist > 400.f){ // Do the vischeck when they aren't this close to us.
        for(i32 j = 0; j < sizeof(shoot_pos) / sizeof(vec3); j++){
          if(seen_point)
            break;

          float ang = -180.f;
          s_trace tr = global->trace->ray(shoot_pos[j], player->shoot_pos(), mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_ANTI_AIM);
          if(!tr.vis(player)){ // Okay we can't directly see their eye position.

            // Can we see the center of their bounding box?
            tr = global->trace->ray(shoot_pos[j], player->obb_center(), mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_ANTI_AIM);
            if(!tr.vis(player)){ // We cannot see the center of the bounding box.

              // Lets go ahead and scan 64 units around the player.
              while(ang < 180.f){
                vec3 point = utils::trace_line(player->shoot_pos(), math::vec_point_from_angle_len(player->shoot_pos(), vec3(0.f, ang, 0.f), 64.f), mask_bullet, TR_CUSTOM_FILTER_ANTI_AIM);
                tr = global->trace->ray(shoot_pos[j], point, mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_ANTI_AIM);

                // Can we see this point? Break out of the angle loop if we do.
                if(tr.vis(player)){
                  seen_point = true;
                  break;
                }

                ang += 45.f;
              }
            }
            else
              seen_point = true;
          }
          else
            seen_point = true;
        }
      }

      if(seen_point || dist <= 400.f){
        threat_data->vis_threats[0]++;
        if(is_cheater)
          threat_data->vis_threats[1]++;
      }
    }

    threat_data->threats[0]++;
    if(is_cheater)
      threat_data->threats[1]++;
  }

  return true;
}

void utils::update_active_threats(){
  static i32 last_tickcount = 0;
  if(last_tickcount == globaldata->tick_count)
    return;

  last_tickcount = globaldata->tick_count;
  utils::get_headshot_threats(&global->headshot_threats);
}

void utils::show_panel(std::string name, bool state){
  DBG("[-] utils::show_panel(%s, %i) is disabled due to not being init anymore, so fix it or remove it?\n", name.c_str(), state);
  //assert(global->viewport != nullptr);
  //if(global->viewport == nullptr){
  //  DBG("[-] utils::show_panel: viewport is nullptr\n");
  //  return;
  //}
//
  //c_viewport_panel* panel = global->viewport->find_panel_by_name(name);
  //if(panel == nullptr)
  //  return;
//
  //panel->show_panel(state);
  //DBG("[!] Panel (%s) shown: %i\n", name.c_str(), state);
}

bool utils::correct_position_for_blast(u8 projectile_type, vec3& pos, c_base_entity* ignore_entity){
  if(projectile_type == EXPLOSION_PYRO || !projectile_type || projectile_type > EXPLOSION_PYRO )
    return true;


  s_trace tr = global->trace->ray(pos, pos, mask_shot_hull, TRACE_EVERYTHING, ignore_entity, TR_CUSTOM_FILTER_NONE);

  //if(projectile_type == EXPLOSION_DEMO || projectile_type == EXPLOSION_THROWABLE){
  //  vec3 spot = pos + vec3(0.f, 0.f, 8.f);
  //  tr        = global->trace->ray(spot, spot + vec3(0.f, 0.f, -32.f), mask_shot_hull, TRACE_EVERYTHING, ignore_entity, TR_CUSTOM_FILTER_NONE); 
  //}
  
  pos = tr.end;

  if(tr.fraction != 1.f){
    pos = tr.end + (tr.plane.normal * 1.f);
    tr = global->trace->ray(pos, pos, mask_shot_hull, TRACE_EVERYTHING, ignore_entity, TR_CUSTOM_FILTER_NONE);
    if(tr.started_solid)
      return false;
  }

  /*
    -- CTFWeaponBaseGrenadeProj::Detonate
    vecSpot = GetAbsOrigin() + Vector ( 0 , 0 , 8 );
    UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -32 ), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, & tr);
  
    Explode( &tr, GetDamageType() );
  */

  /*
    -- CTFProjectile_Throwable::Explode
    vecSpot = GetAbsOrigin() + Vector ( 0 , 0 , 8 );
    UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -32 ), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, & tr);
    Explode( &tr, GetDamageType() );
  */

  return true;
}

bool utils::is_in_blast_range(c_base_entity* target, vec3 origin, vec3 pos, float radius){
  if(target == nullptr || radius <= 0.f)
    return false;

  vec3 nearest_point;
  vec3 old_abs_origin = target->get_abs_origin();
  target->set_abs_origin(origin);
  target->calc_nearest_point(pos, &nearest_point);
  target->set_abs_origin(old_abs_origin);

  return (pos - nearest_point).length_sqr() <= (radius * radius);
}

bool utils::is_in_thirdperson(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_alive())
    return false;

  if(localplayer->observer_mode() != 0 && localplayer->observer_mode() != 4)
    return false;

  if(thirdperson->is_active() || thirdperson->is_in_thirdperson_view() || wrangler_helper->is_overriding_view)
    return true;

  return false;
}

bool utils::get_health_colour(i32 health, i32 max_health, colour* hp_col){
  if(health <= 0 || max_health <= 0 || hp_col == nullptr)
    return false;

  float hp_fraction = math::clamp((float)health / (float)max_health, 0.f, 1.f);
  if(hp_fraction >= 0.75f)
    *hp_col = rgb(46, 204, 113);
  else if(hp_fraction >= 0.5f)
    *hp_col = rgb(241, 196, 15);
  else if(hp_fraction >= 0.25f)
    *hp_col = rgb(230, 126, 34);
  else
    *hp_col = rgb(231, 76, 60); 

  return true;
}

bool utils::get_radius_points(vec3 pos, float radius, vec3* list, u32* size, vec3 normal){
  if(radius <= 0.f)
    return false;
  
  vec3 normal_disp = (normal * 7.f);
  s_trace tr = global->trace->ray(pos, pos - normal_disp, mask_solid_brushonly);
  if(tr.started_solid)
    return false;

  static vec3 min = vec3(-4.f, -4.f, -4.f);
  static vec3 max = vec3(4.f, 4.f, 4.f);

  pos         = tr.end + normal_disp;
  vec3 ref    = math::abs(normal.x) < math::abs(normal.y) ? vec3(1.f, 0.f, 0.f) : vec3(0.f, 1.f, 0.f);

  vec3 u = math::cross(normal, ref);
  vec3 v = math::cross(normal, u);

  static float step     = math::deg_2_rad(1.f);
  static float max_size = math::pi() * 2.f;

  u32 i = 0;
  for(float ang = 0.f; ang <= math::pi() * 2.f; ang += step){
    
    float x = radius * math::cos(ang);
    float y = radius * math::sin(ang);

    vec3 rotated_pos  = pos + u * x + v * y;
    s_trace pos_tr    = global->trace->ray_obb(pos, rotated_pos, min, max, mask_solid_brushonly, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_SPLASH_BOT_TEST);

    list[i] = pos_tr.end;
    i++;
    if(i >= *size)
      return false;
  }

  *size = i;
  return true;
}

bool utils::is_safe_mode_enabled(){
  return global->safe_mode;
}

// We're actually not really using this at all, so might remove it. 
// But you can see why changing the cl_cmdrate affects the latency on the scoreboard.
i32 utils::calculate_score_board_ping(){

  float latency = global->engine->get_avg_latency(true) + global->engine->get_avg_latency(false);

  static c_cvar* cl_cmdrate = global->cvar->find_var(XOR("cl_cmdrate"));
  if(cl_cmdrate == nullptr)
    return 0;

  cl_cmdrate->has_min = false;
  cl_cmdrate->has_max = false;

  i32 cmd_rate = math::biggest(1, cl_cmdrate->val);
  latency -= (0.5f / cmd_rate) + math::ticks_to_time(1.0f);
  latency -= math::ticks_to_time(0.5f);

  i32 ping = latency * 1000.f;
  ping = math::clamp(ping, 5, 1000);

  return ping;
}