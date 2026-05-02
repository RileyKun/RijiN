#pragma once

#define DORMANT_EXPIRE_TIME 1.0f
#define DORMANT_NEXT_PARTY_UPDATE 0.2f
class c_dormant_manager{
public:

  void on_dormant(c_base_entity* entity, bool dormant){
    if(entity == nullptr)
      return;

    if(!entity->is_player())
      return;


    c_base_player* p = (c_base_player*)entity;

    c_player_data* data = p->data();
    if(data == nullptr)
      return;

    if(!dormant)
      data->dormant_timeout = -1.f;
    else
      data->dormant_timeout = math::time() + DORMANT_EXPIRE_TIME;
  }

  void on_death(i32 attacked){
    c_player_data* data = utils::get_player_data(attacked);
    if(data == nullptr)
      return;

    data->dormant_timeout                = -1.f;
    data->dormant_death_timeout          = math::time() + 1.f;
  }

  void on_spawned(i32 index){
    c_player_data* data = utils::get_player_data(index);
    if(data == nullptr)
      return;

    data->dormant_timeout                = -1.f;
    data->dormant_death_timeout          = -1.f;
  }

  void update_player(i32 index, vec3 pos){
    if(index <= 0 || index > 128)
      return;

    // Never update local player data.
    if(index == global->engine->get_local_player())
      return;

    c_base_player* player = global->entity_list->get_entity(index);
    if(player == nullptr)
      return;

    if(!player->is_player())
      return;

    if(!player->is_dormant())
      return;

    c_player_data* data = player->data();
    if(data == nullptr)
      return;

    //c_player_resource* pr = utils::get_player_resource();
    //if(pr != nullptr){

      //player->health        = pr->get_health(index);
      //player->player_class  = pr->get_player_class(index);
      //player->team          = pr->get_team(index);


      player->set_abs_origin(pos);
      data->dormant_timeout = math::time() + DORMANT_EXPIRE_TIME;
    //}
  }

  void process_entity_sounds(start_sound_params params){
    if(!utils::is_in_game())
      return;

    if(!params.from_server) //
      return;

    if(params.sound_source <= 0)
      return;

    c_base_entity* entity = global->entity_list->get_entity(params.sound_source);
    if(entity == nullptr)
      return;

    if(!entity->is_dormant())
      return;

    if(!entity->is_player())
      return;

    c_base_player* p = (c_base_player*)entity;

    float time = math::time();

    c_player_data* data = p->data();
    if(data == nullptr)
      return;

    if(data->dormant_death_timeout > time)
      return;

    update_player(entity->get_index(), params.pos);

  }
};

CLASS_EXTERN(c_dormant_manager, dormant_manager);