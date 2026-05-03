#include "link.h"

void utils::store_bone_data(){
  if(global->speedhack_tick)
    return;

  for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_player* p = global->entity_list->get_entity(i);

    if(p == nullptr)
      continue;

    if(!p->is_npc_or_human())
      continue;

    if(p->is_dormant() || !p->is_alive())
      continue;

    s_studio_hdr* hdr = p->get_studio_hdr();

    if(hdr == nullptr)
      continue;

    c_entity_data* entity_data = p->data();
    if(entity_data == nullptr)
      continue;

    if(p->simulation_time() <= entity_data->bone_last_simulation_time)
      continue;

    entity_data->bone_last_simulation_time = p->simulation_time();
    if(!p->setup_bones((matrix3x4*)&entity_data->bone_matrix, true))
      continue;

    entity_data->bone_count = hdr->num_bones;
  }
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
  if(localplayer->is_steam_friend(player))
    type = 2;
  else
    type = (player->team() == localplayer->team()) ? 0 : 1;

  return utils::get_player_esp_settings(type);
}

c_player_chams_settings* utils::get_player_chams_settings(i32 type){
  if(type < 0 || type > 2)
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
  if(localplayer->is_steam_friend(player))
    type = 2;
  else
    type = (player->team() == localplayer->team()) ? 0 : 1;

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

bool utils::reset_entity_data(i32 index){
  index--;

  if(index < 0 || index >= (sizeof(global->entity_data) / sizeof(c_entity_data)))
    return false;

  memset(&global->entity_data[index], 0, sizeof(c_entity_data));
  global->entity_data[index].playerlist.init();
  return true;
}

void utils::reset_all_entity_data(){
  memset(&global->entity_data, 0, sizeof(global->entity_data));

  DBG("[!] utils::reset_all_entity_data\n");
}

void utils::verify_entity_data(i32 index){
  c_base_entity* entity = global->entity_list->get_entity(index);
  if(entity == nullptr)
    return;

  c_entity_data* data = entity->data();
  if(data == nullptr)
    return;

  if(index > globaldata->max_clients)
    return;

  s_player_info info;
  if(!global->engine->get_player_info(index, &info))
    return;

  if(data->friends_id != info.friends_id){
    DBG("[!] %i friends id mismatch %i != %i\n", index, data->friends_id, info.friends_id);
    utils::reset_entity_data(index);
    data->friends_id = info.friends_id;
  }
}

void utils::verify_all_entity_data(){
  //if(global->verified_entity_data)
  //  return;

  global->verified_entity_data = true;
  for(i32 i = 1; i <= globaldata->max_clients; i++)
    utils::verify_entity_data(i);
}

void utils::check_for_steam_friends(){
  float time = math::time();
  for(i32 i = 1; i <= globaldata->max_clients; i++){
    c_base_player* player = global->entity_list->get_entity(i);
    if(player == nullptr)
      continue;

    c_entity_data* data = player->data();
    if(data == nullptr)
      continue;

    if(data->next_steam_friend_check > time)
      continue;

    s_player_info info = player->info();
    if(!info.friends_id){
      data->steam_friend = false;
      continue;
    }

    if(data->playerlist.mark_as_friend){
      data->next_steam_friend_check = time + 5.0f;
      data->steam_friend = true;
      continue;
    }

    if(i == global->engine->get_local_player()){
      data->next_steam_friend_check = time + 5.0f;
      data->steam_friend = true;
      continue;
    }


    data->next_steam_friend_check = time + 5.0f;
    data->steam_friend            = global->steamcontext->steamfriends->has_friend(utils::get_user_steamid(info.friends_id));
  }
}

void utils::detect_objective_entity(c_base_entity* entity){
  if(entity == nullptr)
    return;

  c_entity_data* data = entity->data();
  if(data == nullptr)
    return;

  float time = math::time();
  s_base_button* button = entity->get_button_entity();
  if(button != nullptr){
    if(!button->usable())
      return;

    if(button->glow_entity() != -1){
      c_base_entity* glow_entity = global->entity_list->get_entity_handle(button->glow_entity());
      if(glow_entity != nullptr){
        c_entity_data* glow_data = glow_entity->data();
        if(glow_data != nullptr){
          glow_data->is_currently_objective_timer = time + 1.f;
          glow_data->is_objective                 = true;
        }
      }
    }
  }

  data->is_objective = data->is_currently_objective_timer > 0.f && data->is_currently_objective_timer >= time;
  s_glow_property* glow = entity->get_glow_property();
  if(glow == nullptr)
    return;

  // There is no glow on this entity.
  if(!glow->type())
    return;

  data->is_currently_objective_timer = time + 1.f;
  data->is_objective                 = true;
}

c_entity_data* utils::get_entity_data(i32 index){
  index--;

  if(index < 0 || index >= (sizeof(global->entity_data) / sizeof(c_entity_data))){
    DBG("[-] get_entity_data overflow: %i\n", index);
    return nullptr;
  }

  return &global->entity_data[index];
}

void utils::transmit_steamid(){
  if(!global->engine->is_in_game())
    return;

  s_player_info info;
  if(!global->engine->get_player_info(global->engine->get_local_player(), &info))
    return;

  static float next_transmit_time = 0.f;
  float time                      = math::time();
  if(next_transmit_time > time)
    return;

  static u32* data = malloc(sizeof(u32));
  *data            = info.friends_id;

  utils::create_worker([](u32* friends_id){
    Sleep(XOR32(200));

    u32 id = *friends_id;
    transmit_identifier(gen_internal->cloud_license_key, id);
    return 0;
  }, data);

  next_transmit_time = time + 30.f;
}

bool utils::is_in_thirdperson(){
  if(thirdperson->is_active() || thirdperson->is_in_thirdperson_view())
    return true;

  return false;
}