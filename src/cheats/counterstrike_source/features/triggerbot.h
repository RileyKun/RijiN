#pragma once

class c_triggerbot{
public:
  float last_valid_target_time;
  float wait_to_fire_time;
  float fire_for_time;
  i32   current_target_index;
  i32   last_target_index;

  ALWAYSINLINE void on_invalid_entity(){
    float time = math::time();

    current_target_index = 0;
    if(last_valid_target_time <= time)
      last_target_index = 0;
  }

  ALWAYSINLINE void reset(){
    memset(this, 0, sizeof(c_triggerbot));
  }

  bool is_valid_target(c_base_player* localplayer, c_base_entity* entity){
    if(entity == nullptr)
      return false;

    if(!entity->is_player())
      return false;

    c_base_player* p = (c_base_player*)entity;
    if(localplayer == p)
      return false;

    if(!p->is_alive())
      return false;

    if(p->is_dormant())
      return false;

    if(p->is_considered_spawn_protected() && global->triggerbot_settings->ignore_spawn_protected)
      return false;

    if(localplayer->team() == entity->team() && global->triggerbot_settings->target_friendlies)
      return true;

    if(localplayer->team() != entity->team() && global->triggerbot_settings->target_enemies)
      return true;

    if(p->is_steam_friend() && global->triggerbot_settings->target_friends)
      return true;

    return false;
  }

  ALWAYSINLINE bool is_targeting_hitbox(i32 hitbox){
    if(global->triggerbot_settings == nullptr)
      return false;
    
    if(global->triggerbot_settings->target_everything)
      return true;

    if(global->triggerbot_settings->target_head && hitbox == 12)
      return true;

    if(global->triggerbot_settings->target_body && (hitbox == 0 || hitbox == 10 || hitbox == 11))
      return true;

    if(global->triggerbot_settings->target_head_and_body && (hitbox >= 10 && hitbox <= 12 || hitbox == 0))
      return true;

    return false;
  }

  NEVERINLINE s_trace do_trace(vec3 start_pos, vec3 end_pos, vec3 fwd){
    s_trace tr = global->trace->ray(start_pos, end_pos, mask_shot_hitbox);
    tr = global->trace->ray_clip_to_player(start_pos, end_pos + fwd * 40.f, mask_shot_hitbox, &tr);
    return tr;
  }

  bool find_best_seed(c_base_player* localplayer, vec3 start_pos, float dist){
    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return false;

    float spread = wep->get_spread();
    if(spread <= 0.f)
      return false;

    float cone = wep->get_inaccuracy();

    i32 command_number = command_manager->get_previous_command();
    i32 c = 0;
    while(c < 255){
      i32 seed = math::md5_pseudo_random(command_number) & INT_MAX;
      math::set_random_seed((seed & 255) + 1);

      float source_pi = 6.2831855f;

      float a0 = math::random_float(0.0f, source_pi);
      float r0 = math::random_float(0.0f, spread);
      float a1 = math::random_float(0.0f, source_pi);
      float r1 = math::random_float(0.0f, cone);

      float x0 = r0 * math::cos(a0);
      float y0 = r0 * math::sin(a0);

      float x1 = r1 * math::cos(a1);
      float y1 = r1 * math::sin(a1);

      float xx = x0 + x1;
      float yy = y0 + y1;

      vec3 fwd, right, up;
      math::angle_2_vector(global->untouched_cmd.view_angles + recoil_control->get_uncompensated_view_punch(), &fwd, &right, &up);

      vec3 angles;
      vec3 spread_recalculated = fwd + (right * xx) + (up * yy);
      spread_recalculated.normalize();
      math::vector_2_angle(&spread_recalculated, &angles);
      math::angle_2_vector(angles, &fwd, nullptr, nullptr);

      vec3 end_pos = start_pos + fwd * dist;

      s_trace tr = do_trace(start_pos, end_pos, fwd);
      if(tr.hit_entity != nullptr){
        if(raytrace_aimbot->is_blocked_by_smoke(tr.end) && global->triggerbot_settings->ignore_players_through_smoke)
          return false;

        if(is_valid_target(localplayer, tr.hit_entity) && is_targeting_hitbox(tr.hitbox)){
          c_base_player* player = (c_base_player*)tr.hit_entity;

          interp_manager->set_tick_count_with_interp(player->simulation_time(), player->get_index(), false);

          // For legit mode we'll want to override the random seed of the nospread with the org cmdnum.
          bool success = command_manager->set_command(command_number);

          DBG("[!] set cmd: %i (%i)\n", command_number, success);
          current_target_index = player->get_index();
          return true;
        }
      }

      if(global->triggerbot_settings->spread_mode_legit)
        break;

      command_number++;
      c++;
    }

    return false;
  }

  bool can_run(c_base_player* localplayer){
    if(localplayer == nullptr)
      return false;

    if(!localplayer->is_alive())
      return false;

    if(!localplayer->can_fire())
      return false;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return false;

    if(wep->is_melee() || wep->is_grenade())
      return false;

    if(wep->clip1() <= 0)
      return false;

    css_weapon_file_info* weapon_info = wep->get_weapon_info();
    if(weapon_info == nullptr)
      return false;

    return true;
  }

  bool run(){
    if(global->current_cmd == nullptr)
      return false;

    if(global->triggerbot_settings == nullptr)
      return false;

    if(!global->triggerbot_settings->enabled)
      return false;

    if(global->triggerbot_settings->trigger_mode_on_key){
      if(!input_system->held(global->triggerbot_settings->trigger_hotkey))
        return false;
    }

    float time = math::time();
    c_base_player* localplayer = utils::localplayer();
    if(!can_run(localplayer)){
      on_invalid_entity();
      return false;
    }

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return false;

    css_weapon_file_info* weapon_info = wep->get_weapon_info();
    if(weapon_info == nullptr)
      return false;

    vec3 fwd;
    math::angle_2_vector(global->untouched_cmd.view_angles + recoil_control->get_uncompensated_view_punch(), &fwd, nullptr, nullptr);

    vec3 start_pos = localplayer->shoot_pos();

    // Continue firing for X amount of time.
    // Should also note when this is active spread mode triggerbot will not run there for making the triggerbot inaccurate.
    // This is intentional to a point.
    if(global->triggerbot_settings->trigger_fire_for_time > 0.0f && fire_for_time > time && wait_to_fire_time <= time){
      if(global->untouched_cmd.buttons & IN_ATTACK || global->untouched_cmd.buttons & IN_RELOAD || wep->clip1() == 0) // If we're manually firing go ahead and turn it off.
        fire_for_time = -1.f;

      global->info_panel->add_entry(INFO_PANEL_TRIGGERBOT, WXOR(L"TRIGGERBOT"), WXOR(L"FIRE FOR TIME"), colour(255, 255, 0));
      global->current_cmd->buttons |= IN_ATTACK;
      return true;
    }

    if(global->triggerbot_settings->nospread_legit_mode){
      if(!find_best_seed(localplayer, start_pos, weapon_info->range)){
        on_invalid_entity();
        return false;
      }
    }
    else{
      vec3 end_pos   = localplayer->shoot_pos() + fwd * weapon_info->range;
      s_trace tr     = do_trace(start_pos, end_pos, fwd);

      if(!is_valid_target(localplayer, tr.hit_entity) || !is_targeting_hitbox(tr.hitbox)){
        on_invalid_entity();
        return false;
      }

      if(raytrace_aimbot->is_blocked_by_smoke(tr.end) && global->triggerbot_settings->ignore_players_through_smoke)
        return false;

      c_base_player* player = (c_base_player*)tr.hit_entity;
      interp_manager->set_tick_count_with_interp(player->simulation_time(), player->get_index(), false);
      current_target_index = player->get_index();
    }

    global->info_panel->add_entry(INFO_PANEL_TRIGGERBOT, WXOR(L"TRIGGERBOT"), global->triggerbot_settings->nospread_legit_mode ? WXOR(L"SPREAD MODE") : WXOR(L"NORMAL"), colour(0, 255, 32, 255));

    if(last_target_index != current_target_index){
      // Setup fire delay.
      if(global->triggerbot_settings->trigger_fire_delay > 0.0f)
        wait_to_fire_time = time + global->triggerbot_settings->trigger_fire_delay / 1000.f;

      // Setup fire for time.
      if(global->triggerbot_settings->trigger_fire_for_time > 0.0f){
        fire_for_time = time + global->triggerbot_settings->trigger_fire_for_time / 1000.f;
        if(wait_to_fire_time > time) // If there's any fire delay add the day on to the fire time.
          fire_for_time += global->triggerbot_settings->trigger_fire_delay / 1000.f;
      }
    }

    if(global->triggerbot_settings->trigger_fire_delay <= 0.0f || wait_to_fire_time <= time)
      global->current_cmd->buttons |= IN_ATTACK;
    else
       global->info_panel->add_entry(INFO_PANEL_TRIGGERBOT, WXOR(L"TRIGGERBOT"), WXOR(L"FIRE DELAY"), rgb(255, 255, 0));

    last_target_index      = current_target_index;

    float valid_time = 0.2f;
    if(global->triggerbot_settings->trigger_fire_delay > 0.0f)
      valid_time += global->triggerbot_settings->trigger_fire_delay / 1000.f;

    if(global->triggerbot_settings->trigger_fire_for_time > 0.0f)
      valid_time += global->triggerbot_settings->trigger_fire_for_time / 1000.f;

    last_valid_target_time = time + valid_time;
    return true;
  }
};

CLASS_EXTERN(c_triggerbot, triggerbot);