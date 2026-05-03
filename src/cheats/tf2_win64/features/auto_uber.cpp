#include "../link.h"

CLASS_ALLOC(c_auto_uber, auto_uber);

void c_auto_uber::run(){
  if(utils::is_truce_active())
    return;

  s_auto_uber_data uberdata;
  memset(&uberdata, 0, sizeof(s_auto_uber_data));
  uberdata.threat = 1.f;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  if(!localplayer->is_player_class(TF_CLASS_MEDIC))
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  if(!weapon->is_medigun() || weapon->weapon_id() == WPN_Vaccinator)
    return;

  if(weapon->is_kritzkrieg()){
    if(!config->automation.medic_auto_kritz)
      return;
  }
  else{
    if(!config->automation.medic_auto_uber)
      return;
  }

  global->info_panel->add_entry(weapon->is_kritzkrieg() ? INFO_PANEL_AUTO_KRITZ : INFO_PANEL_AUTO_UBER, WXOR(L"INACTIVE"));
  if(!weapon->can_uber())
    return;

  global->info_panel->add_entry(weapon->is_kritzkrieg() ? INFO_PANEL_AUTO_KRITZ : INFO_PANEL_AUTO_UBER, WXOR(L"RUNNING"));

  // Get our heal target.
  c_base_player* heal_target = weapon->get_heal_target();
  if(heal_target != nullptr){
    if(!heal_target->is_player())
      heal_target = nullptr;
    else{
      if(!weapon->is_kritzkrieg() && config->automation.auto_uber_friendsonly || weapon->is_kritzkrieg() && config->automation.auto_kritz_friendsonly){
        if(!heal_target->is_steam_friend())
          heal_target = nullptr;
      }
    }
  }

  if(weapon->is_kritzkrieg()){
    // Don't run if we have no heal target with the kritzkrieg.
    if(heal_target == nullptr)
      return;

    // Already crit boosted or is taunting.
    if(heal_target->is_crit_boosted() || heal_target->is_taunting())
      return;

    c_base_weapon* healed_weapon = heal_target->get_weapon();
    if(healed_weapon == nullptr) // Has no valid weapon.
      return;

    // Using weapons that deal no damage.
    if(healed_weapon->is_blast_jumper_weapon() || healed_weapon->is_harmless_item())
      return;

    // These weapons do little to no damage.
    if(healed_weapon->weapon_id() == WPN_SunStick || healed_weapon->weapon_id() == WPN_FanOWar)
      return;
  }

  // Begin processing entities.
  for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* ent = global->entity_list->get_entity(i);
    if(ent == nullptr)
      continue;

    if(ent->is_dormant())
      continue;

    if(ent->is_player()){
      process_player_threats(weapon, ent, localplayer, &uberdata);
      if(heal_target != nullptr)
        process_player_threats(weapon, ent, heal_target, &uberdata);
    }
    else{
      process_entity_threats(weapon, ent, localplayer, &uberdata);
      if(heal_target != nullptr)
        process_entity_threats(weapon, ent, heal_target, &uberdata);
    }
  }

  // Process the data we got.
  process_data(weapon, heal_target, &uberdata);
}

void c_auto_uber::process_player_threats(c_base_weapon* medigun, c_base_player* player, c_base_player* protect, s_auto_uber_data* uberdata){
  if(player == nullptr || protect == nullptr || medigun == nullptr)
    return;

  if(!player->is_valid() || !protect->is_valid())
    return;

  if(player->team() == protect->team())
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  // They will do no harm to us.
  if(!medigun->is_kritzkrieg()){
    if(player->is_bonked() && !config->automation.auto_uber_react_bonked || player->is_cloaked() && !config->automation.auto_uber_react_cloaked)
      return;
  
    if(player->is_steam_friend() && !config->automation.auto_uber_react_friends)
      return;

    if(protect->is_on_fire() && protect->get_health_percent() <= 25.f)
      uberdata->threat      += 75.f;
  }
  else{
    if(player->is_bonked() && !config->automation.auto_kritz_react_bonked || player->is_cloaked() && !config->automation.auto_kritz_react_cloaked)
      return;
  
    if(player->is_steam_friend() && !config->automation.auto_kritz_react_friends)
      return;

    if(player->is_ubercharge_hidden())
      return;
  }

  if(protect->is_ubercharged() || protect->is_bonked() || player->is_taunting())
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  c_base_weapon* weapon = player->get_weapon();
  if(weapon == nullptr)
    return;

  ctf_weapon_info* wep_info = weapon->get_weapon_info();
  if(wep_info == nullptr)
    return;

  // This person isn't a threat to us.
  if(weapon->is_harmless_item() || weapon->is_blast_jumper_weapon())
    return;

  i32 distance = protect->origin().distance_meter(player->origin());
  if(!medigun->is_kritzkrieg()){
    if(distance > AUTO_UBER_PLAYER_MAXDIST && !weapon->is_sniper_rifle())
      return;
  }
  else{
    // If our heal target doesn't have a weapon then don't bother using crits on them.
    c_base_weapon* protect_weapon = protect->get_weapon();
    if(protect_weapon == nullptr)
      return;

    // Only restrict the distance of players if our heal target isn't using a sniper rifle.
    if(!protect_weapon->is_sniper_rifle()){
      if(distance > AUTO_KRITZ_PLAYER_MAXDIST)
        return;
    }
  }

  // We have enemies close by be aware!!
  uberdata->enemies_close_by++;
  uberdata->threat += 1.f;

  bool unused          = false;
  bool cheating        = cheat_detection->is_cheating(player->get_index());
  bool in_danger_range = (distance < auto_vaccinator->get_unknown_reaction_range());
  if(!is_visible(player, protect, unused))
    return;

  #if defined(DEV_MODE)
    render_debug->draw_line(protect->shoot_pos(), player->shoot_pos(), colour(255, 100, 0, 255), true, globaldata->interval_per_tick * 2.f);
  #endif 

  float fov = math::get_fov_delta(player->viewangles(), player->shoot_pos(), protect->shoot_pos());

  uberdata->vis_enemies++;
  uberdata->threat += 12.f;
  if(!medigun->is_kritzkrieg()){

    if(player->is_ubercharged() || player->has_condition(TF_COND_MEGAHEAL))
      uberdata->threat += 25.f;

    // We'll receive extra damage or are currently losing health.
    if(player->is_vulnerable())
      uberdata->threat += 5.f;

    if(player->is_crit_boosted())
      uberdata->threat += 50.f;
    else if(player->has_condition(TF_COND_OFFENSEBUFF) || !(protect->entity_flags() & FL_ONGROUND) && weapon->deals_minicritdmg_inair())
      uberdata->threat += 35.f;

    if(weapon->is_shotgun() || weapon->is_scattergun() || weapon->is_minigun() && player->has_condition(TF_COND_AIMING)){
      uberdata->threat += (distance <= AUTO_UBER_CLOSE_RANGE - 1) ? 40.f : 20.f;
      if(player->is_crit_boosted())
        on_deploy_charge(uberdata, XOR("Near by shotgun or spunup minigun (player has crits)"));
    }

    if(player->is_player_class(TF_CLASS_HEAVY)){
      if(player->has_condition(TF_COND_AIMING))
        uberdata->threat++;

      // We're so dead...
      if(player->is_bullet_resist())
        uberdata->threat += 10.f;

      if(cheating)
        uberdata->threat += 5.f;

      // If a heavy has one of these conditions then we're sadly truly going to die...
      if(player->is_bullet_charge() || player->is_blast_charge() || player->is_fire_charge() || player->is_ubercharged() && !player->is_ubercharge_hidden()){
        on_deploy_charge(uberdata, XOR("Vaccinator charged or ubercharged heavy near by"));
      }
      else{
        // We're gonna die really quick! Uber now!
        if(distance <= AUTO_UBER_CLOSE_RANGE){
          on_deploy_charge(uberdata, XOR("Close range heavy"));
        }
      }
    }
    else if(player->is_player_class(TF_CLASS_SNIPER)){
      if(player->is_scoped() && fov < 4.f || cheating){
       on_deploy_charge(uberdata, XOR("Scoped sniper near head or cheating sniper visible"));
      }
    }

    // Somebody has a projectile weapon and they're with in range where they can deal damage to us before we see the projectile networked to us.
    if(weapon->is_huntsman()){
      if(in_danger_range)
        on_deploy_charge(uberdata, XOR("Close to huntsman sniper"));
    }
    else if(weapon->is_rocket_launcher() || weapon->is_grenade_launcher()){
      uberdata->threat += in_danger_range ? 40.f : 20.f;
      if(player->is_crit_boosted() && in_danger_range){
        on_deploy_charge(uberdata, XOR("Crit boosted soldier / demo near by that is with in danger range"));
      }
    }
    
    // Phlog pyro or pyro with just kritz.
    if(weapon->is_flamethrower() && distance <= 8){ // 8 is the "effective" range of the flamethrower.
      if(player->is_crit_boosted()){
        on_deploy_charge(uberdata, XOR("Flamethrower pyro with crits"));
      }
      else
        uberdata->threat += protect->is_on_fire() ? 25.f : 10.f;
    }

    // Against melee weapons.
    if(distance <= AUTO_UBER_CLOSE_RANGE + 2){
      if(weapon->is_third_degree()){
        on_deploy_charge(uberdata, XOR("Third degree pyro near by"));
      }
      else if(weapon->is_knife() && !player->is_cloaked(false)){
        if(fov > 70.f || cheating)
          on_deploy_charge(uberdata, XOR("Spy close with knife out"));

        // We can get facestabbed.
        if(protect->health() <= wep_info->damage){
          on_deploy_charge(uberdata, XOR("Spy can facestab kill healer or heal target!"));
          return;
        }

        uberdata->threat += 10.f;
      }
      else if(weapon->is_melee()){

        // They can kill us without crits!
        if(protect->health() <= wep_info->damage){
          on_deploy_charge(uberdata, XOR("Melee damage is equal or greater than HP of heal target or healer."));
          return;
        }
        
        uberdata->threat += 15.f;

        c_base_weapon* protect_weapon = protect->get_weapon();
        if(protect_weapon != nullptr){
          if(protect_weapon->weapon_id() == WPN_FistsOfSteel) // They take alot more damage.
            on_deploy_charge(uberdata, XOR("Heal target is using fists of steel near other melee source"));
          else if(weapon->weapon_id() == WPN_Katana && protect_weapon->weapon_id() == WPN_Katana) // You'll get 1 shotted.
            on_deploy_charge(uberdata, XOR("Heal target is near by another player using the Katana melee weapon."));
        }

        if(player->is_crit_boosted() || protect->is_vulnerable() || player->has_condition(TF_COND_OFFENSEBUFF) || player->has_condition(TF_COND_SHIELD_CHARGE) || cheating) // Assume cheaters have 100% melee crits.
          on_deploy_charge(uberdata, XOR("Damage boosted melee player near by"));  
      }
    }
  }
  else{

    // !! -Rud
    // The goal of kritzkrieg auto-uber is to be offensive auto-uber.
    // There for if it never ubers before you die then that's on you. Because it's not designed to "last second uber" before you die from a critical rocket.
    // Its goal is to uber at the correct time to deal the most damage as possible to the enemy team.

    // They can't be damaged so don't bother.
    if(player->is_ubercharged())
      return;

    // These players are immune to crits. So there is no point in doing any logic on them.
    if(player->has_condition(TF_COND_DEFENSEBUFF_HIGH) || player->has_condition(TF_COND_DEFENSEBUFF))
      return;

    c_base_weapon* protect_weapon = protect->get_weapon();
    if(protect_weapon == nullptr)
      return;

    // The vaccinator gives crit immunity.
    i32 resist_type = auto_vaccinator->get_weapon_resist_type(protect_weapon);
    if(player->is_bullet_charge() && resist_type == 0 || player->is_blast_charge() && resist_type == 1 || player->is_fire_charge() && resist_type == 2)
      return;

    // These classes have value in terms of critting them to death.
    switch(player->player_class()){
      case TF_CLASS_HEAVY:
      case TF_CLASS_SOLDIER:
      case TF_CLASS_DEMOMAN:
      case TF_CLASS_MEDIC:
      {
        uberdata->threat += 20.f;
        break;
      }
    }

    // Somebody else has more damage than us. 
    if(player->has_condition(TF_COND_OFFENSEBUFF))
      uberdata->threat += 10.f;

    if(distance <= AUTO_UBER_CLOSE_RANGE){
      uberdata->threat += player->is_crit_boosted() ? 50.f : 20.f;
      if(cheating)
        on_deploy_charge(uberdata, XOR("Deployed crits because a cheater was close to us!"));
    }

    if(player->health() > 0)
      uberdata->threat += (float)player->health() / 10.f;
  }
}

void c_auto_uber::process_entity_threats(c_base_weapon* medigun, c_base_entity* ent, c_base_player* protect, s_auto_uber_data* uberdata){
  if(ent == nullptr || protect == nullptr || medigun == nullptr)
    return;

  if(ent->is_dormant())
    return;

  if(ent->team() == protect->team())
    return;

  if(protect->is_ubercharged() || protect->is_bonked())
    return;
  
  bool blast_in_radius = false;
  i32  distance        = protect->origin().distance_meter(ent->origin());

  if(!medigun->is_kritzkrieg()){
    if(ent->is_fake_projectile())
      return;

    if(ent->team() == protect->team()){
      if(ent->is_dispenser() || ent->is_ammo_pack())
        uberdata->heal_sources_near_by++;
  
      return;
    }

    if(ent->is_sentry() && !protect->is_disguised()){
      if(distance > 27) // Sentry range.
        return;
  
      if(ent->building() || ent->placing() || ent->sapped() || ent->disabled())
        return;
  
      if(!is_visible(ent, protect, blast_in_radius))
        return;
  
      c_base_player* builder = ent->get_building_owner();
      if(builder != nullptr){
        // The sentry will deal mini-crit damage.
        if(builder->is_player() && builder->has_condition(TF_COND_OFFENSEBUFF)){
          on_deploy_charge(uberdata, XOR("Sentry detected. But the owner of it has mini-crits from buff banner!"));
        }
      }
      // Threat level based on the sentry level and type.
      switch(ent->upgrade_level()){
        default: break;
        case 1:
        {
          uberdata->threat += ent->mini_building() ? 25.f : 40.f;
          break;
        }
        case 2:
        case 3:
        {
          on_deploy_charge(uberdata, XOR("Level 2 or 3 sentry visible"));
          break;
        }
      }
    }
    else if(ent->is_arrow()){
      if(distance > AUTO_UBER_PROJ_MAXDIST)
        return;
  
      if(!is_visible(ent, protect, blast_in_radius))
        return;
  
      uberdata->threat += 5.f;
  
      // Copied from autovacc: We want to get the dist to the head so we only uber if we're sure it's gonna headshot.
      float ping = math::clamp(utils::get_latency(), 0.f, 4.f);
      vec3 predicted_shoot_pos = utils::trace_line(protect->shoot_pos(), protect->shoot_pos() + (protect->velocity() * ping));
      vec3 predicted_pos       = utils::trace_line(ent->origin(), ent->origin() + (ent->est_velocity() * ping));
      float dist_to_head       = math::abs(predicted_shoot_pos.z - predicted_pos.z);
  
      if(blast_in_radius){
        uberdata->threat += ent->is_projectile_critical() ? 30.f : 25.f;
        if(dist_to_head <= 18.f)
         on_deploy_charge(uberdata, XOR("Huntsman arrow close by head or is critical"));
        else if(dist_to_head <= 26.f)
          uberdata->threat += 10.f;
      }
  
      if(protect->is_vulnerable())
        uberdata->threat += 10.f; 
    }
    else if(ent->is_flare()){
      if(distance > AUTO_UBER_PROJ_MAXDIST)
        return;
  
      if(!is_visible(ent, protect, blast_in_radius))
        return;
  
      uberdata->threat += 5.f;
      if(blast_in_radius){
        if(protect->is_on_fire() || ent->is_projectile_critical()){
          if(protect->health()<= 90){
            on_deploy_charge(uberdata, XOR("Critical-flare. Would have killed us or our heal target if hit"));
          }
          else
            uberdata->threat += 40.f;
        }
      }
    }
    else if(ent->is_fire_spell()){
      if(distance > AUTO_VACCINATOR_PROJ_MAXDIST)
        return;
  
      if(!is_visible(ent, protect, blast_in_radius))
        return;
  
      if(blast_in_radius){
       on_deploy_charge(uberdata, XOR("Fire spell (Deals alot of damage)"));
      }
      else
        uberdata->threat += 35.f;
    }
    else if(ent->is_rocket()){
      if(distance > AUTO_UBER_PROJ_MAXDIST)
        return;
  
      if(!is_visible(ent, protect, blast_in_radius))
        return;
  
      if(blast_in_radius){
        if(ent->is_projectile_critical())
          on_deploy_charge(uberdata, XOR("Critical rocket with in blast radius"));
        else
          uberdata->threat += 40.f;
      }
      else
        uberdata->threat += ent->is_projectile_critical() ? 50.f : 30.f;
  
      if(protect->is_vulnerable())
        uberdata->threat += 10.f;
    }
    else if(ent->is_demo_projectile()){
      if(distance > AUTO_UBER_PROJ_MAXDIST)
        return;
  
      if(!is_visible(ent, protect, blast_in_radius))
        return;
  
      if(blast_in_radius){
        if(ent->is_projectile_critical())
          on_deploy_charge(uberdata, XOR("Critical demo projectile with in blast radius"));
        else
          uberdata->threat += 40.f;
      }
      else
        uberdata->threat += ent->is_projectile_critical() ? 50.f : 30.f;
      
      if(protect->is_vulnerable())
        uberdata->threat += 10.f;
    }
  }
  else{
    if(ent->is_tank_boss() && config->automation.auto_kritz_deploy_near_tank){
      if(distance > 8)
        return;

      if(!is_visible(ent, protect, blast_in_radius))
        return;
      
      on_deploy_charge(uberdata, XOR("Near mvm Tank"));  
    }
  }
}

void c_auto_uber::process_data(c_base_weapon* medigun, c_base_player* heal_target, s_auto_uber_data* uberdata){

  float uber_cost = calc_uber_cost(heal_target);

  #if defined(DEV_MODE) && defined(AUTO_UBER_DEBUG)
    DBG("[!] cost: %2.2f\n", uber_cost);
    DBG("[!] enemies_close_by: %i\n", uberdata->enemies_close_by);
    DBG("[!] vis_enemies: %i\n", uberdata->vis_enemies);
    DBG("[!] heal_sources_near_by: %i\n", uberdata->heal_sources_near_by);
  #endif

  if(uberdata->threat > 0.f){
    uberdata->threat /= 2.f;

    if(!medigun->is_kritzkrieg()){

      if(heal_target != nullptr){
        c_player_data* data = heal_target->data();
        if(data != nullptr){
  
          // These classes aren't powerful at all. There for we'll reduce our threat score down if we're healing one of them.
          if(!cheat_detection->is_cheating(heal_target->get_index())){
            switch(heal_target->player_class()){
              default: break;
              case TF_CLASS_SPY:
              case TF_CLASS_ENGINEER:
              case TF_CLASS_SCOUT:
              case TF_CLASS_MEDIC:
              case TF_CLASS_SNIPER:
              {
                uberdata->threat *= 0.75f;
                break;
              }
            }
          }
        }
      }

      while(uberdata->heal_sources_near_by > 0){
        uberdata->threat *= 0.85f; // For every healing source near by reduce threat score by 15%.
        uberdata->heal_sources_near_by--;
      }

      if(uberdata->vis_enemies > 0)
        uberdata->vis_enemies /= 2;

      if(uberdata->enemies_close_by > 0)
        uberdata->enemies_close_by /= 2;

      if(uberdata->vis_enemies > 0){
        float mult = 0.75f + (math::clamp(config->automation.auto_uber_visible_enemy_threat_multiplier, 25.f, 100.f) / 100.f); // Make 50 an option.
        while(uberdata->vis_enemies > 0){
          uberdata->threat *= mult;
          uberdata->vis_enemies--;
        }
      }
      else if(uberdata->enemies_close_by > 0){
        float mult = 0.25f + (math::clamp(config->automation.auto_uber_nearby_enemy_threat_multiplier, 25.f, 100.f) / 250.f); // Make 50 an option.
        while(uberdata->enemies_close_by > 0){
          uberdata->threat *= mult;
          uberdata->enemies_close_by--;
        }
      }
    }
  }

  DBG("[!] threat: %2.2f\n", uberdata->threat);
  if(uberdata->threat < uber_cost && !uberdata->deploy_uber)
    return;

  global->current_cmd->buttons |= IN_ATTACK2;
}

void c_auto_uber::handle_medigun_attack2(c_base_weapon* wep){
  if(wep == nullptr)
    return;

  // If we aren't holding a valid medigun return. But, also set the createmove_force_attack2 to false.
  if(!wep->is_medigun()){
    global->createmove_force_attack2 = false;
    return;
  }
  
  // The vaccinator has its own version of this function.
  if(wep->is_vaccinator())
    return;

  if(!global->createmove_force_attack2)
    return;

  global->current_cmd->buttons |= IN_ATTACK2;
  global->createmove_force_attack2 = false;
} 

float c_auto_uber::calc_uber_cost(c_base_player* protect){

  float cost = AUTO_UBER_MAX2CHARGE;
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return cost;

  // More HP = Increased costs.
  // Less HP = Decreased costs.

  // Check our details!
  {
    if(localplayer->is_vulnerable())
      cost *= 0.80f;

    float hp = math::clamp(localplayer->get_health_percent(), 0.f, 300.f);
    if(hp > 0.0f)
      cost *= (hp / 100.f);
  }

  //Check our heal target details!
  if(protect != nullptr){
    if(protect->is_vulnerable())
      cost *= 0.80f;

    float hp = math::clamp(protect->get_health_percent(), 0.f, 300.f);
    if(hp > 0.0f)
      cost *= (hp / 100.f);
  }

  return math::clamp(cost, 1.f, AUTO_UBER_MAX2CHARGE);
}

void c_auto_uber::on_deploy_charge(s_auto_uber_data* uberdata, std::string reason){
  if(uberdata == nullptr)
    return; 

  // This code is temp since in dev mode the can_uber check is disabled.
  {
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    c_base_weapon* weapon = localplayer->get_weapon();
    if(weapon == nullptr)
      return;

    if(!weapon->can_uber())
      return;
  }

  uberdata->threat     += 75.f;
  uberdata->deploy_uber = true;
}

// This function exists just incase we wanna do anything special in it.
bool c_auto_uber::is_visible(c_base_entity* ent, c_base_player* protect, bool& in_blast_radius){
  if(protect == nullptr || ent == nullptr)
    return false;

  if(ent->is_tank_boss()){
    s_trace trace = global->trace->ray(protect->shoot_pos(), ent->obb_center(), mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
    return trace.vis(ent);
  }

  return auto_vaccinator->is_visible(ent, protect, in_blast_radius);
}