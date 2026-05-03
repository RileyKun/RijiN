#pragma once

#define GLOBAL_SETUP() if(global == nullptr) global = new c_global;

class gcs_menu;

class c_player_data{
public:
  u32   friends_id;
  bool  steam_friend;
  float next_steam_friend_check;
};

struct s_dab_spread_dump{
  float spread;
  float x;
  float y;
  bool dumped;
};

class c_game_specific_data{
public:
  c_player_data player_data[129];
  s_view_setup* view_setup = nullptr;
  matrix4x4     view_matrix;

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

  // prediction related
  bool            did_run_prediction      = false;
  bool            is_calling_run_command  = false;
  s_global_data   old_global_data;
  u32             old_prediction_seed;

  // player data verification
  bool verified_player_data               = false;

  // Aimbot target
  vec3 aimbot_target_aim_point;
  i32 aimbot_target_index         = -1;
  float aimbot_target_reset_time  = 0.f;

  bool simulate_bullet;
  s_dab_spread_dump spread_dump;

  ALWAYSINLINE void reset(){
    DBG("[!] game data reset\n");

    utils::reset_all_lag_comp_data();
    utils::reset_all_player_data();

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
  c_font_d3d9*  gui_font                = nullptr;
  c_font_d3d9*  gui_font_emoji          = nullptr;
  void*         target_window           = nullptr;
  gcs_menu*     menu                    = nullptr;
  bool          menu_open               = false;
  u32           focus_overlay_panel_id  = 0;

  // ESP Related fonts
  c_font_d3d9* esp_font_large = nullptr;
  c_font_d3d9* esp_font_small = nullptr;

  // Pointers
  void* clc_move_rtti         = nullptr;

  // settings
  c_settings* settings = nullptr;

  // Interfaces
  c_interface_client*             client          = nullptr;
  c_interface_engine*             engine          = nullptr;
  c_interface_prediction*         prediction      = nullptr;
  c_interface_engine_tool*        engine_tool     = nullptr;
  c_interface_entity_list*        entity_list     = nullptr;
  c_interface_model_info*         model_info      = nullptr;
  c_interface_cvar*               cvar            = nullptr;
  c_interface_material_system*    material_system = nullptr;
  c_interface_engine_render_view* render_view     = nullptr;
  c_interface_engine_model*       engine_model    = nullptr;
  c_interface_trace*              trace           = nullptr;
  c_interface_panel*              panel           = nullptr;
  c_steamapicontext*              steamcontext    = nullptr;

  // Hook original pointers
  void* d3d9_hook_trp                                 = nullptr;
  void* create_move_hook_trp                          = nullptr;
  void* override_view_hook_trp                        = nullptr;
  void* engine_paint_hook_trp                         = nullptr;
  void* cnetchan_sendnetmsg_hook_trp                  = nullptr;
  void* cinput_get_user_cmd_hook_trp                  = nullptr;
  void* run_command_hook_trp                          = nullptr;
  void* post_think_hook_trp                           = nullptr;
  void* base_animating_setupbones_hook_trp            = nullptr;
  void* frame_stage_notify_hook_trp                   = nullptr;
  void* draw_model_execute_hook_trp                   = nullptr;
  void* set_dormant_hook_trp                          = nullptr;
  void* paint_traverse_hook_trp                       = nullptr;
  void* util_impacttrace_trp                          = nullptr;
  void* fire_event_hook_trp                           = nullptr;
  void* cbaseclientstate_process_print_hook_trp       = nullptr;
  void* csdkplayer_firebullet_hook_trp                = nullptr;
  void* csdkplayershared_setrecoil_hook_trp           = nullptr;

  // Pointers
  s_global_data*  global_data                  = nullptr;
  s_client_state* client_state                 = nullptr;
  s_input*        input                        = nullptr;
  void*           get_sequence_name            = nullptr;
  void*           cl_runprediction             = nullptr;
  void*           keyvalues_init               = nullptr;
  void*           keyvalues_setname            = nullptr;
  void*           keyvalues_setint             = nullptr;
  void*           keyvalues_setfloat           = nullptr;
  void*           keyvalues_setstring          = nullptr;

  void*           look_weapon_info_slot                    = nullptr;
  void*           file_weapon_info_from_handle             = nullptr;
  void*           get_class_name_addr                      = nullptr;
  void*           set_abs_origin_addr                      = nullptr;
  void*           set_abs_angles_addr                      = nullptr;
  i32*            prediction_random_seed                   = nullptr;
  void*           vgui_drawhud_retaddr                     = nullptr;
  void*           cweaponpistol_updatepenaltytime_addr     = nullptr;

  // Cvars
  c_cvar*         cl_updaterate              = nullptr;
  c_cvar*         cl_interp                  = nullptr;
  c_cvar*         cl_interp_ratio            = nullptr;
  c_cvar*         sv_client_min_interp_ratio = nullptr;
  c_cvar*         sv_client_max_interp_ratio = nullptr;

  ALWAYSINLINE bool valid(){
    return view_setup != nullptr && engine != nullptr && client != nullptr;
  }
};

CLASS_EXTERN(c_global, global);