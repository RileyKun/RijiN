#include "link.h"

EXPORT void __fastcall create_move_hook(void* ecx, void* edx, s_user_cmd* cmd){
  assert(ecx != nullptr);
  assert(cmd != nullptr);

  c_base_player* localplayer = utils::localplayer();

  if(!global->valid() || localplayer == nullptr || cmd->command_number == 0)
    return;

  utils::store_cmd_data(cmd);
  c_base_weapon* wep = localplayer->get_weapon();
  {
    global->send_packet = __builtin_frame_address(3) - 0x1;
    packet_manager->pre_run();
  }

  misc::bunnyhop();
  misc::server_crasher();

  engine_prediction->run();
  {
    acp->createmove(false);
    bool aimbot_running = raytrace_aimbot->run();
    if(!aimbot_running){
      if(global->aimbot_target_reset_time <= math::time())
        global->aimbot_target_index = -1;
    }

    bool firing         = localplayer->will_fire_this_tick();
    if(aimbot_running || firing){
      if(config->aimbot.no_recoil) // This might be used when a grenade explodes near you... Maybe...
        cmd->view_angles -= localplayer->punchangle();

      misc::nospread(localplayer, aimbot_running);
      misc::auto_pistol();

      if(packet_manager->choking())
        packet_manager->force_send_packet(true);
      else{
        packet_manager->force_send_packet(false);
        packet_manager->force_send_packet(true);
      }
    }

    if(!firing)
      misc::speedhack();

    acp->createmove(true);
  }
  engine_prediction->finish();

  // Fix lag compensation issue.
  if(wep != nullptr){
    if(wep->is_shotgun() && cmd->buttons & IN_ATTACK2)
      cmd->buttons |= IN_ATTACK;
  }

  math::rebase_movement(cmd, global->original_cmd);
  packet_manager->post_run();
}

EXPORT void __cdecl frame_stage_notify_hook(u32 stage){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr){
    utils::call_cdecl<void, u32>(gen_internal->decrypt_asset(global->frame_stage_notify_hook_trp), stage);
    return;
  }

  utils::verify_player_data();
  utils::check_for_steam_friends();
  lag_compensation->frame_stage_notify_run(stage);

  utils::call_cdecl<void, u32>(gen_internal->decrypt_asset(global->frame_stage_notify_hook_trp), stage);
}

EXPORT void __fastcall override_view_hook(void* ecx, void* edx, s_view_setup* view_setup){
  assert(ecx != nullptr);
  assert(view_setup != nullptr);

  if(config->visual.no_visual_recoil){
    c_base_player* localplayer = utils::localplayer();
    if(localplayer != nullptr)
      view_setup->angles -= localplayer->punchangle();
  }

  global->view_setup = view_setup;
  utils::call_fastcall<void, s_view_setup*>(gen_internal->decrypt_asset(global->override_view_hook_trp), ecx, edx, view_setup);
}

EXPORT void __fastcall engine_paint_hook(void* ecx, void* edx, u32 mode){
  assert(ecx != nullptr);

  if(global->valid()){
    static i32 last_frame_count;

    // Make a new cache every new frame.
    if(globaldata->frame_count != last_frame_count)
      global->engine_tool->get_world_to_screen_matrix_for_view(global->view_setup, &global->view_matrix);

    last_frame_count = globaldata->frame_count;
  }

  utils::call_fastcall<void, u32>(gen_internal->decrypt_asset(global->engine_paint_hook_trp), ecx, edx, mode);
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

//TODO: Needs to be done.
EXPORT void __stdcall cl_sendmove_hook(){
  static void* buf      = malloc(XOR32(0xFA0));
  memset(buf, 0, XOR32(0xFA0));
  i32 last_outgoing_cmd = global->client_state->last_outgoing_command;
  i32 choked_cmd        = global->client_state->choked_commands;
  i32 next_cmd          = last_outgoing_cmd + choked_cmd + 1;

  c_clc_move move{};

  assert(global->clc_move_rtti != nullptr);
  move.rtti = (u32)(global->clc_move_rtti);

  move.data_out.start_writing(buf, XOR32(0xFA0), 0, -1);
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
    s_net_channel* net = global->engine->get_net_channel();
    if(net != nullptr){
      if(net->choked_packets > 0)
        net->choked_packets -= extra_cmd;

      utils::virtual_call<40, bool, c_clc_move*, bool, bool>(net, &move, false, false);
    }
  }
}

EXPORT bool __fastcall cnetchan_sendnetmsg_hook(void* ecx, void* edx, c_net_message& net_msg, bool rel, bool voice){
  assert(ecx != nullptr);
  assert(net_msg.get_name() != nullptr);

  u32 name_hash = HASH_RT(net_msg.get_name());
  if(name_hash == HASH("clc_VoiceData"))
    voice = true;
  else if(name_hash == HASH("clc_RespondCvarValue")){
    if(acp->on_cvar_query((s_respond_cvar_value*)&net_msg))
      return true;
  }

  return utils::call_fastcall<bool, c_net_message&, bool, bool>(gen_internal->decrypt_asset(global->cnetchan_sendnetmsg_hook_trp), ecx, edx, net_msg, rel, voice);
}

EXPORT s_user_cmd* __fastcall cinput_get_user_cmd_hook(void* ecx, void* edx, i32 seq){
  assert(ecx != nullptr);
  assert(global->input != nullptr);

  return &global->input->commands[seq % 90];
}

EXPORT i32 __fastcall steam_exception_handler_hook(void* ecx, void* edx, void*, _EXCEPTION_POINTERS* ex_info, void*){
  if(ex_info == nullptr)
    return 0;

  utils::on_exception_x86(ex_info, XOR("DA:B"));
  
  return 0;
}

// This function will fix poses from being affected by cl_interp.
EXPORT float __stdcall client_interp_amount_hook(){
  return 0.0f;
}

// This function will jumping issues caused by the function hook above.
EXPORT float __stdcall cl_client_interp_amount_hook(){
  return 0.0f;
}

EXPORT bool __fastcall fire_event_hook(void* ecx, void* edx, c_game_event* event){
  assert(ecx != nullptr);
  assert(event != nullptr);

  u32 name_hash = HASH_RT(event->get_name());

  if(name_hash == HASH("server_spawn") || name_hash == HASH("client_beginconnect")){
    acp->reset();
    global->reset();
    global->engine->client_cmd(XOR("mem_compact"));
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
    utils::reset_lag_comp_data(attacked);
  }
  else if(name_hash == HASH("player_spawn")){
    i32 index = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));
    utils::reset_lag_comp_data(index);
  }

  return utils::call_fastcall<bool, void*>(gen_internal->decrypt_asset(global->fire_event_hook_trp), ecx, edx, event);
}

EXPORT void __fastcall maintain_sequence_transition_hook(void* ecx, void* edx, void* bone_setup, void* cycle, void* pos, void* q){
  assert(ecx != nullptr);
}

EXPORT void __fastcall csdkplayer_firebullet_hook(void* ecx, void* edx, vec3 shoot_pos, vec3& angle, float spread, i32 id, i32 damage, i32 bullet_type, void* attacker, bool do_effects, float x, float y){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer != attacker){
    if(!global->prediction->first_time_predicted) // Fix poopy game bug l0l
      return;

    utils::call_fastcall<void, vec3, vec3&, float, i32, i32, i32, void*, bool, float, float>(gen_internal->decrypt_asset(global->csdkplayer_firebullet_hook_trp), ecx, edx, shoot_pos, angle, spread, id, damage, bullet_type, attacker, do_effects, x, y);
    return;
  }

  // The game did alot of epicness with the spread before the bullet would've been fired.
  // This will perfectly correct for it!
  if(global->simulate_bullet){
    global->spread_dump.spread = spread;
    global->spread_dump.x      = x;
    global->spread_dump.y      = y;
    global->spread_dump.dumped = true;
    global->simulate_bullet    = false;
    return;
  }

  utils::call_fastcall<void, vec3, vec3&, float, i32, i32, i32, void*, bool, float, float>(gen_internal->decrypt_asset(global->csdkplayer_firebullet_hook_trp), ecx, edx, shoot_pos, angle, spread, id, damage, bullet_type, attacker, do_effects, x, y);
}

EXPORT void __fastcall csdkplayershared_setrecoil_hook(void* ecx, void* edx, float recoil){
  if(config->visual.no_visual_recoil)
    recoil = 0.f;

  utils::call_fastcall<void, float>(gen_internal->decrypt_asset(global->csdkplayershared_setrecoil_hook_trp), ecx, edx, recoil);
}