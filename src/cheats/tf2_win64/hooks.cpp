#include "link.h"

EXPORT CFLAG_O0 void __fastcall create_move_hook(void* rcx, c_user_cmd* cmd){
  assert(rcx != nullptr);
  assert(cmd != nullptr);

  c_base_player* localplayer  = utils::localplayer();
  c_base_weapon* wep = nullptr;
  if(localplayer != nullptr)
    wep = localplayer->get_weapon();

  if(rijin->pre_predicted_create_move(localplayer, wep, cmd)){
    DBG_STAGING("[-] condition met in cm, not running.\n");
    return;
  }

  if(!engine_prediction->start_prediction(localplayer, cmd)){
    DBG_STAGING("[!] pred not possible, canceling cm.\n");
    packet_manager->post_create_move_prediction();
    return;
  }

  rijin->on_predicted_create_move(localplayer, wep, cmd);
  engine_prediction->end_prediction(cmd);
  rijin->on_post_prediction_create_move(localplayer, wep, cmd);
}

EXPORT void __fastcall override_view_hook(void* rcx, s_view_setup* view_setup){
  assert(view_setup != nullptr);

  // Had to do global->localplayer = localplayer here because CreateMove isn't called when playing demos. - Rud
  c_base_player* localplayer = utils::localplayer();
  global->update_local_data();
  global->view_setup         = view_setup;
 
  if(localplayer != nullptr){
    c_base_weapon* wep = localplayer->get_weapon();

    config->visual.custom_fov_slider = math::clamp(config->visual.custom_fov_slider, 90, 140);
    if(config->visual.no_visual_recoil){
      view_setup->angles -= localplayer->punch_angle();
      view_setup->angles.z = 0.f;
    }

    bool is_scoped = localplayer->has_condition(TF_COND_ZOOMED);

    // If we're spectating somebody in first person that is scoped in as sniper then don't run our Custom FOV to avoid possibly destroying the Zoom FOV.
    c_base_player* local_spec_player = localplayer->get_spectate_target();
    if(local_spec_player != nullptr){
      if(local_spec_player->has_condition(TF_COND_ZOOMED) && localplayer->observer_mode() == 4)
        is_scoped = true;
    }

    if(config->visual.no_zoom_fov && is_scoped || !is_scoped){
      view_setup->fov  = (float)config->visual.custom_fov_slider;
      localplayer->fov() = (float)config->visual.custom_fov_slider; // Required to fix the sensitivity.
    }
  }

  global->game_fov = view_setup->fov;
  utils::call_fastcall64<void, s_view_setup*>(gen_internal->decrypt_asset(global->override_view_hook_trp), rcx, view_setup);
}

EXPORT void __fastcall engine_paint_hook(void* rcx, u32 mode){
  assert(rcx != nullptr);
  rijin->on_engine_frame();
  utils::call_fastcall64<void, u32>(gen_internal->decrypt_asset(global->engine_paint_hook_trp), rcx, mode);
}

EXPORT void* __fastcall crender_worldtoscreen_matrix_hook(void* rcx){
  return &global->view_matrix_untouched;
}

EXPORT void __fastcall paint_traverse_hook(void* rcx, u64 panel, bool repaint, bool force_frame){
  assert(rcx != nullptr);

  i8* panel_name = global->panel->get_name(panel);
  assert(panel_name != nullptr);

  u32 panel_hash = HASH_RT(panel_name);
  if(privacy_mode->paint_traverse_hook(panel))
    return;

  global->paint_traverse(panel_hash, panel);
  if(global->hud_scope_id == 0 && panel_hash == HASH("HudScope"))
    global->hud_scope_id = panel;

  if(config->visual.no_zoom_enabled && global->hud_scope_id == panel) // No zoom - Rud
    return;

  utils::call_fastcall64<void, u64, bool, bool>(gen_internal->decrypt_asset(global->paint_traverse_hook_trp), rcx, panel, repaint, force_frame);
}

EXPORT void __fastcall frame_stage_notify_hook(u32 stage){
  if(!utils::is_in_game() || gen_internal->has_sub_expired()){
    utils::call_fastcall64_raw<void, u32>(gen_internal->decrypt_asset(global->frame_stage_notify_hook_trp), stage);
    return;
  }

  entity_info->think(stage);
  utils::verify_player_data();
  utils::verify_entity_data();
  player_list_handler->think();
  player_list_handler->verify_selected_player();

  utils::calculate_sniper_damage(stage);

  if(global->spectator_list_panel != nullptr)
    global->spectator_list_panel->fetch_spectators(stage);

  target_list->frame_stage_notify(stage);
  lag_compensation->frame_stage_notify(stage);
  cheat_detection->frame_stage_notify(stage);
  antiaim->thirdperson(stage);
  resolver->frame_stage_notify_run(stage);
  dormant_manager->send_party_player_data(stage);
  wrangler_helper->frame_stage_notify(stage);

  utils::call_fastcall64_raw<void, u32>(gen_internal->decrypt_asset(global->frame_stage_notify_hook_trp), stage);
  bullet_tracers->frame_stage_notify_run(stage);

  utils::update_player_information(stage);
}

EXPORT void __fastcall check_for_pure_server_whitelist_hook(void* crc){
  if(config->misc.pure_bypass){
    DBG("[!] bypassed sv_pure check\n");
    return;
  }

  utils::call_fastcall64_raw<void, void*>(gen_internal->decrypt_asset(global->check_for_pure_server_whitelist_hook_trp), crc);
}

EXPORT u32 __fastcall get_direct_ping_to_data_center_hook(void* rcx, u32 pop_id, void* unk1){
  assert(rcx != nullptr);

  STACK_CHECK_START;
  u32 ping = utils::call_fastcall64<u32, u32, void*>(gen_internal->decrypt_asset(global->get_direct_ping_to_data_center_hook_trp), rcx, pop_id, unk1);
  STACK_CHECK_END;

  return fast_queue->think(ping, pop_id);
}

// Unused.
#if defined(UNUSED)
EXPORT u32 __fastcall get_direct_ping_to_pop_hook(void* rcx, void* unk, u32 pop_id){
  assert(rcx != nullptr);

  STACK_CHECK_START;
  u32 ping = utils::call_fastcall64<u32, void*, u32>(gen_internal->decrypt_asset(global->get_direct_ping_to_pop_hook_trp), rcx, unk, pop_id);
  STACK_CHECK_END;

  return fast_queue->think(ping, pop_id);
}
#endif

EXPORT i32 __fastcall player_resource_get_team_hook(void* rcx, i32 player_index){
  assert(rcx != nullptr);

  if(config->misc.reveal_scoreboard && __builtin_return_address(0) == global->get_team_check_scoreboard_retaddr1){
    if(global->localplayer_team != 0)
      return global->localplayer_team;
  }

  return utils::call_fastcall64<i32, i32>(gen_internal->decrypt_asset(global->player_resource_get_team_trp), rcx, player_index);
}

EXPORT i32 __fastcall player_panel_get_team_hook(void* rcx){
  assert(rcx != nullptr);

  if(config->misc.reveal_scoreboard && __builtin_return_address(0) == global->get_team_status_panel_retaddr1){
    if(global->localplayer_team != 0)
      return global->localplayer_team;
  }

  return utils::call_fastcall64<i32>(gen_internal->decrypt_asset(global->player_panel_get_team_trp), rcx);
}

EXPORT bool __fastcall is_crit_boosted_hook(void* rcx){
  assert(rcx != nullptr);

  //if( __builtin_return_address(0) == global->is_crit_boosted_material_retaddr1 ||
  //    __builtin_return_address(0) == global->update_crit_boost_effect_is_crit_boost_retaddr1){
  //  return true;
  //}

  return utils::call_fastcall64<bool>(gen_internal->decrypt_asset(global->is_crit_boosted_hook_trp), rcx);
}

EXPORT bool __fastcall ctfplayershared_incondition_hook(s_shared* rcx, u32 condition){
  assert(rcx != nullptr);
  assert(global->ctfplayer_createmove_retaddr1 != nullptr);
  assert(global->ctfplayer_createmove_retaddr2 != nullptr);
  assert(global->ctfplayer_createmove_retaddr3 != nullptr);
  assert(global->shoulddraw_viewmodel_retaddr1 != nullptr);
  assert(global->ctfplayer_shoulddraw_retaddr1 != nullptr);
  assert(global->ctfwearable_shoulddraw_retaddr1 != nullptr);
  assert(global->cdisguisestatus_shoulddraw_retaddr != nullptr);
  assert(global->cdisguisestatus_paint_retaddr != nullptr);
  assert(global->ctfhudplayerclass_onthink_retaddr != nullptr);
  assert(global->ctfhudplayerclass_update_model_panel_retaddr != nullptr);
  assert(global->load_itempreset_incond_retaddr != nullptr);
  void*          ret_addr = __builtin_return_address(0);
  c_base_player* player   = (c_base_player*)rcx->outer;

  // Allow us to change loadouts while taunting.
  if(ret_addr == global->load_itempreset_incond_retaddr)
    return false;

  if(config->misc.reveal_spy_models){
    if(ret_addr != global->cdisguisestatus_paint_retaddr && ret_addr != global->cdisguisestatus_shoulddraw_retaddr && ret_addr != global->ctfhudplayerclass_update_model_panel_retaddr && ret_addr != global->ctfhudplayerclass_onthink_retaddr){
      if(rcx != global->localplayer_shared || config->misc.reveal_spy_models_include_localplayer){
        // Fix the "CRIT!" effect not showing on disguised players.
        if(ret_addr == global->ctfplayer_fireevent_retaddr1 || condition == TF_COND_DISGUISED)
          return false;
      }
    }
  }

  if(config->misc.remove_taunt_restrictions && ret_addr == global->ctfplayer_createmove_retaddr1 || config->misc.remove_cage_restrictions && ret_addr == global->ctfplayer_createmove_retaddr3)
    return false;

  if(config->misc.remove_kart_restrictions){
    if(ret_addr == global->ctfplayer_createmove_retaddr2 || ret_addr == global->ctfinput_applymouse_retaddr1)
      return false;
  }

  // Make it so our sniper can see their hats and stuff. - Rud
  if(ret_addr == global->ctfwearable_shoulddraw_retaddr1 || ret_addr == global->ctfplayer_shoulddraw_retaddr1 || config->visual.no_zoom_enabled && ret_addr == global->shoulddraw_viewmodel_retaddr1)
    return false;

  // Remove taunting animations from players.
  if(condition == TF_COND_TAUNTING){
    if(player != nullptr){
      if((config->misc.disable_taunts || resolver->is_resolver_active(player)) && player->team() != global->localplayer_team)
        return false;
    }
  }

  return utils::call_fastcall64<bool, u32>(gen_internal->decrypt_asset(global->ctfplayershared_incondition_trp), rcx, condition);
}

EXPORT float __fastcall ctfplayer_geteffectiveinvislevel_hook(c_base_player* rcx){
  assert(rcx != nullptr);

  if(chams->override_cloak_effective(rcx))
    return 0.f;

  // Show cloaked spies.
  if(config->misc.reveal_spy_models && config->misc.reveal_spy_models_remove_cloak){
    if(config->misc.reveal_spy_models_remove_clock_always || !rcx->is_cloak_blinking() && config->misc.reveal_spy_models_remove_clock_ignore_flicker)
      if(rcx != global->localplayer || config->misc.reveal_spy_models_include_localplayer)
        return 0.f;
  }

  return utils::call_fastcall64<float>(gen_internal->decrypt_asset(global->ctfplayer_geteffectiveinvislevel_trp), rcx);
}

EXPORT bool __fastcall ctfplayer_isplayerclass_hook(void* rcx, u32 player_class){
  assert(rcx != nullptr);

  // Fix damage text not showing on cloaked / disguised spies.
  if(config->visual.combat_text_enabled && config->visual.combat_text_force_show_spies){
    if(__builtin_return_address(0) == global->displaydmgfeedback_retaddr1)
      return false;
  }

  // So our trick with MedicGetHealTarget will work in DisplayDamageFeedback
  if(config->visual.combat_text_enabled && config->visual.combat_text_show_team_mate_dmg){
    if(__builtin_return_address(0) == global->displaydmgfeedback_isplayerclass_retaddr)
      return true;
  }

  return utils::call_fastcall64<bool, u32>(gen_internal->decrypt_asset(global->ctfplayer_isplayerclass_hook_trp), rcx, player_class);
}

EXPORT void __fastcall engine_trace_ray_hook(void* rcx, s_ray& ray, i32 mask, c_trace_filter* filter, s_trace* tr){
  assert(rcx != nullptr);

  // Make the damage text seen through walls.
  if(config->visual.combat_text_enabled && config->visual.combat_text_show_thru_walls){
    if(__builtin_return_address(0) == global->displaydmgfeedback_retaddr4){
      tr->hit_entity = nullptr;
      tr->fraction   = 1.0f;
      return;
    }
  }

  utils::call_fastcall64<void, s_ray&, i32, c_trace_filter*, s_trace*>(gen_internal->decrypt_asset(global->engine_trace_ray_hook_trp), rcx, ray, mask, filter, tr);
}

EXPORT void __fastcall medigun_itempostframe_hook(c_base_weapon* rcx){
  assert(rcx != nullptr);

  c_base_player* player = rcx->get_owner();

  if(player == nullptr){
    utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->medigun_itempostframe_hook_trp), rcx);
    return;
  }

  bool reload = rcx->reload_button_pressed();
  i32 buttons = player->buttons();

  utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->medigun_itempostframe_hook_trp), rcx);

  auto_vaccinator->medigun_itempostframe_run(player, rcx, reload, buttons);

  // Fix heal speeds
  {
    c_base_player* heal_target = rcx->get_heal_target();
    static bool was_healing = false;

    if(heal_target != nullptr){
      float max_player_speed = heal_target->get_max_speed();
      if(max_player_speed > player->max_speed())
        player->max_speed() = max_player_speed;

      was_healing = true;
    }
    else if(was_healing){
      player->max_speed() = player->get_max_speed();
      was_healing = false;
    }
  }
}

EXPORT void __fastcall maintain_sequence_transition_hook(void* rcx, void* bone_setup, void* cycle, void* pos, void* q){
  assert(rcx != nullptr);
  if(lag_compensation->is_setting_up_bones)
    return;

  utils::call_fastcall64<void, void*, void*, void*, void*>(gen_internal->decrypt_asset(global->maintain_sequence_transition_hook_trp), rcx, bone_setup, cycle, pos, q);
}

EXPORT void __fastcall float_decode_hook(s_decode_info* info){
  utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->float_decode_hook_trp), info);

  if(info == nullptr)
    return;

  if(info->send_prop == nullptr)
    return;

  if(info->send_prop->var_name == nullptr)
    return;

  c_base_entity* entity = info->struct_base;
  if(entity == nullptr)
    return;

  u32 name_hash = HASH_RT(info->send_prop->var_name);

  cheat_detection->on_float_decode(entity, name_hash, info->data);
  if(name_hash == HASH("m_angEyeAngles[0]"))
    resolver->float_decode(entity, *(float*)info->data, false);
  else if(name_hash == HASH("m_angEyeAngles[1]"))
    resolver->float_decode(entity, *(float*)info->data, true);
  if(name_hash == HASH("m_flObservedCritChance")){
    float observed_crit_chance = *(float*)info->data;

    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr)
      return;

    c_base_weapon* localplayer_wep = localplayer->get_weapon();

    if((c_base_weapon*)entity != localplayer_wep && (c_base_player*)entity != localplayer)
      return;

    crit_hack->find_tfstat_damage_ranged_crit_random(observed_crit_chance);
  }
}

EXPORT void __fastcall int_decode_hook(s_decode_info* info){
  utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->int_decode_hook_trp), info);

  if(info == nullptr)
    return;

  if(info->send_prop == nullptr)
    return;

  if(info->send_prop->var_name == nullptr)
    return;

  c_base_entity* entity = info->struct_base;
  if(entity == nullptr)
    return;

  u32 name_hash = HASH_RT(info->send_prop->var_name);
  if(name_hash == HASH("m_nChargeResistType")){
    auto_vaccinator->int_decode_run(entity, info->data);
  }
  else if(name_hash == HASH("m_bViewingCYOAPDA")){
    *(i32*)info->data = 0;
  }
  else if(name_hash == HASH("m_iHealth") && entity->is_player()){
    c_player_data* data = ((c_base_player*)entity)->data();
    if(data != nullptr){
      i32 new_health = *(i32*)info->data;

      if(new_health > 1)
        data->health_before_death = new_health;
    }
  }
  // COMMENT: Dec 30th 2022 - This is no longer needed since we have found a full fix for this.
  // Fix the server telling us we are in the air when we are on the ground
  // Happens because when the server networks us changed flags it also includes FL_ONGROUND
  // This is bad because if it tries to tell us we are in the air after we hit the ground
  // Our entire client will mess up until we can jump again
  /*else if(name_hash == HASH("m_fFlags") && entity == localplayer){
    u32 original_flags = *(i32*)info->data;

    if(!(original_flags & FL_ONGROUND)){
      u32 correct_flags  = original_flags & ~FL_ONGROUND;

      if(localplayer->entity_flags() & FL_ONGROUND){
        DBG("[!] FL_ONGROUND FIXED\n");
        correct_flags |= FL_ONGROUND;
      }

      *(i32*)info->data = correct_flags;
    }
  }*/
}

// This function will jumping issues caused by the function hook above.
EXPORT float __fastcall cl_client_interp_amount_hook(void* rcx){
  return 0.0f;
}

EXPORT void __fastcall ctfplayer_avoidplayers_hook(void* rcx, void* cmd){
  if(config->misc.nopush)
    return;

  utils::call_fastcall64<void, void*>(gen_internal->decrypt_asset(global->ctfplayer_avoidplayers_hook_trp), rcx,  cmd);
}

// This hook will stop the drawing of players econ items (not effects however)
EXPORT bool __fastcall ctfwearable_shoulddraw_hook(c_base_entity* rcx){
  assert(rcx != nullptr);

  if(config->misc.remove_cosmetics){
    // dont even ask how this is working
    // the offset 0x75C is found inside this function.
    c_base_entity* owner       = (c_base_entity*)global->entity_list->get_entity_handle(*(uptr*)((uptr)rcx + (uptr)0x75C));
    c_base_player* localplayer = utils::localplayer();

    if(owner != nullptr && localplayer != nullptr){
      // Exclude ourself.
      if(owner == localplayer && !config->misc.remove_cosmetics_localplayer)
        return utils::call_fastcall64<bool>(gen_internal->decrypt_asset(global->ctfwearable_shoulddraw_hook_trp), rcx);

      // Exclude our friends man.
      if(owner->is_steam_friend() && !config->misc.remove_cosmetics_friends)
        return utils::call_fastcall64<bool>(gen_internal->decrypt_asset(global->ctfwearable_shoulddraw_hook_trp), rcx);

      return false;
    }
  }

  return utils::call_fastcall64<bool>(gen_internal->decrypt_asset(global->ctfwearable_shoulddraw_hook_trp), rcx);
}

EXPORT bool __fastcall fire_event_hook(void* rcx, c_game_event* event){
  assert(event != nullptr);
  assert(event->get_name() != nullptr);

  cheat_detection->on_fire_event(event);

  u32 name_hash = HASH_RT(event->get_name());
  if(name_hash == HASH("party_chat")){

    i8* id   = event->get_string(XOR("steamid"), XOR("0"));
    i8* text = event->get_string(XOR("text"));
    i32 type = event->get_int(XOR("type"));
    /*
      types:
      1000 = joined party chat
      1001 = left party chat
      1002 = send failed party chat
      1003 = now online
      1004 = now offline
    */

    if(party_network->on_party_chat_message(text, type))
      return false;

    // Discord webhook support
    {
      std::string type_str = "";
      switch(type){
        default: break;
        case 1001:
        {
          type_str = XOR("Left");
          break;
        }
        case 1000:
        {
          type_str = XOR("Joined");
          break;
        }
        case 1003:
        {
          type_str = XOR("Online");
          break;
        }
        case 1004:
        {
          type_str = XOR("Offline");
          break;
        }
      }
    }

    // Only run anti-spam if we're dealing not connected since this will be the time when people can "crash" us.
    if(!utils::is_in_game()){
      static i32   messages       = 0;
      static float reset_messages = 0.f;
      float time                  = math::time();

      if(reset_messages <= time){
        messages = 0;
        reset_messages = time + 1.0f;
      }

      //Limit 3 messages per second for all chat messages.
      messages++;
      if(messages >= 3)
        return false;
    }
  }
  else if(name_hash == HASH("server_spawn") || name_hash == HASH("client_beginconnect")){
    cheat_detection->reset();
    interp_manager->reset();
    global->reset();
    crit_hack->reset();
    crit_hack->reset_damages();
    double_tap->reset();
    acm->reset();
    fake_latency->reset();
    thirdperson->reset();
    freecam->reset();
    auto_vote->reset();
    seed_prediction->reset();
    splash_point->reset();
    notify->clear_history();
    global->animation_fire_time = -1.f;
  }
  else if(name_hash == HASH("player_disconnect")){
    i32 id = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));

    // id is sometimes zero when people disconnect resulting in their data not being reset.
    DBG("[!] %i data reset\n", id);
    if( id == 0 ){
      DBG("[-] failed to resolve userID in %s\n", event->get_name());
    }

    i8*  player_name  = event->get_string(XOR("name"));
    bool is_bot       = event->get_bool(XOR("bot"));
    i8*  steamid      = event->get_string(XOR("networkid"));
    u32  steamid_hash = HASH_RT(steamid);

    static u32 last_steamid_hash = 0;

    utils::reset_player_data(id);
    utils::reset_lag_comp_data(id);

    // Filter out disconnect spam.
    if(last_steamid_hash == steamid_hash && !is_bot)
      return false;

    last_steamid_hash = steamid_hash;
  }
  else if(name_hash == HASH("player_connect_client")){
    i32 id = global->engine->get_player_for_user_id(event->get_int(XOR("index")));

    i8*  player_name  = event->get_string(XOR("name"));
    bool is_bot       = event->get_bool(XOR("bot"));
    i8*  steamid      = event->get_string(XOR("networkid"));
    u32  steamid_hash = HASH_RT(steamid);

    static u32 last_steamid_hash = 0;

    utils::reset_player_data(id);
    utils::reset_lag_comp_data(id);

    // Filter out connection spam.
    if(last_steamid_hash == steamid_hash && !is_bot)
      return false;

    last_steamid_hash = steamid_hash;
  }
  else if(name_hash == HASH("player_team")){
    i32 index = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));
    i32 team  = global->engine->get_player_for_user_id(event->get_int(XOR("team")));
    if(index == global->localplayer_index && team <= 1)
      crit_hack->reset_bucket_backup();
  }
  else if(name_hash == HASH("player_death")){
    c_base_player* localplayer = utils::localplayer();

    if(localplayer != nullptr){
      i32 attacked = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));
      i32 attacker = global->engine->get_player_for_user_id(event->get_int(XOR("attacker")));
      i32 weaponid = event->get_int(XOR("weaponid"));
      if(attacker == global->localplayer_index && attacker != attacked){
        misc::infinite_noise_maker_spam(true);
        misc::show_hitbox_on_hit(attacker, attacked, weaponid, true);
      }

      if(global->aimbot_target_index == attacked)
        global->aimbot_target_index = -1;

      auto_heal_arrow->on_death(attacker, attacked);
      dormant_manager->reset(attacked);
      utils::reset_lag_comp_data(attacked);
      utils::reset_transform(attacked);

      if(attacked == localplayer->get_index()){
        misc::force_mvm_revive_response();
        global->on_death();
        fake_latency->reset();
      }

    }
  }
  else if(name_hash == HASH("player_spawn")){
    i32 index = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));
    if(index == global->localplayer_index){
      global->resist_reset = true;
      crit_hack->reset();

      c_base_player* localplayer = utils::localplayer();
      if(localplayer != nullptr){
        if(!(localplayer->entity_flags() & FL_ONGROUND)){
          DBG("[!] On spawn applying FL_ONGROUND flag\n");
          localplayer->entity_flags() |= FL_ONGROUND;
        }
      }
    }

    dormant_manager->reset(index);
    utils::reset_lag_comp_data(index);
    utils::reset_transform(index);
  }
  else if(name_hash == HASH("revive_player_notify")){
    if(config->misc.mvm_revive_onmedicheal){
      i32 index = event->get_int(XOR("entindex"));
      if(index == global->localplayer_index){
        misc::force_mvm_revive_response(true);
      }
    }
  }
  else if(name_hash == HASH("player_healed")){ // Only called if the player is hurt and is being healed by a medic or dispenser.
    misc::process_heal_event(event->get_int(XOR("patient")), event->get_int(XOR("amount")));
  }
  else if(name_hash == HASH("player_hurt")){
    crit_hack->fire_event_run(event);
    auto_vaccinator->on_damage(event);
    resolver->fire_event(event);

    i32 attacker = global->engine->get_player_for_user_id(event->get_int(XOR("attacker")));
    i32 victim   = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));
    i32 weaponid = event->get_int(XOR("weaponid"));

    misc::show_hitbox_on_hit(attacker, victim, weaponid, false);
  }
  else if(name_hash == HASH("player_changename")){
    i32 index = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));
    c_base_player* player = global->entity_list->get_entity(index);
    if(player != nullptr){
      c_player_data* data = player->data();
      if(data != nullptr)
        data->name_cross_check = false;
    }
  }
  else if(name_hash == HASH("game_newmap")){
    i8* map_name = event->get_string(XOR("mapname"));
    utils::update_map_hash(map_name);
  }

  if(privacy_mode->handle_game_event(event))
    return true;

  logs->on_game_event(event);
  return utils::call_fastcall64<bool, void*>(gen_internal->decrypt_asset(global->fire_event_hook_trp), rcx, event);
}

EXPORT bool __fastcall usermessages_dispatch_user_message_hook(void* rcx, i32 msg_type, s_bf_read* bf_read){
  if(dispatch_user_msg->dispatch_user_message(rcx, msg_type, bf_read))
    return true;

  logs->dispatch_user_message(rcx, msg_type,  bf_read);
  return utils::call_fastcall64<bool, i32, s_bf_read*>(gen_internal->decrypt_asset(global->usermessages_dispatch_user_message_hook_trp), rcx, msg_type, bf_read);
}

EXPORT void __fastcall cviewrender_setscreenoverlaymaterial_hook(void* rcx, void* material){
  assert(rcx != nullptr);

  if(config->visual.disable_overlay_effects_enabled){
    if(material != nullptr){
      i8* name = utils::virtual_call<0, i8*>(material); // IMaterial::GetName
      if(name != nullptr){
        u32 name_hash = HASH_RT(name);

        switch(name_hash){
          default:{
            if(str_utils::contains(name, XOR("effects/water"))){
              if(config->visual.disable_overlay_water)
                return;
            }
            DBG("[!] Unknown screenoverlay: %s\n", name);
            break;
          }
          case HASH("effects/invuln_overlay_red"):
          case HASH("effects/invuln_overlay_blue"):{
            if(config->visual.disable_overlay_invuln)
              return;

            break;
          }
          case HASH("effects/milk_screen"):{
            if(config->visual.disable_overlay_milk)
              return;

            break;
          }
          case HASH("effects/jarate_overlay"):{
            if(config->visual.disable_overlay_jarate)
              return;

            break;
          }
          case HASH("effects/bleed_overlay"):{
            if(config->visual.disable_overlay_bleed)
              return;

            break;
          }
          case HASH("effects/stealth_overlay"):{
            if(config->visual.disable_overlay_stealth)
              return;

            break;
          }
          case HASH("effects/dodge_overlay"):{
            if(config->visual.disable_overlay_bonked)
              return;

            break;
          }
          case HASH("effects/gas_overlay"):{
            if(config->visual.disable_overlay_gas)
              return;

            break;
          }
          case HASH("effects/imcookin"):{
            if(config->visual.disable_overlay_onfire)
              return;

            break;
          }
        }
      }
    }
  }

  utils::call_fastcall64<void, void*>(gen_internal->decrypt_asset(global->cviewrender_setscreenoverlaymaterial_hook_trp), rcx, material);
}

EXPORT void __fastcall ctfplayer_createplayergibs_hook(c_base_player* rcx, vec3& origin, vec3& vel, float impact_scale, bool burning, bool wearable_gibs, bool only_head, bool disguise_gibs){
  assert(rcx != nullptr);

  if(config->misc.disable_gibs)
    return;

  // It'll attempt to create gibs on dormant players. Body parts / hats.
  if(rcx != nullptr && rcx->is_dormant())
    return;

  utils::call_fastcall64<void, vec3&, vec3&, float, bool, bool, bool, bool>(gen_internal->decrypt_asset(global->ctfplayer_createplayergibs_hook_trp), rcx, origin, vel, impact_scale, burning, wearable_gibs, only_head, disguise_gibs);
}

EXPORT void __fastcall update_step_sound_hook(void* rcx, void* surface, void* origin, void* velocity){
  if(config->misc.disable_footsteps){
    if(global->localplayer == rcx)
      return;
  }

  utils::call_fastcall64<void, void*, void*, void*>(gen_internal->decrypt_asset(global->update_step_sound_hook_trp), rcx, surface, origin, velocity);
}

EXPORT void __fastcall get_outer_abs_velocity_hook(c_base_player* rcx, vec3& velocity){
  assert(rcx != nullptr);
  if(global->current_map_hash == HASH("maps/tr_walkway_rc2.bsp") || global->current_map_hash == HASH("maps/tr_walkway_rc.bsp")){
    if(rcx != nullptr && rcx != global->localplayer && rcx->team() != 2 && rcx->entity_flags() & FL_ONGROUND){
      velocity = vec3();
      return;
    }
  }

  utils::call_fastcall64<void, vec3&>(gen_internal->decrypt_asset(global->get_outer_abs_velocity_hook_trp), rcx, velocity);
}

EXPORT void __fastcall ctfplayeranimstate_aimyaw_hook(s_animation_state* rcx, void* studio){
  assert(rcx != nullptr);
  rcx->force_aim_yaw    = true;
  rcx->current_feet_yaw = rcx->goal_feet_yaw = rcx->yaw;

  utils::call_fastcall64<void, void*>(gen_internal->decrypt_asset(global->ctfplayeranimstate_aimyaw_hook_trp), rcx, studio);
}

EXPORT void __fastcall hud_chatline_insert_and_colorize_text_hook(void* rcx, wchar_t* buf, i32 client_index){
  static wchar_t new_buf[1024];
  i32 result = chat_format_manager->hud_chatline_insert_and_colorize_text(buf, client_index, new_buf, sizeof(new_buf));
  if(result == e_chat_format_type_replace_msg){
    utils::call_fastcall64<void, wchar_t*, i32>(gen_internal->decrypt_asset(global->hud_chatline_insert_and_colorize_text_hook_trp), rcx, new_buf, client_index);
    return;
  }

  if(result == e_chat_format_type_no_call)
    return;

  utils::call_fastcall64<void, wchar_t*, i32>(gen_internal->decrypt_asset(global->hud_chatline_insert_and_colorize_text_hook_trp), rcx, buf, client_index);
}

EXPORT void __fastcall emit_sound_hook(c_base_entity* rcx, i8* sound_name, float volume, i32 unk){
  assert(rcx != nullptr);
  assert(sound_name != nullptr);

  u32 name_hash = HASH_RT(sound_name);

  if(global->block_golden_kill_sound && name_hash == HASH("Saxxy.TurnGold"))
    return;

  if(global->localplayer == rcx){
    // Fix the game playing these sounds a billion times.
    if(__builtin_return_address(0) == global->medigun_find_and_heal_target_retaddr1 || name_hash == HASH("TFPlayer.ReCharged")
      || name_hash == HASH("Weapon_StickyBombLauncher.ChargeUp")
      || name_hash == HASH("Weapon_RocketPack.BoostersReady")
      || name_hash == HASH("Weapon_LooseCannon.Charge")
      || name_hash == HASH("Physics.WaterSplash")
      || name_hash == HASH("Credits.Updated")
      || name_hash == HASH("DisciplineDevice.PowerUp")){
        if(!global->prediction->first_time_predicted)
          return;
    }
  }

  //if(cheat_detection->on_emit_sound(rcx->get_index(), sound_name))
  //  return;
  
  utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->emit_sound_hook_trp), rcx, sound_name, volume, unk);
}

EXPORT i64 __fastcall s_startsound_hook(start_sound_params& params){
  if(params.sfx != nullptr){

    i8* sound_name = params.sfx->get_name();
    if(sound_name != nullptr){
      assert(sound_name != nullptr);

      //if(params.from_server){
        //if(cheat_detection->on_emit_sound(params.sound_source, sound_name))
        //  return 0;
      //}

      u32 name_hash = HASH_RT(sound_name);
      if(config->misc.block_noise_maker_sounds){
        switch(name_hash){
          default: break;
          case HASH("items\\halloween\\witch01.wav"):
          case HASH("items\\halloween\\witch02.wav"):
          case HASH("items\\halloween\\witch03.wav"):
          case HASH("items\\halloween\\banshee01.wav"):
          case HASH("items\\halloween\\banshee02.wav"):
          case HASH("items\\halloween\\banshee03.wav"):
          case HASH("items\\halloween\\werewolf01.wav"):
          case HASH("items\\halloween\\werewolf02.wav"):
          case HASH("items\\halloween\\werewolf03.wav"):
          case HASH("items\\halloween\\gremlin01.wav"):
          case HASH("items\\halloween\\gremlin02.wav"):
          case HASH("items\\halloween\\gremlin03.wav"):
          case HASH("items\\halloween\\cat01.wav"):
          case HASH("items\\halloween\\cat02.wav"):
          case HASH("items\\halloween\\cat03.wav"):
          case HASH("items\\halloween\\crazy01.wav"):
          case HASH("items\\halloween\\crazy02.wav"):
          case HASH("items\\halloween\\crazy03.wav"):
          case HASH("items\\halloween\\stabby.wav"):
          case HASH(")misc\\happy_birthday_tf_01.wav"):
          case HASH(")misc\\happy_birthday_tf_02.wav"):
          case HASH(")misc\\happy_birthday_tf_03.wav"):
          case HASH(")misc\\happy_birthday_tf_04.wav"):
          case HASH(")misc\\happy_birthday_tf_05.wav"):
          case HASH(")misc\\happy_birthday_tf_06.wav"):
          case HASH(")misc\\happy_birthday_tf_07.wav"):
          case HASH(")misc\\happy_birthday_tf_08.wav"):
          case HASH(")misc\\happy_birthday_tf_09.wav"):
          case HASH(")misc\\happy_birthday_tf_10.wav"):
          case HASH(")misc\\happy_birthday_tf_11.wav"):
          case HASH(")misc\\happy_birthday_tf_12.wav"):
          case HASH(")misc\\happy_birthday_tf_13.wav"):
          case HASH(")misc\\happy_birthday_tf_14.wav"):
          case HASH(")misc\\happy_birthday_tf_15.wav"):
          case HASH(")misc\\happy_birthday_tf_16.wav"):
          case HASH(")misc\\happy_birthday_tf_17.wav"):
          case HASH(")misc\\happy_birthday_tf_18.wav"):
          case HASH(")misc\\happy_birthday_tf_19.wav"):
          case HASH(")misc\\happy_birthday_tf_20.wav"):
          case HASH(")misc\\happy_birthday_tf_21.wav"):
          case HASH(")misc\\happy_birthday_tf_22.wav"):
          case HASH(")misc\\happy_birthday_tf_23.wav"):
          case HASH(")misc\\happy_birthday_tf_24.wav"):
          case HASH(")misc\\happy_birthday_tf_25.wav"):
          case HASH(")misc\\happy_birthday_tf_26.wav"):
          case HASH(")misc\\happy_birthday_tf_27.wav"):
          case HASH(")misc\\happy_birthday_tf_28.wav"):
          case HASH(")misc\\happy_birthday_tf_29.wav"):
          case HASH(")items\\summer\\summer_fireworks1.wav"):
          case HASH(")items\\summer\\summer_fireworks2.wav"):
          case HASH(")items\\summer\\summer_fireworks3.wav"):
          case HASH(")items\\summer\\summer_fireworks4.wav"):
          case HASH(")items\\japan_fundraiser\\tf_zen_bell_01.wav"):
          case HASH(")items\\japan_fundraiser\\tf_zen_bell_02.wav"):
          case HASH(")items\\japan_fundraiser\\tf_zen_bell_03.wav"):
          case HASH(")items\\japan_fundraiser\\tf_zen_bell_04.wav"):
          case HASH(")items\\japan_fundraiser\\tf_zen_bell_05.wav"):
          case HASH(")items\\japan_fundraiser\\tf_zen_prayer_bowl_01.wav"):
          case HASH(")items\\japan_fundraiser\\tf_zen_prayer_bowl_02.wav"):
          case HASH(")items\\japan_fundraiser\\tf_zen_prayer_bowl_03.wav"):
          case HASH(")items\\japan_fundraiser\\tf_zen_tingsha_01.wav"):
          case HASH(")items\\japan_fundraiser\\tf_zen_tingsha_02.wav"):
          case HASH(")items\\japan_fundraiser\\tf_zen_tingsha_03.wav"):
          case HASH(")items\\japan_fundraiser\\tf_zen_tingsha_04.wav"):
          case HASH(")items\\japan_fundraiser\\tf_zen_tingsha_05.wav"):
          case HASH(")items\\japan_fundraiser\\tf_zen_tingsha_06.wav"):
          case HASH(")items\\football_manager\\vuvezela_01.wav"):
          case HASH(")items\\football_manager\\vuvezela_02.wav"):
          case HASH(")items\\football_manager\\vuvezela_03.wav"):
          case HASH(")items\\football_manager\\vuvezela_04.wav"):
          case HASH(")items\\football_manager\\vuvezela_05.wav"):
          case HASH(")items\\football_manager\\vuvezela_06.wav"):
          case HASH(")items\\football_manager\\vuvezela_07.wav"):
          case HASH(")items\\football_manager\\vuvezela_08.wav"):
          case HASH(")items\\football_manager\\vuvezela_09.wav"):
          case HASH(")items\\football_manager\\vuvezela_10.wav"):
          case HASH(")items\\football_manager\\vuvezela_11.wav"):
          case HASH(")items\\football_manager\\vuvezela_12.wav"):
          case HASH(")items\\football_manager\\vuvezela_13.wav"):
          case HASH(")items\\football_manager\\vuvezela_14.wav"):
          case HASH(")items\\football_manager\\vuvezela_15.wav"):
          case HASH(")items\\football_manager\\vuvezela_16.wav"):
          case HASH(")items\\football_manager\\vuvezela_17.wav"):
          case HASH(")items\\samurai\\tf_samurai_noisemaker_setb_01.wav"):
          case HASH(")items\\samurai\\tf_samurai_noisemaker_setb_02.wav"):
          case HASH(")items\\samurai\\tf_samurai_noisemaker_setb_03.wav"):
          case HASH("misc\\jingle_bells\\jingle_bells_nm_01.wav"):
          case HASH("misc\\jingle_bells\\jingle_bells_nm_02.wav"):
          case HASH("misc\\jingle_bells\\jingle_bells_nm_03.wav"):
          case HASH("misc\\jingle_bells\\jingle_bells_nm_04.wav"):
          case HASH("misc\\jingle_bells\\jingle_bells_nm_05.wav"):
            return 0;
        }
      }

      if(config->misc.block_frying_pan_sounds){
        switch(name_hash){
          default: break;
          case HASH("weapons\\pan\\pan_impact_world1.wav"):
          case HASH("weapons\\pan\\pan_impact_world2.wav"):
          case HASH("weapons\\pan\\pan_impact_world3.wav"):
          case HASH("weapons\\pan\\melee_frying_pan_01.wav"):
          case HASH("weapons\\pan\\melee_frying_pan_02.wav"):
          case HASH("weapons\\pan\\melee_frying_pan_03.wav"):
          case HASH("weapons\\pan\\melee_frying_pan_04.wav"):
            return 0;
        }
      }

      if(name_hash == HASH("weapons\\stickybomblauncher_charge_up.wav") || name_hash == HASH("weapons\\loose_cannon_charge.wav")){
        if(!global->prediction->first_time_predicted)
           return 0;
      }

      dormant_manager->process_entity_sounds(params);
    }
  }

  return utils::call_fastcall64_raw<i64, start_sound_params&>(gen_internal->decrypt_asset(global->s_startsound_hook_trp), params);
}

EXPORT c_user_cmd* __fastcall cinput_get_user_cmd_hook(s_input* rcx, i32 seq){
  return &global->input->commands[seq % 90];
}

EXPORT bool __fastcall buf_addtext_withmarkers_hook(i8 marker_left, i8* cmd, i8 marker_right){
  // Stop servers from executing commands on our client.
  if(global->baseclientstate_processstringcmd_retaddr1 == __builtin_return_address(0)){
    bool send = false;
    c_cvar* cvar = global->cvar->find_var(cmd);
    if(cvar != nullptr){
      if(cvar->flags & FCVAR_DEVELOPMENTONLY || cvar->flags & FCVAR_HIDDEN)
        send = true;
    }
    else
      send = true;

    if(send){
      DBG("[!] Relaying server sent string command \"%s\" to server. (anti-cheat bypass)\n", cmd);
      global->engine->server_cmd(cmd, false);
    }
    return false;
  }

  return utils::call_fastcall64_raw<bool, i8, i8*, i8>(gen_internal->decrypt_asset(global->buf_addtext_withmarkers_hook_trp), marker_left, cmd, marker_right);
}

// Fixes the crashing caused by the halloween cage. Do not check if base_entity is nullptr the game's function already does a check.
EXPORT void __fastcall cbaseentity_followentity_hook(void* rcx, void* base_entity, bool bone_merge){
  if(rcx == nullptr){
    DBG("[!] Prevented game crash by halloween cage.\n");
    float time = math::time();

    static float next_notify_time = 0.f;
    if(next_notify_time <= time){
      notify->create(NOTIFY_TYPE_ALERT_FLASH, WXOR(L"Prevented a game-crash exploit. \"addcond 88\""), colour(255, 255, 255, 255));
      next_notify_time = time + 5.f;
    }
    return;
  }

  utils::call_fastcall64<void, void*, bool>(gen_internal->decrypt_asset(global->cbaseentity_followentity_hook_trp), rcx, base_entity, bone_merge);
}

// Allow us to get achievements with sv_cheats 1 or developer 1
EXPORT bool __fastcall achievementmgr_checkachievementenabled_hook(void* rcx){
  return true;
}

EXPORT bool __fastcall engine_get_player_info_hook(void* rcx, i32 entity_id, s_player_info* info){
  assert(rcx != nullptr);
  assert(global->is_steam_friend_get_player_info_retaddr1 != nullptr);
  assert(global->update_player_avatar_get_player_info_retaddr2 != nullptr);

  bool result = utils::call_fastcall64<bool, i32, s_player_info*>(gen_internal->decrypt_asset(global->engine_get_player_info_hook_trp), rcx, entity_id, info);
  if(!result || info == nullptr || entity_id == 0 || entity_id > globaldata->max_clients || global->skip_get_player_info_hook)
    return result;

  void* ret_addr = __builtin_return_address(0);

  // Dont run if is steam friend function is calling this or we will call our hook forever
  if(ret_addr == global->is_steam_friend_get_player_info_retaddr1)
    return result;
  
  privacy_mode->engine_get_player_info_hook(entity_id, info, (ret_addr == global->update_player_avatar_get_player_info_retaddr2));
  cheat_detection->engine_get_player_info_hook(entity_id, info);

  return result;
}

EXPORT void __fastcall ctfragdoll_create_ragdoll_hook(ctf_ragdoll* rcx){
  assert(rcx != nullptr);
  assert(global->cl_ragdoll_fade_time != nullptr);
  assert(global->cl_ragdoll_forcefade != nullptr);
  assert(global->cl_ragdoll_physics_enable != nullptr);

  bool  restore_values     = false;
  float old_fade_time      = global->cl_ragdoll_fade_time->flt;
  i32   old_forcefade      = global->cl_ragdoll_forcefade->val;
  i32   old_physics_enable = global->cl_ragdoll_physics_enable->val;

  // Make sure that if people have this feature enabled. We setup some convars to have the correct values for this feature to work properly.
  if(config->visual.ragdoll_effects){
    global->cl_ragdoll_fade_time->flt      = math::biggest(global->cl_ragdoll_fade_time->flt, 30.f);
    global->cl_ragdoll_forcefade->val      = 0;
    global->cl_ragdoll_physics_enable->val = 1;
    restore_values                               = true;
  }

  if(config->visual.ragdoll_effects && config->visual.ragdoll_effects_disable_death_animations)
    rcx->damage_custom() = 0;

  if(config->visual.ragdoll_effects_show_all || config->visual.ragdoll_effects_show_friendly && global->localplayer_team == rcx->team() || config->visual.ragdoll_effects_show_enemy && global->localplayer_team != rcx->team()){
    if(config->visual.ragdoll_effects){
      if(config->visual.ragdoll_effects_disable_game_spawned_effects){
        rcx->ice_ragdoll()  = false;
        rcx->gold_ragdoll() = false;
        rcx->become_ash()   = false;
        rcx->electrocuted() = false;
        rcx->burning()      = false;
        rcx->dissolving()   = false;
      }

      rcx->become_ash()      = config->visual.ragdoll_effect_weak_ash;
      rcx->electrocuted()    = config->visual.ragdoll_effect_strong_electrocuted;
      rcx->burning()         = config->visual.ragdoll_effect_strong_burning;
      // Frozen one doesn't work unless we set the golden one to true. The game's code will prefer the frozen effects over the golden one.
      rcx->gold_ragdoll()    = config->visual.ragdoll_effect_weak_golden || config->visual.ragdoll_effect_weak_frozen;
      rcx->ice_ragdoll()     = config->visual.ragdoll_effect_weak_frozen;
      rcx->dissolving()      = config->visual.ragdoll_effect_weak_dissolve;
    }
  }

  global->block_golden_kill_sound = rcx->gold_ragdoll() && rcx->ice_ragdoll();
  utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->ctfragdoll_create_ragdoll_hook_trp), rcx);

  // Restore cvar values to their original state.
  if(restore_values){
    global->cl_ragdoll_fade_time->flt      = old_fade_time;
    global->cl_ragdoll_forcefade->val      = old_forcefade;
    global->cl_ragdoll_physics_enable->val = old_physics_enable;
  }

  global->block_golden_kill_sound = false;
}

EXPORT void __fastcall cbaseanimating_update_client_side_animation_hook(void* rcx){
  assert(rcx != nullptr);

  if(!global->engine->is_playing_demo()){
    if(global->localplayer == rcx && global->is_choking_cmd)
      return;
  }
  
  utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->cbaseanimating_update_client_side_animation_hook_trp), rcx);
}

//TODO BUG! viewmodel fov lower than 90 will make the viewmodel not aim towards targets properly.
EXPORT void __fastcall ctfviewmodel_calcviewmodelview_hook(c_base_entity* rcx, void* owner, vec3& eye_pos, vec3& angles){
  assert(rcx != nullptr);

  if(config->visual.no_visual_recoil){
    c_base_player* localplayer = utils::localplayer();

    if(localplayer != nullptr){
      angles -= localplayer->punch_angle();
      angles.z = 0.f;
    }
  }

  viewmodel->calc_view_model(rcx, eye_pos, angles);
  utils::call_fastcall64<void, void*, vec3&, vec3&>(gen_internal->decrypt_asset(global->ctfviewmodel_calcviewmodelview_hook_trp), rcx, owner, eye_pos, angles);
  viewmodel->post_calc_view_model(rcx, owner, eye_pos, angles);
}

EXPORT void __fastcall ctfviewmodel_calcviewmodellag_hook(void* rcx, vec3& origin, vec3& angles, vec3& org_angles){
  assert(rcx != nullptr);

  viewmodel->pre_calc_view_model_lag(rcx, origin, angles);
  utils::call_fastcall64<void, vec3&, vec3&, vec3&>(gen_internal->decrypt_asset(global->ctfviewmodel_calcviewmodellag_hook_trp), rcx, origin, angles, org_angles);
  viewmodel->post_calc_view_model_lag(rcx, origin, angles);
}

EXPORT double __fastcall calc_viewmodel_bob_hook(void* rcx, void* bob_state){
  if(config->visual.no_viewmodel_bob)
    return 0.0;

  return utils::call_fastcall64<double, void*>(gen_internal->decrypt_asset(global->calc_viewmodel_bob_hook_trp), rcx, bob_state);
}

EXPORT bool __fastcall cnetchan_sendnetmsg_hook(c_net_channel* rcx, c_net_message& net_msg, bool rel, bool voice){
  assert(rcx != nullptr);
  assert(net_msg.get_name() != nullptr);

  logs->send_net_msg(net_msg, rel, voice);

  u32 name_hash = HASH_RT(net_msg.get_name());

  // Float based convars will go from 1.0 to 1 resulting in bans. - Rud
  // Does not handle "networked" convars yet.
  if(name_hash == HASH("clc_RespondCvarValue")){
    if(acm->on_process_query_convar_hook((c_clc_respond_cvar_value*)&net_msg))
      return true;
  }
  else if(name_hash == HASH("clc_VoiceData"))
    voice = true;
  else if(name_hash == HASH("clc_ClientInfo")){
    //DBG("0x%p\n", &net_msg);

    //*(uptr*)((uptr)&net_msg + 0x20) = 1055404996;
  }
  else if(name_hash == HASH("clc_Move")){

    // CL_SendMove is inlined in Garry's Mod.
    // So rather than rebuild cl_move which would be very annoying todo we'll do this here.
    // Tested it and it works fine.
    c_clc_move* move = (c_clc_move*)&net_msg;
    {
      i32 last_outgoing_cmd = global->client_state->last_outgoing_command();
      i32 choked_cmd        = global->client_state->choked_commands();
      i32 next_cmd          = last_outgoing_cmd + choked_cmd + 1;

      static void* buf = malloc(0xFA0);
      memset(buf, 0, 0xFA0);

      move->data_out.start_writing(buf, 0xFA0, 0, -1);
      move->new_commands = math::clamp(choked_cmd + 1, 0, 15);

      i32 extra_cmd  = (choked_cmd + 1) - move->new_commands;
      i32 cmd_backup = math::biggest(2, extra_cmd);

      move->backup_commands = math::clamp(cmd_backup, 0, 7);

      i32  num_of_cmds = move->new_commands + move->backup_commands;
      i32  from        = -1;
      bool ok          = true;

      for(i32 to = next_cmd - num_of_cmds + 1; to <= next_cmd; to++){
        bool is_new_cmd = to >= (next_cmd - move->new_commands) + 1;

        ok = ok && global->client->write_user_cmd_delta_to_buffer(&move->data_out, from, to, is_new_cmd);
        from = to;
      }

      if(ok){
        if(rcx->choked_packets > 0)
          rcx->choked_packets -= extra_cmd;
      }
    }
  }

  return utils::call_fastcall64<bool, c_net_message&, bool, bool>(gen_internal->decrypt_asset(global->cnetchan_sendnetmsg_hook_trp), rcx, net_msg, rel, voice);
} 

EXPORT void __fastcall glowobjectmanager_rendergloweffects_hook(void* rcx, void* setup, i32 split_screen_slot){}

EXPORT void __fastcall menu_lobbychat_hook(void* rcx, c_game_event* event){
  if(utils::is_in_game())
    return;

  // This is the function that lags your game when somebody talks with party chat.
  utils::call_fastcall64<void, c_game_event*>(gen_internal->decrypt_asset(global->menu_lobbychat_hook_trp), rcx, event);
}

EXPORT void __fastcall cbuf_executecmd_hook(c_command_list* cmd, u32 type){
  if(cmd != nullptr && cmd->pargv[0] != nullptr){
    assert(cmd->pargv[0] != nullptr);

    u32 command_hash = HASH_RT(cmd->pargv[0]);

    if(interp_manager->is_interp_command(command_hash) && !interp_manager->should_update_interp()){
      DBG("[!] Bad time to modify interp.\n");
      return;
    }

    switch(command_hash){
      default: break;
      case XOR32(HASH("598317060183867824992048198842049")):
      {
        create_server_iden(rijin_userid, "RIJIN_USER_ID");

        i8 buf[IDEAL_MAX_BUF_SIZE];
        formatA(buf, XOR("echo %i"), rijin_userid_gen);
        global->engine->client_cmd(buf);

        return;
      }
      case HASH("heyassertforme"):
      {
        #if defined(DEV_MODE) || defined(STAGING_MODE)
          assert(false && "Oh no, we had an undefined result!");
        #endif
        break;
      }
      case HASH("rijin_import_config"):
      {

        if(cloud::from_clipboard(SETTINGS_VERSION, HASH("TF2"), global->settings, sizeof(c_settings))){
          DBG("[!] Succesfully imported!\n");
        }
        else{
          DBG("[!] Failed to import settings from clipboard\n");
        }

        return;
      }
      case HASH("rijin_givemeall"):
      case HASH("rijin_achievements"):
      case HASH("rijin_achievement"):
      case HASH("rijin_unlockall"):
      {

        c_achievement_mgr* achievement_mgr = global->engine->get_achievement_mgr();
        if(achievement_mgr != nullptr){
          utils::send_con_notification(XOR("Unlocking achievements...\n"));
          for(i32 i = 0; i < achievement_mgr->get_achievement_count(); i++){
            c_achievement* achievement = achievement_mgr->get_achievement_by_index(i);
            if(achievement == nullptr)
              continue;

            achievement_mgr->award_achievement(achievement->get_id());
          }
          utils::send_con_notification(XOR("All achievements were unlocked!\n"));
        }

        return;
      }
      case HASH("rijin_startrecord"):
      case HASH("rijin_record"):
      {
        demosystem->begin(XOR("manual"));
        return;
      }
      case HASH("rijin_stop"):
      case HASH("rijin_stoprecord"):
      {
        demosystem->end();
        return;
      }
      case HASH("rijin_quick_bind"):{
        i8 buf[IDEAL_MAX_BUF_SIZE];
        c_base_player* localplayer = utils::localplayer();
        if(localplayer != nullptr){
          vec3 obb_center = localplayer->obb_center();
          formatA(buf, XOR("bind - \"bot_teleport all %i %i %i 0 0 0\""), (i32)obb_center.x, (i32)obb_center.y, (i32)obb_center.z);
          global->engine->client_cmd(buf);
          utils::send_con_notification(XOR("Quick bound bot_teleport at current position to key \"-\"\n"));
        }
        return;
      }
      case HASH("rijin_quick_tele"):{
        i8 buf[IDEAL_MAX_BUF_SIZE];
        c_base_player* localplayer = utils::localplayer();
        if(localplayer != nullptr){
          vec3 obb_center = localplayer->obb_center();
          formatA(buf, XOR("bot_teleport all %i %i %i 0 0 0"), (i32)obb_center.x, (i32)obb_center.y, (i32)obb_center.z);
          global->engine->client_cmd(buf);
          utils::send_con_notification(XOR("Teleported every bot to your position.\n"));
        }
        return;
      }
      case HASH("lerp"):
      {
        DBG("client lerp: %f\n", interp_manager->get_lerp());
        break;
      }
      case HASH("rijin_crash"):
      {
        global->engine = nullptr;
        DBG("[!] %i\n", global->engine->get_local_player());
        break;
      }
      case HASH("rijin_current_model"):
      {
        c_base_player* localplayer = utils::localplayer();
        if(localplayer != nullptr){
          void* model = localplayer->get_model();
            if(model != nullptr){
            i8* model_name = global->model_info->get_model_name(model);
            if(model_name != nullptr){
              DBG("[!] %p: %s\n", model, model_name);
            }
          }
        }
        return;
      }
      case HASH("rijin_dump_models"):
      {
        #if defined(DEV_MODE) || defined(STAGING_MODE)
          float time = math::time();
          static float next_use_time = 0.f;
  
          if(next_use_time > time)
            return;
  
          c_base_player* localplayer = utils::localplayer();
          if(localplayer == nullptr)
            return;
  
          next_use_time = time + 5.f;
          for(i32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
            c_base_entity* entity = global->entity_list->get_entity(i);
            if(entity == nullptr)
              continue;
  
            if(entity->is_dormant())
              continue;
  
            if(localplayer->origin().distance_meter(entity->origin()) > 8)
              continue;
  
            i8 class_name[IDEAL_MAX_BUF_SIZE];
            formatA(class_name, XOR("Class: %s"), entity->get_client_class()->name);
            render_debug->draw_text(entity->obb_center(), colour(255, 255, 255, 255), class_name, 0, 5.f);
  
            i8 model_name[IDEAL_MAX_BUF_SIZE];
            if(entity->get_model() == nullptr){
              formatA(model_name, XOR("NO MODEL"));
            }
            else{
              formatA(model_name, XOR("Model: %s"), global->model_info->get_model_name(entity->get_model()));
            }
  
            render_debug->draw_text(entity->obb_center(), colour(255, 255, 255, 255), model_name, 2, 5.f);
          }
        #endif
        return;
      }
      case HASH("setcvar"):
      case HASH("rijin_setcvar"):
      {
        if(cmd->arg_count < 3){
          utils::send_con_notification(XOR("setcvar <cvar> <value>\n"));
          return;
        }

        u32 parameter_hash = HASH_RT(cmd->pargv[1]);
        if(interp_manager->is_interp_command(parameter_hash) && !interp_manager->should_update_interp()){
          utils::send_con_notification(XOR("You cannot modify this command right now.\n"));
          return;
        }


        c_cvar* var = global->cvar->find_var(cmd->pargv[1]);
        if(var == nullptr){
          i8 buf[IDEAL_MAX_BUF_SIZE];
          formatA(buf, XOR("\"%s\" isn't a valid cvar\n"), cmd->pargv[1]);
          utils::send_con_notification(buf);
          return;
        }

        var->set_value(cmd->pargv[2]);

        i8 buf[IDEAL_MAX_BUF_SIZE];
        formatA(buf, XOR("Set %s to value \"%s\"\n"), cmd->pargv[1], cmd->pargv[2]);
        utils::send_con_notification(buf);
        return;
      }
      case HASH("setinfo"):
      {
        // Some awesome TF2 "FPS" configs use the setinfo command to modify these cvars.
        if(cmd->pargv[1] != nullptr){
          u32 key = HASH_RT(cmd->pargv[1]);

          switch(key){
            default: break;
            case HASH("cl_interp"):
            case HASH("cl_interpolate"):
            case HASH("cl_interp_ratio"):
            case HASH("cl_cmdrate"):
            case HASH("cl_updaterate"):
            case HASH("cl_smooth"):
            case HASH("cl_interp_all"):
            case HASH("cl_predict"):
            case HASH("cl_predictweapons"):
            case HASH("cl_pred_optimize"):
            case HASH("cl_lagcompensation"):
            case HASH("cl_flipviewmodels"):
            {
              DBG("[!] Blocked setinfo from setting %s\n", cmd->pargv[1]);
              return;
            }
          }
          DBG("[!] setinfo %s executed\n", cmd->pargv[1]);

        }
        break;
      }
      case HASH("slot1"):
      case HASH("slot2"):
      case HASH("slot3"):
      case HASH("slot4"):
      case HASH("slot5"):
      case HASH("slot6"):
      case HASH("slot7"):
      case HASH("slot8"):
      case HASH("slot9"):
      case HASH("slot0"):
      case HASH("invnext"):
      case HASH("invprev"):
      case HASH("lastinv"):
      {
        auto_weapon_swap->reset();
        break;
      }
    }
  }

  utils::call_fastcall64_raw<void, c_command_list*, u32>(gen_internal->decrypt_asset(global->cbuf_executecmd_hook_trp), cmd, type);
}

EXPORT void __fastcall cvar_set_value_hook(c_cvar* rcx, i8* val){
  assert(rcx != nullptr);
  assert(val != nullptr);

  u32 hash = HASH_RT(rcx->name);

  utils::call_fastcall64<void, i8*>(gen_internal->decrypt_asset(global->cvar_set_value_hook_trp), rcx, val);

  if(hash == HASH("cl_connectmethod"))
    demosystem->format_dir(false);
}

// allows you to record with voice_loopback on a demo and not hear it in a live game, only in demos
EXPORT i32 __fastcall voice_assign_channel_hook(i32 entity_id, bool proximity){

  if(global->voice_loopback != nullptr && global->voice_loopback->val == 2){
    if(!global->engine->is_playing_demo() && entity_id == global->localplayer_index)
      return -2;
  }

  return utils::call_fastcall64_raw<i32, i32, bool>(gen_internal->decrypt_asset(global->voice_assign_channel_hook_trp), entity_id, proximity);
}

// make demos use our original view angles instead of aimbot / anti aim
EXPORT void __fastcall get_local_view_angles_hook(void* rcx, vec3* ang){
  if(global->aimbot_settings != nullptr){
    if(global->aimbot_settings->silent_aim_serverside){
      *ang = global->untouched_cmd.view_angles;
      return;
    }
  }

  if(config->misc.demosystem_hide_angles){
    *ang = global->untouched_cmd.view_angles;
  }
  else{
    c_base_player* localplayer = utils::localplayer();
    if(localplayer != nullptr)
      *ang = global->last_sent_angle;
  }
}

EXPORT void __fastcall ctfplayershared_onconditionadded_hook(s_shared* rcx, u32 condition){
  assert(rcx != nullptr);
  assert(rcx->outer != nullptr);
  assert(condition >= 0 && condition < TF_COND_COUNT);

  misc::process_condition_prediction(rcx->outer, condition);

  utils::call_fastcall64<void, u32>(gen_internal->decrypt_asset(global->ctfplayershared_onconditionadded_hook_trp), rcx, condition);
}

EXPORT void __fastcall ctfplayershared_onconditionremoved_hook(s_shared* rcx, u32 condition){
  assert(rcx != nullptr);
  assert(rcx->outer != nullptr);
  assert(condition >= 0 && condition < TF_COND_COUNT);

  c_base_player* player = rcx->outer;
  if(player != nullptr){
    c_player_data* data = player->data();
    if(data != nullptr)
      data->tfdata.condition_active_time[condition] = -1.f;
  }

  utils::call_fastcall64<void, u32>(gen_internal->decrypt_asset(global->ctfplayershared_onconditionremoved_hook_trp), rcx, condition);
}

EXPORT i32 __fastcall ctfplayerinventory_getmaxitems_hook(void* rcx){
  if(config->misc.disable_backpack_expander)
    return utils::call_fastcall64<i32>(gen_internal->decrypt_asset(global->ctfplayerinventory_getmaxitems_hook_trp), rcx);

  return 3000;
}

EXPORT __fastcall void create_holiday_lights_hook(void* holiday_light){

}

EXPORT void __fastcall chudcrosshair_getdrawposition_hook(float* x, float* y, bool *behind_camera, vec3 crosshair_offset){
  assert(x != nullptr);
  assert(y != nullptr);

  vec3i screen;
  if(config->visual.draw_crosshair_at_aim_point && global->aimbot_target_index > 0 && math::world2screen(global->aimbot_target_aim_point, screen)){
    *x = screen.x;
    *y = screen.y;
    *behind_camera = false;
  }
  else
    utils::call_fastcall64<void, float*, bool*, vec3>(gen_internal->decrypt_asset(global->chudcrosshair_getdrawposition_hook_trp), x, y, behind_camera, crosshair_offset);
}

EXPORT void __fastcall cnewparticle_effect_render_hook(void* rcx, c_render_context* render_context, bool is_two_pass, c_base_player* camera_object){
  assert(rcx != nullptr);
  assert(render_context != nullptr);
  assert(global->cnewparticle_get_effect_name_addr != nullptr);
  assert(global->cnewparticle_effect_render_retaddr != nullptr);
  if(render_context == nullptr || camera_object == nullptr || __builtin_return_address(0) != global->cnewparticle_effect_render_retaddr){
    utils::call_fastcall64<void, c_render_context*, bool, void*>(gen_internal->decrypt_asset(global->cnewparticle_effect_render_hook_trp), rcx, render_context, is_two_pass, camera_object);
    return;
  }

  bool apply_depth = false;

  // This function passes ecx as (ecx + 16)
  // When the game goes and calls the GetEffectName it does (ecx - 8??!)
  // Hence why we remove 16 from ecx
  void* effect = (void*)(uptr)rcx - 32;

  if(config->visual.show_icons_thru_walls){
    if(effect != nullptr && camera_object != nullptr){
      STACK_CHECK_START;
      i8* name = utils::call_fastcall64<i8*>(global->cnewparticle_get_effect_name_addr, (void*)(uptr)effect + (uptr)0x20);
      STACK_CHECK_END;
      if(name != nullptr){
        // Since the owner entity in the particle class is somewhere in the world. We'll do this
        bool is_red_enemy = camera_object->team() != 2;
        bool is_blu_enemy = camera_object->team() != 3;

        // Fix config error
        if(!config->visual.show_friendly_team_icons && !config->visual.show_enemy_team_icons && !config->visual.show_both_team_icons)
          config->visual.show_both_team_icons = true;

        // We won't do any logic for the enemy team since it's done by default.
        if(config->visual.show_both_team_icons)
          is_red_enemy = is_blu_enemy = true;
        else if(config->visual.show_friendly_team_icons){
          is_blu_enemy = !is_blu_enemy;
          is_red_enemy = !is_red_enemy;
        }

        u32 name_hash = HASH_RT(name);
        switch(name_hash){
          default: break;
          case HASH("vaccinator_red_buff1"):
          case HASH("vaccinator_red_buff2"):
          case HASH("vaccinator_red_buff3"):
          {
            if(config->visual.show_vaccinator_icons_thru_walls){
              if(is_red_enemy)
                apply_depth = true;
            }

            break;
          }
          case HASH("vaccinator_blue_buff1"):
          case HASH("vaccinator_blue_buff2"):
          case HASH("vaccinator_blue_buff3"):
          {
            if(config->visual.show_vaccinator_icons_thru_walls){
              if(is_blu_enemy)
                apply_depth = true;
            }

            break;
          }
          case HASH("powerup_icon_strength_red"):
          case HASH("powerup_icon_haste_red"):
          case HASH("powerup_icon_regen_red"):
          case HASH("powerup_icon_resist_red"):
          case HASH("powerup_icon_vampire_red"):
          case HASH("powerup_icon_reflect_red"):
          case HASH("powerup_icon_precision_red"):
          case HASH("powerup_icon_agility_red"):
          case HASH("powerup_icon_knockout_red"):
          case HASH("powerup_icon_king_red"):
          case HASH("powerup_icon_plague_red"):
          case HASH("powerup_icon_supernova_red"):
          {
            if(config->visual.show_mannpower_icons_thru_walls){
              if(is_red_enemy)
                apply_depth = true;
            }

            break;
          }
          case HASH("powerup_icon_strength_blue"):
          case HASH("powerup_icon_haste_blue"):
          case HASH("powerup_icon_regen_blue"):
          case HASH("powerup_icon_resist_blue"):
          case HASH("powerup_icon_vampire_blue"):
          case HASH("powerup_icon_reflect_blue"):
          case HASH("powerup_icon_precision_blue"):
          case HASH("powerup_icon_agility_blue"):
          case HASH("powerup_icon_knockout_blue"):
          case HASH("powerup_icon_king_blue"):
          case HASH("powerup_icon_plague_blue"):
          case HASH("powerup_icon_supernova_blue"):
          {
            if(config->visual.show_mannpower_icons_thru_walls){
              if(is_blu_enemy)
                apply_depth = true;
            }

            break;
          }
          case HASH("laser_sight_beam"):
          {
            if(config->visual.sniper_los_enabled && config->visual.sniper_los_ignore_z)
              apply_depth = true;

            break;
          }
        }
      }
    }
  }

  if(apply_depth)
    render_context->depth_range(0.f, 0.02f);

  utils::call_fastcall64<void, c_render_context*, bool, void*>(gen_internal->decrypt_asset(global->cnewparticle_effect_render_hook_trp), rcx, render_context, is_two_pass, camera_object);

  if(apply_depth)
    render_context->depth_range(0.f, 1.f);
}

EXPORT void __fastcall csniperdot_clientthink_hook(c_base_entity* rcx){
  assert(rcx != nullptr);

  c_base_entity* entity = (c_base_entity*)((uptr)rcx - (uptr)0x18);

  bool backup_mvm_playing = utils::is_playing_mvm();
  bool modify             = false;

  if(entity == nullptr || entity->get_client_class()->id != CSniperDot){
    utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->csniperdot_clientthink_hook_trp), rcx);
    return;
  }

  c_base_entity* owner = entity->get_owner();
  if(owner == nullptr){
    utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->csniperdot_clientthink_hook_trp), rcx);
    return;
  }

  if(global->localplayer == nullptr || owner == global->localplayer){
    utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->csniperdot_clientthink_hook_trp), rcx);
    return;
  }

  if(config->visual.sniper_los_enabled){
    modify = true;
    if(config->visual.sniper_los_friendly_only)
      modify = !(global->localplayer_team != owner->team());
    else if(config->visual.sniper_los_enemy_only)
      modify = !(global->localplayer_team == owner->team());
  }

  if(modify){
    global->laser_dot_team_backup = entity->team();
    entity->team() = 3;
    utils::set_playing_mvm(true);
  }

  utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->csniperdot_clientthink_hook_trp), rcx);

  if(modify){
    entity->team() = global->laser_dot_team_backup;
    utils::set_playing_mvm(backup_mvm_playing);
  }
}

EXPORT void __fastcall particlesystem_setcontrolpoint_hook(void* rcx, i32 point, float* pos){
  assert(rcx != nullptr);
  assert(global->csniperdot_clientthink_retaddr1 != nullptr);

  if(global->csniperdot_clientthink_retaddr1 == __builtin_return_address(0)){
    if(point == 2){
      if(global->laser_dot_team_backup == 2){
        pos[0] = 255.f;
        pos[1] = pos[2] = 0.f;
      }
    }
  }

  utils::call_fastcall64<void, i32, float*>(gen_internal->decrypt_asset(global->particlesystem_setcontrolpoint_hook_trp), rcx, point, pos);
}

// Remove underwater fog
// I am pretty sure since the new fog system was added to TF2 there's a new parameter and it might be related to volumetric fog.
EXPORT void __fastcall r_setfogvolumestate_hook(i32 fog_volume, bool use_height_fog){
  if(config->visual.no_fog)
    return;

  utils::call_fastcall64_raw<void, i32, bool>(gen_internal->decrypt_asset(global->r_setfogvolumestate_hook_trp), fog_volume, use_height_fog);
}

// Remove fog from the world.
EXPORT void __fastcall get_fog_enable_hook(){
  if(config->visual.no_fog)
    return;

  utils::call_fastcall64_raw<void>(gen_internal->decrypt_asset(global->get_fog_enable_hook_trp));
}

EXPORT void __fastcall client_entity_list_on_add_entity_hook(void* rcx, void* handle_entity, void* handle){
  utils::call_fastcall64<void, void*, void*>(gen_internal->decrypt_asset(global->client_entity_list_on_add_entity_hook_trp), rcx, handle_entity, handle);
  if(handle_entity == nullptr)
    return;

  c_base_entity* entity = utils::virtual_call64<7, c_base_entity*>(handle_entity);
  utils::create_entity_data(entity);
}

EXPORT void __fastcall client_entity_list_on_remove_entity_hook(void* rcx, void* handle_entity, void* handle){
  utils::call_fastcall64<void, void*, void*>(gen_internal->decrypt_asset(global->client_entity_list_on_remove_entity_hook_trp), rcx, handle_entity, handle);
  if(handle_entity == nullptr)
    return;

  c_base_entity* entity = utils::virtual_call64<7, c_base_entity*>(handle_entity);
  if(entity != nullptr){
    i32 index = entity->get_index();
    if(index == -1)
      return;

    global->memory_manager.release_data(index);
    //DBG("[+] %s (%i) [%i] was removed\n", entity->get_client_class()->name, index, global->memory_manager.get_allocated_bytes());
  }
}

// Rather than have the game do its really slow ass string comparison.
// We calculate our own table but use hashes instead.
EXPORT c_cvar* __fastcall ccvar_find_command_hook(void* rcx, i8* name){
  assert(rcx != nullptr);
  if(name == nullptr)
    return nullptr;

  if(!global->cvar_hash_table->count){
    c_cvar* list = global->cvar->get_commands();
    while(list != nullptr){
      if(list == nullptr)
        break;

      // CVars in source aren't usually deleted. I could be wrong..
      global->cvar_hash_table->table[global->cvar_hash_table->count].pointer = list;
      global->cvar_hash_table->table[global->cvar_hash_table->count].hash = HASH_RTL(list->name);
      global->cvar_hash_table->count++;
      list = list->next;
    }
    DBG("[!] Created cvar hash map %i\n", global->cvar_hash_table->count);
  }

  // I did a for loop test 1 to 1,000,000.
  // This hash check was so fast our time function isn't even accurate enough to pick it up.
  // Meanwhile, valve's function took 27.6 seconds to finish. (locking the game)
  u32 name_hash = HASH_RTL(name);
  for(i32 i = 0; i < 8912; i++){
    if(!global->cvar_hash_table->table[i].hash)
      continue;

    if(name_hash == global->cvar_hash_table->table[i].hash)
      return (c_cvar*)global->cvar_hash_table->table[i].pointer;
  }

  // Valve has left lots of unused code that attempts to get pointers to cvars that are (dev/beta) only.
  // Or there is certain "commands" like build, destroy, voicemenu, taunt, and many others that are handled on the server.
  // They aren't actual commands in the cvar class. Like, if you type "legos" in console. That gets sent to the server.
  return nullptr;
}

EXPORT void __fastcall crenderview_renderview_hook(void* rcx, s_view_setup* view, i32 flags, i32 what_to_draw){
  assert(rcx != nullptr);
  render_cam->render_view(rcx, view);
  freecam->render_view(view, &what_to_draw);
  wrangler_helper->render_view(view);
  thirdperson->render_view(view, &what_to_draw);

  utils::call_fastcall64<void, s_view_setup*, i32, i32>(gen_internal->decrypt_asset(global->crenderview_renderview_hook_trp), rcx, view, flags, what_to_draw);
  
  thirdperson->calculate_view_state(view);
  render_cam->draw();
}

EXPORT bool __fastcall input_cam_is_thirdperson_hook(void* rcx){
  if(freecam->is_active() || thirdperson->is_active() || wrangler_helper->is_overriding_view)
    return true;
  
  return utils::call_fastcall64<bool>(gen_internal->decrypt_asset(global->input_cam_is_thirdperson_hook_trp), rcx);
}

// Something occured in 64-bit TF2 where the model becomes invisible when sv_cheats is enabled.
// Hooking this function fixes it.
EXPORT bool __fastcall ctfgamerules_allowinthirdpersoncamera_hook(c_game_rules* rcx){
  if(freecam->is_active() || thirdperson->is_active() || wrangler_helper->is_overriding_view)
    return true;

  return rcx->show_match_summary() || rcx->medieval();
}

EXPORT bool __fastcall inventorymanager_loadpreset_hook(void* rcx, u16 unk1, u8 unk2){
  // There's a rate-limit inside this function.
  // We're simply trying to disable it.
  // Found by searching for string(client.dll): Loadout change denied. Changing presets too quickly.\n
  *(float*)((uptr)rcx + (uptr)0xC4) = 0.f;
  return utils::call_fastcall64<bool, u16, u8>(gen_internal->decrypt_asset(global->inventorymanager_loadpreset_hook_trp), rcx, unk1, unk2);
}

EXPORT i8* __fastcall ctfweaponbase_getshootsound_hook(c_base_entity* rcx, i32 index){
  assert(rcx != nullptr);

  if(!config->misc.mvm_giant_weapon_sounds || rcx->get_index() != global->localplayer_weapon_index)
    return utils::call_fastcall64<i8*, i32>(gen_internal->decrypt_asset(global->ctfweaponbase_getshootsound_hook_trp), rcx, index);

  static i8 sound_name[64];
  memset(sound_name, 0, sizeof(sound_name));

  i32 old_team = rcx->team();
  rcx->team() = 4; // If we're playing mvm and the entity is considered a miniboss then it'll set the team to this.
  i8* result = utils::call_fastcall64<i8*, i32>(gen_internal->decrypt_asset(global->ctfweaponbase_getshootsound_hook_trp), rcx, index);
  rcx->team() = old_team;

  u32 hash = HASH_RT(result);
  if(hash == HASH("Weapon_GrenadeLauncher.Single") || hash == HASH("Weapon_GrenadeLauncher.SingleCrit")){
    formatA(sound_name, XOR("MVM.GiantDemoman_Grenadeshoot"));
    return sound_name;
  }
  else if(hash == HASH("Weapon_FlameThrower.Fire")){
    formatA(sound_name, XOR("MVM.GiantPyro_FlameStart"));
    return sound_name;
  }
  else if(hash == HASH("Weapon_FlameThrower.FireLoop")){
    formatA(sound_name, XOR("MVM.GiantPyro_FlameLoop"));
    return sound_name;
  }

  return result;
}

EXPORT void __fastcall add_resist_shield_hook(void* shield, c_base_player* player, u32 cond){
  assert(player != nullptr);
  
  if(!config->visual.use_alt_vaccinator_shields){
    utils::call_fastcall64<void, void*, u32>(gen_internal->decrypt_asset(global->add_resist_shield_hook_trp), shield, player, cond);
    return;
  }

  i32 old_team            = player->team();
  bool backup_mvm_playing = utils::is_playing_mvm();

  player->team() = 3;
  utils::set_playing_mvm(true);

  utils::call_fastcall64<void, void*, u32>(gen_internal->decrypt_asset(global->add_resist_shield_hook_trp), shield, player, cond);
  
  player->team() = old_team;
  utils::set_playing_mvm(backup_mvm_playing);
}

// This doesn't have a thisptr, so rcx is _class, adding rcx to this will fuckup the slot parameter.
EXPORT i64 __fastcall generate_equip_region_conflict_mask_hook(i32 _class, i32 slot_up_to, i32 slot){
  if(config->misc.unequip_regions)
    return 0;
  
  return utils::call_fastcall64<i64, i32, i32>(gen_internal->decrypt_asset(global->generate_equip_region_conflict_mask_hook_trp), (void*)_class, slot_up_to, slot);
}

EXPORT void* __fastcall equippable_items_for_slotgenerator_constructor_hook(void* rcx, i32 _class, i32 slot, i32 region_mask, i32 unflags){
  if(config->misc.unequip_regions)
    region_mask = 0;

  return utils::call_fastcall64<void*, i32, i32, i32, i32>(gen_internal->decrypt_asset(global->equippable_items_for_slotgenerator_constructor_hook_trp), rcx, _class, slot, region_mask, unflags);
}

EXPORT void __fastcall inventory_manager_updateinv_equipped_state_hook(void* rcx, void* inv, i64 itemid, u16 _class, u16 slot){
  if(config->misc.unequip_regions && __builtin_return_address(0) != global->updateinvequippedstate_retaddr)
    return;
  
  utils::call_fastcall64<void, void*, i64, u16, u16>(gen_internal->decrypt_asset(global->inventory_manager_updateinv_equipped_state_hook_trp), rcx, inv, itemid, _class, slot);
}

EXPORT void* __fastcall set_context_think_hook(void* rcx, void* unk, void* func, float time, i8* context){
  if(context != nullptr){
    u32 hash = HASH_RT(context);
    if(hash == HASH("LAUNCH_BALL_THINK") || hash == HASH("TOSS_JAR_THINK"))
      global->animation_fire_time = time;
  }

  return utils::call_fastcall64<void*, void*, void*, float, i8*>(gen_internal->decrypt_asset(global->set_context_think_hook_trp), rcx, unk, func, time, context);
}

EXPORT bool __fastcall is_blacklisted_hook(void* rcx, s_game_server_item* item){
  STACK_CHECK_START;
  bool result = utils::call_fastcall64<bool, s_game_server_item*>(gen_internal->decrypt_asset(global->is_blacklisted_hook_trp), rcx, item);
  STACK_CHECK_END;

  i8 server_ip[64];
  formatA(server_ip, XOR("%u.%u.%u.%u"), *(u8*)((uptr)&item->ipaddr.ip_bytes + 3), *(u8*)((uptr)&item->ipaddr.ip_bytes + 2), *(u8*)((uptr)&item->ipaddr.ip_bytes + 1), *(u8*)((uptr)&item->ipaddr.ip_bytes));

  if(utils::should_filter_server_ip(server_ip, item, true)){
    DBG("[!] Blocked server: %s - %s\n", server_ip, item->server_name);
    return false;
  }

  return result;
}

EXPORT void* __fastcall cstudiohdr_seqdesc_hook(void* rcx, i32 seq){
  assert(global->nil_seq_desc_addr != nullptr);

  STACK_CHECK_START;
  void* result = utils::call_fastcall64<void*, i32>(gen_internal->decrypt_asset(global->cstudiohdr_seqdesc_hook_trp), rcx, seq);
  STACK_CHECK_END;

  if(result == nullptr){
    static bool notify_user_box = false;

    if(!notify_user_box){
      PRODUCT_STATUS("TF2_SEQDESC_CRASH");
      I(MessageBoxA)(nullptr, XOR("Looks like you ran into the nullptr bug with studiohdr_t::pSeqdesc, it's not related to RijiN, you can click OK, and maybe you won't crash.\nBut I wouldn't get your hopes up."), WXOR("Darn"), 0);
      notify_user_box = true;
    }

    return global->nil_seq_desc_addr;
  }

  return result;
}

// Disable extrapolation.
EXPORT bool __fastcall engineclient_ispaused_hook(void* rcx){
  void* ret_addr = __builtin_return_address(0);
  if(ret_addr == global->engineclient_ispaused_retaddr)
    return true;

  return utils::call_fastcall64<bool>(gen_internal->decrypt_asset(global->engineclient_ispaused_hook_trp), rcx);
}

// - dev only hooks -
// These are still outdated just an FYI - April 8th, 2025.
#if defined(DEV_MODE)
EXPORT float __fastcall physics_object_getdragindir_hook(void* rcx, vec3* vel){
  vec3* drag_basis = (vec3*)((uptr)rcx + (uptr)0x14);
  //DBG("[!] drag_basis: %f, %f, %f\n", drag_basis->x, drag_basis->y, drag_basis->z);
  return 0.f;//utils::call_fastcall<float, vec3*>(gen_internal->decrypt_asset(global->physics_object_getdragindir_hook_trp), ecx, edx, vel);
}

EXPORT float __fastcall physics_object_getangdir_hook(void* rcx, vec3* vel){
  vec3* drag_basis = (vec3*)((uptr)rcx + (uptr)0x20);
  //DBG("[!] drag_ang_basis: %f, %f, %f\n", drag_basis->x, drag_basis->y, drag_basis->z);
  return 0.f;//utils::call_fastcall<float, vec3*>(gen_internal->decrypt_asset(global->physics_object_getangdir_hook_trp), ecx, edx, vel);
}
#endif