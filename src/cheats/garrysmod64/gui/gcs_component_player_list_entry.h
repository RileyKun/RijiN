#pragma once

#define PLAYER_LIST_OPTION_SIZE 18
//#define RENDER_BOTS
class gcs_component_player_list : public gcs_component_base{

  void setup() override{
     set_size(vec3i(info()->size.x, PLAYER_LIST_OPTION_SIZE));
  }

  void think(vec3i pos, vec3i size) override{
  }

  void pre_run() override{
  }

  void post_run() override{
  }

  vec3i size() override{
    if(globaldata->max_clients <= 1 || !utils::is_in_game())
      return info()->size;

    u32 count = 0;
    for(i32 i = 1; i <= globaldata->max_clients; i++){
      if(i == global->localplayer_index)
        continue;

      c_base_player* player = global->entity_list->get_entity(i);
      if(player == nullptr)
        continue;

      #if !defined(RENDER_BOTS)
      if(player->info().fake_player)
        continue;
      #endif

      c_player_data* data = utils::get_player_data(i);
      if(data == nullptr)
        continue;

      #if !defined(RENDER_BOTS)
      if(!data->friends_id)
        continue;
      #endif

      count++;
    }

    return count > 0 ? vec3i(info()->size.x, (PLAYER_LIST_OPTION_SIZE + (info()->auto_positioning_padding.y / 2)) * count) : info()->size;
  }

  void draw(vec3i pos, vec3i size){
    vec3i   option_size = vec3i(size.x, PLAYER_LIST_OPTION_SIZE);
    i32     padding     = info()->auto_positioning_padding.y;
    colour  text_col    = scheme()->text;

    bool valid_players = false;
    if(globaldata->max_clients > 0){
      {
        for(i32 i = 1; i <= globaldata->max_clients; i++){
          if(i == global->localplayer_index)
            continue;
  
          c_base_player* player = global->entity_list->get_entity(i);
          if(player == nullptr)
            continue;
  
          c_player_data* data = utils::get_player_data(i);
          if(data == nullptr)
            continue;
  
          s_player_info info;
          if(!global->engine->get_player_info(i, &info))
            continue;
  
  #if !defined(RENDER_BOTS)
            if(!data->friends_id)
              continue;
    
            if(info.fake_player)
              continue;
  #endif

    
          colour  r           = text_col * 0.75f;
          bool    _is_in_rect = is_in_rect(pos, option_size);
  
          // Highlight selected index.
          if(global->playerlist_current_player_index == i){
            r = scheme()->main;
            render->filled_rect(pos, option_size, r * 0.5f);
          }
          else{
            colour clr = player_esp->get_draw_colour(player);
            clr.w = text_col.w;
            render->filled_rect(pos, option_size, clr * 0.75f);
          }
  
          if(_is_in_rect){
            r = scheme()->main;
            render->filled_rect(pos, option_size, r * 0.25f);
    
            // Mouse1 Click
            if(input() & GCS_IN_M1_CLICK){
              // Copy data from the player to our global playerlist data.
              memcpy(&global->playerlist_data, &data->playerlist, sizeof(c_player_list_data));
  
              global->playerlist_current_player_index = i;
              global->playerlist_has_valid_player     = true;
              emit_click_sound(1);
            }
            else if(input() & GCS_IN_M2_CLICK){
              i8 buf[512];
              wsprintfA(buf, XOR("https://steamcommunity.com/profiles/[U:1:%i]"), info.friends_id);
              global->steamcontext->steamfriends->game_overlay_to_web_page(buf);
              emit_click_sound(1);
            }
          }
  
          colour tag_col = rgba(188, 188, 188, text_col.w);
          static wchar_t tag[1028];
          memset(tag, 0, sizeof(tag));
          wsprintfW(tag, WXOR(L"❓"));

        /*
              //★   = known
              //🚩  = bot / cheater
              //🚫  = ignored
              //❓  = NN
              //🎯  = priority
              //♦️    = friend
              //🌐  = rijin user
        */

           #if defined(DEV_MODE)
            if(data->is_rijin_user){
              wsprintfW(tag, WXOR(L"🌐"));
              tag_col = rgba(228, 32, 255, text_col.w);
            }
          #endif

          if(data->steam_friend){
            tag_col = rgba(96, 255, 100, text_col.w);
            wsprintfW(tag, WXOR(L"♦️"));
          }
          else if(data->playerlist.priority_mode && data->playerlist.priority > 0){
            tag_col = rgba(255, 32, 32, text_col.w); 
            wsprintfW(tag, WXOR(L"🎯"));
          }
          else if(data->playerlist.ignore_player){
            tag_col = rgba(255, 32, 32, text_col.w);
            wsprintfW(tag, WXOR(L"🚫"));
          }
          else if(data->is_cheater || data->playerlist.mark_as_cheater){
            wsprintfW(tag, WXOR(L"🚩"));
            tag_col = rgb(255, 0, 0);
          }
  
          // If the player is commonly known as an alias then we'll replace the tag_col with something yellow.
          //if(data->has_alias && !data->steam_friend){
          //  if(!(data->is_cheater || data->is_cheater_bot) || data->playerlist.mark_as_legit) // Only replace the tag itself if they're not cheating.
          //    wsprintfW(tag, WXOR(L"★"));
 
          //  tag_col = rgb(255, 255, 0);
          //}

          vec3i tag_size;
          if(wcslen(tag) > 0){
            tag_size = font()->draw(pos + vec3i(padding / 2, PLAYER_LIST_OPTION_SIZE / 2), _is_in_rect ? PLAYER_LIST_OPTION_SIZE : PLAYER_LIST_OPTION_SIZE - 2, tag, tag_col, TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_CENTER);
          }
      
          static wchar_t buf[1028];
          static wchar_t player_name[64];
          convert::str2wstr(info.name, player_name, sizeof(player_name));
          wsprintfW(buf, WXOR(L"%ls"), player_name);
          font()->draw(pos + vec3i(tag_size.x + padding, PLAYER_LIST_OPTION_SIZE / 2), _is_in_rect ? PLAYER_LIST_OPTION_SIZE - 2 : PLAYER_LIST_OPTION_SIZE - 4, buf, text_col, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);
          render->outlined_rect(pos, option_size, r);
      
          valid_players = true;
          pos.y += PLAYER_LIST_OPTION_SIZE + (padding / 2);
        }
      }
    }

    if(!valid_players)
      font()->draw(pos + vec3i(option_size.x / 2, PLAYER_LIST_OPTION_SIZE), 14, globaldata->max_clients > 0 ? WXOR(L"No players in your game") : WXOR(L"Not in-game"), scheme()->text, TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_TOP);

    global->playerlist_has_valid_player = global->playerlist_current_player_index > 0;

    // Copy over global player list data info over to the player's playerlist data.
    if(global->playerlist_has_valid_player){
      c_player_data* data = utils::get_player_data(global->playerlist_current_player_index);
      if(data != nullptr)
        memcpy(&data->playerlist, &global->playerlist_data, sizeof(c_player_list_data));
    }
  }
};