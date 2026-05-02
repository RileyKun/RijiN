#include "../link.h"

c_interpolation_manager* interp_manager = nullptr;

EXPORT i32 __fastcall cbaseentity_baseinterpolatepart1_hook(c_base_entity* ecx, void* edx, vec3* old_origin, vec3* old_angles, vec3* old_vel, i32* no_more_changes){
  assert(ecx != nullptr);
  if(interp_manager->wants_no_interp(ecx))
    return 0;

  return utils::call_fastcall<i32, vec3*, vec3*, vec3*, i32*>(gen_internal->decrypt_asset(global->cbaseentity_baseinterpolatepart1_hook_trp), ecx, edx, old_origin, old_angles, old_vel, no_more_changes);
}

// Makes animations not use interpolated velocity which itself is just inaccurate.
EXPORT void __fastcall cbaseentity_estimateabsvelocity_hook(c_base_entity* entity, void* edx, vec3* out_vel){
  if(entity == nullptr){
    *out_vel = vec3();
    return;
  }

  s_client_class* cc = entity->get_client_class();
  if(cc == nullptr){
    *out_vel = vec3();
    return;
  }

  // Yea so, this function just takes the m_vecOrigin and calculates a velocity from previous versions using interpolation.
  // Which is inaccurate, disabling it and there is no velocity calculated for these entities. Likely for performance.
  if(cc->id == Infected || cc->id == Witch){
    c_entity_data* data = entity->data();
    if(data != nullptr)
      *out_vel = data->vel; // Calculated in the same area as the transforms.

    return;
  }

  // You can find this in C_BaseEntity::CalcAbsoluteVelocity the offset I mean, its absvelocity
  utils::call_thiscall<void>(global->baseentity_calculate_abs_velocity_addr, entity); // Can be old, so we need to update it.

  // For it to use the GetAbsVelocity since this is also called on non players.
  // This does not fix the tr_walkway issue, that's not the purpose of this.
  *out_vel = *(vec3*)((uptr)entity + (uptr)0x88);
}


bool c_interpolation_manager::wants_no_interp(c_internal_base_entity* internal_entity){
  c_base_entity* entity = (c_base_entity*)internal_entity;

  if(entity == nullptr)
    return false;

  // it just looks awful.
  if(entity->get_index() == global->localplayer_index)
    return false;

  if(config->misc.force_no_interpolation)
    return true;

  //if(entity->get_index() == global->localplayer_index){
  //  if(double_tap->is_charging || double_tap->is_shifting)
  //    return true;
//
  //  return false;
  //}

  // Do any global override here.
  if(must_have_interp())
    return false;

  c_base_player* player = entity->get_player();
  if(player != nullptr){
    c_entity_data* data = player->data();
    if(data == nullptr)
      return true;

    c_entity_info* ent_info = cheat_detection->get_entity_info(player->get_index());
    if(ent_info != nullptr){
      if(ent_info->info->is_abusing_tickbase)
        return true;
    }

    // Lagging players, disable interpolation.
    i32 lag_ticks = math::time_to_ticks(math::abs(player->old_simulation_time() - player->simulation_time()));
    if(lag_ticks >= 6) // Assume 6 ticks of lag is enough. Which is nearly 100ms worth of ticks on a 66 tickrate server.
      return true;

    if(cheat_detection->is_cheating(player->get_index())/*|| resolver->is_resolver_active(player)*/)
      return true;

    // Lets make friendly players always use interpolation.
    if(player->team() == global->localplayer_team)
      return false;

    return false;
  }
  else
    return false;

  // Anything unhandled should have no interpolation.
  return true;
}