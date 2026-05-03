#include "../link.h"

c_player_data* c_base_player::data(){
  return utils::get_player_data(get_index());
}

ALWAYSINLINE c_base_weapon* c_base_player::get_weapon(bool second, bool dont_fall_over){
  c_base_weapon* wep = (c_base_weapon*)global->entity_list->get_entity_handle(weapon_handle);
  if(wep == nullptr && !dont_fall_over || second)
    wep = (c_base_weapon*)global->entity_list->get_entity_handle(weapon_handle2);

  return wep;
}

ALWAYSINLINE bool c_base_player::can_fire(){
  if(!is_player())
    return false;

  c_base_weapon* wlist[2] = {get_weapon(false, true), get_weapon(true)};

  for(i32 i = 0; i < 2; i++){
    c_base_weapon* wep = wlist[i];
    if(wep == nullptr)
      continue;

    // TODO reload_mode check here
    if(wep->clip1 == 0 && !wep->is_melee())
      continue;

    if(wep->next_primary_attack <= globaldata->cur_time)
      return true;
  }

  return false;
}

ALWAYSINLINE bool c_base_player::will_fire_this_tick(){
  if(global->current_cmd == nullptr){
    DBG("[-] current_cmd is nullptr\n");
    return false;
  }

  c_base_weapon* wep = get_weapon();
  if(wep == nullptr)
    return false;

  //if(wep->is_melee())
  //  return wep->smack_time > 0.f && globaldata->cur_time > wep->smack_time;

  if(!can_fire())
    return false;

  return global->current_cmd->buttons & IN_ATTACK || has_dual_weapons() && global->current_cmd->buttons & IN_ATTACK2;
}

ALWAYSINLINE bool c_base_player::has_dual_weapons(){
  c_base_weapon* wep1 = get_weapon(false, true);
  c_base_weapon* wep2 = get_weapon(true);
  return wep1 != nullptr && wep2 != nullptr && wep1 != wep2;
}

ALWAYSINLINE bool c_base_player::is_steam_friend(c_base_player* player){
  if(player == nullptr)
    return false;

  if(!player->is_player())
    return false;

  c_player_data* data = player->data();
  if(data == nullptr)
    return false;

  return data->steam_friend;
}