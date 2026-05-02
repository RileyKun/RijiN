#include "../link.h"

CLASS_ALLOC(c_auto_heal_arrow, auto_heal_arrow);

// Get target via heal target.
void c_auto_heal_arrow::find_target(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr){
    invalidate_target();
    return;
  }

  if(!localplayer->is_player_class(TF_CLASS_MEDIC)){
    invalidate_target();
    return;
  }

  c_base_weapon* primary_wep = localplayer->get_weapon_from_belt(0, false);
  if(primary_wep == nullptr){
    invalidate_target();
    return;
  }

  if(!primary_wep->is_crossbow()){
    invalidate_target();
    return;
  }

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr){
    invalidate_target();
    return;
  }

  if(wep->is_crossbow())  
    return; // Don't call invalidate_target, and no we can't check our heal target on our medigun because the crossbow is our active weapon.

  // No need to check if we don't have the crossbow equipped because there's a check above for that.
  // We just wanna avoid swapping from melee to the crossbow to heal a team-mate.
  if(wep->is_melee()){
    invalidate_target();
    return;
  }
  
  c_base_entity* heal_target = wep->get_heal_target();
  if(heal_target == nullptr){
    invalidate_target();
    return;
  }

  if(!heal_target->is_player()){
    invalidate_target();
    return;
  }

  if(heal_target->team() != global->localplayer_team){
    invalidate_target();
    return;
  }

  // We predict 3 seconds worth of good healing.
  // If the value is equal or greater than max health then don't consider this player valid.
  if(heal_target->health() + ASSUMED_PREDICTED_HEALTH >= heal_target->max_health()){
    invalidate_target();
    return;
  }

  target_index_lock = heal_target->get_index();
}

bool c_auto_heal_arrow::has_valid_target(){
  c_aimbot_settings* aimbot_settings = get_proj_aim_settings();
  if(aimbot_settings == nullptr)
    return false;

  if(target_index_lock <= 0)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_player_class(TF_CLASS_MEDIC))
    return false;

  c_base_player* player = global->entity_list->get_entity(target_index_lock);
  if(player == nullptr)
    return false;

  if(!player->is_player())
    return false;

  if(!player->is_valid()){
    cancel_automation();
    return false;
  }

  c_player_data* data = player->data();
  if(data == nullptr)
    return false;

  if(data->playerlist.ignore_player || data->playerlist.ignore_auto_heal)
    return false;

  // Crossbow can't shoot through grates.
  s_trace tr = global->trace->ray(localplayer->shoot_pos(), player->obb_center(), mask_solid, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NONE);
  if(!tr.vis(player))
    return false;
  
  // We can't heal arrow ubercharged players.
  if(player->is_ubercharged()){
    cancel_automation();
    return false;
  }

  float hp_ratio = (player->health()/ player->max_health()) * 100.f;
  if(hp_ratio > 60.f){
    cancel_automation();
    return false;
  }

  if(aimbot_settings->heal_arrow_friendsonly && !player->is_steam_friend())
    return false;
  
  if(config->automation.automatic_heal_arrow_friends_only && !player->is_steam_friend())
    return false;
  
  // Are they with in the heal arrow FOV Limit?
  float fov = math::get_fov_delta(global->untouched_cmd.view_angles, localplayer->shoot_pos(), player->obb_center());
  if(aimbot_settings->heal_arrow_max_fov > 0.f && aimbot_settings->heal_arrow_max_fov < 180.f){
    if(fov > aimbot_settings->heal_arrow_max_fov){
      cancel_automation();
      return false;
    }
  }

  return true;  
}

bool c_auto_heal_arrow::force_enable_heal_arrow(){
  if(!config->automation.automatic_heal_arrow)
    return false;

  if(!has_valid_target())
    return false;

  if(!should_run())
    return false;

  return next_allow_swap_time > math::time();
}

bool c_auto_heal_arrow::should_run(){
  if(!config->automation.automatic_heal_arrow)
    return false;

  c_aimbot_settings* aimbot_settings = get_proj_aim_settings();
  if(aimbot_settings == nullptr)
    return false;

  if(!aimbot_settings->heal_arrow_enabled || !aimbot_settings->enabled)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_player_class(TF_CLASS_MEDIC))
    return false;

  if(!localplayer->is_valid())
    return false;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return false;

  if(!utils::is_playing_mvm())
    return true;

  if(!wep->is_medigun())
    return true;

  // We have a shield active!
  if(localplayer->rage_draining())
    return false;

  if(!wep->is_vaccinator()){
    if(wep->charge_released())
      return false;
  }

  return true;
}

// This function should always return array safe values.
i32 c_auto_heal_arrow::get_resist_type_from_cond(u32 condition){
  switch(condition){
    default: break;
    case TF_COND_MEDIGUN_UBER_BULLET_RESIST: return 0;
    case TF_COND_MEDIGUN_UBER_BLAST_RESIST: return 1;
    case TF_COND_MEDIGUN_UBER_FIRE_RESIST: return 2;
  }

  return 0;
}

// Vaccinator logic functions.
void c_auto_heal_arrow::knowledge_vaccinator_charge_released(c_base_player* player, u32 condition){
  if(player == nullptr)
    return;

  if(player->get_index() != global->localplayer_index)
    return;

  i32 type = get_resist_type_from_cond(condition);
  last_vaccinator_uber_charge_time[type] = math::time();
}

bool c_auto_heal_arrow::is_handling_vaccinator_logic(c_base_player* localplayer, c_base_weapon* crossbow, c_base_weapon* wep){
  if(crossbow == nullptr)
    return false;

  if(localplayer == nullptr)
    return false;

  c_base_player* target = global->entity_list->get_entity(target_index_lock);
  if(target == nullptr)
    return false;

  if(!wep->is_vaccinator())
    return true;

  float charge_level = wep->uber_charge_level();

  // We can't pop a charge with this low of a level!
  if(charge_level < 0.25f)
    return true;

  s_auto_vaccinator_data av_data;
  memset(&av_data, 0, sizeof(s_auto_vaccinator_data));

  av_data.bullet = 1;
  av_data.blast  = 1;
  av_data.fire   = 1;

  // We're not trying to rebuild the vaccinator entirely in here. 
  for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* entity = global->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    if(entity->is_dormant())
      continue;

    if(entity->is_player()){
      auto_vaccinator->handle_player(localplayer, entity, &av_data);
      auto_vaccinator->handle_player(target, entity, &av_data);
    }
    else{
      auto_vaccinator->handle_entity(localplayer, entity, &av_data);
      auto_vaccinator->handle_entity(target, entity, &av_data);
    }
  }

  bool has_manual_charge_enabled = config->automation.auto_vaccinator_manualcharge_resist_cycle_option && !config->automation.auto_vaccinator_manualcharge_resist_none;

  i32  resist = -1;
  if(config->automation.auto_vaccinator){
    if(av_data.bullet > av_data.blast && av_data.bullet > av_data.fire)
      resist = 0;
    else if(av_data.blast > av_data.bullet && av_data.blast > av_data.fire)
      resist = 1;
    else if(av_data.fire > av_data.bullet && av_data.fire > av_data.blast)
      resist = 2;

    // Lets make sure manual charges are supported.
    if(global->current_cmd->buttons & IN_ATTACK2 && has_manual_charge_enabled){
      if(config->automation.auto_vaccinator_manualcharge_resist_bullet)
        resist = 0;
      else if(config->automation.auto_vaccinator_manualcharge_resist_blast)
        resist = 1;
      else if(config->automation.auto_vaccinator_manualcharge_resist_fire)
        resist = 2;
    }
  }
  else
    resist = wep->charge_resist_type();

  if(resist == -1)
    return true;

  // If we got no charges deployed. Lets wait until the highest threat near us is deployed before swapping.
  if(!target->is_bullet_charge(true) && !target->is_blast_charge(true) && !target->is_fire_charge(true)){
    if(resist == 0 && !target->is_bullet_charge(true))
      return false;
  
    if(resist == 1 && !target->is_blast_charge(true))
      return false;
  
    if(resist == 2 && !target->is_fire_charge(true))
      return false;
  }

  // If the auto vaccinator isn't enabled then just continue on.
  if(!config->automation.auto_vaccinator)
    return true;

  if(av_data.bullet >= AUTO_VACCINATOR_MAX2CHARGE && !target->is_bullet_charge(true))
    return false;

  if(av_data.blast >= AUTO_VACCINATOR_MAX2CHARGE && !target->is_blast_charge(true))
    return false;

  if(av_data.fire >= AUTO_VACCINATOR_MAX2CHARGE && !target->is_fire_charge(true))
    return false;

  return true;
}

// Generic functions.
void c_auto_heal_arrow::run(){
  if(!should_run())
    return;

  find_target();
  if(!has_valid_target())
    return;
  
  c_base_player* target = global->entity_list->get_entity(target_index_lock);
  if(target == nullptr)
    return;

  DBG("[!] auto_heal_arrow: has valid target: %i\n", target_index_lock);
  float time = math::time();
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  c_base_weapon* wep = localplayer->get_weapon();
  if((wep->is_medigun() || wep->is_quickfix()) && !wep->is_vaccinator() && !wep->is_kritzkrieg()){
    if(wep->charge_released()){
      DBG("[!] auto_heal_arrow: Releasing an uber-charge, either cant take damage or have a very fast heal rate.\n");
      return;
    }
  }

  c_base_weapon* crossbow = localplayer->get_weapon_from_belt(0, false);
  if(crossbow == nullptr)
    return;
  
  if(!crossbow->is_crossbow())
    return;
  
  c_base_weapon* medigun = localplayer->get_weapon_from_belt(1, false);
  if(medigun == nullptr)
    return; 
  
  if(!medigun->is_medigun())
    return;
  
  if(medigun->is_vaccinator()){
    if(!is_handling_vaccinator_logic(localplayer, crossbow, medigun))
      return;
  }

  if(next_allow_swap_time > time)
    return;
  
  DBG("[!] Swapping to crossbow!\n");
  auto_weapon_swap->swap_and_return_to(0, true, 1.f);

  next_allow_swap_time = time + 2.f;
}

void c_auto_heal_arrow::cancel_automation(){
  auto_weapon_swap->force_return();
}

void c_auto_heal_arrow::on_death(i32 attacker, i32 victim){
  if(victim != target_index_lock)
    return;

  cancel_automation();
}