#include "../link.h"

c_lag_compensation* lag_compensation = nullptr;

float s_lc_record::time_delta(){
  float in  = global->engine->get_latency(false, 0.f);
  float out = global->engine->get_latency(true, 0.f);

  float target_fake_latency = 0.f; // fake_latency->get_target_latency();
  float correct             = out + in + math::ticks_to_time(math::time_to_ticks(interp_manager->get_lerp()));
        correct             = math::clamp(correct, 0.f, 1.f);

  // Where does target_fake_latency go here? -Rud
  return math::abs(correct - (math::ticks_to_time(global->predicted_tick_base) - simulation_time));
}

// When the player goes dormant, clear lag compensation record data
EXPORT void __fastcall set_dormant_hook(c_base_player* ecx, void* edx, bool dormant){
  if(ecx != nullptr){
    if(dormant)
      utils::reset_lag_comp_data(ecx->get_index());
    else
      utils::verify_entity_data(ecx->get_index());
  }

  utils::call_fastcall<void, bool>(gen_internal->decrypt_asset(global->set_dormant_hook_trp), ecx, edx, dormant);
}

//void c_lag_compensation::call_entity_prediction_required(c_base_player* player){
//  entity_prediction->compute_pred_yaw_change(player);
//}