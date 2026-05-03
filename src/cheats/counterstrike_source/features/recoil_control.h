#pragma once

class c_recoil_control{
public:

  bool correct_view_punch(vec3& corrected_angles, bool visual_punch, bool visual_should_inherit = false){
    if(global->aimbot_settings == nullptr)
      return false;

    if(!utils::is_in_game())
      return false;

    if(visual_punch){
      corrected_angles -= get_visual_punch();
      if(!visual_should_inherit)
        return true;

      corrected_angles += (get_view_punch() - get_compensated_view_punch());
    }
    else
      corrected_angles -= get_compensated_view_punch();

    return true;
  }

  ALWAYSINLINE bool should_view_inherit(){
    if(global->aimbot_settings == nullptr)
      return false;

    if(!global->aimbot_settings->no_recoil || !global->aimbot_settings->no_recoil_legit)
      return false;

    return global->aimbot_settings->no_recoil_visual_inherit;
  }

  bool has_no_recoil(){
    if(!global->aimbot_settings->no_recoil)
      return false;

    if(global->aimbot_settings->no_recoil_legit){
      if(global->aimbot_settings->no_recoil_legit_pitch >= 100.f && global->aimbot_settings->no_recoil_legit_yaw >= 100.f)
        return true;

      return false;
    }

    return global->aimbot_settings->no_recoil_rage;
  }

  // For camera view punch.
  vec3 get_visual_punch(){
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return vec3();

    return localplayer->punch_angle();
  }

  // For viewangle view punch.
  vec3 get_view_punch(){
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return vec3();

    return localplayer->punch_angle() * 2.f;
  }

  vec3 get_uncompensated_view_punch(){
    if(has_no_recoil())
      return vec3();

    return global->aimbot_settings->no_recoil ? (get_view_punch() - get_compensated_view_punch()) : get_view_punch();
  }

  vec3 get_compensated_view_punch(){
    if(global->aimbot_settings == nullptr)
      return vec3();

    vec3 punch = get_view_punch();
    if(punch.length() <= 0.0f)
      return punch;

    global->aimbot_settings->no_recoil_legit_pitch = math::clamp(global->aimbot_settings->no_recoil_legit_pitch, 0.f, 100.f);
    global->aimbot_settings->no_recoil_legit_yaw = math::clamp(global->aimbot_settings->no_recoil_legit_yaw, 0.f, 100.f);
    if(global->aimbot_settings->no_recoil_legit){
      if(global->aimbot_settings->no_recoil_legit_pitch > 0.f && global->aimbot_settings->no_recoil_legit_pitch_enabled)
        punch.x *= (global->aimbot_settings->no_recoil_legit_pitch / 100.f);
      else
        punch.x = 0.f;

      if(global->aimbot_settings->no_recoil_legit_yaw > 0.f && global->aimbot_settings->no_recoil_legit_yaw_enabled)
        punch.y *= (global->aimbot_settings->no_recoil_legit_yaw / 100.f);
      else
        punch.y = 0.f;
    }

    return punch;
  }
};

CLASS_EXTERN(c_recoil_control, recoil_control);