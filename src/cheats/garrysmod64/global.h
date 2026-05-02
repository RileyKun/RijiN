#pragma once

#define GLOBAL_SETUP() if(global == nullptr) global = new c_global;

class gcs_menu;
class gcs_info_panel;

struct s_info{
  std::wstring name; // Name of the information.
  std::wstring status; // Status. Zero length makes it unused.
  colour       clr;
  float        time;  // Time it was added.
};

struct s_gmod_data{
  wchar_t weapon_name[256];
  u32     weapon_class_hash;
  bool    is_melee;
  bool    is_harmless; // tools and such.
  bool    is_semi_auto;

  // Lua
  float   next_lua_fetch;
  wchar_t unique_name[512];
  wchar_t user_group_str[1024];
  u32     user_group_hash;
  bool    has_god_mode;
  bool    is_perpheads_police;
  bool    is_perpheads;

  i32     armor;
};

// Explained in resolver data.
struct s_resolver_shot_data{
  bool  using_pitch_antiaim;
  bool  using_yaw_antiaim;
  i32   real_ang_updated;
  bool  should_run;
  float timeout;
};

class c_player_list_data{
public:
  void init(){
    memset(this, 0, sizeof(c_player_list_data));
    mark_as_auto      = true;
    mark_as_friend    = false;
    ignore_player     = false;

    resolver_auto         = true;
    resolver_disallow     = false;
    resolver_force        = false;

    resolver_pitch_auto         = true;
    resolver_pitch_up           = false;
    resolver_pitch_down         = false;
    resolver_pitch_zero         = false;
        
    resolver_yaw_auto           = true;
    resolver_yaw_neg90          = false;
    resolver_yaw_pos90          = false;
    resolver_yaw_neg45          = false;
    resolver_yaw_pos45          = false;
    resolver_yaw_lookat         = false;
    resolver_yaw_180away        = false;

    custom_esp_colour           = false;
    custom_chams_base_colour    = false;
    custom_chams_overlay_colour = false;
    custom_glow_colour          = false;
    priority_mode               = false;
    priority                    = 0;

    // We want to ensure the data is initilized no black colours allowed.
    {
      esp_colour[0] = 96;
      esp_colour[1] = 255;
      esp_colour[2] = 164;
      esp_colour[3] = 255;
    }

    // Base Chams colour
    {
      chams_colour[0] = 96;
      chams_colour[1] = 255;
      chams_colour[2] = 164;
      chams_colour[3] = 32;
    }

    // Overlay Chams colour
    {
      chams_overlay_colour[0] = 96;
      chams_overlay_colour[1] = 255;
      chams_overlay_colour[2] = 164;
      chams_overlay_colour[3] = 32;
    }

    // Glow colour
    {
      glow_colour[0] = 96;
      glow_colour[1] = 255;
      glow_colour[2] = 164;
      glow_colour[3] = 255;
    }


    valid             = true;
  }

  union{
    struct{
      bool mark_as_auto;
      bool mark_as_friend;
      bool mark_as_cheater;
      bool ignore_player;

      bool custom_esp_colour;
      bool custom_chams_base_colour;
      bool custom_chams_overlay_colour;
      bool custom_glow_colour;

      CONFIG_RGB(esp_colour, 96, 255, 164);
      CONFIG_RGBA(chams_colour, 96, 255, 164, 32);
      CONFIG_RGBA(chams_overlay_colour, 96, 255, 164, 32);
      CONFIG_RGB(glow_colour, 96, 255, 164);

      bool priority_mode;
      i32  priority;

      bool resolver_auto;
      bool resolver_disallow;
      bool resolver_force;

      bool resolver_pitch_auto;
      bool resolver_pitch_up;
      bool resolver_pitch_down;
      bool resolver_pitch_zero;

      bool resolver_yaw_auto;
      bool resolver_yaw_neg90;
      bool resolver_yaw_pos90;
      bool resolver_yaw_neg45;
      bool resolver_yaw_pos45;
      bool resolver_yaw_lookat;
      bool resolver_yaw_180away;

      bool valid = false;
    };

    PAD(0x1000);
  };
};

class c_entity_list_data{
public:
  bool                   use_custom_settings;
  bool                   aimbot_entity;
  bool                   filter_from_aimbot_vis_check;
  bool                   render;
  c_world_esp_settings   esp_settings;
  c_world_chams_settings chams_settings;
  c_world_glow_settings  glow_settings;
};

class c_player_data{
public:
  matrix3x4          transform;
  bool               has_transform;
  bool               has_bones;
  u32                friends_id;
  bool               steam_friend;
  float              next_steam_friend_check;
  float              grabbed_by_physgun_sim_time;
  bool               grabbed_by_physgun;
  s_gmod_data        gmod_data;

  bool               is_cheater;
  bool               has_alias;
  i8                 alias[32];
  bool               is_dev_esp;
  bool               is_dev_ignored;
  i32                cheater_groups;
  i32                bot_groups;
  float              infraction_reset;
  float              suspicious_timer;
  i32                infractions;
  bool               has_submitted_cheater_to_network;

  c_player_list_data playerlist;
  u32                playerlist_hash;
  u32                playerlist_cache_hash;
  bool               playerlist_notify;

  bool               seen_localplayer;
  bool               checked_cheater_list;
  bool               is_rijin_user;

  float               last_pred_change;
  float               pred_change;
  float               pred_change_delta;
  float               pred_last_yaw_delta;

  // resolver
  s_resolver_shot_data shot_data;
  float resolver_yaw_active_time;
  i32   resolver_pitch_cycle;
  i32   resolver_yaw_cycle;

  i32  real_ang_updated;
  i32  resolver_movement_factor;
  i32  resolver_miss_counter;

  vec3 real_ang;
  vec3 networked_ang;
};

class c_entity_data{
public:
  matrix3x4              bone_matrix[MAX_SETUP_BONES];
  bool                   has_bones;
  bool                   has_bone_matrix;
  float                  last_simulation_time;
  matrix3x4              transform;
  bool                   has_transform;
  u32                    class_name_hash;
  c_entity_list_data     list_data;
  u32 attachment_count;
  u32 attachment_index_table[32];
};

class c_game_specific_data{
public:
  c_player_data player_data[129];
  c_entity_data entity_data[8912];
  s_view_setup*  view_setup = nullptr;
  matrix4x4      view_matrix;

  c_base_player* localplayer;
  c_base_weapon* localplayer_wep;
  i32            localplayer_index;
  i32            localplayer_team;

  // packet choking
  bool* send_packet   = nullptr;
  i8*   move_ret_addr = nullptr;

  // command related
  bool            speedhack_tick          = false;
  void*           move_helper             = nullptr;
  s_user_cmd*     current_cmd             = nullptr;
  s_user_cmd      untouched_cmd;
  s_user_cmd      original_cmd;
  s_user_cmd      last_cmd;
  i32             predicted_tick_base     = 0;

  // prediction related
  bool                did_run_prediction      = false;
  bool                is_calling_run_command  = false;
  s_global_data       old_global_data;
  u32                 old_prediction_seed;
  s_fire_bullets_info bullets_info_dump;

  // player data verification
  bool verified_player_data               = false;

  c_player_list_data  playerlist_data;
  bool                playerlist_has_valid_player     = false;
  i32                 playerlist_current_player_index = -1;

  // Aimbot target
  vec3  aimbot_target_aim_point;
  i32   aimbot_target_index       = -1;
  float aimbot_target_reset_time  = 0.f;
  bool  override_material_call    = false;
  float game_fov                  = 90.f;

  // anti-backtrack feature for fakelag
  bool anti_backtrack_choke;
  bool anti_backtrack_choke_started;

  // peek feature for fakelag
  bool peek_fakelag_choke;
  bool peek_fakelag_choke_started;

  // resolver
  i32 resolver_manual_target = 0;

  vec3 both_angles;
  vec3 last_sent_angle;
  vec3 last_choked_angle;
  bool firing_this_tick;
  bool is_choking_cmd;

  ALWAYSINLINE void reset(){
    DBG("[!] game data reset\n");

    utils::reset_all_lag_comp_data();
    utils::reset_all_player_data();
    utils::reset_all_entity_data();

    memset(this, 0, XOR32(sizeof(c_game_specific_data)));
    input_system->reset();
  }

  ALWAYSINLINE void on_aimbot_target(i32 index, vec3 aim_point, c_base_weapon* weapon){
    if(weapon == nullptr)
      return;

    aimbot_target_aim_point  = aim_point;
    aimbot_target_index      = index;
    aimbot_target_reset_time = math::time() + 0.5f;
  }
};

#define globaldata global->global_data
#define config global->settings
class c_global : public c_game_specific_data{
public:
  // GUI Related
  c_font_d3d9*          gui_font                = nullptr;
  c_font_d3d9*          gui_font_emoji          = nullptr;
  gcs_info_panel*       info_panel              = nullptr;
  s_info                info[128];
  void*                 target_window           = nullptr;
  gcs_menu*             menu                    = nullptr;
  bool                  menu_open               = false;
  i64                   focus_overlay_panel_id  = 0;
  bool                  unloading               = false;
  u32                   current_steamid         = 0;
  c_game_event_listener listeners;
  bool                  is_talking_to_tfcl;

  // ESP Related fonts
  c_font_d3d9* esp_font_large = nullptr;
  c_font_d3d9* esp_font_small = nullptr;

  // settings
  c_settings* settings = nullptr;

  // Interfaces
  c_interface_client*             client             = nullptr;
  c_interface_engine*             engine             = nullptr;
  c_interface_prediction*         prediction         = nullptr;
  c_interface_engine_tool*        engine_tool        = nullptr;
  c_interface_entity_list*        entity_list        = nullptr;
  c_interface_model_info*         model_info         = nullptr;
  c_interface_cvar*               cvar               = nullptr;
  c_interface_material_system*    material_system    = nullptr;
  c_interface_engine_render_view* render_view        = nullptr;
  c_interface_engine_model*       engine_model       = nullptr;
  c_interface_trace*              trace              = nullptr;
  c_interface_panel*              panel              = nullptr;
  c_interface_game_event_manager* game_event_manager = nullptr;
  c_steamapicontext*              steamcontext       = nullptr;
  c_interface_physics_surface*    physics_surface    = nullptr;
  ILuaShared*                     lua_shared         = nullptr;

  // Hook original pointers
  void* d3d9_hook_trp                                                  = nullptr;
  void* create_move_hook_trp                                           = nullptr;
  void* override_view_hook_trp                                         = nullptr;
  void* engine_paint_hook_trp                                          = nullptr;
  void* cnetchan_sendnetmsg_hook_trp                                   = nullptr;
  void* cinput_get_user_cmd_hook_trp                                   = nullptr;
  void* run_command_hook_trp                                           = nullptr;
  void* post_think_hook_trp                                            = nullptr;
  void* base_animating_setupbones_hook_trp                             = nullptr;
  void* frame_stage_notify_hook_trp                                    = nullptr;
  void* draw_model_execute_hook_trp                                    = nullptr;
  void* set_dormant_hook_trp                                           = nullptr;
  void* paint_traverse_hook_trp                                        = nullptr;
  void* fire_event_hook_trp                                            = nullptr;
  void* cbaseclientstate_process_print_hook_trp                        = nullptr;
  void* cbaseclientstate_process_set_convar_hook_trp                   = nullptr;
  void* lua_gamemode_callwithargs_hook_trp                             = nullptr;
  void* lua_gamemode_callfinish_hook_trp                               = nullptr;
  void* lua_gamemode_call_hook_trp                                     = nullptr;
  void* lua_gamemode_call_str_hook_trp                                 = nullptr;
  void* lua_gamemode_callwithargs_str_hook_trp                         = nullptr;
  void* do_post_screen_space_effects_hook_trp                          = nullptr;
  void* crenderview_renderview_hook_trp                                = nullptr;
  void* render_capture_hook_trp                                        = nullptr;
  void* should_draw_localplayer_hook_trp                               = nullptr;
  void* fire_bullets_hook_trp                                          = nullptr;
  void* set_render_target_hook_trp                                     = nullptr;
  void* engine_client_cmd_hook_trp                                     = nullptr;
  void* client_entity_list_on_add_entity_hook_trp                      = nullptr;
  void* client_entity_list_on_remove_entity_hook_trp                   = nullptr;
  void* maintain_sequence_transition_hook_trp                          = nullptr;
  void* check_for_sequence_change_hook_trp                             = nullptr;
  void* copy_render_target_to_texture_ex_hook_trp                      = nullptr;
  void* copy_render_target_to_texture_hook_trp                         = nullptr;
  void* get_render_target_hook_trp                                     = nullptr;
  void* rtl_dispatch_exception_hook_trp                                = nullptr;
  void* push_render_target_and_viewport_p5_hook_trp                    = nullptr;
  void* push_render_target_and_viewport_p1_hook_trp                    = nullptr;
  void* get_local_view_angles_hook_trp                                 = nullptr;
  void* cmutliplayeranimstate_update_hook_trp                          = nullptr;
  void* cmutliplayeranimstate_restartgesture_hook_trp                  = nullptr;
  void* ivideowriter_addframe_hook_trp                                 = nullptr;
  void* record_game_hook_trp                                           = nullptr;
  void* shaderapi_read_pixels_hook_trp                                 = nullptr;
  
  // Pointers
  s_global_data*  global_data                  = nullptr;
  s_client_state* client_state                 = nullptr;
  s_input*        input                        = nullptr;

  void*           cl_runprediction             = nullptr;
  void*           keyvalues_init               = nullptr;
  void*           keyvalues_setname            = nullptr;
  void*           keyvalues_setint             = nullptr;
  void*           keyvalues_setfloat           = nullptr;
  void*           keyvalues_setstring          = nullptr;
  void*           vgui_drawhud_retaddr         = nullptr;
  void*           fire_bullet                  = nullptr;
  void*           movie_name_addr              = nullptr;
  c_material**    gmod_model_material_override = nullptr;

  void*           set_abs_origin_addr              = nullptr;
  void*           set_abs_angles_addr              = nullptr;
  void*           localization_addr                = nullptr;
  void*           base_address_start               = nullptr;
  void*           base_address_end                 = nullptr;
  i32*            prediction_random_seed           = nullptr;
  void*           md5_pseudorandom                 = nullptr;
  void*           engine_client_cmd_retaddr        = nullptr;
  void*           cviewrender_renderview_ret_addr1 = nullptr;
  void*           chlclient_createmove_push_rdi    = nullptr;
  u64             bsendpacket_rdi_rsp              = 0;

  // Cvars
  c_cvar*         cl_updaterate              = nullptr;
  c_cvar*         cl_interp                  = nullptr;
  c_cvar*         cl_interp_ratio            = nullptr;
  c_cvar*         sv_client_min_interp_ratio = nullptr;
  c_cvar*         sv_client_max_interp_ratio = nullptr;
  c_cvar*         sv_gravity                 = nullptr;
  c_cvar*         sv_airaccelerate           = nullptr;
  c_cvar*         sv_friction                = nullptr;
  c_cvar*         sv_stopspeed               = nullptr;
  c_cvar*         sv_maxvelocity             = nullptr;


  ALWAYSINLINE bool valid(){
    return view_setup != nullptr && engine != nullptr && client != nullptr;
  }
};

CLASS_EXTERN(c_global, global);