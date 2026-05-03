#include "../../../link.h"

bool c_base_third_person::is_view_third_person(s_view_setup* view, c_internal_base_player* localplayer){
  return localplayer != nullptr ? (localplayer->offset_shoot_pos() - view->origin).length() >= 30.f : false;
}

void c_base_third_person::calculate_view_state(s_view_setup* view_setup){
  c_internal_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  is_view_thirdperson   = is_view_third_person(view_setup, localplayer);
}

void c_base_third_person::render_view(s_view_setup* view_setup, i32* what_to_draw){
  c_third_person_settings* settings = get_settings();
  if(settings == nullptr)
    return;

  c_internal_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_base_alive() || is_view_third_person(view_setup, localplayer))
    return;
  
  // Toggle thirdperson!
  if(settings->thirdperson_usekey){
    if(!key.is_valid())
      key = c_key_control(&settings->thirdperson_key, &settings->thirdperson_key_held, &settings->thirdperson_key_toggle, &settings->thirdperson_key_double_click);

    thirdperson_toggle = key.is_toggled();

    if(!settings->thirdperson)
      thirdperson_toggle = false;
  }
  else
    thirdperson_toggle = settings->thirdperson;


  float right_offset = math::clamp((float)settings->thirdperson_right_offset, -64.f, 64.f);
  if(settings->thirdperson_right_offset_enabled){
    bool apply_offset = true;

    if(!right_master_key.is_valid())
      right_master_key = c_key_control(&settings->thirdperson_right_offset_key, &settings->thirdperson_right_offset_held, &settings->thirdperson_right_offset_toggle, &settings->thirdperson_right_offset_double_click);

    if(settings->thirdperson_right_offset_use_key && !right_master_key.is_toggled())
      apply_offset = false;

    // Fix any config errors.
    if(!settings->thirdperson_right_offset_invert_held && !settings->thirdperson_right_offset_invert_toggle && !settings->thirdperson_right_offset_invert_double_click)
      settings->thirdperson_right_offset_invert_held = true;
    else if(settings->thirdperson_right_offset_invert_held && settings->thirdperson_right_offset_invert_toggle && settings->thirdperson_right_offset_invert_double_click){
      settings->thirdperson_right_offset_invert_held   = true;
      settings->thirdperson_right_offset_invert_toggle = settings->thirdperson_right_offset_invert_double_click = false;
    }

    if(!invert_key.is_valid())
      invert_key = c_key_control(&settings->thirdperson_right_offset_invert_key, &settings->thirdperson_right_offset_invert_held, &settings->thirdperson_right_offset_invert_toggle, &settings->thirdperson_right_offset_invert_double_click);

    if(apply_offset){
      if(invert_key.is_toggled())
        right_offset *= -1.f;

      cam_desired_right = right_offset;
    }
    else
      cam_desired_right = 0.f;
  }
  else
    cam_desired_right = 0.f;

  cam_desired_up       = math::clamp((float)settings->thirdperson_up_offset, -64.f, 64.f);
  cam_desired_distance = math::biggest((float)settings->thirdperson_distance, 30.f);

  if(!thirdperson_toggle)
    return;
  
  if(what_to_draw != nullptr){
    if(*what_to_draw & RENDERVIEW_DRAWVIEWMODEL)
      *what_to_draw &= ~RENDERVIEW_DRAWVIEWMODEL;
  }

  vec3 fwd, right, up;
  math::angle_2_vector(view_setup->angles, &fwd, &right, &up);
  vec3 new_origin = view_setup->origin;

  new_origin += (fwd * -cam_desired_distance);
  new_origin += (right * cam_desired_right);
  new_origin += (up * cam_desired_up);

  if(settings->thirdperson_collision){
    c_base_generic_world_only_trace_filter filter;

    s_trace tr = global_internal->base_trace->internal_ray_obb(filter, view_setup->origin, new_origin, vec3(-2.f, -2.f, -2.f), vec3(2.f, 2.f, 2.f), mask_playersolid);
    view_setup->origin = tr.end;
  }
  else
    view_setup->origin = new_origin;
}