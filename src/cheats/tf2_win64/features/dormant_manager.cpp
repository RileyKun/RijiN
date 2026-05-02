#include "../link.h"

c_dormant_manager* dormant_manager = nullptr;

void c_dormant_manager::update(i32 index, vec3 pos, bool set_origin){
  c_base_entity* entity = global->entity_list->get_entity(index);
  if(entity == nullptr)
    return;

  if(!entity->is_dormant())
    return;

  c_base_player* player = entity->get_player();

  if(player != nullptr){
    set_origin = true; // Always set origin.
    c_player_data* data = player->data();
    if(data == nullptr)
      return;

    c_player_resource* pr = utils::get_player_resource();
    if(pr != nullptr){
      player->health()       = pr->get_health(index);
      player->player_class() = pr->get_player_class(index);
      player->team()         = pr->get_team(index);
    }

    data->dormant_timeout = math::time() + DORMANT_EXPIRE_TIME;
  }
  else{
    c_entity_data* data = entity->entity_data();
    if(data == nullptr)
      return;

    float expire_time = DORMANT_EXPIRE_TIME;
    if(entity->is_sentry())
      expire_time += 2.5f;

    data->dormant_timeout = math::time() + expire_time;
  }

  if(set_origin){
    entity->origin() = pos;
    entity->set_abs_origin(pos);
  }
}

void c_dormant_manager::on_dormant(c_internal_base_entity* internal_entity, bool dormant){
  c_base_entity* entity = (c_base_entity*)internal_entity;
  if(entity == nullptr)
    return;

  c_base_player* player = entity->get_player();
  if(player != nullptr){
    c_player_data* data = player->data();
    if(data == nullptr)
      return;

    data->dormant_timeout = dormant ? (math::time() + DORMANT_EXPIRE_TIME) : -1.f;
  }
  else{
    c_entity_data* data = entity->entity_data();
    if(data == nullptr)
      return;

    data->dormant_timeout = dormant ? (math::time() + DORMANT_EXPIRE_TIME) : -1.f;
  }
}

void c_dormant_manager::reset(i32 index){
  if(index <= globaldata->max_clients){
    c_player_data* data = utils::get_player_data(index);
    if(data == nullptr)
      return;

    data->dormant_timeout                = -1.f;
    data->dormant_death_timeout          = math::time() + 1.f;
    data->dormant_received_party_timeout = -1.f;
    data->dormant_send_party_timeout     = -1.f;
  }
  else{
    c_entity_data* data = utils::get_entity_data(index);
    if(data == nullptr)
      return;

    data->dormant_timeout                = -1.f;
    data->dormant_death_timeout          = math::time() + 1.f;
  }
}

void c_dormant_manager::read_party_player_data(c_party_chat_packet_entity_update* entity_update){
  if(!config->misc.party_network || !config->misc.party_network_far_esp)
    return;

  if(entity_update == nullptr)
    return;

  if(entity_update->sender == global->engine->get_local_player())
    return;

  float time = math::time();
  for(i32 i = 0; i < 6; i++){
    if(entity_update->data[i].index == global->engine->get_local_player() || entity_update->data[i].index == 0 || entity_update->data[i].pos == vec3(0, 0, 0))
      continue;

    c_base_player* player = global->entity_list->get_entity(entity_update->data[i].index);
    if(player == nullptr)
      continue;

    if(!player->is_dormant())
      continue;

    c_player_data* data = player->data();
    if(data == nullptr)
     continue;

    update(entity_update->data[i].index, entity_update->data[i].pos, true);
    data->dormant_received_party_timeout = time + DORMANT_EXPIRE_TIME;

    DBG("[!] Updating %s position by party networking (%2.2f, %2.2f, %2.2f)\n", player->info().name, entity_update->data[i].pos.x, entity_update->data[i].pos.y, entity_update->data[i].pos.z);
  }
}

void c_dormant_manager::send_party_player_data(u32 stage){
  if(!party_network->should_transmit())
    return;

  if(!config->misc.party_network_far_esp)
    return;

  if(stage != FRAME_NET_UPDATE_END)
    return;

  c_party_chat_packet_entity_update entity_update;
  if(global->localplayer == nullptr)
    return;
  
  float time = math::time();
  i32 i = 0;
  for(i32 index = 1; index <= globaldata->max_clients; index++){
    if(i >= 6)
      break;

    c_base_player* player = global->entity_list->get_entity(index);
    if(player == nullptr)
      continue;

    if(!player->is_alive() || player->is_dormant())
      continue;

    if(global->localplayer_team == player->team() && index != global->localplayer_index)
      continue;

    c_player_data* data = player->data();
    if(data == nullptr)
      continue;

    if(data->dormant_send_party_timeout > time)
      continue;

    entity_update.data[i].index = (u8)index;
    entity_update.data[i].pos   = player->origin();
    data->dormant_send_party_timeout = time + DORMANT_NEXT_PARTY_UPDATE;
    i++;
  }

  if(i > 0)
    entity_update.transmit();
}