#pragma once

//#define MEDIGUN_AIMBOT_DEBUG
class c_medigun_aimbot{
public:
  c_aimbot_filter_base* filter = nullptr;

  ALWAYSINLINE void destroy_aimbot_filter(){
    if(filter == nullptr)
      return;

    delete filter;
    filter = nullptr;
  }

  void  set_aimbot_filter();
  i32   get_priorty(c_base_entity* entity);
  float get_next_target_delay(c_base_player* target, c_base_weapon* weapon);
  bool  should_target_while_charged(c_base_player* target);
  bool  get_best_hitbox_point(i32 hitbox, vec3 shoot_pos, c_base_player* target, s_lc_record* lc_record, vec3& aim_point);
  bool  get_best_aim_point(c_base_player* target, s_lc_record* lc_record, vec3& aim_point);

  bool get_target(c_base_player*& target, s_lc_record*& lc_record, vec3& aim_point);

  bool enforce_tickcount(c_base_player* target, s_lc_record* record);
  bool run();

  c_base_player* last_target       = nullptr;
  float          next_target_delay = 0.f;
  i32            heal_type         = 0;
  i32            heal_target_maxhp = 0;
  i32            in_attack_spam    = 0;
};

CLASS_EXTERN(c_medigun_aimbot, medigun_aimbot);