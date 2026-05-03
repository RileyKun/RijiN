#include "../link.h"

// Make the engine use our setupbones call if we have records for it
EXPORT bool __fastcall base_animating_setupbones_hook(void* ecx, void* edx, matrix3x4* matrix, i32 max_bones, i32 bone_mask, float current_time){
  if(ecx != nullptr){
    c_base_player* player = (u32)ecx - 0x4;

    // TODO: Feature to not draw teammates except friends (optional)
    //c_base_player* localplayer = utils::localplayer();
//
    //if(localplayer != nullptr && player != localplayer && player->team == localplayer->team)
    //  return false;

    //if(lag_compensation->base_animating_setupbones_run(player, matrix, max_bones, bone_mask, current_time))
    //  return true;
  }

  return utils::call_fastcall<bool, matrix3x4*, i32, i32, float>(gen_internal->decrypt_asset(global->base_animating_setupbones_hook_trp), ecx, edx, matrix, max_bones, bone_mask, current_time);
}

// When the player goes dormant, clear lag compensation record data
EXPORT void __fastcall set_dormant_hook(c_base_player* ecx, void* edx, bool dormant){

  if(ecx != nullptr){
    if(ecx->is_player()){
      //dormant_manager->on_dormant(ecx, dormant);

      if(dormant)
        utils::reset_lag_comp_data(ecx->get_index());
      else
        utils::check_to_reset_data(ecx->get_index());
    }
  }

  utils::call_fastcall<void, bool>(gen_internal->decrypt_asset(global->set_dormant_hook_trp), ecx, edx, dormant);
}

//void c_lag_compensation::call_entity_prediction_required(c_base_player* player){
//  entity_prediction->compute_pred_yaw_change(player);
//}

CLASS_ALLOC(c_lag_compensation, lag_compensation);