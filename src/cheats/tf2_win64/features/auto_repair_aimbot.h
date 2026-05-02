#pragma once

class c_auto_repair_aimbot{
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

    filter = new c_aimbot_filter_autorepair;
  }

  bool get_best_hitbox_point(vec3 shoot_pos, c_base_player* target, vec3& aim_point);
  bool get_best_aim_point(c_base_entity* target, vec3& aim_point);
  bool get_target(c_base_entity*& target, vec3& aim_point);
  bool crithack_force_wrapper(bool b);
  bool run();

  bool rescue_ranger_mode;
  c_base_entity* last_target = nullptr;
};

CLASS_EXTERN(c_auto_repair_aimbot, auto_repair_aimbot);