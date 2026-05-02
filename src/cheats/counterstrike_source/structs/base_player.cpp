#include "../link.h"

c_player_data* c_base_player::data(){
  return utils::get_player_data(get_index());
}

ALWAYSINLINE bool c_base_player::can_fire(){
  if(!is_player())
    return false;

  c_base_weapon* wep = get_weapon();
  if(wep == nullptr)
    return false;

  s_client_class* cc = wep->get_client_class();
  if(cc == nullptr)
    return false;

  // We can't fire under water.
  if(this->water_level() >= 2)
    return false;

  // TODO reload_mode check here
  if(wep->clip1() == 0 && !wep->is_melee() && !wep->is_grenade())
    return false;

  if(wep->get_burst_shots_remaining() > 0 && globaldata->cur_time >= wep->get_next_burst_shot_time())
    return true;

  return wep->next_primary_attack() <= globaldata->cur_time;
}

ALWAYSINLINE bool c_base_player::will_fire_this_tick(){
  if(global->current_cmd == nullptr)
    return false;

  c_base_weapon* wep = get_weapon();
  if(wep == nullptr)
    return false;

  if(wep->get_burst_shots_remaining() > 0 && globaldata->cur_time >= wep->get_next_burst_shot_time())
    return true;

  if(wep->is_melee()){
    if(wep->smack_time() > 0.f){
      if(globaldata->cur_time <= wep->smack_time())
        return false;

      return true;
    }

    return global->current_cmd->buttons & IN_ATTACK && wep->next_primary_attack() <= globaldata->cur_time || global->current_cmd->buttons & IN_ATTACK2 && wep->next_secondary_attack() <= globaldata->cur_time;
  }
  else if(wep->is_grenade()){
    return wep->throw_time() > 0.f && wep->throw_time() < globaldata->cur_time;
  }

  if(!can_fire())
    return false;

  return global->current_cmd->buttons & IN_ATTACK || global->current_cmd->buttons & IN_ATTACK2 && wep->is_melee();
}