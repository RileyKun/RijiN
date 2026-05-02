#pragma once

#define SETTINGS_DATA_PAD 256
#define SETTINGS_VERSION 1

#define CONFIG_RGBA(name, r, g, b, a) float name[4] = {r,g,b,a};
#define CONFIG_RGB(name, r, g, b) CONFIG_RGBA(name, r, g, b, 255);

enum player_esp_settings_type{
  PLAYER_ESP_SETTING_ENEMY,
  PLAYER_ESP_SETTING_FRIEND,
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

      // Glow main toggle
      bool glow_enabled                       = true;

      // Glow player settings
      bool player_glow_enabled                = true;
      bool player_glow_render_enemies         = true;
      bool player_glow_render_steamfriends    = true;

       // Glow settings
      bool glow_stencil                       = true;
      bool glow_blur                          = false;
      i32  glow_thickness                     = 1;


      CONFIG_RGB(player_esp_enemy_colour, 255, 64, 0);
      CONFIG_RGB(player_esp_friend_colour, 96, 255, 164);
      CONFIG_RGB(world_esp_unassigned_colour, 255, 255, 255);
      CONFIG_RGB(world_esp_enemy_colour, 255, 64, 0);

      CONFIG_RGB(player_chams_friend_colour, 96, 255, 164);
      CONFIG_RGB(player_chams_enemy_colour, 255, 64, 0);

      CONFIG_RGB(player_chams_overlay_enemy_colour, 255, 160, 60);;
      CONFIG_RGB(player_chams_overlay_friend_colour, 96, 255, 164);

      // Player glow
      CONFIG_RGB(player_glow_friend_colour, 96, 255, 164);
      CONFIG_RGB(player_glow_enemy_colour, 255, 128, 0);

      CONFIG_RGB(world_chams_unassigned_colour, 255, 255, 255);
      CONFIG_RGB(world_chams_enemy_colour, 255, 64, 0);

      CONFIG_RGB(world_chams_overlay_unassigned_colour, 255, 255, 255);
      CONFIG_RGB(world_chams_overlay_enemy_colour, 255, 64, 0);

      CONFIG_RGB(world_glow_enemy_colour, 255, 128, 0);
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
      CONFIG_RGB(player_chams_aimbot_target_colour, 32, 255, 32);
      CONFIG_RGB(player_chams_overlay_aimbot_target_colour, 32, 255, 32);
      CONFIG_RGB(player_glow_aimbot_target_colour, 64, 255, 128);

      CONFIG_RGB(player_esp_unassigned_colour, 255, 255, 255);
      CONFIG_RGB(player_chams_unassigned_colour, 255, 255, 255);
      CONFIG_RGB(player_chams_overlay_unassigned_colour, 255, 255, 255);
      CONFIG_RGB(player_glow_unassigned_colour, 255, 255, 255);

      CONFIG_RGB(world_esp_active_grenade_colour, 255, 32, 32);
      CONFIG_RGB(world_esp_dropped_weapon_colour, 188, 188, 200);
      CONFIG_RGB(world_esp_objective_colour, 0, 200, 255);


      CONFIG_RGB(world_chams_active_grenade_colour, 255, 32, 32);
      CONFIG_RGB(world_chams_dropped_weapon_colour, 188, 188, 200);
      CONFIG_RGB(world_chams_objective_colour, 0, 200, 255);

      CONFIG_RGB(world_chams_overlay_active_grenade_colour, 255, 32, 32);
      CONFIG_RGB(world_chams_overlay_dropped_weapon_colour, 188, 188, 200);
      CONFIG_RGB(world_chams_overlay_objective_colour, 0, 200, 255);

      CONFIG_RGB(world_glow_active_grenade_colour, 255, 32, 32);
      CONFIG_RGB(world_glow_dropped_weapon_colour, 188, 188, 200);
      CONFIG_RGB(world_glow_objective_colour, 0, 200, 255);


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

      bool speedhack = false;
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