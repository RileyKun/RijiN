#include "../../../link.h"

void c_base_interpolation_manager::update(){
  float lerp = get_lerp();
  if(collected && !should_update_interp())
    return;

  if(interp_time == lerp)
    return;

  interp_time  = lerp;
  interp_ticks = math::time_to_ticks(interp_time);

  // Can do a notification here if wanted.

  collected = true;
}

bool c_base_interpolation_manager::is_interp_command(u32 hash){
  switch(hash){
    default: return false;
    case HASH("cl_interp"):
    case HASH("cl_interp_all"):
    case HASH("cl_updraterate"):
    case HASH("cl_interp_ratio"):
    case HASH("cl_interpolate"):
    case HASH("cl_extrapolate"):
    case HASH("cl_extrapolate_amount"):
      return true;
  }

  return false;
}

float c_base_interpolation_manager::get_lerp(){
  static c_cvar* cl_interp_ratio            = global_internal->cvar->find_var(XOR("cl_interp_ratio"));
  static c_cvar* sv_client_min_interp_ratio = global_internal->cvar->find_var(XOR("sv_client_min_interp_ratio"));
  static c_cvar* sv_client_max_interp_ratio = global_internal->cvar->find_var(XOR("sv_client_max_interp_ratio"));
  static c_cvar* cl_updaterate              = global_internal->cvar->find_var(XOR("cl_updaterate"));
  static c_cvar* cl_interp                  = global_internal->cvar->find_var(XOR("cl_interp"));

  if(cl_interp_ratio == nullptr || sv_client_max_interp_ratio == nullptr || sv_client_min_interp_ratio == nullptr || cl_updaterate == nullptr || cl_interp == nullptr){
    DBG("[-] c_base_interpolation_manager::get_lerp unable to init cvars\n");
    return 0.f;
  }

  #if !defined(GMOD_CHEAT)
    float min_ratio = math::biggest(1.f, sv_client_min_interp_ratio->flt);
    float ratio     = math::clamp(cl_interp_ratio->flt, min_ratio, sv_client_max_interp_ratio->flt);
  
    float lerp = math::smallest(ratio / cl_updaterate->flt, 1.0f);
    float interp = math::smallest(cl_interp->flt, 1.0f);
  #else
    float min_ratio = math::biggest(1.f, sv_client_min_interp_ratio->flt());
    float ratio     = math::clamp(cl_interp_ratio->flt(), min_ratio, sv_client_max_interp_ratio->flt());
  
    float lerp = math::smallest(ratio / cl_updaterate->flt(), 1.0f);
    float interp = math::smallest(cl_interp->flt(), 1.0f);
  #endif

  return interp > lerp ? interp : lerp;
}

bool c_base_interpolation_manager::must_have_interp(){
  if(global_internal->base_acm != nullptr){
    if(!global_internal->base_acm->can_back_track())
      return true;
  }

  return false;
}