#include "../link.h"

CLASS_ALLOC(c_player_esp, player_esp);

void c_player_esp::precache(){
  static bool init = false;
  if(init)
    return;

  init = true;

  DBG("[!] c_player_esp::precache\n");
  memset(_str, 0, sizeof(_str));

  #define ASSIGN_BUFFER(i, x) format_w(_str[i], PLAYER_ESP_BUF_SIZE, WXOR(x));

  // Since this function is only called once, might as well XOR everything.
  for(u32 i = 0; i <= XOR32(PLAYER_ESP_MAX_DIST_CACHE); i++)
    format_w(_dist[i], 10, WXOR(L"[ %im ]"), i); // Create a string cache of all possible distances.

  ASSIGN_BUFFER(IGNORED_TEXT, L"IGNORED");
  ASSIGN_BUFFER(IGNORE_VACCINATOR_TEXT, L"IGNORE VACCINATOR");
  ASSIGN_BUFFER(FORCE_BAIM_KEY_TEXT, L"FORCE BAIM KEY");
  ASSIGN_BUFFER(FORCE_BAIM_TEXT, L"FORCE BAIM");
  ASSIGN_BUFFER(SLOW_DOWN_TEXT, L"SLOW DOWN");
  ASSIGN_BUFFER(ZOOMED_TEXT, L"ZOOMED");
  ASSIGN_BUFFER(CLOAKED_TEXT, L"CLOAKED");
  ASSIGN_BUFFER(DEADRINGER_DEPLOYED_TEXT, L"DEADRINGER DEPLOYED");
  ASSIGN_BUFFER(DISGUISE_TEXT, L"DISGUISE");
  ASSIGN_BUFFER(UBERCHARGE_TEXT, L"UBERCHARGE");
  ASSIGN_BUFFER(UBERCHARGE_HIDDEN_TEXT, L"UBERCHARGE HIDDEN");
  ASSIGN_BUFFER(TAUNT_TEXT, L"TAUNT");
  ASSIGN_BUFFER(CRIT_BOOST_TEXT, L"CRIT BOOSTED");
  ASSIGN_BUFFER(DAMAGE_BUFF_TEXT, L"DAMAGE BUFF");
  ASSIGN_BUFFER(DEMO_CHARGE_TEXT, L"DEMO CHARGE");
  ASSIGN_BUFFER(CRIT_COLA_TEXT, L"CRIT COLA");
  ASSIGN_BUFFER(BURN_TEXT, L"BURN");
  ASSIGN_BUFFER(JARATE_TEXT, L"JARATE");
  ASSIGN_BUFFER(BLEED_TEXT, L"BLEED");
  ASSIGN_BUFFER(DEFENSE_BUFF_HIGH_TEXT, L"DEFENSE BUFF HIGH");
  ASSIGN_BUFFER(DEFENSE_BUFF_TEXT, L"DEFENSE BUFF");
  ASSIGN_BUFFER(SPEED_HEAL_BUFF_TEXT, L"SPEED/HEAL BUFF");
  ASSIGN_BUFFER(MILK_TEXT, L"MILK");
  ASSIGN_BUFFER(QUICK_FIX_UBER_TEXT, L"QUICK FIX UBER");
  ASSIGN_BUFFER(FOCUS_BUFF_TEXT, L"FOCUS BUFF");
  ASSIGN_BUFFER(BULLET_RESIST_TEXT, L"BULLET RESIST");
  ASSIGN_BUFFER(BLAST_RESIST_TEXT, L"BLAST RESIST");
  ASSIGN_BUFFER(FIRE_RESIST_TEXT, L"FIRE");
  ASSIGN_BUFFER(MEDIGUN_DEBUFF_TEXT, L"MEDIGUN DEBUFF");
  ASSIGN_BUFFER(HEAL_DEBUFF_TEXT, L"HEAL DEBUFF");
  ASSIGN_BUFFER(PARTY_TEXT, L"PARTY");
  ASSIGN_BUFFER(FRIEND_TEXT, L"FRIEND");
  ASSIGN_BUFFER(BOT_TEXT, L"BOT");
  ASSIGN_BUFFER(CHEATER_TEXT, L"CHEATER");
  ASSIGN_BUFFER(SUSPICIOUS_TEXT, L"SUSPICIOUS");
  ASSIGN_BUFFER(HIGH_KD_TEXT, L"HIGH K/D");
  ASSIGN_BUFFER(HIGH_HEADSHOT_RATIO_TEXT, L"HIGH HS RATIO");
  ASSIGN_BUFFER(ABNORMAL_LATENCY_TEXT, L"ABNORMAL LATENCY");
  ASSIGN_BUFFER(DORMANT_TEXT, L"DORMANT");
  ASSIGN_BUFFER(DUEL_TEXT, L"DUEL");
  ASSIGN_BUFFER(HAS_OBJECTIVE_TEXT, L"HAS OBJECTIVE");

  ASSIGN_BUFFER(RESOLVER_MP_TEXT, L"MP");
  ASSIGN_BUFFER(RESOLVER_P_TEXT, L"P");
  ASSIGN_BUFFER(RESOLVER_A_MY_TEXT, L"&MY");
  ASSIGN_BUFFER(RESOLVER_MY_TEXT, L"MY");
  ASSIGN_BUFFER(RESOLVER_AY_TEXT, L"&Y");
  ASSIGN_BUFFER(RESOLVER_Y_TEXT, L"Y");
  ASSIGN_BUFFER(RESOLVER_TEXT, L" RESOLVER");

  ASSIGN_BUFFER(RESOLVER_MANUAL_ARROW_TEXT, L"▼");
  ASSIGN_BUFFER(AIM_TARGET_CIRCLE_TEXT, L"🔴");
  ASSIGN_BUFFER(AIM_TARGET_LINE_ARROW_TEXT, L"🡻");

  ASSIGN_BUFFER(RIJIN_STAFF_TEXT, L"EVIL");
  #if defined(DEV_MODE)
    ASSIGN_BUFFER(RIJIN_USER_TEXT, L"RIJIN USER");
  #endif
}

bool c_player_esp::should_draw(c_base_player* p){
  if(p == nullptr)
    return false;

  c_player_esp_settings* settings = utils::get_player_esp_settings(p);
  if(settings == nullptr)
    return false;

  if(!settings->render_cloaked_spies && p->is_cloaked())
    return false;

  bool should_render_local = true;
  if(!config->visual.player_esp_include_localplayer)
    should_render_local = false;


  if(!utils::is_in_thirdperson())
    should_render_local = false;

  if(!should_render_local && p->get_index() == global->localplayer_index)
    return false;

  if(config->visual.render_medic_mode){
    bool hurt = false;
    if(utils::medic_mode((c_base_entity*)p, hurt))
      return hurt;
  }

  return settings->enabled;
}

colour c_player_esp::get_draw_colour(c_base_player* p, i32 colour_type){
  colour draw_col                 = rgb(255,255,255);
  if(p == nullptr)
    return draw_col;
  
  if(!p->is_player()){
    DBG("[-] c_player_esp::get_draw_colour USING NON PLAYER! %X on %i\n", p, colour_type);
    return draw_col;
  }

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return draw_col;

  c_player_data* data = p->data();
  if(data == nullptr)
    return draw_col;

  //if(data->is_dev_esp)
  //  return rgba(165, 0, 255, 255);

  switch(colour_type){
    case PLAYER_ESP_RENDER_TYPE_TEXT:
    {
      if(config->visual.player_esp_use_team_colours){
        if(p->team() == TEAM_BLU)
          draw_col = flt_array2clr(config->visual.player_esp_blu_team_colour);
        else if(p->team() == TEAM_RED)
          draw_col = flt_array2clr(config->visual.player_esp_red_team_colour);
      }
      else if(config->visual.player_esp_use_enemyteam_colours){
        if(p->team() != global->localplayer_team)
          draw_col = flt_array2clr(config->visual.player_esp_enemy_colour);
        else if(p->team() == global->localplayer_team)
          draw_col = flt_array2clr(config->visual.player_esp_team_colour);
      }

      if(p->is_steam_friend() || p->is_localplayer()){
        if(config->visual.player_esp_friends_use_single_colour) // Single colour
          draw_col = flt_array2clr(config->visual.player_esp_friend_colour);
        else if(config->visual.player_esp_friends_use_mult_colour){ // Multi-colour!! (ANTI-RACIST)!!!
          if(config->visual.player_esp_use_team_colours){
            if(p->team() == TEAM_BLU)
              draw_col = flt_array2clr(config->visual.player_esp_friend_blu_team_colour);
            else if(p->team() == TEAM_RED)
              draw_col = flt_array2clr(config->visual.player_esp_friend_red_team_colour);
          }
          else if(config->visual.player_esp_use_enemyteam_colours){
            if(p->team() != global->localplayer_team)
              draw_col = flt_array2clr(config->visual.player_esp_friend_enemy_colour);
            else if(p->team() == global->localplayer_team)
              draw_col = flt_array2clr(config->visual.player_esp_friend_team_colour);
          }
        }
      }

      if(data->playerlist.custom_esp_colour)
        draw_col = flt_array2clr(data->playerlist.esp_colour);
         
      if(config->visual.player_esp_aimbot_target){
        if(p->get_index() == global->aimbot_target_index)
          draw_col = flt_array2clr(config->visual.player_esp_aimbot_target_colour);
      }

      draw_col.w = 255;
      break;
    }
    case PLAYER_ESP_RENDER_TYPE_CHAMS:
    {
      if(config->visual.player_chams_use_team_colours){
        if(p->team() == TEAM_BLU)
          draw_col = flt_array2clr(config->visual.player_chams_blu_team_colour);
        else if(p->team() == TEAM_RED)
          draw_col = flt_array2clr(config->visual.player_chams_red_team_colour);
      }
      else if(config->visual.player_chams_use_enemyteam_colours){
        if(p->team() != global->localplayer_team)
          draw_col = flt_array2clr(config->visual.player_chams_enemy_colour);
        else if(p->team() == global->localplayer_team)
          draw_col = flt_array2clr(config->visual.player_chams_team_colour);
      }

      if(p->is_steam_friend() || p->is_localplayer()){
        if(config->visual.player_chams_friends_use_single_colour) // Single colour
          draw_col = flt_array2clr(config->visual.player_chams_friend_colour);
        else if(config->visual.player_chams_friends_use_mult_colour){ // Multi-colour!! (ANTI-RACIST)!!!
          if(config->visual.player_chams_use_team_colours){
            if(p->team() == TEAM_BLU)
              draw_col = flt_array2clr(config->visual.player_chams_friend_blu_team_colour);
            else if(p->team() == TEAM_RED)
              draw_col = flt_array2clr(config->visual.player_chams_friend_red_team_colour);
          }
          else if(config->visual.player_chams_use_enemyteam_colours){
            if(p->team() != global->localplayer_team)
              draw_col = flt_array2clr(config->visual.player_chams_friend_enemy_colour);
            else if(p->team() == global->localplayer_team)
              draw_col = flt_array2clr(config->visual.player_chams_friend_team_colour);
          }
        }
      }

      if(data->playerlist.custom_chams_base_colour)
        draw_col = flt_array2clr(data->playerlist.chams_colour);
    
      if(config->visual.player_chams_aimbot_target){
        if(p->get_index() == global->aimbot_target_index)
          draw_col = flt_array2clr(config->visual.player_chams_aimbot_target_colour);
      }

      break;
    }
    case PLAYER_ESP_RENDER_TYPE_CHAMS_OVERLAY:
    {
      if(config->visual.player_chams_use_team_colours){
        if(p->team() == TEAM_BLU)
          draw_col = flt_array2clr(config->visual.player_chams_overlay_blu_team_colour);
        else if(p->team() == TEAM_RED)
          draw_col = flt_array2clr(config->visual.player_chams_overlay_red_team_colour);
      }
      else if(config->visual.player_chams_use_enemyteam_colours){
        if(p->team() != global->localplayer_team)
          draw_col = flt_array2clr(config->visual.player_chams_overlay_enemy_colour);
        else if(p->team() == global->localplayer_team)
          draw_col = flt_array2clr(config->visual.player_chams_overlay_team_colour);
      }

      if(p->is_steam_friend() || p->is_localplayer()){
        if(config->visual.player_chams_friends_use_single_colour) // Single colour
          draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_colour);
        else if(config->visual.player_chams_friends_use_mult_colour){ // Multi-colour!! (ANTI-RACIST)!!!
          if(config->visual.player_chams_use_team_colours){
            if(p->team() == TEAM_BLU)
              draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_blu_team_colour);
            else if(p->team() == TEAM_RED)
              draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_red_team_colour);
          }
          else if(config->visual.player_chams_use_enemyteam_colours){
            if(p->team() != global->localplayer_team)
              draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_enemy_colour);
            else if(p->team() == global->localplayer_team)
              draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_team_colour);
          }
        }
      }

      if(data->playerlist.custom_chams_overlay_colour)
        draw_col = flt_array2clr(data->playerlist.chams_overlay_colour);

      if(config->visual.player_chams_overlay_aimbot_target){
        if(p->get_index() == global->aimbot_target_index)
          draw_col = flt_array2clr(config->visual.player_chams_overlay_aimbot_target_colour);
      }
      break;
    }
    case PLAYER_ESP_RENDER_TYPE_GLOW:
    {
      if(config->visual.player_glow_use_team_colours){
        if(p->team() == TEAM_BLU)
          draw_col = flt_array2clr(config->visual.player_glow_blu_team_colour);
        else if(p->team() == TEAM_RED)
          draw_col = flt_array2clr(config->visual.player_glow_red_team_colour);
      }
      else if(config->visual.player_glow_use_enemyteam_colours){
        if(p->team() != global->localplayer_team)
          draw_col = flt_array2clr(config->visual.player_glow_enemy_colour);
        else if(p->team() == global->localplayer_team)
          draw_col = flt_array2clr(config->visual.player_glow_team_colour);
      }

      if(p->is_steam_friend() || p->is_localplayer()){
        if(config->visual.player_glow_friends_use_single_colour) // Single colour
          draw_col = flt_array2clr(config->visual.player_glow_friend_colour);
        else if(config->visual.player_glow_friends_use_mult_colour){ // Multi-colour!! (ANTI-RACIST)!!!
          if(config->visual.player_glow_use_team_colours){
            if(p->team() == TEAM_BLU)
              draw_col = flt_array2clr(config->visual.player_glow_friend_blu_team_colour);
            else if(p->team() == TEAM_RED)
              draw_col = flt_array2clr(config->visual.player_glow_friend_red_team_colour);
          }
          else if(config->visual.player_glow_use_enemyteam_colours){
            if(p->team() != global->localplayer_team)
              draw_col = flt_array2clr(config->visual.player_glow_friend_enemy_colour);
            else if(p->team() == global->localplayer_team)
              draw_col = flt_array2clr(config->visual.player_glow_friend_team_colour);
          }
        }
      }

      if(data->playerlist.custom_glow_colour)
        draw_col = flt_array2clr(data->playerlist.glow_colour);

      if(config->visual.health_glow_enabled && config->visual.health_glow_players){
        colour hp_col;
        if(utils::get_health_colour(p->health(), p->max_health(), &hp_col)){
          bool is_friend = p->is_steam_friend();
          if(global->localplayer_team == p->team() && config->visual.health_glow_show_friendly && !p->is_localplayer() && !is_friend || global->localplayer_team != p->team() && config->visual.health_glow_show_enemy && !is_friend || is_friend && !p->is_localplayer() && config->visual.health_glow_show_friends || p->is_localplayer() && config->visual.health_glow_show_localplayer && !is_friend)
            draw_col = hp_col;
        }
      }

      if(config->visual.player_glow_aimbot_target){
        if(p->get_index() == global->aimbot_target_index)
          draw_col = flt_array2clr(config->visual.player_glow_aimbot_target_colour);
      }

      draw_col.w = 255;
      break;
    }
  }
  return draw_col;
}

colour c_player_esp::get_draw_colour(c_base_entity* entity, i32 colour_type = PLAYER_ESP_RENDER_TYPE_TEXT){
  if(entity == nullptr)
    return rgb(255, 255, 255);

  if(!entity->is_player()){
    DBG("[-] Using player esp draw colour for world entities!\n");
    return rgb(255, 255, 255);
  }

  return get_draw_colour((c_base_player*)entity, colour_type);
}

colour c_player_esp::get_bland_colour(c_base_player* p){
  if(p == nullptr)
    return rgb(255, 255, 255);

  if(p->is_dormant()){
    c_player_esp_settings* settings = utils::get_player_esp_settings(p);
    if(settings == nullptr)
      return rgb(255, 255, 255);

    if(settings->dormant_color_gray_out)
      return rgb(188, 188, 188);
    else if(settings->dormant_color_team)
      return get_draw_colour(p);

  }

  return rgb(255, 255, 255);
}

template<auto c>
void c_player_esp::add_flags(c_base_player* p, c_esp_data<c>* esp_data){
  if(p == nullptr || esp_data == nullptr)
    return;

  c_base_weapon* wep = p->get_weapon();

  colour team_col = get_draw_colour(p);

  c_player_esp_settings* settings = utils::get_player_esp_settings(p);
  if(settings == nullptr)
    return;

  c_player_data* data = p->data();
  if(data == nullptr)
    return;

  if(settings->health_text){
    format_w(get_str(HP_FORMAT), PLAYER_ESP_BUF_SIZE, WXOR(L"%i HP"), p->health());
    colour clr;
    //utils::get_health_colour(i32 health, i32 max_health, colour* hp_col)
    if(utils::get_health_colour(p->health(), p->max_health(), &clr)){
      if(p->health()> p->max_health())
        clr = rgb(15, 230, 166);

      esp_data->add(ESP_TEXT_TYPE_RIGHT, clr, global->esp_font_small, 10, get_str(HP_FORMAT));
    }
  }

  if(settings->flag_aimbot_target){
    if(data->tfdata.rta_target_flags & RT_AIMBOT_PLAYERLIST_IGNORE_AIMBOT)
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(147, 150, 150), global->esp_font_small, 8, get_str(IGNORED_TEXT));
  
    if(data->tfdata.rta_target_flags & RT_AIMBOT_IGNORE_MEDIC_VACCINATOR)
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 85, 85), global->esp_font_small, 8, get_str(IGNORE_VACCINATOR_TEXT));
  
    if(data->tfdata.rta_target_flags & RT_AIMBOT_FORCE_BODY_AIM && !(data->tfdata.rta_target_flags & RT_AIMBOT_FORCE_BODY_AIM_SILENT) || data->tfdata.rta_target_flags & RT_AIMBOT_FORCE_BODY_AIM_MANUAL)
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 255, 132), global->esp_font_small, 8, data->tfdata.rta_target_flags & RT_AIMBOT_FORCE_BODY_AIM_MANUAL ? get_str(FORCE_BAIM_KEY_TEXT) : get_str(FORCE_BAIM_TEXT));
  }

  if(settings->flag_slow_down && p->has_condition(TF_COND_AIMING) && !p->is_scoped())
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(147, 150, 150), global->esp_font_small, 8, get_str(SLOW_DOWN_TEXT));

  if(settings->flag_sniper_charge && p->has_condition(TF_COND_AIMING) && data->sniper_charge_damage > 0){
    format_w(get_str(CHARGE_FORMAT), PLAYER_ESP_BUF_SIZE, WXOR(L"CHARGE: %i%%"), (i32)((data->sniper_charge_damage / 150.f) * 100.f));
    esp_data->add(ESP_TEXT_TYPE_RIGHT, team_col, global->esp_font_small, 8, get_str(CHARGE_FORMAT));
  }

  if(settings->flag_zoomed && p->is_scoped())
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(53, 213, 222), global->esp_font_small, 8, get_str(ZOOMED_TEXT));

  if(settings->flag_cloaked && p->is_cloaked())
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(147, 150, 150), global->esp_font_small, 8, get_str(CLOAKED_TEXT));

  if(settings->flag_cloaked && p->deadringer_deployed())
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(147, 150, 150), global->esp_font_small, 8, get_str(DEADRINGER_DEPLOYED_TEXT));

  if(settings->flag_bonked && p->is_bonked()){
    format_w(get_str(BONK_FORMAT),  PLAYER_ESP_BUF_SIZE, WXOR(L"BONK: %i"), (i32)p->get_condition_prediction_time(TF_COND_PHASE));
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(227, 189, 18), global->esp_font_small, 8, get_str(BONK_FORMAT));
  }

  if(settings->flag_disguised && p->is_disguised())
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(219, 219, 219), global->esp_font_small, 8, get_str(DISGUISE_TEXT));

  if(settings->flag_ubercharged && p->is_ubercharged() && !p->has_condition(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED))
    esp_data->add(ESP_TEXT_TYPE_RIGHT, team_col, global->esp_font_small, 8, get_str(UBERCHARGE_TEXT));

  if(settings->flag_ubercharged_hidden && p->has_condition(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED))
    esp_data->add(ESP_TEXT_TYPE_RIGHT, team_col, global->esp_font_small, 8, get_str(UBERCHARGE_HIDDEN_TEXT));

  if(settings->flag_taunting && p->has_condition(TF_COND_TAUNTING))
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(207, 97, 174), global->esp_font_small, 8, get_str(TAUNT_TEXT));

  if(settings->flag_crit_boosted && p->is_crit_boosted())
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 175, 255), global->esp_font_small, 8, get_str(CRIT_BOOST_TEXT));

  if(settings->flag_crit_boosted && wep != nullptr){
    if(wep->is_market_gardener() && p->has_condition(TF_COND_BLASTJUMPING))
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 175, 255), global->esp_font_small, 8, get_str(CRIT_BOOST_TEXT));
  }

  if(settings->flag_damage_buff && p->has_condition(TF_COND_OFFENSEBUFF))
    esp_data->add(ESP_TEXT_TYPE_RIGHT, team_col, global->esp_font_small, 8, get_str(DAMAGE_BUFF_TEXT));

  if(settings->flag_demo_charge && p->has_condition(TF_COND_SHIELD_CHARGE))
    esp_data->add(ESP_TEXT_TYPE_RIGHT, team_col, global->esp_font_small, 8, get_str(DEMO_CHARGE_TEXT));

  if(settings->flag_crit_cola && p->has_condition(TF_COND_ENERGY_BUFF))
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(120, 77, 184), global->esp_font_small, 8, get_str(CRIT_COLA_TEXT));

  if(settings->flag_on_fire && p->has_condition(TF_COND_BURNING))
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(242, 140, 51), global->esp_font_small, 8, get_str(BURN_TEXT));

  if(settings->flag_jarated && p->has_condition(TF_COND_URINE))
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(242, 229, 51), global->esp_font_small, 8, get_str(JARATE_TEXT));

  if(settings->flag_bleeding && p->has_condition(TF_COND_BLEEDING))
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(179, 11, 11 ), global->esp_font_small, 8, get_str(BLEED_TEXT));

  if(settings->flag_defense_buff){
    if(p->has_condition(TF_COND_DEFENSEBUFF_HIGH))
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(227, 18, 18), global->esp_font_small, 8, get_str(DEFENSE_BUFF_HIGH_TEXT));
    else if(p->has_condition(TF_COND_DEFENSEBUFF)){
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(227, 18, 18), global->esp_font_small, 8, get_str(DEFENSE_BUFF_TEXT));
    }
  }

  if(settings->flag_speed_heal_buff && p->has_condition(TF_COND_REGENONDAMAGEBUFF))
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(18, 122, 227), global->esp_font_small, 8, get_str(SPEED_HEAL_BUFF_TEXT));

  if(settings->flag_mad_milked && p->has_condition(TF_COND_MAD_MILK))
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(227, 227, 227), global->esp_font_small, 8, get_str(MILK_TEXT));

  if(settings->flag_quick_fix_uber && p->has_condition(TF_COND_MEGAHEAL))
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(219, 219, 219), global->esp_font_small, 8, get_str(QUICK_FIX_UBER_TEXT));

  if(settings->flag_hitmans_buff && p->has_condition(TF_COND_SNIPERCHARGE_RAGE_BUFF))
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(196, 151, 102), global->esp_font_small, 8, get_str(FOCUS_BUFF_TEXT));

  if(settings->flag_bullet_resist_charge && p->is_bullet_charge()){
    format_w(get_str(BULLET_RESIST_CHARGE_FORMAT), PLAYER_ESP_BUF_SIZE, WXOR(L"BULLET RESIST CHARGE: %i"), (i32)p->get_condition_prediction_time(TF_COND_MEDIGUN_UBER_BULLET_RESIST));
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 175, 255), global->esp_font_small, 8, get_str(BULLET_RESIST_CHARGE_FORMAT));
  }
  else if(settings->flag_bullet_resist && p->is_bullet_resist())
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 175, 255), global->esp_font_small, 8, get_str(BULLET_RESIST_TEXT));

  if(settings->flag_blast_resist_charge && p->is_blast_charge()){
    format_w(get_str(BLAST_RESIST_CHARGE_FORMAT), PLAYER_ESP_BUF_SIZE, WXOR(L"BLAST RESIST CHARGE: %i"), (i32)p->get_condition_prediction_time(TF_COND_MEDIGUN_UBER_BLAST_RESIST));
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 175, 255), global->esp_font_small, 8, get_str(BLAST_RESIST_CHARGE_FORMAT));
  }
  else if(settings->flag_blast_resist && p->is_blast_resist())
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 175, 255), global->esp_font_small, 8, get_str(BLAST_RESIST_TEXT));

  if(settings->flag_fire_resist_charge && p->is_fire_charge()){
    format_w(get_str(FIRE_RESIST_CHARGE_FORMAT), PLAYER_ESP_BUF_SIZE, WXOR(L"FIRE RESIST CHARGE: %i"), (i32)p->get_condition_prediction_time(TF_COND_MEDIGUN_UBER_FIRE_RESIST));
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 175, 255), global->esp_font_small, 8, get_str(FIRE_RESIST_CHARGE_FORMAT));
  }
  else if(settings->flag_fire_resist && p->is_fire_resist())
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 175, 255), global->esp_font_small, 8, get_str(FIRE_RESIST_TEXT));

  if(settings->flag_medigun_debuff && p->has_condition(TF_COND_MEDIGUN_DEBUFF))
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(188, 232, 100), global->esp_font_small, 8, get_str(MEDIGUN_DEBUFF_TEXT));

  if(settings->flag_healing_debuff && p->has_condition(TF_COND_HEALING_DEBUFF))
    esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(188, 232, 100), global->esp_font_small, 8, get_str(HEAL_DEBUFF_TEXT));

  if(settings->flag_show_healing){
    c_player_data* data = p->data();
    if(data == nullptr){
      return;
    }

    if(data->hp_healed_timeout > math::time(false)){
      format_w(get_str(HEAL_AMOUNT_FORMAT), PLAYER_ESP_BUF_SIZE, WXOR(L"%i+ HP"), data->hp_healed);
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(122, 255, 136), global->esp_font_small, 8, get_str(HEAL_AMOUNT_FORMAT));
    }
  }

  if(settings->flag_has_objective){
    if(p->is_carrying_objective())
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 188, 32), global->esp_font_small, 8, get_str(HAS_OBJECTIVE_TEXT));
  }
}

u32 c_player_esp::fetch_render_flags(c_player_esp_settings* settings){
  u32 flags = 0;

  if(settings->box || settings->box_outlines)
    flags |= ESP_FLAG_BOX;

  if(settings->box_outlines)
    flags |= ESP_FLAG_OUTLINES;

  if(!config->visual.cheap_text_rendering)
    flags |= ESP_FLAG_TEXT_OUTLINES;

  return flags;
}

CFLAG_O0 void c_player_esp::run(){
  precache();
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  float time = math::time();
  for(u32 i = 1; i <= globaldata->max_clients; i++){
    c_base_player* p = global->entity_list->get_entity(i);
    if(p == nullptr)
      continue;

    c_player_esp_settings* settings = utils::get_player_esp_settings(p);
    if(settings == nullptr)
      continue;

    if(!p->is_valid(!settings->dormant_enabled))
      continue;

    if(!should_draw(p))
      continue;

    c_player_data* data = p->data();
    if(data == nullptr)
      continue;

    if(!data->has_transform)
      continue;

    u32 render_esp_flags = fetch_render_flags(settings);
    s_box_data box_data  = math::calculate_box_region(data->obb_min, data->obb_max, data->transform);
    if(!box_data.valid)
      continue;

    if(p->is_dormant()){
      if(data->dormant_timeout <= time)
        continue;
    }

    // box thinning
    {
      static float thin_value_x = 0.65f;
      static float thin_value_y = 0.95f;
      box_data.size.x       =  math::ceil((float)box_data.size.x * thin_value_x);                   // thin box x
      box_data.pos.x        += math::ceil((float)box_data.size.x * ( 1.f - thin_value_x ) * 0.5f);  // thin box x
      box_data.size.y       =  math::ceil((float)box_data.size.y * thin_value_y);                   // thin box y
      box_data.pos.y        += math::ceil((float)box_data.size.y * ( 1.f - thin_value_y ) * 0.5f);  // thin box y

      box_data.size.x = math::biggest(box_data.size.x, 9);
      box_data.size.y = math::biggest(box_data.size.y, 18);
    }

    c_esp_data<32> ex_data;

    bool steam_friend = p->is_steam_friend();

    // health bar
    if(settings->healthbar){
      colour hp_col;
      if(utils::get_health_colour(p->health(), p->max_health(), &hp_col))
        ex_data.add(ESP_BAR_TYPE_LEFT, math::clamp((float)p->health() / (float)p->max_health(), 0.f, 1.f), settings->health_bar_fractions, hp_col);  
    }

    if(settings->ubercharge_bar){
      if(p->is_player_class(TF_CLASS_MEDIC)){
        c_base_weapon* medigun = p->get_weapon_from_belt(1, false);
        if(medigun != nullptr){
          float uber_fraction = math::clamp(medigun->uber_charge_level(), 0.f, 1.f);

          if(uber_fraction > 0.f)
            ex_data.add(ESP_BAR_TYPE_LEFT, uber_fraction, (medigun->weapon_id() == WPN_Vaccinator && uber_fraction >= 0.25f) ? 4 : 0, medigun->weapon_id() == WPN_Vaccinator ? rgb(255, 175, 255) : rgb(30, 150, 255));
        }
      }
    }

    colour bland_colour    = get_bland_colour(p);

    // draw name
    if(settings->name){
      s_player_info info = p->info();
      if(steam_friend && config->visual.use_steam_nicknames){
        i8* steam_name = cheat_detection->get_steam_name(i);
        convert::str2wstr(steam_name != nullptr ? steam_name : info.name, get_str(PLAYER_NAME_FORMAT), 128);
      }
      else
        convert::str2wstr(info.name, get_str(PLAYER_NAME_FORMAT), 128);

      ex_data.add(ESP_TEXT_TYPE_TOP, bland_colour, global->esp_font_large, 12, get_str(PLAYER_NAME_FORMAT));
    }

    if(settings->friend_tag && steam_friend)
      ex_data.add(ESP_TEXT_TYPE_TOP, colour(config->visual.player_esp_friend_colour), global->esp_font_large, 12, (!steam_friend && data->party_member) ? get_str(PARTY_TEXT) : get_str(FRIEND_TEXT));

    #if defined(DEV_MODE)
    bool test_top_text = false;
    #else
    bool test_top_text = false;
    #endif
    // draw cheater detection info.
    if(settings->cheater_bot_alert && !steam_friend){
      if(cheat_detection->is_cheating_bot(i))
        ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 45, 30), global->esp_font_large, 12, get_str(BOT_TEXT));
      else if(cheat_detection->is_cheating(i) || test_top_text)
        ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 45, 30), global->esp_font_large, 12, get_str(CHEATER_TEXT));
      //else if(data->suspicious_timer >= time || data->playerlist.mark_as_suspicious)
      //  ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 125, 30), global->esp_font_large, 12, get_str(SUSPICIOUS_TEXT));


      if(data->fake_latency_timer >= time)
        ex_data.add(ESP_TEXT_TYPE_RIGHT, (i32)(globaldata->cur_time) % 2 == 1 ? rgb(255, 255, 30) : rgb(255, 125, 30), global->esp_font_small, 8, get_str(ABNORMAL_LATENCY_TEXT));
    }

    // Show the dormant text.
    if(settings->dormant_render_text && p->is_dormant()|| test_top_text)
      ex_data.add(ESP_TEXT_TYPE_TOP, rgb(181, 181, 181), global->esp_font_large, 12, get_str(DORMANT_TEXT));
    
    if(settings->flag_show_duels && p->is_dueling())
      ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 128, 16), global->esp_font_large, 12, get_str(DUEL_TEXT));

   //#if defined(DEV_MODE)
   //if(data->is_rijin_user)
   //  ex_data.add(ESP_TEXT_TYPE_TOP, rgb(228, 32, 255), global->esp_font_large, 12, get_str(RIJIN_USER_TEXT));
   //#endif

    //if(data->is_dev_esp)
    //  ex_data.add(ESP_TEXT_TYPE_TOP, rgb(228, 32, 255), global->esp_font_large, 12, get_str(RIJIN_STAFF_TEXT));

    if(!data->playerlist.ignore_player && data->playerlist.priority_mode && data->playerlist.priority > 0){
      format_w(get_str(PRIORITY_FORMAT), PLAYER_ESP_BUF_SIZE, WXOR(L"P %i"), data->playerlist.priority);
      ex_data.add(ESP_TEXT_TYPE_TOP, colour(255, 190, 40, 255), global->esp_font_large, 12, get_str(PRIORITY_FORMAT));
    }

    if(config->hvh.resolver && config->hvh.resolver_esp_status){
      if(resolver->should_run(p)){
        std::wstring info;
        if(math::abs(data->networked_ang.x) >= 90.f){
          info = resolver->is_playerlist_correcting(RESOLVER_MANUAL_PITCH, p) ? get_str(RESOLVER_MP_TEXT) : get_str(RESOLVER_P_TEXT);
        }

        if(data->resolver_yaw_active_time > time || data->playerlist.resolver_force){
          if(resolver->is_playerlist_correcting(RESOLVER_MANUAL_YAW, p))
            info += info.length() > 0 ? get_str(RESOLVER_A_MY_TEXT) : get_str(RESOLVER_MY_TEXT);
          else
            info += info.length() > 0 ? get_str(RESOLVER_AY_TEXT) : get_str(RESOLVER_Y_TEXT);
        }

        if(info.length() > 0)
          ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 45, 30), global->esp_font_large, 12, info + get_str(RESOLVER_TEXT));
      }

      if(resolver->is_manual_input_mode_active() && global->resolver_manual_target == i)
        ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 45, 30), global->esp_font_large, 16, get_str(RESOLVER_MANUAL_ARROW_TEXT));
    }

    if(config->visual.show_estimated_aim_target){
      if(i == target_list->best_target_index){
        i32    size = math::clamp(config->visual.estimated_aim_target_size, 10, 14);
        colour clr  = rgba(config->visual.estimated_aim_target_colour[0], config->visual.estimated_aim_target_colour[1], config->visual.estimated_aim_target_colour[2], 255);

        if(config->visual.estimated_aim_target_circle){
          ex_data.add(ESP_TEXT_TYPE_TOP, clr, global->esp_font_large, size, get_str(AIM_TARGET_CIRCLE_TEXT));
        }
        else if(config->visual.estimated_aim_target_triangle)
          ex_data.add(ESP_TEXT_TYPE_TOP, clr, global->esp_font_large, size, get_str(RESOLVER_MANUAL_ARROW_TEXT));
        else
          ex_data.add(ESP_TEXT_TYPE_TOP, clr, global->esp_font_large, size, get_str(AIM_TARGET_LINE_ARROW_TEXT));
      }
    }

    // draw flags
    if(settings->flags)
      add_flags(p, &ex_data);

     // draw weapon name
    if(settings->weapon_name){
      ex_data.add(ESP_TEXT_TYPE_BOTTOM, bland_colour, global->esp_font_small, 8, data->tfdata.weapon_name);
    }

    // draw distance
    if(settings->distance){
      i32 dist = math::smallest(1024, math::biggest(0, localplayer->origin().distance_meter(p->origin())));
      ex_data.add(ESP_TEXT_TYPE_BOTTOM, bland_colour, global->esp_font_small, 10, _dist[dist]);
    }

    render->draw_esp_box(box_data.pos, box_data.size, get_draw_colour(p), render_esp_flags, &ex_data);
  }
}