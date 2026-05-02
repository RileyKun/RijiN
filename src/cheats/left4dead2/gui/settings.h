#pragma once

#define SETTINGS_DATA_PAD 0x1000
#define SETTINGS_VERSION 3

#define CONFIG_RGBA(name, r, g, b, a) float name[4] = {r,g,b,a};
#define CONFIG_RGB(name, r, g, b) CONFIG_RGBA(name, r, g, b, 255);

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
      bool target_zombies   = true;
      bool target_survivors = false;
      bool target_friends   = false;

      bool  target_method_smart_shot        = true;
      bool  target_method_closest_to_fov    = false;
      bool  target_method_distance          = false;
      bool  no_spread                       = true;

      bool silent_aim_none        = false;
      bool silent_aim_clientside  = true;
      bool silent_aim_serverside  = false;
      bool auto_shoot             = true;

      bool  autowall_enabled      = false;
      float autowall_min_dmg      = 50.f;
      bool  no_recoil             = false;

      bool ignore_idle_zombies   = true;
      bool ignore_burning_zombies = false;
      bool ignore_wandering_zombies = true;

      bool avoid_teammates = false;

      bool auto_bash_enabled                      = false;
      bool auto_bash_mode_smart                   = true;
      bool auto_bash_mode_aggressive              = false;
      bool auto_bash_target_special_only          = false;
      bool auto_bash_target_special_and_infected  = true;
      i32  auto_bash_max_fov                      = 180;
      bool target_special_infected                = true;
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
      bool health_bar                = true;
      i32  health_bar_fractions      = 4;

      bool friend_tag                = true;
      bool show_localplayer          = true;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_world_esp_settings{
public:
  union{
    struct{
      bool enabled                = true;
      bool object_enabled[6]      = {true, true, true, true, true, true};
      bool no_box[6]              = {false, false, false, false, false, false};
      bool box[6]                 = {true, true, true, true, true, true};
      bool box_outlines[6]        = {true, true, true, true, true, true};
      bool name[6]                = {true, true, true, true, true, true};
      bool distance[6]            = {true, true, true, true, true, true};
      float world_esp_render_dist = 32.f;
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
      float world_chams_render_dist = 32.f;
      bool object_enabled[6]        = {true, true, true, true, true, true};

      bool ignore_z                 = true;
      bool base_material_none       = false;
      bool base_material_flat       = false;
      bool base_material_shaded     = false;

      bool overlay_material_none    = false;
      bool overlay_material_fresnel = false;
      bool overlay_material_flat    = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_world_glow_settings{
public:
  union{
    struct{
      bool enabled           = true;
      bool object_enabled[6] = {true, true, true, true, true, true};
      float world_glow_render_dist = 32.f;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_visual_settings{
public:
  union{
    struct{

      // Glow main toggle
      bool glow_enabled                       = true;

      // Glow player settings
      bool player_glow_enabled                = true;
      bool player_glow_render_friendlies      = true;
      bool player_glow_render_enemies         = true;
      bool player_glow_render_cloaked_spies   = true;
      bool player_glow_render_steamfriends    = true;
      bool player_glow_render_localplayer     = true;

       // Glow settings
      bool glow_stencil                       = true;
      bool glow_blur                          = false;
      i32  glow_thickness                     = 1;

      CONFIG_RGB(menu_foreground_colour, 25, 118, 210);
      CONFIG_RGB(menu_background_colour, 41, 44, 51);
      CONFIG_RGB(box_player_colour_friendly, 0, 255, 0);
      CONFIG_RGB(box_player_colour_enemy, 255, 0, 0);
      CONFIG_RGB(box_colour_infected, 130, 186, 217);
      CONFIG_RGB(box_colour_hunter, 181, 96, 96);
      CONFIG_RGB(box_colour_jockey, 245, 119, 47);
      CONFIG_RGB(box_colour_charger, 235, 136, 23);
      CONFIG_RGB(box_colour_smoker, 166, 157, 157);
      CONFIG_RGB(box_colour_boomer, 168, 110, 82);
      CONFIG_RGB(box_colour_spitter, 189, 237, 100);
      CONFIG_RGB(box_colour_tank, 207, 8, 8);
      CONFIG_RGB(box_colour_witch, 191, 99, 214);

      CONFIG_RGB(world_esp_grenade_colour, 255, 32, 32);
      CONFIG_RGB(world_esp_dropped_weapon_colour, 192, 192, 255);
      CONFIG_RGB(world_esp_dropped_items_colour, 200, 200, 200);
      CONFIG_RGB(world_esp_health_items_colour, 0, 255, 64);

      CONFIG_RGBA(chams_player_colour_friendly, 0, 255, 0, 64);
      CONFIG_RGBA(chams_player_colour_enemy, 255, 0, 0, 64);
      CONFIG_RGBA(chams_colour_infected, 130, 186, 217, 64);
      CONFIG_RGBA(chams_colour_hunter, 181, 96, 96, 64);
      CONFIG_RGBA(chams_colour_jockey, 245, 119, 47, 64);
      CONFIG_RGBA(chams_colour_charger, 235, 136, 23, 64);
      CONFIG_RGBA(chams_colour_smoker, 166, 157, 157, 64);
      CONFIG_RGBA(chams_colour_boomer, 168, 110, 82, 64);
      CONFIG_RGBA(chams_colour_spitter, 189, 237, 100, 64);
      CONFIG_RGBA(chams_colour_tank, 207, 8, 8, 64);
      CONFIG_RGBA(chams_colour_witch, 191, 99, 214, 64);

      CONFIG_RGBA(world_chams_grenade_colour, 255, 32, 32, 64);
      CONFIG_RGBA(world_chams_dropped_weapon_colour, 192, 192, 255, 64);
      CONFIG_RGBA(world_chams_dropped_items_colour, 200, 200, 200, 64);
      CONFIG_RGBA(world_chams_health_items_colour, 0, 255, 64, 64);

      CONFIG_RGBA(chams_overlay_player_colour_friendly, 0, 255, 0, 64);
      CONFIG_RGBA(chams_overlay_player_colour_enemy, 255, 0, 0, 64);
      CONFIG_RGBA(chams_overlay_colour_infected, 130, 186, 217, 64);
      CONFIG_RGBA(chams_overlay_colour_hunter, 181, 96, 96, 64);
      CONFIG_RGBA(chams_overlay_colour_jockey, 245, 119, 47, 64);
      CONFIG_RGBA(chams_overlay_colour_charger, 235, 136, 23, 64);
      CONFIG_RGBA(chams_overlay_colour_smoker, 166, 157, 157, 64);
      CONFIG_RGBA(chams_overlay_colour_boomer, 168, 110, 82, 64);
      CONFIG_RGBA(chams_overlay_colour_spitter, 189, 237, 100, 64);
      CONFIG_RGBA(chams_overlay_colour_tank, 207, 8, 8, 64);
      CONFIG_RGBA(chams_overlay_colour_witch, 191, 99, 214, 64);

      CONFIG_RGBA(world_chams_overlay_grenade_colour, 255, 32, 32, 64);
      CONFIG_RGBA(world_chams_overlay_dropped_weapon_colour, 192, 192, 255, 64);
      CONFIG_RGBA(world_chams_overlay_dropped_items_colour, 200, 200, 200, 64);
      CONFIG_RGBA(world_chams_overlay_health_items_colour, 0, 255, 64, 64);

      CONFIG_RGB(glow_player_colour_friendly, 0, 255, 0);
      CONFIG_RGB(glow_player_colour_enemy, 255, 0, 0);
      CONFIG_RGB(glow_colour_infected, 130, 186, 217);
      CONFIG_RGB(glow_colour_hunter, 181, 96, 96);
      CONFIG_RGB(glow_colour_jockey, 245, 119, 47);
      CONFIG_RGB(glow_colour_charger, 235, 136, 23);
      CONFIG_RGB(glow_colour_smoker, 166, 157, 157);
      CONFIG_RGB(glow_colour_boomer, 168, 110, 82);
      CONFIG_RGB(glow_colour_spitter, 189, 237, 100);
      CONFIG_RGB(glow_colour_tank, 207, 8, 8);
      CONFIG_RGB(glow_colour_witch, 191, 99, 214);

      CONFIG_RGB(world_glow_grenade_colour, 255, 32, 32);
      CONFIG_RGB(world_glow_dropped_weapon_colour, 192, 192, 255);
      CONFIG_RGB(world_glow_dropped_items_colour, 200, 200, 200);
      CONFIG_RGB(world_glow_health_items_colour, 0, 255, 64);

      bool no_visual_recoil;

      CONFIG_RGB(player_esp_friend_colour, 96, 255, 164);
      CONFIG_RGB(player_chams_friend_colour, 96, 255, 164);
      CONFIG_RGB(player_chams_overlay_friend_colour, 96, 255, 164);
      CONFIG_RGB(player_glow_friend_colour, 96, 255, 164);


      bool override_fov       = false;
      float override_fov_val  = 90.f;

      CONFIG_RGB(world_esp_objective_colour, 108, 64, 255);
      CONFIG_RGB(world_chams_objective_colour, 108, 64, 255);
      CONFIG_RGB(world_chams_overlay_objective_colour, 108, 64, 255);
      CONFIG_RGB(world_glow_objective_colour, 108, 64, 255);
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_misc_settings{
public:
  union{
    struct{
      bool bhop             = false;
      bool auto_pistol      = false;
      bool teleport         = false;
      bool teleport_use_key = true;
      u8   teleport_key     = VK_F5;

      bool auto_strafe                        = false;
      bool auto_strafe_type_normal            = false;
      bool auto_strafe_type_directional       = true;
      float auto_strafe_retrack               = 75.f;
      bool auto_strafe_avoid_walls_disabled   = true;
      bool auto_strafe_avoid_walls_normal     = false;
      bool auto_strafe_avoid_walls_adv        = false;

      bool speedhack          = false;
      bool speedhack_use_key  = false;
      i32  speedhack_commands = 6;
      u8   speedhack_key      = VK_SHIFT;

      bool break_tickbase     = false;
      u8   break_tickbase_key = VK_LMENU;

      bool godmode_exploit     = false;
      u8   godmode_exploit_key = 'P';

      i32  break_tickbase_amount          = 200;

      bool force_no_interpolation               = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};


class c_settings{
public:
  u32 version = SETTINGS_VERSION;

  c_aimbot_settings           aimbot;
  c_player_esp_settings       player_esp[3];
  c_world_esp_settings        world_esp;
  c_visual_settings           visual;
  c_player_chams_settings     player_chams[3];
  c_world_chams_settings      world_chams;
  c_world_glow_settings       world_glow;
  c_misc_settings             misc;
  c_third_person_settings     tp;
  c_viewmodel_offset_settings vm;
  RESERVE_SETTINGS(24); // Decrease if you add a new setting, and ensure it's in a union and is paddded by SETTINGS_DATA_PAD!
};