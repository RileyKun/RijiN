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

    global->spread_dump.dumped = false;
    global->simulate_bullet    = true;
    {
      i32 old_tick_base               = localplayer->tick_base;
      i32 old_seed                    = *global->prediction_random_seed;
      float old_next_primary_attack   = wep->next_primary_attack;

      *global->prediction_random_seed = global->current_cmd->random_seed;
      global->prediction->run_command(localplayer, global->current_cmd, global->move_helper);

      localplayer->tick_base          = old_tick_base;
      *global->prediction_random_seed = old_seed;
      wep->next_primary_attack        = old_next_primary_attack;
    }
    global->simulate_bullet = false;

    if(!global->spread_dump.dumped || global->spread_dump.spread <= 0.f)
      return;

    float right_mod = -(global->spread_dump.spread * global->spread_dump.x);
    float up_mod    = -(global->spread_dump.spread * global->spread_dump.y);

    vec3 fwd, right, up;
    math::angle_2_vector(global->current_cmd->view_angles, &fwd, &right, &up);

    vec3 spread_recalculated = fwd + right * right_mod + up * up_mod;
    math::vector_2_angle(&spread_recalculated, &global->current_cmd->view_angles);
    if(!aimbot_running)
      packet_manager->set_choke(true);
  }

  inline void auto_pistol(){
    if(!config->misc.auto_pistol)
      return;

    assert(global->current_cmd != nullptr);

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_valid())
      return;

    c_base_weapon* weapon = localplayer->get_weapon();
    if(weapon == nullptr)
      return;

    if(!weapon->is_semi_auto())
      return;

    static i32 cycle = 0;
    if(!cycle)
      global->current_cmd->buttons &= ~IN_ATTACK;

    cycle++;
    if(cycle >= 2)
      cycle = 0;
  }

  inline void bunnyhop(){
    if(!config->misc.bhop)
      return;

    if(acp->is_active())
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_valid())
      return;

    if(!(localplayer->entity_flags & FL_ONGROUND))
      global->current_cmd->buttons &= ~IN_JUMP;
  }

  inline void remove_drunk_effects(u32 stage){
    if(!config->visual.no_drunk_effects)
      return;

    if(stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_valid())
      return;

    localplayer->drunkness = 0.f;
  }

  inline void server_crasher(){
    #if defined(DEV_MODE)
    if(!config->misc.server_crasher)
      return;

    if(!input_system->held(config->misc.server_crasher_key))
      return;

    u32 engine_base = I(GetModuleHandleW)(WXOR(L"engine.dll"));
    if(engine_base == 0)
      return;

    void* msg = malloc(XOR32(sizeof(c_net_message_base) + sizeof(s_clc_client_info_msg)));
    if(msg == nullptr)
      return;

    u32 str_ptr = 0;
    {

#define clc_ClientInfo "clc_ClientInfo"
      // 1. search for clc_ClientInfo string
      for(u32 ptr = 0; ptr < XOR32(INT_MAX); ptr++){
        if(memcmp((void*)(str_ptr = engine_base + ptr), clc_ClientInfo, XOR32(sizeof(clc_ClientInfo))) == 0)
          break;
      }

      // 2. Search for the first value that is a nullptr and replace it with the last entry
      // This will cause a nullptr
      for(i32 i = XOR32(sizeof(i32));; i += XOR32(sizeof(i32))){
        u32 entry = *(u32*)(str_ptr - i);
        if(*(u32*)(entry) != 0)
          continue;

        *(u32*)(msg) = str_ptr - (i - XOR32(sizeof(u32)));
        break;
      }

      s_net_channel* net = global->engine->get_net_channel();
      if(net == nullptr)
        return;

      while(true){
        if(!utils::virtual_call<40, bool, void*, bool, bool>(net, msg, false, false))
          break;

        DBG("[!] Crashing...\n");
      }

      DBG("[!] Crashed!\n");
      free(msg);
    }
    #endif
  }
};