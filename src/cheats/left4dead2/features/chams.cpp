#include "../link.h"

CLASS_ALLOC(c_chams, chams);

EXPORT bool __fastcall do_post_screen_space_effects_hook(void* ecx, void* edx, s_view_setup* setup){
  c_render_context* rc = global->material_system->get_render_context();
  if(rc == nullptr)
    return true;

  // Force game to use DrawModelExecute
  for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* entity = global->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    if(entity->is_dormant())
      continue;

    if(chams->get_chams_type(entity) == CHAMS_TYPE_INVALID)
      continue;

    entity->can_use_fast_path() = false;
  }

  rc->clear_buffers(false, false, true);

  chams->running = true;

  // 1. backup blend, colour
  float  old_blend = global->render_view->get_blend();
  colour old_clr   = global->render_view->get_colour_modulation();

  {
    chams->create_all_required_materials();
    chams->stencil_state = s_stencil_state();
    chams->run_glow_start(rc, chams->stencil_state);
    chams->run_chams(rc);
    chams->run_glow_finish(rc, chams->stencil_state);
  }

  chams->force_material_override(nullptr);
  global->render_view->set_blend(old_blend);
  global->render_view->set_colour_modulation(old_clr);

  chams->running = false;
  return utils::call_fastcall<bool, s_view_setup*>(gen_internal->decrypt_asset(global->do_post_screen_space_effects_hook_trp), ecx, edx, setup);
}

EXPORT bool __fastcall draw_model_execute_hook(void* ecx, void* edx, void* state, s_model_render_data* info, matrix3x4* custom_bone_to_world){
  if(info != nullptr && info->entity_index > 0){
    c_base_player* entity = global->entity_list->get_entity(info->entity_index);

    if(entity != nullptr){
      chams_type  type                    = chams->get_chams_type(entity);
      bool        has_rendered_this_frame = chams->has_rendered_this_frame(entity);

      if(entity->is_npc_or_human() && (type == CHAMS_TYPE_INVALID || !has_rendered_this_frame)){
        //chams->handle_backtrack_chams(entity, ecx, edx, state, info);

        // IMPORATANT!
        // This function must be called exactly before the original call.
        //  chams->handle_fake_angle_chams(entity, ecx, edx, state, info, custom_bone_to_world);
        utils::call_fastcall<bool, void*, s_model_render_data*, matrix3x4*>(gen_internal->decrypt_asset(global->draw_model_execute_hook_trp), ecx, edx, state, info, custom_bone_to_world);

        return false;
      }

      if(has_rendered_this_frame){
        c_base_entity* parent = entity->get_owner();
        if(parent != nullptr)
          entity = parent;

        if(entity != nullptr){
          // if the entity is marked as rendeered this frame but is invalid
          // we need to reset it and allow the game to start rendering it again
          if(chams->get_chams_type(entity) == CHAMS_TYPE_INVALID){
            chams->set_alpha(entity, 1.f, true);
            chams->set_rendered_this_frame(entity, false, true);
          }
        }

        return false;
      }
    }
  }

  return utils::call_fastcall<bool, void*, s_model_render_data*, matrix3x4*>(gen_internal->decrypt_asset(global->draw_model_execute_hook_trp), ecx, edx, state, info, custom_bone_to_world);
}

EXPORT void __fastcall studio_render_forced_material_override_hook(void* ecx, void* edx, c_material* new_material, u32 override_type){
  assert(ecx != nullptr);
  if(chams->disallow_materials)
    return;

  utils::call_fastcall<void, c_material*, u32>(gen_internal->decrypt_asset(global->studio_render_forced_material_override_hook_trp), ecx, edx, new_material, override_type);
}

EXPORT void __fastcall apply_entity_glow_effects_hook(void* ecx, void* edx, s_view_setup* setup, void* render_context_ptr, float scale, i32 unk1, i32 unk2, i32 unk3, i32 unk4){

  for(i32 i = 1; i <= globaldata->max_clients; i++){
    c_base_entity* entity = global->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    // This should be safe.
    i32 type = entity->type();
    if(type & TYPE_REAL_PLAYER || type & TYPE_BOT)
      entity->survivor_glow_enabled() = false;
  }

  utils::call_fastcall<void, s_view_setup*, void*, float, i32, i32, i32, i32>(gen_internal->decrypt_asset(global->apply_entity_glow_effects_hook_trp), ecx, edx, setup, render_context_ptr, scale, unk1, unk2, unk3, unk4);
}

EXPORT void __fastcall render_context_set_stencil_state_hook(void* ecx, void* edx, s_stencil_state* state){
  assert(ecx != nullptr);
  assert(state != nullptr);

  // Left 4 Dead 2 uses stencils alot. It'll constantly override our current stencil state. So during chams we'll force our state no matter what.
  if(chams->running && !global->is_setting_stencils)
    memcpy(state, &chams->stencil_state, sizeof(s_stencil_state));

  //DBG("[!] Set stencil_state call: %X\n", __builtin_return_address(0));
  utils::call_fastcall<void, s_stencil_state*>(gen_internal->decrypt_asset(global->render_context_set_stencil_state_hook_trp), ecx, edx, state);
}

// Fix glow and chams being owned by exposure.
//EXPORT void __cdecl get_exposure_range_hook(float* min, float* max){
//  utils::call_cdecl<void, float*, float*>(gen_internal->decrypt_asset(global->get_exposure_range_hook_trp), min, max);
//  *min = 1.f;
//  *max = 1.f;
//}
//
//// Fix glow and chams being smacked by bloom.
//EXPORT float __stdcall get_bloom_amount_hook(void){
//  return 0.f;
//}