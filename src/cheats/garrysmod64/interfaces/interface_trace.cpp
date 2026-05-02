#include "../link.h"

bool c_trace_filter::should_hit(c_base_entity* ent, u64 mask){
  if (ent == nullptr || ent == ignore_entity || ent != nullptr && custom_trace_filter == TR_CUSTOM_FILTER_IGNORE_ALL_ENTS)
    return false;

  s_client_class* cc = ent->get_client_class( );
  if(cc == nullptr)
    return false;

  if(ent->get_index() == global->engine->get_local_player())
    return false;

  c_entity_data* data = ent->data();
  if(data != nullptr){
    if(data->list_data.filter_from_aimbot_vis_check)
      return false;
  }

  // We don't care about these entities.
  switch(cc->id){
    default: break;
    case CBaseAnimating:
    case CFuncAreaPortalWindow:
    case CPhysicsPropMultiplayer:
      return false;
    case CGMOD_Player:
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
        return localplayer->team != ent->team || custom_trace_filter == TR_CUSTOM_FILTER_HIT_TEAM;

      break;
    }
  }

  return true;
}

CFLAG_O0 s_trace c_interface_trace::ray(vec3 start, vec3 end, u64 mask, u32 trace_type = TRACE_EVERYTHING, c_base_entity* ignore_entity = nullptr, u32 custom_trace_filter = 0){
  s_trace        trace;
  s_ray          ray = build_ray(start, end);
  c_trace_filter filter;
  {
    filter.type                 = trace_type;
    filter.custom_trace_filter  = custom_trace_filter;
    filter.ignore_entity        = ignore_entity;
  }

  utils::virtual_call64<4, void, s_ray*, u64, c_trace_filter*, s_trace*>(this, &ray, mask, &filter, &trace);

#if defined(TRACE_DEBUG)
  render_debug->draw_line(trace.start, trace.end, trace.fraction == 1.f ? rgb(0,255,0) : rgb(255,0,0), false, globaldata->interval_per_tick);
#endif

  return trace;
}

CFLAG_O0 s_trace c_interface_trace::ray_obb(vec3 start, vec3 end, vec3 mins, vec3 maxs, u64 mask, u32 trace_type = TRACE_EVERYTHING, c_base_entity* ignore_entity = nullptr, u32 custom_trace_filter = 0){
  s_trace        trace;
  s_ray          ray = build_ray_mins_maxs(start, end, mins, maxs);
  c_trace_filter filter;
  {
    filter.type                 = trace_type;
    filter.custom_trace_filter  = custom_trace_filter;
    filter.ignore_entity        = ignore_entity;
  }

  utils::virtual_call64<4, void, s_ray*, u64, c_trace_filter*, s_trace*>(this, &ray, mask, &filter, &trace);

#if defined(TRACE_DEBUG)
  render_debug->draw_line(trace.start, trace.end, trace.fraction == 1.f ? rgb(0,255,0) : rgb(255,0,0), false, globaldata->interval_per_tick);
#endif

  return trace;
}

CFLAG_O0 s_trace c_interface_trace::ray_to_entity(vec3 start, vec3 end, u64 mask, c_base_entity* entity){
  s_trace trace;
  s_ray   ray = build_ray(start, end);

  utils::virtual_call64<2, void, s_ray*, u64, c_base_entity*, s_trace*>(this, &ray, mask, entity, &trace);

#if defined(TRACE_DEBUG)
  render_debug->draw_line(trace.start, trace.end, trace.hit_entity == entity ? rgb(0,255,0) : rgb(255,0,0), false, globaldata->interval_per_tick);
#endif

  return trace;
}

s_trace c_interface_trace::utils_traceray(vec3 start, vec3 end, vec3 mins, vec3 maxs, u32 mask, c_base_entity* ignore_entity, i32 collision_group){
  assert(ignore_entity != nullptr);

  //s_trace trace;
 // s_ray   ray = build_ray_mins_maxs(start, end, mins, maxs);

  //utils::call_cdecl<void, s_ray&, u32, void*, i32, s_trace*, void*>(global->util_traceray, ray, mask, ignore_entity, collision_group, &trace, player_move_should_hit);

  // temp.
  s_trace trace = global->trace->ray_obb(start, end, mins, maxs, mask, TRACE_EVERYTHING, ignore_entity, 0);
  

  return trace;
}

s_trace c_interface_trace::player_move(vec3 start, vec3 end, c_base_player* player, bool brush_only = false){
  assert(player != nullptr);

  // calculate the correct trace mask
  u32 correct_mask = brush_only ? mask_playersolid_brushonly : mask_playersolid;

  return utils_traceray(start, end, player->obb_mins(), player->obb_maxs(), correct_mask, player, collision_group_player_movement);
}