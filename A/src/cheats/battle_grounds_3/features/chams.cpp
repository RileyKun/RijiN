#include "../link.h"

CLASS_ALLOC(c_chams, chams);

EXPORT bool __fastcall do_post_screen_space_effects_hook(void* ecx, void* edx, s_view_setup* setup){
  assert(global->vgui_drawhud_retaddr != nullptr);
  if(__builtin_return_address(0) != global->vgui_drawhud_retaddr)
    return true;

  c_render_context* rc = global->material_system->get_render_context();
  if(rc == nullptr)
    return true;

  rc->clear_buffers(false, false, true);

  // 1. backup blend, colour
  float  old_blend = global->render_view->get_blend();
  colour old_clr   = global->render_view->get_colour_modulation();
  {
    chams->create_all_required_materials();

    s_stencil_state stencil_state;

    chams->run_glow_start(rc, stencil_state);
    chams->run_chams(rc);
    chams->run_glow_finish(rc, stencil_state);
  }
  global->engine_model->force_material_override(nullptr);
  global->render_view->set_blend(old_blend);
  global->render_view->set_colour_modulation(old_clr);

  return true;
}

EXPORT void __fastcall draw_model_execute_hook(void* ecx, void* edx, void* state, s_model_render_data* info, matrix3x4* custom_bone_to_world){
  if(info != nullptr && info->entity_index > 0){
    c_base_player* entity = global->entity_list->get_entity(info->entity_index);

    chams_type  type                    = chams->get_chams_type(entity);
    bool        has_rendered_this_frame = chams->has_rendered_this_frame(entity);


    if(entity->is_player() && (type == CHAMS_TYPE_INVALID || !has_rendered_this_frame)){
      chams->handle_backtrack_chams(entity, ecx, edx, state, info);

      // IMPORATANT!
      // This function must be called exactly before the original call.
      //  chams->handle_fake_angle_chams(entity, ecx, edx, state, info, custom_bone_to_world);
      utils::call_fastcall<void, void*, s_model_render_data*, matrix3x4*>(gen_internal->decrypt_asset(global->draw_model_execute_hook_trp), ecx, edx, state, info, custom_bone_to_world);

      return;
    }

    if(has_rendered_this_frame){
      c_base_entity* parent = entity->get_owner();
      if(parent != nullptr)
        entity = parent;

      // if the entity is marked as rendeered this frame but is invalid
      // we need to reset it and allow the game to start rendering it again
      if(chams->get_chams_type(entity) == CHAMS_TYPE_INVALID){
        chams->set_alpha(entity, 1.f, true);
        chams->set_rendered_this_frame(entity, false, true);
      }

      return;
    }
  }

  utils::call_fastcall<void, void*, s_model_render_data*, matrix3x4*>(gen_internal->decrypt_asset(global->draw_model_execute_hook_trp), ecx, edx, state, info, custom_bone_to_world);
}
// Fix glow and chams being owned by exposure.
EXPORT void __cdecl get_exposure_range_hook(float* min, float* max){
  *min = 1.f;
  *max = 1.f;
}

// Fix glow and chams being smacked by bloom.
EXPORT float __stdcall get_bloom_amount_hook(void){
  return 0.f;
}