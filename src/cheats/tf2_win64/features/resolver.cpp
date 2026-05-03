#include "../link.h"

CLASS_ALLOC(c_resolver, resolver);

void c_resolver::on_shot(c_base_player* player){
  if(!config->hvh.resolver)
    return;

  if(utils::is_playing_mvm())
    return;

  if(player == nullptr)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  if(player->team() == localplayer->team())
    return;

  if(global->aimbot_settings != nullptr && !global->aimbot_settings->no_spread){
    if(!is_shot_accurate())
      return;
  }

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  ctf_weapon_info* info = weapon->get_weapon_info();
  if(info == nullptr)
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  // We got a shot queued wait until we hit them or miss them.
  if(data->shot_data.timeout > 0.f)
    return;

  // Make sure everything is reset to prevent any issues.
  memset(&data->shot_data, 0, XOR32(sizeof(s_resolver_shot_data)));

  if(utils::is_truce_active()){
    DBG("[!] not running on_shot for player %s because we're in a truce.\n", player->info().name);
    return;
  }

  data->shot_data.should_run = should_run(player);
  if(!data->shot_data.should_run){
    DBG("[!] not running on_shot for player %s\n", player->info().name);
    return;
  }

  // By ping plus 1 tick.
  float ping         = math::clamp(utils::get_latency(), 0.2f, 1.f);
  data->shot_data.timeout             = (math::time() + ping) + math::ticks_to_time(1);
  data->shot_data.using_pitch_antiaim = !data->playerlist.resolver_disallow && math::abs(data->networked_ang.x) >= 90.f;
  data->shot_data.using_yaw_antiaim   = data->resolver_yaw_active_time > math::time();
  data->shot_data.real_ang_updated    = data->real_ang_updated;
  data->shot_data.is_taunting         = player->is_taunting();
  DBG("[!] Processing shot for %s\n", player->info().name);
}

void c_resolver::detect_yaw_antiaim(c_base_player* player){
  if(player == nullptr)
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  s_lc_data* lc = lag_compensation->get_data(player);
  if(lc == nullptr)
    return;

  if(!lc->max_records)
    return;

  if(data->playerlist.resolver_disallow){
    data->resolver_yaw_active_time = -1.f;
    return;
  }

  u32   collected       = 0;
  u32   avg_choke_ticks = 0;
  float avg_yaw_delta   = 0.f;
  for(u32 i = 1; i < lc->max_records; i++){
    s_lc_record* last    = lc->get_record(i - 1);
    s_lc_record* current = lc->get_record(i);
    if(last == nullptr || current == nullptr)
      continue;

    float cur_yaw  = math::normalize_angle(current->viewangles.y);
    float last_yaw = math::normalize_angle(last->viewangles.y);

    avg_yaw_delta   += math::abs(cur_yaw - last_yaw);
    avg_choke_ticks += math::time_to_ticks(math::abs(current->simulation_time - last->simulation_time));
    collected++;
  }

  if(collected > 0){
    if(avg_choke_ticks > 0)
      avg_choke_ticks /= collected;

    if(avg_yaw_delta > 0.f){
      avg_yaw_delta /= (float)collected;
      if(avg_choke_ticks > 2)
        avg_yaw_delta /= (float)avg_choke_ticks;
    }
  }

  if(avg_choke_ticks >= 2 || avg_yaw_delta >= 25.f || data->playerlist.resolver_force)
    data->resolver_yaw_active_time = math::time() + 3.0f;
}

void c_resolver::process_shots(u32 stage){
  if(!config->hvh.resolver)
    return;

  if(stage != FRAME_RENDER_START)
    return;

  if(utils::is_playing_mvm())
    return;

  float time = math::time();
  if(global->next_resolver_broadcast_time <= time){
    c_party_chat_packet_resolver packet;
    packet.response = RESOLVER_RESPONSE_TYPE_BROADCASTING;
    send_resolver_data(packet);
    global->next_resolver_broadcast_time = time + 1.0f;
  }

  detect_sniper_dot();
  for(i32 i = 1; i <= globaldata->max_clients; i++){
    c_base_player* player = global->entity_list->get_entity(i);
    if(player == nullptr)
      continue;

    c_player_data* data = player->data();
    if(data == nullptr)
      continue;

    if(!data->shot_data.should_run || data->shot_data.is_taunting){
      reset_shot_data(player, false);
      continue;
    }

    // Haven't attempted to shoot at this person or we're still waiting for it to timeout.
    if(data->shot_data.timeout <= 0.f || data->shot_data.timeout > time)
      continue;

    reset_shot_data(player, false);
    bool using_pitch_antiaim  = data->shot_data.using_pitch_antiaim;
    bool using_yaw_antaim     = data->shot_data.using_yaw_antiaim;
    bool real_pitch_collected = data->shot_data.real_ang_updated & RESOLVER_REAL_UPDATE_PITCH;
    bool real_yaw_collected   = data->shot_data.real_ang_updated & RESOLVER_REAL_UPDATE_YAW;

    bool allow_yaw_cycle      = false;

    // Process pitch cycle
    if(using_pitch_antiaim && !is_playerlist_correcting(RESOLVER_MANUAL_PITCH, player)){
      if(!real_pitch_collected){ // Only cycle pitch resolver if don't know their real angle.
        data->resolver_pitch_cycle++;

        // We have tried every pitch angle lets reset it and cycle the player's cycle angle.
        if(data->resolver_pitch_cycle >= RESOLVER_MAX_PITCH_CYCLE){
          allow_yaw_cycle            = true;
          data->resolver_pitch_cycle = 0;
        }

        // Transmit data about how we missed this player during a pitch resolve.
        {
          c_party_chat_packet_resolver packet;
          packet.index                = i;
          packet.response             = RESOLVER_RESPONSE_TYPE_PITCH_MISS;
          packet.resolver_pitch_cycle = data->resolver_pitch_cycle;

          send_resolver_data(packet);
        }

        // Invalidate any real pitch angle we got.
        on_invalidated_real_angle(player, data, RESOLVER_INVALIDATE_PITCH);
      }
      else
        allow_yaw_cycle = true; // We hopefully have their real pitch angle lets allow yaw cycle.

      // Handle notifications for the pitch resolver.
      {
        //if(config->misc.chat_notification_resolver){
        //  char buf[128];
        //  if(!real_pitch_collected)
        //    wsprintfA(buf, XOR("Cycling pitch corrections on player \"\x7%06X%s\x1\""), HEX_ORANGE, player->info().name);
        //  else
        //    wsprintfA(buf, XOR("Used last (hit/detected) real pitch angle \"\x7%06X%s\x1\""), HEX_ORANGE, player->info().name);
//
        //  utils::send_nofitication(buf);
        //}
      }
    }
    else
      allow_yaw_cycle = true; // Not using a fake pitch angle so always allow this yaw angle.

    // We aren't allowed to cycle yaw right now or this player isn't using a yaw anti-aim.
    if(!allow_yaw_cycle || !using_yaw_antaim){
      on_invalidated_real_angle(player, data, RESOLVER_INVALIDATE_PITCH);
      continue;
    }

    // Don't cycle yaw at all, we're manually correcting.
    if(is_playerlist_correcting(RESOLVER_MANUAL_YAW, player))
      continue;

    // Looks like we got some real yaw data collected!
    if(real_yaw_collected){
      //if(config->misc.chat_notification_resolver){
      //  char buf[128];
      //  wsprintfA(buf, XOR("Used last (hit/detected) real yaw angle \"\x7%06X%s\x1\""), HEX_ORANGE, player->info().name);
      //  utils::send_nofitication(buf);
      //}

      // Transmit data about how we missed this player during a yaw resolve.
      {
        c_party_chat_packet_resolver packet;
        packet.index                = i;
        packet.response             = RESOLVER_RESPONSE_TYPE_YAW_MISS;
        packet.resolver_yaw_cycle = data->resolver_yaw_cycle;

        send_resolver_data(packet);
      }

      // Invalidate any real yaw angle we got on this player.
      on_invalidated_real_angle(player, data, RESOLVER_INVALIDATE_YAW);
      continue;
    }

    data->resolver_yaw_cycle++;

    // Handle notifications for the yaw resolver.
    {
      //if(config->misc.chat_notification_resolver){
      //  char buf[128];
      //  wsprintfA(buf, XOR("Cycling yaw correction on player \"\x7%06X%s\x1\""), HEX_ORANGE, player->info().name);
      //  utils::send_nofitication(buf);
      //}
    }

    if(data->resolver_yaw_cycle >= RESOLVER_MAX_YAW_CYCLE){
      // Darn! Lets make sure the pitch resolver has a chance to go through everything again...
      on_invalidated_real_angle(player, data, RESOLVER_INVALIDATE_PITCH);
      data->resolver_yaw_cycle = 0;
    }

    // Transmit data about how we missed this player during a yaw resolve.
    {
      c_party_chat_packet_resolver packet;
      packet.index                = i;
      packet.response             = RESOLVER_RESPONSE_TYPE_YAW_MISS;
      packet.resolver_yaw_cycle = data->resolver_yaw_cycle;

      send_resolver_data(packet);
    }

    // Invalidate any real yaw angle we got on this player.
    on_invalidated_real_angle(player, data, RESOLVER_INVALIDATE_YAW);
  }
}

void c_resolver::frame_stage_notify_run(u32 stage){
  if(!config->hvh.resolver)
    return;

  if(utils::is_playing_mvm())
    return;

  process_shots(stage);
  if(stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  float time = math::time();
  for(i32 i = 1; i <= globaldata->max_clients; i++){
    c_base_player* player = global->entity_list->get_entity(i);
    if(player == nullptr)
      continue;

     if(!should_run(player))
      continue;

    c_player_data* data = player->data();
    if(data == nullptr)
      continue;

    detect_yaw_antiaim(player);
    //check_movement(player, data);
    vec3 viewangle = data->networked_ang;
    if(!data->has_received_network_ang){
      viewangle           = player->viewangles();
      data->networked_ang = player->viewangles();
      data->real_ang      = player->viewangles();
      data->has_received_network_ang  = true;
      DBG("[!] We have never received real angle data on %s!!\n", player->info().name);
    }

    bool using_pitch_antiaim    = !data->playerlist.resolver_disallow && math::abs(viewangle.x) >= 90.f;
    bool using_yaw_antaim       = data->resolver_yaw_active_time > time;
    vec3 view                   = math::calc_view_angle(player->shoot_pos(), localplayer->shoot_pos());

    if(player->is_taunting()){
      player->viewangles() = data->networked_ang;
      continue;
    }

    bool real_pitch_collected = data->real_ang_updated & RESOLVER_REAL_UPDATE_PITCH;
    bool real_yaw_collected   = data->real_ang_updated & RESOLVER_REAL_UPDATE_YAW;

    bool allow_pitch_corrections = using_pitch_antiaim;
    bool allow_yaw_corrections   = using_yaw_antaim && !localplayer->has_condition(TF_COND_SHIELD_CHARGE);

    if(allow_pitch_corrections){
      if(!real_pitch_collected && is_playerlist_correcting(RESOLVER_MANUAL_PITCH, player)){

        if(data->playerlist.resolver_pitch_up)
          viewangle.x = -89.f;
        else if(data->playerlist.resolver_pitch_down)
          viewangle.x = 89.f;
        else if(data->playerlist.resolver_pitch_zero)
          viewangle.x = 0.f;
      }
      else{
        if(real_pitch_collected)
          viewangle.x = data->real_ang.x;
        else{
          switch(data->resolver_pitch_cycle){
            default: break;
            case 1: // Invert angles.
            {
              if(viewangle.x >= 90.f)
                viewangle.x = -89.f;
              else if(viewangle.x <= -90.f)
                viewangle.x = 89.f;
  
              break;
            }
            case 2: viewangle.x = 1.f; break;
            case 3: viewangle.x = -45.f; break;
          }
        }
      }
    }

    if(allow_yaw_corrections){
      if(is_playerlist_correcting(RESOLVER_MANUAL_YAW, player)){

        if(data->playerlist.resolver_yaw_neg90)
          viewangle.y = view.y - 90.f;
        else if(data->playerlist.resolver_yaw_pos90)
          viewangle.y = view.y + 90.f;
        else if(data->playerlist.resolver_yaw_neg45)
          viewangle.y = view.y - 45.f;
        else if(data->playerlist.resolver_yaw_pos45)
          viewangle.y = view.y + 45.f;
        else if(data->playerlist.resolver_yaw_lookat)
          viewangle.y = view.y;
        else if(data->playerlist.resolver_yaw_180away)
          viewangle.y = view.y + 180.f;

      }
      else{
        if(real_yaw_collected)
          viewangle.y = data->real_ang.y;
        else{
          switch(data->resolver_yaw_cycle){
            default: break;
            case 0: viewangle.y = view.y - 90.f; break;
            case 1: viewangle.y = view.y - 45.f; break;
            case 2: viewangle.y = view.y + 90.f; break;
            case 3: viewangle.y = view.y + 45.f; break;
            case 4: viewangle.y = view.y; break;
          }
        }
      }
    }

    //viewangle.y = math::normalize_angle(viewangle.y);
    //viewangle.z = 0.f;

    player->viewangles() = viewangle;
  }
}

void c_resolver::fire_event(c_game_event* event){
  if(!config->hvh.resolver)
    return;

  if(event == nullptr)
    return;

  i32 attacker  = global->engine->get_player_for_user_id(event->get_int(XOR("attacker")));
  i32 victim    = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));
  bool headshot = event->get_int(XOR("custom")) == 1;

  if(attacker != global->engine->get_local_player() || attacker == victim)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  c_base_player* player = global->entity_list->get_entity(victim);
  if(player == nullptr)
    return;

  if(!player->is_valid())
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  if(!should_run(player))
    return;

  if(data->shot_data.timeout <= 0.f){
    DBG("[!] We hit something but we aren't processing this shot.\n");
    return;
  }

  if(!headshot)
    return;

  reset_shot_data(player, true);
  DBG("[!] Hit\n");
}

void c_resolver::float_decode(c_base_entity* entity, float val, bool yaw){

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(entity == nullptr)
    return;

  if(!entity->is_player())
    return;

  c_base_player* player = (c_base_player*)entity;
  assert(player != nullptr);

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  if(!yaw)
    data->networked_ang.x      = val;
  else
    data->networked_ang.y      = val;
  
  data->has_received_network_ang = true;
}

void c_resolver::reset_shot_data(c_base_player* player, bool update_real_angle){
  if(player == nullptr)
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  data->shot_data.timeout = -1.f;

  if(update_real_angle)
    on_updated_real_angle(player, player->viewangles(), (RESOLVER_REAL_UPDATE_PITCH | RESOLVER_REAL_UPDATE_YAW));
}

void c_resolver::on_updated_real_angle(c_base_player* player, vec3 ang, i32 update_type, bool transmit){
  if(player == nullptr)
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  ang.y = math::normalize_angle(ang.y);

  if(update_type & RESOLVER_REAL_UPDATE_PITCH){
    data->real_ang.x = ang.x;
    if(!(data->real_ang_updated & RESOLVER_REAL_UPDATE_PITCH))
      DBG("[!] Updated real pitch angle to %2.2f\n", ang.x);

    if(transmit){
      c_party_chat_packet_resolver packet;
      packet.index                = player->get_index();
      packet.resolver_yaw_cycle   = data->resolver_yaw_cycle;
      packet.resolver_pitch_cycle = data->resolver_pitch_cycle;
      packet.response             = RESOLVER_RESPONSE_TYPE_REAL_PITCH;
      packet.real_ang.x           = ang.x;
      send_resolver_data(packet);
    }

    data->real_ang_updated |= RESOLVER_REAL_UPDATE_PITCH;
  }

  if(update_type & RESOLVER_REAL_UPDATE_YAW){
    data->real_ang.y = ang.y;

    if(!(data->real_ang_updated & RESOLVER_REAL_UPDATE_YAW))
      DBG("[!] Updated real yaw angle to %2.2f\n", ang.y);

    if(transmit){
      c_party_chat_packet_resolver packet;
      packet.index                = player->get_index();
      packet.resolver_yaw_cycle   = data->resolver_yaw_cycle;
      packet.resolver_pitch_cycle = data->resolver_pitch_cycle;
      packet.response             = RESOLVER_RESPONSE_TYPE_REAL_YAW;
      packet.real_ang.y           = ang.y;
      send_resolver_data(packet);
    }

    data->real_ang_updated |= RESOLVER_REAL_UPDATE_YAW;
  }
}

void c_resolver::on_invalidated_real_angle(c_base_player* player, c_player_data* data, i32 type){
  if(player == nullptr)
    return;

  if(data == nullptr)
    return;

  switch(type)
  {
    case 0:
    {
      if(data->real_ang_updated & RESOLVER_REAL_UPDATE_PITCH){
        DBG("[!] Invalidated real pitch angle.\n");
        data->real_ang_updated &= ~RESOLVER_REAL_UPDATE_PITCH;
      }

      break;
    }
    case 1:
    {
      if(data->real_ang_updated & RESOLVER_REAL_UPDATE_YAW){
        DBG("[!] Invalidated real yaw angle.\n");
        data->real_ang_updated &= ~RESOLVER_REAL_UPDATE_YAW;
      }
      break;
    }
    default:
    case 2:
    {
      DBG("[!] Invalidated all angles.\n");
      data->real_ang_updated = 0;
      break;
    }
  }
}

void c_resolver::detect_sniper_dot(){
  for(i32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* entity = global->entity_list->get_entity(i);
    if(entity ==  nullptr)
      continue;

    if(entity->is_dormant())
      continue;

    s_client_class* cc = entity->get_client_class();
    if(cc == nullptr)
      continue;

    if(cc->id != CSniperDot)
      continue;

    c_base_player* owner = entity->get_owner();
    if(owner == nullptr)
      continue;

    if(!owner->is_player())
      continue;

    if(owner->get_index() == global->engine->get_local_player())
      continue;

    c_player_data* data = owner->data();
    if(data == nullptr)
      continue;

    if(!should_run(owner))
      continue;

    vec3 real_ang      = math::calc_view_angle(owner->shoot_pos(), entity->obb_center());
    vec3 ang           = owner->viewangles();
    ang.x              = real_ang.x;

    on_updated_real_angle(owner, ang, RESOLVER_REAL_UPDATE_PITCH, false);
  }
}

void c_resolver::process_manual_input(){
  if(!is_manual_input_mode_active()){
    global->resolver_manual_target = -1;
    return;
  }

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_alive())
    return;

  static c_key_control select_player_key(&config->hvh.resolver_mc_sp_key, &config->hvh.resolver_mc_sp_single_click, nullptr, &config->hvh.resolver_mc_sp_double_click);
  static c_key_control cycle_key(&config->hvh.resolver_mc_up_key, &config->hvh.resolver_mc_up_single_click, nullptr, &config->hvh.resolver_mc_up_double_click);

  static bool select_player_key_active = false;
  static bool cycle_key_active         = false;

  c_base_player* current_target = global->entity_list->get_entity(global->resolver_manual_target);
  if(!is_valid_manual_target(current_target) && global->resolver_manual_target != -1)
    global->resolver_manual_target = -1;

  if(select_player_key.is_toggled()){
    if(!select_player_key_active){

      if(find_manual_yaw_target()){
        c_base_player* player = global->entity_list->get_entity(global->resolver_manual_target);
        if(player != nullptr){
          //if(config->misc.chat_notification_resolver){
          //  char buf[128];
          //  wsprintfA(buf, XOR("Manual target set to player \"\x7%06X%s\x1\""), HEX_ORANGE, player->info().name);
          //  utils::send_nofitication(buf);
          //}
        }
      }
      
      select_player_key_active = true;
    }
  }
  else
    select_player_key_active = false;

  if(cycle_key.is_toggled()){
    if(!cycle_key_active){

      c_base_player* player = global->entity_list->get_entity(global->resolver_manual_target);
      if(player != nullptr){
        c_player_data* data = player->data();
        if(data != nullptr){

          std::wstring what_changed;
          if(data->real_ang_updated & RESOLVER_REAL_UPDATE_YAW)
            data->real_ang_updated &= ~RESOLVER_REAL_UPDATE_YAW;

          data->resolver_yaw_cycle++;
          if(data->resolver_yaw_cycle >= RESOLVER_MAX_YAW_CYCLE)
            data->resolver_yaw_cycle = 0;

          //if(config->misc.chat_notification_resolver){
          //  char buf[128];
          //  wsprintfA(buf, XOR("\"\x7%06X%s\x1\" cycling to correction \"\x7%06X%ls\x1\""), HEX_ORANGE, player->info().name, HEX_GRAY, get_current_yaw_correction_str(player).c_str());
          //  utils::send_nofitication(buf);
          //}
        }
      }

      cycle_key_active = true;
    }
  }
  else
    cycle_key_active = false;
}

bool c_resolver::find_manual_yaw_target(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  i32 current_target_index = global->resolver_manual_target;

  // Find a manual yaw cycle target that is valid and closest to our crosshair.
  float best_fov = -1.f;
  for(i32 i = 1; i <= globaldata->max_clients; i++){
    c_base_player* player = global->entity_list->get_entity(i);
    if(player == nullptr)
      continue;

    if(!is_valid_manual_target(player))
      continue;

    float fov = math::get_fov_delta(global->untouched_cmd.view_angles, localplayer->shoot_pos(), player->shoot_pos());
    if(fov > best_fov && best_fov != -1.f)
      continue;

    best_fov = fov;
    global->resolver_manual_target = i;
  }

  return global->resolver_manual_target != -1 && global->resolver_manual_target != current_target_index;
}

bool c_resolver::is_valid_manual_target(c_base_player* player){
  if(player == nullptr)
    return false;

  if(player->get_index() == global->localplayer_index)
    return false;

  if(player->team() == global->localplayer_team)
    return false;

  if(!player->is_alive())
    return false;

  c_player_data* data = player->data();
  if(data == nullptr)
    return false;

  if(!is_resolver_active(player))
    return false;

  return true;
}

bool c_resolver::is_manual_input_mode_active(){
  if(!config->hvh.resolver || !config->hvh.resolver_allow_manual_cycle)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  // Only run the resolver when we're using a headshot weapon.
  if(config->hvh.resolver_headshot_weponly && !weapon->can_headshot())
    return false;

  return true;
}

std::wstring c_resolver::get_current_pitch_correction_str(c_base_player* player){
  if(player == nullptr)
    return L"";

  c_player_data* data = player->data();
  if(data == nullptr)
    return L"";

  if(is_playerlist_correcting(RESOLVER_MANUAL_PITCH, player)){
    if(data->playerlist.resolver_pitch_auto)
      return WXOR(L"Auto");
    else if(data->playerlist.resolver_pitch_up)
      return WXOR(L"Up");
    else if(data->playerlist.resolver_pitch_down)
      return WXOR(L"Down");
    else if(data->playerlist.resolver_pitch_zero)
      return WXOR(L"Zero");
  }
  else{
    switch(data->resolver_pitch_cycle){
      default: break;
      case 0: return WXOR(L"None");
      case 1: return WXOR(L"Invert angles");
      case 2: return WXOR(L"Zero");
      case 3: return WXOR(L"-45");
    }
  }

  return L"";
}

std::wstring c_resolver::get_current_yaw_correction_str(c_base_player* player){
  if(player == nullptr)
    return L"";

  c_player_data* data = player->data();
  if(data == nullptr)
    return L"";

  if(is_playerlist_correcting(RESOLVER_MANUAL_YAW, player)){

    if(data->playerlist.resolver_yaw_auto)
      return WXOR(L"Auto");
    else if(data->playerlist.resolver_yaw_neg90)
      return WXOR(L"-90");
    else if(data->playerlist.resolver_yaw_pos90)
      return WXOR(L"+90");
    else if(data->playerlist.resolver_yaw_neg45)
      return WXOR(L"-45");
    else if(data->playerlist.resolver_yaw_pos45)
      return WXOR(L"+45");
    else if(data->playerlist.resolver_yaw_lookat)
      return WXOR(L"Look at");
    else if(data->playerlist.resolver_yaw_180away)
      return WXOR(L"Look away");

  }
  else{
    switch(data->resolver_yaw_cycle){
      default: break;
      case 0: return WXOR(L"-90");
      case 1: return WXOR(L"-45");
      case 2: return WXOR(L"+90");
      case 3: return WXOR(L"+45");
      case 4: return WXOR(L"Look at");
    }
  }

  return L"";
}

bool c_resolver::is_resolver_active(c_base_player* player){
  if(player == nullptr)
    return false;

  if(!should_run(player))
    return false;

  c_player_data* data = player->data();
  if(data == nullptr)
    return false;

  if(data->playerlist.resolver_disallow)
    return false;

  if(data->playerlist.resolver_force)
    return true;

  return data->resolver_yaw_active_time > math::time() || math::abs(player->viewangles().x) >= 90.f;
}

bool c_resolver::is_shot_accurate(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(weapon->is_sniper_rifle())
    return true;

  if(!weapon->deals_bullet_dmg())
    return false;

  if(weapon->get_spread() <= 0.f)
    return true;

  return weapon->is_perfect_shot();
}

// is_resolver_active tells us if pitch or yaw resolver is active.
// This function is designed to check whether or not we should even run in the first place on this player.
bool c_resolver::should_run(c_base_player* player){
  if(utils::is_playing_mvm())
    return false;

  if(player == nullptr)
    return false;

  if(!player->is_valid())
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(localplayer->team() == player->team())
    return false;

  c_player_data* data = player->data();
  if(data == nullptr)
    return false;

  // Force the resolver to be enabled for this player.
  if(data->playerlist.resolver_force)
    return true;

  // We'll assume that our friend is a cheater.
  if(config->hvh.resolver_cheatersonly && !(cheat_detection->is_cheating(player->get_index()) || player->is_steam_friend()))
    return false;

  // Only run the resolver when we're using a headshot weapon.
  if(config->hvh.resolver_headshot_weponly && !weapon->can_headshot())
    return false;

  return true;
}

void c_resolver::check_movement(c_base_player* player, c_player_data* data){
  if(player == nullptr || data == nullptr)
    return;

   if(!player->is_scoped() && !(player->entity_flags() & FL_DUCKING)){

      s_lc_data* lc = lag_compensation->get_data(player);
      if(lc == nullptr)
        return;

      vec3 avg_vel;
      u32  collected = 0;

      // Get an average of the velocity.
      for(u32 i = 1; i < lc->max_records; i++){

        s_lc_record* record1 = lc->get_record(i - 1);
        s_lc_record* record2 = lc->get_record(i);
        if(record1 == nullptr || record2 == nullptr)
          continue;

        float time = record1->simulation_time - record2->simulation_time;

        // Only get an average of 200ms worth of data.
        if(math::abs(record1->simulation_time - player->simulation_time()) > 0.2f)
          break;

        vec3  delta = record1->origin - record2->origin;
        for(u32 j = 0; j <= 3; j++){
          if(delta[j] != 0.0f)
            delta[j] /= time;
        }

        avg_vel += delta;
        collected++;
      }

      if(collected > 0)
        avg_vel /= collected;

      vec3 vel             = avg_vel;
      float speed          = math::round(math::clamp(vel.length(), 0.f, player->max_speed()));
      float backward_speed = player->max_speed() * 0.90f;

      if(player->max_speed() > 1.f && speed > 1.f){
        if(speed >= (backward_speed - 2.f) && speed <= (backward_speed + 10.f)){
          data->resolver_movement_factor++;
          if(data->resolver_movement_factor >= 12){

            vec3 movement_angle;
            math::vector_2_angle(&vel, &movement_angle);
            movement_angle.x = player->viewangles().x;


            on_updated_real_angle(player, movement_angle, RESOLVER_REAL_UPDATE_YAW, false);
            data->resolver_movement_factor  = 0;
          }
        }
        else
          data->resolver_movement_factor = 0;
      }
   }
}

void c_resolver::send_resolver_data(c_party_chat_packet_resolver packet){
  if(!config->hvh.resolver || !config->misc.party_network || !config->misc.party_network_resolver)
    return;

  if(!party_network->should_transmit())
    return;

  packet.transmit();
  DBG("[!] Transmit resolver data. Response: %i\n", packet.response);
}

void c_resolver::read_resolver_data(c_party_chat_packet_resolver* resolver_data){
  if(!config->hvh.resolver || !config->misc.party_network || !config->misc.party_network_resolver)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(resolver_data == nullptr){
    DBG("[-] resolver_data nullptr\n");
    return;
  }

  c_base_player* sender_player = global->entity_list->get_entity(resolver_data->sender);
  if(sender_player == nullptr){
    DBG("[-] sender_player nullptr\n");
    return;
  }

  // This is sent every second to let us know they're broadcasting resolver data.
  if(resolver_data->response == RESOLVER_RESPONSE_TYPE_BROADCASTING){
    c_player_data* data = utils::get_player_data(resolver_data->sender);
    if(data != nullptr){
      DBG("[!] %i is sending resolver data.\n", resolver_data->sender);
      data->last_resolver_data_msg_time = math::time();
    }
    return;
  }

  if(resolver_data->index == global->engine->get_local_player()){
    DBG("[!] Received resolver data about ourself!\n");
    return;
  }

  if(resolver_data->index < 0 || resolver_data->index >= MAXPLAYERS){
    DBG("[-] Invalid resolver data entity index.\n");
    return;
  }

  c_base_player* player = global->entity_list->get_entity(resolver_data->index);
  if(player == nullptr){
    DBG("[-] Received entity index %i but entity is nullptr.\n", resolver_data->index);
    return;
  }

  // Don't bother we people on our team.
  if(player->team() == localplayer->team()){
    DBG("[-] Received entity index %i but entity is on our team", resolver_data->index);
    return;
  }

  c_player_data* data = player->data();
  if(data == nullptr){
    DBG("[-] Resolver data entity player data is nullptr.\n");
    return;
  }

  if(!should_run(player)){
    DBG("[!] Resolver data should_run is false on player %s \n", player->info().name);
    return;
  }

  switch(resolver_data->response){
    default:
    {
      DBG("[-] Unknown resolver data response: %i from sender: %i\n", resolver_data->response, resolver_data->sender);
      break;
    }
    case RESOLVER_RESPONSE_TYPE_PITCH_MISS:
    {
      if(resolver_data->resolver_pitch_cycle < 0 && resolver_data->resolver_pitch_cycle >= RESOLVER_MAX_PITCH_CYCLE){
        DBG("[-] Received invalid pitch cycle data from sender: %i\n", resolver_data->sender);
        return;
      }

      if(data->resolver_pitch_cycle == resolver_data->resolver_pitch_cycle){
        data->resolver_pitch_cycle++;
        if(data->resolver_pitch_cycle >= RESOLVER_MAX_PITCH_CYCLE)
          data->resolver_pitch_cycle = 0;

        //if(config->misc.chat_notification_resolver){
        //  char buf[128];
        //  wsprintfA(buf, XOR("\"\x7%06X%s\x1\" missed \"\x7%06X%s\x1\" with the pitch resolver on the same cycle. Moving ahead 1 cycle!"), HEX_ORANGE, sender_player->info().name, HEX_ORANGE, player->info().name);
        //  utils::send_nofitication(buf);
        //}
      }

      break;
    }
    case RESOLVER_RESPONSE_TYPE_YAW_MISS:
    {
      if(resolver_data->resolver_yaw_cycle < 0 && resolver_data->resolver_pitch_cycle >= RESOLVER_MAX_YAW_CYCLE){
        DBG("[-] Received invalid yaw cycle data from sender: %i\n", resolver_data->sender);
        return;
      }

      if(data->resolver_yaw_cycle == resolver_data->resolver_yaw_cycle){
        data->resolver_yaw_cycle++;
        if(data->resolver_yaw_cycle >= RESOLVER_MAX_YAW_CYCLE)
          data->resolver_yaw_cycle = 0;

        //if(config->misc.chat_notification_resolver){
        //  char buf[128];
        //  wsprintfA(buf, XOR("\"\x7%06X%s\x1\" missed \"\x7%06X%s\x1\" with the yaw resolver on the same cycle. Moving ahead 1 cycle!"), HEX_ORANGE, sender_player->info().name, HEX_ORANGE, player->info().name);
        //  utils::send_nofitication(buf);
        //}
      }

      break;
    }
    case RESOLVER_RESPONSE_TYPE_REAL_PITCH:
    {
      if(math::abs(resolver_data->real_ang.x) >= 180.f){
        DBG("[-] Received invalid real pitch angle %2.2f from sender: %i\n", resolver_data->real_ang.x, resolver_data->sender);
        return;
      }

     //if(config->misc.chat_notification_resolver){
     //  char buf[128];
     //  wsprintfA(buf, XOR("\"\x7%06X%s\x1\" sent us real pitch angle data on \"\x7%06X%s\x1\"!"), HEX_ORANGE, sender_player->info().name, HEX_ORANGE, player->info().name);
     //  utils::send_nofitication(buf);
     //}

      on_updated_real_angle(player, resolver_data->real_ang, RESOLVER_REAL_UPDATE_PITCH, false);
      break;
    }
    case RESOLVER_RESPONSE_TYPE_REAL_YAW:
    {
      if(math::abs(resolver_data->real_ang.y) >= 1080.f){
        DBG("[-] Received invalid real yaw angle %2.2f from sender: %i\n", resolver_data->real_ang.x, resolver_data->sender);
        return;
      }

      //if(config->misc.chat_notification_resolver){
      //  char buf[128];
      //  wsprintfA(buf, XOR("\"\x7%06X%s\x1\" sent us real yaw angle data on \"\x7%06X%s\x1\"!"), HEX_ORANGE, sender_player->info().name, HEX_ORANGE, player->info().name);
      //  utils::send_nofitication(buf);
      //}

      on_updated_real_angle(player, resolver_data->real_ang, RESOLVER_REAL_UPDATE_YAW, false);
      break;
    }
  }
}

bool c_resolver::is_playerlist_correcting(u8 angle_type, c_base_player* player){
  if(player == nullptr)
    return false;

  if(!should_run(player))
    return false;

  c_player_data* data = player->data();
  if(data == nullptr)
    return false;


  if(angle_type == RESOLVER_MANUAL_PITCH){
    if(data->playerlist.resolver_pitch_auto)
      return false;

    return true;
  } 
  else if(angle_type == RESOLVER_MANUAL_YAW){
    if(data->playerlist.resolver_yaw_auto)
      return false;

    return true;
  }

  return false;
}

bool c_resolver::is_good_record(c_base_player* player, s_lc_record* record){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(player == nullptr || record == nullptr)
    return false;

  if(!is_resolver_active(player))
    return true;

  c_player_data* data = player->data();
  if(data == nullptr)
    return true;

  vec3 viewangle = data->networked_ang;
  if(!data->has_received_network_ang)
    viewangle = player->viewangles();

  // Try and detect on shot normally.
  float fov = math::get_fov_delta(viewangle, player->shoot_pos(), localplayer->shoot_pos());
  if(fov < 10.f)
    return true;

  return false;
}