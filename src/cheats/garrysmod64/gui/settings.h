#pragma once

#define SETTINGS_DATA_PAD 256
#define SETTINGS_VERSION 6

#define CONFIG_RGBA(name, r, g, b, a) float name[4] = {r,g,b,a};
#define CONFIG_RGB(name, r, g, b) CONFIG_RGBA(name, r, g, b, 255);

enum player_esp_settings_type{
  PLAYER_ESP_SETTING_TYPE_NORMAL,
  PLAYER_ESP_SETTING_TYPE_FRIEND,
};

enum world_esp_settings_type{
  WORLD_ESP_SETTING_TYPE_ALLIED_TEAM = 0,
  WORLD_ESP_SETTING_TYPE_AXIS_TEAM = 1,
  WORLD_ESP_SETTING_TYPE_UNASSIGNED_TEAM = 2,
};

class c_aimbot_settings{
public:
  union{
    struct{
      // aimbot
      bool  enabled                         = false;
      u8    aim_hotkey                      = VK_XBUTTON2;
      bool  aim_mode_automatic              = false;
      bool  aim_mode_on_attack              = false;
      bool  aim_mode_on_key                 = true;
      i32   max_fov                         = 25;
      float smoothing                       = 0;
      bool  position_adjust_history         = true;
      bool  position_adjust_accurate_unlag  = true;

      // target
      bool  target_method_closest_to_fov    = true;
      bool  target_method_distance          = false;
      bool  priority_hitbox[19];
      bool  hitbox[19];
      bool  no_spread                       = true;

      bool silent_aim_none        = false;
      bool silent_aim_clientside  = true;
      bool silent_aim_serverside  = false;
      bool auto_shoot             = true;

      bool  autowall_enabled      = false;
      float autowall_min_dmg      = 50.f;
      bool  no_recoil             = false;

      bool ignore_team_mates      = false;
      bool ignore_friends         = true;
      bool ignore_demi_god        = true;
      bool ignore_noclipping      = true;
      bool ignore_god_mode        = true;
      bool ignore_staff           = true;
      bool ignore_vehicle         = true;

      bool trace_filter_skip_vehicle = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_player_esp_settings{
public:
  union{
    struct{
      bool enabled                   = true;

      bool no_box                    = false;
      bool box                       = false;
      bool box_outlines              = false;
      bool name                      = true;
      bool distance                  = true;
      bool weapon_name               = true;
      bool health_bar                = true;
      i32  health_bar_fractions      = 0;


      bool friend_tag                = true;
      bool show_localplayer          = true;
      bool flags                     = true;
      bool flag_god_mode             = true;
      bool flag_noclipped            = true;
      bool flag_in_vehicle           = true;
      bool flag_on_fire              = true;
      bool flag_frozen               = true;
      bool flag_grabbed              = true;
      bool flag_demi_god             = true;
      bool flag_staff                = true;
      bool flag_hitboxes             = true;
      i32  max_distance              = 128;
      
      bool flag_armor                = true;
      bool armor_bar                 = false;
      i32  armor_bar_fractions       = 0;
      bool cheater_alert_tag         = true;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_world_esp_settings{
public:
  union{
    struct{
      bool object_enabled   = true;
      i32  max_distance     = 32;
      bool box              = true;
      bool box_outlines     = true;
      bool name             = true;
      bool distance         = true;

      CONFIG_RGB(esp_colour, 255, 255, 255);
      bool aimbot_target = false;
      CONFIG_RGB(esp_aimbot_colour, 0, 255, 0);
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_player_chams_settings{
public:
  union{
    struct{
      bool enabled                  = true;
      bool ignore_z                 = true;

      bool base_material_none       = false;
      bool base_material_shaded     = false;
      bool base_material_flat       = true;

      bool overlay_material_none    = false;
      bool overlay_material_fresnel = false;
      bool overlay_material_flat    = false;

      bool show_localplayer         = true;
      i32  max_distance             = 128;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_world_chams_settings{
public:
  union{
    struct{
      bool object_enabled           = true;
      i32  max_distance             = 32;
      bool ignore_z                 = true;
      bool base_material_none       = false;
      bool base_material_flat       = true;
      bool base_material_shaded     = false;
     
      bool overlay_material_none    = false;
      bool overlay_material_fresnel = false;
      bool overlay_material_flat    = false;
      
      CONFIG_RGBA(chams_colour_base, 255, 255, 255, 38);
      CONFIG_RGBA(chams_colour_overlay, 255, 255, 255, 38);

      bool aimbot_target         = false;
      bool aimbot_target_overlay = false;
      CONFIG_RGBA(chams_aimbot_base_colour, 0, 255, 0, 64);
      CONFIG_RGBA(chams_aimbot_overlay_colour, 0, 255, 0, 64);
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

// Just alot of stuff related with non players and glow hence why it has its own class.
class c_world_glow_settings{
public:
  union{
    struct{
      bool object_enabled    = true;
      i32 max_distance       = 32;

      CONFIG_RGB(glow_colour, 255, 255, 220);
      bool aimbot_target = false;
      CONFIG_RGB(glow_aimbot_colour, 0, 255, 0);
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_visual_settings{
public:
  union{
    struct{
      CONFIG_RGB(menu_foreground_colour, 25, 118, 210);
      CONFIG_RGB(menu_background_colour, 41, 44, 51);

      bool no_visual_recoil                     = true;

      bool player_esp_use_team_colours          = true;
      bool player_esp_use_enemyteam_colours     = false;
      bool player_esp_friends_use_single_colour = true;
      bool player_esp_friends_use_mult_colour   = false;

      bool player_chams_use_team_colours          = true;
      bool player_chams_use_enemyteam_colours     = false;
      bool player_chams_friends_use_single_colour = true;
      bool player_chams_friends_use_mult_colour   = false;
      bool world_chams_use_team_colours           = true;
      bool world_chams_use_enemyteam_colours      = false;


      bool player_glow_use_team_colours           = true;
      bool player_glow_use_enemyteam_colours      = false;

      bool player_glow_friends_use_single_colour  = true;
      bool player_glow_friends_use_mult_colour    = false;

      bool world_glow_use_team_colours            = true;
      bool world_glow_use_enemyteam_colours       = false;

      // Glow main toggle
      bool glow_enabled                       = true;

      // Glow player settings
      bool player_glow_enabled                = true;
      bool player_glow_render_friendlies      = true;
      bool player_glow_render_enemies         = true;
      bool player_glow_render_cloaked_spies   = true;
      bool player_glow_render_steamfriends    = true;
      bool player_glow_render_localplayer     = true;
      i32  player_glow_max_distance           = 128;

       // Glow settings
      bool glow_stencil                       = true;
      bool glow_blur                          = false;
      i32  glow_thickness                     = 1;


      CONFIG_RGB(player_esp_colour, 255, 108, 32);
      CONFIG_RGB(player_esp_friend_colour, 96, 255, 164);

      // Player chams
      CONFIG_RGBA(player_chams_colour, 255, 108, 32, 64);
      CONFIG_RGBA(player_chams_friend_colour, 96, 255, 164, 64);

      CONFIG_RGBA(player_chams_overlay_colour, 255, 168, 32, 128);
      CONFIG_RGBA(player_chams_overlay_friend_colour, 96, 255, 164, 128);
     
      // Player glow
      CONFIG_RGB(player_glow_colour, 255, 168, 32);
      CONFIG_RGB(player_glow_friend_colour, 96, 255, 164);

      bool player_esp_aimbot_target           = false;
      bool player_chams_aimbot_target         = false;
      bool player_chams_overlay_aimbot_target = false;
      bool player_glow_aimbot_target          = true;

      CONFIG_RGB(player_esp_aimbot_target_colour, 10, 255, 10);
      CONFIG_RGBA(player_chams_aimbot_target_colour, 32, 255, 32, 64);
      CONFIG_RGBA(player_chams_overlay_aimbot_target_colour, 32, 255, 32, 64);
      CONFIG_RGB(player_glow_aimbot_target_colour, 64, 255, 128);

      bool thirdperson                                  = false;
      i32  thirdperson_distance                         = 150;  
      u8   thirdperson_key                              = VK_F5;
      bool thirdperson_usekey                           = false;
      bool thirdperson_key_held                         = true;
      bool thirdperson_key_toggle                       = false;
      bool thirdperson_key_double_click                 = false;
      bool thirdperson_right_offset_enabled             = false;
      bool thirdperson_right_offset_use_key             = false;
      i32  thirdperson_right_offset                     = 0;
      u8   thirdperson_right_offset_key                 = VK_XBUTTON2;
      u8   thirdperson_right_offset_invert_key          = 0x56;
      bool thirdperson_right_offset_invert_held         = false;
      bool thirdperson_right_offset_invert_toggle       = false;
      bool thirdperson_right_offset_invert_double_click = true;
      bool thirdperson_right_offset_held                = true;
      bool thirdperson_right_offset_toggle              = false;
      bool thirdperson_right_offset_double_click        = false;
      i32  thirdperson_up_offset                        = 0;

      bool  cheap_text_rendering                        = false;
      bool  show_information                            = true;

      bool  aimbot_fov_circle_enabled                   = true;
      bool  aimbot_fov_circle_outlines                  = true;
      CONFIG_RGB(aimbot_fov_circle_colour, 255, 255, 255);

      bool aimbot_target_visual_pred                    = false;
      bool aimbot_target_visual_pred_lines              = false;

    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_hvh_settings{
public:
  union{
    struct{
      bool break_animations_disabled = true;
      bool break_animations_constant = false;
      bool break_animations_on_jump = false;

      bool fakelag                 = false;
      i32  fakelag_maxchoke        = 12;
      bool fakelag_method_interval = false;
      bool fakelag_method_movement = true;
      u8   fakelag_key             = VK_MENU;
      bool fakelag_usekey          = false;
      bool fakelag_key_held        = true;
      bool fakelag_key_toggle      = false;
      bool fakelag_anti_backtrack  = false;

      bool antiaim                              = false;
      bool antiaim_pitch                        = false;
      bool antiaim_yaw                          = true;

      bool pitch_fake_none                      = true;
      bool pitch_fake_up                        = false;
      bool pitch_fake_down                      = false;

      bool pitch_real_up                        = true;
      bool pitch_real_zero                      = false;
      bool pitch_real_down                      = false;

      bool yaw_method_real_rotate               = true;
      bool yaw_method_real_spin                 = false;
      bool yaw_method_real_rotate_dynamic       = false;

      bool yaw_method_fake_rotate               = true;
      bool yaw_method_fake_spin                 = false;
      bool yaw_method_fake_rotate_dynamic       = false;

      float rotate_fake_ang                      = 90.f;
      float rotate_fake_ang2                     = -90.f;
      float rotate_real_ang                      = -90.f;
      float rotate_real_ang2                     = 90.f;

      i32  spin_fake_speed                      = 12;
      i32  spin_real_speed                      = 12;
      bool spin_fake_invert                     = false;
      bool spin_real_invert                     = false;

      float  yaw_rotate_dynamic_update_rate = 750.f;

      bool resolver                             = true;
      bool resolver_esp_status                  = true;
      bool resolver_cheatersonly                = true;

      bool yaw_rotate_real_target_method_view         = false;
      bool yaw_rotate_real_target_method_closetargets = true;

      bool yaw_rotate_fake_target_method_view         = false;
      bool yaw_rotate_fake_target_method_closetargets = true;


      u8   antiaim_key        = 0x54;
      bool antiaim_usekey     = false;
      bool antiaim_key_held   = true;
      bool antiaim_key_toggle = false;

      bool antiaim_cycle_enabled         = false;
      u8   antiaim_cycle_key             = 0x6;
      bool antiaim_cycle_swap_settings   = true;

      bool antiaim_cycle_invert_fake_angle  = false;
      bool antiaim_cycle_invert_real_angle  = false;
      bool antiaim_cycle_invert_both_angles = false;
      bool antiaim_pitch_disabled           = false;
      bool antiaim_pitch_auto               = true;

      bool peek_assist                      = false;
      u8   peek_assist_hotkey               = VK_MENU;

      bool antiaim_legit                    = false;
      bool antiaim_rage                     = true;

      bool yaw_auto_edge_enabled            = false;
      bool yaw_auto_edge_fake               = false;
      bool yaw_auto_edge_real               = true;

      bool yaw_auto_edge_target_method_view         = true;
      bool yaw_auto_edge_target_method_closetargets = false;

      bool fakelag_peek                        = false;

      bool fakeduck_disabled                   = true;
      bool fakeduck_onduck                     = false;
      bool fakeduck_always                     = false;

      bool antiaim_pitch_static                = true;
      bool antiaim_pitch_dynamic               = false;

      bool fakelag_adaptive                    = false;
      bool fakeduck_on_key                     = false;
      u8   fakeduck_key                        = 0x11;
      bool auto_stop                           = false;

      bool antiaim_key_double_click             = false;
      bool fakelag_key_double_click             = false;

      bool resolver_allow_manual_cycle         = false;
      u8   resolver_mc_sp_key                  = VK_SHIFT;
      u8   resolver_mc_up_key                  = VK_TAB;

      bool resolver_mc_sp_single_click         = false;
      bool resolver_mc_sp_double_click         = true;

      bool resolver_mc_up_single_click         = true;
      bool resolver_mc_up_double_click         = false;

    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_misc_settings{
public:
  union{
    struct{
      bool bhop        = false;
      bool auto_pistol = true;

      bool freecam                            = false;
      i32  freecam_speed                      = 5;
      u8   freecam_key                        = VK_XBUTTON2;
      bool freecam_key_held                   = true;
      bool freecam_key_toggle                 = false;
      bool freecam_key_double_click           = false;
      
      bool auto_strafe                        = false;
      bool auto_strafe_type_normal            = false;
      bool auto_strafe_type_directional       = true;
      float auto_strafe_retrack               = 75.f;
      bool auto_strafe_avoid_walls_disabled   = true;
      bool auto_strafe_avoid_walls_normal     = false;
      bool auto_strafe_avoid_walls_adv        = false;
      bool auto_strafe_prespeed               = true;

      bool anti_screen_grab_always             = true;
      bool anti_screen_grab_dont_capture_2dcam = false;
      bool anti_screen_grab_disabled           = false;

      bool no_silent_aim                       = false;
      bool no_movement_fix                     = false;

      bool voicerecord_protect                 = true;
      bool demo_protect                        = true;

      bool custom_server_integrations          = false;
      bool setupbones_optimisations            = false;
      bool auto_disable_hvh_on_leave           = true;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_settings{
public:
  u32 version = SETTINGS_VERSION;

  c_aimbot_settings       aimbot;
  c_player_esp_settings   player_esp[3];
  c_world_esp_settings    world_esp;
  c_player_chams_settings player_chams[3];
  c_world_chams_settings  world_chams;
  c_world_glow_settings   world_glow;
  c_visual_settings       visual;
  c_misc_settings         misc;
  c_hvh_settings          hvh;

  vec3i information_panel_pos;
};