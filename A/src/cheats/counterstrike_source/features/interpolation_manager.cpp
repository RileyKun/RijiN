#include "../link.h";

c_interpolation_manager* interp_manager = nullptr;

// I discovered that interpolation is heavily performed on this function and is used to calculate how the poses are calculated on players.
// So if it uses any form of interpolation it'll be inaccurate. (Cause of GetDerivative_SmoothVelocity)
EXPORT void __fastcall cbaseentity_estimateabsvelocity_hook(c_base_entity* entity, vec3* out_vel){

  // You can find this in C_BaseEntity::CalcAbsoluteVelocity the offset I mean, its absvelocity
  utils::call_fastcall64<void>(global->baseentity_calculate_abs_velocity_addr, entity); // Can be old, so we need to update it.

  // For it to use the GetAbsVelocity since this is also called on non players.
  // This does not fix the tr_walkway issue, that's not the purpose of this.
  *out_vel = *(vec3*)((uptr)entity + (uptr)0x1A8);
}

EXPORT i32 __fastcall cbaseentity_baseinterpolatepart1_hook(c_base_entity* rcx, float* current_time, vec3* old_origin, vec3* old_angles, vec3* old_vel, i32* no_more_changes){
  assert(rcx != nullptr);
  if(interp_manager->wants_no_interp(rcx))
    return 0;

  return utils::call_fastcall64<i32, float*, vec3*, vec3*, vec3*, i32*>(gen_internal->decrypt_asset(global->cbaseentity_baseinterpolatepart1_hook_trp), rcx, current_time, old_origin, old_angles, old_vel, no_more_changes);
}

bool c_interpolation_manager::wants_no_interp(c_internal_base_entity* internal_entity){
  c_base_entity* entity = (c_base_entity*)internal_entity;

  if(entity == nullptr)
    return false;

  if(config->misc.force_no_interpolation)
    return true;

  if(entity->get_index() == global->localplayer_index){
    //if(double_tap->is_charging || double_tap->is_shifting)
    //  return true;

    return false;
  }

  // Do any global override here.
  if(must_have_interp())
    return false;

  c_base_player* player = entity->is_player() ? (c_base_player*)entity : nullptr;
  if(player != nullptr){
    // Lets make friendly players always use interpolation.
    if(player->team() == global->localplayer_team)
      return false;

    c_player_data* data = player->data();
    if(data == nullptr)
      return true;

    // Lagging players, disable interpolation.
    i32 lag_ticks = math::time_to_ticks(math::abs(player->old_simulation_time() - player->simulation_time()));
    if(lag_ticks >= 6) // Assume 6 ticks of lag is enough. Which is nearly 100ms worth of ticks on a 66 tickrate server.
      return true;

    //if(cheat_detection->is_cheating(player->get_index()) || resolver->is_resolver_active(player))
    //  return true;

    return false;
  }
  else{
    return false;
  }

  // Anything unhandled should have no interpolation.
  return true;
}