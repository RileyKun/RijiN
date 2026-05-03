#pragma once

class c_lag_compensation : public c_base_lag_compensation{
public:

  // Didn't know where to put this at so I put in the same location where we calculate the player's.
  void collect_entity_bone_matrix(){
    for(i32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
      c_base_entity* entity = global->entity_list->get_entity(i);
      if(entity == nullptr)
        continue;

      if(entity->is_dormant())
        continue;

      if(!entity->is_sentry() && !entity->is_dispenser() && !entity->is_teleporter())
        continue;

      c_entity_data* data = entity->entity_data();
      if(data == nullptr)
        continue;

      if(entity->simulation_time() <= data->last_simulation_time)
        continue;

      bool result = entity->setup_bones(data->bone_matrix, true);


      data->has_bone_matrix      = result;
      data->last_simulation_time = entity->simulation_time();
    }
  }

  void frame_stage_notify(u32 stage) override{
    if(stage != FRAME_NET_UPDATE_POSTDATAUPDATE_END)
      return;
    
    collect_entity_bone_matrix();
    c_base_lag_compensation::frame_stage_notify(stage);
  }

  void call_entity_prediction_required(c_internal_base_player* player) override;
};

CLASS_EXTERN(c_lag_compensation, lag_compensation);