#pragma once

class c_raytrace_aimbot{
public:
  c_aimbot_filter_base* filter = nullptr;

  NEVERINLINE void destroy_aimbot_filter(){
    if(filter == nullptr)
      return;

    delete filter;
    filter = nullptr;
  }

  NEVERINLINE void set_aimbot_filter(){
    destroy_aimbot_filter();

    if(config->aimbot.target_method_smart_shot)
      filter = new c_aimbot_filter_raytrace_smart_shot;
    else if(config->aimbot.target_method_closest_to_fov)
      filter = new c_aimbot_filter_raytrace_fov;
    else
      filter = new c_aimbot_filter_raytrace_distance;
  }

  bool should_target_entity(c_base_entity* entity);
  bool test_point(c_base_entity* target, s_lc_record* lc_record, vec3& aim_point, i32 hitgroup);
  bool get_best_aim_point(c_base_entity* target, s_lc_record* lc_record, vec3& aim_point);
  bool get_target(c_base_entity*& target, s_lc_record*& lc_record, vec3& aim_point);
  bool run();

  i32            aim_target_hitbox;
  c_base_entity* aim_target;
  c_base_entity* aim_lock_target;
};

CLASS_EXTERN(c_raytrace_aimbot, raytrace_aimbot);