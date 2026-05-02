#include "../link.h"

c_lag_compensation* lag_compensation = nullptr;


float s_lc_record::time_delta(){
  float in  = global_internal->engine->get_latency(false, 0.f);
  float out = global_internal->engine->get_latency(true, 0.f);

  float target_fake_latency = fake_latency->get_target_latency();
  float correct             = out + in + math::ticks_to_time(math::time_to_ticks(interp_manager->get_lerp()));
        correct             = math::clamp(correct, 0.f, 1.f);

  //DBG("target_fake_latency: %f, correct: %f, in: %f, out: %f, total: %f\n", target_fake_latency, correct, in, out, (in + out) - target_fake_latency);
  i32 server_tick = (global_internal->client_state->server_tick() + 1) + math::biggest(math::time_to_ticks((in + out) - target_fake_latency), 0);

  return math::abs(correct - (math::ticks_to_time(server_tick) - simulation_time));
}

EXPORT bool __fastcall base_animating_setupbones_hook(void* rcx, matrix3x4* matrix, i32 max_bones, i32 bone_mask, float current_time){
  if(global->unloading)
    return utils::call_fastcall64<bool, matrix3x4*, i32, i32, float>(gen_internal->decrypt_asset(global->base_animating_setupbones_hook_trp), rcx, matrix, max_bones, bone_mask, current_time);

  return utils::call_fastcall64<bool, matrix3x4*, i32, i32, float>(gen_internal->decrypt_asset(global->base_animating_setupbones_hook_trp), rcx, matrix, max_bones, bone_mask, current_time);
}

EXPORT bool __fastcall cbaseentity_teleported_hook(void* rcx){
  void* ret_addr = __builtin_return_address(0);
  if(ret_addr == global->setup_bones_base_entity_teleport_retaddr)
    return true;
  
  return utils::call_fastcall64<bool>(gen_internal->decrypt_asset(global->cbaseentity_teleported_hook_trp), rcx);
}

// When the player goes dormant, clear lag compensation record data
EXPORT bool __fastcall set_dormant_hook(c_base_player* rcx, bool dormant){

  if(rcx != nullptr){
    dormant_manager->on_dormant(rcx, dormant);
    if(rcx->is_player()){

      if(dormant)
        utils::reset_lag_comp_data(rcx->get_index());
      else
        utils::check_to_reset_data(rcx->get_index());
    }
  }

  return utils::call_fastcall64<bool, bool>(gen_internal->decrypt_asset(global->set_dormant_hook_trp), rcx, dormant);
}

void c_lag_compensation::call_entity_prediction_required(c_internal_base_player* player){
  entity_prediction->compute_pred_yaw_change(player);
  entity_prediction->delete_cache(player);
}