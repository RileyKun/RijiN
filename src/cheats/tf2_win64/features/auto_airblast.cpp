#include "../link.h"

CLASS_ALLOC(c_auto_airblast, auto_airblast);


bool c_auto_airblast::run(){
  if(!config->automation.auto_reflect)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(!weapon->is_flamethrower() && !weapon->is_dragons_fury())
    return false;

  global->info_panel->add_entry(INFO_PANEL_AUTO_AIRBLAST, WXOR(L"INACTIVE"));
  if(config->automation.auto_reflect_use_key){
    if(!input_system->held(config->automation.auto_reflect_key))
      return false;
  }

  if(localplayer->is_ubercharged() && config->automation.auto_reflect_disable_ubered){
    global->info_panel->add_entry(INFO_PANEL_AUTO_AIRBLAST, WXOR(L"DISABLED ON UBER"), INFO_PANEL_RAGE_CLR);
    return false;
  }

  // Disable when 'randomly' critting.
  if(config->automation.auto_reflect_disable_forcing_crits && weapon->crit_time() >= globaldata->cur_time){
    global->info_panel->add_entry(INFO_PANEL_AUTO_AIRBLAST, WXOR(L"CRITTING"), INFO_PANEL_RAGE_CLR);
    return false;
  }

  if(weapon->weapon_id() == WPN_Phlogistinator || weapon->is_dragons_fury() && config->automation.auto_reflect_disable_dragonsfury){
    global->info_panel->add_entry(INFO_PANEL_AUTO_AIRBLAST, WXOR(L"INVALID WEAPON"), INFO_PANEL_RAGE_CLR);
    return false;
  }

  // Only attempt to auto airblast if we have enough ammo for it.
  i32 clip = localplayer->get_ammo_count(1);
  if(weapon->next_secondary_attack() > globaldata->cur_time || clip < 40 && weapon->weapon_id() == WPN_Backburner || clip < 20 || clip < config->automation.auto_reflect_ammo_required){
    global->info_panel->add_entry(INFO_PANEL_AUTO_AIRBLAST, WXOR(L"CANNOT REFLECT"), INFO_PANEL_RAGE_CLR);
    return false;
  }

  global->info_panel->add_entry(INFO_PANEL_AUTO_AIRBLAST, WXOR(L"RUNNING"));

  vec3           aim_point;
  i32            best_priority           = 0;
  i32            entity_type             = 0;
  float          best_distance           = 225.f; // Do not remove default value!!

  bool           target                  = false;
  bool           reflecting_player       = false;
  c_base_entity* target_entity           = nullptr;

  vec3 shoot_pos = localplayer->shoot_pos();
  for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* entity = global->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    if(i == global->localplayer_index)
      continue;

    if(entity->is_dormant())
      continue;

    if(entity->is_player()){
      c_base_player* p = (c_base_player*)entity;
      if(!p->is_alive())
        continue;
    }

    i32 priority = 0;
    if(!is_target_entity(entity, localplayer, weapon, priority))
      continue;

    vec3  pos = get_target_position(entity);
    float fov = math::get_fov_delta(global->untouched_cmd.view_angles, shoot_pos, pos);
    if(config->automation.auto_reflect_aimbot){
      if(config->automation.auto_reflect_fov > 0 && config->automation.auto_reflect_fov < 180){
        if(fov >= config->automation.auto_reflect_fov)
          continue;
      }
    }
    else{
      if(fov >= 30.f)
        continue;
    }

    s_trace tr = global->trace->ray(localplayer->shoot_pos(), pos, mask_bullet);
    if(!tr.vis(entity))
      continue;

    // As of now it's only used for prefering critical projectiles over non critical projectiles.
    {
      if(priority < best_priority)
        continue;

      best_priority = priority;
    }

    float distance = pos.distance(localplayer->obb_center());
    if(distance > best_distance)
      continue;

    aim_point               = math::calc_view_angle(localplayer->shoot_pos(), pos);
    reflecting_player       = entity->is_player();
    target_entity           = entity;
    best_distance           = distance;
    target                  = true;

    // What type of redirect entity do we have?
    {
      if(entity->can_destroy_projectiles())
        entity_type = REDIRECT_DESTROYS_PROJ;
      else if(entity->is_healing_bolt())
        entity_type = REDIRECT_HEALS_TEAM;
      else if(entity->is_arrow())
        entity_type = REDIRECT_HEADSHOTS;
    }
  }


  if(target){
    if(!reflecting_player){
      bool success  = false;
      i32  best_pos = get_best_redirect_position(target_entity);
      vec3 pos      = find_redirect_target(localplayer, aim_point, entity_type, best_pos, success);
      if(success){
        // If has a impact blast radius aim at the feet otherwise aim at the center of their box.
        aim_point = math::calc_view_angle(localplayer->shoot_pos(), pos);
        DBG("[!] Redirecting projectile towards entity.\n");
      }
    }

    global->current_cmd->buttons    |= IN_ATTACK2;
    if(config->automation.auto_reflect_aimbot){
      global->current_cmd->view_angles = aim_point;
      packet_manager->set_choke(true);
    }

    return true;
  }

  return false;
}

bool c_auto_airblast::is_target_entity(c_base_entity* entity, c_base_player* localplayer, c_base_weapon* weapon, i32& priority){
  if(entity == nullptr || localplayer == nullptr || weapon == nullptr)
    return false;

  i32 distance = entity->origin().distance_meter(localplayer->origin());
  if(entity->is_player()){

      c_base_player* p = (c_base_player*)entity;
      if(p->team() == localplayer->team()){

        // We're looking to reflect burning players.
        if(!p->is_on_fire())
          return false;

        if(config->automation.auto_reflect_burning_players)
          return distance <= 4;
        else if(config->automation.auto_reflect_burning_players_ifhurt){

          // Reflect other players if their health is less than 50%. OR if our health is less than 30%
          if(p->get_health_percent() <= 50.f || localplayer->get_health_percent() <= 30.f)
            return distance <= 4;
        }
      }
      else if(p->team() != localplayer->team()){

        if(distance > 1){
          s_lc_data* lc_data = lag_compensation->get_data(p);

          if(lc_data != nullptr){
            s_lc_record* record1 = lc_data->get_record(0);
            s_lc_record* record2 = lc_data->get_record(1);

            if(record1 != nullptr && record2 != nullptr){
              vec3 p1 = localplayer->origin();

              float d1 = math::abs((p1 - record1->origin).length_2d());
              float d2 = math::abs((p1 - record2->origin).length_2d());

              if(d1 >= d2)
                return false;
            }
          }
        }

        if(config->automation.auto_reflect_enemy_players)
          return distance <= 4;
        else if(config->automation.auto_reflect_enemy_players_meleeonly){

          if(p->is_cloaked() || p->is_bonked() || p->is_quickfix_uber())
            return false;

          c_base_weapon* weapon = p->get_weapon();
          if(weapon != nullptr){
            if(!weapon->is_melee())
              return false;
          }

          return distance <= 4;
        }
      }

      return false;
  }
  else if(entity->is_projectile_deflectable()){
      if(entity->team() == localplayer->team())
        return false;

      if(entity->is_fake_projectile())
        return false;

      if(entity->is_projectile_critical())
        priority += 2;

      if(entity->is_rocket() && config->automation.auto_reflect_target_rockets)
        return true;
      else if(entity->is_flare() && config->automation.auto_reflect_target_flares)
        return true;
      else if(entity->is_demo_projectile() && !entity->is_stickybomb() && config->automation.auto_reflect_target_pills)
        return true;
      else if(entity->is_stickybomb() && config->automation.auto_reflect_target_stickies)
        return !entity->is_sticky_onsurface();
      else if(entity->is_arrow()){
        if(entity->arrow_type() == 18)
          return config->automation.auto_reflect_target_mechbolts;

        if(entity->is_healing_bolt())
          return config->automation.auto_reflect_heal_bolts;

        return config->automation.auto_reflect_target_arrows;
      }
      else if(entity->is_throwables() && config->automation.auto_reflect_target_throwables)
        return true;
      else if(entity->is_flame_ball() && config->automation.auto_reflect_target_flameball)
        return true;
      else if(entity->can_destroy_projectiles() && config->automation.auto_reflect_target_mechorb)
        return true;

      return false;
  }

  return false;
}

c_base_entity* c_auto_airblast::find_redirect_target_healing(c_base_player* localplayer, vec3 target_angles){
  c_base_player* best_player = nullptr;
  i32 best_damaged = 0;
  for(i32 i = 1; i <= globaldata->max_clients; i++){
    c_base_player* player = global->entity_list->get_entity(i);
    if(player == nullptr)
      continue;

    if(!player->is_valid())
      continue;

    if(localplayer == player)
      continue;

    if(localplayer->team() != player->team())
      continue;

    if(player->health()>= player->max_health())
      continue;

    if(player->is_ubercharged())
      continue;

    // No FOV checks otherwise it will always prefer the person who's closest the projectile to player.
    float fov = math::get_fov_delta(target_angles, localplayer->shoot_pos(), player->obb_center());
    if(fov > 30.f)
      continue;

    // Choose the player that has the most health missing.
    i32 damaged = math::abs(player->health()- player->max_health());
    if(damaged <= best_damaged)
      continue;

    best_damaged = damaged;
    best_player  = player;
  }

  return best_player;
}

vec3 c_auto_airblast::find_redirect_target(c_base_player* localplayer, vec3 target_angles, i32 entity_type, i32 wanted_position, bool& success){
  if(!config->automation.auto_reflect_redirect_towards){
    success = false;
    return vec3(0, 0, 0);
  }

  if(!config->automation.auto_reflect_aimbot){
    success = false;
    return vec3();
  }

  success = false;

  // If we got a healing projectile. Handle it here.
  if(config->automation.auto_reflect_redirect_healbolt_teammates && entity_type == REDIRECT_HEALS_TEAM){

    c_base_entity* healing_player = find_redirect_target_healing(localplayer, target_angles);
    if(healing_player != nullptr){

      vec3 best_pos;
      for(i32 i = 0; i <= MAX_REDIRECT_POSITIONS; i++){
        vec3 pos;
        if(!get_redirect_position(healing_player, i, pos))
          continue;

        if(config->automation.auto_reflect_fov > 0.f && config->automation.auto_reflect_fov < 180.f){
          if(math::get_fov_delta(global->untouched_cmd.view_angles, localplayer->shoot_pos(), pos) > config->automation.auto_reflect_fov)
            continue;
        }

        s_trace trace = global->trace->ray(localplayer->shoot_pos(), pos, mask_bullet);
        if(trace.vis(healing_player)){
          success  = true;
          best_pos = pos;
          if(i == 1) // We'll prefer to aim slightly above the center position of the player's bbox when attempting to redirect a healing bolt.
            break;
        }
      }

      // Found a player that needs healed.
      if(success)
        return best_pos;

    }
  }

  vec3 best_position;
  float best_fov             = 180.f;
  for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* entity = global->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    if(!is_redirect_target(localplayer, entity, entity_type))
      continue;

     for(i32 i = 0; i <= MAX_REDIRECT_POSITIONS; i++){
        vec3 pos;
        if(!get_redirect_position(entity, i, pos))
          continue;

        if(config->automation.auto_reflect_fov > 0.f && config->automation.auto_reflect_fov < 180.f){
          if(math::get_fov_delta(global->untouched_cmd.view_angles, localplayer->shoot_pos(), pos) > config->automation.auto_reflect_fov)
            continue;
        }

        float fov = math::get_fov_delta(target_angles, localplayer->shoot_pos(), pos);
        if(fov > 30.f || fov > best_fov && wanted_position != i)
          continue;

        //TODO filter out projectiles!
        s_trace trace = global->trace->ray(localplayer->shoot_pos(), pos, mask_bullet);
        if(!trace.vis(entity))
          continue;

        best_fov = fov;
        best_position = pos;
        success = true;

        if(i == wanted_position)
          break;
    }
  }

  return best_position;
}

bool c_auto_airblast::is_redirect_target(c_base_player* localplayer, c_base_entity* entity, i32 entity_type){
  if(localplayer == nullptr || entity == nullptr)
    return false;

  if(entity->is_dormant())
    return false;

  if(localplayer->team() == entity->team())
    return false;

  if(entity->is_player() && config->automation.auto_reflect_redirect_towards_players){

    c_base_player* p = (c_base_player*)entity;
    if(!p->is_alive())
      return false;

    // These people can't be killed by any projectiles we shoot at them.
    if(p->is_ubercharged() || p->is_bonked())
      return false;

    if(p->is_steam_friend() && config->automation.auto_reflect_redirect_ignore_friends)
      return false;

    if(p->is_cloaked() && config->automation.auto_reflect_redirect_ignore_cloaked)
      return false;

    return true;
  }
  else if(entity->is_sentry() && config->automation.auto_reflect_redirect_towards_sentry && !utils::is_truce_active())
    return true;
  else if(entity->is_dispenser() && config->automation.auto_reflect_redirect_towards_dispenser && !utils::is_truce_active())
    return true;
  else if(entity->is_teleporter() && config->automation.auto_reflect_redirect_towards_teleporter && !utils::is_truce_active())
    return true;
  else if(entity->is_projectile() && config->automation.auto_reflect_redirect_towards_projectiles && !utils::is_truce_active())
    return entity_type == REDIRECT_DESTROYS_PROJ;
  else if(entity->is_boss() && config->automation.auto_reflect_redirect_towards_boss)
    return true;

  return false;
}

vec3 c_auto_airblast::get_target_position(c_base_entity* entity){
  if(entity == nullptr)
    return vec3();

  vec3 pos = entity->obb_center();

  // Enemy players are lag compensated. While friendly players I don't know.
  if(entity->is_player())
    return pos;
  else{

    // This works really well on projectiles. I haven't tested on dodge ball servers yet.
    float ping = math::clamp(utils::get_latency(), 0.f, 5.f);
    vec3 pred_pos = pos + entity->est_velocity() * ping;
    vec3 ended = utils::trace_line(pos, pred_pos);
#if defined(DEV_MODE)
      render_debug->draw_line(pos, ended, colour(255, 0, 0), true, globaldata->interval_per_tick * 2);
#endif
    return ended;
  }

  return pos;
}

i32 c_auto_airblast::get_best_redirect_position(c_base_entity* entity){
  if(entity == nullptr)
    return 0;

  if(entity->is_demo_projectile())
    return entity->is_stickybomb() ? MAX_REDIRECT_POSITIONS : 2;
  else if(entity->is_arrow())
    return MAX_REDIRECT_POSITIONS;
  else if(entity->is_flare() || entity->is_healing_bolt() || entity->is_flame_ball())
    return 2;
  else if(entity->can_destroy_projectiles())
    return 1;

  return 0;
}

bool c_auto_airblast::get_redirect_position(c_base_entity* entity, i32 i, vec3& pos){
  if(entity == nullptr)
    return false;

  switch(i){
    case 0:
    {
      pos = entity->origin();
      break;
    }
    case 1:
    {
      pos = entity->obb_center();
      break;
    }
    case 2:{
      pos = entity->obb_center();
      pos.z += 25.f;
      break;
    }
    case 3:{
      pos = entity->obb_center();
      pos.z += 35.f;
      break;
    }
  }

  return true;
}