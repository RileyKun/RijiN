#include "../link.h"

CLASS_ALLOC(c_bullet_tracers, bullet_tracers);

// This functions calls CTFPlayer::FireBullet hence why it's being hooked.
EXPORT void __fastcall fx_firebullets_hook(c_base_weapon* wpn, i32 player, vec3& origin, vec3& angles, i32 weaponid, i32 mode, i32 seed, float spread, float damage, bool crits){
  if(!global->prediction->first_time_predicted)
    return;
  
  if(seed_prediction->active() && seed_prediction->is_synced())
    seed = seed_prediction->get_seed() & 255;

  //cheat_detection->on_shot(player, seed, crits);

  utils::call_fastcall64<void, i32, vec3&, vec3&, i32, i32, i32, float, float, bool>(gen_internal->decrypt_asset(global->fx_firebullets_hook_trp), wpn, player, origin, angles, weaponid, mode, seed, spread, damage, crits);
}

// If bullet tracers are enabled, we will make sure the game shows all our visual tracers
// Also making sure the game only runs this on first time predicted because it wants to call them in prediction frames also
// Causing tons of fps issues and using a ton of effects that could cause crashes
EXPORT void __fastcall ctfplayer_firebullet_hook(void* rcx, c_base_weapon* wpn, s_fire_bullets_info& info, bool effects, i32 damage_type, i32 custom_damage_type){
  // Stop the game from calling this function while in a predicted state
  if(!global->prediction->first_time_predicted)
    return;
  
#if defined(DEV_MODE) && defined(DOUBLE_TAP_DEBUG)
  if(utils::is_listen_server()){
    s_trace trace = global->trace->ray(info.src, info.src + info.dir * info.distance, mask_bullet);
    bullet_tracers->add(trace.start, trace.end, rgb(255,0,0), 2.f);
  }
#endif

  if(rcx == global->localplayer && config->visual.bullet_tracers_enabled){
    info.tracer_freq    = 1;
    effects             = true;
  }
  else if(rcx != global->localplayer)
    return;
  
  utils::call_fastcall64<void, void*, s_fire_bullets_info&, bool, i32, i32>(gen_internal->decrypt_asset(global->ctfplayer_firebullet_hook_trp), rcx, wpn, info, effects, damage_type, custom_damage_type);
}

EXPORT void* __fastcall create_particle_hook(void* rcx, i8* particle_name, i32 attach_type, i32 attachment_point, vec3* vec_origin_offset){
  if(global->ctf_firebullet_retaddr1 == __builtin_return_address(0) && config->visual.bullet_tracers_enabled)
    return nullptr;

  return utils::call_fastcall64<void*, i8*, i32, i32, vec3*>(gen_internal->decrypt_asset(global->create_particle_hook_trp), rcx, particle_name, attach_type, attachment_point, vec_origin_offset);
}

// Allows weapons that dont trace to trace like sniper
EXPORT i8* __fastcall get_tracer_type_hook(void* rcx){
  if(config->visual.bullet_tracers_enabled)
    return "\x01\x00"; // return fake string that means nothing but ends on null terminator

  return utils::call_fastcall64<i8*>(gen_internal->decrypt_asset(global->get_tracer_type_hook_trp), rcx);
}

// Overrides the bullet tracer effects for everything apart from beams
EXPORT void __fastcall util_particle_tracer_hook(u8* trace_effect_name, vec3& start, vec3& end, u32 entity_index, i32 attachment, bool whiz){
  if(config->visual.bullet_tracers_enabled && entity_index == global->localplayer_index){

    // The start position is legos without a valid barrel position.
    // This will "fix" it by making the start position of the tracer at the start position.
    c_base_player* localplayer = utils::localplayer();
    if(localplayer != nullptr){
      if(localplayer->is_scoped() && !config->visual.no_zoom_fov && config->visual.no_zoom_enabled || global->r_drawviewmodel->val == 0)
        start = localplayer->obb_center();
    }

    // the beam effects are specific to the beam system, if we choose those then use that system
    if(config->visual.tracer_beam_effect){
#if defined(DEV_MODE) && defined(DOUBLE_TAP_DEBUG)
        if(!utils::is_listen_server())
#endif

     float life_time = (float)math::clamp(config->visual.tracer_beam_effect_life_time, 15, 2500) / 1000.f;
     bullet_tracers->add(start, end, flt_array2clr(config->visual.bullet_beam_tracer_colour), life_time);
     return;
    }
    else if(config->visual.tracer_lines){
      float life_time = (float)math::clamp(config->visual.tracer_line_life_time, 15, 2500) / 1000.f;
      render_debug->draw_line(start, end, flt_array2clr(config->visual.tracer_line_colour), config->visual.tracer_line_outline, life_time);
      return;
    }
    else{
      i8* new_effect = bullet_tracers->get_particle_effect(global->localplayer_team);

      if(new_effect != nullptr)
        trace_effect_name = new_effect;
    }
  }

  utils::call_fastcall64<void, vec3&, vec3&, u32, i32, bool>(gen_internal->decrypt_asset(global->util_tracer_hook_trp), trace_effect_name, start, end, entity_index, attachment, whiz);
}

EXPORT void __fastcall sentry_muzzle_flash_hook(s_effect_data* data){
  assert(data != nullptr);
  utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->sentry_muzzle_flash_hook_trp), data);
  if(!config->visual.bullet_tracers_enabled || !config->visual.bullet_tracers_include_sentry || global->sentry_end_origin == vec3(0.f, 0.f, 0.f))
    return;

  if(data->entity_handle > 0){
    c_base_entity* entity = global->entity_list->get_entity_handle(data->entity_handle);
    if(entity != nullptr){
      // Make sure the object is a sentry gun.
      if(entity->is_sentry()){
        c_base_entity* owner = entity->get_building_owner();
        if(owner != nullptr){

          // Sentry has to belong to us before we begin.
          if(owner->get_index() == global->localplayer_index){
            vec3 start = data->origin;
            vec3 end   = global->sentry_end_origin;

            // the beam effects are specific to the beam system, if we choose those then use that system
            if(config->visual.tracer_beam_effect){
             float life_time = (float)math::clamp(config->visual.tracer_beam_effect_life_time, 15, 2500) / 1000.f;
             bullet_tracers->add(start, end, flt_array2clr(config->visual.bullet_beam_tracer_colour), life_time);
            }
            else if(config->visual.tracer_lines){
              float life_time = (float)math::clamp(config->visual.tracer_line_life_time, 15, 2500) / 1000.f;
              render_debug->draw_line(start, end, flt_array2clr(config->visual.tracer_line_colour), config->visual.tracer_line_outline, life_time);
            }
            else{
              i8* new_effect = bullet_tracers->get_particle_effect(global->localplayer_team);
              utils::call_fastcall64<void, vec3&, vec3&, u32, i32, bool>(gen_internal->decrypt_asset(global->util_tracer_hook_trp), new_effect, start, end, entity->get_index(), -1, false);
            }
          }
        }
      }
    }
  }
}

EXPORT void __fastcall tracer_callback_hook(s_effect_data* data){
  assert(data != nullptr);
  if(!config->visual.bullet_tracers_enabled || !config->visual.bullet_tracers_include_sentry){
    utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->tracer_callback_hook_trp), data);
    return;
  }

  if(data->entity_handle > 0){
    c_base_entity* entity = global->entity_list->get_entity_handle(data->entity_handle);
    if(entity != nullptr){
      // Make sure the object is a sentry gun.
      if(entity->is_sentry()){
        c_base_entity* owner = entity->get_building_owner();
        if(owner != nullptr){

          // Sentry has to belong to us before we begin.
          if(owner->get_index() == global->localplayer_index){
            global->sentry_end_origin = data->origin;
          }
        }
      }
    }
  }
}

