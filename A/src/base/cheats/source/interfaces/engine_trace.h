#pragma once

class c_base_trace_filter{
public:
  virtual bool should_hit(c_internal_base_entity* ent, uptr mask){
    DBG("[-] c_base_trace_filter::should_hit - no override set!\n");
    return false;
  }

  virtual trace_type get_trace_type(){
    return type;
  }

#if defined(GMOD_CHEAT)
  virtual void unknown(){
  }
#endif

  trace_type               type;
  u32                      custom_trace_filter;
  c_internal_base_entity*  ignore_entity;
};


class c_base_generic_trace_filter : public c_base_trace_filter{
public:
  bool should_hit(c_internal_base_entity* ent, uptr mask) override{
    if(ent == nullptr)
      return false;

    if(ignore_entity == ent)
      return false;

    return true;
  } 
};

class c_base_generic_world_only_trace_filter : public c_base_trace_filter{
public:
  bool should_hit(c_internal_base_entity* ent, uptr mask) override{
    if(ent == nullptr)
      return false;
    
    return ent->get_index() == 0;
  } 
};

class c_base_interface_trace{
public:
  ALWAYSINLINE s_ray build_ray(vec3 start, vec3 end){
    s_ray ray;
    {
      ray.start         = vec3(start.x, start.y, start.z);
      ray.delta         = vec3(end.x - start.x, end.y - start.y, end.z - start.z);
      ray.start_offset  = vec3();
      ray.extents       = vec3();
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

  CFLAG_O0 s_trace internal_ray(c_base_trace_filter& filter, vec3 start, vec3 end, uptr mask){
    s_trace trace;
    s_ray   ray    = build_ray(start, end);

    utils::internal_virtual_call<void, s_ray*, uptr, c_base_trace_filter*, s_trace*>(base_interface->engine_ray_trace_index, this, &ray, mask, &filter, &trace);

    return trace;
  }

  CFLAG_O0 s_trace internal_ray_obb(c_base_trace_filter& filter, vec3 start, vec3 end, vec3 mins, vec3 maxs, uptr mask){
    s_trace trace;
    s_ray   ray    = build_ray_mins_maxs(start, end, mins, maxs);

    utils::internal_virtual_call<void, s_ray*, uptr, c_base_trace_filter*, s_trace*>(base_interface->engine_ray_trace_index, this, &ray, mask, &filter, &trace);

    return trace;
  }

  CFLAG_O0 s_trace ray_to_entity(vec3 start, vec3 end, uptr mask, c_internal_base_entity* entity){
    s_trace trace;
    s_ray   ray = build_ray(start, end);

    utils::internal_virtual_call<void, s_ray&, uptr, c_internal_base_entity*, s_trace*>(base_interface->engine_ray_to_entity_index, this, ray, mask, entity, &trace);

    return trace;
  }
};