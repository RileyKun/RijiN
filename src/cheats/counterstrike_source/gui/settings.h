#pragma once

#define SETTINGS_DATA_PAD 0x1000
#define SETTINGS_VERSION 2

#define CONFIG_RGBA(name, r, g, b, a) float name[4] = {r,g,b,a};
#define CONFIG_RGB(name, r, g, b) CONFIG_RGBA(name, r, g, b, 255);

enum player_esp_settings_type{
  PLAYER_ESP_SETTING_TYPE_ALLIED_TEAM = 0,
  PLAYER_ESP_SETTING_TYPE_AXIS_TEAM = 1,
  PLAYER_ESP_SETTING_TYPE_FRIEND = 2,
};

enum world_esp_settings_type{
  WORLD_ESP_SETTING_TYPE_ALLIED_TEAM = 0,
  WORLD_ESP_SETTING_TYPE_AXIS_TEAM = 1,
  WORLD_ESP_SETTING_TYPE_UNASSIGNED_TEAM = 2,
};

enum aimbot_settings_type{
  AIMBOT_SETTING_RIFLE = 0,
  AIMBOT_SETTING_SNIPER = 1,
  AIMBOT_SETTING_SMG = 2,
  AIMBOT_SETTING_PISTOL = 3,
  AIMBOT_SETTING_SHOTGUN = 4,
  AIMBOT_SETTING_MELEE = 5,
  AIMBOT_SETTING_COUNT,
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
      i32   max_fov                         = 180;
      float smoothing                       = 0;
      bool  position_adjust_history         = true;
      bool  position_adjust_accurate_unlag  = true;

      // target
      bool  target_friendlies               = false;
      bool  target_enemies                  = true;
      bool  target_friends                  = false;
      bool  target_method_closest_to_fov    = true;
      bool  target_method_distance          = false;
      bool  priority_hitbox[19];
      bool  hitbox[19];
      bool  no_spread                       = true;

      bool  silent_aim_none        = false;
      bool  silent_aim_clientside  = true;
      bool  silent_aim_serverside  = false;
      bool  auto_shoot             = true;

      bool  autowall_enabled      = false;
      float autowall_min_dmg      = 50.f;
      bool  no_recoil             = true;
      bool  next_shot             = false;
      bool ignore_spawn_protected = false;

      bool  no_recoil_rage               = true;
      bool  no_recoil_legit              = false;

      bool  no_recoil_legit_pitch_enabled = true;
      float no_recoil_legit_pitch         = 100.f;
      bool  no_recoil_legit_yaw_enabled   = true;
      float no_recoil_legit_yaw           = 100.f;
      bool  no_recoil_visual_inherit      = false;

      bool  no_spread_legit             = false;
      bool  no_spread_rage              = true;
      bool  closest_hitbox_to_crosshair = false;
      bool  ignore_players_through_smoke = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_triggerbot_settings{
public:
  union{
    struct{
      bool  enabled               = false;
      u8    trigger_hotkey        = VK_XBUTTON2;
      bool  trigger_mode_on_key   = true;

      bool target_friendlies      = false;
      bool target_enemies         = true;
      bool target_friends         = true;

      bool target_head            = false;
      bool target_body            = false;
      bool target_head_and_body   = false;
      bool target_everything      = true;

      bool nospread_use_aimbot_settings = false;
      bool nospread_off                 = false;
      bool nospread_legit_mode          = true;
      bool nospread_rage_mode           = false;

      bool spread_mode_legit            = true;
      bool spread_mode_rage             = false;

      float trigger_fire_delay    = 0.f;
      float trigger_fire_for_time = 0.f;
      bool  ignore_players_through_smoke = false;
      bool  ignore_spawn_protected       = false;
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
      bool box_outlines              = true;
      bool name                      = true;
      bool distance                  = true;
      bool weapon_name               = true;
      bool health_bar                = true;
      i32  health_bar_fractions      = 0;

      bool friend_tag                = true;
      bool spawn_protected_tag       = true;

      bool dormant_enabled           = true;
      bool dormant_render_text       = false;

      bool flags                           = true;
      bool flag_has_defuser               = false;
      bool flag_is_defusing               = true;
      bool flag_has_night_vision          = false;
      bool flag_is_using_night_vision     = true;
      bool flag_has_helmet                = true;

      bool armor_bar = true;
      i32  armor_bar_fractions = 0;

    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_world_esp_settings{
public:
  union{
    struct{
      bool enabled          = true;
      bool render_enemies   = true;
      bool render_friendies = true;

      bool object_enabled[4] = {true, true, true, true};
      bool box[4]            = {true, true, true, true};
      bool box_outlines[4]   = {true, true, true, true};
      bool name[4]           = {true, true, true, true};
      bool distance[4]       = {true, true, true, true};
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
      float base_alpha              = 85.f;

      bool overlay_material_none    = false;
      bool overlay_material_fresnel = false;
      bool overlay_material_flat    = true;
      float overlay_alpha           = 10.f;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_world_chams_settings{
public:
  union{
    struct{
      bool enabled                  = true;
      bool render_enemies           = true;
      bool render_friendies         = true;
      bool object_enabled[4]        = {true, true, true, true};

      bool ignore_z                 = true;
      bool base_material_none       = true;
      bool base_material_flat       = false;
      bool base_material_shaded     = false;
      float base_alpha              = 50.f;

      bool overlay_material_none    = false;
      bool overlay_material_fresnel = true;
      bool overlay_material_flat    = false;
      float overlay_alpha           = 50.f;

    };
    PAD(SETTINGS_DATA_PAD);
  };
};

// Just alot of stuff related with non players and glow hence why it has its own class.
class c_world_glow_settings{
public:
  union{
    struct{
      bool enabled           = true;
      bool render_enemies    = true;
      bool render_friendlies  = true;
      bool object_enabled[4] = {true, true, true, true};

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

       // Glow settings
      bool glow_stencil                       = true;
      bool glow_blur                          = false;
      i32  glow_thickness                     = 1;

      CONFIG_RGB(player_esp_allied_team_colour, 255, 94, 94);
      CONFIG_RGB(player_esp_axis_team_colour, 166, 237, 253);
      CONFIG_RGB(player_esp_enemy_colour, 255, 64, 0);
      CONFIG_RGB(player_esp_team_colour, 64, 255, 0);

      CONFIG_RGB(player_esp_friend_colour, 96, 255, 164);

      CONFIG_RGB(player_esp_friend_allied_team_colour, 255, 64, 0);
      CONFIG_RGB(player_esp_friend_axis_team_colour, 0, 192, 255);

      CONFIG_RGB(player_esp_friend_enemy_colour, 255, 128, 0);
      CONFIG_RGB(player_esp_friend_team_colour, 128, 255, 0);

      bool world_esp_use_team_colours          = true;
      bool world_esp_use_enemyteam_colours     = false;

      CONFIG_RGB(world_esp_allied_team_colour, 255, 94, 94);
      CONFIG_RGB(world_esp_axis_team_colour, 166, 237, 253);
      CONFIG_RGB(world_esp_unassigned_colour, 255, 255, 255);

      CONFIG_RGB(world_esp_enemy_colour, 255, 64, 0);
      CONFIG_RGB(world_esp_team_colour, 64, 255, 0);


      // Player chams
      CONFIG_RGB(player_chams_allied_team_colour, 255, 61, 61);
      CONFIG_RGB(player_chams_axis_team_colour, 126, 208, 216);

      CONFIG_RGB(player_chams_friend_colour, 96, 255, 164);
      CONFIG_RGB(player_chams_friend_allied_team_colour, 200, 32, 0);
      CONFIG_RGB(player_chams_friend_axis_team_colour, 0, 96, 200);
      CONFIG_RGB(player_chams_friend_enemy_colour, 255, 128, 0);
      CONFIG_RGB(player_chams_friend_team_colour, 128, 255, 0);
      CONFIG_RGB(player_chams_enemy_colour, 255, 64, 0);
      CONFIG_RGB(player_chams_team_colour, 64, 255, 0);

      CONFIG_RGB(player_chams_overlay_allied_team_colour, 185, 185, 185);
      CONFIG_RGB(player_chams_overlay_axis_team_colour, 255, 255, 255);
      CONFIG_RGB(player_chams_overlay_enemy_colour, 255, 160, 60);
      CONFIG_RGB(player_chams_overlay_team_colour, 20, 255, 90);

      CONFIG_RGB(player_chams_overlay_friend_colour, 96, 255, 164);
      CONFIG_RGB(player_chams_overlay_friend_enemy_colour, 255, 150, 0);
      CONFIG_RGB(player_chams_overlay_friend_team_colour, 0, 255, 125);

      CONFIG_RGB(player_chams_overlay_friend_allied_team_colour, 200, 32, 32);
      CONFIG_RGB(player_chams_overlay_friend_axis_team_colour, 0, 64, 96);


      // Player glow
      CONFIG_RGB(player_glow_allied_team_colour, 200, 118, 118);
      CONFIG_RGB(player_glow_axis_team_colour, 145, 180, 202);
      CONFIG_RGB(player_glow_friend_colour, 96, 255, 164);

      CONFIG_RGB(player_glow_friend_allied_team_colour, 255, 64, 0);
      CONFIG_RGB(player_glow_friend_axis_team_colour,  0, 192, 255);
      CONFIG_RGB(player_glow_friend_enemy_colour, 255, 160, 60);
      CONFIG_RGB(player_glow_friend_team_colour, 20, 255, 90);

      CONFIG_RGB(player_glow_enemy_colour, 255, 128, 0);
      CONFIG_RGB(player_glow_team_colour, 128, 255, 0);

      CONFIG_RGB(world_chams_allied_team_colour, 255, 61, 61);
      CONFIG_RGB(world_chams_axis_team_colour, 126, 208, 216);
      CONFIG_RGB(world_chams_unassigned_colour, 255, 255, 255);

      CONFIG_RGB(world_chams_enemy_colour, 255, 64, 0);
      CONFIG_RGB(world_chams_team_colour, 64, 255, 0);

      CONFIG_RGB(world_chams_overlay_allied_team_colour, 185, 185, 185);
      CONFIG_RGB(world_chams_overlay_axis_team_colour, 255, 255, 255);
      CONFIG_RGB(world_chams_overlay_unassigned_colour, 255, 255, 255);

      CONFIG_RGB(world_chams_overlay_enemy_colour, 255, 64, 0);
      CONFIG_RGB(world_chams_overlay_team_colour, 64, 255, 0);

      CONFIG_RGB(world_glow_allied_team_colour, 200, 118, 118);
      CONFIG_RGB(world_glow_axis_team_colour, 145, 180, 202);
      CONFIG_RGB(world_glow_enemy_colour, 255, 128, 0);
      CONFIG_RGB(world_glow_team_colour, 128, 255, 0);
      CONFIG_RGB(world_glow_unassigned_team_colour, 255, 255, 255);

      bool player_esp_aimbot_target           = false;
      bool world_esp_aimbot_target            = false;

      bool player_chams_aimbot_target         = false;
      bool world_chams_aimbot_target          = false;

      bool player_chams_overlay_aimbot_target = false;
      bool world_chams_overlay_aimbot_target  = false;

      bool player_glow_aimbot_target          = true;
      bool world_glow_aimbot_target           = true;

      CONFIG_RGB(player_esp_aimbot_target_colour, 10, 255, 10);
      CONFIG_RGB(world_esp_aimbot_target_colour, 10, 255, 10);

      CONFIG_RGB(player_chams_aimbot_target_colour, 32, 255, 32);
      CONFIG_RGB(world_chams_aimbot_target_colour, 32, 255, 32);

      CONFIG_RGB(player_chams_overlay_aimbot_target_colour, 32, 255, 32);
      CONFIG_RGB(world_chams_overlay_aimbot_target_colour, 32, 255, 32);

      CONFIG_RGB(player_glow_aimbot_target_colour, 64, 255, 128);
      CONFIG_RGB(world_glow_aimbot_target_colour, 64, 255, 128);

      CONFIG_RGB(player_spawn_protected_text_colour, 100, 64, 192);

      bool show_information          = true;
      bool thirdperson               = false;
      i32  thirdperson_distance      = 150;
      u8   thirdperson_key           = VK_F5;
      bool thirdperson_usekey        = false;
      bool thirdperson_key_held      = true;
      bool thirdperson_key_toggle    = false;

      bool thirdperson_key_double_click     = false;

      bool thirdperson_right_offset_enabled             = false;
      bool thirdperson_right_offset_use_key             = false;
      i32  thirdperson_right_offset                     = 0;
      u8   thirdperson_right_offset_key                 = VK_XBUTTON2;
      u8   thirdperson_right_offset_invert_key          = 0x56;
      bool thirdperson_right_offset_invert_held         = false;
      bool thirdperson_right_offset_invert_toggle       = false;
      bool thirdperson_right_offset_invert_double_click = true;

      bool thirdperson_right_offset_held         = true;
      bool thirdperson_right_offset_toggle       = false;
      bool thirdperson_right_offset_double_click = false;

      i32  thirdperson_up_offset                        = 0;

      bool no_flash                              = false;

      bool backtrack_chams                       = true;
      CONFIG_RGBA(backtrack_chams_colour, 255, 255, 255, 45);
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_misc_settings{
public:
  union{
    struct{
      bool auto_pistol                        = true;

      bool pure_bypass                        = true;

      bool chat_notifications                 = true;
      bool chat_notification_acp              = true;

      bool chat_tags                          = true;
      bool chat_tags_localplayer              = true;
      bool chat_tags_friends                  = true;
      bool chat_tags_cheaters                 = true;

      bool block_server_chat_messages         = false;


      bool force_no_interpolation             = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};


class c_settings{
public:
  u32 version = SETTINGS_VERSION;

  c_aimbot_settings             aimbot[6];
  c_triggerbot_settings         triggerbot[6];
  c_player_esp_settings         player_esp[3];
  c_world_esp_settings          world_esp;
  c_player_chams_settings       player_chams[3];
  c_world_chams_settings        world_chams;
  c_world_glow_settings         world_glow;
  c_visual_settings             visual;
  c_misc_settings               misc;
  c_movement_settings           movement;
  c_third_person_settings       tp;
  c_freecam_settings            freecam;
  c_misc_privacy_mode_settings  privacy_mode;
  RESERVE_SETTINGS(20); // Decrease if you add a new setting, and ensure it's in a union and is paddded by SETTINGS_DATA_PAD!

  vec3i information_panel_pos;
};