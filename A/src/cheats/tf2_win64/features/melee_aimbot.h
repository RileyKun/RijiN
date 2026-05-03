#pragma once

struct s_melee_data_backup{
  vec3 old_abs_origin;
  vec3 last_viewangle;
};

class c_melee_aimbot{
public:
  c_aimbot_filter_base* filter = nullptr;
  s_entity_predict_ctx ctx;
  bool                 prediction_success;

  ALWAYSINLINE void destroy_aimbot_filter(){
    if(filter == nullptr)
      return;

    delete filter;
    filter = nullptr;
  }
  bool crithack_force_wrapper(bool b);
  void set_aimbot_filter();
  bool should_target_entity(c_base_entity* entity);
  bool calculate_aim_position(c_base_entity* target, s_lc_record* record, float mod, vec3* aim_point);
  bool push_local_data(s_melee_data_backup* melee_data);
  bool pop_local_data(s_melee_data_backup* melee_data);

  bool predict_swing(c_base_player* localplayer, c_base_entity* target);
  bool can_backstab(c_base_entity* target, s_lc_record*, vec3 aim_angle);
  bool get_best_aim_point(c_base_entity* target, s_lc_record* record, vec3& aim_point);
  bool get_target(c_base_entity*& target, s_lc_record*& record, vec3& aim_point);
  bool run();
};

CLASS_EXTERN(c_melee_aimbot, melee_aimbot);