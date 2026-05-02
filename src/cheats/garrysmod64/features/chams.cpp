#include "../link.h"

CLASS_ALLOC(c_chams, chams);

EXPORT bool __fastcall do_post_screen_space_effects_hook(void* rcx, s_view_setup* setup){
  if(__builtin_return_address(0) != global->vgui_drawhud_retaddr || anti_screengrab->is_creating_spoofed_view() || utils::is_screen_shot_taken() || acp->is_blocked(FUNC_SCREEN_GRAB))
    return utils::call_fastcall64<bool, s_view_setup*>(gen_internal->decrypt_asset(global->do_post_screen_space_effects_hook_trp), rcx, setup);

  c_render_context* rc = global->material_system->get_render_context();
  if(rc == nullptr)
    return utils::call_fastcall64<bool, s_view_setup*>(gen_internal->decrypt_asset(global->do_post_screen_space_effects_hook_trp), rcx, setup);

  rc->clear_buffers(false, false, true);

  // 1. backup blend, colour
  float  old_blend = global->render_view->get_blend();
  colour old_clr   = global->render_view->get_colour_modulation();
  {
    chams->create_all_required_materials();

    s_stencil_state stencil_state;

   chams->is_rendering = true;
   chams->run_glow_start(rc, stencil_state);
   chams->run_chams(rc);
   chams->run_glow_finish(rc, stencil_state);
   chams->is_rendering = false;
  }

  global->engine_model->force_material_override(nullptr);
  global->render_view->set_blend(old_blend);
  global->render_view->set_colour_modulation(old_clr);

  return utils::call_fastcall64<bool, s_view_setup*>(gen_internal->decrypt_asset(global->do_post_screen_space_effects_hook_trp), rcx, setup);
}

EXPORT void __fastcall draw_model_execute_hook(void* rcx, void* state, s_model_render_data* info, matrix3x4* custom_bone_to_world){
  if(anti_screengrab->is_creating_spoofed_view() || acp->is_blocked(FUNC_SCREEN_GRAB) || utils::is_screen_shot_taken() ||  info == nullptr || info->entity_index <= 0){
    utils::call_fastcall64<void, void*, s_model_render_data*, matrix3x4*>(gen_internal->decrypt_asset(global->draw_model_execute_hook_trp), rcx, state, info, custom_bone_to_world);
    return;
  }

  c_base_entity* entity = global->entity_list->get_entity(info->entity_index);
  if(entity == nullptr){
    utils::call_fastcall64<void, void*, s_model_render_data*, matrix3x4*>(gen_internal->decrypt_asset(global->draw_model_execute_hook_trp), rcx, state, info, custom_bone_to_world);
    return;
  }

  chams_type type                    = chams->get_chams_type(entity);
  bool       has_rendered_this_frame = chams->has_rendered_this_frame(entity);

  if(entity->is_player() && (type == CHAMS_TYPE_INVALID || !has_rendered_this_frame)){
    // Do backtrack / fake angle chams up here.
    utils::call_fastcall64<void, void*, s_model_render_data*, matrix3x4*>(gen_internal->decrypt_asset(global->draw_model_execute_hook_trp), rcx, state, info, custom_bone_to_world);
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

  utils::call_fastcall64<void, void*, s_model_render_data*, matrix3x4*>(gen_internal->decrypt_asset(global->draw_model_execute_hook_trp), rcx, state, info, custom_bone_to_world);
}

EXPORT void __fastcall studio_render_forced_material_override_hook(void* rcx, c_material* new_material, u32 override_type){
  assert(rcx != nullptr);
  if(chams->is_rendering && !global->override_material_call)
    return;
  
  c_material* gmod_override = *global->gmod_model_material_override;
  // If this crashes then clearly this is wrong.
  if(!chams->is_rendering && !override_type && gmod_override != nullptr){
    DBG("[!] studio_render_forced_material_override_hook: Special gmod call %p %p\n", global->gmod_model_material_override, gmod_override);
    new_material = gmod_override;
  }

  *(u32**)((uptr)rcx + (uptr)0x260)  = override_type;
  *(void**)((uptr)rcx + (uptr)0x258) = new_material;
}