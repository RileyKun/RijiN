#include "../link.h"

bool c_melee_aimbot::crithack_force_wrapper(bool b){
  return crit_hack->force(b);
}

void c_melee_aimbot::set_aimbot_filter(){
  if(global->aimbot_settings == nullptr)
    return;

  destroy_aimbot_filter();

  filter = new c_aimbot_filter_melee_fov;
}

bool c_melee_aimbot::should_target_entity(c_base_entity* entity){
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

  c_base_weapon* slot1_wep = localplayer->get_weapon_from_belt(0, false);

  bool truce_active = utils::is_truce_active();
  if(entity->is_player()){
    c_base_player* player = (c_base_player*)entity;
    if(!player->is_valid())
      return false;

    if(!localplayer->is_considered_enemy(player))
      return false;

    c_player_data* data = player->data();
    if(data == nullptr)
      return false;

    if(global->aimbot_settings->aimbot_only_enabled){
      if(!global->aimbot_settings->aimbot_only_use_key || input_system->held(global->aimbot_settings->aimbot_only_key)){

        bool fail = true;
        if(global->aimbot_settings->aimbot_only_prioritized && (data->playerlist.priority_mode && data->playerlist.priority > 0))
          fail = false;
        else if(global->aimbot_settings->aimbot_only_cheaters && cheat_detection->is_cheating(player->get_index()))
          fail = false;
        else if(global->aimbot_settings->aimbot_only_bots && cheat_detection->is_cheating_bot(player->get_index()))
          fail = false;

        if(fail)
          return false;
      }
    }

    bool bypass_ubercharge_check = false;
    bool bypass_bonk_check       = false;
    bool bypass_truce_check      = false;

    if(wep->is_holiday_punch() || wep->is_soldier_whip() && localplayer->team() == player->team()){
      bypass_ubercharge_check = true;
      bypass_bonk_check       = true;
      bypass_truce_check      = wep->is_soldier_whip();
    }
    else if(wep->is_ubersaw() && wep->uber_charge_level() < 1.0f){
      bypass_bonk_check = true;
    }
    else if(wep->is_knife()){
      if(slot1_wep != nullptr){
        if(slot1_wep->is_diamond_back())
          bypass_bonk_check = true;
      }

      if(player->has_razer_back() && global->aimbot_settings->spy_ignore_razer_back)
        return false;
    }

    if(player->is_ubercharged() && !bypass_ubercharge_check || player->is_bonked() && !bypass_bonk_check || truce_active && !bypass_truce_check)
      return false;

    return true;
  }
  else{
    if(entity->is_dormant())
      return false;

    if(entity->team() == localplayer->team())
      return false;

    if(entity->is_boss() && global->aimbot_settings->target_boss)
      return true;

    // This happens in the special halloween maps. We can no longer damage any enemy buildings or stickies.
    if(truce_active)
      return false;

    if(entity->is_sentry() && global->aimbot_settings->target_sentries || entity->is_dispenser() && global->aimbot_settings->target_dispenser || entity->is_teleporter() && global->aimbot_settings->target_teleporter)
      return entity->object_health() > 0;
  }

  return false;
}

bool c_melee_aimbot::calculate_aim_position(c_base_entity* target, s_lc_record* record, float mod, vec3* aim_point){
  if(target == nullptr || aim_point == nullptr)
    return false;

  mod = math::clamp(mod, 0.f, 1.f);

  if(record == nullptr)
    *aim_point = target->origin() + ((target->obb_mins() + target->obb_maxs()) * mod); 
  else
    *aim_point = record->origin + ((record->obb_mins + record->obb_maxs) * mod);

  return true;
}

bool c_melee_aimbot::push_local_data(s_melee_data_backup* melee_data){
  if(melee_data == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  melee_data->old_abs_origin = localplayer->get_abs_origin();
  melee_data->last_viewangle = localplayer->last_viewangle();

  return true;
}

bool c_melee_aimbot::pop_local_data(s_melee_data_backup* melee_data){
  if(melee_data == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  localplayer->set_abs_origin(melee_data->old_abs_origin);
  localplayer->last_viewangle() = melee_data->last_viewangle;

  return true;
}

bool c_melee_aimbot::predict_swing(c_base_player* localplayer, c_base_entity* target){
  #if !defined(DEV_MODE)
    return true;
  #endif
  if(target == nullptr)
    return false;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return false;

  ctf_weapon_info* wep_info = wep->get_weapon_info();
  if(wep_info == nullptr)
    return false;

  if(!global->aimbot_settings->swing_prediction || wep->is_knife())
    return true;

  if(prediction_success){
    localplayer->set_abs_origin(ctx.predicted_origin);
    return true;
  }

  return false;
}

bool c_melee_aimbot::can_backstab(c_base_entity* target, s_lc_record* record, vec3 aim_angle){
  if(target == nullptr || record == nullptr)
    return false;

  c_base_player* player = target->get_player();
  if(player == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_player_class(TF_CLASS_SPY))
    return false;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return false;

  ctf_weapon_info* wep_info = wep->get_weapon_info();
  if(wep_info == nullptr)
    return false;

  vec3 delta = (record->obb_center - localplayer->obb_center());
  delta.z = 0.f;
  delta.normalize();
  
  vec3 fwd, fwd1;
  math::angle_2_vector(aim_angle, &fwd, nullptr, nullptr);
  fwd.z = 0.f;
  fwd.normalize();

  vec3 target_viewangle = player->viewangles();    

  math::angle_2_vector(target_viewangle, &fwd1, nullptr, nullptr);
  fwd1.z = 0.f;
  fwd1.normalize();

  if(delta.dot_2d(fwd1) <= 0.f || delta.dot_2d(fwd) <= 0.5f || fwd.dot_2d(fwd1) <= -0.3f)
    return false;

  return true;
}

bool c_melee_aimbot::get_best_aim_point(c_base_entity* target, s_lc_record* record, vec3& aim_point){
  if(target == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return false;

  ctf_weapon_info* wep_info = wep->get_weapon_info();
  if(wep_info == nullptr)
    return false;

  s_melee_data_backup data;
  memset(&data, 0, sizeof(s_melee_data_backup));

  vec3   shoot_pos      = localplayer->shoot_pos();
  vec3   old_abs_origin = target->get_abs_origin();
  float  swing_range    = wep->get_swing_range(true);

  if(record != nullptr)
    target->set_abs_origin(record->origin);

  bool  result    = false;
  float best_dist = 0.f;

  // Backup localplayer data.
  push_local_data(&data);
  if(!predict_swing(localplayer, target))
    return false;

  {
    for(float x = 0.f; x <= 1.f; x += 0.05f){
      if(!calculate_aim_position(target, record, x, &aim_point))
        continue;

      // Check FOV.
      {
        if(global->aimbot_settings->max_fov > 0 && global->aimbot_settings->max_fov < 180)
          if(math::get_fov_delta(global->untouched_cmd.view_angles, shoot_pos, aim_point) > (float)global->aimbot_settings->max_fov)
            continue;
      }

      vec3 aim_angle = math::calc_view_angle(shoot_pos, aim_point);
      localplayer->last_viewangle() = aim_angle;

      s_trace trace;
      if(!wep->swing_trace(&trace) || trace.hit_entity != target && !should_target_entity(trace.hit_entity))
        continue;
      
      if(target->is_player()){
        if(target->health() > wep_info->damage){
          if(wep->is_knife() && !can_backstab(target, record, aim_angle))
            continue;
        }
      }

      // Check distance.
      float dist = math::round(trace.start.distance(trace.end));
      if(dist >= (swing_range - 2.f))
        continue;
    
      if(result){
        if(dist > best_dist)
          continue;
      }

      best_dist = dist;
      result    = true;
    }
  }

  pop_local_data(&data);
  target->set_abs_origin(old_abs_origin);

  return result;
}

bool c_melee_aimbot::get_target(c_base_entity*& target, s_lc_record*& record, vec3& aim_point){
  if(global->aimbot_settings == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return false;

  set_aimbot_filter();
  if(filter == nullptr)
    return false;

  filter->max_range = 1024.f;
  {
    for(u32 i = target_list->get_loop_start_index(); i <= target_list->get_loop_end_index(); i++){
      i32 index = target_list->get_sorted_target_index(i, true);
      if(index == global->localplayer_index)
        continue;

      c_base_entity* entity = global->entity_list->get_entity(index);
      if(entity == nullptr)
        continue;

      if(!should_target_entity(entity))
        continue;

      if(entity->is_player()){
        s_lc_data* lc = lag_compensation->get_data((c_base_player*)entity);

        if(lc == nullptr)
          continue;

        for(u32 ii = 0; ii < lc->max_records; ii++){
          s_lc_record* record = lc->get_record(ii);
          if(record == nullptr)
            continue;

          if(!record->valid())
            continue;

          // If we dont want to use history, then only use the first valid tick
          if(!acm->can_back_track() || !global->aimbot_settings->position_adjust_history)
            ii = lc->max_records;

          if(!filter->sanity(record, entity))
            continue;

          if(!filter->handle(record, entity))
            continue;

          if(filter->store(record, entity))
            break;
        }
      }
      else{

        if(!filter->sanity(nullptr, entity))
          continue;

        if(!filter->handle(nullptr, entity))
          continue;

         if(filter->store(nullptr, entity))
          break;
      }
    }

    aim_point = filter->aim_point;
    target    = filter->target_ent;
    record    = filter->lc_record;
  }

  return target != nullptr;
}

bool c_melee_aimbot::run(){
  prediction_success = false;
  if(global->aimbot_settings == nullptr)
    return false;

  if(!global->aimbot_settings->enabled)
    return false;

  if(target_list->is_being_spectated() && global->aimbot_settings->disable_on_spectated)
    return false;

  if(global->aimbot_settings->aim_mode_on_key && !input_system->held(global->aimbot_settings->aim_hotkey) || global->aimbot_settings->aim_mode_on_attack && !(global->current_cmd->buttons & IN_ATTACK))
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  // Ducking changes the position of our shoot position which can result in misses if we spam ducking in and out.
  if(localplayer->ducking() && !localplayer->ducked() || localplayer->ducked() && !(global->current_cmd->buttons & IN_DUCK))
    return false;

  if(localplayer->is_bonked() || localplayer->is_taunting())
    return false;

  if(localplayer->has_condition(TF_COND_SHIELD_CHARGE))
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(!weapon->is_melee())
    return false;

  if(weapon->is_harmless_item())
    return false;

  ctf_weapon_info* wep_info = weapon->get_weapon_info();
  if(wep_info == nullptr)
    return false;

  if(global->aimbot_settings->swing_prediction && !weapon->is_knife()){
    const u32   swing_ticks  = math::biggest(1, math::time_to_ticks(wep_info->smack_delay));
    ctx = s_entity_predict_ctx(localplayer, (global->current_cmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT | IN_LEFT | IN_RIGHT)) ? 0 : EP_FLAG_PREDICT_FRICTION);
    prediction_success = entity_prediction->simple(&ctx, swing_ticks);
  } 

  vec3            aim_point;
  c_base_entity*  target = nullptr;
  s_lc_record*    record = nullptr;
  if(!get_target(target, record, aim_point))
    return false;

  global->on_aimbot_target(target->get_index(), aim_point, weapon);

  bool will_fire   = false;
  bool force_crits = true;
  if(target->is_player()){
    c_base_player* target_player = (c_base_player*)target;

    if(weapon->is_holiday_punch() && (target_player->is_disguised() || target_player->is_cloaked() || target_player->is_taunting())) // Disguised, cloaked and taunting players can't be forced to laugh.
      force_crits = false;
    else if(weapon->is_ubersaw() && target_player->is_bonked()) // No point in wasting crits against a bonked scout.
      force_crits = false;
  }
  else if(!target->is_boss())
    force_crits = false; // Can't deal crit damage on anything else!

  if(global->aimbot_settings->auto_shoot || global->aimbot_settings->aim_mode_automatic)
    global->current_cmd->buttons |= IN_ATTACK;

  // set our viewangle
  if(localplayer->will_fire_this_tick()){
    vec3 aim_angle = math::calc_view_angle(localplayer->shoot_pos(), aim_point);

    global->current_cmd->view_angles = aim_angle;

    if(!global->aimbot_settings->silent_aim_clientside && !global->aimbot_settings->silent_aim_serverside)
      global->engine->set_view_angles(aim_angle);

    if(global->aimbot_settings->silent_aim_serverside)
      packet_manager->set_choke(true);

    will_fire = true;
  }

  if(target->is_player()){
    interp_manager->set_tick_count_with_interp(record != nullptr ? record->simulation_time : target->simulation_time(), target->get_index(), record != nullptr);

    if(config->misc.crithack_melee_auto_crit && (global->aimbot_settings->target_friendlies || target->team() != localplayer->team()))
      crithack_force_wrapper(force_crits);

    if(weapon->is_soldier_whip() && target->team() == localplayer->team())
      global->allow_double_tap = false;
  }

  return will_fire || weapon->smack_time() <= 0.f;
}

CLASS_ALLOC(c_melee_aimbot, melee_aimbot);