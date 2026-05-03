#include "../../../link.h"

// Should be the last call in this hook.
void c_base_viewmodel_offset::calc_view_model(c_internal_base_entity* viewmodel, vec3& eye_pos, vec3& angles){
  if(viewmodel == nullptr)
    return;

  c_viewmodel_offset_settings* settings = get_settings();
  if(settings == nullptr)
    return;

  if(!settings->offset_enabled)
    return;

  // Modify angles first or last? Idk.
  const vec3 offset_dir    = vec3((float)settings->offset_x, (float)settings->offset_y, (float)settings->offset_z);
  const vec3 offset_angles = vec3((float)settings->pitch, (float)settings->yaw, (float)settings->roll);

  vec3 fwd, right, up;
  angles += offset_angles;
  math::angle_2_vector(angles, &fwd, &right, &up);

  eye_pos += (fwd * offset_dir.x + right * offset_dir.y + up * offset_dir.z);
}

void c_base_viewmodel_offset::post_calc_view_model(c_internal_base_entity* viewmodel, c_internal_base_player* owner, vec3& eye_pos, vec3& angles){
  if(viewmodel == nullptr)
    return;

  c_viewmodel_offset_settings* settings = get_settings();
  if(settings == nullptr)
    return;

  // We intentionally do not apply the offseted angles, since they look stupid.
  if(settings->silent_aim){
    if(global_internal->entity_list->get_entity(global_internal->aimbot_target_index) != nullptr){
      if(global_internal->aimbot_target_reset_time > math::time()){
        angles = math::calc_view_angle(eye_pos, global_internal->aimbot_target_aim_point);
        if(owner != nullptr)
          angles -= owner->punch_angle();
      }
    }
  }

  viewmodel->set_abs_angles(angles);
}

void c_base_viewmodel_offset::pre_calc_view_model_lag(c_internal_base_entity* viewmodel, vec3& origin, vec3& angles){
  assert(global_internal->cl_wpn_sway_interp != nullptr);
  assert(global_internal->cl_wpn_sway_scale != nullptr);

  if(viewmodel == nullptr)
    return;

  c_viewmodel_offset_settings* settings = get_settings();
  if(settings == nullptr)
    return;

  if(!settings->sway_enabled)
    return;

  #if !defined(GMOD_CHEAT)
    old_cl_wpn_sway_interp = global_internal->cl_wpn_sway_interp->flt;
    old_cl_wpn_sway_scale  = global_internal->cl_wpn_sway_scale->flt;

    global_internal->cl_wpn_sway_interp->flt = 0.1f;
    global_internal->cl_wpn_sway_scale->flt  = math::clamp(settings->sway_amount, 1.f, 10.f);

    reset_sway = true;
  #else
    assert(false && "no gmod support");
  #endif
}

void c_base_viewmodel_offset::post_calc_view_model_lag(c_internal_base_entity* viewmodel, vec3& origin, vec3& angles){
  if(viewmodel == nullptr)
    return;

  reset_sway_data();
}