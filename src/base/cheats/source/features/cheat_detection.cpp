#include "../../../link.h"

void c_base_cheat_detection::transmit_iden(){
  const  float time           = math::time();
  static float next_iden_time =  0.f;
  if(global_internal->steamcontext == nullptr || global_internal->steamcontext->steamuser == nullptr)
    return;

  if(next_iden_time > time)
    return;
  
  next_iden_time = time + 30.f;
  utils::create_worker([](void* p){
    if(global_internal->steamcontext == nullptr)
      return 0;

    if(global_internal->steamcontext->steamuser == nullptr)
      return 0;

    c_steamid sid;
    memset(&sid, 0, sizeof(c_steamid));
    if(!global_internal->steamcontext->steamuser->get_steam_id(&sid)){
      DBG("[-] c_base_cheat_detection::transmit_iden() failed to resolve sid\n");
      return 0;
    }

    DBG("[!] Transmitting ID: %i\n", sid.m_steamid.comp.account_id);
    transmit_identifier(gen_internal->cloud_license_key, sid.m_steamid.comp.account_id);
    return 0;
  }, nullptr);
}

void c_base_cheat_detection::calculate_should_run(){
  _should_run_detections = false;
  cookie_block_check_return();
  #if !defined(DEV_MODE)
  if(!global_internal->engine->is_in_game() || global_internal->engine->is_playing_demo())
    return;
  #endif

  if(utils::is_server_lagging() || utils::is_tick_rate_low())
    return;

  assert(global_internal->sv_cheats != nullptr);
  if(global_internal->sv_cheats->val != 0)
    return;

  _should_run_detections = true;
}

void c_base_cheat_detection::frame_stage_notify(u32 stage){
  if(stage != FRAME_NET_UPDATE_END)
    return;

  calculate_should_run();
  for(i32 i = 1; i <= global_internal->global_data->max_clients; i++){
    fill_player_info(i);
    check_entity(i);
  }
}

void c_base_cheat_detection::on_fire_event(c_game_event* event){
  if(event == nullptr)
    return;

  u32 name = HASH_RT(event->get_name());
  if(name == HASH("player_disconnect") || name == HASH("player_connect") || name == HASH("player_connect_client")){
    if(event->get_bool(XOR("bot")))
      return;

    i8* player_name = event->get_string(XOR("name"));
    i8* steamid     = event->get_string(XOR("networkid"));

    if(steamid == nullptr)
      return;

    if(!str_utils::contains(steamid, XOR("[U:1:")))
      return;

    i8 number_str[128];
    memset(number_str, 0, sizeof(number_str));
    i32 c = 0;
    for(u32 i = 5; i <= strlen(steamid); i++){
      if(!convert::is_number(steamid[i]))
        continue;
  
      number_str[c] = steamid[i];
      c++;
      if(c >= 128)
        break;
    }

    const u32 id = convert::str_to_i32(number_str);

    unlink_entity_index_by_sid(id);

    c_base_detection_info* data = assign_info(id, WXOR(L"on_fire_event"));
    if(data == nullptr)
      return;

    // If we have an entity index set, lets check to make sure this is related to an entry, if not then we need to unlink it and reset data.
    if(data->index != 0){
      bool should_unlink = false;
      c_entity_info* info = get_entity_info(data->index);
      should_unlink = !(info != nullptr && info->info != nullptr && info->info->info.friends_id == id && info->friends_id == id);

      if(should_unlink){
        DBG("\n\n\n\n[+] We got a player %s who is taking an entry from another player!\n", steamid);
        unlink(data->index);
        data->reset_in_game_data();
      }
    }

    data->position_state = name == HASH("player_disconnect") ? SID_STATE_DISCONNECTED : SID_STATE_CONNECTING;
    data->info.friends_id = id;

    if(player_name != nullptr)
      strcpy(data->info.name, player_name);

    process_notify(data);
  }
  else if(name == HASH("object_deflected")){
    i32 reflector = global_internal->engine->get_player_for_user_id(event->get_int(XOR("userid")));
    check_angles_for_inhuman(reflector, 0);
  }
  else if(name == HASH("infected_death") || name == HASH("infected_hurt")){
    i32 attacker = global_internal->engine->get_player_for_user_id(event->get_int(XOR("attacker")));
    check_angles_for_inhuman(attacker, 0);
  }
  else if(name == HASH("player_death")){
    i32 attacker = global_internal->engine->get_player_for_user_id(event->get_int(XOR("attacker")));
    i32 attacked = global_internal->engine->get_player_for_user_id(event->get_int(XOR("userid")));
    if(attacker != attacked)
      check_angles_for_inhuman(attacker, attacked);
  }
}

void c_base_cheat_detection::fill_player_info(i32 index){
  s_player_info info;
  if(!global_internal->engine->get_player_info(index, &info))
    return;

  if(info.fake_player)
    return;

  c_entity_info* entity_info = get_entity_info(index);
  if(entity_info != nullptr){
    if(entity_info->friends_id == info.friends_id){
      process_notify(entity_info->info);
      return;
    }
  }

  c_base_detection_info* data = assign_info(info.friends_id, WXOR(L"fill_player_info"));
  if(data == nullptr)
    return;

  if(data->first_time_index == index)
    return;

  cookie_block_check_return();

  data->first_time_index = index;
  data->index = index;
  memcpy(&data->info, &info, sizeof(s_player_info));
  setup_entity(index, data);
  DBG("[+] c_base_cheat_detection::fill_player_info %i %i %s\n", index, info.friends_id, info.name);
}

void c_base_cheat_detection::check_entity(i32 index){
  if(!_should_run_detections)
    return;

  c_entity_info* data = get_entity_info(index);
  if(data == nullptr)
    return;

  fetch_steam_data(data->info);

  c_internal_base_player* player = global_internal->entity_list->get_entity(index);
  if(player == nullptr)
    return;

  data->info->position_state = SID_STATE_SPAWNED;

  if(!should_run_on_player(player))
    return;

  analyze_angles(player, data);
  analyze_movement(player, data);
}

void c_base_cheat_detection::fetch_steam_data(c_base_detection_info* info){
  if(info == nullptr)
    return;

  if(global_internal->steamcontext == nullptr){
    DBG("[-] c_base_cheat_detection::fetch_steam_data steamcontext is nullptr\n");
    return;
  }

  if(global_internal->steamcontext->steamfriends == nullptr){
    DBG("[-] c_base_cheat_detection::fetch_steam_data steam_friends is nullptr.\n");
    return;
  }

  const float time = math::time();
  if(info->next_steam_fetch_time > time)
    return;

  i8* name = utils::get_steam_name_from_sid(info->info.friends_id);
  if(name != nullptr){
    u32 name_hash = HASH_RT(name);
    if(name_hash != HASH("[unknown]") && name_hash != HASH("[Unknown]")){
      strcpy(info->steam_name, name);
      strcpy(info->abb_steam_name, name);

      i32 trim_at = 14;
      for(i32 i = trim_at; i <= trim_at + 1; i++)
        info->abb_steam_name[i] = '.';

      info->abb_steam_name[trim_at + 2] = '\0';
    }
    else
      info->reset_steam_names();
  }
  else
    info->reset_steam_names();


  info->steam_friend = utils::is_steam_friend(info->info.friends_id);
  info->next_steam_fetch_time = time + 10.f;
}

void c_base_cheat_detection::decay_infractions(c_base_detection_info* info, u32 type){
  if(info == nullptr)
    return;

  const float time = math::time();

  float infractions = info->infractions[type];
  float expire_time = info->infractions_reset_time[type];
  if(expire_time > 0.f && expire_time > time)
    return;

  float expire_delay = 60.f;
  switch(type){
    default: break;
    case DETECTION_ANGLE_ANALYTICAL:
    {
      expire_delay = 10.f;
      break;
    }
    case DETECTION_SPEEDHACK:
    case DETECTION_TICKBASE_ABUSE:
    {
      expire_delay = 5.f;
      break;
    }
  }

  if(infractions > 1){
    DBG("[!] c_base_cheat_detection::decay_infractions for %s %i - %f\n", info->info.name, type, expire_delay);
  }

  info->infractions[type] = 0;
  info->infractions_reset_time[type] = time + expire_delay;
}

bool c_base_cheat_detection::should_run_on_player(c_internal_base_player* player){
  if(player == nullptr)
    return false;

  if(player->is_dormant() || !player->is_base_alive())
    return false;

  if(player->team() == 0 || player->get_index() == global_internal->localplayer_index)
    return false;

  return true;
}

void c_base_cheat_detection::analyze_angles(c_internal_base_player* player, c_entity_info* data){
  if(!should_detect_obb_pitch())
    return;

  vec3 angles = player->viewangles();

  if(math::abs(angles.x) >= get_obb_pitch_value()){
    on_cheater_detected(player, DETECTION_OBB_PITCH);
    return;
  }
}

void c_base_cheat_detection::analyze_movement(c_internal_base_player* player, c_entity_info* data){
  if(!should_detect_movement())
    return;

  c_base_detection_info* info = data->info;
  if(info == nullptr)
    return;

  assert(global_internal->client_state != nullptr);

  float time = math::time();

  const i32 sec_ticks        = math::time_to_ticks(1.f);
  i32       simulation_ticks = math::time_to_ticks(player->simulation_time());
  i32       server_ticks     = global_internal->client_state->server_tick();
  i32       tick_diff        = math::abs(simulation_ticks - server_ticks);
  bool      is_lagging       = tick_diff > 3 && tick_diff <= sec_ticks;

  // If the player does something that can can trigger a false positive then this value should be.
  if(info->unstable_timer <= time){

    // If the simulation time somehow goes down, usually rounds down, it means they could be using the tickbase exploit.
    // This never occurs when they lag or lose internet connection.
    if(simulation_ticks < info->last_simulation_ticks){
      info->suspect_tickbase_occured_time = time + 10.f;
      DBG("[!] simulation_ticks < last_simulation_ticks\n");
      on_infraction_received(player, DETECTION_TICKBASE_ABUSE);
    }

    // The server will always begin to simulate the player after a second. For a buffer period we check if they haven't been simulated for more than 2 seconds.
    if(tick_diff > sec_ticks * 2){
      if(info->tickbase_infraction_wait <= time){
        DBG("[!] [analyze movement] timing out players are simulated by the server, this doesn't make any sense.\n");
        on_infraction_received(player, DETECTION_TICKBASE_ABUSE);
        info->is_abusing_tickbase = true;
        info->tickbase_infraction_wait = time + 0.25f; // Avoid flooding, incase an unhandled false positive event.
      }
    }
    else
      info->is_abusing_tickbase = false;

    // Detect: Speedhacking
    if(info->last_simulation_ticks > 0 && player->move_type() == MOVETYPE_WALK && player->max_speed() == 220.f){
      float cost = is_lagging ? 8.f + (float)tick_diff : 8.f;

      if(is_lagging) // Players who lag can move faster.
        cost += (float)tick_diff;

      // We don't take simulation time deltas since they can be huge deltas.
      const bool is_over_speed = math::biggest((info->last_origin - player->origin()).length_2d() - cost, 0.f) >= 1.f;
      if(is_over_speed){
        // Since I have no idea what can cause an infraction, we require 3 flags to trigger one infraction. And only every 100ms.
        if(info->speedhack_infraction_wait <= time){
          if(info->speedhack_violations >= 6){
            on_infraction_received(player, DETECTION_SPEEDHACK);
            info->speedhack_violations = 0;
          }

          info->speedhack_violations++;
          info->speedhack_infraction_wait = time + 0.25f;
        }
      }
      else{
        if(info->speedhack_infraction_wait <= time)
          info->speedhack_violations = math::biggest(0, info->speedhack_violations - 1);
      }

    }
  }

  info->last_origin           = player->origin();
  info->last_simulation_ticks = simulation_ticks;
}

void c_base_cheat_detection::check_angles_for_inhuman(c_internal_base_player* player, c_internal_base_entity* victim, float aimbot_snap, float angle_repeat_snap){
  if(!should_run_on_player(player))
    return;

  c_internal_base_weapon* weapon = player->get_weapon();
  if(weapon == nullptr)
    return;

  c_entity_info* data = get_entity_info(player->get_index());
  if(data == nullptr)
    return;

  if(!data->info->can_check_angles())
    return;

  aimbot_snap       = math::biggest(aimbot_snap, 45.f);
  angle_repeat_snap = math::biggest(angle_repeat_snap, 5.f);


  vec3 origin = player->origin();

  c_base_generic_trace_filter filter;
  filter.ignore_entity = player;  

  // See if the player is standing on anything.
  s_trace tr = global_internal->base_trace->internal_ray(filter, origin, origin - vec3(0.f, 0.f, 2.f), mask_playersolid);
  if(tr.fraction < 1.f){
    if(tr.hit_entity == nullptr)
      return;

    // If we didn't hit the world then lets just fail to prevent any false positives.
    if(tr.hit_entity->get_index() != 0)
      return;
  }

  // Do angle delta normally.
  float delta = (data->info->current_viewangle - data->info->last_viewangle_1tick).length_2d();
  if(delta > 180.f)
    delta -= 360.f;

  // Otherwise, take current angle vs position.
  if(victim != nullptr){
    if(victim->get_index() != 0)
      delta = math::get_fov_delta(data->info->current_viewangle, player->shoot_pos(), victim->obb_center());
  }

  if(is_weapon_allowed_for_angles(weapon)){
    if(delta >= aimbot_snap){
      DBG("[!] %s snapped %f >= %f\n", player->info().name, delta, aimbot_snap);
      on_infraction_received(player, DETECTION_ANGLE_ANALYTICAL);
    }
  }

  if(is_weapon_allowed_for_repeat(weapon)){
    if(delta >= angle_repeat_snap){
      if((i32)data->info->current_viewangle.y == (i32)data->info->last_viewangle_2tick.y && (i32)data->info->current_viewangle.y != (i32)data->info->last_viewangle_1tick.y){
        DBG("[!] %s angle repeat %f, %f, %f (%f >= %f)\n", player->info().name,
          data->info->current_viewangle.y,
          data->info->last_viewangle_1tick.y,
          data->info->last_viewangle_2tick.y,
          delta,
          angle_repeat_snap);

        on_infraction_received(player, DETECTION_ANGLE_ANALYTICAL);
      }
    }
  }
}

void c_base_cheat_detection::on_infraction_received(c_internal_base_player* player, u32 type){
  if(player == nullptr)
    return;

  c_entity_info* data = get_entity_info(player->get_index());
  if(data == nullptr)
    return;

  const float time = math::time();
  decay_infractions(data->info, type);

  i32   infractions = data->info->infractions[type];
  float expire_time = data->info->infractions_reset_time[type];

  infractions++;
  if(infractions >= 20)
    on_cheater_detected(player, type);

  if(infractions >= 1){
    add_if_empty(WXOR(L"Notice: Just because a player receives an infraction does not mean they're cheating."));
    static wchar_t buf[1024];
    I(wsprintfW)(buf, WXOR(L"[U:1:%i] as \"%ls\" received infraction for %ls"),
      data->info->info.friends_id,
      convert::str2wstr(data->info->info.name).c_str(),
      get_detection_type(type).c_str());

    add(buf, data->info->info.friends_id);
  }

  expire_time += 0.5f;
  DBG("[+] c_base_cheat_detection::on_infraction_received %s [U:1:%i]  %i/20\n", data->info->info.name, data->info->info.friends_id, infractions);

  data->info->infractions[type]            = infractions;
  data->info->infractions_reset_time[type] = expire_time;
}

void c_base_cheat_detection::on_cheater_detected(c_internal_base_player* player, u32 type){
  if(player == nullptr){
    DBG("[-] c_base_cheat_detection::on_cheater_detected player parameter is nullptr.\n");
    return;
  }

  // Lets not report ourselves, since people might get upset from that.
  if(player->get_index() == global_internal->localplayer_index){
    DBG("[-] c_base_cheat_detection::on_cheater_detected can't report localplayer for cheating.")
    return;
  }

  c_entity_info* data = get_entity_info(player->get_index());
  if(data == nullptr){
    DBG("[-] c_base_cheat_detection::on_cheater_detected unable to locate cheater data for player %s\n", player->info().name);
    return;
  }

  assert(data->info != nullptr);
  if(data->info->fetch_state != INFO_FETCHED || data->info->has_submitted){
    //DBG("[-] c_base_cheat_detection::on_cheater_detected %i %i - info state for %s not ready\n", data->info->fetch_state, data->info->has_submitted, player->info().name);
    return;
  }

  utils::submit_cheater_to_network(data->info->info.friends_id, type, game_hash());

  wchar_t buf[1024];
  wchar_t player_name[33];
  convert::str2wstr(data->info->info.name, player_name, sizeof(player_name));

  if(!(data->info->cheater_flags & CHEATER_FLAG_IS_KNOWN_CHEATER))
    I(wsprintfW)(buf, WXOR(L"Marking \"%ls\" as a cheater and reporting the detection \"%ls\" to RijiN's database."), player_name, get_detection_type(type).c_str());
  else
    I(wsprintfW)(buf, WXOR(L"Submitting cheater detection data on \"%ls\" in relation to \"%ls\"."), player_name, get_detection_type(type).c_str());

  notify->create(NOTIFY_TYPE_ALERT, buf)
    ->set_data(NOTIFY_METADATA_STEAM, data->info->info.friends_id);

  data->info->cheater_flags |= CHEATER_FLAG_IS_KNOWN_CHEATER;
  data->info->has_submitted = true;
}

bool c_base_cheat_detection::is_cheating(i32 index, bool exclude_player_list){
  if(index > global_internal->global_data->max_clients){
    //DBG("[-] c_base_cheat_detection::is_cheating index of %i is not valid for players\n", index);
    return false; 
  }

  s_player_info info;
  global_internal->skip_get_player_info_hook = true;
  bool result = global_internal->engine->get_player_info(index, &info);
  global_internal->skip_get_player_info_hook = false;
  if(result){
    if(info.fake_player)
      return false;
  }

  c_entity_info* data = get_entity_info(index);
  if(data == nullptr)
    return false;

  return data->info->cheater_flags & CHEATER_FLAG_IS_KNOWN_CHEATER || data->info->cheater_flags & CHEATER_FLAG_IS_KNOWN_BOT || data->info->cheater_flags & CHEATER_FLAG_KNOWN_CHEATER_IN_OTHER_GAME;
}

bool c_base_cheat_detection::is_cheating_bot(i32 index, bool exclude_player_list){
  if(index > global_internal->global_data->max_clients){
    //DBG("[-] c_base_cheat_detection::is_cheating_bot index of %i is not valid for players\n", index);
    return false; 
  }

  s_player_info info;
  global_internal->skip_get_player_info_hook = true;
  bool result = global_internal->engine->get_player_info(index, &info);
  global_internal->skip_get_player_info_hook = false;
  if(result){
    if(info.fake_player)
      return false;
  }

  c_entity_info* data = get_entity_info(index);
  if(data == nullptr)
    return false;

  return data->info->cheater_flags & CHEATER_FLAG_IS_KNOWN_BOT;
}

bool c_base_cheat_detection::is_friend(i32 index, bool exclude_player_list){
  if(index == global_internal->localplayer_index)
    return true;

  if(index > global_internal->global_data->max_clients){
    DBG("[-] c_base_cheat_detection::is_friend index of %i is not valid for players\n", index);
    return false;
  }

  c_entity_info* data = get_entity_info(index);
  if(data == nullptr)
    return false;

  return data->info->steam_friend || data->info->cheater_flags & CHEATER_FLAG_DEV_IGNORE;
}

i8* c_base_cheat_detection::get_steam_name(i32 index, bool abb = false){
  if(index > global_internal->global_data->max_clients){
    //DBG("[-] c_base_cheat_detection::is_friend index of %i is not valid for players\n", index);
    return nullptr;
  }

  c_entity_info* data = get_entity_info(index);
  if(data == nullptr)
    return nullptr;

  i8* p = abb ? data->info->abb_steam_name : data->info->steam_name;

  return p[0] != '\0' ? p : nullptr;
}


void c_base_cheat_detection::fetch_server_info(){
  float time = math::time();
  static float last_run_time = 0.f;

  if(last_run_time > time)
    return;

  last_run_time = time + 1.f;

  for(u32 i = 0; i < MAX_INFO; i++){
    if(!d_info[i].info.friends_id || d_info[i].fetch_state)
      continue;

    d_info[i].fetch_state = INFO_FETCHING;
    utils::create_worker([](c_base_detection_info* data){
      while(global_internal->is_talking_to_tfcl)
        Sleep(100);

      c_net_request_tf_cheater_sid32_response* response = nullptr;
      DBG("[+] Sending request [U:1:%i]\n", data->info.friends_id);
      bool success = utils::get_cheater_data_from_network(data->info.friends_id, data->game_hash, response);
      data->fetch_state = INFO_FETCHED;
      if(!success || response == nullptr)
        return 0;

      data->cheater_flags  = response->cheater_flags;
      data->cheater_groups = response->cheater_steam_group_count;
      data->bot_groups     = response->bot_steam_group_count;

      if(response->alias_len > 0){
        strcpy(data->alias, response->alias);
        data->has_alias = true;
      }

      if(response->group_len > 0){
        strcpy(data->group, response->group);
        data->has_group = true;
      }

      DBG("[+] Received data for [U:1:%i]\n", data->info.friends_id);
      return 0;
    }, &d_info[i]);
  }
}

void c_base_cheat_detection::process_notify(c_base_detection_info* data){
  if(data == nullptr || notify == nullptr){
    DBG("[-] c_base_cheat_detection::process_notify: %p %p notify or data is nullptr\n", notify, data);
    return;
  }

  if(data->fetch_state != INFO_FETCHED)
    return;

  if(!data->info.friends_id){
    DBG("[-] c_base_cheat_detection::process_notify, %s has no sid?!\n", data->info.friends_id);
    return;
  }

  u32 cheater_flags = data->cheater_flags;
  cheater_flags &= ~CHEATER_FLAG_RIJIN_USER;

  if(!data->has_alias && !data->has_group && !cheater_flags && !data->bot_groups && !data->cheater_groups)
    return;
  
  if(data->notify_position_state == data->position_state)
    return;
  
  DBG("[!] Name: %s\nid: %i\ncheater_flags: %i\nalias: %s\ngroup: %s\n", data->info.name, data->info.friends_id, data->cheater_flags, data->alias, data->group);

  wchar_t b_str[128];
  wchar_t kc_str[128];
  wchar_t kb_str[128];
  wchar_t kco_str[128];
  wchar_t l_str[128];
  wchar_t dev_str[128];
  wchar_t known_actor_info[256];

  wchar_t steam_group_info[256];
  wchar_t alias[33];
  wchar_t group[33];

  I(wsprintfW)(b_str, WXOR(L"The player"));
  I(wsprintfW)(kc_str, WXOR(L"is a known cheater"));
  I(wsprintfW)(kb_str, WXOR(L"is a known bot"));
  I(wsprintfW)(kco_str, WXOR(L"is known to cheat in other games"));
  I(wsprintfW)(l_str, WXOR(L"is not a cheater"));
  I(wsprintfW)(dev_str, WXOR(L"has rijin developer status applied"));

  const i32 total = data->bot_groups + data->cheater_groups;

  if(total)
    wsprintfW(steam_group_info, total == 1 ? WXOR(L", one cheating steam group") : WXOR(L", in %i+ cheating steam groups"), total);
  else
    memset(steam_group_info, 0, sizeof(steam_group_info));

  if(data->has_alias)
    convert::str2wstr(data->alias, alias, sizeof(alias));

  if(data->has_group)
    convert::str2wstr(data->group, group, sizeof(alias));

  if(data->has_alias && !data->has_group)
    wsprintfW(known_actor_info, WXOR(L", known by alias \"%ls\""), alias);
  else if(!data->has_alias && data->has_group)
    wsprintfW(known_actor_info, WXOR(L", known to be in the group \"%ls\""), group);
  else if(data->has_alias && data->has_group)
    wsprintfW(known_actor_info, WXOR(L", known by alias \"%ls\" and in the group \"%ls\""), alias, group);
  else
    memset(known_actor_info, 0, sizeof(known_actor_info));

  wchar_t buf[1024];
  wchar_t player_name[33];

  convert::str2wstr(data->info.name, player_name, sizeof(player_name));

  if(cheater_flags & CHEATER_FLAG_DEV_IGNORE || cheater_flags & CHEATER_FLAG_DEV_ESP)
    I(wsprintfW)(buf, WXOR(L"%ls \"%ls\" %ls%ls%ls"), b_str, player_name, dev_str, known_actor_info, steam_group_info);
  else if(cheater_flags & CHEATER_FLAG_IS_KNOWN_BOT)
    I(wsprintfW)(buf, WXOR(L"%ls \"%ls\" %ls%ls%ls"), b_str, player_name, kb_str, known_actor_info, steam_group_info);
  else if(cheater_flags & CHEATER_FLAG_IS_KNOWN_CHEATER)
    I(wsprintfW)(buf, WXOR(L"%ls \"%ls\" %ls%ls%ls"), b_str, player_name, kc_str, known_actor_info, steam_group_info);
  else if(cheater_flags & CHEATER_FLAG_KNOWN_CHEATER_IN_OTHER_GAME)
    I(wsprintfW)(buf, WXOR(L"%ls \"%ls\" %ls%ls%ls"), b_str, player_name, kco_str, known_actor_info, steam_group_info);
  else
    I(wsprintfW)(buf, WXOR(L"%ls \"%ls\" %ls%ls%ls"), b_str, player_name, l_str, known_actor_info, steam_group_info);

  wchar_t state_info[128];

  if(data->position_state == SID_STATE_CONNECTING)
    I(wsprintfW)(state_info, WXOR(L" [Connecting]"));
  else if(data->position_state == SID_STATE_SPAWNED)
    I(wsprintfW)(state_info, WXOR(L" [In-game]"));
  else if(data->position_state == SID_STATE_DISCONNECTED)
    I(wsprintfW)(state_info, WXOR(L" [Disconnected]"));
  else
    memset(state_info, 0, sizeof(state_info));

  wchar_t final_buf[1024];
  I(wsprintfW)(final_buf, WXOR(L"%ls.%ls"), buf, state_info);


  notify->create((data->cheater_flags & CHEATER_FLAG_IS_KNOWN_BOT || data->cheater_flags & CHEATER_FLAG_IS_KNOWN_CHEATER || data->cheater_flags & CHEATER_FLAG_KNOWN_CHEATER_IN_OTHER_GAME) ? NOTIFY_TYPE_ALERT : NOTIFY_TYPE_WARNING,
   final_buf, colour(255, 255, 255, 255))
  ->set_data(NOTIFY_METADATA_STEAM, data->info.friends_id);

  DBG("[!] sending notification: %s\n", data->info.name);
  data->notify_position_state = data->position_state;
}

void c_base_cheat_detection::engine_get_player_info_hook(i32 entity_id, s_player_info* info){
  if(info == nullptr)
    return;

  c_entity_info* data = get_entity_info(entity_id);
  if(data == nullptr)
    return;

  if(!(data->info->cheater_flags & CHEATER_FLAG_IS_KNOWN_BOT) && !data->info->has_alias)
    return;

  if(data->info->has_alias)
    strcpy(info->name, data->info->alias);
  else if(data->info->cheater_flags & CHEATER_FLAG_IS_KNOWN_BOT)
    strcpy(info->name, XOR("BOT"));
}

void c_base_cheat_detection::on_float_decode(c_internal_base_entity* entity, u32 name_hash, void* p){
  if(p == nullptr)
    return;

  bool is_pitch = (name_hash == HASH("m_angEyeAngles[0]"));
  bool is_yaw   = (name_hash == HASH("m_angEyeAngles[1]"));
  if(!is_pitch && !is_yaw)
    return;

  c_entity_info* data = get_entity_info(entity->get_index());
  if(data == nullptr)
    return;

  if(is_pitch){
    data->info->angle_collection_ticks[0]++;
    data->info->last_viewangle_2tick.x = data->info->last_viewangle_1tick.x;
    data->info->last_viewangle_1tick.x = data->info->current_viewangle.x;
    data->info->current_viewangle.x = *(float*)p;
  }
  else if(is_yaw){
    data->info->angle_collection_ticks[1]++;
    data->info->last_viewangle_2tick.y = data->info->last_viewangle_1tick.y;
    data->info->last_viewangle_1tick.y = data->info->current_viewangle.y;
    data->info->current_viewangle.y = *(float*)p;
  }
}