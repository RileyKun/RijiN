#include "../link.h"

CLASS_ALLOC(c_chams, chams);

void c_chams::process_material_vars(){
  static float last_bloom_value_x = -1.f;
  static float last_bloom_value_y = -1.f;
  if(chams->material_glow_blur_y != nullptr && last_bloom_value_y != config->visual.glow_blur_bloom_value){
    c_material_var* var = chams->material_glow_blur_y->find_var(XOR("$bloomamount"));
    if(var != nullptr)
      var->set_float_value(math::clamp(config->visual.glow_blur_bloom_value, 0.5f, 5.f));

    last_bloom_value_y = config->visual.glow_blur_bloom_value;
  }

  if(chams->material_glow_blur_x != nullptr && last_bloom_value_x != config->visual.glow_blur_bloom_value){
    c_material_var* var = chams->material_glow_blur_x->find_var(XOR("$bloomamount"));
    if(var != nullptr)
      var->set_float_value(math::clamp(config->visual.glow_blur_bloom_value, 0.5f, 5.f));

    last_bloom_value_x = config->visual.glow_blur_bloom_value;
  }
}

void c_chams::update_screen_size(){
  static float next_screen_size_time = -1.f;

  float time = math::time();
  if(next_screen_size_time <= time || screen_size.x == 0){
    vec3i size = global->engine->get_screen_size();
    if(size.x != 0 && size.y != 0)
      screen_size = size;

    next_screen_size_time = time + 1.f;
  }
}

CFLAG_Ofast bool c_chams::is_valid_attachment_entity(c_base_entity* entity, c_base_entity* attachment){
  if(entity == nullptr || attachment == nullptr)
    return false;

  if(entity == attachment)
    return false;

  s_client_class* entity_cc     = entity->get_client_class();
  if(entity_cc == nullptr)
    return false;

  s_client_class* attachment_cc = attachment->get_client_class();
  if(attachment_cc == nullptr)
    return false;

  // Doesn't do anything special on ESP or glow.
  if(attachment_cc->id == CParticleSystem || attachment_cc->id == CSpriteTrail || attachment_cc->id == CEntityDissolve)
    return false;

  if(entity_cc->id == CTFTankBoss && attachment_cc->id == CDynamicProp)
    return true;

  if((entity->is_sentry() || entity->is_dispenser() || entity->is_teleporter()) && attachment_cc->id == CObjectSapper)
    return true;

  c_base_entity* owner = attachment->get_owner();
  if(owner == nullptr){
    //DBG("[!] Why is attachment %s being rendered for entity %s (no owner relation)\n", attachment->get_client_class()->name, entity->get_client_class()->name);
    return false;
  }

  if(owner == entity)
    return true;

  //DBG("[!] Why is attachment %s being rendered for entity %s (not owned by)\n", attachment->get_client_class()->name, entity->get_client_class()->name);
  return false;
}

CFLAG_Ofast void c_chams::set_alpha(c_base_entity* entity, float alpha, bool set_for_children){
  if(entity == nullptr)
    return;

  entity->render_mode()  = alpha < 1.f ? 4 : 0;
  entity->clr_render_a() = entity->render_mode() == 4 ? 254 : 255;
  global->render_view->set_blend(alpha);

  if(set_for_children){
    i32 count = 0;

    // By the looks of it sometimes next_move_peer will loop back around to first_move_child causing a loop lock
    // To solve thie issue when it happens rarely just make sure we dont render more than 32 attachments
    for(c_base_entity* attachment = entity->first_move_child(); attachment != nullptr; attachment = attachment->next_move_peer()){
      if(attachment == nullptr || count++ >= 32)
        break;

      if(!is_valid_attachment_entity(entity, attachment))
        continue;

      if(attachment->should_draw())
        set_alpha(attachment, alpha, false);
    }
  }
}

bool c_chams::override_cloak_effective(c_base_player* player){
  if(player == nullptr)
    return false;

  if(!running)
    return false;

  if(override_cloak)
    return true;

  if(running_chams){
    c_player_chams_settings* settings = utils::get_player_chams_settings(player);
    if(settings != nullptr){
      if(settings->render_cloaked_spies)
        return true;
    }
  }
  else if(is_rendering_glow){
    if(config->visual.player_glow_render_cloaked_spies)
      return true;
  }

  return false;
}

//--------------------------
// HOOKS BELOW
//--------------------------

// Do not bother calling the original.
EXPORT bool __fastcall do_post_screen_space_effects_hook(void* rcx, s_view_setup* setup){
  c_render_context* rc = global->material_system->get_render_context();
  if(rc == nullptr)
    return false;
  
  rc->clear_buffers(false, false, true);
  if(render_cam->is_rendering_view)
    return false;
  
  chams->running = true;
  chams->update_screen_size();

  // 1. backup blend, colour
  float  old_blend = global->render_view->get_blend();
  colour old_clr   = global->render_view->get_colour_modulation();
  {
    chams->create_all_required_materials();
    chams->process_material_vars();

    s_stencil_state stencil_state;
    chams->run_glow_start(rc, stencil_state);
    chams->run_chams(rc);
    chams->run_glow_finish(rc, stencil_state);
  }

  chams->force_material_override(nullptr);
  global->render_view->set_blend(old_blend);
  global->render_view->set_colour_modulation(old_clr);

  chams->running = false;

  return true;
}

EXPORT CFLAG_Ofast void __fastcall draw_model_execute_hook(void* rcx, void* state, s_model_render_data* info, matrix3x4* custom_bone_to_world){
  if(info != nullptr && info->entity_index > 0 && !render_cam->is_rendering_view){
    c_base_player* entity = global->entity_list->get_entity(info->entity_index);

    if(entity != nullptr){
      c_base_player* player = entity->get_player();
      chams_type     type                    = chams->get_chams_type(entity);
      bool           has_rendered_this_frame = chams->has_rendered_this_frame(entity);

      if(entity->is_sentry()){
        if(entity->get_building_owner() == global->localplayer){

          c_base_player* localplayer = utils::localplayer();
          if(localplayer != nullptr){
            c_base_weapon* wep = localplayer->get_weapon();
            if(wep != nullptr){
              if(wep->is_wrangler() && config->visual.hide_sentry_with_wrangler_out)
                return;
            }
          }
        }
      }
  
      //cheat_detection->detect_invalid_wearables(entity, info->model);

      // Check to see if we want to override the matrix at all.
      if(player != nullptr){
        c_player_data* data = player->data();
        if(data != nullptr){
          if(data->override_dme_matrix && data->dme_matrix != nullptr){
            custom_bone_to_world = data->dme_matrix;
            data->dme_matrix          = nullptr;
            data->override_dme_matrix = false;
          }  
        }
      }

      // Do hands here.
      if(chams->handle_hand_chams(entity, rcx, state, info, custom_bone_to_world))
        return;

      if(player != nullptr && (type == CHAMS_TYPE_INVALID || !has_rendered_this_frame)){ 
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
    }
  }

  if(config->visual.door_transparency_enabled){
    if(chams->is_door_model(info->model)){
      if(config->visual.door_transparency_value <= 0.0f)
        return;

      float old_blend = global->render_view->get_blend();
      global->render_view->set_blend(config->visual.door_transparency_value / 100.f);
      utils::call_fastcall64<void, void*, s_model_render_data*, matrix3x4*>(gen_internal->decrypt_asset(global->draw_model_execute_hook_trp), rcx, state, info, custom_bone_to_world);
      global->render_view->set_blend(old_blend);
      return;
    }
  }

  utils::call_fastcall64<void, void*, s_model_render_data*, matrix3x4*>(gen_internal->decrypt_asset(global->draw_model_execute_hook_trp), rcx, state, info, custom_bone_to_world);
}

EXPORT CFLAG_Ofast void __fastcall studio_render_context_drawmodel_hook(c_studio_render_context* studio_render_context, void* results, s_draw_model_info* info, matrix3x4* bone_to_world, float* flex_weights, float* flex_delayed_weights, vec3* origin, i32 flags){
  assert(studio_render_context != nullptr);
  assert(info != nullptr);

  // sanity checks, but most importantly there is a issue with drawmodel causing crashes
  // because the game renders a special sort of prop that seems to mostly be grass related and we need to make sure that we dont run our drawmodel hook when the return address is that function
  // IMPORTANT: results should never be a nullptr or its not rendering entities!
  if(results == nullptr || info == nullptr || info->client_entity == nullptr){
    utils::call_fastcall64<void, void*, s_draw_model_info*, matrix3x4*, float*, float*, vec3*, i32>(gen_internal->decrypt_asset(global->studio_render_context_drawmodel_hook_trp), studio_render_context, results, info, bone_to_world, flex_weights, flex_delayed_weights, origin, flags);
    return;
  }

  // Prevent decals from rendering over our chams.
  if(chams->running)
    info->decals = nullptr;

  c_client_unknown* client_unknown = info->client_entity->get_client_unknown();
  c_base_entity*    render_entity = client_unknown->get_base_entity();

  // Returning but not calling the original? -rud
  if(render_entity == nullptr)
    return;

  if(chams->handle_weapon_viewmodel_chams(render_entity, studio_render_context, results, info, bone_to_world, flex_weights, flex_delayed_weights, origin, flags))
    return;

  utils::call_fastcall64<void, void*, s_draw_model_info*, matrix3x4*, float*, float*, vec3*, i32>(gen_internal->decrypt_asset(global->studio_render_context_drawmodel_hook_trp), studio_render_context, results, info, bone_to_world, flex_weights, flex_delayed_weights, origin, flags);
}

// Fix BLACK chams
EXPORT void __fastcall cmaterial_uncache_hook(c_material* rcx, bool preservevars){
  assert(rcx != nullptr);

  if(chams != nullptr){
    if(chams->is_chams_material(rcx)){
      DBG("[!!] Prevented game from uncaching chams material.\n");
      return;
    }
  }

  utils::call_fastcall64<void, bool>(gen_internal->decrypt_asset(global->cmaterial_uncache_hook_trp), rcx, preservevars);
}

EXPORT void __fastcall studio_render_forced_material_override_hook(void* rcx, c_material* new_material, u32 override_type){
  assert(rcx != nullptr);
  if(chams->disallow_materials){
    //DBG("[!] Disallowing VStudioRender::force_material_override call at 0: %X, 1: %X\n", __builtin_return_address(0), __builtin_return_address(1));
    return;
  }

  utils::call_fastcall64<void, c_material*, u32>(gen_internal->decrypt_asset(global->studio_render_forced_material_override_hook_trp), rcx, new_material, override_type);
}

// Fix glow and chams being owned by exposure.
EXPORT void __cdecl get_exposure_range_hook(float* min, float* max){
  if(config->visual.enable_exposure){
    *min = config->visual.exposure_min;
    *max = config->visual.exposure_max;
  }
  else{
    *min = 1.f;
    *max = 1.f;
  }

  //utils::call_cdecl<void, float*, float*>(gen_internal->decrypt_asset(global->get_exposure_range_hook_trp), min, max);
}

// Fix glow and chams being smacked by bloom.
EXPORT float __stdcall get_bloom_amount_hook(void){
  if(!config->visual.enable_bloom)
    return 0.f;

  return utils::call_stdcall<float>(gen_internal->decrypt_asset(global->get_bloom_amount_hook_trp)) + config->visual.bloom_amount;
}