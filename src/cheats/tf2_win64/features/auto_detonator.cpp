#include "../link.h"

CLASS_ALLOC(c_auto_detonator, auto_detonator);

void c_auto_detonator::run(){
  if(!config->automation.auto_detonator)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  if(weapon->weapon_id() != WPN_Detonator)
    return;

  global->info_panel->add_entry(INFO_PANEL_AUTO_DETONATOR, WXOR(L"RUNNING"));

  // Blow up our flare!!
  bool  det        = false;
  float ping       = math::clamp(utils::get_latency(), globaldata->interval_per_tick, 0.4f) / 2.f;
  float radius     = weapon->get_blast_radius();
  float radius_sqr = (radius * radius) * 0.85f;
  for(i32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* flare = global->entity_list->get_entity(i);
    if(flare == nullptr)
      continue;

    if(flare->is_dormant())
      continue;

    if(!flare->is_flare())
      continue;

    if(flare->team() != localplayer->team())
      continue;

    if(!is_local_flare(flare))
      continue;

     vec3 predicted_pos = utils::trace_line(flare->obb_center(), flare->obb_center() + (flare->est_velocity() * ping));
     c_entity_sphere_query sphere(predicted_pos, radius);

     for(i32 i = 0; i < sphere.list_count; i++){
        c_base_entity* entity = sphere.list[i];
        if(entity == nullptr)
          continue;

        if(entity->is_dormant())
          continue;

        if(!is_target_entity(entity, localplayer))
          continue;

        vec3 pos;
        entity->calc_nearest_point(predicted_pos, &pos);

        float dist = (predicted_pos - pos).length_sqr();
        if(dist > radius_sqr)
          continue;

        s_trace tr = global->trace->ray(predicted_pos, entity->obb_center(), mask_solid_brushonly);
        if(!tr.vis(entity))
          continue;

        det = true;
     }
  }

  if(det){
    global->current_cmd->buttons |= IN_ATTACK2;
    packet_manager->force_send_packet(false);
  }
}

bool c_auto_detonator::is_target_entity(c_base_entity* entity, c_base_player* localplayer){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  if(entity->team() == localplayer->team())
    return false;

  if(entity->is_player() && !utils::is_truce_active()){

    c_base_player* p = (c_base_player*)entity;
    if(!p->is_alive())
      return false;

    if(p->is_bonked() || p->is_ubercharged())
      return false;

    if(p->is_steam_friend() && !config->automation.auto_detonator_friends)
      return false;

    if(p->is_cloaked() && !config->automation.auto_detonator_cloaked)
      return false;

    return true;
  }
  else{

    if(!utils::is_truce_active()){
      if(entity->is_sentry() && config->automation.auto_detonator_sentry)
        return true;

      if(entity->is_dispenser() && config->automation.auto_detonator_dispenser)
        return true;

      if(entity->is_teleporter() && config->automation.auto_detonator_teleporter)
        return true;

      if(entity->is_stickybomb() && config->automation.auto_detonator_sticky){

        if(!entity->is_sticky_onsurface())
          return false;

        return true;
      }
    }

    if(entity->is_boss() && config->automation.auto_detonator_boss)
      return true;
  }

  return false;
}

bool c_auto_detonator::is_local_flare(c_base_entity* entity){
  if(entity == nullptr)
    return false;

  c_base_entity* owner = entity->get_owner();
  if(owner == nullptr)
    return false;

  if(owner->get_index() != global->engine->get_local_player())
    return false;

  return true;
}