#include "../link.h"

CLASS_ALLOC(c_fakelag, fakelag);

void c_fakelag::run(){
  static c_key_control key(&config->hvh.fakelag_key, &config->hvh.fakelag_key_held, &config->hvh.fakelag_key_toggle, &config->hvh.fakelag_key_double_click);
  bool is_fakelag_enabled = !config->hvh.fakelag_usekey || config->hvh.fakelag_usekey && key.is_toggled();
  find_headshot_classes();
  if(!active())
    return;

  anti_backtrack(); // We want to always run no matter what key is pressed or fakelag method is set.
  peek_fakelag();
  desync_sequences();
  if(!config->hvh.fakelag)
    return;

  if(!run_method())
    return;

  global->info_panel->add_entry(INFO_PANEL_FAKELAG, WXOR(L"INACTIVE"));
  if(!headshot_threat && config->hvh.fakelag_headshotdanger){
    global->info_panel->add_entry(INFO_PANEL_FAKELAG, WXOR(L"NO THREAT"), is_fakelag_enabled ? INFO_PANEL_RAGE_CLR : colour(255, 255, 255));
    return;
  }

  if(double_tap->has_any_charged_ticks()){
    global->info_panel->add_entry(INFO_PANEL_FAKELAG, WXOR(L"NOT POSSIBLE"), INFO_PANEL_WARNING_CLR);
    return;
  }

  if(!is_fakelag_enabled)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  if(global->client_state->choked_commands()<= 0){

    float len         = localplayer->velocity().length_2d();
    bool moving       = len > 0.1f;
    ticks_to_break_lc = moving ? math::clamp((i32)(((64.f / globaldata->interval_per_tick) / len) + 0.5f), 0, math::time_to_ticks(1.f)) : math::smallest(packet_manager->get_max_choke_ticks(), config->hvh.fakelag_maxchoke);
    if(moving){ // Add on random ticks when moving.
      math::set_random_seed((globaldata->tick_count ^ global->current_cmd->buttons) + (i32)global->current_cmd->view_angles.y);
      ticks_to_break_lc += math::random_int(1, math::biggest(packet_manager->get_max_choke_ticks() - ticks_to_break_lc, 2));
    }

    if(!config->hvh.fakelag_adaptive)
      ticks_to_break_lc = config->hvh.fakelag_maxchoke;

    // Make sure it doesn't go above our fakelag limit the user set.
    ticks_to_break_lc = math::smallest(ticks_to_break_lc, config->hvh.fakelag_maxchoke);
    if(double_tap->has_any_charged_ticks() && ticks_to_break_lc > 3)
      ticks_to_break_lc = 3;
  }

  if(ticks_to_break_lc != 0){
    wchar_t buf[IDEAL_MIN_BUF_SIZE];
    formatW(buf, WXOR(L"CHOKING %i %s"), ticks_to_break_lc, ticks_to_break_lc > 1 ? WXOR(L"TICKS") : WXOR(L"TICK"));
    global->info_panel->add_entry(INFO_PANEL_FAKELAG, buf, INFO_PANEL_RAGE_CLR);
  }

  // always set the last packet as choke false
  // since if we let the engine choke the last packet the server will invalidate all our commands
  packet_manager->set_choke((global->client_state->choked_commands()+ 1) <= ticks_to_break_lc);
}

void c_fakelag::anti_backtrack(){
  if(!config->hvh.fakelag_anti_backtrack)
    return;

  if(!can_fakelag())
    return;
  
  global->info_panel->add_entry(INFO_PANEL_ANTIBACKTRACK_FAKELAG,  WXOR(L"INACTIVE"), INFO_PANEL_WARNING_CLR);

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  s_lc_data* lc = lag_compensation->get_data(localplayer);
  if(lc == nullptr)
    return; 

  // Before I used to get latency to ticks. That was stupid because people will likely have access to much more records than that.
  s_lc_record* record      = lc->get_record(0);
  s_lc_record* last_record = lc->get_record(1);
  if(record == nullptr || last_record == nullptr){
    DBG("[!] c_fakelag::anti_backtrack: we should have no backtrackable records\n");
    return;
  }

  vec3 furthest_origin = record->origin;
  for(u32 i = 0; i < lc->max_records; i++){
    s_lc_record* r = lc->get_record(i);
    if(r == nullptr)
      continue;

    if(!r->valid())
      continue;

    furthest_origin = r->origin;
  }

  vec3 origin      = record->origin;
  vec3 shoot_pos   = localplayer->shoot_pos();
  vec3 pos         = last_record->obb_center;
  vec3 last_origin = last_record->origin;

  if(!global->anti_backtrack_choke){
    for(u32 i = 1; i <= globaldata->max_clients; i++){
      if(i == global->localplayer_index)
        continue;
  
      c_base_player* player = global->entity_list->get_entity(i);
      if(player == nullptr)
        continue;
  
      // There's alot of server-side trace / lag comp issues when trying to friendly fire people so lets just ignore this.
      if(player->team() == global->localplayer_team)
        continue;
  
      c_player_data* data = player->data();
      if(data == nullptr)
        continue;
  
      c_base_weapon* wep = player->get_weapon();
      if(!player->is_valid() || !localplayer->is_considered_enemy(player) || wep == nullptr){
        anti_backtrack_fail(player);
        continue;
      }
  
       // The player is holding a weapon that isn't lag compensated or does no damage.
      if(wep->is_projectile_weapon() || wep->is_flamethrower() || wep->is_harmless_item() || wep->is_medigun() || wep->is_wrangler()){
        anti_backtrack_fail(player);
        continue;
      }
  
      s_lc_data* lc_data = lag_compensation->get_data(player);
      if(lc_data == nullptr){
        anti_backtrack_fail(player);
        continue;
      }
  
      vec3 old_abs_origin = player->get_abs_origin();
  
      if(wep->is_melee()){ // Handle logic against players holding melee weapons.
        for(i32 j = 1; j < lc_data->max_records - 1; j++){
          s_lc_record* last_record    = lc_data->get_record(j - 1);
          s_lc_record* current_record = lc_data->get_record(j);
          if(last_record == nullptr || current_record == nullptr)
            continue;

          // Is this a good idea? What if they have extended backtrack on? Or does this automatically adjust for this.
          if(!last_record->valid() || !current_record->valid())
            continue;

          // Ignore the player if they aren't moving towards us.
          if(furthest_origin.distance(last_record->origin) >= furthest_origin.distance(current_record->origin) && origin.distance(last_record->origin) >= origin.distance(current_record->origin))
            break; // Changed from continue to a break. If we hit this then we don't care about older positions being closer to us because they aren't THERE anymore.

          float range = wep->get_swing_range(true) * 4.f;
          if((current_record->origin - furthest_origin).length() > range && (current_record->origin - origin).length() > range)
            continue;

          player->set_abs_origin(current_record->origin);
          s_trace tr = global->trace->ray(shoot_pos, current_record->obb_center, mask_shot & ~(0x40000000), TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
          player->set_abs_origin(old_abs_origin);

          if(tr.vis(player)){
            DBG("[+] c_fakelag::anti_backtrack (anti-melee)\n");
            data->seen_localplayer = true;
            break;
          }
          // End of melee check.
        }   
      }
      else{ // Handle non melee weapons.
        s_trace tr1 = global->trace->ray(vec3(furthest_origin.x, furthest_origin.y, furthest_origin.z + 45.f), player->shoot_pos(), mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
        s_trace tr2 = global->trace->ray(shoot_pos, player->shoot_pos(), mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
        
        if(tr1.vis(player) || tr2.vis(player)){
          DBG("[+] c_fakelag::anti_backtrack (anti-hitscan)\n");
          data->seen_localplayer = true;

          if(tr2.vis(player))
            continue;
        }
      }

      // A player that saw us no longer sees us anymore!
      if(data->seen_localplayer){
        DBG("[!] c_fakelag::anti_backtrack (perform)\n");
        global->anti_backtrack_choke         = true;
        global->anti_backtrack_choke_started = true;
        break;
      }
    }
  }
  else{

    // Reset seen by boolean
    for(u32 i = 1; i <= globaldata->max_clients; i++){
      c_base_player* player = global->entity_list->get_entity(i);
      if(player == nullptr)
        continue;

      c_player_data* data = player->data();
      if(data == nullptr)
        continue;

      data->seen_localplayer = false;
    }

    global->info_panel->add_entry(INFO_PANEL_ANTIBACKTRACK_FAKELAG, WXOR(L"ACTIVE"), INFO_PANEL_RAGE_CLR);


    global->anti_backtrack_choke_started = false;
    bool choking = (global->client_state->choked_commands()+ 1 < packet_manager->get_max_choke_ticks());
    packet_manager->set_choke(choking);

    float delta = (origin - last_origin).length_sqr_2d() / (float)math::biggest(1, math::time_to_ticks(math::abs(localplayer->simulation_time() - localplayer->old_simulation_time())));
    if(delta > 4096.f || !choking){
      packet_manager->set_choke(false);
      packet_manager->force_send_packet(false);
      global->anti_backtrack_choke = false;
    }
  }

}

void c_fakelag::desync_sequences(){
  if(!config->hvh.fakelag_on_sequence_changed)
    return;

  // We're using double tap or we have charge ready. Do not run code.
  if(double_tap->is_shift_tick() || double_tap->has_any_charged_ticks())
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  if(localplayer->is_taunting() || localplayer->has_condition(TF_COND_HALLOWEEN_KART))
    return;

  static i32  last_sequence  = localplayer->sequence();
  static bool active         = false;
  if(!active){

    // If our animation sequence has changed then choke 1 batch of commands.
    if(last_sequence != localplayer->sequence()){
       DBG("[!] sequence desync forcing send..\n");
       packet_manager->force_send_packet(false);
       active = true;
    }
  }
  else{
    if((global->client_state->choked_commands()+ 1) < packet_manager->get_max_choke_ticks())
      packet_manager->set_choke(true);
    else{
      active = false;
      DBG("[!] sequence desync finished.\n");
    }
  }


  last_sequence = localplayer->sequence();
}

void c_fakelag::peek_fakelag(){
  if(!config->hvh.fakelag_peek)
    return;

  // We're using double tap or we have charge ready. Do not run code.
  if(double_tap->is_shift_tick() || double_tap->has_any_charged_ticks())
    return;

  if(global->anti_backtrack_choke)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  if(localplayer->is_taunting() || localplayer->has_condition(TF_COND_HALLOWEEN_KART))
    return;

  float delay = utils::get_latency();
  delay += math::ticks_to_time(4);

  s_entity_predict_ctx ctx(localplayer, EP_FLAG_PREDICT_YAW_TURN);
  entity_prediction->simple(&ctx, math::time_to_ticks(delay));

  if(global->peek_fakelag_choke){
    global->info_panel->add_entry(INFO_PANEL_PEEK_FAKELAG, WXOR(L"ACTIVE"), INFO_PANEL_RAGE_CLR);
    if(global->peek_fakelag_choke_started){
      if(global->client_state->choked_commands()> 0){
        packet_manager->force_send_packet(false);
        DBG("[!] peek-fakelag sending off this tick.\n");
      }
      global->peek_fakelag_choke_started = false;
      return;
    }

    // Can we see anybody still?
    bool seen = false;
    for(i32 index = 1; index <= globaldata->max_clients; index++){
      c_base_player* player = global->entity_list->get_entity(index);
      if(player == nullptr)
        continue;

      if(!player->is_valid())
        continue;

      if(!localplayer->is_considered_enemy(player))
        continue;

      s_trace tr = global->trace->ray(localplayer->shoot_pos(), player->shoot_pos(), mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
      if(!tr.vis(player))
        continue;

      seen = true;
      break;
    }

    bool choking = (global->client_state->choked_commands()+ 1 < packet_manager->get_max_choke_ticks());
    packet_manager->set_choke(choking);

    if(global->client_state->choked_commands()> 0 || !seen){
      if(!choking){ // No longer choking... Cancel!
        DBG("[!] peek-fakelag finished with %i choked ticks | seen: %i\n", global->client_state->choked_commands(), seen);
        global->peek_fakelag_choke = false;
      }
    }

  }
  else{

    for(i32 index = 1; index <= globaldata->max_clients; index++){
      if(index == global->engine->get_local_player())
        continue;

      c_base_player* player = global->entity_list->get_entity(index);
      if(player == nullptr)
        continue;

      if(!player->is_valid())
        continue;

      if(!localplayer->is_considered_enemy(player))
        continue;

      vec3 shoot_pos = ctx.predicted_origin + localplayer->view_offset();
      s_trace tr = global->trace->ray(shoot_pos, player->shoot_pos(), mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
      if(!tr.vis(player))
        continue;

      s_trace tr2 = global->trace->ray(localplayer->shoot_pos(), player->shoot_pos(), mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
      if(tr2.vis(player))
        continue;

      DBG("[!] peek fakelag has started!\n");
      global->peek_fakelag_choke         = true;
      global->peek_fakelag_choke_started = true;
      break;
    }
  }
}

void c_fakelag::find_headshot_classes(){
  headshot_threat            = false;
  if(!config->hvh.fakelag || !config->hvh.fakelag_headshotdanger)
    return;

  headshot_threat = global->headshot_threats.get_threats(config->hvh.fakelag_headshotdanger_cheatersonly, config->hvh.fakelag_headshotdanger_vischeck) > 0;
}

bool c_fakelag::active(){
  return !utils::is_playing_mvm() && !utils::is_truce_active() && !double_tap->is_shifting && double_tap->shift_end + math::time_to_ticks(0.5f) < global->original_cmd.command_number;
}

bool c_fakelag::run_method(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  if(localplayer->is_taunting() || localplayer->has_condition(TF_COND_HALLOWEEN_KART))
    return false;

  if(config->hvh.fakelag_method_interval)
    return true;

  // Pretty basic right now. But the idea is it won't run when you aren't moving.
  if(config->hvh.fakelag_method_movement){
    if(global->original_cmd.buttons & IN_JUMP || global->original_cmd.buttons & IN_FORWARD || global->original_cmd.buttons & IN_BACK || global->original_cmd.buttons & IN_MOVERIGHT || global->original_cmd.buttons & IN_MOVELEFT)
      return true;

    if(global->current_cmd->buttons & IN_JUMP || global->current_cmd->buttons & IN_FORWARD || global->current_cmd->buttons & IN_BACK || global->current_cmd->buttons & IN_MOVERIGHT || global->current_cmd->buttons & IN_MOVELEFT)
      return true;
  }

  return false;
}

void c_fakelag::anti_backtrack_fail(c_base_player* player){
  if(player == nullptr)
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  data->seen_localplayer = false;
}

// If true we can do any fakelag based feature otherwise don't do any feature.
bool c_fakelag::can_fakelag(){
  // If this is a shift tick then by no mean ever fakelag.
  if(double_tap->is_shift_tick() || double_tap->has_any_charged_ticks())
    return false;

  // Server is taking a shit. Lets stop fakelagging for accuracy.
  if(utils::is_server_lagging())
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(localplayer->is_taunting() || localplayer->has_condition(TF_COND_HALLOWEEN_KART))
    return false;

  if(!localplayer->is_valid())
    return false;

  // There's no reason todo any fakelag feature when playing mvm or in truce mode. Fakelag also doesn't have any affect against tf_bots.
  if(utils::is_truce_active() || utils::is_playing_mvm())
    return false;

  return true;
}