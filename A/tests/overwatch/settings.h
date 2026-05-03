#pragma once

#define SETTINGS_DATA_PAD 256
#define SETTINGS_VERSION 4

#define CONFIG_RGBA(name, r, g, b, a) float name[4] = {r,g,b,a};
#define CONFIG_RGB(name, r, g, b) CONFIG_RGBA(name, r, g, b, 255);

class c_aimbot_settings{
public:
  //union{
  //  struct{
      // aimbot
      bool enabled               = true;
      u8   aim_hotkey            = VK_XBUTTON2;

      // silent aim related
      bool silent_aim_none       = true;
      bool silent_aim_clientside = false;

      // fov related
      float aim_fov = 10.f;
      float aim_dynamic_fov = 0.f;
      
      float aim_random_point = 0.0f;
      
      float aim_wait_time = 80.0f;
      
      float aim_autofire_time = 0.0f;
      
      // smooth
      bool aim_use_const = true;
      
      float aim_smooth = 10.f;
      float aim_smooth_const = 2.5f;
      
      float aim_smooth_correction = 100.0f;
      float aim_smooth_accel = 15.0f;
      float aim_smooth_sine = 50.0f;
      
      // aim hitboxes
      bool aim_hitbox_head  = true;
      bool aim_hitbox_body  = true;
      bool aim_hitbox_limbs = false;
      
      // trigger
      bool trig_enabled = true;
      u8 trig_hotkey = VK_XBUTTON2;
      
      bool trig_backtrack = true;
      bool trig_forwardtrack = true;
      
      bool trig_shoot_objects = true;
      
      float trig_min_charge = 100.0f;
      
      // aim hitboxes
      bool trig_hitbox_head  = true;
      bool trig_hitbox_body  = true;
      bool trig_hitbox_limbs = true;
      
      // backtrack
      
      bool position_enabled = true;
      float position_backtrack = 16.0f;
      float position_forwardtrack = 4.0f;
      
      // ghost
      bool ghost_aim = false;
      float ghost_aim_amount = 50.0f;
      
      // rage
      bool rage_enabled = false;
      u8   rage_hotkey = VK_XBUTTON2;
      bool rage_anti_aim_enabled        = false;
      bool rage_anti_aim_on_key         = false;
      u8   rage_anti_aim_hotkey         = VK_MENU;
      bool rage_anti_aim_pitch_disabled = false;
      bool rage_anti_aim_pitch_up       = false;
      bool rage_anti_aim_pitch_down     = true;
      bool rage_anti_aim_yaw_disabled   = false;
      bool rage_anti_aim_yaw_backwards  = false;
      bool rage_anti_aim_yaw_spin       = true;
      bool rage_anti_aim_yaw_jitter     = false;
      bool rage_anti_aim_yaw_weed_boss  = false;

      float roadhook_hooking_smoothing = 15.f;

      bool rage_anti_aim_yaw_360_jitter = false;
      bool rage_anti_aim_pitch_jitter = false;

      i32 zen_wait_orbs = 3;
      u8  zen_wait_orbs_hotkey = VK_XBUTTON1;

      bool sticky_aim = false;

      PAD(256 - (sizeof(bool) + sizeof(bool) + sizeof(float)));
    //};
  //  PAD(SETTINGS_DATA_PAD);
  //};
};

class c_esp_settings{
public:
  union{
    struct{
      bool enabled                   = true;
      bool friend_tag                = true;

      bool no_box                    = true;
      bool box                       = false;
      bool box_outlines              = false;
      bool name                      = true;
      bool distance                  = true;
      bool health_bar                = true;
      i32  health_bar_fractions      = 4;

      bool glow_disabled     = false;
      bool glow_custom_color = false;
      bool glow_health_based = true;
      CONFIG_RGBA(glow_health_rgb, 255, 255, 255, 255);

      CONFIG_RGBA(box_colour, 255, 255, 255, 255);
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
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_misc_settings{
public:
  union{
    struct{
      bool bhop = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};


class c_settings{
public:
  u32 version = SETTINGS_VERSION;

  c_aimbot_settings aimbot[33];
  c_esp_settings    esp[2];
  c_visual_settings visual;
  c_misc_settings   misc;
};