#pragma once

#include "entity_list_data.h"

/*
  As for a starter, I wanted to make the playerlist 1:1 with the entitylist.
  BUT, DO NOT MAKE THE SYSTEM THE SAME AS THE ENTITY LIST DO WHAT WAS DONE IN TF2.
  // In the d3d9 test env, I will use the entity_list_data system for tweaking it visually and then copy it visually over to TF2.
*/

#define ENTITY_LIST_OPTION_SIZE render->rescale(18)
#define ENTITY_LIST_OPTION_SIZE_FONT 18
class gcs_component_player_list : public gcs_component_base{

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
    vec3i   option_size = vec3i(size.x, ENTITY_LIST_OPTION_SIZE);
    i32     padding     = info()->auto_positioning_padding.y;
    colour  text_col    = scheme()->text;

    for(i32 team = 0; team < entity_list_data->max_team_process_count(); team++){
      for(i32 i = 0; i < entity_list_data->get_max_entries(); i++){
        s_entity_list_data* entry = entity_list_data->get_data_from_pos(i);
        if(!entry->set) // This entry isn't setup so ignore it.
          continue;

        if(entry->team != team)
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
          switch(entry->team){
            default: break;
            case 0:
            {
              clr = rgba(255, 255, 255, clr.w); 
              break;
            }
            case 1:
            {
              clr = rgba(188, 188, 188, clr.w); 
              break;
            }
            case 2:
            {
              clr = rgba(255, 64, 32, clr.w); 
              break;
            }
            case 3:
            {
              clr = rgba(32, 64, 255, clr.w); 
              break;
            }
          }
       
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

        colour tag_col = rgba(188, 188, 188, text_col.w);
        static wchar_t tag[1028];
        memset(tag, 0, sizeof(tag));

        wsprintfW(tag, WXOR(L"❓"));

        /*
              //★   = known
              //🚩  = bot / cheater
              //❓  = NN
              //♦️    = friend
              //🌐  = rijin user
        */

        if(entry->entity_index == 0){
          tag_col = rgba(255, 0, 0, text_col.w);
          wsprintfW(tag, WXOR(L"🛡️"));
        }
        else if(entry->entity_index == 1){
          tag_col = rgba(255, 0, 0, text_col.w);
          wsprintfW(tag, WXOR(L"🚩"));
        }
        else if(entry->entity_index == 2){
          tag_col = rgba(255, 0, 0, text_col.w);
          wsprintfW(tag, WXOR(L"🚩"));
        }
        else if(entry->entity_index == 3){
          tag_col = rgba(255, 255, 0, text_col.w);
          wsprintfW(tag, WXOR(L"★"));
        }
        else if(entry->entity_index == 4 || entry->entity_index == 5){
          tag_col = rgba(32, 255, 32, text_col.w);
          wsprintfW(tag, WXOR(L"♦️"));
        }
        else if(entry->entity_index == 10){
          tag_col = rgba(228, 32, 255, text_col.w);
          wsprintfW(tag, WXOR(L"🌐"));
        }

        vec3i tag_size;
        if(wcslen(tag) > 0){
          tag_size = font()->draw(pos + vec3i(padding / 2, ENTITY_LIST_OPTION_SIZE / 2), _is_in_rect ? ENTITY_LIST_OPTION_SIZE_FONT : ENTITY_LIST_OPTION_SIZE_FONT - (2), tag, tag_col, TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_CENTER);
        }
    
        static wchar_t buf[1028];
        wsprintfW(buf, WXOR(L"%ls"), entry->class_name);
        font()->draw(pos + vec3i(tag_size.x + padding, ENTITY_LIST_OPTION_SIZE / 2), _is_in_rect ? ENTITY_LIST_OPTION_SIZE_FONT - (2) : ENTITY_LIST_OPTION_SIZE_FONT - (4), buf, text_col, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);
        render->outlined_rect(pos, option_size, r);
  
        pos.y += ENTITY_LIST_OPTION_SIZE + (padding / 2);
      }
    }
  }
};