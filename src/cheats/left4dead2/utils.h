#pragma once

class c_base_entity;
class c_base_player;
class c_player_esp_settings;
class c_player_chams_settings;
class c_entity_data;

namespace utils{
  void                     store_bone_data();
  c_player_esp_settings*   get_player_esp_settings(i32 type);
  c_player_esp_settings*   get_player_esp_settings(c_base_player* player);
  c_player_chams_settings* get_player_chams_settings(i32 type);
  c_player_chams_settings* get_player_chams_settings(c_base_player* player);

  bool                     reset_lag_comp_data(i32 index);
  void                     reset_all_lag_comp_data();
  bool                     reset_entity_data(i32 index);
  void                     reset_all_entity_data();
  void                     verify_entity_data(i32 index);
  void                     verify_all_entity_data();
  void                     check_for_steam_friends();
  void                     detect_objective_entity(c_base_entity* entity);
  c_entity_data*           get_entity_data(i32 index);
  void                     transmit_steamid();
  bool                     is_in_thirdperson();
};