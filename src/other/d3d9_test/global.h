#pragma once

struct s_entry_data{
  i8      name[2048];
  wchar_t wide_name[2048];
};

class c_global{
public:

  void* target_window     = nullptr;
  c_font_d3d9* gui_font   = nullptr;
  c_font_d3d9* gui_font_emoji = nullptr;
  c_font_d3d9* esp_font_large = nullptr;
  gcs_menu*    menu       = nullptr;
  bool menu_open          = true;

  i32 playerlist_current_player_index;
  bool playerlist_has_valid_player;

  gcs_base_info_panel* info_panel = nullptr;

  bool is_option_enabled = false;

  s_entry_data players[1];
  i32 count = 0;
  void add_player(std::wstring name){

  }
};

CLASS_EXTERN(c_global, global);