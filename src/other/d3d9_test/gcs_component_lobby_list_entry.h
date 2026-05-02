#pragma once

#include "entity_list_data.h"

/*
  As for a starter, I wanted to make the playerlist 1:1 with the entitylist.
  BUT, DO NOT MAKE THE SYSTEM THE SAME AS THE ENTITY LIST DO WHAT WAS DONE IN TF2.
  // In the d3d9 test env, I will use the entity_list_data system for tweaking it visually and then copy it visually over to TF2.
*/

#define ENTITY_LIST_OPTION_SIZE (32)
class gcs_component_lobby_list : public gcs_component_base{

  void setup() override{
    set_size(vec3i(info()->size.x, ENTITY_LIST_OPTION_SIZE));
  }

  void think(vec3i pos, vec3i size) override{
  }

  void pre_run() override{
  }

  void post_run() override{
  }

  vec3i size() override{
    if(entity_list_data->get_list_count() <= 0)
      return info()->size;

    return vec3i(info()->size.x, (ENTITY_LIST_OPTION_SIZE + (info()->auto_positioning_padding.y / 2)) * entity_list_data->get_list_count());
  }

  //
  void draw(vec3i pos, vec3i size){
    vec3i   option_size    = vec3i(size.x, ENTITY_LIST_OPTION_SIZE);
    i32     padding        = info()->auto_positioning_padding.y;
    i32     height_padding = (6);
    colour  text_col       = scheme()->text;

    {
      for(i32 i = 0; i < entity_list_data->get_max_entries(); i++){
        s_entity_list_data* entry = entity_list_data->get_data_from_pos(i);
        if(!entry->set) // This entry isn't setup so ignore it.
          continue;

        colour  r           = text_col * 0.75f;
        bool    _is_in_rect = is_in_rect(pos, option_size);


        // Highlight selected index.
        if(entity_list_data->selected_index == i){
          r = scheme()->main;
          render->filled_rect(pos, option_size, r * 0.5f);
        }
        else{
          colour clr = scheme()->main;

          render->filled_rect(pos, option_size, clr * 0.75f);
        }
  
        if(_is_in_rect){
          r = scheme()->main;
          render->filled_rect(pos, option_size, r * 0.25f);
  
          // Mouse1 Click
          if(input() & GCS_IN_M1_CLICK){
            memcpy(&entity_list_data->selected_item, entry, sizeof(s_entity_list_data));
            entity_list_data->selected_index = i;
          }
        }

        colour tag_col        = rgba(188, 188, 255, text_col.w);
        colour lobby_col      = text_col;
        colour lobby_size_col = text_col;

        static wchar_t buf[1028];
        static wchar_t tag[1028];
        static wchar_t lobby_status[1028];
        static wchar_t lobby_size[1028];

        memset(tag, 0, sizeof(tag));
        memset(lobby_status, 0, sizeof(lobby_status));
        memset(lobby_size, 0, sizeof(lobby_size));

        i32 main_font_size = (20);
 

        wsprintfW(lobby_status, L"What's going in the lobby?");
        wsprintfW(lobby_size, L"WW - 6/6");

        // Searching = 🔍
        // In-game   = 🎮
 
        wsprintfW(tag, WXOR(L"🔍"));
        if(i % 2 == 0){
          wsprintfW(tag, WXOR(L"🎮"));
          tag_col = rgba(32, 255, 128, text_col.w);
        }

        vec3i tag_size;
        i32 offset = padding / 2;
        if(wcslen(tag) > 0)
          tag_size = font()->draw(pos + vec3i(offset, (ENTITY_LIST_OPTION_SIZE / 2)), main_font_size, tag, tag_col, TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_CENTER);
          
        offset += tag_size.x + padding;
        wsprintfW(buf, WXOR(L"%ls"), entry->class_name);
        vec3i f_pos = font()->draw(pos + vec3i(offset, (ENTITY_LIST_OPTION_SIZE / 2) + padding / 2), main_font_size, buf, text_col, TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_TOP);
          
        // Lobby status
        font()->draw(pos + vec3i(offset, (ENTITY_LIST_OPTION_SIZE - padding / 2) + 1), 12, lobby_status, lobby_col, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_TOP);

        // Member count
        font()->draw(pos + vec3i(option_size.x * 0.85f, (ENTITY_LIST_OPTION_SIZE / 2)), 18, lobby_size, lobby_size_col, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);
        
        render->outlined_rect(pos, option_size, r);
  
        pos.y += ENTITY_LIST_OPTION_SIZE + (padding / 2);
      }
    }
  }
};