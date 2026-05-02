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

    c_player_data* data = p->data();
    if(data == nullptr)
      return rgb(255, 255, 255);

    colour draw_col                 = rgb(255,255,255);
    switch(colour_type){
      default:
      case PLAYER_ESP_RENDER_TYPE_TEXT:
      {

        if(localplayer->is_steam_friend(p))
          draw_col = flt_array2clr(config->visual.player_esp_friend_colour);
        else
          draw_col = flt_array2clr(config->visual.player_esp_colour);

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
        if(localplayer->is_steam_friend(p))
          draw_col = flt_array2clr(config->visual.player_chams_friend_colour);
        else
          draw_col = flt_array2clr(config->visual.player_chams_colour);

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
        if(localplayer->is_steam_friend(p))
          draw_col = flt_array2clr(config->visual.player_chams_overlay_friend_colour);
        else
          draw_col = flt_array2clr(config->visual.player_chams_overlay_colour);

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
        if(localplayer->is_steam_friend(p))
          draw_col = flt_array2clr(config->visual.player_glow_friend_colour);
        else
          draw_col = flt_array2clr(config->visual.player_glow_colour);

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

  template<auto c>
  ALWAYSINLINE void add_flags(c_base_player* p, c_esp_data<c>* esp_data){
    if(p == nullptr || esp_data == nullptr)
      return;

    c_player_esp_settings* settings = utils::get_player_esp_settings(p);
    if(settings == nullptr)
      return;

    c_player_data* data = p->data();
    if(data == nullptr)
      return;

    if(settings->flag_staff && p->is_staff())
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 98, 32), global->esp_font_small, 8, L"STAFF MEMBER");

    if(settings->flag_demi_god && p->is_demi_god_mode())
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(150, 255, 180), global->esp_font_small, 8, L"DEMI-GOD");

    if(settings->flag_god_mode && p->is_god_moded())
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 255, 155), global->esp_font_small, 8, L"GOD MODE");

    if(settings->flag_noclipped && p->move_type == MOVETYPE_NOCLIP && p->player_vehicle <= 0)
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 92, 92), global->esp_font_small, 8, L"NOCLIPPED");

    if(settings->flag_in_vehicle && p->player_vehicle > 0)
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 92, 92), global->esp_font_small, 8, L"IN VEHICLE");

    if(settings->flag_on_fire && (p->entity_flags & FL_ONFIRE || p->on_fire))
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(242, 140, 51), global->esp_font_small, 8, L"ON FIRE");

    if(settings->flag_frozen && p->is_frozen())
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(0, 105, 255), global->esp_font_small, 8, L"FROZEN");

    if(settings->flag_grabbed && data->grabbed_by_physgun)
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(0, 255, 200), global->esp_font_small, 8, L"GRABBED");

    if(settings->flag_hitboxes)
      esp_data->add(ESP_TEXT_TYPE_RIGHT, data->has_bones ? rgb(195, 255, 160) : rgb(62, 160, 0), global->esp_font_small, 8, data->has_bones ? L"HB" : L"OBB");

    if(settings->flag_armor){
      if(data->gmod_data.armor > 0){
        static wchar_t armor_buf[32];
        wsprintfW(armor_buf, WXOR(L"%i AP"), data->gmod_data.armor);
        esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(100, 168, 255), global->esp_font_small, 8, armor_buf);
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
      if(p == nullptr)
        continue;

      c_player_esp_settings* settings = utils::get_player_esp_settings(p);
      if(settings == nullptr)
        continue;

      if(settings->max_distance > 0 && localplayer->origin().distance_meter(p->origin()) > settings->max_distance)
        continue;

      if(!settings->show_localplayer || !utils::is_in_thirdperson()){
        if(p == localplayer)
          continue;
      }

      if(p->is_dormant() || !p->is_alive())
        continue;

      if(!should_draw(localplayer, p))
        continue;

      c_player_data* data = p->data();
      if(data == nullptr)
        continue;

      if(!data->has_transform)
        continue;

      s_box_data box_data = math::calculate_box_region(p->obb_mins(), p->obb_maxs(), data->transform);
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

      if(settings->armor_bar){
        colour hp_col;
        float  hp_fraction;

        if(data->gmod_data.armor > 0){
          hp_fraction = math::clamp((float)data->gmod_data.armor / 100.f, 0.f, 1.f);

          ex_data.add(ESP_BAR_TYPE_LEFT, hp_fraction, settings->armor_bar_fractions, rgb(100, 168, 255));
        }
      }

      colour bland_colour = rgb(255,255,255);

      // draw name
      if(settings->name){
         s_player_info info = p->info();

        static wchar_t buff[128];
        convert::str2wstr(info.name, buff, sizeof(buff));

        if(wcslen(data->gmod_data.unique_name) != 0)
          ex_data.add(ESP_TEXT_TYPE_TOP, bland_colour, global->esp_font_large, 12, data->gmod_data.unique_name);
        else
          ex_data.add(ESP_TEXT_TYPE_TOP, bland_colour, global->esp_font_large, 12, buff);
      }

      if(settings->friend_tag && localplayer->is_steam_friend(p))
        ex_data.add(ESP_TEXT_TYPE_TOP, colour(config->visual.player_esp_friend_colour), global->esp_font_large, 12, L"FRIEND");

      if(config->misc.custom_server_integrations && p->is_perpheads()){
        if(p->is_perpheads_police())
          ex_data.add(ESP_TEXT_TYPE_TOP, rgb(0,190,255), global->esp_font_large, 12, L"POLICE");

        if(p->is_perpheads_medic())
          ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255,0,218), global->esp_font_large, 12, L"MEDIC");
      }

      if(settings->cheater_alert_tag && cheat_detection->is_cheating(p))
        ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 45, 30), global->esp_font_large, 12, L"CHEATER");

      if(data->playerlist.priority_mode && data->playerlist.priority > 0){
        static wchar_t buff[32];
        wsprintfW(buff, L"P %i", data->playerlist.priority);
        ex_data.add(ESP_TEXT_TYPE_TOP, colour(255, 190, 40, 255), global->esp_font_large, 12, buff);
      }

      if(config->hvh.resolver && config->hvh.resolver_esp_status){
        if(resolver->should_run(p)){
          std::wstring info;
          
          if(resolver->is_using_pitch_aa(p))
            info = resolver->is_playerlist_correcting(RESOLVER_MANUAL_PITCH, p) ? L"MP" : L"P";

          if(data->resolver_yaw_active_time > time || data->playerlist.resolver_force){
            if(resolver->is_playerlist_correcting(RESOLVER_MANUAL_YAW, p))
              info += info.length() > 0 ? L"&MY": L"MY";
            else
              info += info.length() > 0 ? L"&Y" : L"Y";
          }
  
          if(info.length() > 0)
            ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 45, 30), global->esp_font_large, 12, info + L" RESOLVER");
        }
  
        if(resolver->is_manual_input_mode_active() && global->resolver_manual_target == i)
          ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 45, 30), global->esp_font_large, 16, L"▼");
      }

      #if defined(DEV_MODE)
      if(data->is_rijin_user)
        ex_data.add(ESP_TEXT_TYPE_TOP, rgb(228, 32, 255), global->esp_font_large, 12, WXOR(L"RIJIN"));
      #endif

      if(settings->flags)
        add_flags(p, &ex_data);

      if(settings->weapon_name){
        ex_data.add(ESP_TEXT_TYPE_BOTTOM, bland_colour, global->esp_font_small, 10, data->gmod_data.weapon_name); 
      }

      // draw distance
      if(settings->distance){
        // TODO: CACHE ME!!!
        static wchar_t dist_buf[17];
        wsprintfW(dist_buf, L"[ %im ]", localplayer->origin().distance_meter(p->origin()));

        ex_data.add(ESP_TEXT_TYPE_BOTTOM, bland_colour, global->esp_font_small, 10, dist_buf);
      }

      u32 esp_box_flags = config->visual.cheap_text_rendering ? 0 : ESP_FLAG_TEXT_OUTLINES;
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