#pragma once

namespace misc{
  inline void nospread(c_base_player* localplayer, bool aimbot_running){
    if(!config->aimbot.no_spread)
      return;

    if(localplayer == nullptr)
      return;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return;

    if(!wep->has_spread())
      return;

    const float spread = wep->spread();
    if(spread <= 0.f)
      return;

    if(wep->is_shotgun()){
      // I think it's important to encrypt these to prevent these from being reverse'd easily.
      global->current_cmd->command_number = XOR32(0x807FF533);
      global->current_cmd->random_seed    = math::md5_pseudo_random(global->current_cmd->command_number) & XOR32(0x7FFFFFFF);
    }

    assert(global->shared_random_float_addr != nullptr);

    static std::string horiz_spread = XOR("CTerrorGun::FireBullet HorizSpread");
    static std::string vert_spread = XOR("CTerrorGun::FireBullet VertSpread");

    *global->prediction_random_seed = global->current_cmd->random_seed;

    global->current_cmd->view_angles.x -= utils::call_cdecl<float, i8*, float, float, i32>(global->shared_random_float_addr, horiz_spread.c_str(), -spread, spread, 0);
    global->current_cmd->view_angles.y -= utils::call_cdecl<float, i8*, float, float, i32>(global->shared_random_float_addr, vert_spread.c_str(), -spread, spread, 0);
  }

  inline void fix_nospread_pred(c_base_player* player, c_user_cmd* cmd){
    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr || localplayer != player)
      return;

    c_base_weapon* wep = localplayer->get_weapon();

    if(wep == nullptr)
      return;

    // NOTE: Not doing this check will cause random writes and crash the game!!!
    // Took days to figure this out; CUNT!!!
    if(!wep->has_spread())
      return;

    static float old_spread = 0.f;
    if(cmd->predicted)
      wep->spread() = old_spread;
    else
      old_spread = wep->spread();
  }

  inline void bunnyhop(){
    if(!config->misc.bhop)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(localplayer->move_type() == MOVETYPE_NOCLIP)
      return;

    if(!(localplayer->entity_flags() & FL_ONGROUND))
      global->current_cmd->buttons &= ~IN_JUMP;
  }

  inline void auto_pistol(){
    if(!config->misc.auto_pistol)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return;

    if(!wep->is_semi_auto())
      return;

    static bool cycle = false;
    if(cycle)
      global->current_cmd->buttons &= ~IN_ATTACK;

    cycle = !cycle;
  }

  inline void teleport(){
    //if(!config->misc.teleport)
    //  return;
//
    //if(config->misc.teleport_use_key && !input_system->held(config->misc.teleport_key))
    //  return;
//
    //global->current_cmd->move = vec3(INFINITY, INFINITY, 0.f);
  }

  inline void break_tickbase(){
    if(!config->misc.break_tickbase || !input_system->held(config->misc.break_tickbase_key))
      return;

    c_net_channel* net = global->engine->get_net_channel();
    if(net == nullptr)
      return;

    net->out_sequence_nr += math::clamp(config->misc.break_tickbase_amount, 10, 5000);
  }

  inline void instant_afk(){
    if(!config->misc.break_tickbase || !input_system->held(config->misc.break_tickbase_key))
      return;

    c_net_channel* net = global->engine->get_net_channel();

    if(net == nullptr)
      return;

    net->out_sequence_nr += 50000;
  }

  inline void pill_exploit(){
#if defined(STAGING_MODE) || defined(DEV_MODE)
    if(!config->misc.godmode_exploit || !input_system->held(config->misc.godmode_exploit_key))
      return;

    static u32 s_tick = 0;
    float test = math::fmod(math::time(), 2.f);

    if(test > 1.f){
      s_tick = 0;
      return;
    }

    c_net_channel* net = global->engine->get_net_channel();

    if(net == nullptr)
      return;

    net->out_sequence_nr += XOR32(50000);
    global->current_cmd->view_angles = vec3(0,0,0);
    global->current_cmd->move        = vec3(0,0,0);
    global->original_cmd.move        = vec3(0,0,0);

    if(s_tick++ < XOR32(40))
      *global->move_ret_addr -= 0x5/*prev ret addr*/;
    else
      s_tick = 0;
#endif
  }

  NEVERINLINE static void speedhack(){
    packet_manager->lock(false);

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_alive())
      return;

    static i32 s_tick = 0;
    if(!config->misc.speedhack){
      global->speedhack_active = false;
      s_tick = 0;
      return;
    }

    if(config->misc.speedhack_use_key && !input_system->held(config->misc.speedhack_key)){
      global->speedhack_active = false;
      s_tick = 0;
      return;
    }

    // If we have any batched commands sent off this batch and then speedhack next tick.
    if(s_tick == 0 && global->client_state->choked_commands() > 0){
      DBG("[!] Sending off command batch before speedhacking.\n");
      packet_manager->force_send_packet(false);
      return;
    }

    i32 wanted_speedhack_ticks = math::clamp(config->misc.speedhack_commands, 2, 21);

    if(s_tick == 0){
      STACK_CHECK_START;
      utils::call_cdecl<void, u32>(global->cl_runprediction, 0);
      STACK_CHECK_END;
      //tickbase_correction->add_record(localplayer->tick_base, global->current_cmd->command_number, wanted_speedhack_ticks);
    }

    if(s_tick++ < wanted_speedhack_ticks){
      *global->move_ret_addr -= 0x5/*prev ret addr*/;
      packet_manager->set_choke(true);
    }
    else{
      s_tick = 0;
      packet_manager->force_send_packet(false);
    }

    global->speedhack_active = true;
    packet_manager->lock(true);
  }
};