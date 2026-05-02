#include "../link.h"

CLASS_ALLOC(c_auto_medigun_shield, auto_shield);

void c_auto_medigun_shield::run(){
  if(!config->automation.auto_shield)
    return;

  if(!utils::is_playing_mvm())
    return;

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

  if(!weapon->is_medigun())
    return;

  global->info_panel->add_entry(INFO_PANEL_AUTO_MEDIC_SHIELD, WXOR(L"INACTIVE"));
  if(global->current_cmd->buttons & IN_ATTACK)
    return;

  // Save the FPS!!! BRO!!!!
  // If we don't have enough charge and we aren't using it then return.
  if(localplayer->rage_meter() < 100.f && !localplayer->rage_draining())
    return;

  global->info_panel->add_entry(INFO_PANEL_AUTO_MEDIC_SHIELD, WXOR(L"ACTIVE"));

  // Our shield is currently not active.
  if(!config->automation.auto_shield_auto_deploy && !localplayer->rage_draining()){
    global->info_panel->add_entry(INFO_PANEL_AUTO_MEDIC_SHIELD, WXOR(L"MANUALLY DEPLOY"), INFO_PANEL_WARNING_CLR);
    return;
  }

  vec3 best_angle;
  u32  total_entities = 0;
  bool best_cover_target_valid = found_best_cover_target(localplayer, best_angle, total_entities);
  if(!best_cover_target_valid) // This is required.
    return;

  // Fill in the info panel related to auto-deploy.
  if(config->automation.auto_shield_auto_deploy && !localplayer->rage_draining()){
    if(total_entities >= config->automation.auto_shield_deploy_at_count)
      global->info_panel->add_entry(INFO_PANEL_AUTO_MEDIC_SHIELD_AUTO_DEPLOY, WXOR(L"DEPLOYING"), INFO_PANEL_LEGIT_CLR);
    else{
      if(!config->automation.auto_shield_deploy_at_count) // If we somehow get down here then we have it handled.
        global->info_panel->add_entry(INFO_PANEL_AUTO_MEDIC_SHIELD_AUTO_DEPLOY, WXOR(L"SEARCHING FOR ENTITIES"));
      else{
        wchar_t buf[IDEAL_MIN_BUF_SIZE];
        formatW(buf, WXOR(L"NEED %i ENTITIES"), config->automation.auto_shield_deploy_at_count - total_entities);
        global->info_panel->add_entry(INFO_PANEL_AUTO_MEDIC_SHIELD_AUTO_DEPLOY, buf, INFO_PANEL_WARNING_CLR);
      }
    }
  }

  if(found_target_near_by(localplayer, best_angle)){
    if(config->automation.auto_shield_auto_deploy && total_entities >= config->automation.auto_shield_deploy_at_count){
      if(!localplayer->rage_draining())
        global->current_cmd->buttons |= IN_ATTACK3;
    }

    global->info_panel->add_entry(INFO_PANEL_AUTO_MEDIC_SHIELD, WXOR(L"NEAR BY MODE"), INFO_PANEL_LEGIT_CLR);
    global->current_cmd->view_angles.y = best_angle.y;
  }
  else {
    if(config->automation.auto_shield_auto_deploy && total_entities >= config->automation.auto_shield_deploy_at_count){
      if(!localplayer->rage_draining())
        global->current_cmd->buttons |= IN_ATTACK3;
    }

    global->info_panel->add_entry(INFO_PANEL_AUTO_MEDIC_SHIELD, WXOR(L"BEST COVER MODE"), INFO_PANEL_LEGIT_CLR);
    global->current_cmd->view_angles.y = best_angle.y;
  }

  // DEBUG
  {
    vec3 pos;
    if(get_predicted_shield_position(vec3(0.f, global->current_cmd->view_angles.y, 0.f), pos))
      render_debug->draw_3dbox(pos, vec3(-12.f, -12.f, -12.f), vec3(12.f, 12.f, 12.f), vec3(0.f, global->current_cmd->view_angles.y, 0.f), colour(0, 0, 0, 0), colour(255, 255, 255, 0), globaldata->interval_per_tick * 2.f);
  }
}

// find targets that are both to collide with our shield.
bool c_auto_medigun_shield::found_target_near_by(c_base_player* localplayer, vec3& best_angle){
  if(localplayer == nullptr)
    return false;

  bool  target_found = false;
  float closest_dist = -1.f;
  for(u32 i = 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* entity = global->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    if(!is_valid_entity(entity))
      continue;

    vec3 shield_position;
    vec3 angle = math::calc_view_angle(localplayer->shoot_pos(), entity->obb_center());
    if(!get_predicted_shield_position(angle, shield_position)) // Get the shield position.
      continue;

    // Predict the position of the "incoming player" or projectile.
    vec3 predicted_pos = utils::trace_line(entity->obb_center(), entity->obb_center() + entity->est_velocity(), mask_bullet, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);

    float dist = predicted_pos.distance(shield_position);
    if(dist > 256.f)
      continue;

    if(dist > closest_dist && closest_dist != -1.f)
      continue;

    closest_dist = dist;
    best_angle   = angle;
    target_found = true;
  }

  return target_found;
}

// find the yaw angle that covers the most enemies.
bool c_auto_medigun_shield::found_best_cover_target(c_base_player* localplayer, vec3& best_angle, u32& total_entities){
  if(localplayer == nullptr)
    return false;

  bool target_found   = false;
  total_entities      = 0;
  for(u32 i = 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* entity = global->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    if(!is_valid_entity(entity))
      continue;

    // There is no point in processing this entity. Our shield will do no good and we'll just die from them.
    if(localplayer->origin().distance(entity->origin()) <= 128.f)
      continue;

    vec3 angle = math::calc_view_angle(localplayer->shoot_pos(), entity->obb_center());
    if(math::abs(angle.x) > 45.f) // The shield is locked at 0 pitch. So any entities past this point are invalid to us.
      continue;

    // Ensure the main entity visible. We don't care about threats that are behind cover.
    s_trace trace = global->trace->ray(localplayer->shoot_pos(), entity->obb_center(), mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
    if(!trace.vis(entity))
      continue;

    // If this is the first time we've found a target make sure we set our best angle!!
    if(!target_found){
      best_angle   = angle;
      target_found = true;
    }

    i32 entities = 0;
    for(u32 j = 1; j <= global->entity_list->get_highest_index(); j++){
      if(j == i)
        continue;

      c_base_entity* other_entity = global->entity_list->get_entity(j);
      if(other_entity == nullptr)
        continue;

      if(!is_valid_entity(other_entity))
        continue;

      // Only check to see if robots/sentries are visible. This is way to heavy on performance otherwise.
      if(other_entity->is_player() || other_entity->is_sentry()){
        trace = global->trace->ray(localplayer->shoot_pos(), other_entity->obb_center(), mask_bullet, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
        if(!trace.vis(other_entity))
          continue;
      }
      else{
        // Any projectile this far away doesn't need to be counted for.
        if(localplayer->origin().distance(other_entity->origin()) > 12512.f)
          continue;
      }

      // We've created an angle towards the "main entity" we're checking if this entity is with in 45 degrees.
      if(math::get_fov_delta(angle, localplayer->shoot_pos(), other_entity->obb_center()) > 45.f)
        continue;

      entities++;
    }

    if(entities >= total_entities){
      best_angle     = angle;
      total_entities = entities;
    }
  }

  return target_found;
}

bool c_auto_medigun_shield::is_valid_entity(c_base_entity* entity){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  if(entity == nullptr)
    return false;

  if(entity->is_dormant())
    return false;

  if(localplayer->team() == entity->team())
    return false;

  if(entity->is_player()){
    c_base_player* player = (c_base_player*)entity;
    if(!player->is_valid())
      return false;

    // The robots are coming to us they won't attack until then.
    if(player->is_ubercharge_hidden())
      return false;

    c_base_weapon* weapon = player->get_weapon();
    if(weapon == nullptr)
      return false;

    // The shield won't protect from these robots holding these weapons. Flames go through the shield.
    if(weapon->is_melee() || weapon->is_harmless_item() || weapon->is_flamethrower() || weapon->is_medigun())
      return false;

    return true;
  }
  else{
    if(entity->is_stickybomb()){
      if(entity->is_sticky_onsurface())
        return false;

      return true;
    }
    else if(entity->is_projectile() && !entity->is_fake_projectile()){
      vec3 pos = entity->origin();
      vec3 vel = entity->est_velocity();

      // This projectile is really slow. Could be bugged or a grenade on the floor.
      if(vel.length_2d() <= 20.f)
        return false;

      float latency = math::clamp(utils::get_latency(), globaldata->interval_per_tick, 0.5f);

      // The projectile is moving away from us so it ignore it. (Avoid aiming at projectiles that went past us)
      if(localplayer->origin().distance(pos) < localplayer->origin().distance(pos + (vel * latency)))
        return false;

      return true;
    }
    else if(entity->is_sentry() && localplayer->origin().distance_meter(entity->origin()) <= 27)
      return true;
  }

  return false;
}

// rebuilt CTFMedigunShield::UpdateShieldPosition
bool c_auto_medigun_shield::get_predicted_shield_position(vec3 eye_angles, vec3& pos){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  vec3 fwd;
  math::angle_2_vector(eye_angles, &fwd, nullptr, nullptr);
  fwd.z = 0.f;

  // The game doesn't care if the shield is inside a wall.
  pos = localplayer->origin() + fwd * 145.f;
  return true;
}