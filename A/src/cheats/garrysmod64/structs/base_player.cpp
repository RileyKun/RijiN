#include "../link.h"

c_player_data* c_base_player::data(){
  return utils::get_player_data(get_index());
}

ALWAYSINLINE c_base_weapon* c_base_player::get_weapon(){
  return (c_base_weapon*)global->entity_list->get_entity_handle( weapon_handle );
}

ALWAYSINLINE bool c_base_player::is_can_fire_supported(){
  if(!is_player())
    return false;

  c_base_weapon* wep = get_weapon();
  if(wep == nullptr)
    return false;


  u32 script_type = wep->scripted_type();
  if(script_type == weapon_type_fas || script_type == weapon_type_cw)
    return false;

  return true;
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
  if(this->water_level >= 2)
    return false;

  // TODO reload_mode check here
  if(wep->clip1 == 0 && !wep->is_melee())
    return false;

  if(wep->is_rpg_guiding_missile())
    return false;

  if(!is_can_fire_supported())
    return true;
  
  return wep->next_primary_attack <= globaldata->cur_time;
}

ALWAYSINLINE bool c_base_player::will_fire_this_tick(){
  if(global->current_cmd == nullptr)
    return false;

  c_base_weapon* wep = get_weapon();
  if(wep == nullptr)
    return false;

  //if(wep->is_melee())
  //  return wep->smack_time > 0.f && globaldata->cur_time > wep->smack_time;

  if(!can_fire())
    return false;

  return global->current_cmd->buttons & IN_ATTACK;
}

bool c_base_player::is_perpheads_police(){
  if(!is_player())
    return false;

  c_player_data* data = this->data();
  if(data == nullptr)
    return false;

  return data->gmod_data.is_perpheads_police;
}

bool c_base_player::is_perpheads(){
  if(!is_player())
    return false;

  c_player_data* data = this->data();
  if(data == nullptr)
    return false;

  return data->gmod_data.is_perpheads;
}

bool c_base_player::is_perpheads_medic(){
  if(!is_player())
    return false;

  return team == 4/*TEAM_MEDIC*/;
}

bool c_base_player::is_god_moded(){
  if(!is_player())
    return false;

  c_player_data* data = this->data();
  if(data == nullptr)
    return false;

  return this->entity_flags & FL_GODMODE || data->gmod_data.has_god_mode;
}

bool c_base_player::is_considered_enemy(c_base_player* target){
  if(target == nullptr || target == this)
    return false;

  if(target->is_player()){
    c_base_player* target_player = (c_base_player*)target;
    c_player_data* data = target_player->data();
    if(data == nullptr)
      return false;

    // Don't target other perpheads officers if we are one ourselves
    if(config->misc.custom_server_integrations && is_perpheads_police() && (target->is_perpheads_police() || target->is_perpheads_medic()))
      return false;

    if(data->playerlist.ignore_player)
      return false;

    if(config->aimbot.ignore_team_mates && this->team == target->team)
      return false;

    if(config->aimbot.ignore_friends){
      c_base_player* me = (c_base_player*)this;
      if(config->aimbot.ignore_friends && me->is_steam_friend((c_base_player*)target))
        return false;
    }

    if(config->aimbot.ignore_demi_god && target_player->is_demi_god_mode())
      return false;

    if(config->aimbot.ignore_noclipping && target_player->move_type == MOVETYPE_NOCLIP)
      return false;

    if(config->aimbot.ignore_god_mode && target_player->is_god_moded())
      return false;

    if(config->aimbot.ignore_staff && target_player->is_staff())
      return false;

    if(config->aimbot.ignore_vehicle && target_player->player_vehicle > 0)
      return false;
  }

  return true;
}

bool c_base_player::is_staff(){
  if(!is_player())
    return false;

  c_player_data* data = this->data();
  if(data == nullptr)
    return false;

  switch(data->gmod_data.user_group_hash){
    default: return false;
    case HASH("admin"):
    case HASH("superadmin"):
    case HASH("moderator"):
    case HASH("owner"):
    case HASH("coowner"):
    case HASH("developer"):
    case HASH("headadmin"):
    case HASH("trialadmin"):
    case HASH("manager"):
      return true;
  }
  return false;
}

bool c_base_player::is_vip(){
  if(!is_player())
    return false;

  c_player_data* data = this->data();
  if(data == nullptr)
    return false;

  switch(data->gmod_data.user_group_hash){
    default: return false;
    case HASH("vip"):
    case HASH("donator"):
      return true;
  }
  return false;
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

bool c_base_player::is_holding_semi_auto(){
  c_player_data* data = this->data();
  if(data == nullptr)
    return false;

  return data->gmod_data.is_semi_auto;
}