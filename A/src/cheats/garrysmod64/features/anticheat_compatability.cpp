#include "../link.h"

CLASS_ALLOC(c_acp, acp);

void c_acp::calculate_feature_support(){
  if(!config->misc.anti_screen_grab_always && !config->misc.anti_screen_grab_dont_capture_2dcam && !config->misc.anti_screen_grab_disabled)
    config->misc.anti_screen_grab_always = true;

  toggle_from_bool(FUNC_SILENT_AIM, config->misc.no_silent_aim);
  toggle_from_bool(FUNC_SCREEN_GRAB_NO_2D, config->misc.anti_screen_grab_dont_capture_2dcam);
  toggle_from_bool(FUNC_SCREEN_GRAB, config->misc.anti_screen_grab_disabled);
  toggle_from_bool(FUNC_MOVE_FIX, config->misc.no_movement_fix);
}

void c_acp::calculate_should_move_fix(s_user_cmd* cmd){
  if(cmd == nullptr)
    return;

  should_fix_move = false;
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_alive())
    return;

  if(!(localplayer->entity_flags & FL_ONGROUND)){
    should_fix_move = true;
    return;
  }
 

  {
    vec3 engine_view = global->engine->get_view_angles();
    if((engine_view - cmd->view_angles).length() <= 0)
      return;
  
    if(config->aimbot.silent_aim_none && !config->misc.auto_strafe|| is_blocked(FUNC_MOVE_FIX))
      return;
  }
  should_fix_move = true;
}

void c_acp::post_create_move(s_user_cmd* cmd){
  if(cmd == nullptr)
    return;

  calculate_should_move_fix(cmd);
  clamp_viewangles(cmd);
  clamp_movement(cmd);
  set_mouse(cmd);
}

void c_acp::clamp_viewangles(s_user_cmd* cmd){
  if(cmd == nullptr)
    return;

  if(antiaim->antiaim_enabled)
    return;

  cmd->view_angles.x = math::clamp(math::normalize_angle(cmd->view_angles.x), -89.f, 89.f);
  cmd->view_angles.y = math::clamp(math::normalize_angle(cmd->view_angles.y), -180.f, 180.f);
}

void c_acp::clamp_movement(s_user_cmd* cmd){
  if(cmd == nullptr)
    return;

  for(u32 i = 0; i <= 1; i++)
    cmd->move[i] = math::clamp(cmd->move[i], -10000.f, 10000.f);

  cmd->buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);

  if(cmd->move.x > 0.f)
    cmd->buttons |= IN_FORWARD;
  else if(cmd->move.x < 0.f)
    cmd->buttons |= IN_BACK;

  if(cmd->move.y > 0.f)
    cmd->buttons |= IN_MOVERIGHT;
  else if(cmd->move.y < 0.f)
    cmd->buttons |= IN_MOVELEFT;
}

void c_acp::set_mouse(s_user_cmd* cmd){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_alive())
    return;

  if(localplayer->is_in_vehicle() || localplayer->is_frozen())
    return;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return;

  if(wep->get_physgun_grabbed_entity() != nullptr && cmd->buttons & IN_USE)
    return;

  if(!(cmd->buttons & IN_ATTACK))
    return;

  static c_cvar* _pitch = global->cvar->find_var(XOR("m_pitch"));
  static c_cvar* _yaw   = global->cvar->find_var(XOR("m_yaw"));
  if(_pitch == nullptr || _yaw == nullptr){
    DBG("[-] c_acp::set_mouse %p %p one of the cvars is nullptr\n", _pitch, _yaw);
    return;
  }

  vec3 delta = (global->untouched_cmd.view_angles - cmd->view_angles);
  if(delta.length() <= 0)
    return;

  cmd->mouse_x = (i32)(delta.x / _yaw->flt());
  cmd->mouse_y = (i32)(delta.y / _pitch->flt());
}