#include "../link.h"

CLASS_ALLOC(c_freecamera, freecam);


EXPORT bool __fastcall should_draw_localplayer_hook(void* rcx){
  if(anti_screengrab->is_creating_spoofed_view() || utils::is_screen_shot_taken() || acp->is_blocked(FUNC_SCREEN_GRAB))
    return utils::call_fastcall64<bool>(gen_internal->decrypt_asset(global->should_draw_localplayer_hook_trp), rcx);
  
  if(freecam->is_active() || thirdperson->is_active())
    return true;

  return utils::call_fastcall64<bool>(gen_internal->decrypt_asset(global->should_draw_localplayer_hook_trp), rcx);
}

bool c_freecamera::is_active(){
  if(thirdperson->is_in_view_from_camera())
    return false;
  
  if(!config->misc.freecam)
    return false;

  if(acp->is_blocked(FUNC_SCREEN_GRAB))
    return false;

  if(!key.is_valid())
    key = c_key_control(&config->misc.freecam_key, &config->misc.freecam_key_held, &config->misc.freecam_key_toggle, &config->misc.freecam_key_double_click);

  return key.is_toggled();
}

bool c_freecamera::run(s_view_setup* view_setup, i32* what_to_draw){
  if(!is_active()){
    pos = view_setup->origin;
    return false;
  }

  global->info_panel->add_entry(INFO_PANEL_FREECAM, WXOR(L"FREECAM"), WXOR(L"INACTIVE"), rgb(255, 255, 255));
  if(utils::is_screen_shot_taken()){
    global->info_panel->add_entry(INFO_PANEL_FREECAM, WXOR(L"FREECAM"), WXOR(L"ANTI-SCREENGRAB"), INFO_PANEL_WARNING_CLR);
    return false;
  }
  
  if(!pos_setup){
    pos       = view_setup->origin;
    pos_setup = true;
  }

  // Make the screen obviously look different.
  if(what_to_draw != nullptr){
    if(*what_to_draw & RENDERVIEW_DRAWVIEWMODEL)
      *what_to_draw &= ~RENDERVIEW_DRAWVIEWMODEL;

    if(*what_to_draw & RENDERVIEW_DRAWHUD)
      *what_to_draw &= ~RENDERVIEW_DRAWHUD;
  }

  vec3 angle  = view_setup->angles;
  vec3 origin = pos;

  float speed = math::clamp((float)config->misc.freecam_speed, 5.f, 15.f);

  vec3 fwd, side;
  math::angle_2_vector(angle, &fwd, &side, nullptr);

  if(input_system->held(VK_LSHIFT))
    speed *= 2.f;

  if(input_system->held(0x57))
    origin += (fwd * speed);

  if(input_system->held(0x53))
    origin -= (fwd * speed);

  if(input_system->held(0x41))
    origin -= (side * speed);

  if(input_system->held(0x44))
    origin += (side * speed);

  if(input_system->held(VK_SPACE))
    origin.z += speed;
  else if(input_system->held(VK_LCONTROL))
    origin.z -= speed;

  view_setup->origin = origin;
  pos               = origin;

  global->info_panel->add_entry(INFO_PANEL_FREECAM, WXOR(L"FREECAM"), WXOR(L"RUNNING"), INFO_PANEL_LEGIT_CLR);
  return true;
}

bool c_freecamera::run_createmove(s_user_cmd* cmd){
  if(cmd == nullptr)
    return false;

  if(!is_active()){
    // restore the players view to the original angle before being frozen
    if(was_freecam_active){
      cmd->view_angles = freeze_angle;
      global->engine->set_view_angles(freeze_angle);

      was_freecam_active = false;
    }

    freeze_angle = cmd->view_angles;
    return false;
  }

  // set this to true so we can restore the players view to the freeze angle
  was_freecam_active = true;

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