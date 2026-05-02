#pragma once

#define GLOBAL_SETUP() if(global == nullptr) global = new c_global; \
global_internal = (c_base_global*)global; \

class gcs_menu;

class c_player_list_data{
public:
  void init(){
    mark_as_friend        = false;
    mark_as_legit         = false;
    mark_as_cheater       = false;
    mark_as_suspicious    = false;
    mark_as_auto          = true;


    ignore_player         = false;

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

  union {
    struct {
      bool mark_as_friend;

      bool mark_as_auto;
      bool mark_as_legit;
      bool mark_as_cheater;
      bool mark_as_suspicious;


      bool ignore_player;
      bool custom_esp_colour;
      bool custom_chams_base_colour;
      bool custom_chams_overlay_colour;
      bool custom_glow_colour;

      float esp_colour[4];
      float chams_colour[4];
      float chams_overlay_colour[4];
      float glow_colour[4];

      //CONFIG_RGB(esp_colour, 96, 255, 164);
      //CONFIG_RGBA(chams_colour, 96, 255, 164, 32);
      //CONFIG_RGBA(chams_overlay_colour, 96, 255, 164, 32);
      //CONFIG_RGB(glow_colour, 96, 255, 164);

      bool priority_mode;
      i32  priority;

      bool valid = false;
    };

    PAD(512); // <-- 128 (4 bytes of whatever)
  };
};

class c_entity_data{
public:
  // transform related
  bool      has_transform = false;
  matrix3x4 transform;

  matrix3x4 bone_matrix[128];
  u32       bone_count = 0;
  float     bone_last_simulation_time;

  i8        sequence_name[1024];

  float     last_simulation_time;
  float     last_update_time;
  bool      idle;
  bool      model_drawn;

  bool  is_objective;
  float is_currently_objective_timer;

  float blacklist_time;

  float     next_steam_friend_check;
  bool      steam_friend;
  u32       friends_id;
  bool      is_rijin_user;

  vec3      last_origin;
  vec3      vel;

  c_player_list_data playerlist;
  u32                playerlist_hash;
  u32                playerlist_cache_hash;
  bool               playerlist_notify;
};

class c_game_specific_data{
public:
  c_entity_data entity_data[2048];

  c_player_list_data  playerlist_data;

  ALWAYSINLINE void reset(){
    DBG("[!] game data reset\n");

    utils::reset_all_lag_comp_data();
    utils::reset_all_entity_data();

    memset(this, 0, XOR32(sizeof(c_game_specific_data)));
    input_system->reset();
  }
};

#define globaldata global->global_data
#define config global->settings
class c_global : public c_base_global, public c_game_specific_data{
public:
  // GUI Related
  gcs_menu*     menu          = nullptr;


  // Pointers
  void* clc_move_rtti         = nullptr;

  // settings
  c_settings* settings = nullptr;


  // Hook original pointers
  void* d3d9_hook_trp                                   = nullptr;
  void* d3d9_reset_trp                                  = nullptr;
  void* create_move_hook_trp                            = nullptr;
  void* override_view_hook_trp                          = nullptr;
  void* engine_paint_hook_trp                           = nullptr;
  void* cnetchan_sendnetmsg_hook_trp                    = nullptr;
  void* cinput_get_user_cmd_hook_trp                    = nullptr;
  void* post_think_hook_trp                             = nullptr;
  void* base_animating_setupbones_hook_trp              = nullptr;
  void* frame_stage_notify_hook_trp                     = nullptr;
  void* server_adjust_player_time_base_hook_trp         = nullptr;
  void* run_simulation_hook_trp                         = nullptr;
  void* clock_drift_manager_set_server_tick_hook_trp    = nullptr;
  void* draw_model_execute_hook_trp                     = nullptr;
  void* studio_render_forced_material_override_hook_trp = nullptr;
  void* render_context_set_stencil_state_hook_trp       = nullptr;
  void* set_dormant_hook_trp                            = nullptr;
  void* fire_event_hook_trp                             = nullptr;
  void* paint_traverse_hook_trp                         = nullptr;
  void* do_post_screen_space_effects_hook_trp           = nullptr;
  void* int_decode_hook_trp                             = nullptr;
  void* apply_entity_glow_effects_hook_trp              = nullptr;
  void* trace_ray_hook_trp                              = nullptr;
  void* cmd_executecommand_hook_trp                     = nullptr;
  void* crenderview_renderview_hook_trp                 = nullptr;
  void* should_draw_localplayer_hook_trp                = nullptr;
  void* cam_isthirdperson_hook_trp                      = nullptr;
  void* cbaseviewmodel_calcviewmodelview_hook_trp       = nullptr;
  void* cbaseentity_baseinterpolatepart1_hook_trp       = nullptr;

  // Pointers
  s_global_data*  server_global_data                      = nullptr;
  s_input*        input                                   = nullptr;

  c_interface_trace* trace                                = nullptr;
  void*           get_sequence_name                       = nullptr;

  void*           shared_random_float_addr                = nullptr;

  void*           test_swing_collision                    = nullptr;
  void*           cam_isthirdperson_retaddr_override_view = nullptr;
  void*           baseentity_calculate_abs_velocity_addr  = nullptr;

  ALWAYSINLINE bool valid(){
    return view_setup != nullptr && engine != nullptr && client != nullptr;
  }

  NEVERINLINE void on_aimbot_target(i32 index, vec3 aim_point, c_base_weapon* weapon){
    if(weapon == nullptr)
      return;

    aimbot_target_aim_point  = aim_point;
    aimbot_target_index      = index;
    aimbot_target_reset_time = math::time() + 0.5f;
  }
};

CLASS_EXTERN(c_global, global);