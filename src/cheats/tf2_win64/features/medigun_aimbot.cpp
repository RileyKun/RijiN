#include "../link.h"

CLASS_ALLOC(c_medigun_aimbot, medigun_aimbot);

void c_medigun_aimbot::set_aimbot_filter(){
  if(global->aimbot_settings == nullptr)
    return;

  destroy_aimbot_filter();
  filter = new c_aimbot_filter_medigun;

  heal_type = 1;
  if(utils::is_in_setup()){
    heal_type = 2;
    return;
  }

  // This check isn't supposed to be 100% accurate to the nail. It's supposed to be very easy on resources.
  // A quick check to see how many players are below max health near us.
  c_base_player* localplayer = utils::localplayer();
  for(u32 i = 1; i <= globaldata->max_clients; i++){
    c_base_player* player = global->entity_list->get_entity(i);
    if(player == nullptr)
      continue;

    if(!player->is_valid())
      continue;

    if(player == localplayer)
      continue;

    if(player->team() != localplayer->team())
      continue;

    float dist = localplayer->origin().distance(player->origin());
    if(dist > 900.f)
      continue;

    if(player->health()< player->max_health()){
      heal_type = 0;
      break;
    }
  }
}

i32 c_medigun_aimbot::get_priorty(c_base_entity* entity){
  if(entity == nullptr)
    return 0;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return 0;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return 0;

  c_base_player* player = entity->get_player();
  if(player != nullptr){
    i32 priority   = 0;
    i32 health     = player->get_health();
    i32 max_health = player->max_health();

    // The player can't be damaged.
    if(player->is_ubercharged() || player->is_ubercharge_hidden())
      return 0;

    // We need this here to ensure prefer friends doesn't break.
    if(global->aimbot_settings->auto_medigun_preferfriends || (global->aimbot_settings->auto_medigun_preferfriends_except_on_key && !input_system->held(global->aimbot_settings->auto_medigun_preferfriends_except_on_key_hotkey))){
      if(player->is_steam_friend())
        priority += 100;
    }

    // If the health is equal or greater than this then the charge rate will be affected by 50%
    if(health < player->get_max_buffed_health() * 0.95f)
      priority++;

    // Heal-rate will be affected if their heal is greater than max health.
    if(health < max_health){
      priority++;
      if(wep->is_vaccinator())
        priority++;
    }

    max_health *= 0.25f;
    while(max_health > 1 && health < max_health){
      priority++;
      max_health *= 0.25f;
    }

    // Healers reduce the charge rate.
    i32 healers = player->healers();
    if(healers > 0){

      // We'll increase the healers value also. Lets take that into account.
      c_base_player* heal_target = wep->get_heal_target();
      if(heal_target != nullptr && heal_target == player)
        healers--;

      priority -= healers;
    }

    return priority;
  }
  else if(entity->is_revive_marker()){
    return 10;
  }

  return 0;
}

// !! weapon is our weapon not the target's weapon. -Rud
float c_medigun_aimbot::get_next_target_delay(c_base_player* target, c_base_weapon* weapon){
  if(target == nullptr || weapon == nullptr)
    return 0.f;

  c_player_data* data = target->data();
  if(data == nullptr)
    return 0.f;

  float delay = global->aimbot_settings->silent_aim_serverside ? 0.f : math::ticks_to_time(2);
  if(weapon->is_vaccinator())
    delay += 0.30f;
  else{
    if(global->aimbot_settings->enable_uber_sharing){
      if(weapon->charge_released()){
        return math::clamp(global->aimbot_settings->uber_sharing_time, 0.f, 10.f);
      }
    }
  }

  const float user_input_delay = math::clamp(global->aimbot_settings->auto_heal_next_target_delay, 0.0f, 5.f);
  if(user_input_delay > 0.0f)
    delay += user_input_delay;

  delay = math::ticks_to_time(math::time_to_ticks(delay));
  if(heal_type == 2)
    return delay;

  // The reasons why only do this with 2 healers is once we heal the player the counter will go up by 1.
  // Also, if they get healed by dispensers or payload carts it'll go up again.
  // Having 2 medic's won't increase heal rate. It'll be averaged out between both of them.
  if(target->healers() <= 2 && heal_type == 0){
    // This is a point where the target could possibly die.
    if(target->is_vulnerable())
      delay += 0.5f;

    float new_delay  = 1.5f;
    float hp_percent = target->get_health_percent();
    while(hp_percent > 0.f && new_delay > 0){
      new_delay  -= 0.15f;
      hp_percent -= 10.f;
    }

    delay += new_delay;
  }

  delay = math::ticks_to_time(math::time_to_ticks(delay));
  return delay;
}

bool c_medigun_aimbot::should_target_while_charged(c_base_player* target){
  if(target == nullptr)
    return false;

  if(!target->is_player())
    return true;

  c_player_data* data = target->data();
  if(data == nullptr)
    return true;

  if(data->playerlist.ignore_auto_heal)
    return false;
  
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(!weapon->charge_released())
    return true;

  c_base_player* heal_target = weapon->get_heal_target();
  if(heal_target == nullptr) // Lets heal anyone at this point.
    return true;

  if(heal_target == target)
    return true;

  // We're not allowed to swap to new targets.
  if(!global->aimbot_settings->enable_uber_sharing)
    return false;

  // Ignore useless players. Even if they're our
  if(target->is_bonked() || target->is_taunting())
    return false;

  if(weapon->is_medigun()){
    if(target->is_ubercharged()) // They're being ubercharged by somebody else.
      return false;
  }
  else if(weapon->is_kritzkrieg()){
    if(target->is_crit_boosted()) // They're being crit-boosted by some(body/thing) else.
      return false;
  }
  else if(weapon->is_quickfix()){
    if(target->is_quickfix_uber()) // Somebody has already quickfix uber charged them.
      return false;
  }

  return true;
}

bool c_medigun_aimbot::get_best_aim_point(c_base_player* target, s_lc_record* lc_record, vec3& aim_point){
  if(target == nullptr  || global->aimbot_settings == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  return get_best_hitbox_point(2, localplayer->shoot_pos(), target, lc_record, aim_point);
}

bool c_medigun_aimbot::get_best_hitbox_point(i32 hitbox, vec3 shoot_pos, c_base_player* target, s_lc_record* lc_record, vec3& aim_point){
  if(target == nullptr)
    return false;

  if(target->is_player()){
    if(lc_record == nullptr)
      return false;

    if(!lc_record->get_hitbox_pos(target, hitbox, aim_point))
      return false;
  }
  else
    aim_point = target->obb_center(); // There isn't hitbox support for world entities yet.

  // Are they in our fov?
  if(global->aimbot_settings->max_fov > 0 && global->aimbot_settings->max_fov < 180)
    if(math::get_fov_delta(global->untouched_cmd.view_angles, shoot_pos, aim_point) > (float)global->aimbot_settings->max_fov)
      return false;

  s_trace tr = global->trace->ray(shoot_pos, aim_point, mask_shot & ~(0x40000000), TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_HIT_TEAM);

  #if defined(DEV_MODE) && defined(MEDIGUN_AIMBOT_DEBUG)
    render_debug->draw_3dbox(target->origin(), target->obb_mins(), target->obb_maxs(), vec3(0, 0, 0), colour(255, 100, 0, 0), colour(0, 0, 0, 255), globaldata->interval_per_tick * 2.f);
    i8 buf[IDEAL_MIN_BUF_SIZE];
    formatA(buf, "%i", get_priorty(target));
    render_debug->draw_text(target->origin(), colour(255, 255, 255, 255), buf, 0, globaldata->interval_per_tick * 2.f);
  #endif

  if(!tr.vis(target)){

    // Always assume the heal target is visible.
    c_base_player* localplayer = utils::localplayer();
    if(localplayer != nullptr){
      c_base_weapon* weapon = localplayer->get_weapon();
      if(weapon != nullptr){
        if(weapon->get_heal_target() == target)
          return true;
      }
    }

    return false;
  }

  return true;
}

// Enforce the target's tickcount.
bool c_medigun_aimbot::enforce_tickcount(c_base_player* target, s_lc_record* record){
  if(target == nullptr)
    return false;

  if(!target->is_player())
    return false;

  interp_manager->set_tick_count_with_interp(record != nullptr ? record->simulation_time : target->simulation_time(), target->get_index(), record != nullptr);
  return true;
}

bool c_medigun_aimbot::get_target(c_base_player*& target, s_lc_record*& lc_record, vec3& aim_point){
  if(global->aimbot_settings == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return false;

  c_base_player* heal_target = wep->get_heal_target();

  set_aimbot_filter();
  if(filter == nullptr)
    return false;

  float time = globaldata->cur_time;
  for(u32 i = 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_player* player = global->entity_list->get_entity(i);

    if(player == nullptr || !player->is_valid() || player == localplayer)
      continue;

    if(localplayer->team() != player->team())
      continue;

    if(!should_target_while_charged(player))
      continue;

    // There was a null heal target check here. But it seems that the heal target will become invalid when we swap targets for a tick and possibly cause a false positive.
    if(heal_target != nullptr){
      if((next_target_delay > time) && player != last_target)
        continue;
    }

    if(player->is_player()){
      s_lc_data* lc = lag_compensation->get_data(player);
      if(lc == nullptr)
        continue;

      // Don't target razer back snipers.
      if(player->health()>= player->max_health() && player->has_razer_back())
        continue;

      for(u32 ii = 0; ii < lc->max_records; ii++){
        s_lc_record* record = lc->get_record(ii);
        if(record == nullptr)
          continue;

        if(!record->valid() && player != heal_target)
          continue;

        // If we dont want to use history, then only use the first valid tick
        if(!acm->can_back_track() || !global->aimbot_settings->position_adjust_history)
          ii = lc->max_records;

        if(!filter->sanity(record, player))
          continue;

        if(!filter->handle(record, player))
          continue;

        if(filter->store(record, player))
          break;
      }
    }
    else{
      c_base_entity* entity = (c_base_entity*)player;
      if(!entity->is_revive_marker())
        continue;

      if(!filter->sanity(nullptr, player))
        continue;

      if(!filter->handle(nullptr, player))
        continue;

      if(filter->store(nullptr, player))
        continue;
    }
  }

  aim_point = filter->aim_point;
  target    = filter->target;
  lc_record = filter->lc_record;
  

  return target != nullptr;
}

bool c_medigun_aimbot::run(){
  if(global->aimbot_settings == nullptr)
    return false;

  // Unsure if the this would be an issue with a the IN_ATTACK check below, as the comment says.
  if(target_list->is_being_spectated() && global->aimbot_settings->disable_on_spectated)
    return false;

  // Disable auto-medigun when manually firing. But only if the aim mode isn't set to on attack.
  if(global->original_cmd.buttons & IN_ATTACK && !global->aimbot_settings->aim_mode_on_attack){
    if(last_target != nullptr){
      DBG("[!] Disabling auto-medigun\n");
      heal_target_maxhp  = 0;
      global->current_cmd->buttons &= ~IN_ATTACK;
      last_target = nullptr;
    }
    return false;
  }

  if(!global->aimbot_settings->enabled)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(localplayer->is_bonked() || localplayer->is_taunting())
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  bool can_medigun_attack = (weapon->next_primary_attack() > globaldata->cur_time);
  // Medigun isnt ready yet...
  //if(weapon->next_primary_attack() > globaldata->cur_time)
 //   return false;

  if(!weapon->is_medigun())
    return false;

  if(global->aimbot_settings->aim_mode_on_key && !input_system->held(global->aimbot_settings->aim_hotkey) || global->aimbot_settings->aim_mode_on_attack && !(global->current_cmd->buttons & IN_ATTACK))
    return false;

  vec3            aim_point;
  c_base_player*  target = nullptr;
  s_lc_record*    record = nullptr;
  if(!get_target(target, record, aim_point)){
    heal_target_maxhp = 0;
    return false;
  }

  enforce_tickcount(target, record);
  global->on_aimbot_target(target->get_index(), aim_point, weapon);


  float time                 = globaldata->cur_time;
  c_base_player* heal_target = weapon->get_heal_target();
  i32 maxhp                  = target->is_player() ? target->get_max_buffed_health() : 0;

  // Apply the overheal expert modifiers.
  float overheal_expert = weapon->get_overheal_expert_mod();
  if(overheal_expert > 0.f)
    maxhp += (target->max_health() * overheal_expert);

  if(heal_target != target)
    heal_target_maxhp = maxhp;


  if(heal_target != target || /* Reheal if targets change*/
      // Will reheal if the heal target pulls out the fists of steel as an example.
      maxhp > 0 && 
      last_target == target &&
      heal_target == target && 
      heal_target_maxhp > 0 &&
      maxhp < heal_target_maxhp &&
      math::abs(maxhp - heal_target_maxhp) > 5){

    if(!global->aimbot_settings->silent_aim_serverside){
      if(in_attack_spam++ % 3 == 0)
        global->current_cmd->buttons &= ~IN_ATTACK;
      else
        global->current_cmd->buttons |= IN_ATTACK;
    }
    else{
      if(!packet_manager->should_force_send_packet){
        if(in_attack_spam++ % 3 == 0)
          global->current_cmd->buttons &= ~IN_ATTACK;
        else
          global->current_cmd->buttons |= IN_ATTACK;
      }
      else
        global->current_cmd->buttons |= IN_ATTACK; 
    }
    
    // Seems to be an issue with serverside silent aimbot. 
    // If this happens just set a new target delay for 100ms. Seems to address the issue instantly.
    if(in_attack_spam > 20){
      DBG("[-] auto heal error: Switching/Reheal target %i | %i | %i %s\n", in_attack_spam, heal_target_maxhp, maxhp, in_attack_spam % 3 == 0 ? "BLOCKING ATTACK": "ALLOWING ATTACK");
      next_target_delay = time + 0.1f;
    }
  }
  else{
    global->current_cmd->buttons |= IN_ATTACK;
    in_attack_spam                = 0;

    if(target == heal_target){
      heal_target_maxhp = target->is_player() ? maxhp : 0;
      last_target       = target;
      return false;
    }
  }

  // If we aimbot with medigun on this tick we are going to leak our aimbot
  if(global->aimbot_settings->silent_aim_serverside && packet_manager->should_force_send_packet){
    if(heal_target != nullptr) // Prevent a case where we lose our heal target depending on the convar setting.
      global->current_cmd->buttons |= IN_ATTACK;

    return false;
  }

  // Target the player!
  {
    vec3 aim_angle = math::calc_view_angle(localplayer->shoot_pos(), aim_point);
    global->current_cmd->view_angles = aim_angle;

    if(!global->aimbot_settings->silent_aim_clientside && !global->aimbot_settings->silent_aim_serverside)
      global->engine->set_view_angles(aim_angle);

    if(global->aimbot_settings->silent_aim_serverside)
      packet_manager->set_choke(true);
  }

  if(target->is_player())
    heal_target_maxhp = maxhp;

  next_target_delay = time + get_next_target_delay(target, weapon);
  last_target       = target;

  #if defined(DEV_MODE) && defined(MEDIGUN_AIMBOT_DEBUG)
    float delay = math::ticks_to_time(1) + get_next_target_delay(target, weapon);
    render_debug->draw_line(localplayer->shoot_pos(), target->obb_center(), colour(0, 255, 0, 255), true, delay);

    i8 buf[IDEAL_MIN_BUF_SIZE];
    formatA(buf, "heal_type: %i - %i - %i", heal_type, target->get_health(), heal_target_maxhp);
    render_debug->draw_text(target->obb_center(), colour(0, 255, 0, 255), buf, 0, delay);
  #endif

  return true;
}