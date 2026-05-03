#pragma once

namespace misc{

  inline void nospread(c_base_player* localplayer, bool aimbot_running, bool pre_prediction, bool triggerbot_running){
    if(global->aimbot_settings == nullptr || global->triggerbot_settings == nullptr)
      return;

    if(localplayer == nullptr)
      return;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return;

    if(wep->is_melee() || wep->is_grenade())
      return;

    if(triggerbot_running && !global->triggerbot_settings->nospread_use_aimbot_settings){
      if(global->triggerbot_settings->nospread_legit_mode || global->triggerbot_settings->nospread_off)
        return;
    }
    else{
      if(!global->aimbot_settings->no_spread)
        return;
    }

    if(pre_prediction){
      if(global->aimbot_settings->no_spread_rage){ // Rage
          static i32 command_number = global->current_cmd->command_number;
          while(true){
            i32 seed = math::md5_pseudo_random(command_number) & INT_MAX;
            if((seed & 255) == 234)
              break;

            command_number++;
          }

          command_manager->set_command(command_number);
          command_number++;
      }
      return;
    }

    // The game will increase the seed by 1 and then set it.
    math::set_random_seed((global->current_cmd->random_seed & 255) + 1);

    float spread = wep->get_spread();
    if(spread <= 0.f)
      return;

    global->info_panel->add_entry(INFO_PANEL_NOSPREAD, WXOR(L"NOSPREAD"), WXOR(L"RUNNING"));
    float cone = wep->get_inaccuracy();

    float source_pi = 6.2831855f;

    float a0 = math::random_float(0.0f, source_pi);
    float r0 = math::random_float(0.0f, spread);
    float a1 = math::random_float(0.0f, source_pi);
    float r1 = math::random_float(0.0f, cone);

    float x0 = r0 * math::cos(a0);
    float y0 = r0 * math::sin(a0);

    float x1 = r1 * math::cos(a1);
    float y1 = r1 * math::sin(a1);

    float xx = x0 + x1;
    float yy = y0 + y1;

    vec3 fwd, right, up;
    math::angle_2_vector(global->current_cmd->view_angles, &fwd, &right, &up);

    vec3 spread_recalculated = fwd + (right * xx * -1.0f) + (up * yy * -1.0f);
    spread_recalculated.normalize();
    math::vector_2_angle(&spread_recalculated, &global->current_cmd->view_angles);

    if(!pre_prediction){
      if(!aimbot_running)
        packet_manager->set_choke(true);
    }
  }

  inline void norecoil(c_base_player* localplayer){
    if(global->aimbot_settings == nullptr)
      return;

    if(!global->aimbot_settings->no_recoil)
      return;

    if(localplayer == nullptr)
      return;

    if(!localplayer->is_valid())
      return;

    if(!global->aimbot_settings->no_recoil_rage && !global->aimbot_settings->no_recoil_legit)
       global->aimbot_settings->no_recoil_rage = true;

    if(global->aimbot_settings->no_recoil_legit)
      global->info_panel->add_entry(INFO_PANEL_NORECOIL, WXOR(L"NO RECOIL"), WXOR(L"LEGIT"), rgb(32, 255, 64));
    else
      global->info_panel->add_entry(INFO_PANEL_NORECOIL, WXOR(L"NO RECOIL"), WXOR(L"RAGE"), rgb(255, 64, 64));

    recoil_control->correct_view_punch(global->current_cmd->view_angles, false);
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

    global->info_panel->add_entry(INFO_PANEL_AUTO_PISTOL, WXOR(L"AUTO PISTOL"), WXOR(L"RUNNING"));
    static i32 cycle = 0;
    if(!cycle)
      global->current_cmd->buttons &= ~IN_ATTACK;

    cycle++;
    if(cycle >= 2)
      cycle = 0;
  }
};