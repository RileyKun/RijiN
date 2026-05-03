#include "../link.h"

bool c_aimbot_filter_raytrace_fov::handle(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  float current_distance = localplayer->origin().distance(entity->origin());
  if(entity->is_player() && record != nullptr)
    current_distance = localplayer->origin().distance(record->origin);

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

  float current_fov = math::get_fov_delta(global->untouched_cmd.view_angles, localplayer->shoot_pos(), new_aim_point);

  if(target_ent != nullptr && current_fov >= closest_fov)
    return false;

  aim_point  = new_aim_point;
  target_ent = entity;
  lc_record  = record;

  closest_fov = current_fov;

  return true;
};

bool c_aimbot_filter_raytrace_distance::handle(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  current_distance = localplayer->origin().distance(entity->origin());
  if(entity->is_player() && record != nullptr)
    current_distance = localplayer->origin().distance(record->origin);

  if(target_ent != nullptr && current_distance >= closest_distance || max_range > 0.f && current_distance > max_range)
    return false;

  return true;
};

bool c_aimbot_filter_raytrace_distance::store(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  vec3 new_aim_point;
  if(!raytrace_aimbot->get_best_aim_point(entity, record, new_aim_point))
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
