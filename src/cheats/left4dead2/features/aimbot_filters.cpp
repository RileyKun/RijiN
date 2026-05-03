#include "../link.h"

bool c_aimbot_filter_raytrace_fov::handle(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  float current_distance = localplayer->origin().distance(entity->origin());
  if(record != nullptr)
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

  float current_fov = math::get_fov_delta(global->untouched_cmd.view_angles, localplayer->shoot_pos(), new_aim_point);
  if(target_ent != nullptr && current_fov >= closest_fov)
    return false;

  aim_point  = new_aim_point;
  target_ent = entity;
  lc_record  = record;

  closest_fov = current_fov;

  return true;
};

bool c_aimbot_filter_raytrace_smart_shot::handle(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  current_distance = localplayer->origin().distance(entity->origin());
  if(record != nullptr)
    current_distance = localplayer->origin().distance(record->origin);

  if(max_range > 0.f && current_distance > max_range)
    return false;

  c_entity_data* data         = entity->data();
  i32            type         = entity->type();
  bool           can_oneshot  = localplayer_wep->can_oneshot(type);

  if(!(type & TYPE_IS_ZOMBIE) && entity->is_player_entity())
    return true;

  if((can_oneshot || target_ent != nullptr) && data->blacklist_time >= current_time)
    return false;

  if(target_ent == nullptr)
    first_target_blacklisted = data->blacklist_time >= current_time;

  if(target_ent != nullptr){
    if(was_idle && !data->idle)
      return true;

    if(data->idle && !was_idle)
      return false;
  }

  return true;
};

bool c_aimbot_filter_raytrace_smart_shot::store(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  vec3 new_aim_point;
  if(!raytrace_aimbot->get_best_aim_point(entity, record, new_aim_point))
    return false;

  // This is a good idea, or nah?
  if(!handle_priority(record, entity))
    return false;

  bool bypass_distance_check = false;
  if(target_ent != nullptr){
    u32 current_target_type = target_ent->type();
    u32 new_target_type     = entity->type();

    if((current_target_type & TYPE_SPECIAL) && !(new_target_type & TYPE_SPECIAL) && !(current_target_type & TYPE_JOCKEY) && !(current_target_type & TYPE_SMOKER) && !(current_target_type & TYPE_CHARGER) && current_distance < 300.f)
      bypass_distance_check = true;
    // If the new target isnt a special, then skip it
    else if((current_target_type & TYPE_SPECIAL) && !(new_target_type & TYPE_SPECIAL))
      return false;
  }

  if(!bypass_distance_check && target_ent != nullptr && current_distance >= closest_distance)
    return false;

  aim_point  = new_aim_point;
  target_ent = entity;
  lc_record  = record;

  closest_distance = current_distance;

  return true;
};

bool c_aimbot_filter_raytrace_distance::handle(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  current_distance = localplayer->origin().distance(entity->origin());
  if(record != nullptr)
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

  if(target_ent != nullptr && current_distance >= closest_distance )
    return false;

  aim_point  = new_aim_point;
  target_ent = entity;
  lc_record  = record;

  closest_distance = current_distance;

  return true;
};

bool c_aimbot_filter_bash_distance::handle(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  u32 type = entity->type();

  // Can't bash these classes
  if((type & TYPE_TANK) || (type & TYPE_CHARGER) || (type & TYPE_WITCH))
    return false;

  c_entity_data* ed = entity->data();
  if(ed != nullptr){
    // Don't bash the bashed
    if(str_utils::contains(ed->sequence_name, XOR("Shoved_")))
      return false;
  
    if(config->aimbot.auto_bash_mode_smart){
      // Ignore idle but not Upper_KNIFE
      u32 seq_hash = HASH_RT(ed->sequence_name);
      if(seq_hash != HASH("Idle_Upper_KNIFE") && str_utils::contains(ed->sequence_name, XOR("Idle_")))
        return false;
    
      // Dont target random walking
      if(str_utils::contains(ed->sequence_name, XOR("Walk_Neutral")))
        return false;
    
      //DBG("%s\n", entity->get_sequence_name());
    }
  }

  current_distance = localplayer->origin().distance(entity->origin());
  if(record != nullptr)
    current_distance = localplayer->origin().distance(record->origin);

  max_range = 150.f;
  if(max_range > 0.f && current_distance > max_range)
    return false;

  return true;
};

bool c_aimbot_filter_bash_distance::store(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  vec3 new_aim_point;
  if(!raytrace_aimbot->get_best_aim_point(entity, record, new_aim_point))
    return false;

  if(target_ent != nullptr && current_distance >= closest_distance )
    return false;

  // Only target specials
  if(target_ent != nullptr && (target_ent->type() & TYPE_SPECIAL) && !(entity->type() & TYPE_SPECIAL))
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
