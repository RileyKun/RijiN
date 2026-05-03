#pragma once

#define PROJ_TRACE_FILTER TR_CUSTOM_FILTER_NONE
#define PROJ_TRACE_MASK mask_solid
#define PROJ_SPLASH_POINTS 2048


#if defined(DEV_MODE)
//#define SPLASH_BOT_DBG
#endif

//#define PROJ_DEBUG_GRAVITY
#define PROJ_TRAJECTORY_SPLASH_BOT_TEST (1 << 1)
#define PROJ_TRAJECTORY_SKIP_OBB_SCAN (1 << 2)


// This is not data related to a target we're gonna aimbot.
struct s_current_target_data{
  c_base_player* localplayer;
  c_base_weapon* wep;

  c_base_entity* entity; // Current entity we're looking at.

  vec3 old_abs_origin; // The original position.
  vec3 shoot_pos; // Our actual shoot position.
  vec3 aim_angle; // Aim angle calculated with our shoot position.
  vec3 highest_simulated_pos; // Highest pos in proj sim.
  vec3 proj_shoot_pos; // Corrected shoot position with the projectile offset.
  vec3 proj_angle; // Eye angles calculated from the projectile shoot pos.

  vec3 aim_offset; // Not the projectile eye position offset. But the offset applied to aim_point (Does not apply get_correct_shoot_pos)
  vec3 aim_point; // Current aim point unaffected by gravity, get_correct_shoot_pos_info etc...
  vec3 target_aim_point; // Aim point affected by gravity and everything else.

  vec3 predicted_pos;
  vec3 predicted_obb;

  vec3 proj_min;
  vec3 proj_max;
  float travel_time;
  float proj_speed;
  float proj_gravity;

  bool in_water;
  bool performed_trajectory_vis_check; // True when the trajectory vis check was performed.
  bool obb_scan;
  ALWAYSINLINE void dump_info(){
    DBG("\n\n[!] localplayer: %X\tweapon:%X\n[!] Entity:%X\n[!] shoot_pos: %f, %f, %f\n[!] aim_angle: %f, %f, %f\n[!] aim_offset: %f, %f, %f\n[!] aim_point: %f, %f, %f\n[!] target_aim_point: %f, %f, %f\n[!] predicted_pos: %f, %f, %f\n[!] predicted_obb: %f, %f, %f\n[!] proj_min: %f, %f, %f\n[!] proj_max: %f, %f, %f\n[!] travel_time: %f\n[!] proj_speed: %f\n[!] proj_gravity: %f\n", localplayer, wep, entity, shoot_pos.x, shoot_pos.y, shoot_pos.z, aim_angle.x, aim_angle.y, aim_angle.z, aim_offset.x, aim_offset.y, aim_offset.z, aim_point.x, aim_point.y, aim_point.z, target_aim_point.x, target_aim_point.y, target_aim_point.z, predicted_pos.x, predicted_pos.y, predicted_pos.z, predicted_obb.x, predicted_obb.y, predicted_obb.z, proj_min.x, proj_min.y, proj_min.z, proj_max.x, proj_max.y, proj_max.z, travel_time, proj_speed, proj_gravity);
  }
};

//#define PROJ_DEBUG_GRAVITY
class c_projectile_aimbot{
public:
  c_key_control splash_key_bind;
  c_aimbot_filter_base* filter   = nullptr;
  i32 throwable_target_confirms  = 0;
  bool is_first_processed_entity = true;

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
      filter = new c_aimbot_filter_projectile_fov;
    else if(global->aimbot_settings->target_method_distance)
      filter = new c_aimbot_filter_projectile_distance;
  }

  bool should_predict_entity(s_current_target_data* entity);
  bool performs_trajectory_cast(s_current_target_data* proj_ctx);
  bool aimbot_only_mode_active(c_base_player* player);
  bool should_target_entity(c_base_entity* entity);
  bool crusaders_crossbow_logic_enabled();
  void crusaders_crossbow_logic();
  float get_max_travel_time();
  float get_min_hit_chance(float travel_time);

  bool calc_travel_time(vec3 shoot_pos, vec3 aim_point, float v, float& time);
  float get_splash_bot_blast_radius(c_base_weapon* wep);

  bool get_correct_shoot_pos_info(s_current_target_data* proj_ctx, bool skip_trajectory_vis_check, std::string str = "");
  bool calc_gravity_offset(s_current_target_data* proj_ctx, float* travel_time = nullptr);
  bool calc_aim_offset(s_current_target_data* proj_ctx);
  bool predict_projectile(s_entity_predict_ctx* ctx, s_current_target_data* proj_ctx);
  bool post_predict_projectile(s_entity_predict_ctx* ctx, s_current_target_data* proj_ctx);
  bool predict_evasion(s_current_target_data* proj_ctx, bool simulate = false);
  bool predict_splash_damage(s_current_target_data* proj_ctx);
  bool get_adjusted_aim_point(s_current_target_data* proj_ctx, s_trace* tr);
  bool get_best_aim_point(s_current_target_data* proj_ctx);

  bool is_projectile_trajectory_point_visible(s_current_target_data* proj_ctx, i32 flags = 0);
  bool is_projectile_trajectory_visible(s_current_target_data* proj_ctx, i32 flags = 0);
  bool on_projectile_trajectory_hit(s_current_target_data* proj_ctx, s_trace* tr, i32 flags = 0);

  bool process_splash_bot(s_trace* tr, s_current_target_data* proj_ctx, i32 flags);

  bool get_target(c_base_entity*& target, vec3& aim_point, s_current_target_data& td);

  bool should_release_weapon(c_base_player* target, vec3 shoot_pos, vec3 aim_point);
  bool should_force_release_weapon(c_base_player* target, vec3 shoot_pos, vec3 aim_point);
  bool should_bypass_hit_chance(c_base_player* entity, vec3 shoot_pos, vec3 angles, vec3& aim_point);
  bool do_beggars_logic(c_base_weapon* weapon);
  bool should_auto_shoot();
  bool should_disallow_crits(c_base_entity* target, c_base_weapon* wep);
  bool process_attack_buttons(c_base_weapon* wep, u32 type);
  bool crithack_force_wrapper(bool b);
  bool run();

  bool  heal_arrow_team_mates_only;
  bool  heal_arrow_team_mates_within_view;
  bool  proj_aimbot_ran;
  bool  found_sentry;
  bool  sentry_fires_rockets;
  vec3  sentry_shoot_pos;
};

CLASS_EXTERN(c_projectile_aimbot, projectile_aimbot);