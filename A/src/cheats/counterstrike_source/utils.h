#pragma once

class c_base_entity;
class c_base_player;
class c_user_cmd;
class c_player_data;
class c_player_esp_settings;
class c_player_chams_settings;
class c_aimbot_settings;
class c_triggerbot_settings;
class c_steamid;
namespace utils{
  c_player_data*           get_player_data(i32 index);
  c_player_esp_settings*   get_player_esp_settings(i32 type);
  c_player_esp_settings*   get_player_esp_settings(c_base_player* player);
  c_player_chams_settings* get_player_chams_settings(i32 type);
  c_player_chams_settings* get_player_chams_settings(c_base_player* player);
  void                     check_to_reset_data(i32 index);
  void                     verify_player_data();
  bool                     reset_lag_comp_data(i32 index);
  void                     reset_all_lag_comp_data();
  bool                     reset_player_data(i32 index);
  void                     reset_all_player_data();
  bool                     wait_lag_fix(c_base_player* player);
  bool                     is_in_thirdperson();
  c_aimbot_settings*       get_aimbot_settings();
  c_triggerbot_settings*   get_triggerbot_settings();
  float                    get_latency();
  vec3                     trace_line(vec3 start, vec3 end, u32 mask = 0, u32 custom_trace_filter = 0);
};