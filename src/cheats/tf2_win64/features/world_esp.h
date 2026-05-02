#pragma once

enum objects_type{
  OBJECT_TYPE_INVALID = -1,
  OBJECT_TYPE_SENTRY = 0,
  OBJECT_TYPE_DISPENSER,
  OBJECT_TYPE_TELEPORTER,
  OBJECT_TYPE_OBJECTIVE,
  OBJECT_TYPE_AMMOPACK,
  OBJECT_TYPE_HEALTHPACK,
  OBJECT_TYPE_PROJECTILES,
  OBJECT_TYPE_WORLD_ENTITY,
  OBJECT_TYPE_MAX,
};

enum world_esp_render_type{
  WORLD_ESP_RENDER_TYPE_TEXT = 0,
  WORLD_ESP_RENDER_TYPE_CHAMS,
  WORLD_ESP_RENDER_TYPE_GLOW,
  WORLD_ESP_RENDER_TYPE_CHAMS_OVERLAY,
};

enum world_esp_buffer_type{
  WORLD_ESP_TEXT_UNKNOWN = 0,

  DISPENSER_TEXT,
  TELEPORTER_ENTRANCE_TEXT,
  TELEPORTER_EXIT_TEXT,
  OBJECTIVE_TEXT,
  AMMOPACK_TEXT,
  HEALTHPACK_TEXT,
  LUNCHABLE_TEXT,
  BOSS_TEXT,
  MONEY_TEXT,
  BOMB_TEXT,
  REVIVE_MARKER_TEXT,
  HALLOWEEN_GIFT_TEXT,
  HALLOWEEN_PICKUP_TEXT,
  NORMAL_SPELLBOOK_TEXT,
  RARE_SPELLBOOK_TEXT,
  MINI_SENTRY_TEXT,
  SENTRY_TEXT,


  CONTROLLED_TEXT,
  SAPPED_TEXT,
  DISABLED_TEXT,
  CONSTRUCTING_TEXT,
  SENTRY_NO_AMMO_TEXT,
  INTEL_HOME_TEXT,
  INTEL_STOLEN_TEXT,
  INTEL_DROPPED_TEXT,

  CRITICAL_TEXT,
  REFLECTED_TEXT,
};

#define WORLD_ESP_BUF_SIZE 128
#define WORLD_ESP_MAX_DIST_CACHE 1024
class c_world_esp{
public:

  void precache();
  u32  get_object_index(c_base_entity* ent);
  bool should_render_extra_entity(c_base_entity* ent, u32 type);
  std::wstring get_object_name(c_base_entity* ent, u32 type);

  bool should_draw(c_base_entity* ent);
  bool should_draw_extra(c_base_entity* ent, u32 type, u32 render_type);
  colour get_draw_colour(c_base_entity* ent, i32 type, i32 colour_type = WORLD_ESP_RENDER_TYPE_TEXT);
  colour get_bland_colour(c_base_entity* entity);

  template<auto c>
  void add_flags(c_base_entity* ent, u32 type, c_esp_data<c>* esp_data);

  u32 fetch_render_flags(u32 type, c_world_esp_settings* settings);
  void run();
private:
  wchar_t _str[WORLD_ESP_BUF_SIZE][128]; // 16.3kb
  wchar_t _dist[1024][10]; // 
  wchar_t _intel_returns_time[301][19];
  wchar_t _building_level[4][8];
  ALWAYSINLINE wchar_t* get_str(u32 i){
    return _str[i];
  }
};

CLASS_EXTERN(c_world_esp, world_esp);