#pragma once

#define MAX_PRED_TRACK_ENTRIES 930
#define MAX_KNOWN_ACTOR_NAME_SIZE 32

#if defined(GMOD_CHEAT)
  #define MAXPLAYERS 128
  #define MAX_BONES 256
#else
  #define MAXPLAYERS 101
  #define MAX_BONES 128
#endif

#if defined(DEV_MODE)
  #define FUNCTION_OVERRIDE_NEEDED DBG("[-] %s needs to be overwritten with its own function!\n", __func__);
#else
  #define FUNCTION_OVERRIDE_NEEDED
#endif

// For info panel.
struct s_info{
  wchar_t      name[1024];
  std::wstring status;
  colour       clr;
  float        time;
};

struct s_pointer_hash_data{
  void* pointer;
  u32   hash;
};

struct s_pointer_hash_table{
  s_pointer_hash_data* table;
  i32 count;
};

struct s_webhook_data{
  i8 url[4096];
  i8 msg[4096];
};

struct s_pred_track_entry{
  vec3 pos;
};

struct s_active_headshot_threat{
  i32 threats[2];
  i32 vis_threats[2];
  i32 get_threats(bool cheaters_only, bool vis_check){
    i32 i = cheaters_only ? 1 : 0;
    return vis_check ? vis_threats[i] : threats[i];
  }
};

struct s_pred_track_array{
  u32 size;
  s_pred_track_entry entries[MAX_PRED_TRACK_ENTRIES];

  void reset(){
    size = 0;
  }

  bool add(vec3 pos){
    if(size >= MAX_PRED_TRACK_ENTRIES)
      return false;

    s_pred_track_entry* new_entry = &entries[size++];
    {
      new_entry->pos = pos;
    }

    return true;
  }
};

// Explained in resolver data.
struct s_resolver_shot_data{
  bool  using_pitch_antiaim;
  bool  using_yaw_antiaim;
  i32   real_ang_updated;
  bool  should_run;
  #if defined(TF2_CHEAT)
  bool  is_taunting;
  #endif
  float timeout;
};

// base classes

class c_base_entity_data{
public:
  vec3      obb_min;
  vec3      obb_max;
  bool      has_transform;
  matrix3x4 transform;

  bool      has_bone_matrix;
  matrix3x4 bone_matrix[MAX_BONES];

  float last_simulation_time;
};

class c_base_player_data{
public:
  vec3      obb_min;
  vec3      obb_max;
  bool      has_transform;
  matrix3x4 transform;
  u32       friends_id;

  float fake_latency_timer;

  bool  name_cross_check;
  bool  sent_newlined_chat;
  bool  muted;

  // Resolver
  s_resolver_shot_data shot_data;
  float resolver_yaw_active_time;
  i32   resolver_pitch_cycle;
  i32   resolver_yaw_cycle;

  i32  real_ang_updated;
  i32  resolver_movement_factor;

  vec3 real_ang;
  vec3 networked_ang;
  bool has_received_network_ang;

  float last_raw_yaw_delta;
};

class c_base_game_specific_data{
public:
  bool unloading             = false;

  // Should be used for for testing only.
  c_internal_base_player* localplayer              = nullptr;
  i32            localplayer_team         = 0;
  i32            localplayer_index        = 0;
  i32            localplayer_weapon_index = 0;

  c_user_cmd*     current_cmd     = nullptr;
  c_user_cmd      untouched_cmd;
  c_user_cmd      original_cmd;
  c_user_cmd      last_cmd;
  i32             predicted_tick_base;

  bool is_talking_to_tfcl    = false;

  s_view_setup* view_setup   = nullptr;
  matrix4x4     view_matrix;
  matrix4x4     view_matrix_untouched;
  vec3i         ws2_position_offset;

  // ignoring get_player_info hook
  bool skip_get_player_info_hook          = false;
  
  // ignoring 'hud_chatline_insert_and_colorize_text' hook
  bool skip_chatline_insert_hook          = false;

  // When we first get in-game it'll check every player's friends id for a mismatch.
  bool verified_player_data;
  bool verified_entity_data;

  // prediction related
  bool            is_firing               = false;

  // Set at the bottom of createmove.
  bool            is_choking_cmd          = false;

  // packet angle tracking.
  vec3 last_sent_angle;
  vec3 last_choked_angle;
  vec3 both_angles;
  vec3 fire_angle;
  float reset_fire_angle_time;
  
  // packet choking
  bool*           send_packet                             = nullptr;
  i8*             move_ret_addr                           = nullptr;
  i32             last_ping                               = 0;
  bool            network_lag                             = false;

  bool            speedhack_active        = false;
  bool            speedhack_tick          = false;
  void*           move_helper             = nullptr;

  bool            did_run_prediction      = false;
  bool            is_calling_run_command  = false;

  bool            allow_double_tap        = false;
  bool            double_jumping          = false;
  s_global_data   old_global_data;
  u32             old_prediction_seed;

  // Aimbot target
  vec3  aimbot_target_aim_point;
  i32   aimbot_target_index      = -1;
  float aimbot_target_reset_time = 0.f;
  u32   aimbot_target_type       = 0;

  // rendering
  bool is_setting_stencils = false;

  // Current steamid being used right now.
  u32 current_steamid                  = 0;
  float next_steamid_update_check_time = 0.f;

  s_active_headshot_threat headshot_threats;

  i8   current_map[1024];
  u32  current_map_hash;

  bool playerlist_has_valid_player     = false;
  i32  playerlist_current_player_index = -1;

  // We can use this to check what entity we're looking at without doing a billion traces.
  s_trace lookat_target;
  i32 lookat_target_entindex  = 0;
  i32 last_update_lookat_tick = 0;
};

class c_font_d3d9; // Assume d3d9 for source engine.
class c_base_lag_compensation;
class c_base_cheat_detection;
class c_base_packet_manager;
class c_base_anti_cheat_manager;
class c_base_log_manager;
class c_base_dispatch_user_message_features;
class c_base_global : public c_base_game_specific_data{
public:
  uptr                            focus_overlay_panel_id  = 0;
  bool                            safe_mode               = false;
  bool                            menu_open               = true;
  void*                           target_window           = nullptr;

  void*                           base_address_start = nullptr;
  void*                           base_address_end   = nullptr;

  s_global_data*                  global_data        = nullptr;
  c_interface_viewport*           viewport           = nullptr;
  c_interface_client*             client             = nullptr;
  c_interface_entity_list*        entity_list        = nullptr;
  c_interface_engine*             engine             = nullptr;
  c_interface_engine_tool*        engine_tool        = nullptr;
  c_interface_model_info*         model_info         = nullptr;
  c_interface_panel*              panel              = nullptr;
  c_interface_cvar*               cvar               = nullptr;
  c_interface_prediction*         prediction         = nullptr;
  c_interface_material_system*    material_system    = nullptr;
  c_physics_interface*            physics            = nullptr;
  c_physics_collision_interface*  physics_collision  = nullptr;
  c_game_manager*                 game_manager       = nullptr;
  c_view_render_beams*            view_render_beams  = nullptr;
  c_interface_engine_render_view* render_view        = nullptr;
  c_interface_engine_model*       engine_model       = nullptr;
  c_base_interface_trace*         base_trace         = nullptr;
  c_steamapicontext*              steamcontext       = nullptr;
  c_base_hud_chat*                hud_chat           = nullptr;


  c_cvar*                         cl_predict                          = nullptr;
  c_cvar*                         sv_cheats                           = nullptr;
  c_cvar*                         cl_updaterate                       = nullptr;
  c_cvar*                         cl_interp_ratio                     = nullptr;
  c_cvar*                         cl_interp                           = nullptr;
  c_cvar*                         sv_airaccelerate                    = nullptr;
  c_cvar*                         sv_friction                         = nullptr;
  c_cvar*                         sv_stopspeed                        = nullptr;
  c_cvar*                         sv_maxvelocity                      = nullptr;
  c_cvar*                         sv_client_min_interp_ratio          = nullptr;
  c_cvar*                         sv_client_max_interp_ratio          = nullptr;
  c_cvar*                         cl_interpolate                      = nullptr;
  c_cvar*                         cl_connectmethod                    = nullptr;
  c_cvar*                         sv_gravity                          = nullptr;
  c_cvar*                         fps_max                             = nullptr;
  c_cvar*                         r_drawviewmodel                     = nullptr;
  c_cvar*                         cl_wpn_sway_scale                   = nullptr;
  c_cvar*                         cl_wpn_sway_interp                  = nullptr;
  c_cvar*                         voice_loopback                      = nullptr;


  void*                           run_command_hook_trp                = nullptr;
  void*                           post_think_hook_trp                 = nullptr;
  void*                           process_impacts_hook_trp            = nullptr;
  void*                           cl_move_hook_trp                    = nullptr;

  // Pointers to functions
  void*                           md5_pseudorandom                          = nullptr;
  i32*                            prediction_random_seed                    = nullptr;
  void*                           shared_random_int_addr                    = nullptr;
  void*                           cl_runprediction                          = nullptr;
  void*                           set_abs_origin                            = nullptr;
  void*                           set_abs_angles                            = nullptr;
  void*                           update_clientside_animations              = nullptr;
  s_client_state*                 client_state                              = nullptr;
  void*                           keyvalues_init                            = nullptr;
  void*                           keyvalues_setname                         = nullptr;
  void*                           keyvalues_setint                          = nullptr;
  void*                           keyvalues_setfloat                        = nullptr;
  void*                           keyvalues_setstring                       = nullptr;
  void*                           cl_move_test_dil_addr                     = nullptr;
  void*                           cl_move_send_packet_false_addr            = nullptr;
  void*                           baseclientstate_processstringcmd_retaddr1 = nullptr;
  void*                           steam3client_addr                         = nullptr;


  // base class pointers to certain features.
  c_base_dispatch_user_message_features* base_dispatch_user_msg         = nullptr;
  c_base_lag_compensation*               base_lag_compensation          = nullptr;
  c_base_cheat_detection*                base_cheat_detection           = nullptr;
  c_base_packet_manager*                 base_packet_manager            = nullptr;
  c_base_anti_cheat_manager*             base_acm                       = nullptr;
  c_base_log_manager*                    base_log                       = nullptr;

  // Used for FOV calculations.
  float           game_fov                = 0.f;

  // ESP fonts
  c_font_d3d9*    esp_font_large          = nullptr;
  c_font_d3d9*    esp_font_small          = nullptr;

  // GUI Related
  c_font_d3d9*                   gui_font                  = nullptr;
  c_font_d3d9*                   gui_font_emoji            = nullptr;

  ALWAYSINLINE void init(void* trace){
    assert(cvar != nullptr);
    assert(base_interface != nullptr);

    cookie_block_check_return();

    entity_info        = new c_base_entity_info_manager;

    #if defined(GMOD_CHEAT)
      entity_info->init(XOR32(8912));
    #elif defined(L4D_CHEAT)
      entity_info->init(XOR32(4096));
    #else
      entity_info->init(XOR32(2048));
    #endif

    cl_predict         = cvar->find_var(XOR("cl_predict"));
    sv_gravity         = cvar->find_var(XOR("sv_gravity"));
    sv_cheats          = cvar->find_var(XOR("sv_cheats"));
    cl_connectmethod   = cvar->find_var(XOR("cl_connectmethod"));
    cl_updaterate      = cvar->find_var(XOR("cl_updaterate"));
    cl_interp_ratio    = cvar->find_var(XOR("cl_interp_ratio"));
    cl_interp          = cvar->find_var(XOR("cl_interp"));
    cl_interpolate     = cvar->find_var(XOR("cl_interpolate"));
    sv_airaccelerate   = cvar->find_var(XOR("sv_airaccelerate"));
    sv_friction        = cvar->find_var(XOR("sv_friction"));
    sv_stopspeed       = cvar->find_var(XOR("sv_stopspeed"));
    sv_maxvelocity     = cvar->find_var(XOR("sv_maxvelocity"));
    fps_max            = cvar->find_var(XOR("fps_max"));
    r_drawviewmodel    = cvar->find_var(XOR("r_drawviewmodel"));
    cl_wpn_sway_scale  = cvar->find_var(XOR("cl_wpn_sway_scale"));
    cl_wpn_sway_interp = cvar->find_var(XOR("cl_wpn_sway_interp"));
    voice_loopback     = cvar->find_var(XOR("voice_loopback"));

    sv_client_min_interp_ratio = cvar->find_var(XOR("sv_client_min_interp_ratio"));
    sv_client_max_interp_ratio = cvar->find_var(XOR("sv_client_max_interp_ratio"));

    base_trace = (c_base_interface_trace*)trace;

    // hooks
    run_command_hook_trp           = GET_PKG("run_command_hook");
    post_think_hook_trp            = GET_PKG("post_think_hook");
    cl_move_hook_trp               = GET_PKG("cl_move_hook");
    #if defined(__x86_64__)
    process_impacts_hook_trp       = GET_PKG("process_impacts_hook");
    #endif
    // signatures.
    #if defined(__x86_64__)
      base_address_start = GET_PKG_NER("base_address_start64");
      base_address_end   = GET_PKG_NER("base_address_end64");
    #else
      base_address_start = GET_PKG_NER("base_address_start");
      base_address_end   = GET_PKG_NER("base_address_end");
    #endif

    global_data                               = GET_PKG("globaldata");
    md5_pseudorandom                          = GET_PKG("md5_pseudorandom");

    #if defined(__x86_64__)
      prediction_random_seed                    = GET_PKG("prediction_random_seed");
    #else
      prediction_random_seed                    = *(i32*)GET_PKG("prediction_random_seed");
    #endif
    shared_random_int_addr                    = GET_PKG_NER("shared_random_int_addr");
    cl_runprediction                          = GET_PKG("cl_runprediction");
    keyvalues_init                            = GET_PKG("keyvalues_init");
    keyvalues_setname                         = GET_PKG("keyvalues_setname");
    keyvalues_setint                          = GET_PKG("keyvalues_setint");
    keyvalues_setfloat                        = GET_PKG("keyvalues_setfloat");
    keyvalues_setstring                       = GET_PKG("keyvalues_setstring");
    set_abs_origin                            = GET_PKG("set_abs_origin");
    set_abs_angles                            = GET_PKG("set_abs_angles");

#if defined(__x86_64__)
    cl_move_test_dil_addr                     = GET_PKG("cl_move_test_dil_addr");
    cl_move_send_packet_false_addr            = GET_PKG("cl_move_send_packet_false_addr");
#endif

    update_clientside_animations              = GET_PKG_NER("update_clientside_animations");
    baseclientstate_processstringcmd_retaddr1 = GET_PKG_NER("baseclientstate_processstringcmd_retaddr1");
    steam3client_addr                         = GET_PKG("steam3client_addr");
  }

  virtual void update_look_at_trace(c_user_cmd* cmd);
  virtual c_internal_base_player* update_local_data();

  NEVERINLINE void post_create_move(c_user_cmd* cmd, bool choking){
    is_choking_cmd = choking;

    if(!choking)
      last_sent_angle   = cmd->view_angles;
    else
      last_choked_angle = cmd->view_angles;

    both_angles = cmd->view_angles;
  }

  NEVERINLINE void pre_create_move(c_user_cmd* cmd){
    if(cmd == nullptr){
      DBG("[-] c_base_global::pre_create_move cmd is nullptr\n");
      return;
    }

    cookie_block_check_return();

    update_local_data();

    current_cmd = cmd;
    memcpy(&last_cmd, &original_cmd, sizeof(c_user_cmd));
    memcpy(&untouched_cmd, cmd, sizeof(c_user_cmd));
    memcpy(&original_cmd, cmd, sizeof(c_user_cmd));

    static i32 last_tick_count = 0;

    assert(global_data != nullptr);
    speedhack_tick  = (last_tick_count == global_data->tick_count);
    last_tick_count = global_data->tick_count;

    #if defined(DEV_MODE)
      if(speedhack_tick){
        DBG("[!] c_base_global::pre_create_move speedhack_tick is active (false pos on game pauses)\n");
      }
    #endif

    // If the map isn't setup yet, then lets get the details about it.
    if(!current_map_hash)
      utils::update_map_hash(engine->get_level_name());

    update_look_at_trace(cmd);
  }

  virtual void paint_traverse(u32 panel_hash, uptr panel){
    if(focus_overlay_panel_id == 0 && panel_hash == HASH("FocusOverlayPanel"))
      focus_overlay_panel_id = panel;
  }
};

CLASS_EXTERN(c_base_global, global_internal);