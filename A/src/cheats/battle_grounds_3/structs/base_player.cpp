#include "../link.h"

c_player_data* c_base_player::data(){
  return utils::get_player_data(get_index());
}

ALWAYSINLINE c_base_weapon* c_base_player::get_weapon(){
  return (c_base_weapon*)global->entity_list->get_entity_handle(weapon_handle);
}

ALWAYSINLINE bool c_base_player::can_fire(){
  if(!is_player())
    return false;

  c_base_weapon* wep = get_weapon();
  if(wep == nullptr)
    return false;

  if(wep->clip1 == 0 && !wep->is_melee())
    return false;

  return wep->next_primary_attack <= globaldata->cur_time;
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

  return global->current_cmd->buttons & IN_ATTACK;
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