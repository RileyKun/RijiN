#include "link.h"

#define CALL_CL_MOVE(extra_samples, final_tick) \
  packet_manager->compute_send_packet_val(final_tick); \
  packet_manager->set_breakpoint(GetCurrentThread(), EH_CL_MOVE_SP_LOCATION_ZEROING); \
  packet_manager->set_breakpoint(GetCurrentThread(), EH_CL_MOVE_SP_LOCATION_SET_CHOKE); \
  utils::call_fastcall64_raw<void, float, bool>(gen_internal->decrypt_asset(global->cl_move_hook_trp), extra_samples, final_tick);

EXPORT void __fastcall cl_move_hook(float extra_samples, bool final_tick){
  CALL_CL_MOVE(extra_samples, final_tick);
}

EXPORT void __fastcall create_move_hook(void* rcx, c_user_cmd* cmd){
  assert(rcx != nullptr);
  assert(cmd != nullptr);

  c_base_player* localplayer = utils::localplayer();
  if(!global->valid() || localplayer == nullptr || cmd->command_number == 0)
    return;

  global->pre_create_move(cmd);
  packet_manager->post_send_packet_init();
  interp_manager->pre_create_move(cmd);
  command_manager->pre_create_move(cmd);
  freecam->pre_create_move_prediction(cmd);

  global->aimbot_settings     = utils::get_aimbot_settings();
  global->triggerbot_settings = utils::get_triggerbot_settings();

  if(!localplayer->is_alive())
    return;

  acm->pre_create_move(cmd);

  c_base_weapon* wep = localplayer->get_weapon();

  movement->pre_predicted_create_move(localplayer, wep, cmd);

  if(!engine_prediction->start_prediction(localplayer, cmd)){
    packet_manager->post_create_move_prediction();
    return;
  }

  {
    bool triggerbot_running = false;
    bool aimbot_running     = raytrace_aimbot->run();
    if(!aimbot_running){
      if(global->aimbot_target_reset_time <= math::time())
        global->aimbot_target_index = -1;

      triggerbot_running = triggerbot->run();
    }

    misc::nospread(localplayer, aimbot_running, true, triggerbot_running);

    bool firing              = localplayer->will_fire_this_tick();
    global->firing_this_tick = firing;
    if(aimbot_running || firing){
      misc::norecoil(localplayer);
      misc::nospread(localplayer, aimbot_running, false, triggerbot_running);

      if(!aimbot_running) // Aimbot already does this.
        misc::auto_pistol();

      packet_manager->on_input();
    }

    command_manager->create_move(cmd, true);
    acm->post_create_move(cmd, false);
    math::rebase_movement(cmd, global->original_cmd);
    acm->post_create_move(cmd, true);
  }

  packet_manager->post_create_move_prediction();
  engine_prediction->end_prediction(cmd);

  global->post_create_move(cmd, packet_manager->choking());
  interp_manager->post_create_move(cmd);
}

EXPORT void __fastcall frame_stage_notify_hook(u32 stage){
  if(!utils::is_in_game()){
    utils::call_fastcall64_raw<void, u32>(gen_internal->decrypt_asset(global->frame_stage_notify_hook_trp), stage);
    return;
  }

  utils::verify_player_data();
  player_list_handler->think();
  player_list_handler->verify_selected_player();
  lag_compensation->frame_stage_notify(stage);
  cheat_detection->frame_stage_notify(stage);

  utils::call_fastcall64_raw<void, u32>(gen_internal->decrypt_asset(global->frame_stage_notify_hook_trp), stage);
}

EXPORT void __fastcall check_for_pure_server_whitelist_hook(void* crc){
  if(config->misc.pure_bypass){
    DBG("[!] bypassed sv_pure check\n");
    return;
  }

  utils::call_fastcall64_raw<void, void*>(gen_internal->decrypt_asset(global->check_for_pure_server_whitelist_hook_trp), crc);
}

EXPORT void __fastcall override_view_hook(void* rcx, s_view_setup* view_setup){
  assert(view_setup != nullptr);

  global->update_local_data();
  if(config->visual.no_visual_recoil)
    recoil_control->correct_view_punch(view_setup->angles, true, recoil_control->should_view_inherit());

  global->view_setup = view_setup;
  utils::call_fastcall64<void, s_view_setup*>(gen_internal->decrypt_asset(global->override_view_hook_trp), rcx, view_setup);
}

EXPORT void __fastcall engine_paint_hook(void* rcx, u32 mode){
  assert(rcx != nullptr);

  interp_manager->update();
  if(global->valid()){
    static i32 last_frame_count;

    // Make a new cache every new frame.
    if(globaldata->frame_count != last_frame_count)
      global->engine_tool->get_world_to_screen_matrix_for_view(global->view_setup, &global->view_matrix);

    for(i32 i = 1; i <= globaldata->max_clients; i++){
      c_base_player* player = global->entity_list->get_entity(i);
      if(player == nullptr)
        continue;

      c_player_data* data = player->data();
      if(data == nullptr)
        continue;

      data->obb_min                = player->obb_mins();
      data->obb_max                = player->obb_maxs();
      data->transform              = player->rgfl_coordinate_frame();
      data->has_transform          = true;
    }

    last_frame_count = globaldata->frame_count;
  }

  utils::call_fastcall64<void, u32>(gen_internal->decrypt_asset(global->engine_paint_hook_trp), rcx, mode);
}

EXPORT void __fastcall paint_traverse_hook(void* rcx, uptr panel, bool repaint, bool force_frame){
  assert(rcx != nullptr);

  i8* panel_name = global->panel->get_name(panel);
  assert(panel_name != nullptr);

  u32 panel_hash = HASH_RT(panel_name);
  if(privacy_mode->paint_traverse_hook(panel))
    return;

  global->paint_traverse(panel_hash, panel);
  utils::call_fastcall64<void, uptr, bool, bool>(gen_internal->decrypt_asset(global->paint_traverse_hook_trp), rcx, panel, repaint, force_frame);
}

EXPORT bool __fastcall cnetchan_sendnetmsg_hook(void* rcx, c_net_message& net_msg, bool rel, bool voice){
  assert(rcx != nullptr);
  assert(net_msg.get_name() != nullptr);

  u32 name_hash = HASH_RT(net_msg.get_name());
  if(name_hash == HASH("clc_VoiceData"))
    voice = true;

  return utils::call_fastcall64<bool, c_net_message&, bool, bool>(gen_internal->decrypt_asset(global->cnetchan_sendnetmsg_hook_trp), rcx, net_msg, rel, voice);
}

// This function will jumping issues caused by the function hook above.
EXPORT float __fastcall cl_client_interp_amount_hook(){
  return 0.0f;
}

EXPORT void __fastcall maintain_sequence_transition_hook(void* rcx, void* bone_setup, void* cycle, void* pos, void* q){
  assert(rcx != nullptr);
}

EXPORT bool __fastcall fire_event_hook(void* rcx, c_game_event* event){
  assert(rcx != nullptr);
  assert(event != nullptr);

  u32 name_hash = HASH_RT(event->get_name());

  if(name_hash == HASH("server_spawn") || name_hash == HASH("client_beginconnect")){
    DBG("[!] DISCONNECTED!\n");
    interp_manager->reset();
    acm->reset();
    global->reset();
    thirdperson->reset();
    freecam->reset();
    command_manager->reset();
    triggerbot->reset();
  }
  else if(name_hash == HASH("player_disconnect")){
    i32 id = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));
    DBG("[!] %i data reset\n", id);

    utils::reset_player_data(id);
    utils::reset_lag_comp_data(id);
  }
  else if(name_hash == HASH("player_connect_client")){
    i32 id = global->engine->get_player_for_user_id(event->get_int(XOR("index")));
    DBG("[!] %i data reset\n", id);

    utils::reset_player_data(id);
    utils::reset_lag_comp_data(id);
  }
  else if(name_hash == HASH("player_death")){
    i32 attacked = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));

    dormant_manager->on_death(attacked);
    utils::reset_lag_comp_data(attacked);
  }
  else if(name_hash == HASH("player_spawn")){
    i32 index = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));

    dormant_manager->on_spawned(index);
    utils::reset_lag_comp_data(index);
  }
  else if(name_hash == HASH("teamplay_round_start")){
    triggerbot->reset();
  }

  if(privacy_mode->handle_game_event(event))
    return true;

  return utils::call_fastcall64<bool, void*>(gen_internal->decrypt_asset(global->fire_event_hook_trp), rcx, event);
}

EXPORT void __fastcall cbaseviewmodel_calcviewmodelview_hook(c_base_entity* rcx, c_base_player* owner, vec3& eye_pos, vec3& angles){
  assert(rcx != nullptr);
  if(owner != global->localplayer){
    utils::call_fastcall64<void, void*, vec3&, vec3&>(gen_internal->decrypt_asset(global->cbaseviewmodel_calcviewmodelview_hook_trp), rcx, owner, eye_pos, angles);
    return;
  }

  vec3 old_angles = angles;
  if(config->visual.no_visual_recoil){
    angles   -= owner->punch_angle();
    angles.z = 0.f;
  }

  utils::call_fastcall64<void, void*, vec3&, vec3&>(gen_internal->decrypt_asset(global->cbaseviewmodel_calcviewmodelview_hook_trp), rcx, owner, eye_pos, angles);

  // We're trying to get rid of the viewmodel angles being affected by recoil.
  // This angle will be used in FireBullets which will make our bullets look off.
  angles = old_angles;
}

EXPORT bool __fastcall input_cam_is_thirdperson_hook(void* rcx){
  if(freecam->is_active() || thirdperson->is_active())
    return true;

  return utils::call_fastcall64<bool>(gen_internal->decrypt_asset(global->input_cam_is_thirdperson_hook_trp), rcx);
}

EXPORT c_user_cmd* __fastcall cinput_get_user_cmd_hook(void* rcx, i32 seq){
  assert(rcx != nullptr);
  assert(global->input != nullptr);
  return &global->input->commands[seq % 90];
}

EXPORT void __fastcall hud_chatline_insert_and_colorize_text_hook(void* rcx, wchar_t* buf, i32 client_index){
  if((config->misc.block_server_chat_messages /*|| config->misc.streamer_mode*/) && client_index == 0){
    DBG("[!] Blocked server chat message from being printed in chat.\n");
    return;
  }

  void* ret_addr = __builtin_return_address(1);
  if(global->skip_chatline_insert_hook || buf == nullptr || buf[0] == L' ' || client_index == 0){
    utils::call_fastcall64<void, wchar_t*, i32>(gen_internal->decrypt_asset(global->hud_chatline_insert_and_colorize_text_hook_trp), rcx, buf, client_index);
    return;
  }

  bool steam_friend   = cheat_detection->is_friend(client_index) || client_index == global->localplayer_index;
  bool cheater        = cheat_detection->is_cheating(client_index);
  bool cheater_bot    = cheat_detection->is_cheating_bot(client_index);
  c_base_player* player = global->entity_list->get_entity(client_index);
  c_player_data* data   = utils::get_player_data(client_index);

  uptr len                    = wcslen(buf);
  uptr tag_colour             = 0xFFFFFF;
  std::wstring tag            = L"";
  bool message_newlined       = false;
  u32  message_newline_count  = 0;

  // Remove newlines from the message.
  for(u32 i = 0; i < len; i++){
    if(buf[i] == L'\n' || buf[i] == L'\r'){

      // The game will actually have some newlines in certain chat messages.
      if(message_newline_count >= 2){
        buf[i]           = L'\x1';
        message_newlined = true;
        //if(data != nullptr)
        //  data->sent_newlined_chat = true;
      }

      message_newline_count++;
    }
  }

  s_player_info info;
  global->skip_get_player_info_hook = true;
  if(global->engine->get_player_info(client_index, &info)){

    // Fixes the color issues with chat message.
    if(buf[0] == L'\x2')
      buf[0] = L'\x3';

    static wchar_t name_buff[64];
    memset(name_buff, 0, sizeof(name_buff));
    convert::str2wstr(info.name, name_buff, sizeof(name_buff));

    uptr name_len = wcslen(name_buff);

    i32 start = -1;
    i32 end   = -1;
    if(utils::find_start_and_end(buf, name_buff, &start, &end)){

      // Censor our name and our friends name from chat.
      if(privacy_mode->should_censor_chat(client_index)){
        for(i32 i = start; i <= end; i++){
          buf[i] = L'*';
        }
      }

      // Prevent team colour from "leaking" into the chat message.
      if(start == 1){
        if(name_len + 1< len)
          buf[name_len + 1] = L'\x1';
      }
      else{
        if(name_len + (start) < len)
          buf[name_len + (start)] = L'\x1';
      }

      // Prevent servers from applying annoying text colors in the chat message.
      for(u32 i = end + 2; i < len; i++){
        if(buf[i] == L'\x8' || buf[i] == L'\x7')
          buf[i] = L'\x1';
      }

      // Make our tags!
      if(client_index == global->engine->get_local_player()){
        if(config->misc.chat_tags_localplayer){
          tag = WXOR(L"[You] ");
          tag_colour = 0x60FF64;
        }
      }
      else if(steam_friend){
        if(config->misc.chat_tags_friends){
          tag = WXOR(L"[Friend] ");
          tag_colour = 0x60FF64;
        }
      }
      else{
        if(config->misc.chat_tags_cheaters){
          if(cheater_bot){
            tag = WXOR(L"[Bot] ");
            tag_colour = 0xFF2D1E;
          }
          else if(cheater){
            tag = WXOR(L"[Cheater] ");
            tag_colour = 0xFF2D1E;
          }
        }
      }
    }
  }

  global->skip_get_player_info_hook = false;

  static wchar_t* new_buf = malloc(XOR32(4096));
  assert(new_buf != nullptr);
  memset(new_buf, 0, XOR32(4096));

  if(config->misc.chat_tags)
    wsprintfW(new_buf, WXOR(L"\x7%06X%ls\x3%ls"), tag_colour, tag.c_str(), buf);
  else
    wsprintfW(new_buf, WXOR(L"\x3%ls"), buf);

  utils::call_fastcall64<void, wchar_t*, i32>(gen_internal->decrypt_asset(global->hud_chatline_insert_and_colorize_text_hook_trp), rcx, new_buf, client_index);
}

EXPORT bool __fastcall engine_get_player_info_hook(void* rcx, i32 entity_id, s_player_info* info){
  assert(rcx != nullptr);

  bool result = utils::call_fastcall64<bool, i32, s_player_info*>(gen_internal->decrypt_asset(global->engine_get_player_info_hook_trp), rcx, entity_id, info);
  if(!result || info == nullptr || entity_id == 0 || entity_id > globaldata->max_clients || global->skip_get_player_info_hook)
    return result;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return result;

  bool steam_friend     = (entity_id == global->engine->get_local_player()) || cheat_detection->is_friend(entity_id);
  c_base_player* player = global->entity_list->get_entity(entity_id);
  c_player_data* data   = utils::get_player_data(entity_id);

  privacy_mode->engine_get_player_info_hook(entity_id, info, true); // Far as I am considered sid writing is allowed.
  cheat_detection->engine_get_player_info_hook(entity_id, info);
  return result;
}

EXPORT i32 __fastcall s_startsound_hook(start_sound_params& params){
  if(params.sfx != nullptr)
    dormant_manager->process_entity_sounds(params);

  return utils::call_fastcall64_raw<i32, start_sound_params&>(gen_internal->decrypt_asset(global->s_startsound_hook_trp), params);
}

EXPORT void __fastcall ccsviewrender_perform_flashbangeffect_hook(void* rcx, s_view_setup* setup){
  assert(rcx != nullptr);
  if(config->visual.no_flash){
    c_base_player* localplayer = utils::localplayer();
    if(localplayer != nullptr)
      localplayer->flash_bang_time() = -1.f;
  }

  utils::call_fastcall64<void, s_view_setup*>(gen_internal->decrypt_asset(global->ccsviewrender_perform_flashbangeffect_hook_trp), rcx, setup);
}

EXPORT bool __fastcall clientmodecsnormal_canrecorddemo_hook(i8* error_msg, i32 length){
  return true;
}

EXPORT void __fastcall crenderview_renderview_hook(void* rcx, s_view_setup* view, i32 flags, i32 what_to_draw){
  freecam->render_view(view, &what_to_draw);
  thirdperson->render_view(view, &what_to_draw);
  utils::call_fastcall64<void, s_view_setup*, i32, i32>(gen_internal->decrypt_asset(global->crenderview_renderview_hook_trp), rcx, view, flags, what_to_draw);
  thirdperson->calculate_view_state(view);
}

EXPORT bool __fastcall is_blacklisted_hook(void* rcx, s_game_server_item* item){
  i8 server_ip[1024] = {0};
  I(wsprintfA)(server_ip, XOR("%u.%u.%u.%u"), *(u8*)((uptr)&item->ipaddr.ip_bytes + 3), *(u8*)((uptr)&item->ipaddr.ip_bytes + 2), *(u8*)((uptr)&item->ipaddr.ip_bytes + 1), *(u8*)((uptr)&item->ipaddr.ip_bytes));

  u32 ip_hash = HASH_RT(server_ip);

#if defined(DEV_MODE)
  static u32 i = 0;
  static u32 ip_table[8912];
#endif

  if(utils::should_filter_server_ip(server_ip, item, true)){
    #if defined(DEV_MODE)
      bool ok = true;
      for(i32 j = 0; j < i; j++){
        if(ip_table[j] == ip_hash){
          ok = false;
          break;
        }
      }

      if(ok){
        ip_table[i] = ip_hash;
        i++;

        i8 buf[1024];
        wsprintfA(buf, XOR("%s\n"), server_ip);
        file::write_text(WXOR(L"stupid_russians.txt"), buf, true);
      }
    #endif
    return false;
  }

  return utils::call_fastcall64<bool, s_game_server_item*>(gen_internal->decrypt_asset(global->is_blacklisted_hook_trp), rcx, item);
}