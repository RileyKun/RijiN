#include "link.h"

EXPORT void __fastcall create_move_hook(void* rcx, s_user_cmd* cmd){
  assert(rcx != nullptr);
  assert(cmd != nullptr);

  c_base_player* localplayer = utils::localplayer();
  if(!global->valid() || localplayer == nullptr || cmd->command_number == 0)
    return;

  vec3 cur_view_angle = cmd->view_angles;

  utils::store_cmd_data(cmd);
  utils::update_local_data();

  // Dump bSendPacket stack pointer using exception handler
  // When player is alive, place L0BPDR0 hook on push rdi at chlclient createmove
  // Dump RSP and happy days 
  // (dont move this code -senator)
  if(localplayer->is_alive() && !global->unloading){
    if(global->bsendpacket_rdi_rsp == 0){
      CONTEXT ctx;
      ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
      GetThreadContext(GetCurrentThread(), &ctx);

      if(ctx.Dr0 == 0){
        ctx.Dr0 = global->chlclient_createmove_push_rdi;
        ctx.Dr7 = (1 << 0); // L0BPDR0 Enable Flag
        SetThreadContext(GetCurrentThread(), &ctx);
  
        DBG("[!] EH: DR0 bSendPacket HOOK PLACED ON %p\n", global->chlclient_createmove_push_rdi);
      }
    }
    else{
      register u64 rsp asm("rsp");

      u8* sp_sendpacket = (u8*)global->bsendpacket_rdi_rsp;
      if(*sp_sendpacket == 0 || *sp_sendpacket == 1)
        global->send_packet = (bool*)sp_sendpacket;
      else{
        global->bsendpacket_rdi_rsp = 0;
        global->send_packet = nullptr;
      }
    }
  }

  c_base_weapon* wep = localplayer->get_weapon();
  packet_manager->pre_run(); 

  misc::bunnyhop();
  auto_strafe->run();
  antiaim->pre_prediction();

  freecam->run_createmove(cmd);
  engine_prediction->run();
  {
    resolver->process_manual_input();
    bool aimbot_running = raytrace_aimbot->run();
    if(!aimbot_running){
      if(global->aimbot_target_reset_time <= math::time())
        global->aimbot_target_index = -1;
    }

    misc::auto_pistol();
    misc::process_attack();
    accuracy->run();

    bool firing              = localplayer->will_fire_this_tick();
    global->firing_this_tick = firing;
    if(aimbot_running || firing){
      antiaim->run_on_fire();
      if(aimbot_running)
        resolver->on_shot(raytrace_aimbot->aim_target);

      // send this tick and next tick
      if(packet_manager->choking())
        packet_manager->force_send_packet(true);
      else{
        packet_manager->force_send_packet(false);
        packet_manager->force_send_packet(true);
      }
    }
    else{
      fakelag->run();
      antiaim->run();
    }
  }
  engine_prediction->finish();
  
  {
    #if defined(DEV_MODE)
    antiaim->fake_duck(false);
    #endif
    antiaim->break_animations(false);
  }
 
  global->is_choking_cmd = packet_manager->choking();
  acp->post_create_move(cmd);

  if(!global->is_choking_cmd)
    global->last_sent_angle = cmd->view_angles;
  else
    global->last_choked_angle = cmd->view_angles;

  global->both_angles = cmd->view_angles;
  if(acp->should_fix_movement())
    math::rebase_movement(cmd, global->original_cmd);

  packet_manager->post_run();
}

EXPORT void __fastcall frame_stage_notify_hook(u32 stage){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr){
    global->client->frame_stage_notify(stage);
    return;
  }

  utils::collect_entity_attachments();
  utils::update_local_data();
  utils::verify_player_data();
  player_list_handler->think();
  player_list_handler->verify_selected_player();
  
  utils::sync_entity_list_data();
  utils::check_for_steam_friends();
  utils::update_player_information(stage);
  lag_compensation->frame_stage_notify_run(stage);
  cheat_detection->think(stage);
  resolver->process_shots(stage);

  // Calling anything after this crashes the game.
  global->client->frame_stage_notify(stage);
}

EXPORT void __fastcall engine_paint_hook(void* rcx, u32 mode){
  assert(rcx != nullptr);

  float time = math::time();
  static float add_try_add_listener_time = 0.f;
  if(add_try_add_listener_time <= time){
    global->game_event_manager->add_listener(XOR("player_hurt"));
    add_try_add_listener_time = time + 5.f;
  }

  acp->calculate_feature_support();
  cheat_detection->transmit_steamid();
  if(utils::is_in_game()){
    c_base_player* localplayer = utils::localplayer();

    static i32 last_frame_count = 0;
    if(globaldata->frame_count != last_frame_count){
      
      static s_view_setup view_setup;
      // is_creating_spoofed_view here, I removed it since it actually can make the view_matrix be outdated and appear laggy.
      { 
        if(global->client->get_player_view(view_setup))
          global->engine_tool->get_world_to_screen_matrix_for_view(&view_setup, &global->view_matrix);    
      }
 
      for(u32 i = 1; i <= global->entity_list->get_highest_index(); i++){
        c_base_entity* entity = global->entity_list->get_entity(i);
        if(entity == nullptr)
          continue;
  
        if(entity->is_player()){
          c_base_player* p = (c_base_player*)entity;
          c_player_data* data = p->data();
          if(data == nullptr)
            continue;
  
          data->transform     = entity->rgfl_coordinate_frame;
          data->has_transform = true;
          data->has_bones     = entity->has_bones();
        }
        else{
          c_entity_data* data = entity->data();
          if(data == nullptr)
            continue;
  
          data->transform     = entity->rgfl_coordinate_frame;
          data->has_transform = true;

          data->has_bones = entity->has_bones();
          if(data->has_bones){
            if(data->last_simulation_time <= entity->simulation_time){

              // Only call setup bones on non player entities if it makes sense. Like we wanna aimbot them.
              if(data->list_data.aimbot_entity){
                lag_compensation->is_setting_up_bones = true;
                data->has_bone_matrix = data->has_bones = entity->setup_bones((matrix3x4*)&data->bone_matrix, true);
                lag_compensation->is_setting_up_bones = false;
              }

              data->last_simulation_time = entity->simulation_time;
            }
          }
        }
      }
  
      last_frame_count = globaldata->frame_count;
    }
  }

  utils::call_fastcall64<void, u32>(gen_internal->decrypt_asset(global->engine_paint_hook_trp), rcx, mode);
}

EXPORT void __fastcall paint_traverse_hook(void* rcx, i64 panel, bool repaint, bool force_frame){
  assert(rcx != nullptr);

  i8* panel_name = global->panel->get_name(panel);
  assert(panel_name != nullptr);

  u32 panel_hash = HASH_RT(panel_name);
  if(global->focus_overlay_panel_id == 0 && panel_hash == HASH("FocusOverlayPanel"))
    global->focus_overlay_panel_id = panel;

  utils::call_fastcall64<void, i64, bool, bool>(gen_internal->decrypt_asset(global->paint_traverse_hook_trp), rcx, panel, repaint, force_frame);
}

EXPORT bool __fastcall cnetchan_sendnetmsg_hook(s_net_channel* rcx, c_net_message& net_msg, bool rel, bool voice){
  assert(rcx != nullptr);
  assert(net_msg.get_name() != nullptr);

  u32 name_hash = HASH_RT(net_msg.get_name());
  if(name_hash == HASH("clc_Move")){

    // CL_SendMove is inlined in Garry's Mod.
    // So rather than rebuild cl_move which would be very annoying todo we'll do this here.
    // Tested it and it works fine.
    c_clc_move* move = (c_clc_move*)&net_msg;
    {
      i32 last_outgoing_cmd = global->client_state->last_outgoing_command;
      i32 choked_cmd        = global->client_state->choked_commands;
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
  else if(name_hash == HASH("clc_VoiceData")){
    voice = true;

    if(config->misc.voicerecord_protect && !GetAsyncKeyState('X') && !GetAsyncKeyState('B') && !GetAsyncKeyState('N')){
      DBG("clc_VoiceData blocked\n");
      global->info_panel->add_entry(INFO_PANEL_VOICERECORD_PROTECT, WXOR(L"VOICERECORD PROTECT"), WXOR(L"ACTIVE!"), INFO_PANEL_WARN_RAGE_CLR);
      return false;
    }
  }
  else if(false && name_hash == HASH("clc_GMod_ClientToServer")){
    clc_gmod_clienttoserver* cts = (clc_gmod_clienttoserver*)&net_msg;

    if(cts->name != nullptr && (uptr)cts->name & 0xFFFFFFFF00000000){
      u32 msg_name_hash = FNV1A_RT(cts->name, 14);

      if(msg_name_hash == HASH("ph_playerisafk")){
        DBG("Skipped ph_playerisafk\n");
        return false;
      }
      else
        DBG("%s\n", cts->name);
    }
  }


  return utils::call_fastcall64<bool, c_net_message&, bool, bool>(gen_internal->decrypt_asset(global->cnetchan_sendnetmsg_hook_trp), rcx, net_msg, rel, voice);
}

EXPORT bool __fastcall fire_event_hook(void* rcx, c_game_event* event){
  assert(rcx != nullptr);
  assert(event != nullptr);

  utils::process_game_event(event);

  return utils::call_fastcall64<bool, void*>(gen_internal->decrypt_asset(global->fire_event_hook_trp), rcx, event);
}

EXPORT bool __fastcall lua_gamemode_callwithargs_hook(void* rcx, i32 id){
  if(utils::is_call_from_rijin(__builtin_return_address(0))){
    DBG("lua_gamemode_callwithargs_hook ret0\n");
    return false; // return true or not?
  }
  else if(global->is_calling_run_command)
    return false;

  return utils::call_fastcall64<bool, i32>(gen_internal->decrypt_asset(global->lua_gamemode_callwithargs_hook_trp), rcx, id);
}

EXPORT bool __fastcall lua_gamemode_call_hook(void* rcx, i32 id){
  if(utils::is_call_from_rijin(__builtin_return_address(0))){
    DBG("lua_gamemode_call_hook ret0\n");
    return false; // return true or not?
  }

  return utils::call_fastcall64<bool, i32>(gen_internal->decrypt_asset(global->lua_gamemode_call_hook_trp), rcx, id);
}

EXPORT bool __fastcall lua_gamemode_callfinish_hook(void* rcx, i32 id){
  if(utils::is_call_from_rijin(__builtin_return_address(0))){
    DBG("lua_gamemode_callfinish_hook ret0\n");
    return false; // return true or not?
  }

  return utils::call_fastcall64_raw<bool, void*, i32>(gen_internal->decrypt_asset(global->lua_gamemode_callfinish_hook_trp), rcx, id);
}

// Str versions here.
EXPORT bool __fastcall lua_gamemode_callwithargs_str_hook(void* rcx, i8* id){
  if(utils::is_call_from_rijin(__builtin_return_address(0))){
    DBG("lua_gamemode_callwithargs_str_hook ret0\n");
    return false; // return true or not?
  }
  else if(global->is_calling_run_command)
    return false;

  return utils::call_fastcall64<bool, i8*>(gen_internal->decrypt_asset(global->lua_gamemode_callwithargs_hook_trp), rcx, id);
}

EXPORT bool __fastcall lua_gamemode_call_str_hook(void* rcx, i8* id){
  if(utils::is_call_from_rijin(__builtin_return_address(0))){
    DBG("lua_gamemode_call_str_hook ret0\n");
    return false; // return true or not?
  }

  return utils::call_fastcall64<bool, i8*>(gen_internal->decrypt_asset(global->lua_gamemode_call_str_hook_trp), rcx, id);
}

EXPORT void __fastcall crenderview_renderview_hook(void* rcx, s_view_setup* view, i32 flags, i32 what_to_draw){
  assert(rcx != nullptr);

  void* ret_addr = __builtin_return_address(0);
  if(ret_addr != global->cviewrender_renderview_ret_addr1 || anti_screengrab->is_creating_spoofed_view()){
    DBG("[!] Filtering crenderview_renderview_hook call\n");
    utils::call_fastcall64<void, s_view_setup*, u32, u32>(gen_internal->decrypt_asset(global->crenderview_renderview_hook_trp), rcx, view, flags, what_to_draw);
    return;
  }

  // This has to always be on top.
  anti_screengrab->create_spoofed_view(rcx, view, flags, what_to_draw);
  
  thirdperson->calculate_view_status(view);
  
  thirdperson->run(view, &what_to_draw);
  freecam->run(view, &what_to_draw);

  utils::call_fastcall64<void, s_view_setup*, i32, i32>(gen_internal->decrypt_asset(global->crenderview_renderview_hook_trp), rcx, view, flags, what_to_draw);


  //anti_screengrab->debug_draw();
}

// NOTE: Game might have removed the parameters of this function. Doesn't matter since we don't use them.
EXPORT CFLAG_O0 bool __fastcall render_capture_hook(void* rcx){
  if(!acp->is_blocked(FUNC_SCREEN_GRAB)){
    anti_screengrab->on_screen_grab_start();
    anti_screengrab->push();
  }
  bool result = utils::call_fastcall64<bool>(gen_internal->decrypt_asset(global->render_capture_hook_trp), rcx);
  if(!acp->is_blocked(FUNC_SCREEN_GRAB)){
    anti_screengrab->pop();
    anti_screengrab->on_screen_grab_end();
  }

  return result;
}

EXPORT CFLAG_O0 bool __fastcall ivideowriter_addframe_hook(void* rcx){
  if(!acp->is_blocked(FUNC_SCREEN_GRAB))
    anti_screengrab->on_video_add_frame_start();

  bool result = utils::call_fastcall64<bool>(gen_internal->decrypt_asset(global->ivideowriter_addframe_hook_trp), rcx);

  if(!acp->is_blocked(FUNC_SCREEN_GRAB))
    anti_screengrab->on_video_add_frame_finish();
  
  return result;  
}

EXPORT CFLAG_O0 void __fastcall record_game_hook(void* rcx, void* unk1){
  if(!acp->is_blocked(FUNC_SCREEN_GRAB))
    anti_screengrab->on_screen_recording_start();

  utils::call_fastcall64<void, void*>(gen_internal->decrypt_asset(global->record_game_hook_trp), rcx, unk1);
}

EXPORT CFLAG_O0 void __fastcall shaderapi_read_pixels_hook(void* rcx, i32 x, i32 y, i32 w, i32 h, u8* data, void* image_format){
  c_render_context* rc = global->material_system->get_render_context();
  if(rc == nullptr){
    utils::call_fastcall64<void, i32, i32, i32, i32, u8*, void*>(gen_internal->decrypt_asset(global->shaderapi_read_pixels_hook_trp), rcx, x, y, w, h, data, image_format);
    return;
  }

  // This works, expect if the size does not match read_pixels
  c_texture* old_rt = rc->get_render_target();
  if(!acp->is_blocked(FUNC_SCREEN_GRAB)){

    // Lets say they decided to record our screen in 1280 x 720 but we're in 1920 x 1080. 
    // It'll looked all fucked, so this fixes it by creating a resized texture that it then can record to buffer.
    anti_screengrab->calcuating_viewport = true;
    {
      rc->push_render_target_and_viewport();
      rc->viewport(0, 0, x, y);
      rc->set_render_target(anti_screengrab->read_pixels_texture);
      rc->draw_screen_space_rectangle(anti_screengrab->view_material, 0, 0, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
      rc->pop_render_target_and_viewport();
    }
    anti_screengrab->calcuating_viewport = false;

    rc->set_render_target(anti_screengrab->read_pixels_texture);
  }

  utils::call_fastcall64<void, i32, i32, i32, i32, u8*, void*>(gen_internal->decrypt_asset(global->shaderapi_read_pixels_hook_trp), rcx, x, y, w, h, data, image_format);

  if(!acp->is_blocked(FUNC_SCREEN_GRAB))
    rc->set_render_target(old_rt);
  
}

EXPORT void __fastcall set_render_target_hook(c_render_context* rcx, c_texture* texture){
  if(utils::is_call_from_rijin(__builtin_return_address(0))){
    utils::call_fastcall64<void, c_texture*>(gen_internal->decrypt_asset(global->set_render_target_hook_trp), rcx, texture);
    return;
  }

  if(chams->is_rendering){
    DBG("[!] blocking set_render_target_hook due to chams\n");
    return;
  }

  // Everything below here handles the 'texture variable' properly in the event it's null. Don't add a check in here.
  if(!anti_screengrab->is_rijin_texture(texture))
    anti_screengrab->set_original_texture(texture);

  if(anti_screengrab->should_override_set_render_target(texture))
    texture = anti_screengrab->view_texture;
  
  utils::call_fastcall64<void, c_texture*>(gen_internal->decrypt_asset(global->set_render_target_hook_trp), rcx, texture);
}

EXPORT void __fastcall push_render_target_and_viewport_p5_hook(c_render_context* rcx, c_texture* texture, i64 x, i64 y, i64 w, i64 h){
  if(utils::is_call_from_rijin(__builtin_return_address(0))){
    utils::call_fastcall64<void, c_texture*, i64, i64, i64, i64>(gen_internal->decrypt_asset(global->push_render_target_and_viewport_p5_hook_trp), rcx, texture, x, y, w, h);
    return;
  }

  // Everything below here handles the 'texture variable' properly in the event it's null. Don't add a check in here.
  if(!anti_screengrab->is_rijin_texture(texture))
    anti_screengrab->set_original_texture(texture);

  if(anti_screengrab->should_override_set_render_target(texture))
    texture = anti_screengrab->view_texture;
  
  utils::call_fastcall64<void, c_texture*, i64, i64, i64, i64>(gen_internal->decrypt_asset(global->push_render_target_and_viewport_p5_hook_trp), rcx, texture, x, y, w, h);
}

EXPORT void __fastcall push_render_target_and_viewport_p1_hook(c_render_context* rcx, c_texture* texture){
  if(utils::is_call_from_rijin(__builtin_return_address(0))){
    utils::call_fastcall64<void, c_texture*>(gen_internal->decrypt_asset(global->push_render_target_and_viewport_p1_hook_trp), rcx, texture);
    return;
  }

  // Everything below here handles the 'texture variable' properly in the event it's null. Don't add a check in here.
  if(!anti_screengrab->is_rijin_texture(texture))
    anti_screengrab->set_original_texture(texture);

  if(anti_screengrab->should_override_set_render_target(texture))
    texture = anti_screengrab->view_texture;
  
  utils::call_fastcall64<void, c_texture*>(gen_internal->decrypt_asset(global->push_render_target_and_viewport_p1_hook_trp), rcx, texture);
}

// Hooked CopyRenderTargetTextureEx & CopyRenderTargetToTexture. We might need them at some point.
EXPORT void __fastcall copy_render_target_to_texture_hook(c_render_context* rcx, c_texture* texture){
  if(texture == nullptr || utils::is_call_from_rijin(__builtin_return_address(0))){
    utils::call_fastcall64<void, c_texture*>(gen_internal->decrypt_asset(global->copy_render_target_to_texture_hook_trp), rcx, texture);
    return;
  }

  utils::call_fastcall64<void, c_texture*>(gen_internal->decrypt_asset(global->copy_render_target_to_texture_hook_trp), rcx, texture);
}

EXPORT void __fastcall copy_render_target_to_texture_ex_hook(c_render_context* rcx, c_texture* texture, i64 unk1, i64 unk2, i64 unk3){
  if(texture == nullptr || utils::is_call_from_rijin(__builtin_return_address(0))){
    utils::call_fastcall64<void, c_texture*, i64, i64, i64>(gen_internal->decrypt_asset(global->copy_render_target_to_texture_ex_hook_trp), rcx, texture, unk1, unk2, unk3);
    return;
  }

  utils::call_fastcall64<void, c_texture*, i64, i64, i64>(gen_internal->decrypt_asset(global->copy_render_target_to_texture_ex_hook_trp), rcx,  texture, unk1, unk2, unk3);
}

EXPORT c_texture* __fastcall get_render_target_hook(void* rcx){
  if(utils::is_call_from_rijin(__builtin_return_address(0)))
    return utils::call_fastcall64<c_texture*>(gen_internal->decrypt_asset(global->get_render_target_hook_trp), rcx);
  
  c_texture* tex = utils::call_fastcall64<c_texture*>(gen_internal->decrypt_asset(global->get_render_target_hook_trp), rcx);
  if(anti_screengrab->is_rijin_texture(tex)) // Prevent the game from being able to see our textures.
    return anti_screengrab->get_original_texture();

  return tex;
}

EXPORT void __fastcall engine_client_cmd_hook(void* rcx, i8* str){
  if(str != nullptr){

    u32 command_hash = HASH_RT(str);
    if(__builtin_return_address(0) == global->engine_client_cmd_retaddr){
      DBG("[!] RunConsoleCommand: %s\n", str);
    }
    else{
      DBG("[!] Internal: %s\n", str);
    }
  }

  utils::call_fastcall64<void, i8*>(gen_internal->decrypt_asset(global->engine_client_cmd_hook_trp), rcx, str);
}

EXPORT void __fastcall override_view_hook(void* rcx, s_view_setup* view_setup){
  if(!anti_screengrab->is_creating_spoofed_view()){
    global->view_setup = view_setup;
  
    c_base_player* localplayer = utils::localplayer();
    if(localplayer != nullptr && config->visual.no_visual_recoil)
     view_setup->angles -= localplayer->punchangle();

    global->game_fov = view_setup->fov;
  }
  utils::call_fastcall64<void, s_view_setup*>(gen_internal->decrypt_asset(global->override_view_hook_trp), rcx, view_setup);
}

// Probably should do a filter for our fake view here or something?
EXPORT void __fastcall do_image_space_motion_blur_hook(const s_view_setup* view_blur, i32 x, i32 y, i32 w, i32 h){

}

EXPORT void __fastcall client_entity_list_on_add_entity_hook(void* rcx, void* handle_entity, void* handle){
  utils::call_fastcall64<void, void*, void*>(gen_internal->decrypt_asset(global->client_entity_list_on_add_entity_hook_trp), rcx, handle_entity, handle);
  if(handle_entity == nullptr)
    return;

  c_base_entity* entity = utils::virtual_call64<7, c_base_entity*>(handle_entity);
  if(entity != nullptr){
    i32 index = entity->get_index();
    if(index == -1)
      return;
    
    utils::reset_entity_data(entity->get_index());
  }
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

    utils::reset_entity_data(index);
    //DBG("[+] %s (%i) [%i] was removed\n", entity->get_client_class()->name, index, global->memory_manager.get_allocated_bytes());
  }
}

EXPORT void __fastcall get_local_view_angles_hook(void* rcx, vec3* ang){
  utils::call_fastcall64<void, vec3*>(gen_internal->decrypt_asset(global->get_local_view_angles_hook_trp), rcx, ang);

  if(config->misc.demo_protect && ang != nullptr){
    global->info_panel->add_entry(INFO_PANEL_DEMO_PROTECT, WXOR(L"DEMO PROTECT"), WXOR(L"ACTIVE"), INFO_PANEL_WARNING_CLR);

    *ang = global->untouched_cmd.view_angles;
  }
}

EXPORT void __fastcall cmutliplayeranimstate_update_hook(void* rcx, float yaw, float pitch){
  void* entity = *(void**)((uptr)rcx + (uptr)0x130);

  if(entity == global->localplayer){
    vec3 new_ang;
    if(antiaim->thirdperson(&new_ang)){
      pitch = new_ang.x;
      yaw   = new_ang.y;
    }
  }
  else{
    resolver->apply_angles(entity, &pitch, &yaw);
  }

  utils::call_fastcall64<void, float, float>(gen_internal->decrypt_asset(global->cmutliplayeranimstate_update_hook_trp), rcx, yaw, pitch);
}

EXPORT void __fastcall cmutliplayeranimstate_restartgesture_hook(void* rcx, i32 gesture_slot, i32 gesture_seq, bool auto_kill){
  //if(gesture_slot == 5){
  //  switch(gesture_seq){
  //    default: break;
  //    case 1610: // agree
  //    case 1611: // becon
  //    case 1612: // bow
  //    case 1613: // disagree
  //    case 53: // forward
  //    case 54: // group
  //    case 55: // halt
  //    case 1620: // cheer
  //    case 1641: // zombie
  //    case 1642: // dance
  //    case 1618: // laugh
  //    case 1617: // muscle
  //    case 1616: // pers
  //    case 1643: // robot
  //    case 1614: // salute
  //    case 1615: // wave
  //      return;
  //  }
  //}

  utils::call_fastcall64<void, i32, i32, bool>(gen_internal->decrypt_asset(global->cmutliplayeranimstate_restartgesture_hook_trp), rcx, gesture_slot, gesture_seq, auto_kill);
}