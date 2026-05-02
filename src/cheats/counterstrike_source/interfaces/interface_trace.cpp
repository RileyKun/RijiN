#include "../link.h"

bool c_trace_filter::should_hit(c_internal_base_entity* _ent, uptr mask){
  c_base_entity* ent = (c_base_entity*)_ent;
  if (ent == nullptr || ent == ignore_entity || ent != nullptr && custom_trace_filter == TR_CUSTOM_FILTER_IGNORE_ALL_ENTS)
    return false;

  s_client_class* cc = ent->get_client_class( );
  if(cc == nullptr)
    return false;

  if(ent->is_dropped_weapon() || ent->is_active_grenade() || ent->get_index() == global->engine->get_local_player())
    return false;

  // We don't care about these entities.
  switch(cc->id){
    default: break;
    case CBaseAnimating:
    case CFuncAreaPortalWindow:
    case CPhysicsPropMultiplayer:
      return false;
    case CCSPlayer:
    {
      if(custom_trace_filter == TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS)
        return false;
      else if(custom_trace_filter == TR_CUSTOM_FILTER_NONE)
          return true;

      //Always hit entities that aren't on our team.
      //Because maybe someday with players we're gonna make multi-point fix head positions on people looking down and backwards or going around arms blocking the head.
      //Otherwise if we filter this entity our traces will never hit a player and make this impossible. -Rud
      c_base_player* localplayer = utils::localplayer();
      if(localplayer != nullptr)
        return localplayer->team() != ent->team() || custom_trace_filter == TR_CUSTOM_FILTER_HIT_TEAM;

      break;
    }
  }

  return true;
}


CFLAG_O0 s_trace c_interface_trace::ray_clip_to_player(vec3 start, vec3 end, u32 mask, s_trace* tr, u32 trace_type = TRACE_EVERYTHING, c_base_entity* ignore_entity = nullptr, u32 custom_trace_filter = 0){
  s_ray ray = build_ray(start, end);
  c_trace_filter filter;
  {
    filter.type                 = trace_type;
    filter.custom_trace_filter  = custom_trace_filter;
    filter.ignore_entity        = ignore_entity;
  }

  utils::call_fastcall64_raw<void, vec3&, vec3&, u32, void*, void*>(global->util_cliptrace_to_players, start, end, mask, &filter, tr);
  return *tr;
}

CFLAG_O0 s_trace c_interface_trace::utils_traceray(vec3 start, vec3 end, vec3 mins, vec3 maxs, uptr mask, c_base_entity* ignore_entity, i32 collision_group){
  assert(ignore_entity != nullptr);

  s_trace trace;
  s_ray   ray = build_ray_mins_maxs(start, end, mins, maxs);

  // \x48\x8D\x05\x00\x00\x00\x00\x48\x89\x51\x00\x48\x89\x01\x48\x8B\xC1\x44\x89\x41
  // call  CTraceFilterSimple constructor and then just pass it to
  // enginetrace->TraceRay( ray, mask, &traceFilter, ptr );
  u8 filter[0x20];

  // call CTraceFilterSimple constructor
  utils::call_fastcall64<void*, c_base_entity*, i32, void*>(global->ctrace_filter_simple, &filter[0], ignore_entity, collision_group, 0);

  // call raytrace with filter
  utils::internal_virtual_call<void, s_ray*, uptr, u8*, s_trace*>(base_interface->engine_ray_trace_index, this, &ray, mask, &filter[0], &trace);

  return trace;
}