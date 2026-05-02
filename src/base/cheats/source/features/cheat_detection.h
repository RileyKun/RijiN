#pragma once

#define MAX_INFO 128

enum CHEAT_INFO_FETCH_STATE{
  INFO_FETCH_NONE = 0,
  INFO_FETCHING   = 1,
  INFO_FETCHED    = 2,
};

enum CHEAT_NOTIFY_STATE{
  NOTIFY_STATE_NONE = 0,
  NOTIFY_STATE_NEEDED = 1,
};

enum SID_POSITION_STATE{
  SID_STATE_INVALID,
  SID_STATE_CONNECTING,
  SID_STATE_SPAWNED,
  SID_STATE_DISCONNECTED,
};

enum detection_type{
  DETECTION_OBB_PITCH = 1,
  DETECTION_ANGLE_ANALYTICAL = 2,
  DETECTION_TICKBASE_ABUSE = 3,
  DETECTION_SPEEDHACK = 4,
  DETECTION_MAX,
};

class c_base_detection_info;
class c_entity_info{
public:
  u32 friends_id              = 0;
  c_base_detection_info* info = nullptr;
};

class c_base_detection_info{
public:
  u32 game_hash;
  i32 first_time_index;
  i32 index;
  s_player_info info;

  u32  fetch_state;
  u32  position_state;
  u32  notify_position_state;
  bool has_submitted; // If we detected them cheating, then we wanna avoid sending again for a while.

  u32 cheater_flags;
  
  i32 bot_groups;
  i32 cheater_groups;

  bool has_alias;
  i8   alias[32];

  bool has_group;
  i8   group[32];

  i32   infractions[DETECTION_MAX];
  float infractions_reset_time[DETECTION_MAX];

  bool  steam_friend;
  i8    steam_name[33];
  i8    abb_steam_name[33];
  float next_steam_fetch_time;

  float last_fetched_time;


  // Like if we have a case where a player lags but is not cheating.
  float unstable_timer;
  float tickbase_infraction_wait; // If we get a detection lets not flood on_infraction call.
  float suspect_tickbase_occured_time; // Happens when current sim_ticks are somehow less than  last_simulation_ticks.
  float is_abusing_tickbase;

  i32   speedhack_violations;
  float speedhack_infraction_wait;

  vec3  last_origin;
  i32   last_simulation_ticks;

  vec3  current_viewangle;
  vec3  last_viewangle_1tick;
  vec3  last_viewangle_2tick;
  i32   angle_collection_ticks[2];

  NEVERINLINE bool can_check_angles(){
    return angle_collection_ticks[0] >= 3 && angle_collection_ticks[1] >= 3;
  }

  NEVERINLINE void reset_steam_names(){
    steam_name[0]     = '\0';
    abb_steam_name[0] = '\0';
  }

  NEVERINLINE void accessed(){
    last_fetched_time = math::time();
  }

  NEVERINLINE float accessed_delta(){
    return math::abs(last_fetched_time - math::time());
  }

  NEVERINLINE void reset_in_game_data(){
    first_time_index = 0;
    index = -1;
  }

  NEVERINLINE void reset(){
    DBG("[-] c_base_detection_info::reset full reset called %s %i\n", info.name, info.friends_id);
    memset(this, 0, sizeof(c_base_detection_info));
    accessed();
  }
};

class c_base_cheat_detection{
public:
  virtual u32 game_hash(){
    FUNCTION_OVERRIDE_NEEDED;
    return HASH("UNK");
  }

  virtual void reset(){
    DBG("[!] c_base_cheat_detection::reset\n");
    memset(info, 0, sizeof(info));
    for(u32 i = 0; i < MAX_INFO; i++){
      d_info[i].first_time_index = 0;
      d_info[i].index            = 0;
    }

    if(global_internal->base_log != nullptr){
      if(global_internal->base_log->infractions != nullptr)
        global_internal->base_log->infractions->clear_history();
    }
  }

  void add(wchar_t* buf, u32 sid = 0){
    if(global_internal->base_log != nullptr){
      if(global_internal->base_log->infractions != nullptr){
        s_log_window_metadata* metadata = global_internal->base_log->infractions->add(buf);
        if(sid != 0){
          metadata->set_type(LOG_WINDOW_STEAM_URL)->
          set_value(sid);
        }
      }
    }
  }

  void add_if_empty(wchar_t* buf){
    if(global_internal->base_log != nullptr){
      if(global_internal->base_log->infractions != nullptr)
        global_internal->base_log->infractions->add_if_empty(buf);
    }
  }

  virtual void d3d9_hook(){
    transmit_iden();
  }
  virtual void transmit_iden();
  virtual void calculate_should_run();
  virtual void frame_stage_notify(u32 stage);
  virtual void on_fire_event(c_game_event* event);

  virtual void fill_player_info(i32 index);
  virtual void check_entity(i32 index);
  virtual void fetch_steam_data(c_base_detection_info* info);
  virtual void decay_infractions(c_base_detection_info* info, u32 type);

  virtual bool should_run_on_player(c_internal_base_player* player);
  virtual void analyze_angles(c_internal_base_player* player, c_entity_info* data);
  virtual void check_angles_for_inhuman(c_internal_base_player* player, c_internal_base_entity* victim, float aimbot_snap = 45.f, float angle_repeat_snap = 1.f);
  virtual void check_angles_for_inhuman(i32 index, i32 victim = 0, float aimbot_snap = 45.f, float angle_repeat_snap = 1.f){
    check_angles_for_inhuman((c_internal_base_player*)global_internal->entity_list->get_entity(index), global_internal->entity_list->get_entity(victim), aimbot_snap, angle_repeat_snap);
  }
  virtual void analyze_movement(c_internal_base_player* player, c_entity_info* data);
  virtual void on_infraction_received(c_internal_base_player* player, u32 type);
  virtual void on_cheater_detected(c_internal_base_player* player, u32 type);

  virtual bool is_cheating(i32 index, bool exclude_player_list = false);
  virtual bool is_cheating_bot(i32 index, bool exclude_player_list = false);
  virtual bool is_friend(i32 index, bool exclude_player_list = false);
  virtual i8*  get_steam_name(i32 index, bool abb = false);

  virtual void fetch_server_info();
  virtual void process_notify(c_base_detection_info* data);
  virtual void engine_get_player_info_hook(i32 entity_id, s_player_info* info);
  virtual void on_float_decode(c_internal_base_entity* entity, u32 name_hash, void* data);
  virtual bool should_detect_obb_pitch(){
    return game_hash() == HASH("TF2");
  }

  virtual float get_obb_pitch_value(){
    return 90.f;
  }

  virtual bool should_detect_movement(){
    return game_hash() == HASH("L4D2");
  }

  virtual bool is_weapon_allowed_for_angles(c_internal_base_weapon* weapon){
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  virtual bool is_weapon_allowed_for_repeat(c_internal_base_weapon* weapon){
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  NEVERINLINE c_entity_info* get_entity_info(i32 index){
    if(index < 0 || index >= MAX_INFO)
      return nullptr;

    c_entity_info* data = &info[index];
    if(data->friends_id == 0 || data->info == nullptr)
      return nullptr;

    return data;
  }

private:
  NEVERINLINE std::wstring get_detection_type(u32 type){
    switch(type){
      default: break;
      case DETECTION_OBB_PITCH: return WXOR(L"OB PITCH");
      case DETECTION_ANGLE_ANALYTICAL: return WXOR(L"ANGLE ANALYSIS");
      case DETECTION_TICKBASE_ABUSE: return WXOR(L"TICKBASE ABUSE");
      case DETECTION_SPEEDHACK: return WXOR(L"SPEEDHACK");
    }

    return WXOR(L"UNK");
  };
  NEVERINLINE void setup_entity(i32 index, c_base_detection_info* data){
    if(!data->info.friends_id)
      return;

    info[index].friends_id = data->info.friends_id;
    info[index].info       = data;

    DBG("[+] c_base_cheat_detection::setup_entity %i %p\n", index, data);
  }

  NEVERINLINE void unlink(i32 index){
    if(index < 0 || index >= MAX_INFO)
      return;

    info[index].friends_id = 0;
    info[index].info       = nullptr;
  }

  NEVERINLINE void unlink_entity_index_by_sid(u32 sid){
    for(u32 i = 0; i < MAX_INFO; i++){
      if(info[i].friends_id == sid){
        DBG("[!] %i is no longer owned by %i\n", i, sid);
        info[i].friends_id = 0;
        info[i].info = nullptr;
        break;
      }
    }
  }

  c_base_detection_info* assign_info(u32 sid, std::wstring location = L""){
    if(!sid)
      return nullptr;

    DBG("[!] c_base_cheat_detection::assign_info: %i (%ls)\n\n", sid, location.c_str());
    c_base_detection_info* info = get_info(sid);
    if(info != nullptr){
      DBG("[!] c_base_cheat_detection::assign_info %i vs %i\n", sid, info->info.friends_id);
      return info; 
    }
    
    for(u32 i = 0; i < MAX_INFO; i++){
      if(!d_info[i].info.friends_id){
        d_info[i].reset();
        d_info[i].game_hash = game_hash();
        d_info[i].info.friends_id = sid;
        d_info[i].accessed();
        return &d_info[i];
      }
    }

    i32 best_index = 0;
    float biggest_delta = 0.f;

    for(u32 i = 0; i < MAX_INFO; i++){
      if(biggest_delta != 0.f && d_info[i].accessed_delta() < biggest_delta)
        continue;

      best_index = i;
      biggest_delta = d_info[i].accessed_delta();
    }

    DBG("[-] assign_info overflowed: [U:1:%i] assigning data to entry %i\n", sid, best_index);
    d_info[best_index].reset();
    d_info[best_index].game_hash = game_hash();
    d_info[best_index].info.friends_id = sid;
    d_info[best_index].accessed();
    return &d_info[best_index];
  }

  c_base_detection_info* get_info(u32 sid){
    if(!sid)
      return nullptr;
    
    DBG("[-] c_base_cheat_detection::get_info: %i\n", sid);
    for(u32 i = 0; i < MAX_INFO; i++){
      if(!d_info[i].info.friends_id)
        return nullptr;

      if(d_info[i].info.friends_id == sid)
        return &d_info[i];
    }

    return nullptr;
  }

  c_entity_info         info[MAX_INFO];
  c_base_detection_info d_info[MAX_INFO];

  bool _should_run_detections;
};