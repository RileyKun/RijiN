#pragma once

enum objects_type{
  OBJECT_TYPE_INVALID = -1,
  OBJECT_TYPE_GRENADE = 0,
  OBJECT_TYPE_DROPPED_WEAPON,
  OBJECT_TYPE_ITEMS,
  OBJECT_TYPE_OBJECTIVE,

  OBJECT_TYPE_MAX,
};

enum world_esp_render_type{
  WORLD_ESP_RENDER_TYPE_TEXT = 0,
  WORLD_ESP_RENDER_TYPE_CHAMS,
  WORLD_ESP_RENDER_TYPE_GLOW,
  WORLD_ESP_RENDER_TYPE_CHAMS_OVERLAY,
};

class c_world_esp{
public:
  u32 get_object_index(c_base_entity* entity);
  bool should_render_entity(c_base_entity* entity, u32 type);
  std::wstring get_object_name(c_base_entity* entity, u32 type);
  bool should_draw(c_base_player* localplayer, c_base_entity* entity);
  bool should_draw_extra(c_base_entity* entity, u32 type, u32 render_type);
  colour get_draw_colour(c_base_entity* entity, i32 colour_type = WORLD_ESP_RENDER_TYPE_TEXT);
  void run();
};

CLASS_EXTERN(c_world_esp, world_esp);