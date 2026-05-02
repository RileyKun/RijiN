#include "link.h"


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

  i32 type = 0;
  if(player->is_steam_friend())
    type = PLAYER_ESP_SETTING_TYPE_FRIEND;
  else
    type = (player->team() == localplayer->team()) ? PLAYER_ESP_SETTING_TYPE_AXIS_TEAM : PLAYER_ESP_SETTING_TYPE_ALLIED_TEAM;

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

  i32 type = 0;
  if(player->is_steam_friend())
    type = PLAYER_ESP_SETTING_TYPE_FRIEND;
  else
    type = (player->team() == localplayer->team()) ? PLAYER_ESP_SETTING_TYPE_AXIS_TEAM : PLAYER_ESP_SETTING_TYPE_ALLIED_TEAM;

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
  global->player_data[index].playerlist.init();
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

bool utils::wait_lag_fix(c_base_player* player){
  if(!global->aimbot_settings->position_adjust_accurate_unlag || player == nullptr /*|| double_tap->is_shift_tick()*/)
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
  i32 update_delta               = math::clamp(global->client_state->server_tick() - record1->server_tick, 0, math::time_to_ticks(1.f)) + 1 + global->engine->get_latency(true);

  return update_delta > ticks_required_to_break_lc;
}

bool utils::is_in_thirdperson(){
  return freecam->is_active() || thirdperson->is_in_thirdperson_view();
}

c_aimbot_settings* utils::get_aimbot_settings(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return nullptr;

  if(!localplayer->is_alive())
    return nullptr;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return nullptr;

  if(wep->is_rifle() || wep->is_shotgun())
    return &config->aimbot[AIMBOT_SETTING_RIFLE];
  else if(wep->is_sniper_rifle())
    return &config->aimbot[AIMBOT_SETTING_SNIPER];
  else if(wep->is_smg())
    return &config->aimbot[AIMBOT_SETTING_SMG];
  else if(wep->is_pistol())
    return &config->aimbot[AIMBOT_SETTING_PISTOL];

  return &config->aimbot[AIMBOT_SETTING_MELEE];
}

c_triggerbot_settings* utils::get_triggerbot_settings(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return nullptr;

  if(!localplayer->is_alive())
    return nullptr;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return nullptr;

  if(wep->is_rifle() || wep->is_shotgun())
    return &config->triggerbot[AIMBOT_SETTING_RIFLE];
  else if(wep->is_sniper_rifle())
    return &config->triggerbot[AIMBOT_SETTING_SNIPER];
  else if(wep->is_smg())
    return &config->triggerbot[AIMBOT_SETTING_SMG];
  else if(wep->is_pistol())
    return &config->triggerbot[AIMBOT_SETTING_PISTOL];

  return &config->triggerbot[AIMBOT_SETTING_MELEE];
}

float utils::get_latency(){
  return global->engine->get_latency(false, true) + global->engine->get_latency(true, true);
}

vec3 utils::trace_line(vec3 start, vec3 end, u32 mask, u32 custom_trace_filter){
  s_trace trace = global->trace->ray(start, end, mask != 0 ? mask : mask_bullet, TRACE_EVERYTHING, nullptr, custom_trace_filter);
  return trace.end;
}