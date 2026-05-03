#include "../link.h"

// Make the engine use our setupbones call if we have records for it
EXPORT bool __fastcall base_animating_setupbones_hook(void* rcx, matrix3x4* matrix, i32 max_bones, i32 bone_mask, float current_time){
  if(rcx != nullptr && config->misc.setupbones_optimisations){
    c_base_player* player = (uptr)rcx - 0x8;

    // TODO: Feature to not draw teammates except friends (optional)
    c_base_player* localplayer = utils::localplayer();
    if(player != localplayer && lag_compensation->base_animating_setupbones_run(player, matrix, max_bones, bone_mask, current_time))
      return true;
  }

  return utils::call_fastcall64<bool, matrix3x4*, i32, i32, float>(gen_internal->decrypt_asset(global->base_animating_setupbones_hook_trp), rcx, matrix, max_bones, bone_mask, current_time);
}

// When the player goes dormant, clear lag compensation record data
EXPORT void __fastcall set_dormant_hook(c_base_player* rcx, bool dormant){

  if(rcx != nullptr){
    if(rcx->is_player()){
      //dormant_manager->on_dormant(ecx, dormant);

      if(dormant)
        utils::reset_lag_comp_data(rcx->get_index());
      else
        utils::check_to_reset_data(rcx->get_index());
    }
  }

  utils::call_fastcall64<void, bool>(gen_internal->decrypt_asset(global->set_dormant_hook_trp), rcx, dormant);
}

EXPORT void __fastcall maintain_sequence_transition_hook(void* rcx, void* bone_setup, void* cycle, void* pos, void* q){
  assert(rcx != nullptr);
  if(lag_compensation->is_setting_up_bones)
    return;

  utils::call_fastcall64<void, void*, void*, void*, void*>(gen_internal->decrypt_asset(global->maintain_sequence_transition_hook_trp), rcx, bone_setup, cycle, pos, q);
}

// Gets called in C_BaseAnimating::ClientSideAnimationChanged() want to ensure there is no sequence / layer interpolation at all. - Rud
EXPORT void __fastcall check_for_sequence_change_hook(void* rcx, void* hdr, i32 seq, bool force_new_seq, bool interpolate){
  if(lag_compensation->is_setting_up_bones)
    interpolate = false;

  utils::call_fastcall64<void, void*, i32, bool, bool>(gen_internal->decrypt_asset(global->check_for_sequence_change_hook_trp), rcx, hdr, seq, force_new_seq, interpolate);
}

//void c_lag_compensation::call_entity_prediction_required(c_base_player* player){
//  entity_prediction->compute_pred_yaw_change(player);
//}

CLASS_ALLOC(c_lag_compensation, lag_compensation);