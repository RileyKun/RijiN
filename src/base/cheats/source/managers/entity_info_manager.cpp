#include "../../../link.h"
c_base_entity_info_manager* entity_info = nullptr;

void c_base_entity_info_manager::init(u32 max_entities){
  max_entities_count = max_entities;
  table = (s_entity_info*)malloc(size_of_struct() * max_entities);
}

void c_base_entity_info_manager::think(u32& stage){
  if(stage != FRAME_NET_UPDATE_POSTDATAUPDATE_END)
    return;


  c_internal_base_player* localplayer = utils::localplayer();

  for(i32 i = 1; i <= global_internal->entity_list->get_highest_index(); i++){
    c_internal_base_entity* entity = global_internal->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    s_client_class* cc = entity->get_client_class();
    if(cc == nullptr)
      continue;

    s_entity_info* data = get_data(i);
    if(data == nullptr)
      continue;

    i32   model_first_char = 0;
    i8*   model_name       = nullptr;
    void* model            = entity->get_model();
    if(model != nullptr){
      model_name       = global_internal->model_info->get_model_name(model);
      if(model_name != nullptr)
        model_first_char = (i32)model_name[0];
    }

    const uptr cur_id = (i + cc->id + (i32)cc->name[0] + model_first_char + (uptr)model);
    if(cur_id != data->id){
      DBG("[!] Updating entity info for %i %s - %p\n", i, cc->name, model);
      data->id              = cur_id;
      data->class_id        = cc->id;
      data->class_name_hash = HASH_RT(cc->name);
      data->model_hash      = model_name != nullptr ? HASH_RT(model_name) : 0;
      data->model_ptr       = model;

      data->origin          = entity->origin();
      data->abs_origin      = entity->get_abs_origin();
      data->abs_angles      = entity->get_abs_angles();
      data->obb_min         = entity->obb_mins();
      data->obb_max         = entity->obb_maxs();

      data->is_on_team      = localplayer != nullptr ? (localplayer->team() == entity->team()) : false;
      data->meter_dist      = localplayer != nullptr ? (localplayer->origin().distance_meter(entity->origin())) : 0;
      data->vector_dist     = localplayer != nullptr ? (localplayer->origin().distance(entity->origin())) : 0.f;
    }

  }
}