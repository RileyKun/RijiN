#pragma once

enum player_esp_render_type{
  PLAYER_ESP_RENDER_TYPE_TEXT = 0,
  PLAYER_ESP_RENDER_TYPE_CHAMS,
  PLAYER_ESP_RENDER_TYPE_GLOW,
  PLAYER_ESP_RENDER_TYPE_CHAMS_OVERLAY,
};

class c_player_esp{
public:
  bool   get_esp_name(c_base_player* p, wchar_t* out);
  colour get_draw_colour(c_base_player* p, i32 colour_type = PLAYER_ESP_RENDER_TYPE_TEXT);
  template<auto c>
  void add_flags(c_base_player* p, c_esp_data<c>* esp_data);
  void run();
};

CLASS_EXTERN(c_player_esp, player_esp);