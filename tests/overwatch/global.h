#pragma once

class gcs_menu;
class c_settings;

#define aim_config config->aimbot[local->object.hero]
#define config global->settings
class c_global{
public:
  // Hooks
  void* d3d11_hook_trp = 0;

  // Signatures
  void* user_cmd_func_address   = nullptr;
  void* key_func_address        = nullptr;
  void* key_func2_address       = nullptr;
  void* glow_handle_player      = nullptr;
  void* mouse_move_func_address = nullptr;
  void* sens_func_address       = nullptr;
  void* ray_trace               = nullptr;
  void* ray_add_filter          = nullptr;
  void* skill_struct            = nullptr;

  // level
  u32 access_level = 0;

  // fonts
  c_font_base* font_default = nullptr;
  c_font_base* font_esp_big = nullptr;

  // gcs menu instances
  gcs_menu* menu = nullptr;

  // settings
  c_settings* settings = nullptr;

  bool menu_open = false;
};

CLASS_EXTERN(c_global, global);