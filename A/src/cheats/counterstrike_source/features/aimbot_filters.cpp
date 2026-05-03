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

  if(!handle_priority(record, entity))
    return false;

  bool skip_fov_check = false;
  if(global->aimbot_settings->next_shot && target_ent != nullptr && entity->is_player()){
    if(target_ent->is_player()){

      c_base_player* p1    = (c_base_player*)target_ent;
      c_base_player* p2    = (c_base_player*)entity;

      c_player_data* data1 = p1->data();
      c_player_data* data2 = p2->data();
      assert(data1 != nullptr);
      assert(data2 != nullptr);

      float time = math::time();
      if(data2->next_shot_time > time && data1->next_shot_time <= time || data1->next_shot_time < data2->next_shot_time)
        return false;
      else if(data1->next_shot_time < data2->next_shot_time)
        return false;

      if(data1->next_shot_time > data2->next_shot_time || data1->next_shot_time > time && data2->next_shot_time <= time)
        skip_fov_check = true;
    }
  }

  float current_fov = math::get_fov_delta(global->untouched_cmd.view_angles, localplayer->shoot_pos(), new_aim_point);
  if(!skip_fov_check){
    if(target_ent != nullptr && current_fov >= closest_fov)
      return false;
  }

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

  bool skip_dist_check = false;
  if(global->aimbot_settings->next_shot && target_ent != nullptr && entity->is_player()){
    if(target_ent->is_player()){

      c_base_player* p1    = (c_base_player*)target_ent;
      c_base_player* p2    = (c_base_player*)entity;

      c_player_data* data1 = p1->data();
      c_player_data* data2 = p2->data();
      assert(data1 != nullptr);
      assert(data2 != nullptr);

      float time = math::time();
      if(data2->next_shot_time > time && data1->next_shot_time <= time || data1->next_shot_time < data2->next_shot_time)
        return false;
      else if(data1->next_shot_time < data2->next_shot_time)
        return false;

      if(data1->next_shot_time > data2->next_shot_time || data1->next_shot_time > time && data2->next_shot_time <= time)
        skip_dist_check = true;
    }
  }

  if(!skip_dist_check){
    if(target_ent != nullptr && current_distance >= closest_distance )
      return false;
  }

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
