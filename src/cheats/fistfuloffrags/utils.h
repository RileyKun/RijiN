#pragma once

class c_base_entity;
class c_base_player;
class s_user_cmd;
class c_player_data;
class c_player_esp_settings;
class c_player_chams_settings;
class c_steamid;
namespace utils{
  void                     lock_mouse(bool lock);
  void*                    get_interface(std::wstring module, u32 name_hash, bool find_list = true);
  c_base_player*           localplayer();
  void                     store_cmd_data(s_user_cmd* cmd);
  c_player_data*           get_player_data(i32 index);
  c_player_esp_settings*   get_player_esp_settings(i32 type);
  c_player_esp_settings*   get_player_esp_settings(c_base_player* player);
  c_player_chams_settings* get_player_chams_settings(i32 type);
  c_player_chams_settings* get_player_chams_settings(c_base_player* player);
  void                     check_to_reset_data(i32 index);
  void                     verify_player_data();
  void                     check_for_steam_friends();
  bool                     reset_lag_comp_data(i32 index);
  void                     reset_all_lag_comp_data();
  bool                     reset_player_data(i32 index);
  void                     reset_all_player_data();
  c_steamid                get_user_steamid(u32 friends_id);
  void                     send_con_notification(i8* txt);
  bool                     is_teamplay_enabled();
  bool                     wait_lag_fix(c_base_player* player);
};