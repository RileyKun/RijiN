#pragma once

#define CHEAT_DETECTION_RESET_INFRACTION_TIME 30.f
#define CHEAT_DETECTION_SUSPECT_TIME 15.f
#define CHEAT_DETECTION_INFRACTION_MAX 60
#define CHEAT_DETECTION_INFRACTION_SUSPECT 3

enum CHEATER_DETECTION_REASONS{
  DETECTION_REASON_INVALID = 0,
  DETECTION_TOO_MANY_INFRACTIONS = 5,
};

struct s_known_cheater_data{
  u32 sid;
  u32 name_hash;
  i32 player_index;
};

class c_cheat_detection{
public:

  bool is_response_cheater(u32 flag){
    return flag & CHEATER_FLAG_IS_KNOWN_CHEATER || flag & CHEATER_FLAG_KNOWN_CHEATER_IN_OTHER_GAME;
  }

  void transmit_steamid();
  void think(u32 stage);
  void run(c_base_player* player);

  void fire_bullets(c_base_entity* entity, s_fire_bullets_info* info, bool post);

  bool should_run_on_player(c_base_player* player, bool ignore_marked_cheaters = false);
  bool should_notify(c_base_player* player, bool* already_marked = nullptr);
  void on_infraction(c_base_player* player);
  void check_for_known_cheater(c_base_player* player);
  void on_known_cheater_data_received(i32 player_index, c_net_request_tf_cheater_sid32_response* response);
  void on_known_person_received(i32 player_index, c_net_request_tf_cheater_sid32_response* response);
  void process_steam_group_data(i32 player_index, c_net_request_tf_cheater_sid32_response* response);
  void setup_known_alias(i32 player_index, c_net_request_tf_cheater_sid32_response* response);
  bool is_cheating(c_base_player* player, bool exclude_playerlist = false);
  bool on_playerlist_cfg_load(c_base_player* player);
  void angle_snapped(c_base_player* player, c_player_data* data, float trigger_degree, float angle_repeat_degree = 2.5f);
  void snap_check(c_base_player* player);
  void snap_check(i32 attacker){
    c_base_player* player = global->entity_list->get_entity(attacker);
    if(player == nullptr)
      return;

    snap_check(player);
  }
  void on_death(i32 attacker, i32 victim);
  void on_cheater_detected(c_base_player* player, std::wstring reason, u32 reason_code, bool submit_cheater_to_network = true, bool disable_notification = false);
};

CLASS_EXTERN(c_cheat_detection, cheat_detection);