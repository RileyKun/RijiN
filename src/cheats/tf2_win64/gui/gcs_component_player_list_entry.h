#pragma once


class gcs_component_player_list : public gcs_base_player_list{
public:
  i32 compute_flags(i32 index) override{
    i32 flags = gcs_base_player_list::compute_flags(index);

    c_player_data* data = utils::get_player_data(index);
    if(data == nullptr)
      return flags;

    if(!data->friends_id)
      return flags;

    if(data->playerlist.mark_as_friend)
      flags |= GCS_ICON_FRIEND;

    if(data->playerlist.mark_as_bot)
      flags |= GCS_ICON_KNOWN_BOT;
    else if(data->playerlist.mark_as_cheater)
      flags |= GCS_ICON_KNOWN_CHEATER;

    if(data->playerlist.mark_as_suspicious)
      flags |= GCS_ICON_SUSPECT;

    if(data->playerlist.ignore_player)
      flags |= GCS_ICON_AIMBOT_IGNORE;

    if(data->playerlist.priority_mode && data->playerlist.priority > 0)
      flags |= GCS_ICON_PRIORITY;

    return flags;
  }

  bool input_allowed(i32 index){
    c_player_data* data = utils::get_player_data(index);
    if(data == nullptr)
      return false;

    if(!data->friends_id)
      return false;

    return gcs_base_player_list::input_allowed(index);
  }

  colour get_team_colour_ent(c_internal_base_entity* entity) override{
    return player_esp->get_draw_colour(entity);
  }

  void on_player_list_selected_index(const i32 index) override{
    c_player_data* data = utils::get_player_data(index);
    if(data == nullptr)
      return;

    memcpy(&global->playerlist_data, &data->playerlist, sizeof(c_player_list_data));
  }

  void on_player_list_render_finished() override{
    c_player_data* data = utils::get_player_data(global->playerlist_current_player_index);
    if(data != nullptr)
      memcpy(&data->playerlist, &global->playerlist_data, sizeof(c_player_list_data));
  }
};