#pragma once

enum player_esp_render_type{
  PLAYER_ESP_RENDER_TYPE_TEXT = 0,
  PLAYER_ESP_RENDER_TYPE_CHAMS,
  PLAYER_ESP_RENDER_TYPE_GLOW,
  PLAYER_ESP_RENDER_TYPE_CHAMS_OVERLAY,
};

class c_player_esp{
public:

  ALWAYSINLINE bool should_draw(c_base_player* localplayer, c_base_player* p){
    if(localplayer == nullptr || p == nullptr)
      return false;

    c_player_esp_settings* settings = utils::get_player_esp_settings(p);
    if(settings == nullptr)
      return false;

    return settings->enabled;
  }

  ALWAYSINLINE colour get_draw_colour(c_base_player* p, i32 colour_type = PLAYER_ESP_RENDER_TYPE_TEXT){
    colour draw_col            = rgb(255,255,255);
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return draw_col;

    if(p == nullptr)
      return draw_col;

    c_player_data* data = p->data();
    if(data == nullptr)
     return draw_col;

    switch(colour_type){
      default:
      case PLAYER_ESP_RENDER_TYPE_TEXT:
      {

        if(config->visual.player_esp_use_team_colours){
          if(p->team() == 2)
            draw_col = flt_array2clr(config->visual.player_esp_allied_team_colour);
          else if(p->team() == 3)
            draw_col = flt_array2clr(config->visual.player_esp_axis_team_colour);
        }
        else if(config->visual.player_esp_use_enemyteam_colours){
          if(p->team() != localplayer->team())
            draw_col = flt_array2clr(config->visual.player_esp_enemy_colour);
          else if(p->team() == localplayer->team())
            draw_col = flt_array2clr(config->visual.player_esp_team_colour);
        }

        if(p->is_steam_friend()){
          if(config->visual.player_esp_friends_use_single_colour) // Single colour
            draw_col = flt_array2clr(config->visual.player_esp_friend_colour);
          else if(config->visual.player_esp_friends_use_mult_colour){ // Multi-colour!! (ANTI-RACIST)!!!
            if(config->visual.player_esp_use_team_colours){
              if(p->team() == 3)
                draw_col = flt_array2clr(config->visual.player_esp_friend_axis_team_colour);
              else if(p->team() == 2)
                draw_col = flt_array2clr(config->visual.player_esp_friend_allied_team_colour);
            }
            else if(config->visual.player_esp_use_enemyteam_colours){
              if(p->team() != localplayer->team())
                draw_col = flt_array2clr(config->visual.player_esp_friend_enemy_colour);
              else if(p->team() == localplayer->team())
                draw_col = flt_array2clr(config->visual.player_esp_friend_team_colour);
            }
          }
        }

        if(data->playerlist.custom_esp_colour)
          draw_col = flt_array2clr(data->playerlist.esp_colour);

        if(config->visual.player_esp_aimbot_target){
          if(global->aimbot_target_index == p->get_index())
            draw_col = flt_array2clr(config->visual.player_esp_aimbot_target_colour);
        }

        break;
      }
      case PLAYER_ESP_RENDER_TYPE_CHAMS:
      {
        if(config->visual.player_chams_use_team_colours){
          if(p->team() == 3)
            draw_col = flt_array2clr(config->visual.player_chams_axis_team_colour);
          else if(p->team() == 2)
            draw_col = flt_array2clr(config->visual.player_chams_allied_team_colour);
        }
        else if(config->visual.player_chams_use_enemyteam_colours){
          if(p->team() != localplayer->team())
            draw_col = flt_array2clr(config->visual.player_chams_enemy_colour);
          else if(p->team() == localplayer->team())
            draw_col = flt_array2clr(config->visual.player_chams_team_colour);
        }

        if(p->is_steam_friend()){
          if(config->visual.player_chams_friends_use_single_colour) // Single colour
            draw_col = flt_array2clr(config->visual.player_chams_friend_colour);
          else if(config->visual.player_chams_friends_use_mult_colour){ // Multi-colour!! (ANTI-RACIST)!!!
            if(config->visual.player_chams_use_team_colours){
              if(p->team() == 3)
                draw_col = flt_array2clr(config->visual.player_chams_friend_axis_team_colour);
              else if(p->team() == 2)
                draw_col = flt_array2clr(config->visual.player_chams_friend_allied_team_colour);
            }
            else if(config->visual.player_chams_use_enemyteam_colours){
              if(p->team() != localplayer->team())
                draw_col = flt_array2clr(config->visual.player_chams_friend_enemy_colour);
              else if(p->team() == localplayer->team())
                draw_col = flt_array2clr(config->visual.player_chams_friend_team_colour);
            }
          }
        }

        if(data->playerlist.custom_chams_base_colour)
          draw_col = flt_array2clr(data->playerlist.chams_colour);

        if(config->visual.player_chams_aimbot_target){
          if(global->aimbot_target_index == p->get_index())
            draw_col = flt_array2clr(config->visual.player_chams_aimbot_target_colour);
        }

        break;
      }
      case PLAYER_ESP_RENDER_TYPE_CHAMS_OVERLAY:
      {
        if(config->visual.player_chams_use_team_colours){
          if(p->team() == 3)
            draw_col = flt_array2clr(config->visual.player_chams_overlay_axis_team_colour);
          else if(p->team() == 2)
            draw_col = flt_array2clr(config->visual.player_chams_overlay_allied_team_colour);
        }
        else if(config->visual.player_chams_use_enemyteam_colours){
          if(p->team() != localplayer->team())
            draw_col = flt_array2clr(config->visual.player_chams_overlay_enemy_colour);
          else if(p->team() == localplayer->team())
            draw_col = flt_array2clr(config->visual.player_chams_overlay_team_colour);
        }

        if(p->is_steam_friend()){
          if(config->visual.player_chams_friends_use_single_colour) // Single colour
            draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_colour);
          else if(config->visual.player_chams_friends_use_mult_colour){ // Multi-colour!! (ANTI-RACIST)!!!
            if(config->visual.player_chams_use_team_colours){
              if(p->team() == 3)
                draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_axis_team_colour);
              else if(p->team() == 2)
                draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_allied_team_colour);
            }
            else if(config->visual.player_chams_use_enemyteam_colours){
              if(p->team() != localplayer->team())
                draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_enemy_colour);
              else if(p->team() == localplayer->team())
                draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_team_colour);
            }
          }
        }

        if(data->playerlist.custom_chams_overlay_colour)
          draw_col = flt_array2clr(data->playerlist.chams_overlay_colour);

        if(config->visual.player_chams_overlay_aimbot_target){
          if(global->aimbot_target_index == p->get_index())
            draw_col = flt_array2clr(config->visual.player_chams_overlay_aimbot_target_colour);
        }

        break;
      }
      case PLAYER_ESP_RENDER_TYPE_GLOW:
      {
        if(config->visual.player_glow_use_team_colours){
          if(p->team() == 3)
            draw_col = flt_array2clr(config->visual.player_glow_axis_team_colour);
          else if(p->team() == 2)
            draw_col = flt_array2clr(config->visual.player_glow_allied_team_colour);
        }
        else if(config->visual.player_glow_use_enemyteam_colours){
          if(p->team() != localplayer->team())
            draw_col = flt_array2clr(config->visual.player_glow_enemy_colour);
          else if(p->team() == localplayer->team())
            draw_col = flt_array2clr(config->visual.player_glow_team_colour);
        }

        if(p->is_steam_friend()){
          if(config->visual.player_glow_friends_use_single_colour) // Single colour
            draw_col = flt_array2clr(config->visual.player_glow_friend_colour);
          else if(config->visual.player_glow_friends_use_mult_colour){ // Multi-colour!! (ANTI-RACIST)!!!
            if(config->visual.player_glow_use_team_colours){
              if(p->team() == 3)
                draw_col = flt_array2clr(config->visual.player_glow_friend_axis_team_colour);
              else if(p->team() == 2)
                draw_col = flt_array2clr(config->visual.player_glow_friend_allied_team_colour);
            }
            else if(config->visual.player_glow_use_enemyteam_colours){
              if(p->team() != localplayer->team())
                draw_col = flt_array2clr(config->visual.player_glow_friend_enemy_colour);
              else if(p->team() == localplayer->team())
                draw_col = flt_array2clr(config->visual.player_glow_friend_team_colour);
            }
          }
        }

        if(data->playerlist.custom_glow_colour)
          draw_col = flt_array2clr(data->playerlist.glow_colour);

        if(config->visual.player_glow_aimbot_target){
          if(global->aimbot_target_index == p->get_index())
            draw_col = flt_array2clr(config->visual.player_glow_aimbot_target_colour);
        }
        break;
      }
    }

    return draw_col;
  }

  ALWAYSINLINE colour get_bland_colour(c_base_player* p){
    if(p == nullptr)
      return rgb(255, 255, 255);

    return p->is_dormant() ? rgb(181, 181, 181) : rgb(255, 255, 255);
  }

  template<auto c>
  ALWAYSINLINE void add_flags(c_base_player* p, c_esp_data<c>* esp_data){
    if(p == nullptr || esp_data == nullptr)
      return;

    colour team_col = get_draw_colour(p);

    c_player_esp_settings* settings = utils::get_player_esp_settings(p);
    if(settings == nullptr)
      return;

    c_player_data* data = p->data();
    if(data == nullptr)
      return;

    if(settings->flag_has_defuser && p->has_defuser())
      esp_data->add(ESP_TEXT_TYPE_RIGHT, team_col, global->esp_font_small, 8, L"DEFUSER");

    if(settings->flag_is_defusing && p->is_defusing())
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 255, 152), global->esp_font_small, 8, L"DEFUSING");

    if(settings->flag_has_night_vision && p->has_nv())
      esp_data->add(ESP_TEXT_TYPE_RIGHT, team_col, global->esp_font_small, 8, L"HAS NVG");

    if(settings->flag_is_using_night_vision && p->is_using_nv())
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(66, 225, 128), global->esp_font_small, 8, L"USING NVG");

    if(settings->flag_has_helmet){
      std::wstring armor_str;

      if(p->has_helmet())
        armor_str += L"H";

      if(p->armor_value() > 0)
        armor_str += L"K";

      if(armor_str.length() > 0){
        static wchar_t armor_type[18];
        wsprintfW(armor_type, L"%ls", armor_str.c_str());
        esp_data->add(ESP_TEXT_TYPE_RIGHT, team_col, global->esp_font_small, 8, armor_type);
      }
    }

  }

  void run(){
    if(!global->engine->is_in_game())
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    float time = math::time();
    for(i32 i = 1; i <= globaldata->max_clients; i++){
      c_base_player* p = global->entity_list->get_entity(i);
      if(p == nullptr || p == localplayer && !utils::is_in_thirdperson())
        continue;

      c_player_esp_settings* settings = utils::get_player_esp_settings(p);
      if(settings == nullptr)
        continue;

      if(p->is_dormant() && !settings->dormant_enabled || !p->is_alive())
        continue;

      if(!should_draw(localplayer, p))
        continue;

      c_player_data* data = p->data();
      if(data == nullptr)
        continue;

      if(!data->has_transform)
        continue;

      s_box_data box_data = math::calculate_box_region(data->obb_min, data->obb_max + vec3(0.f, 0.f, 12.f), data->transform);
      if(!box_data.valid)
        continue;

      if(p->is_dormant() && data->dormant_timeout <= time)
        continue;

      // box thinning
      {
        static float thin_value_x = 0.75f;
        static float thin_value_y = 0.95f;
        box_data.size.x       =  math::ceil((float)box_data.size.x * thin_value_x);                   // thin box x
        box_data.pos.x        += math::ceil((float)box_data.size.x * ( 1.f - thin_value_x ) * 0.5f);  // thin box x
        box_data.size.y       =  math::ceil((float)box_data.size.y * thin_value_y);                   // thin box y
        box_data.pos.y        += math::ceil((float)box_data.size.y * ( 1.f - thin_value_y ) * 0.5f);  // thin box y

        box_data.size.x = math::biggest(box_data.size.x, 9);
        box_data.size.y = math::biggest(box_data.size.y, 18);
      }

      c_esp_data<32> ex_data;

      // health bar
      if(settings->health_bar){
        colour hp_col;
        float  hp_fraction;

        if(p->health() > 0){
          hp_fraction = math::clamp((float)p->health() / 100.f, 0.f, 1.f);

          if(hp_fraction >= 0.75f)
            hp_col = rgb(46, 204, 113);
          else if(hp_fraction >= 0.5f)
            hp_col = rgb(241, 196, 15);
          else if(hp_fraction >= 0.25f)
            hp_col = rgb(230, 126, 34);
          else
            hp_col = rgb(231, 76, 60);

          ex_data.add(ESP_BAR_TYPE_LEFT, hp_fraction, settings->health_bar_fractions, hp_col);
        }
      }

      if(settings->armor_bar){
        colour hp_col;
        float  hp_fraction;

        if(p->armor_value() > 0){
          hp_fraction = math::clamp((float)p->armor_value() / 100.f, 0.f, 1.f);

          ex_data.add(ESP_BAR_TYPE_LEFT, hp_fraction, settings->armor_bar_fractions, rgb(100, 168, 255));
        }
      }

      colour bland_colour = get_bland_colour(p);

      // draw name
      if(settings->name){
         s_player_info info = p->info();

        static wchar_t buff[128];
        convert::str2wstr(info.name, buff, sizeof(buff));

        ex_data.add(ESP_TEXT_TYPE_TOP, bland_colour, global->esp_font_large, 12, buff);
      }

      const bool is_friend = cheat_detection->is_friend(i);
      if(settings->friend_tag && is_friend)
        ex_data.add(ESP_TEXT_TYPE_TOP, colour(config->visual.player_esp_friend_colour), global->esp_font_large, 12, WXOR(L"FRIEND"));

      if(!is_friend){
        if(cheat_detection->is_cheating(i))
          ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 45, 30), global->esp_font_large, 12, WXOR(L"CHEATER"));
        else if(cheat_detection->is_cheating_bot(i))
          ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 45, 30), global->esp_font_large, 12, WXOR(L"BOT"));
      }

      if(p->is_considered_spawn_protected() && settings->spawn_protected_tag){
        ex_data.add(ESP_TEXT_TYPE_TOP, flt_array2clr(config->visual.player_spawn_protected_text_colour), global->esp_font_large, 12, WXOR(L"SPAWN PROTECTED"));
      }

      if(p->is_dormant() && settings->dormant_render_text)
        ex_data.add(ESP_TEXT_TYPE_TOP, rgb(181, 181, 181), global->esp_font_large, 12, WXOR(L"DORMANT"));


      if(!data->playerlist.ignore_player && data->playerlist.priority_mode && data->playerlist.priority > 0){
        static wchar_t buff[32];
        wsprintfW(buff, WXOR(L"P %i"), data->playerlist.priority);
        ex_data.add(ESP_TEXT_TYPE_TOP, colour(255, 190, 40, 255), global->esp_font_large, 12, buff);
      }

        // draw flags
      if(settings->flags)
        add_flags(p, &ex_data);

      if(settings->weapon_name){
        c_base_weapon* weapon = p->get_weapon();
        if(weapon != nullptr)
           ex_data.add(ESP_TEXT_TYPE_BOTTOM, bland_colour, global->esp_font_small, 10, weapon->get_weapon_name());
      }

      // draw distance
      if(settings->distance){
        // TODO: CACHE ME!!!
        static wchar_t dist_buf[17];
        wsprintfW(dist_buf, WXOR(L"[ %im ]"), localplayer->origin().distance_meter(p->origin()));

        ex_data.add(ESP_TEXT_TYPE_BOTTOM, bland_colour, global->esp_font_small, 10, dist_buf);
      }

      u32 esp_box_flags = ESP_FLAG_TEXT_OUTLINES;
      {
        if(settings->box || settings->box_outlines)
          esp_box_flags |= ESP_FLAG_BOX;

        if(settings->box_outlines)
          esp_box_flags |= ESP_FLAG_OUTLINES;
      }

      render->draw_esp_box(box_data.pos, box_data.size, get_draw_colour(p), esp_box_flags, &ex_data);
    }

  }
};

CLASS_EXTERN(c_player_esp, player_esp);