#include "../link.h"

bool c_trace_filter::should_hit(c_internal_base_entity* ent, uptr mask){
  if (ent == nullptr || ent == ignore_entity)
    return false;

  s_client_class* cc = ent->get_client_class( );
  if(cc == nullptr)
    return false;

  if(ent->get_index() == global->engine->get_local_player())
    return false;

  // We don't care about these entities.
  switch(cc->id){
    default: break;
    case CBaseAnimating:
    case CFuncAreaPortalWindow:
    case CFuncPlayerGhostInfectedClip:
    case CFuncPlayerInfectedClip:
    case CEnvPhysicsBlocker:
      return false;
    case CBreakable:
    case CBreakableSurface:
    case CBasePropDoor:
    case CPropDoorRotatingCheckpoint:
      return custom_trace_filter == TR_CUSTOM_FILTER_AUTO_BASH;
    case CTerrorPlayer:
    case CCSPlayer:
    case SurvivorBot:
    case Jockey:
    case Spitter:
    case Hunter:
    case Smoker:
    case Boomer:
    case Tank:
    case Witch:
    case Infected:
    {
      if((cc->id == CTerrorPlayer || cc->id == SurvivorBot) && config->aimbot.avoid_teammates)
        return true;

      //Always hit entities that aren't on our team.
      //Because maybe someday with players we're gonna make multi-point fix head positions on people looking down and backwards or going around arms blocking the head.
      //Otherwise if we filter this entity our traces will never hit a player and make this impossible. -Rud
      c_base_player* localplayer = utils::localplayer();
      if(localplayer != nullptr)
        return localplayer->team() != ent->team()|| custom_trace_filter == TR_CUSTOM_FILTER_HIT_TEAM;

      break;
    }
  }


  return true;
}
