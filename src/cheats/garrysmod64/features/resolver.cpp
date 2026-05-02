#include "../link.h"

CLASS_ALLOC(c_resolver, resolver);

void c_resolver::on_shot(c_base_player* player){
  if(!config->hvh.resolver)
    return;

  if(player == nullptr)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  // We got a shot queued wait until we hit them or miss them.
  if(data->shot_data.timeout > 0.f)
    return;

  // Make sure everything is reset to prevent any issues.
  memset(&data->shot_data, 0, XOR32(sizeof(s_resolver_shot_data)));

  data->shot_data.should_run = should_run(player);
  if(!data->shot_data.should_run)
    return;

  // By ping plus 1 tick.
  float ping         = math::clamp(utils::get_latency(), 0.2f, 1.f);
  data->shot_data.timeout             = (math::time() + ping) + math::ticks_to_time(1);
  data->shot_data.using_pitch_antiaim = !data->playerlist.resolver_disallow && is_using_pitch_aa(player);
  data->shot_data.using_yaw_antiaim   = data->resolver_yaw_active_time > math::time();
  data->shot_data.real_ang_updated    = data->real_ang_updated;
  DBG("[!] Processing shot for %s\n", player->info().name);
  data->resolver_miss_counter++;
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
  kernel_time_expired_return();
  if(!config->hvh.resolver)
    return;

  if(stage != FRAME_RENDER_START)
    return;

  float time = math::time();
  for(i32 i = 1; i <= globaldata->max_clients; i++){
    c_base_player* player = global->entity_list->get_entity(i);
    if(player == nullptr)
      continue;

    c_player_data* data = player->data();
    if(data == nullptr)
      continue;

    if(!data->shot_data.should_run){
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

    if(data->resolver_miss_counter <= 0)
      continue;

    data->resolver_miss_counter = 0;

    wchar_t buf[128];
    wchar_t player_name[128];
    convert::str2wstr(player->info().name, player_name, sizeof(player_name));

    // Process pitch cycle
    if(using_pitch_antiaim && !is_playerlist_correcting(RESOLVER_MANUAL_PITCH, player)){
      if(!real_pitch_collected){ // Only cycle pitch resolver if don't know their real angle.
        data->resolver_pitch_cycle++;

        // We have tried every pitch angle lets reset it and cycle the player's cycle angle.
        if(data->resolver_pitch_cycle >= RESOLVER_MAX_PITCH_CYCLE){
          allow_yaw_cycle            = true;
          data->resolver_pitch_cycle = 0;
        }


        // Invalidate any real pitch angle we got.
        on_invalidated_real_angle(player, data, RESOLVER_INVALIDATE_PITCH);
      }
      else
        allow_yaw_cycle = true; // We hopefully have their real pitch angle lets allow yaw cycle.

      // Handle notifications for the pitch resolver.
      {
        if(true){
          wchar_t buf[128];
          if(!real_pitch_collected)
            wsprintfW(buf, WXOR(L"Cycling pitch corrections on player \"%ls\""), player_name);
          else
            wsprintfW(buf, WXOR(L"Used last (hit/detected) real pitch angle \"%ls\""), player_name);

          notify->create(NOTIFY_TYPE_INFO, buf);
        }
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
      if(true){
        wsprintfW(buf, WXOR(L"Used last (hit/detected) real yaw angle \"%ls\""), player_name);
        notify->create(NOTIFY_TYPE_INFO, buf);
      }

      // Invalidate any real yaw angle we got on this player.
      on_invalidated_real_angle(player, data, RESOLVER_INVALIDATE_YAW);
      continue;
    }

    data->resolver_yaw_cycle++;
    DBG("[!] Cycling yaw to %i\n", data->resolver_yaw_cycle);

    // Handle notifications for the yaw resolver.
    {
      if(true){
        wsprintfW(buf, WXOR(L"Cycling yaw correction on player \"%ls\""), player_name);
        notify->create(NOTIFY_TYPE_INFO, buf);
      }
    }

    if(data->resolver_yaw_cycle >= RESOLVER_MAX_YAW_CYCLE){
      // Darn! Lets make sure the pitch resolver has a chance to go through everything again...
      on_invalidated_real_angle(player, data, RESOLVER_INVALIDATE_PITCH);
      data->resolver_yaw_cycle = 0;
    }

    // Invalidate any real yaw angle we got on this player.
    on_invalidated_real_angle(player, data, RESOLVER_INVALIDATE_YAW);
  }
}

void c_resolver::apply_angles(void* ref, float* pitch, float* yaw){
  if(!config->hvh.resolver)
    return;

  // Moved to call to FSN
  // process_shots(stage);

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

    if(player != ref)
      continue;

    if(!should_run(player))
     continue;

    c_player_data* data = player->data();
    if(data == nullptr)
      continue;

    detect_yaw_antiaim(player);
    vec3 viewangle = data->networked_ang;

    // Since we're applying angle changes in the animstate update function we don't need to worry about backing up the angles from the float decode function.
    {
      viewangle           = player->viewangles();
      data->networked_ang = player->viewangles();
      data->real_ang      = player->viewangles();
    }

    bool using_pitch_antiaim    = !data->playerlist.resolver_disallow && is_using_pitch_aa(player);
    bool using_yaw_antaim       = data->resolver_yaw_active_time > time;

    vec3 view = math::calc_view_angle(player->shoot_pos(), localplayer->shoot_pos());
    float best_dist = 99999.f;
    for(u32 j = 1; j <= globaldata->max_clients; j++){
      c_base_player* near_player = global->entity_list->get_entity(j);
      if(near_player == nullptr)
        continue;

      if(near_player == player)
        continue;

      if(!near_player->is_valid())
        continue;

      float dist = (player->obb_center() - near_player->obb_center()).length();
      if(dist > best_dist)
        continue;

      view = math::calc_view_angle(player->shoot_pos(), near_player->shoot_pos());
      best_dist = dist;
    }
    

    bool real_pitch_collected = data->real_ang_updated & RESOLVER_REAL_UPDATE_PITCH;
    bool real_yaw_collected   = data->real_ang_updated & RESOLVER_REAL_UPDATE_YAW;

    bool allow_pitch_corrections = using_pitch_antiaim;
    bool allow_yaw_corrections   = using_yaw_antaim;

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
            default:
            {
              //DBG("[-] There's no pitch cycle setup, so if you're missing every shot. Then maybe, they have fake pitch? idk")
              break;
            }
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
        switch(data->resolver_yaw_cycle){
          default: break;
          case 1: viewangle.y += 180.f; break;
          case 2: viewangle.y = view.y - 90.f; break;
          case 3: viewangle.y = view.y + 90.f; break;
        }    
      }
    }

    *pitch = viewangle.x;
    *yaw   = viewangle.y;
    break;
  }
}

void c_resolver::fire_event(c_game_event* event){
  if(!config->hvh.resolver)
    return;

  if(event == nullptr)
    return;

  i32 attacker  = global->engine->get_player_for_user_id(event->get_int(XOR("attacker")));
  i32 victim    = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));

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

  //if(data->shot_data.timeout <= 0.f){
  //  DBG("[!] We hit something but we aren't processing this shot.\n");
  //  return;
  //}

  reset_shot_data(player, true);
  DBG("[!] Hit\n");

}

void c_resolver::reset_shot_data(c_base_player* player, bool update_real_angle){
  if(player == nullptr)
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  data->shot_data.timeout = -1.f;
  if(update_real_angle){
    data->resolver_miss_counter = 0;
    on_updated_real_angle(player, player->viewangles(), (RESOLVER_REAL_UPDATE_PITCH | RESOLVER_REAL_UPDATE_YAW));
  }
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

    data->real_ang_updated |= RESOLVER_REAL_UPDATE_PITCH;
  }

  if(update_type & RESOLVER_REAL_UPDATE_YAW){
    data->real_ang.y = ang.y;

    if(!(data->real_ang_updated & RESOLVER_REAL_UPDATE_YAW))
      DBG("[!] Updated real yaw angle to %2.2f\n", ang.y);

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
          wchar_t player_name[128];
          convert::str2wstr(player->info().name, player_name, sizeof(player_name));

          if(true){
            wchar_t buf[128];
            wsprintfW(buf, WXOR(L"Manual target set to player \"%ls\""), player_name);
            notify->create(NOTIFY_TYPE_INFO, buf);
          }
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

          if(true){
            wchar_t player_name[128];
            convert::str2wstr(player->info().name, player_name, sizeof(player_name));

            wchar_t buf[128];
            wsprintfW(buf, WXOR(L"\"%ls\" cycling to correction \"%ls\""), player_name, get_current_yaw_correction_str(player).c_str());
            notify->create(NOTIFY_TYPE_INFO, buf);
          }
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

  if(player->team == global->localplayer_team)
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
      case 0: return WXOR(L"None");
      case 1: return WXOR(L"Rotation");
      case 2: return WXOR(L"-90");
      case 3: return WXOR(L"+90");
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

  return data->resolver_yaw_active_time > math::time();
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

  return true;
}

// is_resolver_active tells us if pitch or yaw resolver is active.
// This function is designed to check whether or not we should even run in the first place on this player.
bool c_resolver::should_run(c_base_player* player){
  if(player == nullptr)
    return false;

  if(!player->is_valid())
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(player == localplayer)
    return false;

  c_player_data* data = player->data();
  if(data == nullptr)
    return false;

  // Force the resolver to be enabled for this player.
  if(data->playerlist.resolver_force)
    return true;

  // We'll assume that our friend is a cheater.
  if(config->hvh.resolver_cheatersonly && !(cheat_detection->is_cheating(player) || localplayer->is_steam_friend(player)))
    return false;

  return true;
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