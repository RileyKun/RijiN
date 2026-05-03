#pragma once


enum raytrace_entity_flags{
  RT_AIMBOT_IGNORE_MEDIC_VACCINATOR = (1 << 1),
  RT_AIMBOT_PLAYERLIST_IGNORE_AIMBOT = (1 << 2),
  RT_AIMBOT_FORCE_BODY_AIM = (1 << 3),
  RT_AIMBOT_FORCE_BODY_AIM_MANUAL = (1 << 4),
  RT_AIMBOT_FORCE_BODY_AIM_AMBY = (1 << 5),
  RT_AIMBOT_FORCE_BODY_AIM_SILENT = (1 << 6),
};

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
    if(global->aimbot_settings == nullptr)
      return;

    destroy_aimbot_filter();

    if(global->aimbot_settings->target_method_closest_to_fov)
      filter = new c_aimbot_filter_raytrace_fov;
    else if(global->aimbot_settings->target_method_distance)
      filter = new c_aimbot_filter_raytrace_distance;
  }

  bool should_target_entity(c_base_entity* entity);
  i32 get_priority_hitbox();
  i32 get_target_flags(c_base_entity* entity);
  bool should_use_hitbox(i32 hitbox);
  bool get_other_aim_points(i32 hitbox, vec3 shoot_pos, c_base_entity* target, s_lc_record* lc_record, vec3& aim_point);
  bool get_best_hitbox_point(i32 hitbox, vec3 shoot_pos, c_base_entity* target, s_lc_record* lc_record, vec3& aim_point);
  bool get_best_aim_point(c_base_entity* target, s_lc_record* lc_record, vec3& aim_point);
  bool get_target(c_base_entity*& target, s_lc_record*& lc_record, vec3& aim_point);
  bool crithack_force_wrapper(bool b);
  bool run();
  
  i32            aim_target_hitbox;
  c_base_entity* aim_target;
  c_base_entity* aim_lock_target;
};

CLASS_EXTERN(c_raytrace_aimbot, raytrace_aimbot);