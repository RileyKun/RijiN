#include "../../../link.h"

void c_base_lag_compensation::frame_stage_notify(u32 stage){
  if(stage != FRAME_NET_UPDATE_POSTDATAUPDATE_END)
    return;

  c_internal_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  cookie_block_check_return();

  static s_global_data old_globaldata;
  memcpy(&old_globaldata, global_internal->global_data, sizeof(s_global_data));

  // This is very important.
  global_internal->global_data->cur_time   = math::ticks_to_time(localplayer->tick_base());
  global_internal->global_data->frame_time = math::ticks_to_time(1);


  for(i32 i = 1; i <= global_internal->global_data->max_clients; i++){
    c_internal_base_player* player = global_internal->entity_list->get_entity(i);
    if(player == nullptr)
      continue;

    if(!player->is_base_alive() || player->is_dormant())
      continue;

    s_studio_hdr* hdr = player->get_studio_hdr();
    if(hdr == nullptr)
      continue;

    s_lc_data* lc = get_data(player);
    if(lc == nullptr)
      continue;

    lc->think();
    if(!lc->needs_update(player))
      continue;

    s_lc_record new_record;
    {
      is_setting_up_bones = true;
      bool result = player->setup_bones((matrix3x4*)&new_record.bone_matrix, true);
      is_setting_up_bones = false;

      if(!result){
        DBG("[!] c_base_lag_compensation::frame_stage_notify failed to update bones on player %s\n", player->info().name);
        continue;
      }

      new_record.bone_count = hdr->num_bones;
    }

    new_record.origin           = player->origin();
    new_record.velocity         = player->velocity();
    new_record.obb_center       = player->origin() + ((player->obb_mins() + player->obb_maxs()) * 0.5f);
    new_record.obb_mins         = player->obb_mins();
    new_record.obb_maxs         = player->obb_maxs();
    new_record.viewangles       = player->viewangles();
    new_record.simulation_time  = player->simulation_time();
    new_record.entity_flags     = player->entity_flags();
    new_record.server_tick      = global_internal->client_state->server_tick();

    lc->add_record(new_record);

    // If teleport distance check fails on record 0, all records after 0 are going to be broken
    if(lc->is_record_broken(0)){
      for(u32 i = 1; i < lc->max_records; i++){
        s_lc_record* record = lc->get_record(i);

        // end of track
        if(record == nullptr)
          break;

        record->broken = true;
      }
    }

    call_entity_prediction_required(player);
  }


  cookie_block_check_return();

  memcpy(global_internal->global_data, &old_globaldata, sizeof(s_global_data));
}