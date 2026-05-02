#pragma once

#define SETTINGS_DATA_PAD 256
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

      bool silent_aim_none        = false;
      bool silent_aim_clientside  = true;
      bool silent_aim_serverside  = false;
      bool auto_shoot             = true;

      bool  autowall_enabled      = false;
      float autowall_min_dmg      = 50.f;
      bool  no_recoil             = false;
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
      i32  health_bar_fractions      = 4;

      bool friend_tag                = true;
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

      bool object_enabled[8] = {true, true, true, true, true, true, true, true};
      bool box[8]            = {true, true, true, true, true, true, true, true};
      bool box_outlines[8]   = {true, true, true, true, true, true, true, true};
      bool name[8]           = {true, true, true, true, true, true, true, true};
      bool distance[8]       = {true, true, true, true, true, true, true, true};
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
      float base_alpha              = 50.f;

      bool overlay_material_none    = false;
      bool overlay_material_fresnel = true;
      bool overlay_material_flat    = false;
      float overlay_alpha           = 50.f;
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
      bool object_enabled[8]        = {true, true, true, true, true, true, true, true};

      bool ignore_z                 = true;
      bool base_material_none       = false;
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
      bool render_friendlies = true;
      bool object_enabled[8] = {true, true, true, true, true, true, true, true};

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
      bool no_drunk_effects                     = true;

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

      CONFIG_RGB(player_esp_vigil_team_colour,  0, 92, 255);
      CONFIG_RGB(player_esp_desper_team_colour, 255, 0, 32);
      CONFIG_RGB(player_esp_bandio_team_colour, 255, 232, 0);
      CONFIG_RGB(player_esp_rangers_team_colour, 32, 255, 0);

      CONFIG_RGB(player_esp_enemy_colour, 255, 64, 0);
      CONFIG_RGB(player_esp_team_colour, 64, 255, 0);

      CONFIG_RGB(player_esp_friend_colour, 96, 255, 164);

      CONFIG_RGB(player_esp_friend_vigil_team_colour,  0, 184, 255);
      CONFIG_RGB(player_esp_friend_desper_team_colour, 255, 48, 32);
      CONFIG_RGB(player_esp_friend_bandio_team_colour, 255, 232, 64);
      CONFIG_RGB(player_esp_friend_rangers_team_colour, 132, 255, 0);

      CONFIG_RGB(player_esp_friend_enemy_colour, 255, 128, 0);
      CONFIG_RGB(player_esp_friend_team_colour, 128, 255, 0);

      bool world_esp_use_team_colours          = true;
      bool world_esp_use_enemyteam_colours     = false;

      CONFIG_RGB(world_esp_vigil_team_colour,  0, 92, 255);
      CONFIG_RGB(world_esp_desper_team_colour, 255, 0, 32);
      CONFIG_RGB(world_esp_bandio_team_colour, 255, 232, 0);
      CONFIG_RGB(world_esp_rangers_team_colour, 32, 255, 0);

      CONFIG_RGB(world_esp_unassigned_colour, 255, 255, 255);

      CONFIG_RGB(world_esp_enemy_colour, 255, 64, 0);
      CONFIG_RGB(world_esp_team_colour, 64, 255, 0);
      CONFIG_RGB(world_esp_whiskey_colour, 0, 255, 92);

      // Player chams
      CONFIG_RGB(player_chams_vigil_team_colour,  0, 92, 255);
      CONFIG_RGB(player_chams_desper_team_colour, 255, 0, 32);
      CONFIG_RGB(player_chams_bandio_team_colour, 255, 232, 0);
      CONFIG_RGB(player_chams_rangers_team_colour, 32, 255, 0);

      CONFIG_RGB(player_chams_friend_colour, 96, 255, 164);
      CONFIG_RGB(player_chams_friend_vigil_team_colour,  0, 184, 255);
      CONFIG_RGB(player_chams_friend_desper_team_colour, 255, 48, 32);
      CONFIG_RGB(player_chams_friend_bandio_team_colour, 255, 232, 64);
      CONFIG_RGB(player_chams_friend_rangers_team_colour, 132, 255, 0);

      CONFIG_RGB(player_chams_friend_enemy_colour, 255, 128, 0);
      CONFIG_RGB(player_chams_friend_team_colour, 128, 255, 0);
      CONFIG_RGB(player_chams_enemy_colour, 255, 64, 0);
      CONFIG_RGB(player_chams_team_colour, 64, 255, 0);

      CONFIG_RGB(player_chams_overlay_vigil_team_colour,  0, 92, 223);
      CONFIG_RGB(player_chams_overlay_desper_team_colour, 223, 0, 32);
      CONFIG_RGB(player_chams_overlay_bandio_team_colour, 223, 232, 0);
      CONFIG_RGB(player_chams_overlay_rangers_team_colour, 32, 223, 0);

      CONFIG_RGB(player_chams_overlay_enemy_colour, 255, 160, 60);
      CONFIG_RGB(player_chams_overlay_team_colour, 20, 255, 90);

      CONFIG_RGB(player_chams_overlay_friend_colour, 96, 255, 164);
      CONFIG_RGB(player_chams_overlay_friend_enemy_colour, 255, 150, 0);
      CONFIG_RGB(player_chams_overlay_friend_team_colour, 0, 255, 125);

      CONFIG_RGB(player_chams_overlay_friend_vigil_team_colour,  0, 184, 223);
      CONFIG_RGB(player_chams_overlay_friend_desper_team_colour, 223, 48, 32);
      CONFIG_RGB(player_chams_overlay_friend_bandio_team_colour, 223, 232, 64);
      CONFIG_RGB(player_chams_overlay_friend_rangers_team_colour, 132, 255, 0);


      // Player glow
      CONFIG_RGB(player_glow_vigil_team_colour,  0, 92, 255);
      CONFIG_RGB(player_glow_desper_team_colour, 255, 0, 32);
      CONFIG_RGB(player_glow_bandio_team_colour, 255, 232, 0);
      CONFIG_RGB(player_glow_rangers_team_colour, 32, 255, 0);

      CONFIG_RGB(player_glow_friend_colour, 96, 255, 164);

      CONFIG_RGB(player_glow_friend_vigil_team_colour,  0, 184, 255);
      CONFIG_RGB(player_glow_friend_desper_team_colour, 255, 48, 32);
      CONFIG_RGB(player_glow_friend_bandio_team_colour, 255, 232, 64);
      CONFIG_RGB(player_glow_friend_rangers_team_colour, 132, 255, 0);

      CONFIG_RGB(player_glow_friend_enemy_colour, 255, 160, 60);
      CONFIG_RGB(player_glow_friend_team_colour, 20, 255, 90);

      CONFIG_RGB(player_glow_enemy_colour, 255, 128, 0);
      CONFIG_RGB(player_glow_team_colour, 128, 255, 0);

      // NEXT (REMOVE ME)
      CONFIG_RGB(world_chams_vigil_team_colour,  0, 92, 255);
      CONFIG_RGB(world_chams_desper_team_colour, 255, 0, 32);
      CONFIG_RGB(world_chams_bandio_team_colour, 255, 232, 0);
      CONFIG_RGB(world_chams_rangers_team_colour, 32, 255, 0);
      CONFIG_RGB(world_chams_unassigned_colour, 255, 255, 255);

      CONFIG_RGB(world_chams_enemy_colour, 255, 64, 0);
      CONFIG_RGB(world_chams_team_colour, 64, 255, 0);

      CONFIG_RGB(world_chams_whiskey_colour, 0, 255, 32);

      CONFIG_RGB(world_chams_overlay_vigil_team_colour,  0, 92, 223);
      CONFIG_RGB(world_chams_overlay_desper_team_colour, 223, 0, 32);
      CONFIG_RGB(world_chams_overlay_bandio_team_colour, 223, 232, 0);
      CONFIG_RGB(world_chams_overlay_rangers_team_colour, 32, 223, 0);

      CONFIG_RGB(world_chams_overlay_unassigned_colour, 255, 255, 255);
      CONFIG_RGB(world_chams_overlay_enemy_colour, 255, 64, 0);
      CONFIG_RGB(world_chams_overlay_team_colour, 64, 255, 0);
      CONFIG_RGB(world_chams_overlay_whiskey_colour, 0, 200, 0);

      CONFIG_RGB(world_glow_vigil_team_colour,  0, 92, 255);
      CONFIG_RGB(world_glow_desper_team_colour, 255, 0, 32);
      CONFIG_RGB(world_glow_bandio_team_colour, 255, 232, 0);
      CONFIG_RGB(world_glow_rangers_team_colour, 32, 255, 0);

      CONFIG_RGB(world_glow_enemy_colour, 255, 128, 0);
      CONFIG_RGB(world_glow_team_colour, 128, 255, 0);
      CONFIG_RGB(world_glow_unassigned_team_colour, 255, 255, 255);
      CONFIG_RGB(world_glow_whiskey_colour, 0, 255, 92);

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

      CONFIG_RGB(player_esp_unassigned_colour, 255, 255, 255);
      CONFIG_RGB(player_chams_unassigned_colour, 255, 255, 255);
      CONFIG_RGB(player_chams_overlay_unassigned_colour, 255, 255, 255);
      CONFIG_RGB(player_glow_unassigned_colour, 255, 255, 255);

      CONFIG_RGB(world_esp_dynamite_colour, 255, 32, 0);
      CONFIG_RGB(world_chams_dynamite_colour, 255, 32, 0);
      CONFIG_RGB(world_chams_overlay_dynamite_colour, 255, 64, 0);
      CONFIG_RGB(world_glow_dynamite_colour, 255, 32, 0);

      CONFIG_RGB(world_esp_horse_colour, 255, 128, 32);
      CONFIG_RGB(world_chams_horse_colour, 255, 128, 32);
      CONFIG_RGB(world_chams_overlay_horse_colour, 255, 192, 32);
      CONFIG_RGB(world_glow_horse_colour, 255, 128, 32);
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
      bool server_crasher     = false;
      u8   server_crasher_key = VK_F5;
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
};