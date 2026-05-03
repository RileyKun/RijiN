#include "../link.h"

bool c_auto_repair_aimbot::get_best_hitbox_point(vec3 shoot_pos, c_base_player* target, vec3& aim_point){
  if(target == nullptr)
    return false;

  aim_point = target->obb_center(); // There isn't hitbox support for world entities yet.

  //// Are they in our fov?
  if(config->automation.auto_repair_max_fov > 0 && config->automation.auto_repair_max_fov < 180)
    if(math::get_fov_delta(global->untouched_cmd.view_angles, shoot_pos, aim_point) > (float)config->automation.auto_repair_max_fov)
      return false;

  return true;
}

bool c_auto_repair_aimbot::get_best_aim_point(c_base_entity* target, vec3& aim_point){
  if(target == nullptr  || global->aimbot_settings == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* wep = localplayer->get_weapon();

  if(wep == nullptr)
    return false;

  vec3 shoot_pos = localplayer->shoot_pos();
  if(!get_best_hitbox_point(shoot_pos, target, aim_point))
    return false;

  // Rebuilt CTFWrench::Smack trace code.
  if(wep->is_wrench()){
    vec3 fwd;
    vec3 aim_angle  = math::calc_view_angle(shoot_pos, aim_point);
    vec3 swing_mins = vec3(-18.f, -18.f, -18.f);
    vec3 swing_maxs = vec3(18.f, 18.f, 18.f);

    math::angle_2_vector(aim_angle, &fwd, nullptr, nullptr);
    vec3 end = localplayer->shoot_pos() + fwd * 70.f;

    s_trace trace = global->trace->ray(localplayer->shoot_pos(), end, mask_solid, TRACE_EVERYTHING, localplayer, TR_CUSTOM_FILTER_WRENCH_SWING);
    if(trace.fraction >= 1.0f)
      trace = global->trace->ray_obb(localplayer->shoot_pos(), end, swing_mins, swing_maxs, mask_solid, TRACE_EVERYTHING, localplayer, TR_CUSTOM_FILTER_WRENCH_SWING);

    // We sadly didn't hit anything...
    if(trace.fraction >= 1.0f)
      return false;

    // We hit something! But it wasn't our target!!!
    if(trace.hit_entity != target)
      return false;
  }
  else if(rescue_ranger_mode){
    s_current_target_data target_data;
    target_data.localplayer       = localplayer;
    target_data.wep               = wep;
    target_data.proj_speed        = wep->get_projectile_speed();
    target_data.proj_gravity      = wep->get_projectile_gravity();
    target_data.entity            = target;
    target_data.predicted_pos     = target->origin();
    target_data.predicted_obb     = target->obb_center();
    target_data.shoot_pos         = localplayer->shoot_pos();
    target_data.aim_point         = target->obb_center();
    target_data.target_aim_point  = target_data.aim_point;

    if(target->sapped() || target->placing() || target->building())
      return false;

    if(!projectile_aimbot->calc_gravity_offset(&target_data))
      return false;

    if(!projectile_aimbot->get_correct_shoot_pos_info(&target_data, true, XOR("c_auto_repair_aimbot::get_best_aim_point")))
      return false;

    if(!projectile_aimbot->is_projectile_trajectory_visible(&target_data))
     return false;

   aim_point = target_data.target_aim_point;
  }
  return true;
}

bool c_auto_repair_aimbot::get_target(c_base_entity*& target, vec3& aim_point){
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

  {
    for(u32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
      c_base_entity* entity = global->entity_list->get_entity(i);

      if(entity == nullptr || entity->is_dormant()|| entity == localplayer)
        continue;

      if(localplayer->team() != entity->team())
        continue;

      if(!entity->is_sentry() && !entity->is_dispenser() && !entity->is_teleporter())
        continue;

      if(!filter->sanity(nullptr, entity))
        continue;

      if(!filter->handle(nullptr, entity))
        continue;

      filter->store(nullptr, entity);
    }

    aim_point = filter->aim_point;
    target    = filter->target_ent;
  }

  return target != nullptr;
}

bool c_auto_repair_aimbot::crithack_force_wrapper(bool b){
  return crit_hack->force(b);
}

bool c_auto_repair_aimbot::run(){
  // Disable auto-wrench when manually firing.
  if(global->original_cmd.buttons & IN_ATTACK){
    if(last_target != nullptr){
      DBG("[!] Disabling auto-wrench.\n");
      global->current_cmd->buttons &= ~IN_ATTACK;
      last_target = nullptr;
    }
    return false;
  }
  else if(global->original_cmd.buttons & IN_ATTACK2){ // Disable auto-wrench when pressing IN_ATTACK2
    if(last_target != nullptr){
      DBG("[!] Disabling auto-wrench.\n");
      global->current_cmd->buttons &= ~IN_ATTACK2;
      last_target = nullptr;
    }
    return false;
  }

  if(!config->automation.auto_repair)
    return false;

  if(config->automation.auto_repair_use_key){
    if(!input_system->held(config->automation.auto_repair_key))
      return false;
  }

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(localplayer->is_bonked() || localplayer->is_taunting())
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(!weapon->is_wrench() && !weapon->is_rescue_ranger())
    return false;

  rescue_ranger_mode = weapon->is_rescue_ranger();
  if(!config->automation.auto_repair_rescue_ranger_enabled && rescue_ranger_mode)
    return false;

  if(rescue_ranger_mode){
    if(!localplayer->can_fire())
      return false;
    
    if(global->aimbot_target_index > 0 && global->aimbot_target_type != AIMBOT_AUTO_REPAIR)
      return false;
  }

  vec3            aim_point;
  c_base_entity*  target = nullptr;
  if(!get_target(target, aim_point))
    return false;

  global->on_aimbot_target(target->get_index(), aim_point, weapon, AIMBOT_AUTO_REPAIR);

  bool will_fire                = false;
  global->current_cmd->buttons |= IN_ATTACK;

  bool will_fire_this_tick = localplayer->will_fire_this_tick();
  if(will_fire_this_tick){
    vec3 aim_angle = math::calc_view_angle(localplayer->shoot_pos(), aim_point);
    global->current_cmd->view_angles = aim_angle;

    if(!global->aimbot_settings->silent_aim_clientside && !global->aimbot_settings->silent_aim_serverside)
      global->engine->set_view_angles(aim_angle);

    if(global->aimbot_settings->silent_aim_serverside)
      packet_manager->set_choke(true);

    will_fire = true;
  }

  if(!target->is_sentry())
    global->allow_double_tap = false;

  // don't allow crithack when aimbotting objects
  crithack_force_wrapper(false);

  last_target = target;
  return will_fire;
}

CLASS_ALLOC(c_auto_repair_aimbot, auto_repair_aimbot);