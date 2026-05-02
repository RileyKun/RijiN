#pragma once

//#define TRACE_DEBUG

enum custom_filters{
  TR_CUSTOM_FILTER_NONE = 0,
  TR_CUSTOM_FILTER_HIT_TEAM = 1,
  TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS = 2,
  TR_CUSTOM_FILTER_IGNORE_ALL_ENTS = 3,
  TR_CUSTOM_FILTER_WRENCH_SWING = 4,
  TR_CUSTOM_FILTER_MELEE = 5,
  TR_CUSTOM_FILTER_ANTI_AIM = 6,
  TR_CUSTOM_FILTER_HIT_WEAPONS = 7,
  TR_CUSTOM_FILTER_HIT_TEAM_NO_PLAYERS = 8,
  TR_CUSTOM_FILTER_SPLASH_BOT_TEST = 9,
  TR_CUSTOM_FILTER_WRANGLER_AIMBOT_TEST = 10,
};

class c_trace_filter : public c_base_trace_filter{
public:
  bool should_hit(c_internal_base_entity* ent, uptr mask) override;
};

class c_interface_trace : public c_base_interface_trace{
public:

  CFLAG_O0 s_trace ray(vec3 start, vec3 end, uptr mask, u32 trace_type = TRACE_EVERYTHING, c_base_entity* ignore_entity = nullptr, u32 custom_trace_filter = 0){
    c_trace_filter filter;
    {
      filter.type                = trace_type;
      filter.custom_trace_filter = custom_trace_filter;
      filter.ignore_entity       = ignore_entity;
    }

    return internal_ray(filter, start, end, mask);
  }

  CFLAG_O0 s_trace ray_obb(vec3 start, vec3 end, vec3 mins, vec3 maxs, uptr mask, u32 trace_type = TRACE_EVERYTHING, c_base_entity* ignore_entity = nullptr, u32 custom_trace_filter = 0){
    s_ray   ray    = build_ray_mins_maxs(start, end, mins, maxs);

    c_trace_filter filter;
    {
      filter.type                = trace_type;
      filter.custom_trace_filter = custom_trace_filter;
      filter.ignore_entity       = ignore_entity;
    }

    return internal_ray_obb(filter, start, end, mins, maxs, mask);
  }

  s_trace utils_traceray(vec3 start, vec3 end, vec3 mins, vec3 maxs, uptr mask, c_base_entity* ignore_entity, i32 collision_group);
  s_trace player_move(vec3 start, vec3 end, c_base_player* player, bool brush_only = false);
};