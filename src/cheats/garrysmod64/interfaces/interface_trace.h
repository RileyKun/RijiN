#pragma once

//#define TRACE_DEBUG

enum custom_filters{
  TR_CUSTOM_FILTER_NONE = 0,
  TR_CUSTOM_FILTER_HIT_TEAM = 1,
  TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS = 2,
  TR_CUSTOM_FILTER_IGNORE_ALL_ENTS = 3,
};

class c_trace_filter{
public:
  virtual bool should_hit(c_base_entity* ent, u64 mask);

  virtual trace_type get_trace_type(){
    return type;
  }
  
  virtual void unknown(){
    
  }

  trace_type      type;
  u32             custom_trace_filter;
  c_base_entity*  ignore_entity;
};

class c_interface_trace{
public:
  ALWAYSINLINE s_ray build_ray(vec3 start, vec3 end){
    s_ray ray;
    {
      ray.start         = vec3(start.x, start.y, start.z);
      ray.delta         = vec3(end.x - start.x, end.y - start.y, end.z - start.z);
      ray.start_offset  = vec3();
      ray.extents       = vec3();
      ray.unk           = nullptr;
      ray.is_ray        = true;
      ray.is_swept      = ray.delta.length_sqr() != 0.f;
    }
    return ray;
  }

  ALWAYSINLINE s_ray build_ray_mins_maxs(vec3 start, vec3 end, vec3 mins, vec3 maxs){
    s_ray ray;
    {
      ray.delta.x          = end.x - start.x;
      ray.delta.y          = end.y - start.y;
      ray.delta.z          = end.z - start.z;
      ray.is_swept         = ray.delta.length_sqr() != 0.f;

      ray.extents.x        = maxs.x - mins.x;
      ray.extents.y        = maxs.y - mins.y;
      ray.extents.z        = maxs.z - mins.z;
      ray.extents         *= 0.5f;
      ray.unk              = nullptr;

      ray.is_ray           = ray.extents.length_sqr() < 1e-6;

      ray.start_offset.x   = mins.x + maxs.x;
      ray.start_offset.y   = mins.y + maxs.y;
      ray.start_offset.z   = mins.z + maxs.z;
      ray.start_offset    *= 0.5f;

      ray.start.x          = start.x + ray.start_offset.x;
      ray.start.y          = start.y + ray.start_offset.y;
      ray.start.z          = start.z + ray.start_offset.z;
      ray.start_offset    *= -1.f;
    }
    return ray;
  }

  // TODO: build ray with mins maxs extents

  s_trace ray(vec3 start, vec3 end, u64 mask, u32 trace_type = TRACE_EVERYTHING, c_base_entity* ignore_entity = nullptr, u32 custom_trace_filter = 0);
  s_trace ray_obb(vec3 start, vec3 end, vec3 mins, vec3 maxs, u64 mask, u32 trace_type = TRACE_EVERYTHING, c_base_entity* ignore_entity = nullptr, u32 custom_trace_filter = 0);
  s_trace ray_to_entity(vec3 start, vec3 end, u64 mask, c_base_entity* entity);
  s_trace utils_traceray(vec3 start, vec3 end, vec3 mins, vec3 maxs, u32 mask, c_base_entity* ignore_entity, i32 collision_group);
  s_trace player_move(vec3 start, vec3 end, c_base_player* player, bool brush_only = false);
};