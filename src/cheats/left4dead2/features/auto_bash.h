#pragma once

class c_auto_bash{
public:
  bool                  should_dump = false;
  s_trace               trace_dump;
  c_aimbot_filter_base* filter = nullptr;
  i32                   aim_target_hitbox;
  c_base_entity*        aim_target;
  c_base_entity*        aim_lock_target;

  ALWAYSINLINE void destroy_aimbot_filter(){
    if(filter == nullptr)
      return;

    delete filter;
    filter = nullptr;
  }

  ALWAYSINLINE void set_aimbot_filter(){
    destroy_aimbot_filter();

    filter = new c_aimbot_filter_bash_distance;
  }

  ALWAYSINLINE bool should_target_entity(c_base_entity* entity){
    if(entity == nullptr)
      return false;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return false;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return false;

    u32 type = entity->type();
    if(type == 0)
      return false;

    c_entity_data* data = entity->data();
    if(data == nullptr)
      return false;

    c_base_player* player = (c_base_player*)entity;
    if(type & TYPE_IS_ZOMBIE){
      //if(data->idle && config->aimbot.ignore_idle_zombies)
        //return false;

      if(entity->is_infected_burning() && config->aimbot.ignore_burning_zombies)
        return false;

      if(entity->is_invisible())
        return false;

      if(type & TYPE_WITCH && config->aimbot.ignore_wandering_zombies){
        if(entity->raged() <= 0.0f)
          return false;
      }

      return player->is_valid();
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

    //i32 hitbox_index = target->get_best_hitbox_index();
    //if(hitbox_index == -1)
    //  return false;

    c_entity_data* data = target->data();
    assert(data != nullptr);
    i32 hitgroup = data->idle ? hitgroup_stomach : hitgroup_head;

    aim_point = target->obb_center();
    if(lc_record != nullptr)
      lc_record->get_hitgroup_pos(target->get_player(), hitgroup, aim_point);
    else{
      c_entity_data* data = target->data();
      if(data == nullptr)
        return false;

      if(!data->bone_count || data->bone_count > 128)
        return false;

      s_studio_bbox* bbox = target->get_studio_bbox_from_hitgroup(hitgroup);
      if(bbox == nullptr)
        return false;

      aim_point = math::bbox_pos(bbox, data->bone_matrix, target->model_scale());
    }

    if(config->aimbot.auto_bash_max_fov > 0 && config->aimbot.auto_bash_max_fov < 180){
      if(math::get_fov_delta(global->untouched_cmd.view_angles, localplayer->shoot_pos(), aim_point) > (float)config->aimbot.auto_bash_max_fov)
        return false;
    }

    s_trace tr = global->trace->ray(localplayer->shoot_pos(), aim_point, mask_shot, TRACE_EVERYTHING, localplayer, TR_CUSTOM_FILTER_AUTO_BASH);
    if(tr.vis(target))
      return true;

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
            if(true || !config->aimbot.position_adjust_history)
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

  bool simulate_swing(c_base_weapon* wep, vec3 wish_view_angle){
    if(wep == nullptr)
      return false;

    memset(&trace_dump, 0, sizeof(s_trace));

    static vec3 fwd;
    math::angle_2_vector(wish_view_angle, &fwd, nullptr, nullptr);

    /* This offset has no information and is likely a crashing issue waiting to occur.

       Is it for every weapon? Or only certain weapons? Because if it's not for every weapon then we should expect to have undefined
       isuses.

       What if the game updates? How does one find this offset?

       -Rud July 25th 2025.
    */

    // This needs to be 75 or test_swing_collision wont work
    *(float*)((u32)wep + 0xAA4) = 75.f;

    should_dump = true;
    utils::call_thiscall<bool, vec3&>(global->test_swing_collision, wep, fwd);
    should_dump = false;

    return trace_dump.hit_entity != nullptr;
  }

  void run(){
    if(!config->aimbot.auto_bash_enabled)
      return;

    // Don't run if we are manually firing or trying to use something
    if((global->original_cmd.buttons & IN_ATTACK) || (global->original_cmd.buttons & IN_USE))
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return;

    u32 localplayer_type = localplayer->type();
    if(localplayer_type & TYPE_IS_ZOMBIE)
      return;

    if(!wep->is_gun() && !wep->is_throwable() && !wep->is_melee() && !wep->is_item() && !wep->is_grenade_launcher())
      return;

    i32 ticks_since_last_shove = math::time_to_ticks(localplayer->last_shove_delta());
    if(!localplayer->can_shove() && ticks_since_last_shove > 3)
      return;

    vec3            aim_point;
    c_base_entity*  target = nullptr;
    s_lc_record*    record = nullptr;

    if(!get_target(target, record, aim_point)){
      aim_lock_target = nullptr;
      return;
    }

    vec3 aim_angle = math::calc_view_angle(localplayer->shoot_pos(), aim_point);

    if(simulate_swing(wep, aim_angle)){
      interp_manager->set_tick_count_with_interp(record != nullptr ? record->simulation_time : target->simulation_time(), target->get_index(), record != nullptr);

      global->current_cmd->view_angles = aim_angle;
      global->current_cmd->buttons &= ~IN_ATTACK;
      global->current_cmd->buttons |= IN_ATTACK2;

      if(config->aimbot.silent_aim_serverside)
        packet_manager->set_choke(true);
    }
  }
};

CLASS_EXTERN(c_auto_bash, auto_bash);