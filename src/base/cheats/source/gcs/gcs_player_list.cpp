#include "../../../link.h"


void gcs_base_player_list::draw(vec3i pos, vec3i size){
  assert_private(global_internal->base_cheat_detection != nullptr && "needed for this game");
  const vec3i base_option_size = vec3i(size.x, PLAYER_LIST_OPTION_SIZE);
  const i32   padding          = info()->auto_positioning_padding.y;

  const colour text_col        = scheme()->text;
  const colour selected_clr    = scheme()->main * 0.5f;
  const colour hover_clr = text_col * 0.75f;

  i32 rendered_players = 0;
  if(utils::is_in_game()){
    for(i32 team = 0; team < get_max_team_count(); team++){
      for(i32 i = 0; i <= global_internal->global_data->max_clients; i++){
        if(i == global_internal->localplayer_index)
          continue;

        c_entity_info* entity_info = global_internal->base_cheat_detection->get_entity_info(i);
        if(entity_info == nullptr)
          continue;

        s_player_info info = entity_info->info->info;

        c_internal_base_player* player = global_internal->entity_list->get_entity(i);
        if(player == nullptr){
          if(team != 0)
            continue;

        }
        else{
          if(player->team() != team)
            continue;
        }

        colour main_clr  = global_internal->playerlist_current_player_index == i ? selected_clr : get_team_clr(i);
        main_clr.w = text_col.w;

        // Base rect.
        render->filled_rect(pos, base_option_size, main_clr * 0.5f);

        colour hover_clr = text_col * 0.75f;
        bool input_enabled = input_allowed(i);
        bool hovering      = is_in_rect(pos, base_option_size);
        if(hovering){
          render->filled_rect(pos, base_option_size, selected_clr * 0.95f);

          if(input() & GCS_IN_M1_CLICK && input_enabled){
            on_player_list_selected_index(i);

            global_internal->playerlist_current_player_index = i;
            global_internal->playerlist_has_valid_player     = true;
          }
          else if(input() & GCS_IN_M2_CLICK){
            i8 buf[1024];
            wsprintfA(buf, XOR("https://steamcommunity.com/profiles/%s"), info.guid);
            assert(global_internal->steamcontext != nullptr);
            assert(global_internal->steamcontext->steamfriends != nullptr);
            global_internal->steamcontext->steamfriends->game_overlay_to_web_page(buf);
          }
        }

        // Fetch the emoji we wanna use as an icon!
        auto emoji_pair = get_emoji(compute_flags(i));
        colour       icon_clr = emoji_pair.first;
        std::wstring icon     = emoji_pair.second;

        static wchar_t tag[1024];
        wsprintfW(tag, WXOR(L"%ls"), icon.c_str()); // we don't wanna pass something on the stack.

        const u32   font_size = (hovering && input_enabled) ? PLAYER_LIST_OPTION_SIZE : (PLAYER_LIST_OPTION_SIZE - render->rescale(2));
        const vec3i tag_size  = font()->draw(pos + vec3i(padding / 2, PLAYER_LIST_OPTION_SIZE / 2), font_size, tag, icon_clr, TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_CENTER);

        static wchar_t player_name[128];
        convert::str2wstr(info.name, player_name, sizeof(player_name));

        font()->draw(pos + vec3i(tag_size.x + padding, PLAYER_LIST_OPTION_SIZE / 2), font_size, player_name, text_col, TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_CENTER);
        render->outlined_rect(pos, base_option_size, main_clr);

        pos.y += PLAYER_LIST_OPTION_SIZE + (padding / 2);
        rendered_players++;
      }
    }
  }

  if(!rendered_players){
    font()->draw(pos + vec3i(base_option_size.x / 2, PLAYER_LIST_OPTION_SIZE),
                 14, global_internal->global_data->max_clients > 0 ? WXOR(L"No players in your game") : WXOR(L"Not in-game"), scheme()->text, TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_TOP);
  }


  global_internal->playerlist_has_valid_player = global_internal->playerlist_current_player_index > 0;
  if(global_internal->playerlist_has_valid_player)
    on_player_list_render_finished();
}