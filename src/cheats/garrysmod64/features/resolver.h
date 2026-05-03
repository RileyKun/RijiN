#pragma once

enum resolver_response_types{
  RESOLVER_RESPONSE_TYPE_BROADCASTING = 0,
  RESOLVER_RESPONSE_TYPE_YAW_MISS,
  RESOLVER_RESPONSE_TYPE_PITCH_MISS,
  RESOLVER_RESPONSE_TYPE_REAL_PITCH,
  RESOLVER_RESPONSE_TYPE_REAL_YAW,

};


#define RESOLVER_MAX_PITCH_CYCLE 0
#define RESOLVER_MAX_YAW_CYCLE 3

#define RESOLVER_REAL_UPDATE_PITCH (1 << 1)
#define RESOLVER_REAL_UPDATE_YAW (1 << 2)

#define RESOLVER_INVALIDATE_PITCH 0
#define RESOLVER_INVALIDATE_YAW 1
#define RESOLVER_INVALIDATE_ALL 2

#define RESOLVER_MANUAL_PITCH 0
#define RESOLVER_MANUAL_YAW 1

class c_resolver{
public:
  void process_shots(u32 stage);
  void apply_angles(void* ref, float* pitch, float* yaw);
  void on_shot(c_base_player* player);
  void detect_yaw_antiaim(c_base_player* player);
  void fire_event(c_game_event* event);
  void reset_shot_data(c_base_player* player, bool update_real_angle);
  void on_updated_real_angle(c_base_player* player, vec3 ang, i32 update_type, bool transmit = true);
  void on_invalidated_real_angle(c_base_player* player, c_player_data* data, i32 type);

  void detect_sniper_dot();
  void process_manual_input();
  bool find_manual_yaw_target();
  bool is_valid_manual_target(c_base_player* player);
  bool is_manual_input_mode_active();

  std::wstring get_current_pitch_correction_str(c_base_player* player);
  std::wstring get_current_yaw_correction_str(c_base_player* player);

  bool is_resolver_active(c_base_player* player);
  bool is_shot_accurate();
  bool should_run(c_base_player* player);
  bool is_using_pitch_aa(c_base_player* player){
    if(player == nullptr)
      return false;

    vec3 ang = player->viewangles();
    ang.x = math::clamp(math::normalize_angle(ang.x), -90.f, 90.f);
    return math::abs(ang.x >= 87.f);
  }

  bool is_playerlist_correcting(u8 angle_type, c_base_player* player);
};

CLASS_EXTERN(c_resolver, resolver);