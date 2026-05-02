#pragma once

namespace utils{
  void                    lock_mouse(const bool lock);
  bool                    is_in_game();
  c_internal_base_player* localplayer();
  bool                    is_match_making_server();
  bool                    is_listen_server();
  bool                    is_community_server();
  void                    update_map_hash(i8* map_name);
  void*                   get_interface(std::wstring module, u32 name_hash, bool find_list = true);
  c_steamid               get_user_steamid(u32 friends_id);
  void                    send_con_notification(std::string str);
  void                    draw_high_ping_warning();
  void                    draw_hitbox_custom_matrix(i32 index, matrix3x4* bone_matrix, colour clr, float duration = 4.f);
  bool                    get_cheater_data_from_network(u32 sid, u32 game_hash, c_net_request_tf_cheater_sid32_response*& _response);
  void                    submit_cheater_to_network(u32 sid, u32 reason, u32 game_hash);
  void                    submit_bot_to_network(u32 sid);
  i32                     get_entity_index_from_sid(u32 sid);
  bool                    is_server_lagging();
  bool                    is_tick_rate_low();
  bool                    should_filter_server_ip(i8* ip, s_game_server_item* item, bool server_browser);
  float                   get_ideal_pixel_fov(c_internal_base_entity* entity);

  // Slow function calls
  i8*                     get_steam_name_from_sid(u32 sid);
  bool                    is_steam_friend(u32 sid);

  wchar_t*                get_team_relation(i32 team);
};