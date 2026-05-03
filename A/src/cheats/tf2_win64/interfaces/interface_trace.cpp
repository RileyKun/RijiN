#include "../link.h"

bool c_trace_filter::should_hit(c_internal_base_entity* _ent, uptr mask){
  c_base_entity* ent = (c_base_entity*)_ent;
  if(ent == nullptr || ent == ignore_entity || ent != nullptr && custom_trace_filter == TR_CUSTOM_FILTER_IGNORE_ALL_ENTS)
    return false;

  s_client_class* cc = ent->get_client_class();

  if(cc == nullptr)
    return false;

  if(ent->is_projectile() || ent->get_index() == global->engine->get_local_player())
    return false;

  if(custom_trace_filter == TR_CUSTOM_FILTER_HIT_WEAPONS && cc->id == CTFDroppedWeapon)
    return true;

  // We don't care about these entities.
  switch(cc->id){
    default: break;
    case CBaseAnimating:
    case CFuncAreaPortalWindow:
    case CFuncRespawnRoomVisualizer:
    case CFuncRespawnRoom:
    case CTFMedigunShield:
    case CTFAmmoPack:
    case CTFDroppedWeapon:
    case CTFRagdoll:
    case CTFReviveMarker:
    case CPasstimeBall:
    case CTFTauntProp:
    case CCaptureFlag:
    case CTFProjectile_BallOfFire:
    case CTFRobotDestruction_Robot:
    case CSniperDot:
    case CLaserDot:
      return false;
    case CTFPlayer:
    case CObjectSentrygun:
    case CObjectDispenser:
    case CObjectTeleporter:
    {
      if(custom_trace_filter == TR_CUSTOM_FILTER_WRANGLER_AIMBOT_TEST && cc->id == CTFPlayer)
        return false;

      if(custom_trace_filter == TR_CUSTOM_FILTER_SPLASH_BOT_TEST){
        if(cc->id == CObjectDispenser || cc->id == CObjectSentrygun && ent->team() == global->localplayer_team)
          return true;

        return false;
      }

      if(custom_trace_filter == TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS || custom_trace_filter == TR_CUSTOM_FILTER_ANTI_AIM)
        return false;

      if(custom_trace_filter == TR_CUSTOM_FILTER_WRENCH_SWING){
        if(cc->id == CTFPlayer)
          return false;

        return true;
      }
      else if(custom_trace_filter == TR_CUSTOM_FILTER_MELEE){
        if(cc->id == CTFPlayer)
          return true;
      }
      //Always hit entities that aren't on our team.
      //Because maybe someday with players we're gonna make multi-point fix head positions on people looking down and backwards or going around arms blocking the head.
      //Otherwise if we filter this entity our traces will never hit a player and make this impossible. -Rud

      return global->localplayer_team != ent->team() || custom_trace_filter == TR_CUSTOM_FILTER_HIT_TEAM || custom_trace_filter == TR_CUSTOM_FILTER_HIT_TEAM_NO_PLAYERS && cc->id != CTFPlayer;
    }
    case CBaseDoor:
    case CBasePropDoor:
    {
      if(custom_trace_filter == TR_CUSTOM_FILTER_ANTI_AIM)
        return false;

      break;
    }
  }


  return true;
}

CFLAG_Ofast s_trace c_interface_trace::utils_traceray(vec3 start, vec3 end, vec3 mins, vec3 maxs, u64 mask, c_base_entity* ignore_entity, i32 collision_group){
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
  utils::virtual_call64<4, void, s_ray*, u64, u8*, s_trace*>(this, &ray, mask, &filter[0], &trace);

#if defined(TRACE_DEBUG)
  //global->debugoverlay->add_box(trace.end, mins, maxs, vec3(), rgba(255, 0, 0, 1), globaldata->interval_per_tick * 2);
  bullet_tracers->add(trace.start, trace.end, trace.fraction == 1.f ? rgb(0,255,0) : rgb(255,0,0), globaldata->interval_per_tick);
#endif

  return trace;
}

CFLAG_Ofast s_trace c_interface_trace::player_move(vec3 start, vec3 end, c_base_player* player, bool brush_only = false){
  assert(player != nullptr);

  // calculate the correct trace mask
  u32 correct_mask = brush_only ? mask_playersolid_brushonly : mask_playersolid;
  {
    if(player->has_condition(TF_COND_HALLOWEEN_GHOST_MODE))
      correct_mask = mask_playersolid_brushonly;
    else if(player->team() == TEAM_BLU)
      correct_mask |= 0x800;
    else if(player->team() == TEAM_RED)
      correct_mask |= 0x1000;
  }

  return utils_traceray(start, end, player->obb_mins(), player->obb_maxs(), correct_mask, player, collision_group_player_movement);
}