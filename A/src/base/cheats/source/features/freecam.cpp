#include "../../../link.h"

bool c_base_free_camera::render_view(s_view_setup* view_setup, i32* what_to_draw){
  if(!is_active()){
    pos = view_setup->origin;
    return false;
  }

  on_free_camera_begin();
  {
    if(!pos_setup){
      pos       = view_setup->origin;
      pos_setup = true;
    }

    // Hide the view model.
    if(what_to_draw != nullptr){
      if(*what_to_draw & RENDERVIEW_DRAWVIEWMODEL)
        *what_to_draw &= ~RENDERVIEW_DRAWVIEWMODEL;
    }

    vec3 ang    = view_setup->angles;
    vec3 origin = pos;

    float speed = math::clamp(get_freecam_speed(), 5.f, 20.f) * 100.f;
    if(input_system->held(VK_LSHIFT))
      speed *= 2.0f;


    vec3 fwd, side, up;
    math::angle_2_vector(ang, &fwd, &side, &up);
    if(input_system->held(0x57))
      origin += (fwd * speed) * get_frame_time();

    if(input_system->held(0x53))
      origin -= (fwd * speed) * get_frame_time();

    if(input_system->held(0x41))
      origin -= (side * speed) * get_frame_time();

    if(input_system->held(0x44))
      origin += (side * speed) * get_frame_time();

    if(input_system->held(VK_SPACE))
      origin += (up * speed) * get_frame_time();
    else if(input_system->held(VK_LCONTROL))
      origin -= (up * speed) * get_frame_time();

    view_setup->origin = origin;
    pos                = origin;
  }
  on_free_camera_end();
  return true;
}


bool c_base_free_camera::pre_create_move_prediction(c_user_cmd* cmd){
  if(cmd == nullptr)
    return false;

  if(!is_active()){
    freeze_angle = cmd->view_angles;
    return false;
  }

  cmd->buttons &= ~IN_FORWARD;
  cmd->buttons &= ~IN_BACK;
  cmd->buttons &= ~IN_MOVELEFT;
  cmd->buttons &= ~IN_MOVERIGHT;
  cmd->buttons &= ~IN_DUCK;
  cmd->buttons &= ~IN_JUMP;
  cmd->buttons &= ~IN_RUN;

  for(i32 i = 0; i <= 2; i++)
    cmd->move[i] = 0.f;

  cmd->view_angles = freeze_angle;

  return true;
}