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
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return rgb(255, 255, 255);

    colour draw_col                 = rgb(255,255,255);
    switch(colour_type){
      default:
      case PLAYER_ESP_RENDER_TYPE_TEXT:
      {

        if(config->visual.player_esp_use_team_colours){
          if(p->team == 2)
            draw_col = flt_array2clr(config->visual.player_esp_vigil_team_colour);
          else if(p->team == 3)
            draw_col = flt_array2clr(config->visual.player_esp_desper_team_colour);
          else if(p->team == 4)
            draw_col = flt_array2clr(config->visual.player_esp_bandio_team_colour);
          else if(p->team == 5)
            draw_col = flt_array2clr(config->visual.player_esp_rangers_team_colour);
          else{
            draw_col = flt_array2clr(config->visual.player_esp_unassigned_colour);
          }
        }
        else if(config->visual.player_esp_use_enemyteam_colours){
          if(p->team != localplayer->team || !utils::is_teamplay_enabled())
            draw_col = flt_array2clr(config->visual.player_esp_enemy_colour);
          else if(p->team == localplayer->team)
            draw_col = flt_array2clr(config->visual.player_esp_team_colour);
        }

        if(localplayer->is_steam_friend(p)){
          if(config->visual.player_esp_friends_use_single_colour) // Single colour
            draw_col = flt_array2clr(config->visual.player_esp_friend_colour);
          else if(config->visual.player_esp_friends_use_mult_colour){ // Multi-colour!! (ANTI-RACIST)!!!
            if(config->visual.player_esp_use_team_colours){
              if(p->team == 3)
                draw_col = flt_array2clr(config->visual.player_esp_friend_desper_team_colour);
              else if(p->team == 2)
                draw_col = flt_array2clr(config->visual.player_esp_friend_vigil_team_colour);
              else if(p->team == 4)
                draw_col = flt_array2clr(config->visual.player_esp_friend_bandio_team_colour);
              else if(p->team == 5)
                draw_col = flt_array2clr(config->visual.player_esp_friend_rangers_team_colour);
            }
            else if(config->visual.player_esp_use_enemyteam_colours){
              if(p->team != localplayer->team || !utils::is_teamplay_enabled())
                draw_col = flt_array2clr(config->visual.player_esp_friend_enemy_colour);
              else if(p->team == localplayer->team)
                draw_col = flt_array2clr(config->visual.player_esp_friend_team_colour);
            }
          }
        }

        if(config->visual.player_esp_aimbot_target){
          if(global->aimbot_target_index == p->get_index())
            draw_col = flt_array2clr(config->visual.player_esp_aimbot_target_colour);
        }

        break;
      }
      case PLAYER_ESP_RENDER_TYPE_CHAMS:
      {
        if(config->visual.player_chams_use_team_colours){
          if(p->team == 3)
            draw_col = flt_array2clr(config->visual.player_chams_desper_team_colour);
          else if(p->team == 2)
            draw_col = flt_array2clr(config->visual.player_chams_vigil_team_colour);
          else if(p->team == 4)
            draw_col = flt_array2clr(config->visual.player_chams_bandio_team_colour);
          else if(p->team == 5)
            draw_col = flt_array2clr(config->visual.player_chams_rangers_team_colour);
          else{
            draw_col = flt_array2clr(config->visual.player_chams_unassigned_colour);
          }
        }
        else if(config->visual.player_chams_use_enemyteam_colours){
          if(p->team != localplayer->team || !utils::is_teamplay_enabled())
            draw_col = flt_array2clr(config->visual.player_chams_enemy_colour);
          else if(p->team == localplayer->team)
            draw_col = flt_array2clr(config->visual.player_chams_team_colour);
        }

        if(localplayer->is_steam_friend(p)){
          if(config->visual.player_chams_friends_use_single_colour) // Single colour
            draw_col = flt_array2clr(config->visual.player_chams_friend_colour);
          else if(config->visual.player_chams_friends_use_mult_colour){ // Multi-colour!! (ANTI-RACIST)!!!
            if(config->visual.player_chams_use_team_colours){
              if(p->team == 3)
                draw_col = flt_array2clr(config->visual.player_chams_friend_desper_team_colour);
              else if(p->team == 2)
                draw_col = flt_array2clr(config->visual.player_chams_friend_vigil_team_colour);
              else if(p->team == 4)
                draw_col = flt_array2clr(config->visual.player_chams_friend_bandio_team_colour);
              else if(p->team == 5)
                draw_col = flt_array2clr(config->visual.player_chams_friend_rangers_team_colour);
            }
            else if(config->visual.player_chams_use_enemyteam_colours){
              if(p->team != localplayer->team || !utils::is_teamplay_enabled())
                draw_col = flt_array2clr(config->visual.player_chams_friend_enemy_colour);
              else if(p->team == localplayer->team)
                draw_col = flt_array2clr(config->visual.player_chams_friend_team_colour);
            }
          }
        }

        if(config->visual.player_chams_aimbot_target){
          if(global->aimbot_target_index == p->get_index())
            draw_col = flt_array2clr(config->visual.player_chams_aimbot_target_colour);
        }

        break;
      }
      case PLAYER_ESP_RENDER_TYPE_CHAMS_OVERLAY:
      {
        if(config->visual.player_chams_use_team_colours){
          if(p->team == 3)
            draw_col = flt_array2clr(config->visual.player_chams_overlay_desper_team_colour);
          else if(p->team == 2)
            draw_col = flt_array2clr(config->visual.player_chams_overlay_vigil_team_colour);
          else if(p->team == 4)
            draw_col = flt_array2clr(config->visual.player_chams_overlay_bandio_team_colour);
          else if(p->team == 5)
            draw_col = flt_array2clr(config->visual.player_chams_overlay_rangers_team_colour);
          else{
            draw_col = flt_array2clr(config->visual.player_chams_overlay_unassigned_colour);
          }
        }
        else if(config->visual.player_chams_use_enemyteam_colours){
          if(p->team != localplayer->team || !utils::is_teamplay_enabled())
            draw_col = flt_array2clr(config->visual.player_chams_overlay_enemy_colour);
          else if(p->team == localplayer->team)
            draw_col = flt_array2clr(config->visual.player_chams_overlay_team_colour);
        }

        if(localplayer->is_steam_friend(p)){
          if(config->visual.player_chams_friends_use_single_colour) // Single colour
            draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_colour);
          else if(config->visual.player_chams_friends_use_mult_colour){ // Multi-colour!! (ANTI-RACIST)!!!
            if(config->visual.player_chams_use_team_colours){
              if(p->team == 3)
                draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_desper_team_colour);
              else if(p->team == 2)
                draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_vigil_team_colour);
              else if(p->team == 4)
                draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_bandio_team_colour);
              else if(p->team == 5)
                draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_rangers_team_colour);
            }
            else if(config->visual.player_chams_use_enemyteam_colours){
              if(p->team != localplayer->team || !utils::is_teamplay_enabled())
                draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_enemy_colour);
              else if(p->team == localplayer->team)
                draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_team_colour);
            }
          }
        }

        if(config->visual.player_chams_overlay_aimbot_target){
          if(global->aimbot_target_index == p->get_index())
            draw_col = flt_array2clr(config->visual.player_chams_overlay_aimbot_target_colour);
        }

        break;
      }
      case PLAYER_ESP_RENDER_TYPE_GLOW:
      {
        if(config->visual.player_glow_use_team_colours){
          if(p->team == 3)
            draw_col = flt_array2clr(config->visual.player_glow_desper_team_colour);
          else if(p->team == 2)
            draw_col = flt_array2clr(config->visual.player_glow_vigil_team_colour);
          else if(p->team == 4)
            draw_col = flt_array2clr(config->visual.player_glow_bandio_team_colour);
          else if(p->team == 5)
            draw_col = flt_array2clr(config->visual.player_glow_rangers_team_colour);
          else{
            draw_col = flt_array2clr(config->visual.player_glow_unassigned_colour);
          }
        }
        else if(config->visual.player_glow_use_enemyteam_colours){
          if(p->team != localplayer->team || !utils::is_teamplay_enabled())
            draw_col = flt_array2clr(config->visual.player_glow_enemy_colour);
          else if(p->team == localplayer->team)
            draw_col = flt_array2clr(config->visual.player_glow_team_colour);
        }

        if(localplayer->is_steam_friend(p)){
          if(config->visual.player_glow_friends_use_single_colour) // Single colour
            draw_col = flt_array2clr(config->visual.player_glow_friend_colour);
          else if(config->visual.player_glow_friends_use_mult_colour){ // Multi-colour!! (ANTI-RACIST)!!!
            if(config->visual.player_glow_use_team_colours){
              if(p->team == 3)
                draw_col = flt_array2clr(config->visual.player_glow_friend_desper_team_colour);
              else if(p->team == 2)
                draw_col = flt_array2clr(config->visual.player_glow_friend_vigil_team_colour);
              else if(p->team == 4)
                draw_col = flt_array2clr(config->visual.player_glow_friend_bandio_team_colour);
              else if(p->team == 5)
                draw_col = flt_array2clr(config->visual.player_glow_friend_rangers_team_colour);
            }
            else if(config->visual.player_glow_use_enemyteam_colours){
              if(p->team != localplayer->team || !utils::is_teamplay_enabled())
                draw_col = flt_array2clr(config->visual.player_glow_friend_enemy_colour);
              else if(p->team == localplayer->team)
                draw_col = flt_array2clr(config->visual.player_glow_friend_team_colour);
            }
          }
        }

        if(config->visual.player_glow_aimbot_target){
          if(global->aimbot_target_index == p->get_index())
            draw_col = flt_array2clr(config->visual.player_glow_aimbot_target_colour);
        }
        break;
      }
    }

    return draw_col;
  }

  void run(){
    if(!global->engine->is_in_game())
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    for(i32 i = 1; i <= globaldata->max_clients; i++){
      c_base_player* p = global->entity_list->get_entity(i);
      if(p == nullptr || p == localplayer)
        continue;

      c_player_esp_settings* settings = utils::get_player_esp_settings(p);
      if(settings == nullptr)
        continue;

      if(p->is_dormant() || !p->is_alive())
        continue;

      if(!should_draw(localplayer, p))
        continue;

      c_player_data* data = p->data();
      if(data == nullptr)
        continue;

      s_box_data box_data = math::calculate_box_region(p->obb_mins(), p->obb_maxs(), p->rgfl_coordinate_frame);
      if(!box_data.valid)
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

        if(p->health > 0){
          hp_fraction = math::clamp((float)p->health / 100.f, 0.f, 1.f);

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

      colour bland_colour = rgb(255,255,255);

      if(settings->friend_tag && localplayer->is_steam_friend(p))
        ex_data.add(ESP_TEXT_TYPE_TOP, colour(config->visual.player_esp_friend_colour), global->esp_font_large, 12, L"FRIEND");

      // draw name
      if(settings->name){
         s_player_info info = p->info();

        static wchar_t buff[128];
        convert::str2wstr(info.name, buff);

        ex_data.add(ESP_TEXT_TYPE_TOP, bland_colour, global->esp_font_large, 12, buff);
      }

      if(settings->weapon_name){
        c_base_weapon* weapon = p->get_weapon();
        if(weapon != nullptr)
           ex_data.add(ESP_TEXT_TYPE_BOTTOM, bland_colour, global->esp_font_small, 10, weapon->get_weapon_name());
      }

      // draw distance
      if(settings->distance){
        // TODO: CACHE ME!!!
        static wchar_t dist_buf[17];
        wsprintfW(dist_buf, L"[ %im ]", localplayer->origin().distance_meter(p->origin()));

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