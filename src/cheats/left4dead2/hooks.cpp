#include "link.h"

EXPORT void __fastcall create_move_hook(void* ecx, void* edx, c_user_cmd* cmd){
  if(cmd == nullptr || !cmd->command_number)
    return;

  // dump and store bSendPacket
  {
    global->send_packet   = __builtin_frame_address(3) - 0x1D;
    global->move_ret_addr = __builtin_frame_address(3) + 0x4;
  }

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  global->pre_create_move(cmd);
  packet_manager->post_send_packet_init(global->send_packet);
  interp_manager->pre_create_move(cmd);

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return;

  misc::pill_exploit();
  misc::speedhack();
  misc::bunnyhop();
  misc::break_tickbase();

  auto_strafe->run();

  if(!engine_prediction->start_prediction(localplayer, cmd)){
    DBG("[-] create_move_hook: Unable to run engine prediction\n");
    return;
  }

  utils::store_bone_data();
  {
    bool aimbot_running = raytrace_aimbot->run();
    if(!aimbot_running){
      if(global->aimbot_target_reset_time <= math::time())
        global->aimbot_target_index = -1;
    }

    bool firing = localplayer->will_fire_this_tick();
    if(aimbot_running || firing){
      if(config->aimbot.no_recoil)
        cmd->view_angles -= localplayer->punch_angle();

      misc::nospread(localplayer, aimbot_running);
      misc::auto_pistol();

      // send this tick and next tick
      packet_manager->on_input();
    }
    else{
      auto_bash->run();
    }
  }

  // Clamp viewangles.
  cmd->view_angles  = math::clamp_angles(cmd->view_angles);

  math::rebase_movement(cmd, global->original_cmd);
  interp_manager->post_create_move(cmd);

  packet_manager->post_create_move_prediction();
  misc::teleport();

  global->post_create_move(cmd, packet_manager->choking());
  engine_prediction->end_prediction(cmd);
}

EXPORT void* __cdecl cmd_executecommand_hook(u32 target, c_command_list& cmd, u32 client_slot){
  if(cmd.pargv[0] != nullptr){
    u32 command_hash = HASH_RT(cmd.pargv[0]);
    if(command_hash == HASH("RIJIN_IDGEN100")){
      create_server_iden(rijin_user_id, "RIJIN_USER_ID");
      i8 buf[1024];
      wsprintfA(buf, XOR("echo %i"), rijin_user_id_gen);
      global->engine->client_cmd(buf);
    }
  }
  return utils::call_cdecl<void*, u32, c_command_list&, u32>(gen_internal->decrypt_asset(global->cmd_executecommand_hook_trp), target, cmd, client_slot);
}

EXPORT void __cdecl int_decode_hook(s_decode_info* info){
  utils::call_cdecl<void, s_decode_info*>(gen_internal->decrypt_asset(global->int_decode_hook_trp), info);

  if(info == nullptr)
    return;

  if(info->send_prop == nullptr)
    return;

  if(info->send_prop->var_name == nullptr)
    return;

  c_base_entity* entity = info->struct_base;
  if(entity == nullptr)
    return;

  //if(!(info->send_prop->flags & SPROP_VARINT) && !(info->send_prop->flags & SPROP_UNSIGNED))
  //  return;

  u32 name_hash = HASH_RT(info->send_prop->var_name);

  // STRIP FL_FROZEN from client
  if(name_hash == HASH("m_fFlags"))
    *(i32*)info->data &= ~FL_FROZEN;
}

EXPORT void __fastcall crenderview_renderview_hook(void* ecx, void* edx, s_view_setup* view, s_view_setup* hudview, i32 flags, i32 what_to_draw){
  assert(ecx != nullptr);

  if(config->visual.override_fov){
    config->visual.override_fov_val = math::clamp(config->visual.override_fov_val, 75.f, 140.f);
    view->fov = config->visual.override_fov_val;
  }

  thirdperson->render_view(view, &what_to_draw);
  //freecam->run(view, &what_to_draw);
  utils::call_fastcall<void, s_view_setup*, s_view_setup*, i32, i32>(gen_internal->decrypt_asset(global->crenderview_renderview_hook_trp), ecx, edx, view, hudview, flags, what_to_draw);
  thirdperson->calculate_view_state(view);
}

EXPORT bool __fastcall should_draw_localplayer_hook(void* ecx, void* edx){
  if(thirdperson->is_active())
    return true;
  
  return utils::call_fastcall<bool>(gen_internal->decrypt_asset(global->should_draw_localplayer_hook_trp), ecx, edx);
}

EXPORT bool __fastcall cam_isthirdperson_hook(void* ecx, void* edx, i32 index){
  if(global->cam_isthirdperson_retaddr_override_view == __builtin_return_address(0))
    return false;

  if(thirdperson->is_active())
    return true;
  
  return utils::call_fastcall<bool, i32>(gen_internal->decrypt_asset(global->cam_isthirdperson_hook_trp), ecx, edx, index);
}

EXPORT void __cdecl frame_stage_notify_hook(u32 stage){

  entity_info->think(stage);
  utils::check_for_steam_friends();
  utils::verify_all_entity_data();
  player_list_handler->think();
  player_list_handler->verify_selected_player();

  lag_compensation->frame_stage_notify(stage);
  cheat_detection->frame_stage_notify(stage);

  utils::call_cdecl<void, u32>(gen_internal->decrypt_asset(global->frame_stage_notify_hook_trp), stage);
}

EXPORT void __stdcall override_view_hook(s_view_setup* view_setup){
  assert(view_setup != nullptr);

  if(config->visual.no_visual_recoil){
    c_base_player* localplayer = utils::localplayer();
    if(localplayer != nullptr)
      view_setup->angles -= localplayer->punch_angle();
  }

  global->view_setup = view_setup;

  utils::call_stdcall<void, s_view_setup*>(gen_internal->decrypt_asset(global->override_view_hook_trp), view_setup);
}

EXPORT void __fastcall engine_paint_hook(void* ecx, void* edx, u32 mode){
  assert(ecx != nullptr);

  utils::transmit_steamid();
  interp_manager->update();

  if(global->valid()){
    static i32 last_frame_count;

    // Make a new cache every new frame.
    if(globaldata->frame_count != last_frame_count){
      global->engine_tool->get_world_to_screen_matrix_for_view(global->view_setup, &global->view_matrix);

      float time = math::time();
      // Store every entities coordinate frame we want to render
      for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
        c_base_entity* entity = global->entity_list->get_entity(i);

        if(entity == nullptr)
          continue;

        if(entity->is_dormant())
          continue;

        utils::detect_objective_entity(entity);
        c_entity_data* data = entity->data();
        if(data == nullptr)
          continue;

        data->has_transform = true;
        data->transform     = entity->rgfl_coordinate_frame();

        if(entity->type() & TYPE_INFECTED || entity->type() & TYPE_WITCH){
          const float delta = math::abs(entity->simulation_time() - data->last_simulation_time);
          if(delta > 0.f)
            data->last_update_time = time;

          data->idle       = data->last_update_time > 0.f && math::abs(time - data->last_update_time) > 0.3f;

          const i32 update_delta = math::biggest(math::time_to_ticks(entity->simulation_time() - entity->old_simulation_time()), 1);
          if(entity->simulation_time() > data->last_simulation_time){
            data->vel = (entity->origin() - data->last_origin) / math::ticks_to_time(update_delta);
            data->last_origin = entity->origin();
          }

          // If the infected is idle, zero out velocity.
          if(data->idle)
            data->vel = vec3();

          data->last_simulation_time = entity->simulation_time();
        }
        else{
          data->idle             = false;
          data->last_update_time = 0.f;
        }
      }
    }

    last_frame_count = globaldata->frame_count;
  }

  utils::call_fastcall<void, u32>(gen_internal->decrypt_asset(global->engine_paint_hook_trp), ecx, edx, mode);
}

EXPORT void __stdcall cl_sendmove_hook(){
  DBG("[!] cl_sendmove_hook\n");
  static void* buf      = malloc(0xFA0);
  memset(buf, 0, 0xFA0);
  i32 last_outgoing_cmd = global->client_state->last_outgoing_command();
  i32 choked_cmd        = global->client_state->choked_commands();
  i32 next_cmd          = last_outgoing_cmd + choked_cmd + 1;

  c_clc_move move{};

  assert(global->clc_move_rtti != nullptr);
  move.rtti = (void*)(global->clc_move_rtti);

  move.data_out.start_writing(buf, 0xFA0, 0, -1);
  move.new_commands = math::clamp(choked_cmd + 1, 0, 15);

  i32 extra_cmd     = (choked_cmd + 1) - move.new_commands;
  i32 cmd_backup    = math::biggest(2, extra_cmd);

  move.backup_commands = math::clamp(cmd_backup, 0, 7);

  i32 num_of_cmds = move.new_commands + move.backup_commands;
  i32 from        = -1;
  bool ok         = true;

  for(i32 to = next_cmd - num_of_cmds + 1; to <= next_cmd; to++){
    bool is_new_cmd = to >= (next_cmd - move.new_commands) + 1;

    ok = ok && global->client->write_user_cmd_delta_to_buffer(&move.data_out, from, to, is_new_cmd);
    from = to;
  }

  if(ok){
    c_net_channel* net = global->engine->get_net_channel();
    if(net != nullptr){
      if(net->choked_packets > 0)
        net->choked_packets -= extra_cmd;

      STACK_CHECK_START;
      utils::virtual_call<41, bool, c_clc_move*, bool, bool>(net, &move, false, false);
      STACK_CHECK_END;
    }
  }
}

EXPORT bool __fastcall base_animating_setupbones_hook(void* ecx, void* edx, matrix3x4* matrix, i32 max_bones, i32 bone_mask, float current_time){
  assert(ecx != nullptr);
  STACK_CHECK_START;
  bool ret = utils::call_fastcall<bool, matrix3x4*, i32, i32, float>(gen_internal->decrypt_asset(global->base_animating_setupbones_hook_trp), ecx, edx, matrix, max_bones, bone_mask, current_time);
  STACK_CHECK_END;

  // We're just gonna assume a call to this function is a base entity? Likely explain crashing if there's a non base entity being called here.
  // Rud: April 10th, 2025.
  // Edit: Seems to be fine.
  {
    c_base_player* player = (u32)ecx - 0x4;

    if(player->is_npc_or_human() && !player->is_dormant()){
      c_entity_data* data = player->data();

      // This should be the best threadsafe way to call this
      if(data != nullptr){
        i8* seq_name = player->get_sequence_name();
        if(seq_name != nullptr){
          u32 len = strlen(seq_name);
          assert(len < 1024);

          for(i32 i = 0; i < len; i++)
            data->sequence_name[i] = seq_name[i];

          data->sequence_name[len] = '\0';
        }
        else{
          data->sequence_name[0] = '\0';
        }

      }
    }
  }

  return ret;
}

EXPORT void __fastcall server_adjust_player_time_base_hook(void* ecx, void* edx, i32 simulation_ticks){
#if defined(DEV_MODE)
  static bool did_dt = false;

  if(simulation_ticks > 1 || did_dt)
    DBG("[!] APTB BEFORE: simulation_ticks: %i tick_base: %i\n", simulation_ticks, *(i32*)((u32)ecx + 0x2058));

  utils::call_fastcall<void, i32>(gen_internal->decrypt_asset(global->server_adjust_player_time_base_hook_trp), ecx, edx, simulation_ticks);

  if(simulation_ticks > 1 || did_dt)
    DBG("[!] APTB AFTER: simulation_ticks: %i tick_base: %i\n", simulation_ticks, *(i32*)((u32)ecx + 0x2058));

  if(did_dt)
    did_dt = false;

  if(simulation_ticks > 1)
    did_dt = true;
#else
  utils::call_fastcall<void, i32>(gen_internal->decrypt_asset(global->server_adjust_player_time_base_hook_trp), ecx, edx, simulation_ticks);
#endif
}

EXPORT bool __fastcall cnetchan_sendnetmsg_hook(void* ecx, void* edx, c_net_message& net_msg, bool rel, bool voice){
  assert(ecx != nullptr);
  assert(net_msg.get_name() != nullptr);

  u32 name_hash = HASH_RT(net_msg.get_name());
  if(name_hash == HASH("clc_VoiceData"))
    voice = true;

  return utils::call_fastcall<bool, c_net_message&, bool, bool>(gen_internal->decrypt_asset(global->cnetchan_sendnetmsg_hook_trp), ecx, edx, net_msg, rel, voice);
}

EXPORT c_user_cmd* __fastcall cinput_get_user_cmd_hook(void* ecx, void* edx, u32 slot, i32 seq){
  assert(ecx != nullptr);
  assert(global->input != nullptr);

  // Only run on screen slot 0 (massive issues otherwise)
  if(slot != 0)
    return utils::call_fastcall<c_user_cmd*, u32, i32>(gen_internal->decrypt_asset(global->cinput_get_user_cmd_hook_trp), ecx, edx, slot, seq);

  return &global->input->commands[seq % 150];
}

EXPORT i32 __fastcall steam_exception_handler_hook(void* ecx, void* edx, void*, _EXCEPTION_POINTERS* ex_info, void*){
  if(ex_info == nullptr)
    return 0;

  utils::on_exception_x86(ex_info, XOR("L4D2"));
  return 0;
}

// This function will jumping issues caused by the function hook above.
EXPORT float __stdcall cl_client_interp_amount_hook(){
  return 0.0f;
}

EXPORT bool __fastcall fire_event_hook(void* ecx, void* edx, c_game_event* event, bool server, bool client){
  assert(ecx != nullptr);
  assert(event != nullptr);

  u32 name_hash = HASH_RT(event->get_name());

  cheat_detection->on_fire_event(event);

  if(name_hash == HASH("server_spawn") || name_hash == HASH("client_beginconnect")){
    cheat_detection->reset();
    thirdperson->reset();
    global->reset();
  }
  else if(name_hash == HASH("player_disconnect")){
    i32 id = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));
    DBG("[!] %i data reset\n", id);

    utils::reset_entity_data(id);
    utils::reset_lag_comp_data(id);
  }
  else if(name_hash == HASH("player_connect_client")){
    i32 id = global->engine->get_player_for_user_id(event->get_int(XOR("index")));
    DBG("[!] %i data reset\n", id);

    utils::reset_entity_data(id);
    utils::reset_lag_comp_data(id);
  }
  else if(name_hash == HASH("player_death")){
    i32 attacked = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));
    utils::reset_lag_comp_data(attacked);
  }
  else if(name_hash == HASH("player_spawn")){
    i32 index = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));
    utils::reset_entity_data(index);
    utils::reset_lag_comp_data(index);
  }

  return utils::call_fastcall<bool, c_game_event*, bool, bool>(gen_internal->decrypt_asset(global->fire_event_hook_trp), ecx, edx, event, server, client);
}

EXPORT void __fastcall paint_traverse_hook(void* ecx, void* edx, u32 panel, bool repaint, bool force_frame){
  assert(ecx != nullptr);

  i8* panel_name = global->panel->get_name(panel);
  assert(panel_name != nullptr);

  u32 panel_hash = HASH_RT(panel_name);

  if(global->focus_overlay_panel_id == 0 && panel_hash == HASH("FocusOverlayPanel"))
    global->focus_overlay_panel_id = panel;

  utils::call_fastcall<void, u32, bool, bool>(gen_internal->decrypt_asset(global->paint_traverse_hook_trp), ecx, edx, panel, repaint, force_frame);
}

EXPORT void __fastcall cbaseviewmodel_calcviewmodelview_hook(c_base_entity* ecx, void* edx, c_base_entity* owner, vec3& eye_pos, vec3& angles){
  const vec3 e_pos = eye_pos;
  const vec3 e_ang = angles;

  if(config->visual.no_visual_recoil){
    c_base_player* localplayer = utils::localplayer();

    if(localplayer != nullptr){
      angles -= localplayer->punch_angle();
      angles.z = 0.f;
    }
  }

  ecx->last_bone_setup_time()  = -(3.402823e+38f);
  ecx->model_bone_counter()    = -1;

  viewmodel->calc_view_model(ecx, eye_pos, angles);
  utils::call_fastcall<void, void*, vec3&, vec3&>(gen_internal->decrypt_asset(global->cbaseviewmodel_calcviewmodelview_hook_trp), ecx, edx, owner, eye_pos, angles);
  viewmodel->post_calc_view_model(ecx, owner, eye_pos, angles);

  ecx->set_abs_origin(eye_pos);

  eye_pos = e_pos;
  angles  = e_ang;
}

EXPORT void __cdecl sv_cheats_callback_hook(c_cvar* convar, i8* old_str, float old_value){
  if(old_value != 0.0f && convar->val == 0){
    utils::virtual_call<24, void, uptr>(global->cvar, FCVAR_CHEAT); // This is the RevertCvar function with the flag as the parameter.
    DBG("[!] Reverting FCVAR_CHEAT flagged cvars\n");
  }
}
