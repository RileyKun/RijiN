#include "../link.h"

c_lag_compensation* lag_compensation = nullptr;

float s_lc_record::time_delta(){
  float in  = global_internal->engine->get_latency(false, 0.f);
  float out = global_internal->engine->get_latency(true, 0.f);

  float target_fake_latency = 0.f;//fake_latency->get_target_latency();
  //interp_manager->get_lerp()
  float correct             = out + in + math::ticks_to_time(math::time_to_ticks(interp_manager->get_lerp()));
        correct             = math::clamp(correct, 0.f, 1.f);

  //DBG("target_fake_latency: %f, correct: %f, in: %f, out: %f, total: %f\n", target_fake_latency, correct, in, out, (in + out) - target_fake_latency);
  i32 server_tick = (global_internal->client_state->server_tick() + 1) + math::biggest(math::time_to_ticks((in + out) - target_fake_latency), 0);

  return math::abs(correct - (math::ticks_to_time(server_tick) - simulation_time));
}

// Make the engine use our setupbones call if we have records for it
EXPORT bool __fastcall base_animating_setupbones_hook(void* rcx, matrix3x4* matrix, i32 max_bones, i32 bone_mask, float current_time){
  return utils::call_fastcall64<bool, matrix3x4*, i32, i32, float>(gen_internal->decrypt_asset(global->base_animating_setupbones_hook_trp), rcx, matrix, max_bones, bone_mask, current_time);
}

// When the player goes dormant, clear lag compensation record data
EXPORT void __fastcall set_dormant_hook(c_base_player* rcx, bool dormant){
  if(rcx != nullptr){
    if(rcx->is_player()){
      dormant_manager->on_dormant(rcx, dormant);

      if(dormant)
        utils::reset_lag_comp_data(rcx->get_index());
      else
        utils::check_to_reset_data(rcx->get_index());
    }
  }

  utils::call_fastcall64<void, bool>(gen_internal->decrypt_asset(global->set_dormant_hook_trp), rcx, dormant);
}