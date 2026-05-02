#pragma once

class c_raytrace_aimbot{
public:
  c_aimbot_filter_base* filter = nullptr;

  ALWAYSINLINE void destroy_aimbot_filter(){
    if(filter == nullptr)
      return;

    delete filter;
    filter = nullptr;
  }

  ALWAYSINLINE void set_aimbot_filter(){
    destroy_aimbot_filter();

    if(global->aimbot_settings->target_method_closest_to_fov)
      filter = new c_aimbot_filter_raytrace_fov;
    else if(global->aimbot_settings->target_method_distance)
      filter = new c_aimbot_filter_raytrace_distance;
  }

  ALWAYSINLINE bool should_target_entity(c_base_entity* entity){
    if(global->aimbot_settings == nullptr)
      return false;
    
    if(entity == nullptr)
      return false;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return false;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return false;

    if(entity->is_player()){
      c_base_player* player = (c_base_player*)entity;
      if(!player->is_valid())
        return false;

      if(!localplayer->is_considered_enemy(player))
        return false;

      if(player->is_considered_spawn_protected() && global->aimbot_settings->ignore_spawn_protected)
        return false;

      c_player_data* data = player->data();
      if(data == nullptr)
        return false;

      if(data->playerlist.ignore_player)
        return false;

      return true;
    }

    return false;
  }

  ALWAYSINLINE i32 get_priority_hitbox(){
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return -1;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return -1;

    i32 priority_hitbox = -1;
    if(global->aimbot_settings->closest_hitbox_to_crosshair)
      return -2;

    for(i32 i = 0; i < sizeof(global->aimbot_settings->priority_hitbox) / sizeof(bool); i++){
      if(global->aimbot_settings->priority_hitbox[i]){
        priority_hitbox = i;
        break;
      }
    }


    // Config error fix.
    if(priority_hitbox == -1)
      global->aimbot_settings->priority_hitbox[12] = true;

    return priority_hitbox;
  }

  ALWAYSINLINE bool should_use_hitbox(i32 hitbox){
    if(hitbox > sizeof(global->aimbot_settings->hitbox) / sizeof(bool))
      return false;

    return global->aimbot_settings->hitbox[hitbox];
  }

  bool get_best_hitbox_point(i32 hitbox, vec3 shoot_pos, c_base_entity* target, s_lc_record* lc_record, vec3& aim_point){
    if(target == nullptr)
      return false;

    if(target->is_player()){
      aim_point = target->obb_center();
      if(lc_record != nullptr) // We can get error models. Don't fail if invalid just aim at obb center.
        lc_record->get_hitbox_pos((c_base_player*)target, hitbox, aim_point);
    }
    else // World objects for now aim at OBB center.
      aim_point = target->obb_center();

    // Are they in our fov?
    if(global->aimbot_settings->max_fov > 0 && global->aimbot_settings->max_fov < 180)
      if(math::get_fov_delta(global->untouched_cmd.view_angles, shoot_pos, aim_point) > (float)global->aimbot_settings->max_fov)
        return false;


    if(is_blocked_by_smoke(aim_point) && global->aimbot_settings->ignore_players_through_smoke)
      return false;

    s_trace tr = global->trace->ray(shoot_pos, aim_point, mask_shot);
    if(tr.vis(target))
      return true;

    // Autowall
    if(global->aimbot_settings->autowall_enabled){
      if(autowall->simulate(shoot_pos, aim_point)){
        float wish_damage_frac  = math::clamp(global->aimbot_settings->autowall_min_dmg / 100.f, 0.f, 1.f);
        float total_frac        = math::clamp(autowall->dump.damage / (float)autowall->dump.hit_entity->health(), 0.f, 1.f);

        if(total_frac >= wish_damage_frac)
          return true;
      }
    }

    return false;
  }

  bool get_best_aim_point(c_base_entity* target, s_lc_record* lc_record, vec3& aim_point){
    if(target == nullptr)
      return false;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return false;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return false;

    vec3 shoot_pos      = localplayer->shoot_pos();
    i32 priority_hitbox = get_priority_hitbox();

    if(priority_hitbox == -1)
      return false;
    else if(priority_hitbox == -2){ // Crosshair Closest to hitbox
      if(lc_record == nullptr)
        return false;

      float best_fov = 999.f;
      vec3 output_pos;
      for(i32 i = 0; i <= 20; i++){
        if(!lc_record->get_hitbox_pos(target, i, output_pos))
          break;

        float fov = math::get_fov_delta(global->untouched_cmd.view_angles, shoot_pos, output_pos);
        if(fov > best_fov || fov > global->aimbot_settings->max_fov)
          continue;

        priority_hitbox = i;
        best_fov        = fov;
      }

      if(priority_hitbox < 0)
        return false;
    }

    for(i32 i = -1; i < 18; i++){
      if(priority_hitbox < 0 || i == priority_hitbox || i >= 0 && !should_use_hitbox(i))
        continue;

      i32 target_hitbox = i == -1 ? priority_hitbox : i;

      if(get_best_hitbox_point(target_hitbox, shoot_pos, target, lc_record, aim_point)){
        aim_target_hitbox = target_hitbox;
        return true;
      }
    }

    return false;
  }

  bool is_blocked_by_smoke(vec3 aim_point){
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return false;

    vec3 shoot_pos = localplayer->shoot_pos();
    vec3 angle     = math::calc_view_angle(shoot_pos, aim_point);
    math::clamp_angles(angle);

    vec3 fwd;
    math::angle_2_vector(angle, &fwd, nullptr, nullptr);


    for(i32 j = globaldata->max_clients + 1; j <= global->entity_list->get_highest_index(); j++){
      c_base_entity* smoke = global->entity_list->get_entity(j);
      if(smoke == nullptr)
        continue;

      if(smoke->is_dormant())
        continue;

      if(!smoke->is_smoke_entity() || !smoke->is_smoke_entity_alive())
        continue;

      vec3 smoke_pos = smoke->origin();
      if(math::intersect_inf_ray_with_sphere(shoot_pos, fwd, smoke_pos, 140.f))
        return true;
    }

    return false;
  }

  bool get_target(c_base_entity*& target, s_lc_record*& lc_record, vec3& aim_point){

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return false;

    set_aimbot_filter();

    if(filter == nullptr)
      return false;

    {
      for(u32 i = 1; i <= globaldata->max_clients; i++){
        if(i == global->engine->get_local_player())
          continue;

        c_base_entity* entity = global->entity_list->get_entity(i);
        if(entity == nullptr)
          continue;

        if(!should_target_entity(entity))
          continue;

        // If we have a valid lock target then only process this entity.
        if(aim_lock_target != nullptr && aim_lock_target != entity)
          continue;

        if(entity->is_player()){
          c_base_player* player = (c_base_player*)entity;

          //c_player_data* data = player->data();
          //if(data == nullptr)
          //  continue;

          s_lc_data* lc = lag_compensation->get_data(player);
          if(lc == nullptr)
            continue;

          if(utils::wait_lag_fix(player))
            continue;

          for(u32 ii = 0; ii < lc->max_records; ii++){
            s_lc_record* record = lc->get_record(ii);

            if(record == nullptr)
              continue;

            if(!record->valid())
              continue;

            // If we dont want to use history, then only use the first valid tick
            if(!global->aimbot_settings->position_adjust_history)
              ii = lc->max_records;

            if(!filter->sanity(record, entity))
              continue;

            if(!filter->handle(record, entity))
              continue;

            if(!filter->store(record, entity))
              continue;
          }
        }
      }

      aim_point = filter->aim_point;
      target    = filter->target_ent;
      lc_record = filter->lc_record;
    }

    return target != nullptr;
  }

  bool run(){
    if(global->aimbot_settings == nullptr)
      return false;
    //if(double_tap->charged_ticks != 0 && !double_tap->can_weapon_dt())
    //  return false;

    aim_target        = nullptr;
    aim_target_hitbox = -1;

    if(!global->aimbot_settings->enabled)
      return false;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return false;

    c_base_weapon* weapon = localplayer->get_weapon();
    if(weapon == nullptr)
      return false;

    if(weapon->is_melee() || weapon->is_grenade())
      return false;

    if(!global->aimbot_settings->silent_aim_none && !localplayer->can_fire())
      return false;

    if(global->aimbot_settings->aim_mode_on_key && !input_system->held(global->aimbot_settings->aim_hotkey) || global->aimbot_settings->aim_mode_on_attack && !(global->current_cmd->buttons & IN_ATTACK))
      return false;

    //if(!double_tap->is_shift_tick())
    //  aim_lock_target = nullptr;

    vec3            aim_point;
    c_base_entity*  target = nullptr;
    s_lc_record*    record = nullptr;

    if(!get_target(target, record, aim_point)){
      aim_lock_target = nullptr;
      return false;
    }

    global->on_aimbot_target(target->get_index(), aim_point, weapon);

    vec3 aim_angle = math::calc_view_angle(localplayer->shoot_pos(), aim_point);
    aim_angle -= recoil_control->get_uncompensated_view_punch();

    aim_target = target;
    bool will_fire = false;

    if(global->aimbot_settings->auto_shoot || !global->aimbot_settings->aim_mode_on_key)
      global->current_cmd->buttons |= IN_ATTACK;
    else{
      if(!global->aimbot_settings->smoothing || !global->aimbot_settings->silent_aim_none){
        if(global->aimbot_settings->aim_mode_on_key && !(global->current_cmd->buttons & IN_ATTACK))
          return false;
      }
    }

    // set our viewangle
    {
      if(global->aimbot_settings->silent_aim_none && global->aimbot_settings->smoothing > 0)
        aim_angle = math::lerp_angle(global->engine->get_view_angles(), aim_angle, math::clamp(1.f - (global->aimbot_settings->smoothing / 100.f), 0.f, 1.f) );

      if(localplayer->will_fire_this_tick()){
        global->current_cmd->view_angles = aim_angle;

        if(global->aimbot_settings->silent_aim_serverside)
          packet_manager->set_choke(true);

        // mark this tick as a fire
        will_fire = true;
      }

      if(global->aimbot_settings->silent_aim_none)
        global->engine->set_view_angles(aim_angle);
    }

    if(target->is_player()){
      interp_manager->set_tick_count_with_interp(record != nullptr ? record->simulation_time : target->simulation_time(), target->get_index(), record != nullptr);

      if(will_fire){
        c_base_player*    p = (c_base_player*)target;

        c_player_data* data = p->data();
        if(data != nullptr)
          data->next_shot_time = math::time() + (global->engine->get_latency(true) + global->engine->get_latency(false));
      }
    }

    #if defined(DEV_MODE)
    if(will_fire){
      render_debug->draw_line(localplayer->shoot_pos(), aim_point, colour(255, 255, 255, 255), true, 2.f);
    }
    #endif

    // - Don't allow DT here on target conditional check here.

    //if(double_tap->is_shift_tick())
    //  aim_lock_target = target;

    return will_fire;
  }

  i32            aim_target_hitbox;
  c_base_entity* aim_target;
  c_base_entity* aim_lock_target;
};

CLASS_EXTERN(c_raytrace_aimbot, raytrace_aimbot);