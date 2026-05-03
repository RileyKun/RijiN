#include "../link.h"

CLASS_ALLOC(c_auto_sticky, auto_sticky);

bool c_auto_sticky::run(){
  if(!is_active(true))
    return false;
  
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  // The "get_weapon_from_belt" function is gay.
  c_base_weapon* weapon = localplayer->get_weapon_from_belt(1, false);
  if(weapon == nullptr)
    return false;

  if(!weapon->is_sticky_launcher())
    return false;

  // It does no damage.
  if(weapon->weapon_id() == WPN_StickyJumper){
    global->info_panel->add_entry(INFO_PANEL_AUTO_STICKY, WXOR(L"INVALID WEAPON"), INFO_PANEL_WARNING_CLR);
    return false;
  }

  if(weapon->pipebomb_count() <= 0){
    global->info_panel->add_entry(INFO_PANEL_AUTO_STICKY, WXOR(L"NO STICKIES PLACED"), INFO_PANEL_WARNING_CLR);
    return false;
  }

  if(weapon->weapon_id() == WPN_ScottishResistance){
    if(localplayer->will_fire_this_tick()){
      global->info_panel->add_entry(INFO_PANEL_AUTO_STICKY,  WXOR(L"LAUNCHING STICKIES"), INFO_PANEL_WARNING_CLR);
      return false;
    }
  }

  bool is_playing_mvm = utils::is_playing_mvm();

  global->info_panel->add_entry(INFO_PANEL_AUTO_STICKY,is_playing_mvm ? WXOR(L"MVM MODE RUNNING") : WXOR(L"RUNNING"));

  // Blow up our stickies!
  bool det = false;
  vec3 aim_point;

  i32 stickies        = 0;
  bool target_found   = false;
  bool will_self_harm = false;
  float ping          = utils::get_latency();

  bool is_scottish_resistance = weapon->weapon_id() == WPN_ScottishResistance;

  // !! Pretty sure the game has a table of all of our stickies. Should look into that at some point.
  for(i32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* sticky = global->entity_list->get_entity(i);
    if(sticky == nullptr)
      continue;

    if(sticky->is_dormant())
      continue;

    if(sticky->team() != localplayer->team())
      continue;

    if(!is_local_sticky(sticky))
      continue;

    if(!is_sticky_armed(sticky, weapon))
      continue;

    if(is_scottish_resistance){
      s_sticky_entry* e = sticky_list.add_entry(sticky);

      if(e != nullptr && math::time() <= e->time)
        continue;
    }

    c_base_weapon* launcher = sticky->get_launcher();
    if(launcher == nullptr)
      continue;

    vec3 sticky_pos = sticky->obb_center();

    // Before doing any vischecks and damage calculation code the game does this to the position of the explosion.
    {
      sticky_pos.z += 8.f;
      s_trace tr1 = global->trace->ray(sticky_pos, sticky_pos + vec3(0.f, 0.f, -32.f), mask_shot_hull, TRACE_EVERYTHING, sticky);
      if(tr1.fraction != 1.0f)
        sticky_pos = tr1.end + (tr1.plane.normal * 1.f);
    }

    // We're gonna use this to see if this sticky would move out of the way before our command reaches the server.
    vec3 predicted_sticky_pos = utils::trace_line(sticky_pos, sticky_pos + sticky->est_velocity() * ping, 0);

    float radius     = weapon->get_blast_radius();

    radius *= (get_blast_radius_ratio() / 100.f);


    float radius_sqr = (radius * radius);
    assert(radius > 0.0f);

    // They changed the in air radius to fix a game bug...
    if(!sticky->is_sticky_onsurface()){
      radius     = weapon->get_blast_radius(sticky->sticky_creation_time(), sticky->pipebomb_touched());
      radius_sqr = radius * radius;
    }

    // Detect to see if we'll can damage ourself with this sticky.
    if(localplayer->obb_center().distance(sticky_pos) <= radius * 2.15f){
      s_trace tr     = global->trace->ray(sticky_pos, localplayer->obb_center(), mask_shot_hull, TRACE_EVERYTHING, sticky);
      if(tr.vis(localplayer))
        will_self_harm = true;
    }

    c_entity_sphere_query sphere(sticky_pos, radius); // Used by the game to find targets.

    for(i32 j = 0; j < sphere.list_count; j++){
      c_base_entity* entity = sphere.list[j];
      if(entity == nullptr)
        continue;

      if(entity->is_dormant())
        continue;

      if(entity->get_index() != global->localplayer_index || entity->get_index() == global->localplayer_index && !config->automation.auto_sticky_prevent_self_dmg){
        if(!is_target_entity(entity, localplayer, weapon))
          continue;
      }

      // Get the nearest point on the entity's bounding box. The game uses this position for whether or not the entity takes damage.
      vec3 nearest_point;
      vec3 old_obb_mins = entity->obb_mins();
      vec3 old_obb_maxs = entity->obb_maxs();

      // The nearest point is based on how the bounding box is rotated.
      // So with latency, fake angles, and other things that will likely cause it to be inaccurate..
      // Where couldn't actually deal damage to do being too far or some other cause. And this is a big deal for TF2 cheats!!!!!!
      entity->set_min_and_maxs(entity->obb_mins() * 0.9f, entity->obb_maxs() * 0.9f);
      entity->calc_nearest_point(sticky_pos, &nearest_point);
      entity->set_min_and_maxs(old_obb_mins, old_obb_maxs);

      float dist_sqr           = (sticky_pos - nearest_point).length_sqr();
      float predicted_dist_sqr = (predicted_sticky_pos - nearest_point).length_sqr();

      if(dist_sqr > radius_sqr || !sticky->is_sticky_onsurface() && predicted_dist_sqr > radius_sqr)
        continue;

      if(!is_visible(localplayer, entity, sticky, sticky_pos, nearest_point))
        continue;

      aim_point    = sticky_pos;

      if(is_scottish_resistance){
        global->current_cmd->view_angles = math::calc_view_angle(localplayer->shoot_pos(), aim_point);

        s_sticky_entry* e = sticky_list.get_entry(sticky);
        if(e != nullptr)
          e->time = math::time() + utils::get_latency();

        global->current_cmd->buttons |= IN_ATTACK2;
        packet_manager->set_choke(true);
        return true;
      }
      else{
        det          = true;
        target_found = true;
      }
    }

    if(target_found){
      stickies++;
      target_found = false;
    }
  }

  // Do we wanna prevent self harm?
  if(config->automation.auto_sticky_prevent_self_dmg && will_self_harm){
    global->info_panel->add_entry(INFO_PANEL_AUTO_STICKY, WXOR(L"PREVENTING SELF DAMAGE"), INFO_PANEL_WARNING_CLR);
    return false;
  }

  if(det){
    if(weapon->pipebomb_count() < config->automation.auto_sticky_min_bomb_count || stickies >= config->automation.auto_sticky_min_bomb_count){
      if(is_scottish_resistance){
        global->current_cmd->view_angles = math::calc_view_angle(localplayer->shoot_pos(), aim_point);
        packet_manager->set_choke(true);
      }
      else
        packet_manager->force_send_packet(false);

      global->current_cmd->buttons |= IN_ATTACK2;     
      return is_scottish_resistance;
    }
  }

  return false;
}

bool c_auto_sticky::is_active(bool update_info){
  if(!config->automation.auto_sticky_detonate)
    return false;
  
  static c_key_control key(&config->automation.auto_sticky_detonate_key, &config->automation.auto_sticky_detonate_key_held, &config->automation.auto_sticky_detonate_key_toggle, &config->automation.auto_sticky_detonate_key_double_click);
  if(config->automation.auto_sticky_detonate_use_key){
    if(!key.is_toggled()){
      if(update_info)
        global->info_panel->add_entry(INFO_PANEL_AUTO_STICKY, WXOR(L"KEY NOT PRESSED"));

      return false;
    }
  }

  return true;
}

bool c_auto_sticky::is_visible(c_base_player* localplayer, c_base_entity* entity, c_base_entity* sticky, vec3 sticky_pos, vec3 pos){
  if(localplayer == nullptr || entity == nullptr)
    return false;

  // We have to see the center of the bounding no matter what.
  s_trace tr = global->trace->ray(sticky_pos, entity->obb_center(), mask_explosion, TRACE_EVERYTHING, sticky);
  if(!tr.vis(entity))
    return false;

  tr = global->trace->ray(sticky_pos, pos, mask_explosion, TRACE_EVERYTHING, sticky);
  if(!tr.vis(entity))
    return false;

  return true;
}

bool c_auto_sticky::is_target_entity(c_base_entity* entity, c_base_player* localplayer, c_base_weapon* weapon){
  if(entity == nullptr || localplayer == nullptr || weapon == nullptr)
    return false;

  if(entity->team() == localplayer->team())
    return false;

  if(entity->is_player() && !utils::is_truce_active()){

    c_base_player* p = (c_base_player*)entity;
    if(!p->is_alive())
      return false;

    if(p->is_bonked() || p->is_ubercharged())
      return false;

    if(p->is_steam_friend() && !config->automation.auto_sticky_target_friend)
      return false;

    if(p->is_cloaked() && !config->automation.auto_sticky_target_cloaked)
      return false;

    if(utils::is_playing_mvm()){
      if(p->healers() > 0 && !p->is_player_class(TF_CLASS_MEDIC)) // We don't want a case where a medic bot heals another medic bot.
        return false;
    }

    return true;
  }
  else{

    if(!utils::is_truce_active()){
      if(entity->is_sentry() && config->automation.auto_sticky_target_sentry)
        return true;

      if(entity->is_dispenser() && config->automation.auto_sticky_target_dispenser)
        return true;

      if(entity->is_teleporter() && config->automation.auto_sticky_target_teleporter)
        return true;

       // Some sticky launchers can destroy other stickies.
      if(entity->is_stickybomb() && config->automation.auto_sticky_target_sticky){

        // Only these sticky launchers can destroy stickies.
        if(weapon->weapon_id() != WPN_ScottishResistance && weapon->weapon_id() != WPN_QuickieBombLauncher)
          return false;

        // Can only be destroyed while on the ground. (Pretty sure)
        if(!entity->is_sticky_onsurface())
          return false;

        return true;
      }
    }

    if(entity->is_boss() && config->automation.auto_sticky_target_boss)
      return true;
  }

  return false;
}

// Used to avoid attempting to blow up stickies that fly near players but can't be detonated.
bool c_auto_sticky::is_sticky_armed(c_base_entity* entity, c_base_weapon* weapon){
  if(entity == nullptr || weapon == nullptr)
    return false;

  if(!entity->is_stickybomb())
    return false;

  // Predicted cur time will attempt to detonate our stickies when we actually can't...
  return (global->old_global_data.cur_time - entity->sticky_creation_time()) >= weapon->get_sticky_launcher_arm_time();
}

bool c_auto_sticky::is_local_sticky(c_base_entity* entity){
  if(entity == nullptr)
    return false;

  if(!entity->is_stickybomb())
    return false;

  c_base_weapon* launcher = entity->get_launcher();
  if(launcher == nullptr)
    return false;

  c_base_player* owner = launcher->get_owner();
  if(!owner->is_player())
    return false;

  return owner->get_index() == global->engine->get_local_player();
}

float c_auto_sticky::get_blast_radius_ratio(){
  float radius = math::clamp(config->automation.auto_sticky_blast_radius, 10.f, 100.f);

  // Better for dealing with mvm robots.
  if(utils::is_playing_mvm())
    radius = math::smallest(radius, 65.f);
  
  return radius;
}