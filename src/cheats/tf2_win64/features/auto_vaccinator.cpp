#include "../link.h"

CLASS_ALLOC(c_auto_vaccinator, auto_vaccinator);

void c_auto_vaccinator::run(){
  if(!config->automation.auto_vaccinator)
    return;

  s_auto_vaccinator_data data;
  memset(&data, 0, sizeof(s_auto_vaccinator_data));
  data.bullet                   = 1;
  data.blast                    = 1;
  data.fire                     = 1;

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

  if(!weapon->is_vaccinator())
    return;

  global->info_panel->add_entry(INFO_PANEL_AUTO_VACCINATOR, WXOR(L"INACTIVE"));
  if(config->automation.auto_vaccinator_use_key){
    if(!key.is_valid())
      key = c_key_control(&config->automation.auto_vaccinator_key, &config->automation.auto_vaccinator_key_held, &config->automation.auto_vaccinator_key_toggle, &config->automation.auto_vaccinator_key_double_click);

    if(!key.is_toggled())
      return;
  }

  global->info_panel->add_entry(INFO_PANEL_AUTO_VACCINATOR, WXOR(L"RUNNING"));
  if(!weapon->can_uber()) // Not enough charge.
    data.flags |= AUTO_CHARGE_CANNOT_UBER;

  process_manual_charge_cycle();
  if(config->automation.auto_vaccinator_fullcontrol)
    global->current_cmd->buttons &= ~IN_RELOAD;

  c_base_player* heal_target = weapon->get_heal_target();
  if(heal_target != nullptr){
    if(!heal_target->is_player())
      heal_target = nullptr;
    else{
      if(config->automation.auto_vaccinator_friendsonly){
        if(!heal_target->is_steam_friend())
          heal_target = nullptr;
      }
    }
  }

  // Show off what our manual charge is.
  if(config->automation.auto_vaccinator_fullcontrol && config->automation.auto_vaccinator_manualcharge_resist_cycle_option && !config->automation.auto_vaccinator_manualcharge_resist_none){
    if(config->automation.auto_vaccinator_manualcharge_resist_bullet)
      global->info_panel->add_entry(INFO_PANEL_AUTO_VACCINATOR_MANUAL_CHARGE, WXOR(L"BULLET"), colour(28, 255, 62, 255));
    else if(config->automation.auto_vaccinator_manualcharge_resist_blast)
      global->info_panel->add_entry(INFO_PANEL_AUTO_VACCINATOR_MANUAL_CHARGE, WXOR(L"BLAST"), colour(128, 128, 128, 255));
    else if(config->automation.auto_vaccinator_manualcharge_resist_fire)
      global->info_panel->add_entry(INFO_PANEL_AUTO_VACCINATOR_MANUAL_CHARGE, WXOR(L"FIRE"), colour(255, 70, 0, 255));
  }

  for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* ent = global->entity_list->get_entity(i);
    if(ent == nullptr)
      continue;

    if(ent->is_dormant())
      continue;

    if(ent->is_player()){
      handle_player(localplayer, ent, &data);

      if(heal_target != nullptr)
        handle_player(heal_target, ent, &data);
    }
    else{
      handle_entity(localplayer, ent, &data);

      if(heal_target != nullptr)
        handle_entity(heal_target, ent, &data);
    }
  }

  // Process data if we aren't trying to perform a "valid" manual charge on ourself or our heal target.
  bool shouldnt_process_data = process_manual_charge(localplayer);
  if(!shouldnt_process_data)
     shouldnt_process_data = process_manual_charge(heal_target);

  if(!shouldnt_process_data)
    process_data(heal_target, &data);
}

bool c_auto_vaccinator::is_visible(c_base_entity* ent, c_base_player* protect, bool& in_blast_radius, bool predict_players){
  if(ent == nullptr || protect == nullptr)
    return false;

    float ping = math::clamp(utils::get_latency(), 0.1f, 4.f);
    vec3 predicted_shoot_pos = utils::trace_line(protect->shoot_pos(), protect->shoot_pos() + (protect->velocity() * ping), mask_bullet, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);

#if defined(DEV_MODE) && defined(AUTO_VACCINATOR_DEBUG)
      render_debug->draw_line(protect->shoot_pos(), predicted_shoot_pos, colour(255, 100, 0), true, globaldata->interval_per_tick * 2.f);
#endif

  in_blast_radius = false;
  if(ent->is_player()){
    c_base_player* player = (c_base_player*)ent;
    if(!player->is_valid())
      return false;

    vec3 other_shoot_pos = predict_players ? utils::trace_line(player->shoot_pos(), player->shoot_pos() + (player->velocity() * ping), mask_bullet, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS) : player->shoot_pos();
    s_trace trace        = global->trace->ray(protect->shoot_pos(), other_shoot_pos, mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
    if(trace.vis(player))
      return true;
    else{
      // Its more reliable to predict our healer targets position than the enemy
      // Its true that people could peak you and kill you before you can vac
      // But in reality you will only have your charges wasted by someone who will just a,d spam peak
      // To make your charges drain, much more reliable in those cases for you to hug the vac key
      // - senator
      trace = global->trace->ray(predicted_shoot_pos, other_shoot_pos, mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
      return trace.vis(player);
    }

  }
  else{
    if(ent->is_dormant())
      return false;

    vec3 predicted_pos = utils::trace_line(ent->origin(), ent->origin() + (ent->est_velocity() * ping));
    if(ent->is_sentry()){
      s_trace trace = global->trace->ray(predicted_shoot_pos, ent->obb_center(), mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
      if(trace.vis(ent))
        return true;

      // If the trace failed for whatever reason check to see if the sentry is attacking us.
      c_base_player* sentry_target = ent->get_sentry_target();
      if(sentry_target != nullptr){
        if(sentry_target == protect)
          return true;
      }

      return false;
    }
    else{
      if(ent->is_rocket()){
        // predict the trace of the rocket
        vec3 fwd, right, up;
        math::angle_2_vector(ent->get_abs_angles(), &fwd, &right, &up);
        vec3 pos = fwd * ent->est_velocity().length();

        i32 blast_dist = 4;
        c_base_weapon* launcher = ent->get_launcher();
        if(launcher != nullptr){
          if(launcher->weapon_id() == WPN_DirectHit)
            blast_dist = 2;
        }

        s_trace blast_trace         = global->trace->ray(ent->origin(), ent->origin() + (pos * 1024.f), mask_shot_hull, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_HIT_TEAM);
        s_trace blast_visible_trace = global->trace->ray(blast_trace.end, predicted_shoot_pos, mask_explosion, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_HIT_TEAM);

        if(blast_visible_trace.vis(protect)){
          in_blast_radius = predicted_shoot_pos.distance_meter(blast_trace.end) <= blast_dist;
          return true;
        }
      }
      else if(ent->is_demo_projectile()){ // Demo projectiles are explosive so they can damage us through various surfaces. 
        s_trace trace = global->trace->ray(predicted_shoot_pos, predicted_pos, mask_explosion, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_HIT_TEAM);
        if(trace.vis()){
          in_blast_radius = trace.start.distance(trace.end) <= 250.f;
          return true;
        }
      }
      else{
        s_trace trace = global->trace->ray(predicted_shoot_pos, predicted_pos, mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_HIT_TEAM);
        if(trace.vis()){

          i32 death_dist = AUTO_VACCINATOR_CLOSE_RANGE;
          if(ent->is_arrow())
            death_dist = 2;

          // This won't be as accurate as the rocket one since these projectiles are affected by gravity.
          in_blast_radius = predicted_shoot_pos.distance_meter(trace.end) <= death_dist;
          return true;
        }
      }
    }
  }
  return false;
}

void c_auto_vaccinator::handle_player(c_base_player* protect, c_base_player* player, s_auto_vaccinator_data* vacdata){
  if(vacdata == nullptr)
    return;

  if(player == nullptr || protect == nullptr)
    return;

  if(!player->is_valid() || !protect->is_valid())
    return;

  if(protect->is_on_fire())
    vacdata->after_burn = true;

  if(player->team() == protect->team())
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  // Don't check players during a truce.
  if(utils::is_truce_active())
    return;

  // These people aren't hurting anybody.
  if(player->is_bonked(true) && !config->automation.auto_vaccinator_react_bonked || player->is_cloaked() && !config->automation.auto_vaccinator_react_cloaked || player->is_taunting())
    return;

  // Don't react to steam friends.
  if(player->is_steam_friend() && !config->automation.auto_vaccinator_react_friends)
    return;

  // We're ubercharged.
  if(protect->is_ubercharged() || protect->is_bonked(true))
    return;

  if(protect->is_on_fire() && protect->get_health_percent() <= 10.f){
    vacdata->fire += 12;
    vacdata->flags |= AUTO_CHARGE_FLAG_FIRE;
  }

  c_base_weapon* weapon = player->get_weapon();
  if(weapon == nullptr)
    return;

  ctf_weapon_info* wep_info = weapon->get_weapon_info();
  if(wep_info == nullptr)
    return;

  i32 distance = protect->origin().distance_meter(player->origin());
  if(distance > AUTO_VACCINATOR_PLAYER_MAXDIST && !weapon->is_sniper_rifle()) // Player is no threat to us far away.
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  float fov             = math::get_fov_delta(player->viewangles(), player->shoot_pos(), protect->shoot_pos());
  bool  predict_players = should_predict_other_players(protect, player);

  // used for the in_blast_radius bool.
  bool unused          = false;
  bool cheating        = cheat_detection->is_cheating(player->get_index());
  bool in_danger_range = (distance < get_unknown_reaction_range());

  i32 resist_type = get_weapon_resist_type(weapon);
  if(resist_type == -1)
    return;

  if(resist_type == RESIST_TYPE_BULLET){
    if(protect->is_bullet_charge(true))
      return;

    if(!player->is_player_class(TF_CLASS_MEDIC)) // Medic's are a bit useless when it comes to damage. Don't track them for the passive heal resist.
      vacdata->overall_bullet++;

    if(!is_visible(player, protect, unused, predict_players) && distance >= AUTO_VACCINATOR_CLOSE_RANGE - 2)
      return;

    float expected_damage = utils::get_damage(player, player->shoot_pos(), protect->obb_center()) * wep_info->bullets_per_shot;
    vacdata->bullet_dmg  += expected_damage;

    vacdata->bullet++;
    vacdata->bullet += player->healers();

    if(expected_damage > 0.f){
      expected_damage /= 2.f; // This damage isn't accurate 101 with the game. Hence why we divide by 2
      if((i32)expected_damage > protect->health()){
        force_uber_charge(vacdata, WXOR(L"Expected damage exceeds protected health"), AUTO_CHARGE_FLAG_BULLET);
      }
    }

    // A huntsman sniper is with in a zone we can't react fast enough time! Deploy charges now!!!
    if(weapon->is_huntsman() && in_danger_range){
      force_uber_charge(vacdata, WXOR(L"Huntsman player too close"), AUTO_CHARGE_FLAG_BULLET);
    }

    if(player->is_ubercharged() || player->has_condition(TF_COND_MEGAHEAL))
      vacdata->bullet += 6;

    // We'll receive extra damage or are currently losing health.
    if(protect->is_vulnerable())
      vacdata->bullet += 2;

    // Is this player receiving extra damage some how?
    {
      if(player->is_crit_boosted())
        vacdata->bullet += 6;
      else if(player->has_condition(TF_COND_OFFENSEBUFF) || !(protect->entity_flags() & FL_ONGROUND) && weapon->deals_minicritdmg_inair())
        vacdata->bullet += 4;
    }

    // They have high spread weapons and they're very close!
    if(weapon->is_shotgun() || weapon->is_scattergun() || weapon->is_minigun() && player->has_condition(TF_COND_AIMING)){
      if(distance <= AUTO_VACCINATOR_CLOSE_RANGE){
        if(cheating)
          force_uber_charge(vacdata, WXOR(L"Cheater in lethal DT range"), AUTO_CHARGE_FLAG_BULLET);
        else
          force_uber_charge(vacdata, weapon->is_minigun() ? WXOR(L"Minigun in lethal range") : WXOR(L"Shotgun in lethal range"), AUTO_CHARGE_FLAG_BULLET);
      }
    }

    // Heavies are high DPS
    if(player->is_player_class(TF_CLASS_HEAVY)){
      vacdata->bullet += cheating ? 10 : 2;

      // Heavy might have his minigun deployed.
      if(player->has_condition(TF_COND_AIMING))
        vacdata->bullet += 2;

      if(player->is_bullet_resist())
        vacdata->bullet += 2;

      // Heavy is vaccinator bullet charged.
      if(player->is_bullet_charge() || player->is_blast_charge() || player->is_fire_charge() || player->is_ubercharged()){
        force_uber_charge(vacdata, WXOR(L"Heavy nearby that is uber/vaccinator charged"), AUTO_CHARGE_FLAG_BULLET);
      }
      else if(player->is_bullet_resist())
        vacdata->bullet += 4;

      // The heavy will light us on fire and deal 15% more damage.
      if(distance <= AUTO_VACCINATOR_CLOSE_RANGE || protect->is_on_fire()){
        if(weapon->weapon_id() == WPN_HuoLongHeatmaker1 || weapon->weapon_id() == WPN_HuoLongHeatmaker2)
          vacdata->bullet += 10;
      }
    }
    else if(player->is_player_class(TF_CLASS_SNIPER)){ // Is the sniper close to headshotting us?
      if(weapon->can_headshot()){

        bool is_scoped = player->is_scoped();
        if(weapon->is_classic_sniper_rifle() && !is_scoped) // You can be unscoped with this weapon and headshot people.
          is_scoped = player->has_condition(TF_COND_AIMING);

        if(is_scoped && fov < 8.f || cheating){
          force_uber_charge(vacdata, cheating ? WXOR(L"A cheating sniper was visible") : WXOR(L"Sniper aiming near head"), AUTO_CHARGE_FLAG_BULLET);
        }
      }
    }
  }
  else if(resist_type == RESIST_TYPE_BLAST){
    if(protect->is_blast_charge(true))
      return;

    if(weapon->is_blast_jumper_weapon())
      return;

    vacdata->overall_blast++;
    if(!is_visible(player, protect, unused))
      return;

    vacdata->blast_dmg += utils::get_damage(player, player->shoot_pos(), protect->obb_center());
    if(in_danger_range)
      force_uber_charge(vacdata, WXOR(L"Projectile weapon too close"), AUTO_CHARGE_FLAG_BLAST);


    vacdata->blast++;
    vacdata->blast += player->healers();
    if(player->is_ubercharged() || player->has_condition(TF_COND_MEGAHEAL))
      force_uber_charge(vacdata, WXOR(L"Player nearby that is uber/quickfix charged"), AUTO_CHARGE_FLAG_BLAST | AUTO_CHARGE_FLAG_BLAST_INSTANT_KILL);

    // We'll receive extra damage or are currently losing health.
    if(protect->is_vulnerable())
      vacdata->blast += 2;

    if(player->is_crit_boosted())
      vacdata->blast += 6;
    else if(player->has_condition(TF_COND_OFFENSEBUFF) || !(protect->entity_flags() & FL_ONGROUND) && weapon->deals_minicritdmg_inair())
      vacdata->blast += 4;

    if(weapon->weapon_id() == WPN_DirectHit)
      vacdata->blast++;
  }
  else if(resist_type == RESIST_TYPE_FIRE){
    if(protect->is_fire_charge(true))
      return;

    vacdata->overall_fire++;
    if(!is_visible(player, protect, unused))
      return;

    vacdata->fire_dmg += utils::get_damage(player, player->shoot_pos(), protect->obb_center());
    vacdata->fire++;
    vacdata->fire += player->healers();
    if(player->is_ubercharged() || player->has_condition(TF_COND_MEGAHEAL))
      force_uber_charge(vacdata, WXOR(L"Player nearby that is uber/quickfix charged"), AUTO_CHARGE_FLAG_FIRE | AUTO_CHARGE_FLAG_FIRE_INSTANT_KILL);

    // We'll receive extra damage or are currently losing health.
    if(protect->is_vulnerable())
      vacdata->fire += 2;

    if(player->is_crit_boosted()){
      if(distance <= 12){
        force_uber_charge(vacdata, WXOR(L"Crit boosted pyro nearby"), AUTO_CHARGE_FLAG_FIRE | AUTO_CHARGE_FLAG_FIRE_INSTANT_KILL);
      }
      else
        vacdata->fire += 2;
    }
    else if(player->has_condition(TF_COND_OFFENSEBUFF))
      vacdata->fire += 2;

    // A pyro is rushing us!
    if(weapon->is_flamethrower() && distance <= AUTO_VACCINATOR_CLOSE_RANGE)
      vacdata->fire += 12;
  }

#if defined(AUTO_VACCINATOR_DEBUG)
  DBG("[!] handle_player: %s\n", player->info().name);
#endif
}

void c_auto_vaccinator::handle_entity(c_base_player* protect, c_base_entity* ent, s_auto_vaccinator_data* vacdata){
  if(vacdata == nullptr)
    return;

  if(ent == nullptr || protect == nullptr)
    return;

  if(ent->is_dormant())
    return;

  if(ent->team() == protect->team())
    return;

  if(ent->is_fake_projectile())
    return;

  if(protect->is_ubercharged() || protect->is_bonked())
    return;

  // If a truce is active. Then only process rockets.
  if(utils::is_truce_active() && !ent->is_rocket())
    return;

  bool blast_in_radius = false;
  i32 distance = protect->origin().distance_meter(ent->origin());

  // Only predict these entities.
  if(ent->is_rocket() || ent->is_demo_projectile() || ent->is_flame_ball() || ent->is_arrow() || ent->is_flare()){
    float ping         = math::clamp(utils::get_latency(), 0.1f, 4.f);
    vec3 predicted_pos = utils::trace_line(ent->origin(), ent->origin() + (ent->est_velocity() * ping));

    distance = protect->shoot_pos().distance_meter(predicted_pos);
  }

  if(ent->is_sentry() && !protect->is_disguised()){
    if(protect->is_bullet_charge(true))
      return;

    if(distance > 27)
      return;

    if(ent->building() || ent->placing() || ent->sapped() || ent->disabled())
      return;

    vacdata->overall_bullet++; // Just being near a sentry scares US!
    if(!is_visible(ent, protect, blast_in_radius))
      return;

    vacdata->bullet++;

    // Check to see if the engineer is being healed by the buff banner.
    c_base_player* builder = ent->get_building_owner();
    if(builder != nullptr){
      if(builder->is_player()){
        if(builder->has_condition(TF_COND_OFFENSEBUFF))
          vacdata->bullet += 6;
      }
    }

    // mini-sentries are very useless in their DPS.
    if(!ent->mini_building())
      force_uber_charge(vacdata, WXOR(L"Sentry visible"), AUTO_CHARGE_FLAG_BULLET_INSTANT_KILL | AUTO_CHARGE_FLAG_BULLET);
    else{
      vacdata->bullet += 16;
      if(protect->is_vulnerable())
        vacdata->bullet += 8;
    }
  }
  else if(ent->is_arrow()){
    if(protect->is_bullet_charge(true))
      return;

    if(distance > AUTO_VACCINATOR_PROJ_MAXDIST)
      return;

    vacdata->overall_bullet++;
    if(!is_visible(ent, protect, blast_in_radius))
      return;

    vacdata->bullet += 2;

    // We want to get the dist to the head so we only uber if we're sure it's gonna headshot.
    float ping = math::clamp(utils::get_latency(), 0.f, 4.f);
    vec3 predicted_shoot_pos = utils::trace_line(protect->shoot_pos(), protect->shoot_pos() + (protect->velocity() * ping));
    vec3 predicted_pos       = utils::trace_line(ent->origin(), ent->origin() + (ent->est_velocity() * ping));
    float dist_to_head       = math::abs(predicted_shoot_pos.z - predicted_pos.z);

    if(blast_in_radius){
      vacdata->bullet += 16;
      if(dist_to_head <= 18.f && ent->is_huntsman_arrow() || ent->is_projectile_critical() || ent->is_deflected())
        force_uber_charge(vacdata, WXOR(L"Arrow lethal"), AUTO_CHARGE_FLAG_BULLET | AUTO_CHARGE_FLAG_BULLET_INSTANT_KILL);
    }

    if(protect->is_vulnerable())
      vacdata->bullet += 6;
  }
  else if(ent->is_flare()){
    if(protect->is_fire_charge(true))
      return;

    if(distance > AUTO_VACCINATOR_PROJ_MAXDIST)
      return;

    vacdata->overall_fire++;
    if(!is_visible(ent, protect, blast_in_radius))
      return;

    vacdata->fire++;
    if(protect->is_on_fire() || ent->is_projectile_critical() || ent->is_deflected()){
      if(protect->health()<= 90)
        force_uber_charge(vacdata, WXOR(L"Flare lethal"), AUTO_CHARGE_FLAG_FIRE | AUTO_CHARGE_FLAG_FIRE_INSTANT_KILL);
      else
        vacdata->fire += blast_in_radius ? 6 : 4;
    }
    else
      vacdata->fire += blast_in_radius ? 4 : 2;

    if(protect->is_vulnerable())
      vacdata->fire += 2;
  }
  else if(ent->is_fire_spell()){
    if(protect->is_fire_charge(true))
      return;

    if(distance > AUTO_VACCINATOR_PROJ_MAXDIST)
      return;

    vacdata->fire++;
    if(!is_visible(ent, protect, blast_in_radius))
      return;

    vacdata->fire++;
    if(blast_in_radius){ // These fire spells do ALOT of damage.
      force_uber_charge(vacdata, WXOR(L"Firespell"), AUTO_CHARGE_FLAG_FIRE | AUTO_CHARGE_FLAG_FIRE_INSTANT_KILL);
    }
    else
      vacdata->fire += 8;

    if(protect->is_vulnerable())
      vacdata->fire += 4;
  }
  else if(ent->is_flame_ball()){
    if(protect->is_fire_charge(true))
      return;

    if(distance > AUTO_VACCINATOR_PROJ_MAXDIST)
      return;

    vacdata->overall_fire++;
    if(!is_visible(ent, protect, blast_in_radius))
      return;

    vacdata->fire++;
    if(protect->is_on_fire() || protect->is_vulnerable()){
      force_uber_charge(vacdata, WXOR(L"Flameball lethal"), AUTO_CHARGE_FLAG_BLAST | AUTO_CHARGE_FLAG_BLAST_INSTANT_KILL);
    }
    else
      vacdata->fire++;
  }
  else if(ent->is_rocket()){
    if(protect->is_blast_charge(true))
      return;

    if(distance > AUTO_VACCINATOR_PROJ_MAXDIST)
      return;

    vacdata->overall_blast++;
    if(!is_visible(ent, protect, blast_in_radius))
      return;

    vacdata->blast++;
    if(ent->is_projectile_critical() || ent->is_deflected()){
      if(blast_in_radius)
        force_uber_charge(vacdata, WXOR(L"Critical rocket with in blast radius"), AUTO_CHARGE_FLAG_BLAST | AUTO_CHARGE_FLAG_BLAST_INSTANT_KILL);
      else
        vacdata->blast += 8;
    }
    else
      vacdata->blast += blast_in_radius ? 12 : 8;

    if(protect->is_vulnerable())
      vacdata->blast += 4;

    if(blast_in_radius)
      vacdata->blast_projectile_near_by++;
  }
  else if(ent->is_demo_projectile()){
    if(protect->is_blast_charge(true))
      return;

    if(distance > AUTO_VACCINATOR_PROJ_MAXDIST)
      return;

    vacdata->overall_blast++;
    if(!is_visible(ent, protect, blast_in_radius))
      return;

    vacdata->blast++;
    if(ent->is_projectile_critical() || ent->is_deflected()){
      if(blast_in_radius){
        force_uber_charge(vacdata, WXOR(L"Critical pill/sticky with in blast radius"), AUTO_CHARGE_FLAG_BLAST | AUTO_CHARGE_FLAG_BLAST_INSTANT_KILL);
      }
      else{
        if(ent->is_stickybomb())
          vacdata->blast += distance <= AUTO_VACCINATOR_CLOSE_RANGE ? 12 : 6;
        else
          vacdata->blast += distance <= AUTO_VACCINATOR_CLOSE_RANGE ? 12 : 6;
      }
    }
    else{
      if(ent->is_stickybomb()){
        vacdata->blast += distance <= AUTO_VACCINATOR_CLOSE_RANGE ? 8 : 4;
        if(blast_in_radius){
          vacdata->stickies_near_by++;
          if(vacdata->stickies_near_by >= 2){
            force_uber_charge(vacdata, WXOR(L"2+ stickies with in blast radius"), AUTO_CHARGE_FLAG_BLAST | AUTO_CHARGE_FLAG_BLAST_INSTANT_KILL);
          }
        }
      }
      else
        vacdata->blast += 8;
    }

    if(blast_in_radius)
      vacdata->blast_projectile_near_by++;
  }
}

void c_auto_vaccinator::process_data(c_base_player* protect, s_auto_vaccinator_data* vacdata){
  if(vacdata == nullptr)
    return;

  i32 resist = -1;
  i32 uber_cost = calc_uber_cost(protect);
  if(!global->reset_resist_state){
    global->resist_uber_state  = -1;
    global->reset_resist_state = true;
  }

  // Fix config errors.
  if(config->automation.auto_vaccinator_fullcontrol == config->automation.auto_vaccinator_passive){
    config->automation.auto_vaccinator_passive     = false;
    config->automation.auto_vaccinator_fullcontrol = true;
  }

  bool block_bullet = false;
  bool block_blast  = false;
  bool block_fire   = false;
  i32  block_count  = 0;

  if(config->automation.auto_vaccinator_disallow_resist_bullet){
    vacdata->overall_bullet = vacdata->bullet = -AUTO_VACCINATOR_MAGIC_THREAT_VALUE;
    block_bullet = true;
    block_count++;
  }

  if(config->automation.auto_vaccinator_disallow_resist_blast){
    vacdata->overall_blast  = vacdata->blast  = -AUTO_VACCINATOR_MAGIC_THREAT_VALUE;
    block_blast = true;
    block_count++;
  }

  if(config->automation.auto_vaccinator_disallow_resist_fire){
    vacdata->overall_fire   = vacdata->fire   = -AUTO_VACCINATOR_MAGIC_THREAT_VALUE;
    block_fire = true;
    block_count++;
  }

  if(block_count == 3){
    DBG("[!] c_auto_vaccinator::process_data - Everything is blocked!\n");
    return;
  } 

  bool single_charge_mode = (block_count == 2);

  // There is a lot of projectiles near us!
  if(vacdata->blast_projectile_near_by >= AUTO_VACCINATOR_PROJ_DANGER && !block_blast)
    vacdata->blast += 4;

  // Process 'Instant kill' panic flags.
  if(vacdata->flags > 0){
    if(config->automation.auto_vaccinator_fullcontrol){

      if(vacdata->flags & AUTO_CHARGE_FLAG_BULLET && !block_bullet)
        vacdata->bullet += AUTO_VACCINATOR_MAGIC_THREAT_VALUE;
      else if(vacdata->flags & AUTO_CHARGE_FLAG_BLAST && !block_blast)
        vacdata->blast  += AUTO_VACCINATOR_MAGIC_THREAT_VALUE;
      else if(vacdata->flags & AUTO_CHARGE_FLAG_FIRE && !block_fire)
        vacdata->fire   += AUTO_VACCINATOR_MAGIC_THREAT_VALUE;

    }
    else if(config->automation.auto_vaccinator_passive){

      if(vacdata->flags & AUTO_CHARGE_FLAG_BULLET_INSTANT_KILL && !block_bullet)
        resist = RESIST_TYPE_BULLET;
      else if(vacdata->flags & AUTO_CHARGE_FLAG_BLAST_INSTANT_KILL && !block_blast)
        resist = RESIST_TYPE_BLAST;
      else if(vacdata->flags & AUTO_CHARGE_FLAG_FIRE_INSTANT_KILL && !block_fire)
        resist = RESIST_TYPE_FIRE;

    }
  }

  // Apply sensitivity values.
  // Make these options placebo, since some people put these to really stupid values and then complain about it being bad.
  #if defined(DEV_MODE)
  {
    if(vacdata->bullet > 1 && config->automation.auto_vaccinator_sensitivity_bullet != 0 && !block_bullet){
      float offset = math::clamp(1.f + (float)config->automation.auto_vaccinator_sensitivity_bullet / 100.f, 0.01f, 2.0f);
      float score  = math::biggest(math::clamp(math::round((float)vacdata->bullet * offset), 1.f, AUTO_VACCINATOR_MAGIC_THREAT_FVALUE), 1.f);
      vacdata->bullet = (i32)score;
    }

    if(vacdata->blast > 1 && config->automation.auto_vaccinator_sensitivity_blast != 0 && !block_blast){
      float offset   = math::clamp(1.f + (float)config->automation.auto_vaccinator_sensitivity_blast / 100.f, 0.01f, 2.0f);
      float score    = math::biggest(math::clamp(math::round((float)vacdata->blast * offset), 1.f, AUTO_VACCINATOR_MAGIC_THREAT_FVALUE), 1.f);
      vacdata->blast = (i32)score;
    }

    if(vacdata->fire > 1 && config->automation.auto_vaccinator_sensitivity_fire != 0 && !block_fire){
      float offset   = math::clamp(1.f + (float)config->automation.auto_vaccinator_sensitivity_fire / 100.f, 0.01f, 2.0f);
      float score    = math::biggest(math::clamp(math::round((float)vacdata->fire * offset), 1.f, AUTO_VACCINATOR_MAGIC_THREAT_FVALUE), 1.f);
      vacdata->fire += (i32)score;
    }
  }
  #endif

  if(!single_charge_mode){ // We're only using one resistance so lets go ahead and just use that one.
    bool is_equal = block_count == 0 && (vacdata->bullet == vacdata->blast == vacdata->fire);
    if(!is_equal)
      is_equal = (block_bullet && vacdata->blast == vacdata->fire) || (block_blast && vacdata->bullet == vacdata->fire) || (block_fire && vacdata->bullet == vacdata->blast);
    
    // Process passive healing resistance.
    if(is_equal){

      // If we're on fire lets reduce the after burn damage.
      if(vacdata->after_burn && !block_fire)
        vacdata->fire++;
      else{

        if(config->automation.auto_vaccinator_passive_resist_bullet && !block_bullet)
          vacdata->bullet++;
        else if(config->automation.auto_vaccinator_passive_resist_blast && !block_blast)
          vacdata->blast++;
        else if(config->automation.auto_vaccinator_passive_resist_fire && !block_fire)
          vacdata->fire++;
        else{
          // Auto select the the resist where the most damage is currently at. This usually happens when there is no damage types visible.
          // Overall is a counter for how much XYZ damage sources are close by. No vischecks.
          if(vacdata->overall_bullet > vacdata->overall_blast && vacdata->overall_bullet > vacdata->overall_fire && !block_bullet)
            vacdata->bullet++;
          else if(vacdata->overall_blast > vacdata->overall_bullet && vacdata->overall_blast > vacdata->overall_fire && !block_blast)
            vacdata->blast++;
          else if(vacdata->overall_fire > vacdata->overall_bullet && vacdata->overall_fire > vacdata->overall_blast && !block_fire)
            vacdata->fire++;
        }
      }   
    }
  }

  // Find the resist with the highest threat score.
  if(config->automation.auto_vaccinator_fullcontrol){
    if(vacdata->bullet > vacdata->blast && vacdata->bullet > vacdata->fire && !block_bullet)
      resist = RESIST_TYPE_BULLET;
    else if(vacdata->blast > vacdata->bullet && vacdata->blast > vacdata->fire && !block_blast)
      resist = RESIST_TYPE_BLAST;
    else if(vacdata->fire > vacdata->bullet && vacdata->fire > vacdata->blast && !block_fire)
      resist = RESIST_TYPE_FIRE;
    else{
      return;
    }
  }

  bool ubercharge = false;

  // In the event we're cycling we want to remember that we wanna pop a charge on this resist type.
  if(global->resist_uber_state == -1 && resist != -1){
    if(config->automation.auto_vaccinator_passive || resist == RESIST_TYPE_BULLET && vacdata->bullet >= uber_cost || resist == RESIST_TYPE_BLAST && vacdata->blast >= uber_cost || resist == RESIST_TYPE_FIRE && vacdata->fire >= uber_cost){
      global->resist_uber_state = resist;
      ubercharge = true;
    }
  }
  else{
    // We got a resist we're looking to deploy a charge.
    ubercharge = true;
    resist = global->resist_uber_state;
  }

  // We don't have enough uber or something else is blocking us from ubering!
  if(vacdata->flags & AUTO_CHARGE_CANNOT_UBER){
    global->resist_uber_state = -1;
    ubercharge = false;
  }

  // The ubercharge we want and wait for the cheat to cycle to it.
  set_wanted_resist(resist);
  if(!is_wanted_cycle(resist)){
    if(config->automation.auto_vaccinator_fullcontrol)
      global->current_cmd->buttons &= ~IN_ATTACK2; // Remove IN_ATTACK2 haven't gotten the cycle we wanted yet.
  }
  else{
    if(ubercharge)
      global->current_cmd->buttons |= IN_ATTACK2;

    global->resist_uber_state = -1;
  }
}

void c_auto_vaccinator::force_uber_charge(s_auto_vaccinator_data* data, std::wstring reason, i32 add_flags){
  assert(data != nullptr);
  if(data->flags & add_flags || data->flags & AUTO_CHARGE_CANNOT_UBER)
    return;

  data->flags |= add_flags;

  //if(!config->misc.chat_notification_autovacc)
  //  return;

  std::string type_str = XOR("Bullet");
  i32         type     = 0;
  {
    if(add_flags & AUTO_CHARGE_FLAG_BLAST){
      type_str = XOR("Blast");
      type     = 1;
    }
    else if(add_flags & AUTO_CHARGE_FLAG_FIRE){
      type_str = XOR("Fire");
      type     = 2;
    }
  }

  static float next_notification_time[3];
  float time = math::time();
  if(next_notification_time[type] > time)
    return;

  #if defined(DEV_MODE)
  wchar_t buf[IDEAL_MAX_BUF_SIZE];
  formatW(buf, WXOR(L"Auto-vaccinator: %ls"), reason.c_str());

  notify->create(NOTIFY_TYPE_WARNING, buf);
  #endif

  next_notification_time[type] = time + 1.5f;
}

bool c_auto_vaccinator::is_wanted_cycle(i32 resist){
  if(resist < 0 || resist > 2)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(weapon->weapon_id() != WPN_Vaccinator)
    return false;

  return weapon->charge_resist_type() == resist;
}

void c_auto_vaccinator::perform_cycle(){
  if(wanted_resist_cycle < 0 || wanted_resist_cycle > 2)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  if(weapon->weapon_id() != WPN_Vaccinator){
    set_wanted_resist(-1);
    global->auto_vaccinator_reload_held = false;
    return;
  }

  if(weapon->charge_resist_type() != wanted_resist_cycle){
    if(global->auto_vaccinator_reload_held)
      global->current_cmd->buttons &= ~IN_RELOAD;
    else
      global->current_cmd->buttons |= IN_RELOAD;
    #if defined(AUTO_VACCINATOR_DEBUG)
    DBG("[!] Changing cycle from %i to %i\n", weapon->charge_resist_type(), wanted_resist_cycle);
    #endif
    global->current_cmd->buttons &= ~IN_ATTACK2; // We don't want people to be performing manual ubers and mistakenly use charges when the cheat switches.
    global->auto_vaccinator_reload_held = !global->auto_vaccinator_reload_held;
  }
  else{
    global->auto_vaccinator_reload_held = false;
    #if defined(AUTO_VACCINATOR_DEBUG)
    DBG("[!] Finished cycling resist to %i\n", wanted_resist_cycle);
    #endif
    set_wanted_resist(-2);
  }
}

bool c_auto_vaccinator::should_predict_other_players(c_base_player* protect, c_base_player* player){
  if(protect == nullptr || player == nullptr)
    return false;

  if(!player->is_valid())
    return false;

  c_player_data* data = player->data();
  if(data == nullptr)
    return false;

  // If we don't predict scoped snipers peeking us. We'll die epicly and then we won't have any charges to use.
  if(player->is_scoped() && cheat_detection->is_cheating(player->get_index()))
    return true;

  i32 dist = protect->origin().distance_meter(player->origin());
  if(dist > AUTO_VACCINATOR_CLOSE_RANGE + 2)
    return false;

  c_base_weapon* weapon = player->get_weapon();
  if(weapon == nullptr)
    return false;

  // Does this heavy have his minigun spun up? (Most cases heavies aren't marked as cheaters until they kill alot of people. Hence no cheater check here.)
  if(player->is_player_class(TF_CLASS_HEAVY) && player->has_condition(TF_COND_AIMING)){
    // They have to be getting healed by a vaccinator medic or be under a special condition.
    if(player->is_bullet_resist() || player->is_blast_resist() || player->is_fire_resist() || player->is_ubercharged() || player->is_quickfix_uber() || player->is_crit_boosted())
      return true;
  }
  else if(player->is_player_class(TF_CLASS_SCOUT)){
    if(weapon->is_scattergun()) // Counter DT scouts.
      return dist <= AUTO_VACCINATOR_CLOSE_RANGE; // It's 4 meters as of nov 1st 2021. It should be the ideal range for the 50% damage boost they get it with the scattergun.
  }

  return false;
}

// Used for projectile classes. A range where they could shoot a projectile weapon. But there is no way we can react in time to it.
i32 c_auto_vaccinator::get_unknown_reaction_range(){
  float ping = math::biggest(0.1f, utils::get_latency()) * 1000.f;
  return ping > 0.0f ? (i32)(ping / 16.f) : 1;
}

void c_auto_vaccinator::on_damage(c_game_event* event){
  if(!config->automation.auto_vaccinator)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  if(!localplayer->is_player_class(TF_CLASS_MEDIC))
    return;

  if(event == nullptr)
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  if(weapon->weapon_id() != WPN_Vaccinator)
    return;

  i32 heal_target_index = 0;
  c_base_player* heal_target = weapon->get_heal_target();
  if(heal_target != nullptr)
    heal_target_index = heal_target->get_index();

  i32 attacker  = event->get_int(XOR("attacker"));
  i32 victim    = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));
  if(attacker == victim)
    return;

  if(victim != global->localplayer_index && heal_target_index != victim)
    return;

  bool crit     = event->get_bool(XOR("crit"));
  bool minicrit = event->get_bool(XOR("minicrit"));
  i32 weaponid  = event->get_int(XOR("weaponid"));
  if(crit || minicrit){
    if(is_hitscan_weapon(weaponid)){
      #if defined(AUTO_VACCINATOR_DEBUG)
        DBG("[!] Forcing vaccinator charge use because we're hit by a critical shot!\n");
      #endif
      global->auto_vaccinator_prefer_resist = 0; // We want to override any "Activate charge" checks.
      global->createmove_force_attack2      = true;
    }
  }
}

// When we press IN_ATTACK2 to use a charge manually.
bool c_auto_vaccinator::process_manual_charge(c_base_player* protect){
  if(!(global->original_cmd.buttons & IN_ATTACK2) || !config->automation.auto_vaccinator_fullcontrol)
    return false;

  if(protect == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  // Don't bother running if we don't have even have 1 charge.
  if(weapon->uber_charge_level() <= 0.25f)
    return false;

  i32 resist = 0;
  // Manual charges aren't allowed. Abort!
  if(config->automation.auto_vaccinator_manualcharge_resist_none){
    global->current_cmd->buttons &= ~IN_ATTACK2;
    return false;
  }

  if(config->automation.auto_vaccinator_manualcharge_resist_blast)
    resist = 1;
  else if(config->automation.auto_vaccinator_manualcharge_resist_fire)
    resist = 2;

  // Check to see if we're already vaccinator charged with this resist type. If we are then don't cycle to this resist.
  // Removed (Mar 03rd 2024)
  // Example:
  // This was flawed from the start, if we popped bullet charge on a soldier and then wanted to do it on a heavy in hvh or w/e.
  // It would prevent us from being able to uber charge the heavy resulting in the heavy getting killed.

  //if(protect->is_bullet_charge(true) && resist == 0 || protect->is_blast_charge(true) && resist == 1 || protect->is_fire_charge(true) && resist == 2){
  //  global->current_cmd->buttons &= ~IN_ATTACK2;
  //  return false;
  //}

  // We're gonna let this function do whatever it has todo. Don't care about the boolean it returns.
  set_wanted_resist(resist);

  return true;
}

bool c_auto_vaccinator::process_manual_charge_cycle(){
  if(!config->automation.auto_vaccinator_fullcontrol || !config->automation.auto_vaccinator_manualcharge_resist_cycle_option || config->automation.auto_vaccinator_manualcharge_resist_none)
    return false;

  static bool reload = false;

  if(global->current_cmd->buttons & IN_RELOAD){
    if(!reload){

      if(config->automation.auto_vaccinator_manualcharge_resist_bullet){
        config->automation.auto_vaccinator_manualcharge_resist_bullet = false;
        config->automation.auto_vaccinator_manualcharge_resist_blast  = true;
      }
      else if(config->automation.auto_vaccinator_manualcharge_resist_blast){
        config->automation.auto_vaccinator_manualcharge_resist_blast  = false;
        config->automation.auto_vaccinator_manualcharge_resist_fire   = true;
      }
      else if(config->automation.auto_vaccinator_manualcharge_resist_fire){
        config->automation.auto_vaccinator_manualcharge_resist_bullet = true;
        config->automation.auto_vaccinator_manualcharge_resist_fire   = false;
      }
      else{

        // Fix config issues.
        config->automation.auto_vaccinator_manualcharge_resist_bullet = true;
        config->automation.auto_vaccinator_manualcharge_resist_blast  = false;
        config->automation.auto_vaccinator_manualcharge_resist_fire   = false;

        DBG("[-] process_manual_charge_cycle fixed config error\n");
      }

      // Let the user know that they've switched a resistiance.
      global->engine->client_cmd(XOR("play weapons\\vaccinator_toggle.wav"));

      reload = true;
      return true;
    }

    return false;
  }
  else
    reload = false;


  return false;
}

void c_auto_vaccinator::handle_medigun_attack2(c_base_weapon* wep){
  if(wep == nullptr)
    return;

  // If we aren't holding a valid medigun return. But, also set the createmove_force_attack2 to false. 
  if(!wep->is_medigun()){
    global->createmove_force_attack2 = false;
    return;
  }

  // This is for the vaccinator only so return if we're not holding the vaccinator.
  if(!wep->is_vaccinator())
    return;
  
  if(!global->createmove_force_attack2)
    return;

  // If we have no valid prefered resist type then return.
  if(global->auto_vaccinator_prefer_resist < 0 || global->auto_vaccinator_prefer_resist > 2)
    return;

  set_wanted_resist(global->auto_vaccinator_prefer_resist);
  if(is_wanted_cycle(global->auto_vaccinator_prefer_resist)){
    global->current_cmd->buttons |= IN_ATTACK2;
    global->createmove_force_attack2 = false;
  }
}

bool c_auto_vaccinator::is_hitscan_weapon(i32 weaponid){
  switch(weaponid){
    default: return false;
    case TF_WEAPON_SHOTGUN_PRIMARY:
    case TF_WEAPON_SHOTGUN_SOLDIER:
    case TF_WEAPON_SHOTGUN_HWG:
    case TF_WEAPON_SHOTGUN_PYRO:
    case TF_WEAPON_SCATTERGUN:
    case TF_WEAPON_SNIPERRIFLE:
    case TF_WEAPON_MINIGUN:
    case TF_WEAPON_SMG:
    case TF_WEAPON_PISTOL:
    case TF_WEAPON_PISTOL_SCOUT:
    case TF_WEAPON_REVOLVER:
    case TF_WEAPON_SNIPERRIFLE_CLASSIC:
    case TF_WEAPON_CHARGED_SMG:
    case TF_WEAPON_PEP_BRAWLER_BLASTER:
    case TF_WEAPON_HANDGUN_SCOUT_PRIMARY:
    case TF_WEAPON_HANDGUN_SCOUT_SECONDARY:
      return true;
  }

  return false;
}

i32 c_auto_vaccinator::get_weapon_resist_type(c_base_weapon* weapon){
  if(weapon == nullptr)
    return -1;

  // This weapon shoots through the vaccinator.
  if(weapon->is_enforcer())
    return -1;

  if(weapon->deals_bullet_dmg())
    return 0;
  else if(weapon->deals_blast_dmg())
    return 1;
  else if(weapon->deals_fire_dmg())
    return 2;

  return -1;
}

i32 c_auto_vaccinator::calc_uber_cost(c_base_player* protect){
  if(config->automation.auto_vaccinator_passive)
    return AUTO_VACCINATOR_MAGIC_THREAT_VALUE;

  float cost = AUTO_VACCINATOR_MAX2CHARGE;
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return (i32)cost;

  // Check our details!
  {
    if(localplayer->is_vulnerable())
      cost *= 0.90f;

    float hp = math::clamp(localplayer->get_health_percent(), 0.f, 100.f);
    if(hp > 0.0f)
      cost *= (hp / 100.f);
  }

  //Check our heal target details!
  if(protect != nullptr){
    if(protect->is_vulnerable())
      cost *= 0.90f;

    float hp = math::clamp(protect->get_health_percent(), 0.f, 100.f);
    if(hp > 0.0f)
      cost *= (hp / 100.f);
  }

  return math::clamp((i32)cost, 12, AUTO_VACCINATOR_MAX2CHARGE);
}

// Runs prediction for swapping resist type on the client
void c_auto_vaccinator::medigun_itempostframe_run(c_base_player* player, c_base_weapon* wep, bool reload, i32 buttons){
  if(player == nullptr || wep == nullptr)
    return;

  if(!global->prediction->first_time_predicted)
    return;

  if(player->get_index() != global->localplayer_index || !wep->is_vaccinator())
    return;

  if(player->has_condition(TF_COND_TAUNTING))
    return;

  // If we have died, reset this to 0
  // TODO: check if we have another weapon and then reset it -senator
  if(global->resist_reset){
    global->resist_reset      = false;
    global->resist_predicted  = -1; // Allow the server to set the next value until we predict again
    global->resist_networked  = -1;
    wep->charge_resist_type()   = 0;
    c_auto_vaccinator::set_wanted_resist(-1);
  }

  float time = math::time();
  if(global->resist_check_prediction_time <= time && global->resist_networked != global->resist_predicted && global->resist_networked > -1){
    DBG("[-] charge resist type prediction error! received: %i vs predicted: %i\n", global->resist_networked, global->resist_predicted);
    wep->charge_resist_type() = global->resist_predicted = global->resist_networked;
  }

  // If we press reload, respect it and predict the value on the client
  if(buttons & IN_RELOAD && !reload){
    wep->charge_resist_type()              = (wep->charge_resist_type() + 1) % 3;
    global->resist_predicted             = wep->charge_resist_type();
    global->resist_check_prediction_time = time + utils::get_command_latency();

    packet_manager->force_send_packet(true);
  }
}

// If we have an update come through on the server, its going to be delayed from our last prediction by our ping
// So since we predict it, ignore completely what the server does with it and restore it back to our predicted value
void c_auto_vaccinator::int_decode_run(c_base_entity* entity, i32* data){
  if(data == nullptr)
    return;

  if(entity == nullptr)
    return;

  c_base_entity* owner = entity->get_owner();
  if(owner == nullptr)
    return;

  if(owner->get_index() != global->localplayer_index)
    return;

  if(global->resist_predicted >= 0){
    global->resist_networked = *data;
    *data = global->resist_predicted;
  }
}