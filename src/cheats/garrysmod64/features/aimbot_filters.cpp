#include "../link.h"

bool c_aimbot_filter_raytrace_fov::handle(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  float current_distance = localplayer->origin().distance(entity->origin());
  //if(entity->is_player() && record != nullptr)
    //current_distance = localplayer->origin().distance(record->origin);

  if(max_range > 0.f && current_distance > max_range)
    return false;

  return true;
};

bool c_aimbot_filter_raytrace_fov::store(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  vec3 new_aim_point;
  if(!raytrace_aimbot->get_best_aim_point(entity, record, new_aim_point))
    return false;

  if(!handle_priority(record, entity))
    return false;

  // calcualte the fov radius appropriate to the screen
  float fov_rad = (float)config->aimbot.max_fov;
        fov_rad = math::clamp(fov_rad / global->game_fov, 0.f, 1.f) * (render->screen_size.x * 0.5f);

  vec3i scr_new_aim_point;
  if(math::world2screen(new_aim_point, scr_new_aim_point)){
    vec3i delta = (render->screen_size / 2) - scr_new_aim_point;

    // I normalized the angle and then absolute the value, this fixed the issue of it not aiming at people in certain cases.
    // I suppose if this is wrong then you need to fix it.
    // At certain degrees the fov value has really really huge value. Use 180 max fov and aimbot a prop (for testing since props can't die) and move your yaw in a circle with auto shoot on and til it stop shooting.
    float fov = math::sqrt(delta.x * delta.x + delta.y * delta.y);
          fov = math::abs(math::normalize_angle((fov / fov_rad) * config->aimbot.max_fov)); // translate the pixel delta back to fov
    
    if(target_ent != nullptr && fov > closest_fov || target_ent == nullptr && fov > (float)config->aimbot.max_fov)
      return false;
    
    closest_fov = fov;
  }
  else{
    float current_fov = math::get_fov_delta(global->untouched_cmd.view_angles, localplayer->shoot_pos(), new_aim_point);
    if(target_ent != nullptr && current_fov > closest_fov)
      return false;

    // I had every entity behind me but one, and it always prefered to target the entities behind instead of the one in my crosshair.
    // If you don't set this, then this will become sorted by entity index rather than FOV.
    // This doesn't break anything with the FOV circle or the pixel fov above.
    closest_fov = current_fov;
  }

  aim_point  = new_aim_point;
  target_ent = entity;
  lc_record  = record;

  return true;
};

bool c_aimbot_filter_raytrace_distance::handle(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  current_distance = localplayer->origin().distance(entity->origin());
  //if(entity->is_player() && record != nullptr)
    //current_distance = localplayer->origin().distance(record->origin);

  if(max_range > 0.f && current_distance > max_range)
    return false;

  return true;
};

bool c_aimbot_filter_raytrace_distance::store(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  vec3 new_aim_point;
  if(!raytrace_aimbot->get_best_aim_point(entity, record, new_aim_point))
    return false;

  if(!handle_priority(record, entity))
    return false;

  if(target_ent != nullptr && current_distance >= closest_distance )
    return false;

  aim_point  = new_aim_point;
  target_ent = entity;
  lc_record  = record;

  closest_distance = current_distance;

  return true;
};

/*
bool c_aimbot_filter_melee_fov::handle(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  vec3 origin = entity->origin();
  if(record != nullptr)
    origin = record->origin;

  float current_distance = localplayer->origin().distance(origin);
  if(current_distance > max_range)
    return false;

  return true;
};

bool c_aimbot_filter_melee_fov::store(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  vec3 new_aim_point;
  if(!melee_aimbot->get_best_aim_point(entity, record, new_aim_point))
    return false;

  float current_fov = math::get_fov_delta(global->untouched_cmd.view_angles, localplayer->shoot_pos(), new_aim_point);

  if(target_ent != nullptr && current_fov >= closest_fov)
    return false;

  aim_point  = new_aim_point;
  target_ent = entity;
  lc_record  = record;

  closest_fov = current_fov;

  return true;
};*/
