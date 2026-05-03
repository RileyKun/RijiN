#pragma once

#define GLOBAL_SETUP() if(global == nullptr) global = new c_global; \
global_internal = (c_base_global*)global; \

class gcs_menu;
class gcs_info_panel;


class c_player_list_data : public c_base_player_list_data{
public:
  void init() override{
    c_base_player_list_data::init();
  }

  u32 get_size() override{
    return sizeof(c_player_list_data);
  }
};

class c_player_data : public c_base_player_data, public c_base_player_dormant_data{
public:
  float      next_shot_time;

  vec3       obb_min;
  vec3       obb_max;

  float      last_pred_change;
  float      pred_change;
  float      pred_change_delta;
  bool       pred_mod;
  float      pred_real_time;
  bool       bypass_hit_chance;
  float      pred_last_yaw_delta;

  bool       seen_localplayer;

  c_player_list_data playerlist;
  u32                playerlist_hash;
  u32                playerlist_cache_hash;
  bool               playerlist_notify;
};

class c_game_specific_data{
public:
  c_player_data player_data[129];

  c_aimbot_settings*     aimbot_settings     = nullptr;
  c_triggerbot_settings* triggerbot_settings = nullptr;

  bool firing_this_tick = false;

  c_player_list_data  playerlist_data;

  ALWAYSINLINE void reset(){
    DBG("[!] game data reset\n");

    utils::reset_all_lag_comp_data();
    utils::reset_all_player_data();

    memset(this, 0, XOR32(sizeof(c_game_specific_data)));
    input_system->reset();
  }
};

#define globaldata global->global_data
#define config global->settings
class c_global : public c_base_global,  public c_game_specific_data{
public:
  // GUI Related
  gcs_info_panel*  info_panel              = nullptr;
  gcs_menu*        menu                    = nullptr;
  s_info           info[128];

  void*            bsendpacket_rdi_rsp           = nullptr;
  void*            chlclient_createmove_push_rdi = nullptr;


  // Pointers
  void*            clc_move_rtti                      = nullptr;

  void*            util_cliptrace_to_players          = nullptr;
  void*            get_hud_element_addr               = nullptr;
  void*            hud_element_addr                   = nullptr;
  c_base_hud_chat* hud_chat                           = nullptr;
  void*            chatprintf_retaddr1                = nullptr;
  void*            ctrace_filter_simple               = nullptr;
  void*            intersect_inf_ray_with_sphere_addr = nullptr;
  void*            baseentity_calculate_abs_velocity_addr = nullptr;

  // settings
  c_settings*                     settings = nullptr;

  // Interfaces
  c_interface_trace*              trace                = nullptr;

  // Hook original pointers
  void* d3d9_hook_trp                                  = nullptr;
  void* create_move_hook_trp                           = nullptr;
  void* override_view_hook_trp                         = nullptr;
  void* engine_paint_hook_trp                          = nullptr;
  void* cnetchan_sendnetmsg_hook_trp                   = nullptr;
  void* cinput_get_user_cmd_hook_trp                   = nullptr;

  void* base_animating_setupbones_hook_trp             = nullptr;
  void* frame_stage_notify_hook_trp                    = nullptr;
  void* cdodplayer_clientthink_hook_trp                = nullptr;
  void* draw_model_execute_hook_trp                    = nullptr;
  void* set_dormant_hook_trp                           = nullptr;
  void* paint_traverse_hook_trp                        = nullptr;
  void* util_impacttrace_trp                           = nullptr;
  void* fire_event_hook_trp                            = nullptr;
  void* get_exposure_range_hook_trp                    = nullptr;
  void* cbaseclientstate_process_set_convar_hook_trp   = nullptr;
  void* cbaseviewmodel_calcviewmodelview_hook_trp      = nullptr;
  void* check_for_pure_server_whitelist_hook_trp       = nullptr;
  void* hud_chatline_insert_and_colorize_text_hook_trp = nullptr;
  void* engine_get_player_info_hook_trp                = nullptr;
  void* s_startsound_hook_trp                          = nullptr;
  void* ccsviewrender_perform_flashbangeffect_hook_trp = nullptr;
  void* clientmodecsnormal_canrecorddemo_hook_trp      = nullptr;
  void* is_blacklisted_hook_trp                        = nullptr;
  void* rtl_dispatch_exception_hook_trp                = nullptr;
  void* cbaseentity_baseinterpolatepart1_hook_trp      = nullptr;
  void* cbaseentity_estimateabsvelocity_hook_trp       = nullptr;
  void* crenderview_renderview_hook_trp                = nullptr;
  void* input_cam_is_thirdperson_hook_trp              = nullptr;

  // Pointers
  s_input*        input                        = nullptr;

  void*           vgui_drawhud_retaddr         = nullptr;
  void*           fire_bullet                  = nullptr;

  void*           look_weapon_info_slot        = nullptr;
  void*           file_weapon_info_from_handle = nullptr;
  void*           get_class_name_addr          = nullptr;
  void*           set_abs_origin_addr          = nullptr;
  void*           set_abs_angles_addr          = nullptr;


  ALWAYSINLINE inline bool valid(){
    return view_setup != nullptr && engine != nullptr && client != nullptr;
  }

  ALWAYSINLINE inline void on_aimbot_target(i32 index, vec3 aim_point, c_base_weapon* weapon){
    if(weapon == nullptr)
      return;

    css_weapon_file_info* wep_info = weapon->get_weapon_info();
    if(wep_info == nullptr)
      return;

    aimbot_target_aim_point  = aim_point;
    aimbot_target_index      = index;
    aimbot_target_reset_time = math::time() + 0.5f;
  }
};

CLASS_EXTERN(c_global, global);