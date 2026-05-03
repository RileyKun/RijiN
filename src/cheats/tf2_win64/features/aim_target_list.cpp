#include "../link.h"

c_target_list* target_list = nullptr;

// A list of entities we're looking to aimbot.
bool c_target_list::is_valid_target_entity(i32 index){
  if(index == 0 || index == global->localplayer_index)
    return false;

  if(global->aimbot_settings == nullptr)
    return false;

  c_base_entity* entity = global->entity_list->get_entity(index);
  if(entity == nullptr)
    return false;

  if(entity->is_dormant())
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(weapon->is_projectile_weapon())
    return projectile_aimbot->should_target_entity(entity);
  else if(weapon->is_hitscan_weapon())
    return raytrace_aimbot->should_target_entity(entity);
  else if(weapon->is_melee()){
    if(localplayer->origin().distance(entity->origin()) > 256.f)
      return false;

    return melee_aimbot->should_target_entity(entity);
  }
  
  if(entity->is_player()){
    if(entity->team() == global->localplayer_team && !global->aimbot_settings->target_friendlies)
      return false;

    return true;
  }

  // If you're using this system for aiming at your own team's stuff then change the logic properly here.
  if(entity->is_sentry() || entity->is_dispenser() || entity->is_teleporter())
    return entity->team() != global->localplayer_team;

  if(entity->is_boss())
    return true;

  if(entity->is_stickybomb() && entity->is_sticky_onsurface()){
    if(entity->team() != global->localplayer_team)
      return true;
  }

  return false;
}

void c_target_list::compute_active_spectators(){
  _is_being_spectated = false;
  spectator_count     = 0;
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  for(i32 i = 1; i <= globaldata->max_clients; i++){
    c_base_player* player = global->entity_list->get_entity(i);
    if(player == nullptr)
      continue; 

    if(player->observer_target() <= 0 || player->observer_mode() != OBS_MODE_IN_EYE)
      continue;

    c_base_player* spectator_target = global->entity_list->get_entity_handle(player->observer_target());
    if(spectator_target == nullptr)
      continue;

    if(spectator_target->get_index() != global->localplayer_index)
      continue;

    _is_being_spectated = true;
    spectator_count++;
  }
}

i32 c_target_list::get_loop_end_index(){
  if(is_sorted_table_valid()){
    if(global->aimbot_settings != nullptr){
      if(global->aimbot_settings->max_targets_enabled){
        if(!global->aimbot_settings->max_targets_auto)
          return math::smallest(game_table_count - 1, math::biggest(global->aimbot_settings->max_targets - 1, 0));
        else
          return math::smallest(game_table_count - 1, processor_core_count); // Literally just use the core count, but clamp it to 16
      }
    }

    return game_table_count - 1;
  }

  return global->entity_list->get_highest_index();
}

bool c_target_list::is_local_ready(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  return localplayer->is_alive() && !localplayer->is_dormant();
}

i32 c_target_list::get_wanted_list_type(){
  if(global->aimbot_settings != nullptr){
    if(global->aimbot_settings->target_method_distance)
      return TARGET_LIST_SORT_DIST;
  }

  return TARGET_LIST_SORT_FOV;
}

// As in, if we aimbot will it actually run with this weapon?
bool c_target_list::is_weapon_supported(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  return weapon->is_projectile_weapon() || weapon->is_hitscan_weapon() || weapon->is_melee();
}


bool c_target_list::on_collection_iteration(i32 index){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_entity* entity = global->entity_list->get_entity(index);
  if(entity == nullptr)
    return false;

  if(entity->is_dormant())
    return false;

  float priority = 0.f;
  if(entity->is_player()){
    c_base_player* player = entity->get_player();
    if(player != nullptr && !player->is_alive())
      return;
  
    c_player_data* data = utils::get_player_data(index);
    if(data != nullptr){
      if(data->playerlist.ignore_player)
        return;

      if(data->playerlist.priority_mode && data->playerlist.priority > 0)
        priority = data->playerlist.priority;
    }
  }

  const float dist = localplayer->shoot_pos().distance(entity->origin());
  const float fov  = utils::get_ideal_pixel_fov(entity);

  write_to_thread_section(index, fov, dist, priority);
  return true;
}