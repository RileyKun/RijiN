#include "../link.h"

CLASS_ALLOC(c_auto_rocket_jumper, auto_rocket_jumper);

//TODO
// Fix issue where the rocket will epicly miss your feet due to your position on the map (above an opening or on a thin ledge)
// Make it so when you're about to hit the ground next tick. To not do the wall detection stuff.
void c_auto_rocket_jumper::run(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr){
    is_performing_jump = false;
    return;
  }

  if(!localplayer->is_valid()){
    is_performing_jump = false;
    return;
  }

  // Reset if we're on the ground.
  if(localplayer->entity_flags() & FL_ONGROUND)
    is_performing_jump = false;

  if(!config->automation.auto_rocket_jumper)
    return;

  // Always require a key.
  if(!input_system->held(config->automation.auto_rocket_jumper_key))
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  if(!can_weapon_blast_jump())
    return;

  //if(acp->is_active()){
  //  global->info_panel->add_entry(INFO_PANEL_AUTO_ROCKET_JUMPER, WXOR(L"ANTI-CHEAT COMPATIBILITY"), INFO_PANEL_WARNING_CLR);
  //  return;
  //}

  if(localplayer->water_level() > 1 || !weapon->is_rocket_launcher() && localplayer->water_level() > 0){
    global->info_panel->add_entry(INFO_PANEL_AUTO_ROCKET_JUMPER, WXOR(L"IN DEEP WATER"), INFO_PANEL_WARNING_CLR);
    null_fire_buttons();
    return;
  }

  // There is a surface that is too close above us. Don't perform any jumps!
  if(!should_auto_jump()){
    global->info_panel->add_entry(INFO_PANEL_AUTO_ROCKET_JUMPER, WXOR(L"SURFACE ABOVE"), INFO_PANEL_WARNING_CLR);
    null_fire_buttons();
    return;
  }

  float time = math::time();
  if(target_angle_time > time){
    global->current_cmd->view_angles = vec3(best_pitch, best_yaw, 0.f);
    math::clamp_angles(global->current_cmd->view_angles);
    if(!localplayer->can_fire())
      target_angle_time = -1.f;

    global->info_panel->add_entry(INFO_PANEL_AUTO_ROCKET_JUMPER,  WXOR(L"RUNNING"), INFO_PANEL_LEGIT_CLR);
    return;
  }

  // Disallow double tap since it causes issues with this feature.
  global->allow_double_tap = false;
  if(!(global->current_cmd->buttons & IN_ATTACK))
    return;

  if(!localplayer->can_fire()){
    null_fire_buttons();
    return;
  }

  bool  fan           = weapon->weapon_id() == WPN_FestiveFAN || weapon->weapon_id() == WPN_FAN;
  bool  on_ground     = localplayer->entity_flags() & FL_ONGROUND;

  vec3  vel           = localplayer->velocity();
  vel.z = 0.f;
  float speed         = vel.length();

  float pitch = 89.f;
  float yaw   = global->current_cmd->view_angles.y;
  if(on_ground){
    if(global->original_cmd.buttons & IN_FORWARD)
      yaw += 180.f;
    else if(global->original_cmd.buttons & IN_MOVELEFT || global->original_cmd.buttons & IN_LEFT)
      yaw -= 90.f;
    else if(global->original_cmd.buttons & IN_MOVERIGHT || global->original_cmd.buttons & IN_RIGHT)
      yaw += 90.f;

    if(speed < 50.f){

      // Nothing fancy. Attempt to do a "c-tap height rocket jump"
      if(!should_target_angle(pitch, yaw)){
        if(!should_target_angle(pitch, yaw, 0)){
          global->info_panel->add_entry(INFO_PANEL_AUTO_ROCKET_JUMPER, WXOR(L"NO GROUND SURFACE 1"), INFO_PANEL_WARNING_CLR);
          null_fire_buttons();
          return;
        }
      }
    }
    else{

      // We want a little more speed when we're moving.
      pitch = 65.f;
      if(!should_target_angle(pitch, yaw)){
        if(!should_target_angle(pitch, yaw, 0)){
          global->info_panel->add_entry(INFO_PANEL_AUTO_ROCKET_JUMPER, WXOR(L"NO GROUND SURFACE 2"), INFO_PANEL_WARNING_CLR);
          null_fire_buttons();
          return;
        }
      }
    }

    if(fan){
      global->info_panel->add_entry(INFO_PANEL_AUTO_ROCKET_JUMPER,WXOR(L"NOT IN AIR"), INFO_PANEL_WARNING_CLR);
      null_fire_buttons();
      return;
    }
  }
  else{
    if(fan)  // Nothing over the top with the fan. No traces required just go for it.
      pitch = 89.f;
    else{
      global->info_panel->add_entry(INFO_PANEL_AUTO_ROCKET_JUMPER, WXOR(L"IN AIR"), INFO_PANEL_WARNING_CLR);
      null_fire_buttons();
      return;
    }
  }

  {
    is_performing_jump  = true;
    best_pitch          = pitch;
    best_yaw            = yaw;
    target_angle_time   = time + utils::get_latency() + math::ticks_to_time(1);
    global->current_cmd->view_angles = vec3(pitch, yaw, 0.f);
    if(on_ground)
      global->current_cmd->buttons |= (IN_JUMP | IN_DUCK);

    global->current_cmd->buttons |= IN_ATTACK;
    math::clamp_angles(global->current_cmd->view_angles);
  }
}

bool c_auto_rocket_jumper::should_target_angle(float& pitch, float& yaw, i32 scan){

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  vec3 start_pos = localplayer->origin();
  vec3 mins      = localplayer->obb_mins();
  vec3 maxs      = localplayer->obb_maxs();
  vec3 pos       = create_trace_point(start_pos, pitch, yaw, ROCKET_JUMPER_TRACE_SIZE);
  s_trace trace  = global->trace->ray_obb(start_pos, pos, mins, maxs, mask_playersolid);
  if(trace.hit_entity != nullptr)
    return true;


  bool found_target   = false;
  float best_fraction = 1.0f;
  //Pitch scan.
  if(scan == 0){

    for(float x = 65.f; x <= 87.f; x += 1.0f){
      vec3 pos      = create_trace_point(start_pos, x, yaw, ROCKET_JUMPER_TRACE_SIZE);
      s_trace trace = global->trace->ray_obb(start_pos, pos, mins, maxs, mask_playersolid);

      // Must hit a surface that is close to us.
      if(trace.fraction >= best_fraction || trace.fraction >= 1.0f || trace.hit_entity == nullptr)
        continue;

      pitch         = x;
      best_fraction = trace.fraction;
      found_target  = true;
      DBG("[!] pitch: %2.2f - %2.2f\n", pitch, best_fraction);
    }
  }
  else if(scan == 1){

    for(float y = yaw - 90.f; y <= yaw + 90.f; y += 10.0f){
      vec3 pos      = create_trace_point(start_pos, pitch, y, ROCKET_JUMPER_TRACE_SIZE);
      s_trace trace = global->trace->ray_obb(start_pos, pos, mins, maxs, mask_playersolid);

      // Must hit a surface that is close to us.
      if(trace.fraction >= best_fraction || trace.fraction >= 1.0f ||  trace.hit_entity == nullptr)
        continue;

      yaw        = y;
      best_fraction = trace.fraction;
      found_target  = true;
    }
  }
  else if(scan == 2){

    for(float x = 65.f; x <= 87.f; x += 1.0f){
      for(float y = yaw - 90.f; y <= yaw + 90.f; y += 10.0f){
        vec3 pos      = create_trace_point(start_pos, x, y, ROCKET_JUMPER_TRACE_SIZE);
        s_trace trace = global->trace->ray_obb(start_pos, pos, mins, maxs, mask_playersolid);

        // Must hit a surface that is close to us.
        if(trace.fraction >= best_fraction|| trace.fraction >= 1.0f || trace.hit_entity == nullptr)
          continue;

        pitch      = x;
        yaw        = y;
        best_fraction = trace.fraction;
        found_target  = true;
      }
    }
  }

  return found_target && best_fraction != 1.0f;
}

bool c_auto_rocket_jumper::can_weapon_blast_jump(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return false;

  if(wep->is_rocket_launcher() || wep->weapon_id() == WPN_Detonator || wep->weapon_id() == WPN_ScorchShot || wep->weapon_id() == WPN_FAN || wep->weapon_id() == WPN_FestiveFAN)
    return true;

  return false;
}

bool c_auto_rocket_jumper::should_auto_jump(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  if(!(localplayer->entity_flags() & FL_ONGROUND))
    return true;

  float ping = math::clamp(utils::get_latency(), 0.1f, 4.f);

  vec3 mins          = localplayer->obb_mins();
  vec3 maxs          = localplayer->obb_maxs();
  vec3 shoot_pos     = utils::trace_line(localplayer->shoot_pos(), localplayer->shoot_pos() + (localplayer->velocity() * ping));

  vec3 pos           = create_trace_point(shoot_pos, -89.f, global->current_cmd->view_angles.y, ROCKET_JUMPER_OBSTRUCTION_ABOVE_SIZE);
  s_trace trace = global->trace->ray_obb(shoot_pos, pos, mins, maxs, mask_playersolid);

  return trace.hit_entity == nullptr;
}