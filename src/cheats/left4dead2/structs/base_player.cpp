#include "../link.h"

ALWAYSINLINE c_base_weapon* c_base_player::get_weapon(){
  return (c_base_weapon*)global->entity_list->get_entity_handle(weapon_handle());
}

// Need to handle melee swings.
ALWAYSINLINE bool c_base_player::can_fire(){
  c_base_weapon* wep = get_weapon();
  if(wep == nullptr)
    return false;

  if(wep->clip1() == 0 && !wep->is_melee())
    return false;

  return global->speedhack_active || wep->next_primary_attack() <= globaldata->cur_time;
}

ALWAYSINLINE bool c_base_player::can_shove(){
  return next_shove_time() == 0.f || next_shove_time() <= globaldata->cur_time;
}

ALWAYSINLINE float c_base_player::last_shove_delta(){
  if(last_shove_time() == 0.f)
    return 0.f;

  return math::abs(globaldata->cur_time - last_shove_time());
}

ALWAYSINLINE bool c_base_player::will_fire_this_tick(){
  if(global->current_cmd == nullptr)
    return false;

  c_base_weapon* wep = get_weapon();
  if(wep == nullptr)
    return false;

  /*
  if(wep->is_melee())
    return wep->smack_time > 0.f && globaldata->cur_time > wep->smack_time;
  */

  if(!can_fire())
    return false;

  return global->current_cmd->buttons & IN_ATTACK;
}

ALWAYSINLINE bool c_base_player::is_steam_friend(c_base_player* player){
  if(player == nullptr)
    return false;

  c_entity_data* data = player->data();
  if(data == nullptr)
    return false;

  return data->steam_friend;
}