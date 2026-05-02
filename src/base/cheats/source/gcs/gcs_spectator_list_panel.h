#pragma once

struct s_spectator_data{
  bool    valid;
  bool    spectating_localplayer;
  wchar_t spectator_name[33];
  wchar_t target_name[33];
  wchar_t observer_str[33];
  i32     observer_mode;
};

class gcs_spectator_list_panel : public gcs_scene{
public:

  gcs_component_window* window    = nullptr;
  vec3i                 window_size    = vec3i(150, 0);
  vec3i*                window_pos_ptr = nullptr;
  bool*                 active_ptr     = nullptr;
  bool*                 local_only_ptr = nullptr;
  s_spectator_data      data[128];
  bool                  needs_reset    = false;

  void init(vec3i* pos_ptr, bool* _active_ptr, bool* _local_only_ptr){
    static bool is_setup = false;
    if(window == nullptr)
      return;

    if(is_setup)
      return;

    is_setup = true;

    window_pos_ptr = pos_ptr;
    active_ptr     = _active_ptr;
    local_only_ptr = _local_only_ptr;
    reset_data();

    if(*window_pos_ptr == vec3i())
      *window_pos_ptr = vec3i(4, render->screen_size.y / 4);

    window->set_pos_ptr(window_pos_ptr);
    DBG("[+] gcs_spectator_list_panel set pos ptr\n");
  }

  ALWAYSINLINE void reset_data(){
    if(!needs_reset)
      return;

    needs_reset = false;
    for(i32 i = 0; i < 128; i++)
      data[i].valid = false;
  }

  NEVERINLINE bool does_entity_fail(i32 index){
    if(index == global_internal->localplayer_index)
      return true;

    c_internal_base_player* player = global_internal->entity_list->get_entity(index);
    if(player == nullptr)
      return true;

    if(player->is_base_alive() || player->is_dormant())
      return true;

    if(player->observer_target() <= 0 || player->observer_mode() != OBS_MODE_IN_EYE && player->observer_mode() != OBS_MODE_CHASE)
      return true;

    c_internal_base_player* spectator_target = global_internal->entity_list->get_entity_handle(player->observer_target());
    if(spectator_target == nullptr)
      return true;

    if(spectator_target->get_index() > global_internal->global_data->max_clients ||
      spectator_target->get_index() != global_internal->localplayer_index && wants_local_only())
      return true;

    return false;
  }

  // Call in game thread, in frame stage notify.
  NEVERINLINE void fetch_spectators(u32 stage){
    if(stage != FRAME_NET_UPDATE_END)
      return;

    c_internal_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr){ // If this occurs then we have more issues to worry about.
      reset_data();
      return;
    }

    if(!localplayer->is_base_alive()){
      reset_data();
      return;
    }

    //data[i].valid = false;
    needs_reset = true;
    for(i32 i = 1; i <= global_internal->global_data->max_clients; i++){
      if(does_entity_fail(i)){
        data[i].valid = false;
        continue;
      }

      c_internal_base_player* player = global_internal->entity_list->get_entity(i);
      if(player == nullptr){
        data[i].valid = false;
        continue;
      }

      c_internal_base_player* spectator_target = global_internal->entity_list->get_entity_handle(player->observer_target());
      if(spectator_target == nullptr){
        data[i].valid = false;
        continue;
      }

      data[i].spectating_localplayer = spectator_target->get_index() == global_internal->localplayer_index;
      data[i].observer_mode          = player->observer_mode();
      I(wsprintfW)(data[i].observer_str, player->observer_mode() == OBS_MODE_IN_EYE ? WXOR(L"1st") : WXOR(L"3rd"));

      convert::str2wstr(player->info().name, data[i].spectator_name, sizeof(data[i].spectator_name));
      convert::str2wstr(spectator_target->info().name, data[i].target_name, sizeof(data[i].target_name));
      data[i].valid = true;
    }
  }

  NEVERINLINE bool is_enabled(){
    if(active_ptr == nullptr){
      DBG("[!] active_ptr is nullptr\n");
      return false;
    }

    if(*active_ptr)
      return true;

    return false;
  }

  NEVERINLINE bool wants_local_only(){
    if(local_only_ptr == nullptr)
      return false;

    if(*local_only_ptr)
      return true;

    return false;
  }

  bool setup() override;
};