#include "../link.h"

CLASS_ALLOC(c_antiaim, antiaim);
#define ANTI_AIM_MAX_CHOKE 1
void c_antiaim::run(){
  kernel_time_expired_return();
  if(!config->hvh.antiaim)
    return;

  if(!should_antiaim())
    return;

  if(global->current_cmd->buttons & IN_USE)
    return;

  if(!config->hvh.antiaim_rage && !config->hvh.antiaim_legit)
    config->hvh.antiaim_rage = true;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  if(!antiaim_enabled){
    global->info_panel->add_entry(INFO_PANEL_ANTIAIM, WXOR(L"ANTI-AIM"), WXOR(L"INACTIVE"));
    return;
  }

  config->hvh.pitch_fake_none = true;
  config->hvh.pitch_fake_up   = false;
  config->hvh.pitch_fake_down = false;

  global->info_panel->add_entry(INFO_PANEL_ANTIAIM, WXOR(L"ANTI-AIM"), config->hvh.antiaim_legit ? WXOR(L"LEGIT") : WXOR(L"RAGE"), config->hvh.antiaim_legit ? INFO_PANEL_LEGIT_CLR : INFO_PANEL_RAGE_CLR);

  // Re-enable me when added
  //if(!fake_latency->is_synced())
  //  global->info_panel->add_entry(INFO_PANEL_ANTIAIM, WXOR(L"ANTI-AIM"), WXOR(L"FAKE LATENCY SYNC"), INFO_PANEL_WARNING_CLR);

  antiaim_on_fire       = false;
  antiaim_is_first_time = true; // Tell that's the first time we're calling the pitch & yaw aa function this batch.

  if(global->client_state->choked_commands <= ANTI_AIM_MAX_CHOKE)
    set_choke(true);

  setup_antiaims();
  antiaim_is_first_time = false;
}

// Returns false on fail
bool c_antiaim::set_choke(bool choke){
  bool result = true;

  // If we're currently faking yaw angles we need at least 3 choked commands for an effective anti-aim.
  if(is_faking_yaw() && global->client_state->choked_commands <= ANTI_AIM_MAX_CHOKE){
    choke  = true;
    result = false;
  }

  packet_manager->set_choke(choke);
  antiaim_send_packet = packet_manager->choking();

  if(antiaim_is_first_time)
    return;

  // Resetup all of the anti-aims.
  setup_antiaims();
  return result;
}

void c_antiaim::setup_antiaims(){
  if(!config->hvh.antiaim)
    return;

  if(!should_antiaim())
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  if(!antiaim_enabled)
    return;

  // Do not resetup anti-aim if we've fired this tick. Check c_antiaim::run_on_fire to see how this is set.
  if(antiaim_on_fire){
    DBG("[!] disallowed setup of anti-aim because we have fired a shot this tick!\n");
    return;
  }

  using_fake_pitch    = false;
  antiaim_send_packet = packet_manager->choking();

  pitch(localplayer);
  yaw();

  //DBG("[!] anti-aim %s\n", antiaim_is_first_time ? "first time setup" : "re-setup");
}

void c_antiaim::pitch(c_base_player* localplayer){
  if(config->hvh.antiaim_pitch_disabled)
    return;

  if(config->hvh.antiaim_legit)
    return;

  float              wanted_angle = 0.f;
  static bool swap   = false;

  // Fix config errors.
  {
    if(!config->hvh.antiaim_pitch_static && !config->hvh.antiaim_pitch_dynamic)
      config->hvh.antiaim_pitch_static = true;
  }

  if(config->hvh.antiaim_pitch){
    if(config->hvh.antiaim_pitch_static){
      if(config->hvh.pitch_real_up)
        wanted_angle = config->hvh.pitch_fake_none ? -89.f : 271.f;
      else if(config->hvh.pitch_real_zero)
        wanted_angle = config->hvh.pitch_fake_none ? 0.f : 1080.f;
      else if(config->hvh.pitch_real_down)
        wanted_angle = config->hvh.pitch_fake_none ? 89 : 271.f;
      else
        wanted_angle = -89.f;

      // fake angles are enabled.
      if(!config->hvh.pitch_fake_none){
        if(config->hvh.pitch_fake_up)
          wanted_angle = -math::abs(wanted_angle);
        else if(config->hvh.pitch_fake_down)
          wanted_angle = math::abs(wanted_angle);

        using_fake_pitch = true;
      }

      // Handle the same real & fake like real up and fakeup.
      if(config->hvh.pitch_real_up && config->hvh.pitch_fake_up)
        wanted_angle = -91.f;
      else if(config->hvh.pitch_real_down && config->hvh.pitch_fake_down)
        wanted_angle = 91.f;

      global->current_cmd->view_angles.x = wanted_angle;
    }
    else if(config->hvh.antiaim_pitch_dynamic){
      using_fake_pitch = true;
      global->current_cmd->view_angles.x = swap ? -89.f : 89.f;
    }
  }
  else if(config->hvh.antiaim_pitch_auto){
    using_fake_pitch                   = true;
    global->current_cmd->view_angles.x = get_best_pitch(localplayer);
    if(config->hvh.antiaim_pitch_dynamic){
      if(!swap){
        if(global->current_cmd->view_angles.x > 0.f)
          global->current_cmd->view_angles.x = -89.f;
        else if(global->current_cmd->view_angles.x < 0.f)
          global->current_cmd->view_angles.x = 89.f;
        else{
          DBG("[-] Invalid auto dynamic pitch!\n");
        }
      }
    }
  }

  // Only change when sending. Otherwise we can have cases where our pitch wouldn't appear to be "jittering" to other players due to packet choking.
  if(!packet_manager->choking())
    swap = !swap;

  // If we're in the water we'll start moving towards where our pitch is at.
  if(localplayer->water_level > 1){
    if(!config->hvh.pitch_fake_none){
      if(config->hvh.pitch_fake_up)
        global->current_cmd->view_angles.x = -1080.f + global->untouched_cmd.view_angles.x;
      else if(config->hvh.pitch_fake_down)
        global->current_cmd->view_angles.x = 1080.f + global->untouched_cmd.view_angles.x;
    }
    else
      global->current_cmd->view_angles.x = global->untouched_cmd.view_angles.x;
  }
}

void c_antiaim::yaw(){
  if(!config->hvh.antiaim_yaw)
    return;

  float time = math::time();
  static float real_spin = 0.f;
  static float fake_spin = 0.f;

  clamp_and_normalize_angle(real_spin);
  clamp_and_normalize_angle(fake_spin);

  static bool  rotate_dynamic = false;
  static float next_swap_time = 0.f;
  if(config->hvh.yaw_rotate_dynamic_update_rate > 0.f){
    if(next_swap_time <= time){
      rotate_dynamic = !rotate_dynamic;
      next_swap_time = time + (config->hvh.yaw_rotate_dynamic_update_rate / 1000.f);
    }
  }

  // REAL
  if(packet_manager->choking()/* || !fake_latency->is_synced()*/){

    if(config->hvh.yaw_method_real_rotate || config->hvh.yaw_method_real_rotate_dynamic){

      i32 rotation_target = 0;
      {
        if(config->hvh.yaw_rotate_real_target_method_closetargets)
          rotation_target = 1;
      }

      float rotate_real_ang = config->hvh.rotate_real_ang;
      adjust_real_rotate_angle(rotate_real_ang);

      vec3 angle;
      get_target_angle(rotation_target, &angle);
      if(!config->hvh.yaw_method_real_rotate_dynamic || !rotate_dynamic)
        angle.y += rotate_real_ang;
      else
        angle.y += config->hvh.rotate_real_ang2;

      global->current_cmd->view_angles.y = angle.y;

    }
    else if(config->hvh.yaw_method_real_spin){

      // Only increase / decrease spin values if first time running the function.
      if(antiaim_is_first_time){
        if(config->hvh.spin_real_invert)
          real_spin -= config->hvh.spin_real_speed;
        else
          real_spin += config->hvh.spin_real_speed;
      }

      global->current_cmd->view_angles.y = real_spin;
    }
  }
  else{ // FAKE
    if(!config->hvh.antiaim_legit){
      if(config->hvh.yaw_method_fake_rotate || config->hvh.yaw_method_fake_rotate_dynamic){
        i32 rotation_target = 0;
        {
          if(config->hvh.yaw_rotate_fake_target_method_closetargets)
            rotation_target = 1;
        }
  
        vec3 angle;
        get_target_angle(rotation_target, &angle);
        if(!config->hvh.yaw_method_fake_rotate_dynamic || !rotate_dynamic)
          angle.y += config->hvh.rotate_fake_ang;
        else
          angle.y += config->hvh.rotate_fake_ang2;
  
        global->current_cmd->view_angles.y = angle.y;
        center_real_yaw();
      }
      else if(config->hvh.yaw_method_fake_spin){
  
        // Only increase / decrease spin values if first time running the function.
        if(antiaim_is_first_time){
          if(config->hvh.spin_fake_invert)
            fake_spin -= config->hvh.spin_fake_speed;
          else
            fake_spin += config->hvh.spin_fake_speed;
        }
  
        global->current_cmd->view_angles.y = fake_spin;
      }
    }
  }

  auto_edge();
 
  // detect other features breaking yaw anti-aim.
  antiaim_send_packet = packet_manager->choking();

  // normalize the yaw angle.
  global->current_cmd->view_angles.y = math::normalize_angle(global->current_cmd->view_angles.y);
  if(packet_manager->choking()){
    // We're currently choking packets. Lets add our choked angles to a list.
    add_to_batch(global->current_cmd->view_angles, false);
  }
  else{
    // Add this tick to this batch before sending off.
    add_to_batch(global->current_cmd->view_angles, true);
    compute_aim_yaw();
  }
}

void c_antiaim::adjust_real_rotate_angle(float& yaw){
  if(!config->hvh.antiaim_legit)
    return;

  assert(global->current_cmd != nullptr);
  float pitch = global->current_cmd->view_angles.x;
  if(pitch <= -15.f) // This is an issue with pitch angles that are downwards / near zero.
    return;

  yaw = math::normalize_angle(yaw);

  float delta_from_best_angle = math::abs(math::abs(yaw) - 135.f);
  if(yaw > 0.f && yaw < 135.f)
    yaw += delta_from_best_angle;
  else if(yaw < 0.f && yaw > -135.f)
    yaw -= delta_from_best_angle;
}

ALWAYSINLINE void c_antiaim::clamp_and_normalize_angle(float& angle){
  angle = math::clamp(math::normalize_angle(angle), -180.f, 180.f);
}

void c_antiaim::auto_edge(){
  if(!config->hvh.antiaim_yaw)
    return;

  if(!config->hvh.yaw_auto_edge_enabled)
    return;

  // Resolve config conflicts.
  if(config->hvh.yaw_auto_edge_fake == config->hvh.yaw_auto_edge_real)
    config->hvh.yaw_auto_edge_real = true;

  // We have legit anti-aim enabled.
  if(config->hvh.yaw_auto_edge_fake && config->hvh.antiaim_legit && !packet_manager->choking())
    return;
  
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  i32 rotation_target = 0;
  {
    if(config->hvh.yaw_auto_edge_target_method_closetargets)
      rotation_target = 1;
  }

  vec3 angle;
  vec3 target_pos;
  if(!get_target_angle(rotation_target, &angle, &target_pos))
    return;

  float new_angle = 0.f;
  if(!get_auto_yaw_angle(localplayer, angle.y, target_pos, 1.5f, {-90.f, 90.f}, new_angle))
    return;

  if(global->current_cmd->view_angles.x <= -25.f)
    new_angle = math::normalize_angle(new_angle + 180.f);

  if(packet_manager->choking() && config->hvh.yaw_auto_edge_real){ // REAL
    global->current_cmd->view_angles.y = new_angle;
  }
  else if(!packet_manager->choking() && config->hvh.yaw_auto_edge_fake){ // FAKE
    global->current_cmd->view_angles.y = new_angle;
  }

  center_real_yaw();
}

void c_antiaim::break_animations(bool pre_prediction){
  if(global->settings->hvh.break_animations_disabled)
    return;

  c_base_player* localplayer = utils::localplayer();

  if(localplayer == nullptr)
    return;

  if(localplayer->move_type == MOVETYPE_NOCLIP || localplayer->move_type == MOVETYPE_LADDER)
    return;

  if(localplayer->entity_flags & FL_ONGROUND)
    return;

  if(localplayer->velocity().z < 0.f)
    return;

  // If we are in a small area then we cant do this
  {
    vec3 mins = localplayer->obb_mins();
    vec3 maxs = localplayer->obb_mins();
    s_trace tr = global->trace->ray_obb(localplayer->origin(), localplayer->origin() + vec3(0.f, 0.f, 24.f), mins, maxs, localplayer);

    if(tr.fraction != 1.f)
      return;
  }

  if(pre_prediction){
    s_lc_data* lc_data = lag_compensation->get_data(localplayer);

    if(lc_data == nullptr)
      return;

    s_lc_record* record1 = lc_data->get_record(0);
    s_lc_record* record2 = lc_data->get_record(1);

    if(record1 == nullptr || record2 == nullptr)
      return;

    static bool should_run = true;
    if((record1->entity_flags & FL_ONGROUND) && (record2->entity_flags & FL_ONGROUND))
      should_run = true;

    if(localplayer->entity_flags & FL_ONGROUND)
      return;

    if((global->settings->hvh.break_animations_on_jump || global->original_cmd.buttons & IN_DUCK) && !should_run)
      return;

    if(localplayer->entity_flags & FL_DUCKING){
      global->current_cmd->buttons &= ~IN_DUCK;
      should_run = false;
    }
    else
      global->current_cmd->buttons |= IN_DUCK;
  }
  else if((localplayer->entity_flags & FL_ONGROUND) && (global->original_cmd.buttons & IN_JUMP)){
    global->info_panel->add_entry(INFO_PANEL_ANTIAIM_BREAK_ANIMATIONS, WXOR(L"BREAK ANIMATIONS"), WXOR(L"ACTIVE"), INFO_PANEL_RAGE_CLR);
    set_choke(true);
  }
}

void c_antiaim::run_on_fire(){
  return;
  antiaim_on_fire = true;
  if(!antiaim_enabled)
    return;

  if(!config->hvh.antiaim || config->hvh.antiaim_pitch_disabled)
    return;

  if(!should_antiaim())
    return;

  if(!using_fake_pitch)
    return;

  if(config->hvh.antiaim_usekey && !antiaim_toggle)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  global->info_panel->add_entry(INFO_PANEL_ANTIAIM, WXOR(L"ANTI-AIM"), WXOR(L"FIRING MODE"), INFO_PANEL_WARNING_CLR);
  assert(global->current_cmd != nullptr);
  if(config->hvh.antiaim_pitch_auto){

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    float pitch = get_best_pitch(localplayer);
    if(pitch > 0.f)
      global->current_cmd->view_angles.x = 1080.f + global->current_cmd->view_angles.x;
    else if(pitch <= 0.f)
      global->current_cmd->view_angles.x = -1080.f + global->current_cmd->view_angles.x;
  }
  else if(!config->hvh.pitch_fake_none){
    if(config->hvh.pitch_fake_up)
      global->current_cmd->view_angles.x = -1080.f + global->current_cmd->view_angles.x;
    else if(config->hvh.pitch_fake_down)
      global->current_cmd->view_angles.x = 1080.f + global->current_cmd->view_angles.x;
  }
}

// This has an issue.
bool c_antiaim::thirdperson(vec3* ang){
  if(!utils::is_in_thirdperson())
    return false;

  if(global->engine->is_playing_demo())
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  // Want to see the real pitch angle.
  float real_pitch_angle = global->both_angles.x;

  if(math::abs(real_pitch_angle) > 91.f){
    while(real_pitch_angle < -91.f)
      real_pitch_angle += 360.f;

    while(real_pitch_angle > 91.f)
      real_pitch_angle -= 360.f;
  }

  if(antiaim->is_faking_yaw())
    *ang = vec3(real_pitch_angle, feet_yaw, 0.f);
  else
    *ang = (global->engine->get_latency(true) <= math::ticks_to_time(1)) ? global->last_sent_angle : localplayer->viewangles();

  ang->x = math::clamp(math::normalize_angle(ang->x), -89.f, 89.f);
  ang->y = math::clamp(math::normalize_angle(ang->y), -180.f, 180.f);
  ang->z = 0.f;
  return true;
}

bool c_antiaim::should_antiaim(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return false;

  if(wep->is_rpg_guiding_missile())
    return false;

  if(localplayer->move_type == MOVETYPE_NOCLIP || localplayer->move_type == MOVETYPE_LADDER)
    return false;

  if(acp->is_blocked(FUNC_MOVE_FIX) || acp->is_blocked(FUNC_SILENT_AIM))
    return false;

  return true;
}

bool c_antiaim::is_faking_yaw(){
  if(!config->hvh.antiaim || !antiaim_enabled)
    return false;

  if(!config->hvh.antiaim_yaw)
    return false;

  if(!should_antiaim())
    return false;

  if(global->firing_this_tick)
    return false;

  //if(!fake_latency->is_synced())
  //  return false;

  return true;
}

void c_antiaim::stabilize_yaw(){
  if(!config->hvh.antiaim || !antiaim_enabled)
    return;

  if(!config->hvh.antiaim_yaw)
    return;

  if(!should_antiaim())
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  if(!(localplayer->entity_flags & FL_ONGROUND))
    return;

  if(global->current_cmd->buttons & IN_FORWARD || global->current_cmd->buttons & IN_BACK || global->current_cmd->buttons & IN_MOVELEFT || global->current_cmd->buttons & IN_MOVERIGHT || global->current_cmd->buttons & IN_JUMP)
    return;

  if(localplayer->will_fire_this_tick())
    return;

  if(localplayer->velocity().length() > 5.f)
    return;

  // The goal is to always be constantly moving more than 1 hu so the server makes our yaw angle constantly update.
  // There is a problem with this. It will move you very slowly backwards. Maybe increasing the yaw spin speed might fix it or make it even slower.
  // I moved like 6 meters in 30 minutes when I went afk.
  // - Rud
  static float spin = 0.f;
  global->original_cmd.view_angles.x = 89.f;

  spin += 12.f;
  if(spin > 180.f)
    spin = -180.f;

  global->original_cmd.view_angles.y = spin;
  global->current_cmd->move.x = 1.25f;
  if(global->current_cmd->buttons & IN_DUCK)
    global->current_cmd->move.x = 4.175f;

  global->current_cmd->move.x *= 6.5f;
}

void c_antiaim::center_real_yaw(){

  return;
  if(!antiaim_is_first_time)
    return;

  if(!config->hvh.antiaim || !antiaim_enabled)
    return;

  if(!config->hvh.antiaim_yaw)
    return;

  if(config->hvh.antiaim_legit)
    return;

  if(packet_manager->choking()) // We need to jitter the fake angle to center the real yaw angle. 
    return;

  if(!should_antiaim())
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  static i32  jitter_tick = 0;
  static bool fake_jitter = false;

  if(jitter_tick == globaldata->tick_count)
    return;

  jitter_tick = globaldata->tick_count;
  fake_jitter = !fake_jitter;

  // Help to center our real angle on the server.
  global->current_cmd->view_angles.y += fake_jitter ? 10.8f : -10.8f;
  global->current_cmd->view_angles.y = math::normalize_angle(global->current_cmd->view_angles.y);
}

void c_antiaim::pre_prediction(){
  process_input();
  process_cycle();
  stabilize_yaw();
  break_animations(true);
  fake_duck(true);
}

bool c_antiaim::get_target_angle(i32 type, vec3* angle, vec3* pos){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  if(angle == nullptr)
    return false;

  float ping                 = utils::get_latency() + globaldata->interval_per_tick;
  float best_dist            = -1.f;
  c_base_player* best_player = nullptr;


  for(i32 i = 1; i <= globaldata->max_clients; i++){
    if(i == global->engine->get_local_player())
      continue;

    c_base_player* player = global->entity_list->get_entity(i);
    if(player == nullptr)
      continue;

    if(!player->is_valid())
      continue;

    // Ignore bots.
    s_player_info info = player->info();
    if(info.fake_player)
      continue;

    c_base_weapon* wep = player->get_weapon();
    if(wep == nullptr)
      continue;

    // Ignore weapons that deal no damage or have their hit detection based on the bounding box rather than hitboxes.
    if(wep->is_harmless_weapon() || wep->is_melee())
      continue;

    if(!localplayer->is_considered_enemy(player))
      continue;

    float dist = localplayer->origin().distance(player->origin());
    if(dist > best_dist && best_dist > -1.f)
      continue;

    best_dist   = dist;
    best_player = player;
  }

  if(best_player != nullptr){
    s_trace tr = global->trace->ray(best_player->shoot_pos(), best_player->shoot_pos() + (best_player->velocity() * ping), mask_playersolid);
    if(pos != nullptr)
      *pos                = best_player->shoot_pos();
    
    if(type != 0)
      *angle              = math::calc_view_angle(localplayer->shoot_pos(), tr.end);
    else
      *angle              = global->untouched_cmd.view_angles;

    return true;
  }
  else
    *angle = global->untouched_cmd.view_angles;

  return false;
}

void c_antiaim::process_input(){

  // Have alot of functions in here that require checks to keybinds and other various states. So this function and variable makes sense.
  antiaim_enabled = false;
  if(!config->hvh.antiaim)
    return;

  if(!should_antiaim())
    return;

  if(acp->is_blocked(FUNC_MOVE_FIX))
    return;

  // Fix config issue.
  if(config->hvh.antiaim_usekey){
    static c_key_control key(&config->hvh.antiaim_key, &config->hvh.antiaim_key_held, &config->hvh.antiaim_key_toggle, &config->hvh.antiaim_key_double_click);

    antiaim_toggle = key.is_toggled();
    antiaim_enabled = antiaim_toggle;
    return;
  }

  antiaim_enabled = true;
}

void c_antiaim::process_cycle(){
  if(!config->hvh.antiaim || !config->hvh.antiaim_yaw || !config->hvh.antiaim_cycle_enabled)
    return;

  if(!should_antiaim())
    return;

  if(!antiaim_enabled)
    return;

  static bool last_cycle_key_held = false;
  bool cycle_key_held             = input_system->held(config->hvh.antiaim_cycle_key);
  if(cycle_key_held && !last_cycle_key_held){

    if(config->hvh.antiaim_cycle_swap_settings){
      bool old_method_real_rotate         = config->hvh.yaw_method_real_rotate;
      bool old_method_real_rotate_dynamic = config->hvh.yaw_method_real_rotate_dynamic;
      bool old_method_real_spin           = config->hvh.yaw_method_real_spin;

      float old_real_ang  = config->hvh.rotate_real_ang;
      float old_real_ang2 = config->hvh.rotate_real_ang2;

      i32  old_spin_real_speed    = config->hvh.spin_real_speed;
      bool old_spin_real_invert   = config->hvh.spin_real_invert;

      // Copy the fake settings over to the real settings.
      config->hvh.yaw_method_real_rotate         = config->hvh.yaw_method_fake_rotate;
      config->hvh.yaw_method_real_rotate_dynamic = config->hvh.yaw_method_fake_rotate_dynamic;
      config->hvh.yaw_method_real_spin           = config->hvh.yaw_method_fake_spin;

      config->hvh.rotate_real_ang  = config->hvh.rotate_fake_ang;
      config->hvh.rotate_real_ang2 = config->hvh.rotate_fake_ang2;

      config->hvh.spin_real_speed        = config->hvh.spin_fake_speed;
      config->hvh.spin_real_invert       = config->hvh.spin_fake_invert;

      // Copy the real settings we've backed up to the fake settings.
      config->hvh.yaw_method_fake_rotate         = old_method_real_rotate;
      config->hvh.yaw_method_fake_rotate_dynamic = old_method_real_rotate_dynamic;
      config->hvh.yaw_method_fake_spin           = old_method_real_spin;

      config->hvh.rotate_fake_ang  = old_real_ang;
      config->hvh.rotate_fake_ang2 = old_real_ang2;

      config->hvh.spin_fake_speed        = old_spin_real_speed;
      config->hvh.spin_fake_invert       = old_spin_real_invert;

    }
    else{
      // This pretty much will "invert" the option. Means if the yaw angle is 90 it'll become -90 and vice versa.
      if(config->hvh.antiaim_cycle_invert_fake_angle || config->hvh.antiaim_cycle_invert_both_angles){
        config->hvh.rotate_fake_ang  = -config->hvh.rotate_fake_ang;
        config->hvh.rotate_fake_ang2 = -config->hvh.rotate_fake_ang2;
      }

      if(config->hvh.antiaim_cycle_invert_real_angle || config->hvh.antiaim_cycle_invert_both_angles){
        config->hvh.rotate_real_ang  = -config->hvh.rotate_real_ang;
        config->hvh.rotate_real_ang2 = -config->hvh.rotate_real_ang2;
      }
    }
  }
  last_cycle_key_held = cycle_key_held;
}

float c_antiaim::get_best_pitch(c_base_player* localplayer){
  if(localplayer == nullptr)
    return 89.f;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  return 89.f;
}

bool c_antiaim::fake_duck(bool pre_prediction){
  if(config->hvh.fakeduck_disabled)
    return false;

  c_base_player* localplayer  = utils::localplayer();

  if(localplayer == nullptr)
    return false;

  if(localplayer->move_type == MOVETYPE_NOCLIP || localplayer->move_type == MOVETYPE_LADDER)
    return false;

  if(!config->hvh.fakeduck_on_key){
    if(!(global->original_cmd.buttons & IN_DUCK) && !config->hvh.fakeduck_always || !(localplayer->entity_flags & FL_ONGROUND))
      return false;
  }
  else{
    if(!input_system->held(config->hvh.fakeduck_key))
      return;
  }

  if((global->original_cmd.buttons & IN_DUCK) && (global->original_cmd.buttons & IN_JUMP))
    global->original_cmd.buttons &= ~IN_JUMP;
  else if(global->original_cmd.buttons & IN_JUMP)
    return false;

  global->info_panel->add_entry(INFO_PANEL_ANTIAIM_FAKEDUCK, WXOR(L"FAKE-DUCK"), WXOR(L"ACTIVE"), INFO_PANEL_RAGE_CLR);
  //if(double_tap->has_any_charged_ticks()){
  //  global->info_panel->add_entry(INFO_PANEL_ANTIAIM_FAKEDUCK, WXOR(L"FAKE-DUCK"), WXOR(L"NOT POSSIBLE"), INFO_PANEL_WARNING_CLR);
  //  return false;
  //}

  if(pre_prediction){
    // Strip our duck flags, fake duck handles this for us
    // Having the engine predict us as ducking is not a good idea on the ground
    if(localplayer->entity_flags & FL_ONGROUND)
      global->current_cmd->buttons &= ~IN_DUCK;
  }
  else{

    bool choke_success = set_choke(!localplayer->ducked);

    // if packet_choking is true that means we need to continue choking packets to stay sync'd with the anti-aim.
    // packet_choking will set choke to true if more packets are needed. If we don't do this we can't anti-aim properly.
    if(!localplayer->ducked || !choke_success)
      global->current_cmd->buttons |= IN_DUCK;
    else if(!(localplayer->entity_flags & FL_ONGROUND))
      global->current_cmd->buttons &= ~IN_DUCK;
  }

  return false;
}

bool c_antiaim::get_fake_angle(vec3& fake_angle){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  fake_angle   = (global->engine->get_latency(true) <= math::ticks_to_time(1)) ? global->last_sent_angle : localplayer->viewangles();
  fake_angle.x = math::clamp(fake_angle.x, -90.f, 90.f);
  return true;
}

bool c_antiaim::get_auto_yaw_angle(c_base_player* player, float current_yaw, vec3 target_pos, float radius, std::vector<float> list, float& angle){
  if(player == nullptr || radius < 0.f)
    return false;

  if(!player->is_valid())
    return false;

  s_lc_data* lc = lag_compensation->get_data(player);
  if(lc == nullptr)
    return false;

  s_lc_record* record = lc->get_record(0);
  if(record == nullptr)
    return false;

  vec3 head;
  if(!record->get_hitbox_pos(player, 0, head))
    return false;
  
  vec3 shoot_pos       = player->shoot_pos();
  float area           = math::abs((head - shoot_pos).length_2d()) * radius;
  float points         = 12.f;
  float best_thickness = 0.f;
  bool  hit_player     = false;

  // If there isn't enough angles in the list and then preform a dynamic scan.
  if(list.size() <= 1){
    for(float z = -math::deg_2_rad(180.f); z <= math::deg_2_rad(180.f); z += math::deg_2_rad(90.f)){
      float x           = math::deg_2_rad(math::normalize_angle(current_yaw + math::rad_2_deg(z)));
      vec3  rotated_pos = vec3(area * math::cos(x) + shoot_pos.x, area * math::sin(x) + shoot_pos.y, shoot_pos.z);
      
      s_trace tr1 = global->trace->ray(rotated_pos, target_pos, mask_shot, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
      if(tr1.hit_entity == nullptr){
        hit_player = true;
        continue;
      }
  
      s_trace tr2 = global->trace->ray(target_pos, rotated_pos, mask_shot, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
      if(tr2.hit_entity == nullptr){
        hit_player = true;
        continue;
      }

      float thickness = (tr1.end - tr2.end).length();
      if(thickness >= best_thickness){
        angle          = math::rad_2_deg(x);
        best_thickness = thickness;
      }    
    }
  }
  else{
    // Use the angles with in the list that was given to us.
    for(u32 i = 0; i < list.size(); i++){
      float x           = math::deg_2_rad(math::normalize_angle(current_yaw + list[i]));
      vec3  rotated_pos = vec3(area * math::cos(x) + shoot_pos.x, area * math::sin(x) + shoot_pos.y, shoot_pos.z);

      s_trace tr1 = global->trace->ray(rotated_pos, target_pos, mask_shot, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
      if(tr1.hit_entity == nullptr){
        hit_player = true;
        continue;
      }

      s_trace tr2 = global->trace->ray(target_pos, rotated_pos, mask_shot, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
      if(tr2.hit_entity == nullptr){
        hit_player = true;
        continue;
      }
      
      float thickness = (tr1.end - tr2.end).length();
      if(thickness >= best_thickness){
        angle          = math::rad_2_deg(x);
        best_thickness = thickness;
      }
    }
  }

  return best_thickness > 0.f;
}

void c_antiaim::compute_aim_yaw(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  for(i32 i = 0; i < sizeof(batched_angle) / sizeof(s_batched_angle); i++){
    if(!batched_angle[i].is_batched)
      break;

    float eye_yaw = math::normalize_angle(batched_angle[i].angle.y);
    if(!init_goal_yaw){
      current_goal_yaw = feet_yaw = eye_yaw;
      init_goal_yaw = true;
    }

    if(localplayer->velocity().length() > 1.0f)
      current_goal_yaw = eye_yaw;
    else{
      float yaw_delta = math::normalize_angle(current_goal_yaw - eye_yaw);
      if(math::abs(yaw_delta) > 45.f){
        float side = (yaw_delta > 0.0f) ? -1.f : 1.f;
        current_goal_yaw += 45.f * side;
      }
    }

    current_goal_yaw = math::normalize_angle(current_goal_yaw);
    if(current_goal_yaw != feet_yaw){
      converge_yaw_angles(current_goal_yaw, feet_yaw);
    }
    else{
      DBG("[+] %i: didn't update feet_yaw (eye_yaw: %f, feet_yaw: %f == goal_feet_yaw: %f)\n",  i, eye_yaw, feet_yaw, current_goal_yaw);
    }
  }
  memset(&batched_angle, 0, sizeof(batched_angle));
}

void c_antiaim::converge_yaw_angles(float goal_yaw, float& current_yaw){
  float delta_yaw     = math::normalize_angle(goal_yaw - current_yaw);
  float delta_yaw_abs = math::abs(delta_yaw);
  delta_yaw           = delta_yaw;

  float scale = 1.0f;
  scale = math::clamp(delta_yaw_abs / 60.f, 0.01f, 1.0f); // 60.f = FADE_TURN_DEGREES

  // 720.f = YAW_RATE
  // The highest yaw rate in TF2 is 10.8 degrees per tick.
  float yaw = 720.f * globaldata->interval_per_tick * scale;
  if(delta_yaw_abs < yaw)
    current_yaw = goal_yaw;
  else{
    float side = (delta_yaw < 0.0f) ? -1.f : 1.f;
    current_yaw += yaw * side;
  }

  current_yaw = math::normalize_angle(current_yaw);
}

void c_antiaim::add_to_batch(vec3 angle, bool sending){
  assert(global->client_state != nullptr);
  static i32 last_c = 0;
  i32 c  = global->client_state->choked_commands;
  if(c >= sizeof(batched_angle) / sizeof(s_batched_angle))
    return;

  // Prevent duped entries.
  if(last_c == c){
    DBG("[!] add_to_batch: (%f, %f) : %s (%i) duped entry\n", angle.x, angle.y, sending ? "SENDING" : "CHOKING", c);
    c++;
  }

  last_c = c;

  if(sending){
    batched_angle[c].is_batched = true;
    batched_angle[c].angle      = angle;
  }
  else{
    batched_angle[c].is_batched = true;
    batched_angle[c].angle      = angle;
  }
}