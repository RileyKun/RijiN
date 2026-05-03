#pragma once

class c_engine_prediction : public c_base_engine_prediction{
public:

  void on_pre_prediction(c_user_cmd* cmd) override{
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    c_base_engine_prediction::on_pre_prediction(cmd);

    if(is_frozen())
      clear_movement(cmd, !localplayer->has_condition(TF_COND_TAUNTING));
  }

  void on_post_prediction(c_user_cmd* cmd) override{
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return;

    c_base_engine_prediction::on_post_prediction(cmd);

    // store the predicted time between shots here
    // we need to do it after engine prediction, or we will get weird results   
    global->time_between_shots = wep->time_between_shots();
  }

  bool is_frozen() override{
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return false;

    return localplayer->has_condition(TF_COND_TAUNTING) && !localplayer->can_move_during_taunt() || c_base_engine_prediction::is_frozen();
  }

  // TF2 issue fixes:
  void fix_ground_flags(){
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    c_player_data* data = localplayer->data();
    if(data == nullptr)
      return;

    if(localplayer->entity_flags() & FL_ONGROUND)
      return;

    bool on_func_track = false;
    if(data->tfdata.ground_entity_index != -1){
      c_base_entity* ground_entity = global->entity_list->get_entity(data->tfdata.ground_entity_index);
      if(ground_entity != nullptr)
        on_func_track = ground_entity->is_on_func_track();
    }

    // Issue #1: The game knows what the ground entity is, but the entity_flags doesn't have the ground_entity set.
    // Issue #2: Our localplayer data reset either from just spawning in the first time or a fullupdate.
    // We'll be on the ground but our client won't run any code to detect if we're on the ground. So ground entity will be null and entity_flags won't have FL_ONGROUND set
    if(localplayer->ground_entity() != -1 || on_func_track || !localplayer->is_player_class(TF_CLASS_SCOUT) && data->tfdata.ground_entity_index != -1)
      localplayer->entity_flags() |= FL_ONGROUND;
  }
};

CLASS_EXTERN(c_engine_prediction, engine_prediction);