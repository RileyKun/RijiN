#pragma once

namespace misc{
  inline void anti_afk(){
    if(!config->misc.no_idle_kick)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    float time = math::time();

    // Select a random class every 250ms but only if we have never selected a class.
    if(!utils::is_playing_mvm()){
      if(config->misc.no_idle_kick_auto_pick_class){
        static float next_class_selection_time = 0.f;
        if(next_class_selection_time <= time){
          if(localplayer->player_class() == 0){

            i32 wanted_class = 0;
            for(i32 i = 1; i <= 9; i++){
              if(config->misc.no_idle_kick_auto_pick_class_type[i]){
                wanted_class = i;
                break;
              }
            }
     
            utils::choose_class(wanted_class == 0 ? math::random_int(1, 9) : wanted_class);
            global->last_movement_time = time;
          }

          next_class_selection_time = time + 1.f;
        }
      }
    }

    if(global->current_cmd == nullptr)
      return;

    // There might be times where we don't move at all for a long time.
    if(antiaim->is_faking_yaw()){
      global->last_movement_time = time;
      return;
    }

    // We're manually moving.
    if(global->current_cmd->buttons & (IN_ATTACK | IN_ATTACK2 | IN_ATTACK3 | IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT | IN_JUMP | IN_DUCK | IN_RELOAD)){
      global->last_movement_time = time;
      return;
    }

    if(global->last_movement_time <= 0.f)
      global->last_movement_time = time;

    float delta = math::abs(global->last_movement_time - time);
    if(delta <= 30.f)
      return;

    global->info_panel->add_entry(INFO_PANEL_ANTI_AFK, WXOR(L"ACTIVE"), INFO_PANEL_RAGE_CLR);

    /*
      If the buttons are the same from last tick it won't update the last action timer.
    */
    global->current_cmd->buttons |= IN_FORWARD;
    static bool swap = false;
    global->current_cmd->buttons = swap ? IN_MOVERIGHT : IN_MOVELEFT;

    // This can be detected by server anti-cheats. So only do this on valve servers.
    if(utils::is_match_making_server()){
      if(swap)
        global->current_cmd->buttons |= (1 << 27); // If we don't do this. We'll get kicked from valve servers.
    }

    swap = !swap;

    // We want to slowly move in a tiny circle. Kinda like how our anti-aim does it.
    static float next_move_circle_time;

    if(next_move_circle_time <= time){
      static float spin                  = 0.f;
      global->original_cmd.view_angles.x = 89.f;
      global->original_cmd.view_angles.y = spin;
      global->current_cmd->move.x = 1.25f;
      spin += 12.f;
      if(spin > 180.f)
        spin = -180.f;

      next_move_circle_time = time + 5.f;
    }
  }

  inline void auto_rev_jump(){
    if(!config->misc.auto_rev_jump)
      return;

    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr)
      return;

    if(!localplayer->is_player_class(TF_CLASS_HEAVY))
      return;

    c_base_weapon* weapon = localplayer->get_weapon();

    if(weapon == nullptr)
      return;

    if(!weapon->is_minigun() || weapon->weapon_state() != 0)
      return;

    if(global->current_cmd->buttons & IN_ATTACK2)
      global->current_cmd->buttons |= IN_JUMP;
  }

  // TODO: move me to medic feature related class
  inline void uber_on_active_charge(i32 client){
    if(!config->automation.medic_uber_active_charge || client == global->localplayer_index)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_alive())
      return;

    if(!localplayer->is_player_class(TF_CLASS_MEDIC))
      return;

    c_base_weapon* weapon = localplayer->get_weapon();
    if(weapon == nullptr)
      return;

    if(!weapon->can_uber())
      return;

    c_base_player* heal_target = weapon->get_heal_target();
    if(heal_target == nullptr)
      return;

    if(!heal_target->is_player())
      return;

    if(config->automation.medic_uber_active_charge_friendsonly){
      if(!heal_target->is_steam_friend())
        return;
    }

    if(heal_target->get_index() == client){

      global->auto_vaccinator_prefer_resist = 0;
      if(config->automation.medic_uber_active_charge_resist_blast)
        global->auto_vaccinator_prefer_resist = 1;
      else if(config->automation.medic_uber_active_charge_resist_fire)
        global->auto_vaccinator_prefer_resist = 2;

      global->createmove_force_attack2 = true;
    }
  }

  inline void auto_call_medic(){
    if(!config->automation.auto_call_medic)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_alive())
      return;

    if(localplayer->health() >= localplayer->max_health())
      return;

    if(localplayer->is_cloaked() || localplayer->is_ubercharged() || localplayer->is_over_healed())
      return;

    bool is_disguised = localplayer->is_disguised();

    // Check if someone is healing us
    bool is_someone_healing_me = false;
    i32  medics_found          = 0;
    for(i32 i = 0; i <= globaldata->max_clients; i++){
      c_base_player* entity = global->entity_list->get_entity(i);

      if(entity == nullptr || entity == localplayer)
        continue;

      if(entity->is_dormant()|| !entity->is_alive())
        continue;

      // If we aren't disguised, only call on our team medics
      if(!is_disguised && entity->team() != localplayer->team())
        continue;

      if(!entity->is_player_class(TF_CLASS_MEDIC))
        continue;

      medics_found++;

      c_base_weapon* entity_wep = entity->get_weapon();

      if(entity_wep == nullptr)
        continue;

      if(!entity_wep->is_medigun())
        continue;

      if(entity_wep->get_heal_target() != localplayer)
        continue;

      is_someone_healing_me = true;
      break;
    }

    // Someone is healing us, no need
    if(is_someone_healing_me)
      return;

    // No medics found
    if(medics_found <= 0)
      return;

    global->info_panel->add_entry(INFO_PANEL_AUTO_MEDIC, WXOR(L"ACTIVE"), INFO_PANEL_LEGIT_CLR);

    static float last_medic_call_time = 0.f;
    float        current_time = math::time(false);

    // Only call a medic every few seconds
    if(last_medic_call_time > current_time)
      return;

    last_medic_call_time = current_time + 5.f;

    global->engine->client_cmd(XOR("voicemenu 0 0"));

    DBG("[!] Calling medic (medics_found: %i)\n", medics_found);
  }

  // Used for Player ESP.
  inline void process_heal_event(i32 patient, i32 amount){
    patient             = global->engine->get_player_for_user_id(patient);

    c_player_data* data = utils::get_player_data(patient);
    if(data == nullptr)
      return;

    data->hp_healed         = amount;
    data->hp_healed_timeout = math::time(false) + 1.5f;
  }

  inline void anti_backstab(){
    if(!config->misc.anti_backstab)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_alive())
      return;

    if(localplayer->is_bonked() || localplayer->is_ubercharged() || localplayer->has_condition(TF_COND_SHIELD_CHARGE) || localplayer->has_condition(TF_COND_HALLOWEEN_KART))
      return;

    if(utils::is_truce_active())
      return;

    c_base_weapon* localplayer_wep = localplayer->get_weapon();
    if(localplayer_wep == nullptr)
      return;

    if(localplayer_wep->is_sticky_launcher() && global->current_cmd->buttons & IN_ATTACK2)
      return;

    // TODO: check each of our backtrack records to see if they are too close to any of them
    i32             closest_distance  = 0;
    c_base_player*  found_target      = nullptr;
    for(i32 i = 1; i <= globaldata->max_clients; i++){
      c_base_player* player = global->entity_list->get_entity(i);
      if(player == nullptr)
        continue;

      if(!player->is_valid())
        continue;

      if(player->team() == localplayer->team())
        continue;

      c_base_weapon* weapon = player->get_weapon();
      if(weapon == nullptr)
        continue;

      if(!weapon->is_knife() && weapon->weapon_id() != WPN_Backburner && weapon->weapon_id() != WPN_BackScatter)
        continue;

      if(player->is_cloaked())
        continue;

      if(player->is_steam_friend() && config->misc.anti_backstab_ignore_friends)
        continue;

      float latency      = utils::get_latency() + globaldata->interval_per_tick;
      vec3 predicted_pos = utils::trace_line(player->obb_center(), player->obb_center() + (player->velocity() * latency));

      i32 distance = localplayer->obb_center().distance_meter(predicted_pos);
      if(distance > 8)
        continue;

      if(found_target != nullptr && distance >= closest_distance)
        continue;

      s_trace trace = global->trace->ray(localplayer->shoot_pos(), predicted_pos, mask_bullet);
      if(!trace.vis(player))
        continue;

      //TODO Make this FAKED!
      global->current_cmd->view_angles.y = math::calc_view_angle(localplayer->shoot_pos(), predicted_pos).y;

      found_target      = player;
      closest_distance  = distance;
      global->info_panel->add_entry(INFO_PANEL_ANTI_BACKSTAB, WXOR(L"ACTIVE"), INFO_PANEL_RAGE_CLR);
    }
  }

  inline void auto_disguise(){
    if(!config->automation.auto_disguise)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_valid())
      return;

    if(!localplayer->is_player_class(TF_CLASS_SPY))
      return;

    static float next_disguise = 0.f;
    float time  = math::time(false);
    float delay = math::clamp(config->automation.auto_disguise_delay, 200.f, 5000.f);
    if(localplayer->is_disguised() || localplayer->is_carrying_objective()){
      next_disguise = time + (delay / 1000.f);
      return;
    }

    if(utils::attrib_hook_int(0, XOR("mod_disguise_consumes_cloak"), localplayer, 0, 1) && localplayer->cloak_meter() < 100.f)
      return;

    global->info_panel->add_entry(INFO_PANEL_AUTO_DISGUISE, WXOR(L"ACTIVE"));
    if(next_disguise > time)
      return;

    next_disguise = time + (delay / 1000.f);
    global->engine->client_cmd(XOR("lastdisguise"));
  }

  inline void infinite_noise_maker_spam(bool on_kill = false){
    if(!config->misc.infinite_noisemaker && !on_kill || !config->misc.infinite_noisemaker_on_kill && on_kill)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(localplayer->has_canteen_equipped())
      return;

    if(localplayer->get_equip_wearable_for_loadout_slot(9) == nullptr)
      return;

    global->info_panel->add_entry(INFO_PANEL_NOISEMAKER, WXOR(L"ACTIVE"), INFO_PANEL_RAGE_CLR);

    float time             = math::time();
    static float next_time = 0.f;
    if(next_time > time)
      return;

    // Prevent us from picking up weapons on the ground.
    // Will make it not run again for 1 second if we hit a weapon on the ground.
    {
      vec3 fwd;
      math::angle_2_vector(global->untouched_cmd.view_angles, &fwd, nullptr, nullptr);
      vec3 pos = (fwd * 256.f) + localplayer->shoot_pos();
      s_trace tr = global->trace->ray(localplayer->shoot_pos(), pos, mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_HIT_WEAPONS);


      if(tr.hit_entity != nullptr){
        // If we're looking at a dropped weapon then don't run.
        if(tr.hit_entity->get_client_class()->id == CTFDroppedWeapon){
          DBG("[!] (1) Not running infinite noisemaker because we could mistakenly pickup a weapon.\n");
          next_time = time + 1.0f;
          return;
        }
        else{
          // Okay, check to see if we're looking near a dropped weapon.
          for(i32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
            c_base_entity* entity = global->entity_list->get_entity(i);
            if(entity == nullptr)
              continue;

            if(entity->is_dormant())
              continue;

            if(entity->get_client_class()->id != CTFDroppedWeapon)
              continue;

            if((entity->obb_center() - localplayer->obb_center()).length() > 256.f)
              continue;

            if(math::get_fov_delta(global->untouched_cmd.view_angles, localplayer->shoot_pos(), entity->obb_center()) > 15.f)
              continue;

            DBG("[!] (2) Not running infinite noisemaker because we could mistakenly pickup a weapon.\n");
            next_time = time + 1.0f;
            return;
          }
        }
      }
    }

    next_time = time + 0.1f;
    c_keyvalues* kv = c_keyvalues::create(XOR("use_action_slot_item_server"));
    if(kv != nullptr){
      kv->transmit();

      DBG("[!] Sent infinite noise maker spam\n");
    }
  }

  inline void force_mvm_revive_response(bool bypass_revive_config_check = false){
    if(!utils::is_playing_mvm())
      return;

    if(!bypass_revive_config_check && !config->misc.mvm_revive_always)
      return;

    DBG("[!] Forcing mvm revive response.\n");
    c_keyvalues* kv = c_keyvalues::create(XOR("MVM_Revive_Response"));

    if(kv != nullptr){
      kv->set_int(XOR("accepted"), 1);
      kv->transmit();
      global->info_panel->add_entry(INFO_PANEL_MVM_REVIVE, WXOR(L"ACTIVE"), INFO_PANEL_RAGE_CLR);
    }
  }

  inline void medic_follow_demo_charge(){
    if(!config->automation.medic_follow_demo_charge)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_alive())
      return;

    if(!localplayer->is_player_class(TF_CLASS_MEDIC))
      return;

    c_base_weapon* weapon = localplayer->get_weapon();
    if(weapon == nullptr)
      return;

    if(weapon->weapon_id() != WPN_QuickFix)
      return;

    assert(global->aimbot_settings != nullptr);
    c_base_player* heal_target = weapon->get_heal_target();
    if(heal_target == nullptr)
      return;

    // Only run on players since players are the only ones that can charge.
    if(!heal_target->is_player())
      return;

    if(!heal_target->has_condition(TF_COND_SHIELD_CHARGE))
      return;

    // Find a lag compensated position to follow.
    s_lc_data* lc = lag_compensation->get_data(heal_target);
    if(lc == nullptr)
      return;

    for(u32 ii = 0; ii < lc->max_records; ii++){
      s_lc_record* record = lc->get_record(ii);
      if(record == nullptr)
        continue;

      // No lag compensation valid check. Simply because we don't care. It's all about movement.

      s_trace trace = global->trace->ray(localplayer->shoot_pos(), record->obb_center, mask_bullet);
      if(!trace.vis(heal_target))
        continue;

      vec3 aim_angle                      = math::calc_view_angle(localplayer->shoot_pos(), record->obb_center);
      global->current_cmd->view_angles.y  = aim_angle.y;

      global->info_panel->add_entry(INFO_PANEL_CHARGE_BOT_MEDIC_FOLLOW, WXOR(L"ACTIVE"), INFO_PANEL_RAGE_CLR);
      break;
    }
  }

  inline void demo_charge_bot(){
    if(!config->automation.demo_charge_bot)
      return;

    if(utils::is_truce_active())
      return;

    if(config->automation.demo_charge_bot_use_key){
      if(!input_system->held(config->automation.demo_charge_bot_key))
        return;
    }

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_alive())
      return;

    if(!localplayer->has_condition(TF_COND_SHIELD_CHARGE))
      return;

    c_base_weapon* weapon = localplayer->get_weapon();
    if(weapon == nullptr)
      return;

    if(!weapon->is_melee())
      return;

    global->info_panel->add_entry(INFO_PANEL_CHARGE_BOT, WXOR(L"ACTIVE"), INFO_PANEL_RAGE_CLR);
    assert(global->aimbot_settings != nullptr);

    c_base_player* target = nullptr;
    i32 best_dist         = 0;
    vec3 best_pos;
    for(i32 i = 1; i <= globaldata->max_clients; i++){
      c_base_player* player = global->entity_list->get_entity(i);
      if(player == nullptr)
        continue;

      if(!player->is_valid())
        continue;

      // Avoid trying to target people who are too high.
      float height = math::abs(localplayer->origin().z - player->origin().z);
      if(height >= 16.f)
        continue;

      i32 dist = localplayer->origin().distance_meter(player->origin());
      if(target != nullptr && dist >= best_dist)
        continue;

      // Never target the team no matter what is_considered_enemy says.
      if(!player->is_player() || !melee_aimbot->should_target_entity(player))
        continue;

      if(player->is_bonked() || player->is_ubercharged())
        continue;

      s_lc_data* lc = lag_compensation->get_data(player);
      if(lc == nullptr)
        continue;

      vec3 pos;
      for(u32 ii = 0; ii < lc->max_records; ii++){
        s_lc_record* record = lc->get_record(ii);
        if(record == nullptr)
          continue;

        if(!record->valid())
          continue;

        // If we dont want to use history, then only use the first valid tick
        if(!acm->can_back_track() || !global->aimbot_settings->position_adjust_history)
          ii = lc->max_records;

        if(global->aimbot_settings->max_fov > 0 && global->aimbot_settings->max_fov < 180){
          if(math::get_fov_delta(global->untouched_cmd.view_angles, localplayer->shoot_pos(), record->obb_center) > (float)global->aimbot_settings->max_fov)
            continue;
        }

        s_trace trace = global->trace->ray(localplayer->shoot_pos(), record->obb_center, mask_bullet);
        if(!trace.vis(player))
          continue;

        target    = player;
        best_dist = dist;
        best_pos  = record->obb_center;
        break;
      }
    }

    // The demo shield movement is based on our yaw angles sadly.
    if(target != nullptr){
       vec3 aim_angle                      = math::calc_view_angle(localplayer->shoot_pos(), best_pos);
       global->current_cmd->view_angles.y  = aim_angle.y;
    }
  }

  inline void aim_at_backtrack(){
    if(!config->automation.backtrack_to_crosshair)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    c_base_weapon* weapon = localplayer->get_weapon();
    if(weapon == nullptr)
      return;

    // Not lag compensated at all / properly.
    if(weapon->is_projectile_weapon() || weapon->is_harmless_item())
      return;

    global->info_panel->add_entry(INFO_PANEL_BACKTRACK_TO_CROSSHAIR, WXOR(L"INACTIVE"));

    i32   wanted_ticks = -1;
    i32   wanted_entity = -1;
    float best_fov   = 25.f;
    float best_dist  = 100.f;
    for(i32 index = 1; index <= globaldata->max_clients; index++){
      c_base_player* player = global->entity_list->get_entity(index);
      if(player == nullptr)
        continue;

      if(!player->is_valid())
        continue;

      // Ignore the friendly team if we aren't using the medigun.
      if(player->team() == localplayer->team() && !weapon->is_medigun())
        continue;

      s_lc_data* lc = lag_compensation->get_data(player);
      if(lc == nullptr)
        continue;

      vec3 pos;
      for(u32 i = 0; i < lc->max_records; i++){
        s_lc_record* record = lc->get_record(i);
        if(record == nullptr)
          continue;

        if(!record->valid())
          continue;

        if(weapon->is_melee() || weapon->is_medigun()){
          float dist = localplayer->obb_center().distance(record->obb_center);
          if(dist >= best_dist)
            continue;
        }

        float fov = math::get_fov_delta(global->untouched_cmd.view_angles, localplayer->shoot_pos(), record->obb_center);
        if(fov >= best_fov)
          continue;

        best_fov = fov;
        wanted_ticks  = math::time_to_ticks(record->simulation_time);
        wanted_entity = index;
      }
    }

    if(wanted_ticks != -1){
      global->info_panel->add_entry(INFO_PANEL_BACKTRACK_TO_CROSSHAIR, WXOR(L"ACTIVE"));
      interp_manager->set_tick_count_with_interp((u32)wanted_ticks, wanted_entity, wanted_ticks > 0);
    }
  }

  inline void auto_sapper(){
    if(!config->automation.auto_sapper)
      return;

    if(utils::is_truce_active())
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_valid())
      return;

    if(!localplayer->is_player_class(TF_CLASS_SPY))
      return;

    if(localplayer->is_cloaked(false))
      return;

    c_base_weapon* weapon = localplayer->get_weapon();
    if(weapon == nullptr)
      return;

    if(!weapon->is_sapper())
      return;

    global->info_panel->add_entry(INFO_PANEL_AUTO_SAPPER, WXOR(L"RUNNING"));
    i32            robots    = 0;
    i32            best_dist = 4;
    c_base_entity* target    = nullptr;
    for(i32 index = utils::is_playing_mvm() ? 1 : globaldata->max_clients + 1; index <= global->entity_list->get_highest_index(); index++){
      c_base_entity* entity = global->entity_list->get_entity(index);
      if(entity == nullptr)
        continue;

      if(entity->is_dormant())
        continue;

      if(entity->team() == localplayer->team() || entity->team() == 0)
        continue;

      i32 dist = localplayer->origin().distance_meter(entity->origin());
      if(entity->is_player()){
        if(!config->automation.auto_sapper_target_robots)
          continue;

        c_base_player* player = (c_base_player*)entity;
        if(!player->is_valid())
          continue;

        if(player->has_condition(TF_COND_SAPPED))
          continue;

        if(player->is_ubercharged() || player->is_bonked())
          continue;

        if(dist <= 4)
          robots++;
      }
      else{
        if(!entity->is_sentry() && !entity->is_dispenser() && !entity->is_teleporter())
          continue;

        if(entity->is_sentry() && !config->automation.auto_sapper_target_sentry)
          continue;

        if(entity->is_dispenser() && !config->automation.auto_sapper_target_dispenser)
          continue;

        if(entity->is_teleporter() && !config->automation.auto_sapper_target_teleporter)
          continue;

        if(entity->sapped())
          continue;
      }

      if(best_dist > 0 && dist >= best_dist)
        continue;

      s_trace trace = global->trace->ray(localplayer->shoot_pos(), entity->obb_center(), mask_shot);
      if(!trace.vis(entity))
        continue;

      target    = entity;
      best_dist = dist;
    }

    // Sap this!
    if(target != nullptr){
      if(target->is_player() && robots < config->automation.auto_sapper_min_robots)
        return;

      global->current_cmd->view_angles = math::calc_view_angle(localplayer->shoot_pos(), target->obb_center());
      global->current_cmd->buttons |= IN_ATTACK;
    }
  }

  inline void auto_sticky_spam(){
    if(!config->automation.auto_sticky_spam)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    c_base_weapon* weapon = localplayer->get_weapon();
    if(weapon == nullptr)
      return;

    if(!weapon->is_sticky_launcher())
      return;

    if(weapon->is_blast_jumper_weapon())
      return;

    if(config->automation.auto_sticky_spam_use_key){
      if(!input_system->held(config->automation.auto_sticky_spam_key))
        return;
    }
  
    if(config->automation.auto_sticky_spam_auto_release_near_entity){
      if(weapon->charge_begin_time() <= 0.f)
        return;

      // If we don't find any entities and this is true then we'll run the 'release at charge logic'
      bool should_release_at = true;
      for(u32 i = 1; i <= global->entity_list->get_highest_index(); i++){
        c_base_entity* entity = global->entity_list->get_entity(i);
        if(entity == nullptr)
          continue;

        if(entity->is_dormant())
          continue;

        // If auto sticky is enabled then we'll copy those settings.
        if(auto_sticky->is_active(false)){

          if(!auto_sticky->is_target_entity(entity, localplayer, weapon))
            continue;
        }
        else{
          if(!projectile_aimbot->should_target_entity(entity))
            continue;
        }

        if(localplayer->origin().distance_meter(entity->origin()) >= 65)
          continue;

        // Only consider entities with in a FOV range.
        if(math::get_fov_delta(global->untouched_cmd.view_angles, localplayer->shoot_pos(), entity->obb_center()) > 45.f)
          continue;

        should_release_at = false;
        break;
      }

      float charge_percent = math::clamp((globaldata->cur_time - weapon->charge_begin_time()) / weapon->get_sticky_launcher_max_charge() * 100.f, 0.f, 100.f);
      
      s_projectile_simulation_context ctx(weapon, global->current_cmd->view_angles, true);
      for(u32 i = 0; i < math::time_to_ticks(10.f); i++){
        if(!proj_simulate->predict_tick(&ctx))
          break;
      }

      if(!ctx.valid)
        return;

      vec3 sticky_pos = ctx.tr.end;
  
      // Before doing any vischecks and damage calculation code the game does this to the position of the explosion.
      {
        sticky_pos.z += 8.f;
        s_trace tr1 = global->trace->ray(sticky_pos, sticky_pos + vec3(0.f, 0.f, -32.f), mask_shot_hull, TRACE_EVERYTHING, nullptr);
        if(tr1.fraction != 1.0f)
          sticky_pos = tr1.end + (tr1.plane.normal * 1.f);
      }

      // A crude version of the auto sticky bomb radius checks.
      float radius     = weapon->get_blast_radius();
      radius *= (auto_sticky->get_blast_radius_ratio() / 100.f);

      c_entity_sphere_query sphere(sticky_pos, radius);
      for(u32 i = 0; i < sphere.list_count; i++){
        c_base_entity* entity = sphere.list[i];
        if(entity == nullptr)
          continue;

        if(entity->is_dormant())
          continue;

        // If auto sticky is enabled then we'll copy those settings.
        if(auto_sticky->is_active(false)){
          if(!auto_sticky->is_target_entity(entity, localplayer, weapon))
            continue;
        }
        else{
          if(!projectile_aimbot->should_target_entity(entity))
            continue;
        }

        vec3 nearest_point;
        entity->calc_nearest_point(sticky_pos, &nearest_point);
        float dist_sqr           = (sticky_pos - nearest_point).length_sqr();
        if(dist_sqr > (radius * radius))
          continue;

        if(!auto_sticky->is_visible(localplayer, entity, nullptr, sticky_pos, nearest_point))
          continue;
        
        if(config->automation.auto_sticky_spam_set_charge_release_at)
          config->automation.auto_sticky_spam_at_charge = (i32)charge_percent;

        global->info_panel->add_entry(INFO_PANEL_AUTO_STICKY_RELEASE, XOR(L"ENTITY FOUND"));
        global->current_cmd->buttons &= ~IN_ATTACK;
        return;  
      }

      if(!should_release_at)
        return;
    }
  
    global->info_panel->add_entry(INFO_PANEL_AUTO_STICKY_RELEASE, XOR(L"RELEASE AT"));
    float max_charge     = weapon->get_sticky_launcher_max_charge() * (float)math::clamp(config->automation.auto_sticky_spam_at_charge, 1, 100) / 100.f;
    float current_charge = (globaldata->cur_time - weapon->charge_begin_time());
    if(weapon->charge_begin_time() > 0.f && current_charge >= max_charge)
      global->current_cmd->buttons &= ~IN_ATTACK;
    
  }

  inline void auto_stop(){
    if(!config->hvh.auto_stop)
      return;

    // This has its own auto stop.
    if(double_tap->is_shifting || double_tap->is_charging)
      return;

    // Only run when we aren't moving.
    if(global->current_cmd->buttons & IN_FORWARD || global->current_cmd->buttons & IN_BACK || global->current_cmd->buttons & IN_MOVELEFT || global->current_cmd->buttons & IN_MOVERIGHT
      || global->current_cmd->buttons & IN_JUMP)
      return;

    c_base_player* localplayer  = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_alive())
      return;

    if(!(localplayer->entity_flags() & FL_ONGROUND) || localplayer->move_type() != MOVETYPE_WALK)
      return;

    vec3 vel = localplayer->velocity();
    vel.z = 0.f;

    float speed = vel.length_2d();
    if(speed < 30.f){
      global->current_cmd->move.x = 0.f;
      global->current_cmd->move.y = 0.f;
      return;
    }

    vec3 negative_dir = (vel * -1.f);
    vec3 negative_ang;
    math::vector_2_angle(&negative_dir, &negative_ang);

    negative_ang.y = global->current_cmd->view_angles.y - negative_ang.y;
    math::angle_2_vector(negative_ang, &negative_dir, nullptr, nullptr);
    negative_dir.normalize();

    global->current_cmd->move.x = negative_dir.x * 450.f;
    global->current_cmd->move.y = negative_dir.y * 450.f;
  }

  inline void show_hitbox_on_hit(i32 attacker, i32 attacked, i32 weaponid, bool death){
    if(!config->visual.show_hitboxes_on_hit)
      return;

    if(attacker != global->localplayer_index)
      return;

    if(attacker == attacked)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    c_base_weapon* wep = localplayer->get_weapon_from_belt(weaponid);
    if(wep == nullptr)
      return;

    ctf_weapon_info* weapon_info = wep->get_weapon_info();
    if(weapon_info == nullptr)
      return;

    if(wep->is_melee() || wep->is_projectile_weapon())
      utils::draw_bounding_box(attacked, death ? colour(config->visual.show_hitboxes_on_hit_colour) * 0.5f : colour(config->visual.show_hitboxes_on_hit_colour), death ? 4.f + weapon_info->time_fire_delay : weapon_info->time_fire_delay);
    else
      utils::draw_hitbox(attacked, death ? colour(config->visual.show_hitboxes_on_hit_colour) * 0.5f : colour(config->visual.show_hitboxes_on_hit_colour), death ? 4.f + weapon_info->time_fire_delay : weapon_info->time_fire_delay);
  }

  inline void auto_huo_longer(){
    if(!config->automation.auto_conserve_hlh_ammo)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_valid())
      return;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return;

    if(!wep->is_huo_long_heater())
      return;

    global->info_panel->add_entry(INFO_PANEL_HUO_LONG_HEATER, WXOR(L"CONSERVING AMMO"), INFO_PANEL_RAGE_CLR);
    if(wep->weapon_state() == 2 || global->current_cmd->buttons & IN_ATTACK){ // We're currently firing our minigun. Don't do anything!
      global->info_panel->add_entry(INFO_PANEL_HUO_LONG_HEATER, WXOR(L"FIRING"), INFO_PANEL_WARNING_CLR);
      return;
    }

    // If we're holding our DT key down. Then chances are we're ready to peek a target of some sort.
    // There is no check to see if we can dt because at this state it would always return false.
    if(double_tap->has_any_charged_ticks()){
      if(!config->misc.double_tap_shift_on_key)
        return;

      if(input_system->held(config->misc.double_tap_shift_hotkey)){
        global->info_panel->add_entry(INFO_PANEL_HUO_LONG_HEATER, WXOR(L"WANTS TO DT"), INFO_PANEL_WARNING_CLR);
        return;
      }
    }

    // Predict our movement by our latency.
    float delay = utils::get_latency();
    delay += math::ticks_to_time(1);

    s_entity_predict_ctx ctx(localplayer, EP_FLAG_PREDICT_YAW_TURN);
    entity_prediction->simple(&ctx, math::time_to_ticks(delay));

    vec3  origin = ctx.predicted_origin;
    vec3  center = ctx.predicted_origin + (ctx.predicted_obb_mins + ctx.predicted_obb_maxs) * 0.5f;
    float radius = wep->get_blast_radius();

    radius *= (float)math::clamp(config->automation.auto_conserve_hlh_range_mult, 1, 16);
    c_entity_sphere_query sphere(origin, radius * 2.f);
    for(i32 i = 0; i < sphere.list_count; i++){
      c_base_entity* entity = sphere.list[i];
      if(entity == nullptr)
        continue;

      if(entity->is_dormant())
        continue;

      // If we're near a payload cart or dispenser then don't run.
      if((entity->is_payload_dispenser() || entity->is_dispenser()) && entity->team() == localplayer->team()){
        if(origin.distance_meter(entity->origin()) <= 4){
          s_trace tr = global->trace->ray(center, entity->obb_center(), mask_solid_brushonly);
          if(!tr.vis(entity))
            continue; // Only if we can see it.

          global->info_panel->add_entry(INFO_PANEL_HUO_LONG_HEATER, WXOR(L"AMMO SOURCE"), INFO_PANEL_LEGIT_CLR);
          return;
        }
      }

      if(entity->team() == localplayer->team())
        continue;

      // Should we react to sentries and maybe stickies?
      if(!entity->is_player() && !entity->is_sentry())
        continue;

      if(entity->is_player()){
        c_base_player* player = entity->get_player();
        if(player == nullptr)
          continue;

        if(!player->is_alive())
          continue;

        if(!localplayer->is_considered_enemy(player))
          continue;

        // Don't run on uber-charged players.
        if(player->is_ubercharged())
          continue;
      }

      vec3 pos;
      entity->calc_nearest_point(origin, &pos);
      if((origin - pos).length_sqr() > (radius * radius))
        continue;

      vec3 mins = entity->obb_mins();
      vec3 maxs = entity->obb_maxs();

      // A check the game does to see if the fire will actually hit the entity.
      if( !(origin.z > entity->origin().z + mins.z - 32.f && origin.z < entity->origin().z + maxs.z))
        continue;

      // Can the fire ring see the player? Should we do this?
      s_trace tr = global->trace->ray(center, entity->obb_center(), mask_solid_brushonly, TRACE_EVERYTHING, localplayer);
      if(!tr.vis(entity))
        continue;

      global->info_panel->add_entry(INFO_PANEL_HUO_LONG_HEATER, WXOR(L"ENTITY DETECTED"), INFO_PANEL_LEGIT_CLR);
      return;
    }

    if(wep->weapon_state() > 1)
      global->current_cmd->buttons &= ~IN_ATTACK2;
  }

  inline void fix_attack_attack2_bug(c_base_weapon* wep){
    assert(global->current_cmd != nullptr);
    if(wep == nullptr)
      return;

    if(!wep->is_rocket_launcher() && !wep->is_man_melter() && !wep->is_bison())
      return;

    if(global->current_cmd->buttons & IN_ATTACK && global->current_cmd->buttons & IN_ATTACK2)
      global->current_cmd->buttons &= ~IN_ATTACK2;
  }

  // We placed this in here rather than utils.
  inline void process_condition_prediction(c_base_player* player, u32 condition, bool heal_target = false){
    assert(condition >= 0 && condition < TF_COND_COUNT);
    if(player == nullptr)
      return;

    c_player_data* data = player->data();
    if(data == nullptr)
      return;

    bool  resist      = false;
    float ping        = math::ticks_to_time(math::time_to_ticks(utils::get_latency()));
    float expire_time = -1.f;
    if(condition >= TF_COND_MEDIGUN_UBER_BULLET_RESIST && condition <= TF_COND_MEDIGUN_UBER_FIRE_RESIST || condition == TF_COND_HALLOWEEN_QUICK_HEAL)
      expire_time = 3.0f;
    else if(condition == TF_COND_PHASE || condition == TF_COND_ENERGY_BUFF) // Inside tf_player.cpp
      expire_time = (100.f / 12.5/*tf_scout_energydrink_consume_rate*/) + 1.f;

    // Update the player's expire time.
    if(expire_time > 0.f){
      data->tfdata.condition_active_time[condition] = (math::time() + expire_time) - ping;
      resist                                        = true;
    }

    // Update the resist timers on their heal target.
    // We do this since the AddCond function isn't called when the other player has the condition set.
    if(resist && !heal_target){
      auto_heal_arrow->knowledge_vaccinator_charge_released(player, condition);
      if(player->is_player_class(TF_CLASS_MEDIC)){
        c_base_weapon* wep = player->get_weapon();
        if(wep == nullptr)
          return;
  
        c_base_player* heal_target = wep->get_heal_target();
        if(heal_target != nullptr)
          process_condition_prediction(heal_target, condition, true);
      }
    }
  }

  inline void draw_fov_circle(){
    if(global->aimbot_settings == nullptr)
      return;

    if(!global->aimbot_settings->enabled || !global->aimbot_settings->target_method_closest_to_fov || !utils::is_in_game())
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return;

    if(!wep->is_wrangler()){
      if(wep->is_harmless_item() || wep->is_blast_jumper_weapon())
        return;
    }

    i32   alpha   = (i32)config->visual.aimbot_fov_circle_colour[3];
    float fov_rad = (float)global->aimbot_settings->max_fov / global->game_fov * (render->screen_size.x * 0.5f);
  
    if(fov_rad > 0.f){
      if(config->visual.aimbot_fov_circle_enabled){
        if(config->visual.aimbot_fov_circle_outlines){
          render->outlined_circle(vec3i(render->screen_size.x * 0.5f, render->screen_size.y * 0.5f), fov_rad + 1.f, 150.f, rgba(0, 0, 0, alpha));
          render->outlined_circle(vec3i(render->screen_size.x * 0.5f, render->screen_size.y * 0.5f), fov_rad - 1.f, 150.f, rgba(0, 0, 0, alpha));
        }
  
        render->outlined_circle(vec3i(render->screen_size.x * 0.5f, render->screen_size.y * 0.5f), fov_rad, 150.f, flt_array2clr(config->visual.aimbot_fov_circle_colour));
      }
    }
  }

  inline void auto_zoom(){
    if(!config->automation.auto_rezoom)
      return;

    if(config->automation.auto_rezoom_when_moving){
      if(global->current_cmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT | IN_JUMP | IN_DUCK))
        return;
    }

    if(config->automation.auto_rezoom_on_aimbot){
      if(global->aimbot_settings->aim_mode_on_key && !input_system->held(global->aimbot_settings->aim_hotkey))
        return;

      if(global->aimbot_settings->aim_mode_on_attack && !(global->current_cmd->buttons & IN_ATTACK))
        return;
    }

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(localplayer->is_scoped() || localplayer->is_jumping())
      return;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return;

    if(!wep->is_sniper_rifle())
      return;

    if(wep->next_secondary_attack() > globaldata->cur_time)
      return;

    global->current_cmd->buttons |= IN_ATTACK2;
  }

  inline void auto_ready_up(){
    if(!config->automation.mvm_auto_ready_up)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_alive())
      return;

    if(!utils::is_in_setup())
      return;

    if(!utils::is_playing_mvm())
      return;

    if(utils::is_player_ready(localplayer->get_index()))
      return;

    static float next_run_time = 0.f;
    float time = math::time();
    if(next_run_time > time)
      return;

    next_run_time = time + 0.15f;

    global->engine->client_cmd(XOR("tournament_player_readystate 1"));
  }
};