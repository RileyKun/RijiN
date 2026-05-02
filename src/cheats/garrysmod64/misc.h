#pragma once

namespace misc{

  inline void auto_pistol(){
    if(!config->misc.auto_pistol)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_holding_semi_auto())
      return;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return;

    if(wep->is_harmless_weapon() || wep->is_melee())
      return;

    static u32 cycle = 0;

    if(cycle++){
      global->current_cmd->buttons &= ~IN_ATTACK;
      if(cycle >= 2)
        cycle = 0;
    }
  }

  inline void process_attack(){
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_valid())
      return;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return;

    if(wep->is_harmless_weapon() || wep->is_melee())
      return;

    if(wep->is_physgun())
      return;

    if(wep->is_rpg_guiding_missile())
      global->current_cmd->buttons &= ~IN_ATTACK;
    else{
      //if(wep->clip1 == 0)
      //  global->current_cmd->buttons &= ~IN_ATTACK;

      //if(wep->clip2 == 0)
      //  global->current_cmd->buttons &= ~IN_ATTACK2;
    }

  }

  inline void bunnyhop(){
    if(!config->misc.bhop)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_valid())
      return;

    if(localplayer->move_type == MOVETYPE_NOCLIP)
      return;

    if(!(localplayer->entity_flags & FL_ONGROUND)){
      global->current_cmd->buttons &= ~IN_JUMP;

      if(global->current_cmd->buttons & IN_SPEED)
        global->current_cmd->buttons &= ~IN_SPEED;
    }

    global->info_panel->add_entry(INFO_PANEL_BUNNYHOP, WXOR(L"BUNNYHOP"), WXOR(L"ACTIVE"), INFO_PANEL_WARNING_CLR);
  }
};