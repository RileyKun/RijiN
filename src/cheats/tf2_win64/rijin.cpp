#include "link.h"

c_rijin_product* rijin = nullptr;

bool c_rijin_product::is_valid(){
  if(!global->valid() || global->unloading)
    return false;

  cookie_block_check_return_val(false);

  return c_base_rijin_source1::is_valid();
}

// This function can be used to test variables that are expected to be at certain values.
bool c_rijin_product::bug_check(){
#if defined(DEV_MODE)
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  DBG("[!] bug_checking - review output for bugs\n");

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  {
    assert(weapon->get_id() >= 0 && weapon->get_id() <= 512);
    assert(weapon->get_slot() >= 0 && weapon->get_slot() <= 33);

    float spread = weapon->get_spread();
    if(spread != spread)
      assert(false && "spread NaN bug check");

    assert(spread >= 0.0f && spread <= 10.f);

    s_trace tr;
    weapon->get_swing_range(&tr);
    DBG("[!] %p\n", tr);

    assert(weapon->get_projectile_speed() >= 0.0f);
    assert(weapon->get_projectile_gravity() >= 0.0f);
  }

  DBG("[!] post weapon\n");

  i32 max_health = localplayer->max_health();
  assert(max_health >= -1);

  vec3 shoot_pos = localplayer->shoot_pos();
  DBG("[!] shoot_pos: %f, %f, %f\n", shoot_pos.x, shoot_pos.y, shoot_pos.z);

  vec3 origin = localplayer->get_abs_origin();
  DBG("[!] abs origin: %f, %f, %f\n", origin.x, origin.y, origin.z);

  vec3 angles = localplayer->get_abs_angles();
  DBG("[!] abs angles: %f, %f, %f\n", angles.x, angles.y, angles.z);

  vec3 old_origin = localplayer->old_origin();
  DBG("[!] old_origin: %f, %f, %f\n", old_origin.x, old_origin.y, old_origin.z);

  assert(localplayer->get_index() >= 0 && localplayer->get_index() <= 8912);

  DBG("[!] is_dormant: %i\n", localplayer->is_dormant());
  DBG("[!] should_draw: %i\n",  localplayer->should_draw());
  DBG("[!] client_class: %p\n", localplayer->get_client_class());

  DBG("[!] bug check done\n");
#endif
  return false;
}

// d3d9
CFLAG_O0 bool c_rijin_product::d3d9_input(c_render_d3d9* render_base){

  bool result = c_base_rijin_source1::d3d9_input(render_base);
  
  // This is for entity data.
  global->memory_manager.process();

  cheat_detection->d3d9_hook();
  return result;
}

CFLAG_O0 bool c_rijin_product::d3d9_hook_game_specific(c_render_d3d9* render_base){
  if(!global->valid())
    return false;

  render_debug->render_objects();
  global->ws2_position_offset = vec3i(0, 0, 0);

  entity_prediction->draw_prediction_track();
  proj_simulate->draw_projectile_trajectory_lines();

  world_esp->run();          
  player_esp->run();

  utils::draw_high_ping_warning();
  misc::draw_fov_circle();
  return false;
}

CFLAG_O0 bool c_rijin_product::d3d9_hook_ui(c_render_d3d9* render_base){
  if(notify == nullptr)
    return false;

  assert(global->menu != nullptr);

  // Not supposed to be changed.
  config->visual.menu_foreground_colour[3] = 210.f;
  config->visual.menu_background_colour[3] = 51.f;

  render_cam->render_visuals_in_camera();

  notify->update_theme(flt_array2clr(config->visual.menu_foreground_colour), flt_array2clr(config->visual.menu_background_colour));
  notify->draw_notifications();

  vec3i m_pos = math::mouse_pos(global->target_window);

  // handle colours (the easy way)
  {
    global->menu->scheme()->main                           = colour(config->visual.menu_foreground_colour);
    global->menu->scheme()->background                     = colour(config->visual.menu_background_colour);
    global->spectator_list_panel->scheme()->main           = colour(config->visual.menu_foreground_colour);
    global->spectator_list_panel->scheme()->background     = colour(config->visual.menu_background_colour);
    //global->fake_latency_panel->scheme()->main           = colour(config->visual.menu_foreground_colour);
    //global->fake_latency_panel->scheme()->background     = colour(config->visual.menu_background_colour);
    global->info_panel->scheme()->main                     = colour(config->visual.menu_foreground_colour);
    global->info_panel->scheme()->background               = colour(config->visual.menu_background_colour);
    global->double_tap_panel->scheme()->main               = colour(config->visual.menu_foreground_colour);
    global->double_tap_panel->scheme()->background         = colour(config->visual.menu_background_colour);
    global->crit_hack_panel->scheme()->main                = colour(config->visual.menu_foreground_colour);
    global->crit_hack_panel->scheme()->background          = colour(config->visual.menu_background_colour);

    global->proj_impact_camera_panel->scheme()->main       = colour(config->visual.menu_foreground_colour);
    global->proj_impact_camera_panel->scheme()->background = colour(config->visual.menu_background_colour);
    global->proj_impact_camera_panel->scheme()->disable_window_background = render_cam->should_render_camera();
  }

  gcs::render_scene(global->info_panel,               m_pos, global->gui_font);
  gcs::render_scene(global->spectator_list_panel,     m_pos, global->gui_font);
  gcs::render_scene(global->double_tap_panel,         m_pos, global->gui_font);
  gcs::render_scene(global->crit_hack_panel,          m_pos, global->gui_font);
  gcs::render_scene(global->proj_impact_camera_panel, m_pos, global->gui_font);
  gcs::render_scene(global->menu,                     m_pos, global->gui_font, global->gui_font_emoji);


  return false;
}

CFLAG_O0 bool c_rijin_product::pre_predicted_create_move(c_base_player* localplayer, c_base_weapon* wep, c_user_cmd* cmd){
  if(cmd == nullptr || localplayer == nullptr || !is_valid() || cmd->command_number == 0)
    return true;

  if(global->speedhack_tick){
    DBG("[+] speedhack tick\n");
  }

  global->update_local_data();
  global->pre_create_move(cmd);
  packet_manager->post_send_packet_init();

  // Call order below does not 'matter'.
  splash_point->generate_base_points();
  seed_prediction->pre_create_move();
  interp_manager->pre_create_move(cmd);
  fake_latency->calculate_fake_ping();
  misc::anti_afk();

  if(config->misc.mvm_revive_use_key && input_system->held(config->misc.mvm_revive_key)){
    if(!localplayer->is_alive())
      misc::force_mvm_revive_response(true);    
  }

  command_manager->pre_create_move(cmd);

  // tf2 only issue.
  engine_prediction->fix_ground_flags();

  // Alive and weapon check
  if(!localplayer->is_alive() || wep == nullptr)
    return true;

  utils::update_active_threats();

  // we need to make sure we've told the server we arent firing anymore at the start of a DT charge
  if(double_tap->is_charging){
    DBG_STAGING("[!] charging...\n");
    cmd->buttons &= ~IN_ATTACK;
    return true;
  }

  global->allow_double_tap = true;

  auto_weapon_swap->think();
  acm->pre_create_move(cmd);

  peek_assist->pre_run();
  fake_latency->update_information_panel();
  
  double_tap->auto_stop();
  misc::anti_afk();
  misc::auto_stop();

  movement->pre_predicted_create_move(localplayer, wep, cmd);
  accuracy->create_move_run(cmd, true);
  auto_rocket_jumper->run();
  antiaim->pre_prediction();
  wrangler_helper->createmove(true);

  crit_hack->fix_primary_attack_bug(true);
  misc::fix_attack_attack2_bug(wep);
  freecam->pre_create_move_prediction(cmd);

  return global->aimbot_settings == nullptr;
}

CFLAG_O0 bool c_rijin_product::on_predicted_create_move(c_base_player* localplayer, c_base_weapon* wep, c_user_cmd* cmd){
  if(cmd == nullptr || localplayer == nullptr || !is_valid() || cmd->command_number == 0)
    return true;

  crit_hack->createmove_run();
  misc::aim_at_backtrack();
  misc::auto_call_medic();
  misc::auto_sticky_spam();

  resolver->process_manual_input();
  auto_vaccinator->run();
  auto_uber->run();
  bool auto_sticky_det_running = auto_sticky->run();
  auto_detonator->run();
  bool reflect_aimbot_running = auto_airblast->run();
  trigger_bot->run();
  auto_shield->run();

  auto_heal_arrow->run();

  bool aimbot_running = (melee_aimbot->run() || projectile_aimbot->run() || raytrace_aimbot->run() || medigun_aimbot->run());
  if(!aimbot_running)
     aimbot_running = auto_repair_aimbot->run();

  if(!aimbot_running){
    if(global->aimbot_target_reset_time <= math::time()){
      global->aimbot_target_type  = -1;
      global->aimbot_target_index = -1;
    }
  }

  misc::auto_zoom();

  bool firing         = localplayer->will_fire_this_tick();
  global->firing_this_tick = firing;
  if((aimbot_running || firing) && !double_tap->shift(aimbot_running)){
    if(config->misc.no_recoil)
      cmd->view_angles -= localplayer->punch_angle();

    if(aimbot_running && wep != nullptr){
      ctf_weapon_info* wep_info = wep->get_weapon_info();

      if(wep_info != nullptr){
        global->fire_angle = cmd->view_angles;
        global->reset_fire_angle_time = math::time() + wep_info->time_fire_delay + math::ticks_to_time(1);
      }

      // handle resolver
      if(firing && raytrace_aimbot->aim_target_hitbox == 0)
        resolver->on_shot(raytrace_aimbot->aim_target);
    }

    if(firing && !double_tap->is_shifting){
      proj_simulate->draw_flight_lines(wep);
    }

    accuracy->create_move_run(cmd, false);
    antiaim->run_on_fire();
    auto_weapon_swap->on_weapon_fired(wep);

    packet_manager->on_input();
  }
  else if((auto_sticky_det_running || reflect_aimbot_running || localplayer->can_fire_secondary()) && !double_tap->shift(aimbot_running)){
    antiaim->run_on_fire();
    packet_manager->on_input();
  }
  else{
    if(global->reset_fire_angle_time <= math::time())
      global->fire_angle = cmd->view_angles;

    fakelag->run();
    antiaim->run();
    misc::anti_backstab();

    // Unchoke when we reload. If we use the auto vaccinator we want this.
    if(wep != nullptr){
      if(wep->is_vaccinator()){
        if(!(global->last_cmd.buttons & IN_RELOAD) && cmd->buttons & IN_RELOAD)
          packet_manager->set_choke(false);
      }
    }
  }

  crit_hack->fix_shared_attack_bug();
  antiaim->fake_duck(false);
  antiaim->break_animations(false);
  auto_vote->run_auto_cast();
  demosystem->createmove();
  misc::auto_disguise();
  misc::infinite_noise_maker_spam();
  misc::auto_rev_jump();
  misc::demo_charge_bot();
  misc::medic_follow_demo_charge();
  misc::auto_sapper();
  misc::auto_huo_longer();
  party_network->create_marker_packet();
  peek_assist->post_run();

  auto_uber->handle_medigun_attack2(wep);
  auto_vaccinator->handle_medigun_attack2(wep);
  auto_vaccinator->perform_cycle();

  crit_hack->fix_primary_attack_bug(false);
  packet_manager->post_create_move_prediction();
  command_manager->create_move(cmd, true);

  wrangler_helper->createmove(false);
  acm->post_create_move(cmd, false);
  math::rebase_movement(cmd, global->original_cmd);
  interp_manager->post_create_move(cmd);
  acm->post_create_move(cmd, true);

  return false;
}

bool c_rijin_product::on_post_prediction_create_move(c_base_player* localplayer, c_base_weapon* wep, c_user_cmd* cmd){
  global->post_create_move(cmd, packet_manager->choking());
  return false;
}

bool c_rijin_product::on_engine_frame(){
  if(c_base_rijin_source1::on_engine_frame() || !global->valid())
    return true;

  interp_manager->update();
  seed_prediction->engine_paint();
  misc::auto_ready_up();

  c_base_player* localplayer = utils::localplayer();
  if(localplayer != nullptr)
    proj_simulate->setup_constant_lines(localplayer);

  render_cam->update_view_matrix();

  for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* entity = global->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    if(entity->is_player()){
      c_player_data* data = entity->get_player()->data();
      if(data == nullptr)
        continue;

      data->obb_min       = entity->obb_mins();
      data->obb_max       = entity->obb_maxs();
      data->transform     = entity->obb_transform();
      data->has_transform = true;
    }
    else{
      c_entity_data* data = entity->entity_data();
      if(data == nullptr)
        continue;

      data->obb_min       = entity->obb_mins();
      data->obb_max       = entity->obb_maxs();
      data->transform     = entity->obb_transform();
      data->has_transform = true;
    }
  }

  return false;
}

void c_rijin_product::on_1000ms(){
  c_base_rijin_source1::on_1000ms();
  cheat_detection->fetch_server_info();
}

void c_rijin_product::on_5000ms(){
  c_base_rijin_source1::on_5000ms();
#if defined(DEV_MODE)
  bug_check();
#endif
}