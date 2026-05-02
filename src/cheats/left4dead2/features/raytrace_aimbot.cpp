#include "../link.h"

CLASS_ALLOC(c_raytrace_aimbot, raytrace_aimbot);

bool c_raytrace_aimbot::should_target_entity(c_base_entity* entity){
  if(entity == nullptr)
    return false;

  if(entity->is_dormant())
    return false;

  u32 type = entity->type();
  if(!type)
    return false;

  c_entity_data* data = entity->data();
  if(data == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_player* player = entity->get_player();
  if(player != nullptr){
    if(localplayer->is_steam_friend(player) && !config->aimbot.target_friends)
      return false;

    if(data->playerlist.ignore_player)
      return false;
  }

  const bool is_normal_zombie  = entity->is_infected();
  const bool is_special_zombie = entity->is_special_infected();
  const bool is_human          = entity->is_player_entity();

  if(is_normal_zombie){
    if(!config->aimbot.target_zombies)
      return false;

    // Ignore idle zombies.
    if(data->idle && config->aimbot.ignore_idle_zombies)
      return false;

    if(entity->is_infected_burning() && config->aimbot.ignore_burning_zombies)
      return false;

    return entity->is_alive();
  }
  else if(is_special_zombie){
    if(!config->aimbot.target_special_infected)
      return false;

    // Warning, witches aren't playable, and thus are their own class.
    if(type & TYPE_WITCH){
      if(config->aimbot.ignore_wandering_zombies && !entity->is_witch_raged())
        return false;

      return entity->is_alive();
    }
    else{
      // We can't harm invisible special infected in vs mode.
      if(entity->is_invisible())
        return false;

      return entity->is_alive();
    }
  }
  else if(is_human){
    if(config->aimbot.target_survivors)
      return entity->is_alive();
  }

  return false;
}

bool c_raytrace_aimbot::test_point(c_base_entity* target, s_lc_record* record, vec3& aim_point, i32 hitgroup){
  assert(hitgroup >= 0 && hitgroup <= 10);
  if(target == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  const u32 type      = target->type();
  c_entity_data* data = target->data();
  if(data == nullptr)
    return false;

  // Setup a base aim position in the event we can't setup hitboxes.
  aim_point = target->obb_center();

  // Not all aimbot entities support lag comp.
  if(record != nullptr){
    if(!record->get_hitgroup_pos(target->get_player(), hitgroup, aim_point))
      return false;
  }
  else{
    if(!data->bone_count || data->bone_count > 128)
      return false;

    s_studio_bbox* bbox = target->get_studio_bbox_from_hitgroup(hitgroup);
    if(bbox == nullptr)
      return false;

    aim_point = math::bbox_pos(bbox, data->bone_matrix, target->model_scale());
  }

  if(config->aimbot.max_fov > 0 && config->aimbot.max_fov < 180){
    if(math::get_fov_delta(global->untouched_cmd.view_angles, localplayer->shoot_pos(), aim_point) > (float)config->aimbot.max_fov)
      return false;
  }

  s_trace tr = global->trace->ray(localplayer->shoot_pos(), aim_point, mask_shot);
  return tr.vis(target);
}

bool c_raytrace_aimbot::get_best_aim_point(c_base_entity* target, s_lc_record* record, vec3& aim_point){
  if(target == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return false;

  i32 hitgroup = hitgroup_head;
  u32 type = target->type();
  if(wep->is_shotgun() && type & TYPE_WITCH || wep->can_oneshot(target->type()))
    hitgroup = hitgroup_chest;

  return test_point(target, record, aim_point, hitgroup);
}

bool c_raytrace_aimbot::get_target(c_base_entity*& target, s_lc_record*& lc_record, vec3& aim_point){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  set_aimbot_filter();

  if(filter == nullptr)
    return false;

  {
    for(u32 i = 1; i <= global->entity_list->get_highest_index(); i++){
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

      u32 type = entity->type();

      if(entity->is_special_infected() && !(type & TYPE_WITCH)){
        c_base_player* player = (c_base_player*)entity;

        s_lc_data* lc = lag_compensation->get_data(player);
        if(lc == nullptr)
          continue;

        //if(utils::wait_lag_fix(player))
        //  continue;

        for(u32 ii = 0; ii < lc->max_records; ii++){
          s_lc_record* record = lc->get_record(ii);

          if(record == nullptr)
            continue;

          if(!record->valid())
            continue;

          // If we dont want to use history, then only use the first valid tick
          if(!config->aimbot.position_adjust_history)
            ii = lc->max_records;

          if(!filter->sanity(record, entity))
            continue;

          if(!filter->handle(record, entity))
            continue;

          if(!filter->store(record, entity))
            continue;
        }
      }
      else{
        if(!filter->sanity(nullptr, entity))
          continue;

        if(!filter->handle(nullptr, entity))
          continue;

        if(!filter->store(nullptr, entity))
          continue;
      }
    }

    aim_point = filter->aim_point;
    target    = filter->target_ent;
    lc_record = filter->lc_record;
  }

  return target != nullptr;
}

bool c_raytrace_aimbot::run(){
  //if(double_tap->charged_ticks != 0 && !double_tap->can_weapon_dt())
  //  return false;
  aim_target        = nullptr;
  aim_target_hitbox = -1;

  if(!config->aimbot.enabled)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(weapon->is_melee() || weapon->is_throwable() || weapon->is_item() || weapon->is_grenade_launcher())
    return false;

  if(!config->aimbot.silent_aim_none && !localplayer->can_fire() /*&& acp->allow_bullet_time()*/)
    return false;

  if(config->aimbot.aim_mode_on_key && !input_system->held(config->aimbot.aim_hotkey) || config->aimbot.aim_mode_on_attack && !(global->current_cmd->buttons & IN_ATTACK))
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

  aim_target = target;
  bool will_fire = false;

  if(config->aimbot.auto_shoot || !config->aimbot.aim_mode_on_key)
    global->current_cmd->buttons |= IN_ATTACK;
  else{
    if(!config->aimbot.smoothing || !config->aimbot.silent_aim_none){
      if(config->aimbot.aim_mode_on_key && !(global->current_cmd->buttons & IN_ATTACK))
        return false;
    }
  }

  // set our viewangle
  {
    if(config->aimbot.silent_aim_none && config->aimbot.smoothing > 0)
      aim_angle = math::lerp_angle(global->engine->get_view_angles(), aim_angle, math::clamp(1.f - (config->aimbot.smoothing / 100.f), 0.f, 1.f) );

    if(localplayer->will_fire_this_tick()/* || !acp->allow_bullet_time()*/){
      global->current_cmd->view_angles = aim_angle;

      interp_manager->set_tick_count_with_interp(record != nullptr ? record->simulation_time : target->simulation_time(), target->get_index(), record != nullptr);

      if(config->aimbot.silent_aim_serverside)
        packet_manager->set_choke(true);

      // mark this tick as a fire
      will_fire = true;

      if(config->aimbot.silent_aim_none)
        global->engine->set_view_angles(aim_angle);

      #if defined(DEV_MODE)
      render_debug->draw_line(localplayer->obb_center(), aim_point, colour(255, 255, 255, 255), false, globaldata->interval_per_tick * 2.f);
      render_debug->draw_3dbox(aim_point, colour(255, 255, 255, 32), globaldata->interval_per_tick * 2.f);
      #endif
    }
  }

  if(will_fire && !global->speedhack_tick){
    c_entity_data* data = target->data();
    if(data != nullptr)
      data->blacklist_time = math::time() + (global->engine->get_latency(true) + global->engine->get_latency(false)) + math::ticks_to_time(2);
  }
  // - Don't allow DT here on target conditional check here.

  //if(double_tap->is_shift_tick())
  //  aim_lock_target = target;

  return will_fire;
}