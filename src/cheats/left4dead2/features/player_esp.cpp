#include "../link.h"

CLASS_ALLOC(c_player_esp, player_esp);

bool c_player_esp::get_esp_name(c_base_player* p, wchar_t* out){
  if(p == nullptr || out == nullptr)
    return false;
  
  u32 t = p->type();
  if(t & TYPE_REAL_PLAYER || t & TYPE_BOT)
    convert::str2wstr(p->info().name, out);
  else if(t & TYPE_INFECTED)
    convert::str2wstr(XOR("Infected"), out);
  else if(t & TYPE_HUNTER)
    convert::str2wstr(XOR("Hunter"), out);
  else if(t & TYPE_JOCKEY)
    convert::str2wstr(XOR("Jockey"), out);
  else if(t & TYPE_CHARGER)
    convert::str2wstr(XOR("Charger"), out);
  else if(t & TYPE_BOOMER)
    convert::str2wstr(XOR("Boomer"), out);
  else if(t & TYPE_SMOKER)
    convert::str2wstr(XOR("Smoker"), out);
  else if(t & TYPE_SPITTER)
    convert::str2wstr(XOR("Spitter"), out);
  else if(t & TYPE_TANK)
    convert::str2wstr(XOR("Tank"), out);
  else if(t & TYPE_WITCH)
    convert::str2wstr(XOR("Witch"), out);

  return true;
}

colour c_player_esp::get_draw_colour(c_base_player* p, i32 colour_type){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return rgb(255, 255, 255);

  c_entity_data* data = p->data();
  if(data == nullptr)
    return rgb(255, 255, 255);

  u32 t              = p->type();
  colour draw_colour = rgb(255,255,255);
  switch(colour_type){
    default: break;
    case PLAYER_ESP_RENDER_TYPE_TEXT:
    {
      if(t & TYPE_REAL_PLAYER || t & TYPE_BOT)
        draw_colour = localplayer->team()== p->team()? flt_array2clr(config->visual.box_player_colour_friendly) : flt_array2clr(config->visual.box_player_colour_enemy);
      else if(t & TYPE_INFECTED)
        draw_colour = flt_array2clr(config->visual.box_colour_infected);
      else if(t & TYPE_HUNTER)
        draw_colour = flt_array2clr(config->visual.box_colour_hunter);
      else if(t & TYPE_CHARGER)
        draw_colour = flt_array2clr(config->visual.box_colour_charger);
      else if(t & TYPE_SMOKER)
        draw_colour = flt_array2clr(config->visual.box_colour_smoker);
      else if(t & TYPE_BOOMER)
        draw_colour = flt_array2clr(config->visual.box_colour_boomer);
      else if(t & TYPE_SPITTER)
        draw_colour = flt_array2clr(config->visual.box_colour_spitter);
      else if(t & TYPE_TANK)
        draw_colour = flt_array2clr(config->visual.box_colour_tank);
      else if(t & TYPE_WITCH)
        draw_colour = flt_array2clr(config->visual.box_colour_witch);
      else if(t & TYPE_JOCKEY)
        draw_colour = flt_array2clr(config->visual.box_colour_jockey);

      if(localplayer->is_steam_friend(p))
        draw_colour = flt_array2clr(config->visual.player_esp_friend_colour);

      if(data->playerlist.custom_esp_colour)
        draw_colour = flt_array2clr(data->playerlist.esp_colour);

      draw_colour.w = 255;
      break;
    }
    case PLAYER_ESP_RENDER_TYPE_CHAMS:
    {
      if(t & TYPE_REAL_PLAYER || t & TYPE_BOT)
        draw_colour = localplayer->team()== p->team()? flt_array2clr(config->visual.chams_player_colour_friendly) : flt_array2clr(config->visual.chams_player_colour_enemy);
      else if(t & TYPE_INFECTED)
        draw_colour = flt_array2clr(config->visual.chams_colour_infected);
      else if(t & TYPE_HUNTER)
        draw_colour = flt_array2clr(config->visual.chams_colour_hunter);
      else if(t & TYPE_CHARGER)
        draw_colour = flt_array2clr(config->visual.chams_colour_charger);
      else if(t & TYPE_SMOKER)
        draw_colour = flt_array2clr(config->visual.chams_colour_smoker);
      else if(t & TYPE_BOOMER)
        draw_colour = flt_array2clr(config->visual.chams_colour_boomer);
      else if(t & TYPE_SPITTER)
        draw_colour = flt_array2clr(config->visual.chams_colour_spitter);
      else if(t & TYPE_TANK)
        draw_colour = flt_array2clr(config->visual.chams_colour_tank);
      else if(t & TYPE_WITCH)
        draw_colour = flt_array2clr(config->visual.chams_colour_witch);
      else if(t & TYPE_JOCKEY)
        draw_colour = flt_array2clr(config->visual.chams_colour_jockey);

      if(localplayer->is_steam_friend(p))
        draw_colour = flt_array2clr(config->visual.player_chams_friend_colour);

      if(data->playerlist.custom_chams_base_colour)
        draw_colour = flt_array2clr(data->playerlist.chams_colour);

      break;
    }
    case PLAYER_ESP_RENDER_TYPE_CHAMS_OVERLAY:
    {
      if(t & TYPE_REAL_PLAYER || t & TYPE_BOT)
        draw_colour = localplayer->team()== p->team()? flt_array2clr(config->visual.chams_overlay_player_colour_friendly) : flt_array2clr(config->visual.chams_overlay_player_colour_enemy);
      else if(t & TYPE_INFECTED)
        draw_colour = flt_array2clr(config->visual.chams_overlay_colour_infected);
      else if(t & TYPE_HUNTER)
        draw_colour = flt_array2clr(config->visual.chams_overlay_colour_hunter);
      else if(t & TYPE_CHARGER)
        draw_colour = flt_array2clr(config->visual.chams_overlay_colour_charger);
      else if(t & TYPE_SMOKER)
        draw_colour = flt_array2clr(config->visual.chams_overlay_colour_smoker);
      else if(t & TYPE_BOOMER)
        draw_colour = flt_array2clr(config->visual.chams_overlay_colour_boomer);
      else if(t & TYPE_SPITTER)
        draw_colour = flt_array2clr(config->visual.chams_overlay_colour_spitter);
      else if(t & TYPE_TANK)
        draw_colour = flt_array2clr(config->visual.chams_overlay_colour_tank);
      else if(t & TYPE_WITCH)
        draw_colour = flt_array2clr(config->visual.chams_overlay_colour_witch);
      else if(t & TYPE_JOCKEY)
        draw_colour = flt_array2clr(config->visual.chams_overlay_colour_jockey);

      if(localplayer->is_steam_friend(p))
        draw_colour = flt_array2clr(config->visual.player_chams_overlay_friend_colour);

      if(data->playerlist.custom_chams_overlay_colour)
        draw_colour = flt_array2clr(data->playerlist.chams_overlay_colour);

      break;
    }
    case PLAYER_ESP_RENDER_TYPE_GLOW:
    {
      if(t & TYPE_REAL_PLAYER || t & TYPE_BOT)
        draw_colour = localplayer->team()== p->team()? flt_array2clr(config->visual.glow_player_colour_friendly) : flt_array2clr(config->visual.glow_player_colour_enemy);
      else if(t & TYPE_INFECTED)
        draw_colour = flt_array2clr(config->visual.glow_colour_infected);
      else if(t & TYPE_HUNTER)
        draw_colour = flt_array2clr(config->visual.glow_colour_hunter);
      else if(t & TYPE_CHARGER)
        draw_colour = flt_array2clr(config->visual.glow_colour_charger);
      else if(t & TYPE_SMOKER)
        draw_colour = flt_array2clr(config->visual.glow_colour_smoker);
      else if(t & TYPE_BOOMER)
        draw_colour = flt_array2clr(config->visual.glow_colour_boomer);
      else if(t & TYPE_SPITTER)
        draw_colour = flt_array2clr(config->visual.glow_colour_spitter);
      else if(t & TYPE_TANK)
        draw_colour = flt_array2clr(config->visual.glow_colour_tank);
      else if(t & TYPE_WITCH)
        draw_colour = flt_array2clr(config->visual.glow_colour_witch);
      else if(t & TYPE_JOCKEY)
        draw_colour = flt_array2clr(config->visual.glow_colour_jockey);

      if(localplayer->is_steam_friend(p))
        draw_colour = flt_array2clr(config->visual.player_glow_friend_colour);

      if(data->playerlist.custom_glow_colour)
        draw_colour = flt_array2clr(data->playerlist.glow_colour);

      break;
    }
  }

  return draw_colour;
}

template<auto c>
void c_player_esp::add_flags(c_base_player* p, c_esp_data<c>* esp_data){
  if(p == nullptr || esp_data == nullptr)
    return;

  colour team_col = get_draw_colour(p);
  u32 t = p->type();

  c_entity_data* data = p->data();
  if(data == nullptr)
    return;

  if(t & TYPE_INFECTED){
    if(data->idle)
      esp_data->add(ESP_TEXT_TYPE_RIGHT, team_col, global->esp_font_small, 8, L"IDLE");
    else if(p->is_infected_burning())
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 100, 32), global->esp_font_small, 8, L"ON FIRE");
  }
  else{
    if(p->is_invisible())
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(225, 225, 255), global->esp_font_small, 8, L"GHOST");

    if(t & TYPE_WITCH){
      if(p->is_witch_raged())
        esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(245, 48, 0), global->esp_font_small, 8, L"RAGED");
      else{
        if(p->wander_rage() > 0.0f){
          static wchar_t rage_meter[32];
          wsprintfW(rage_meter, WXOR(L"ANGER: %i%%"), (i32)(p->wander_rage() * 100.f));
          esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(238, 208, 0), global->esp_font_small, 8, rage_meter);
        }
      }
    }
  }
}

void c_player_esp::run(){
  if(!global->engine->is_in_game())
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
    s_entity_info* info = entity_info->get_data(i);
    if(info == nullptr)
      continue;

    c_base_player* p = global->entity_list->get_entity(i);

    if(p == nullptr)
      continue;

    if(!p->should_render())
      continue;

    if(p->is_dormant() || !p->is_alive())
      continue;

    c_entity_data* data = p->data();

    if(data == nullptr)
      continue;

    if(!data->has_transform)
      continue;

    u32 t = p->type();

    if(t == 0)
      continue;

    c_player_esp_settings* esp_settings = utils::get_player_esp_settings(p);
    assert(esp_settings != nullptr);

    if(!esp_settings->enabled)
      continue;

    if(esp_settings->show_localplayer){
      if(i == global->localplayer_index && !utils::is_in_thirdperson())
        continue;
    }
    else{
      if(i == global->localplayer_index)
        continue;
    }

    s_box_data box_data = math::calculate_box_region(info->obb_min, info->obb_max, data->transform);

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
    if(esp_settings->health_bar){
      colour hp_col;
      float  hp_fraction;

      if(p->health() > 0 && p->max_health() > 0){
        hp_fraction = math::clamp((float)p->health() / (float)p->max_health(), 0.f, 1.f);

        if(hp_fraction >= 0.75f)
          hp_col = rgb(46, 204, 113);
        else if(hp_fraction >= 0.5f)
          hp_col = rgb(241, 196, 15);
        else if(hp_fraction >= 0.25f)
          hp_col = rgb(230, 126, 34);
        else
          hp_col = rgb(231, 76, 60);

        ex_data.add(ESP_BAR_TYPE_LEFT, hp_fraction, esp_settings->health_bar_fractions, hp_col);
      }
    }

    colour bland_colour = rgb(255,255,255);

    // draw name
    if(esp_settings->name){
      static wchar_t name[128];
      get_esp_name(p, name);

      ex_data.add(ESP_TEXT_TYPE_TOP, bland_colour, global->esp_font_large, 12, name);
    }

    if(esp_settings->friend_tag && localplayer->is_steam_friend(p))
      ex_data.add(ESP_TEXT_TYPE_TOP, colour(config->visual.player_esp_friend_colour), global->esp_font_large, 12, WXOR(L"FRIEND"));

    if(!cheat_detection->is_friend(i) && cheat_detection->is_cheating(i)){
      ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 45, 30), global->esp_font_large, 12, WXOR(L"CHEATER"));
      c_entity_info* ent_info = cheat_detection->get_entity_info(i);
      if(ent_info != nullptr){
        if(ent_info->info->is_abusing_tickbase)
          ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 180, 72), global->esp_font_large, 12, WXOR(L"TICKBASE ABUSE"));
      }
    }

    if(!data->playerlist.ignore_player && data->playerlist.priority_mode && data->playerlist.priority > 0){
      static wchar_t buff[32];
      wsprintfW(buff, WXOR(L"P %i"), data->playerlist.priority);
      ex_data.add(ESP_TEXT_TYPE_TOP, colour(255, 190, 40, 255), global->esp_font_large, 12, buff);
    }

    #if defined(DEV_MODE)
    if(data->is_rijin_user)
      ex_data.add(ESP_TEXT_TYPE_TOP, rgb(228, 32, 255), global->esp_font_large, 12, WXOR(L"RIJIN"));
    #endif

    add_flags(p, &ex_data);

    // draw distance
    if(esp_settings->distance){
      static wchar_t dist_buf[17];
      wsprintfW(dist_buf, WXOR(L"[ %im ]"), info->meter_dist);

      ex_data.add(ESP_TEXT_TYPE_BOTTOM, bland_colour, global->esp_font_small, 10, dist_buf);
    }

    u32 esp_box_flags = ESP_FLAG_TEXT_OUTLINES;
    {
      if(esp_settings->box)
        esp_box_flags |= ESP_FLAG_BOX;

      if(esp_settings->box_outlines)
        esp_box_flags |= ESP_FLAG_BOX | ESP_FLAG_OUTLINES;
    }

    colour draw_colour = get_draw_colour(p, PLAYER_ESP_RENDER_TYPE_TEXT);


    render->draw_esp_box(box_data.pos, box_data.size, draw_colour, esp_box_flags, &ex_data);
  }
}