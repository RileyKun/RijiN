#include "../link.h"

bool c_aimbot_filter_raytrace_fov::handle(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  float current_distance = localplayer->origin().distance(entity->origin());
  if(entity->is_player() && record != nullptr)
    current_distance = localplayer->origin().distance(record->origin);

  if(max_range > 0.f && current_distance > max_range)
    return false;

  return true;
};

bool c_aimbot_filter_raytrace_fov::store(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr || global->aimbot_settings == nullptr)
    return false;

  vec3 new_aim_point;
  if(!raytrace_aimbot->get_best_aim_point(entity, record, new_aim_point))
    return false;

  if(!handle_priority(record, entity))
    return false;

  float current_fov = math::get_pixel_fov_delta(new_aim_point);
  if(target_ent != nullptr && current_fov > closest_fov)
    return false;

  // I had every entity behind me but one, and it always prefered to target the entities behind instead of the one in my crosshair.
  // If you don't set this, then this will become sorted by entity index rather than FOV.
  // This doesn't break anything with the FOV circle or the pixel fov above.
  closest_fov = current_fov;

  aim_point  = new_aim_point;
  target_ent = entity;
  lc_record  = record;

  return true;
};

bool c_aimbot_filter_raytrace_distance::handle(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  current_distance = localplayer->origin().distance(entity->origin());
  if(entity->is_player() && record != nullptr)
    current_distance = localplayer->origin().distance(record->origin);

  if(target_ent != nullptr && current_distance >= closest_distance || max_range > 0.f && current_distance > max_range)
    return false;

  return true;
};

bool c_aimbot_filter_raytrace_distance::store(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr || global->aimbot_settings == nullptr)
    return false;

  vec3 new_aim_point;
  if(!raytrace_aimbot->get_best_aim_point(entity, record, new_aim_point))
    return false;

  if(!handle_priority(record, entity))
    return false;

  aim_point  = new_aim_point;
  target_ent = entity;
  lc_record  = record;

  closest_distance = current_distance;

  return true;
};

bool c_aimbot_filter_projectile_fov::handle(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  float current_distance = localplayer->origin().distance(entity->origin());

  if(max_range > 0.f && current_distance > max_range)
    return false;

  return true;
};

bool c_aimbot_filter_projectile_fov::store(s_lc_record* record, c_base_entity* entity){
  if(global->aimbot_settings == nullptr)
    return false;

  if(entity == nullptr || localplayer == nullptr || global->aimbot_settings == nullptr)
    return false;

  s_current_target_data* proj_ctx = (s_current_target_data*)record;
  if(!projectile_aimbot->get_best_aim_point(proj_ctx))
    return false;

  if(!projectile_aimbot->is_projectile_trajectory_visible(proj_ctx))
    return false;

  if(!handle_priority(record, entity))
    return false;

  // FOV logic.
  float current_fov = math::get_pixel_fov_delta(proj_ctx->aim_point);
  if(target_ent != nullptr && current_fov > closest_fov)
    return false;

  closest_fov = current_fov;

  aim_point   = proj_ctx->target_aim_point;
  target_ent  = entity;

  return true;
};

bool c_aimbot_filter_projectile_distance::handle(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  current_distance = localplayer->origin().distance(entity->origin());

  if(target_ent != nullptr && current_distance >= closest_distance || max_range > 0.f && current_distance > max_range)
    return false;

  return true;
};

bool c_aimbot_filter_projectile_distance::store(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr || global->aimbot_settings == nullptr)
    return false;

  s_current_target_data* proj_ctx = (s_current_target_data*)record;
  if(!projectile_aimbot->get_best_aim_point(proj_ctx))
    return false;

  if(!projectile_aimbot->is_projectile_trajectory_visible(proj_ctx))
    return false;

  if(!handle_priority(record, entity))
    return false;

  aim_point        = proj_ctx->target_aim_point;
  target_ent       = entity;
  closest_distance = current_distance;

  return true;
};

bool c_aimbot_filter_melee_fov::handle(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  vec3 origin = entity->origin();
  if(record != nullptr)
    origin = record->origin;

  float current_distance = localplayer->origin().distance(origin);
  if(current_distance > max_range)
    return false;

  return true;
};

bool c_aimbot_filter_melee_fov::store(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr || global->aimbot_settings == nullptr)
    return false;

  vec3 new_aim_point;
  if(!melee_aimbot->get_best_aim_point(entity, record, new_aim_point))
    return false;

  if(!handle_priority(record, entity))
    return false;

  float current_fov = math::get_pixel_fov_delta(new_aim_point);
  if(target_ent != nullptr && current_fov > closest_fov)
    return false;

  closest_fov = current_fov;

  // Basically if we have the whip out, always target enemies over friendlies if we are able to melee one
  // We wouldnt want to be whipping teammates when we clearly want to kill enemies
  if(config->automation.auto_soldier_whip && target_ent != nullptr && entity != nullptr){
    if(localplayer != nullptr && localplayer->is_player_class(TF_CLASS_SOLDIER)){
      c_base_weapon* weapon = localplayer->get_weapon();

      if(weapon != nullptr && weapon->is_melee() && weapon->is_soldier_whip() && localplayer->team() != target_ent->team() && localplayer->team() == entity->team())
        return false;
    }
  }

  aim_point  = new_aim_point;
  target_ent = entity;
  lc_record  = record;

  return true;
};

bool c_aimbot_filter_medigun::handle(s_lc_record* record, c_base_player* player){
  if(player == nullptr || localplayer == nullptr)
    return false;


  if(player->is_player()){

    // There is a fixed range on the medic guns.
    float current_distance = localplayer->origin().distance(record->origin);
    if(localplayer_wep->get_heal_target() != player){
      if(current_distance > 470.f)
        return false;
    }
    else{
      if(current_distance > 570.f)
        return false;
    }

    // Can't heal cloaked spies.
    if(player->is_cloaked())
      return false;

    if(global->aimbot_settings->auto_medigun_friendsonly){
      if(!player->is_steam_friend())
        return false;
    }
  }
  else{
    c_base_entity* entity = (c_base_entity*)player;
    if(!entity->is_revive_marker())
      return false;

    float current_distance = localplayer->origin().distance(entity->origin());
    if(current_distance > max_range)
      return false;
  }

  return true;
};

bool c_aimbot_filter_medigun::store(s_lc_record* record, c_base_player* player){
  if(player == nullptr || localplayer == nullptr || global->aimbot_settings == nullptr)
    return false;

  i32 hp_loss = 0;
  if(player->is_player()){
    hp_loss = math::biggest(0, player->max_health() - player->health());
    if(medigun_aimbot->heal_type == 1 || medigun_aimbot->heal_type == 2){
      float max_buffed_health = (float)player->get_max_buffed_health();

      // Apply overheal expert modifier.
      float overheal_expert = localplayer_wep->get_overheal_expert_mod();
      if(overheal_expert > 0.f)
        max_buffed_health += (float)(player->max_health() * overheal_expert);

      if(localplayer_wep->weapon_id() == WPN_QuickFix)
        max_buffed_health *= 0.83333f;

      hp_loss = math::biggest(0, (i32)max_buffed_health - player->health());
    }

    if(global->aimbot_settings->auto_medigun_preferfriends || (global->aimbot_settings->auto_medigun_preferfriends_except_on_key && !input_system->held(global->aimbot_settings->auto_medigun_preferfriends_except_on_key_hotkey))){
      if(player->is_steam_friend())
        hp_loss += 500;
    }
  }
  else{
    hp_loss = 1000; // At this point we only aim at revive markers. So give this entity a 1000 HP loss.
    medigun_aimbot->heal_type = 0; // Over-ride.
  }

  i32 p = medigun_aimbot->get_priorty(target);
  if(p < best_priority)
    return false;

  if(target != nullptr && hp_loss <= most_lost_hp)
    return false;

  vec3 new_aim_point;
  if(!medigun_aimbot->get_best_aim_point(player, record, new_aim_point))
    return false;

  // I don't know if this is a good idea.
  //if(!handle_priority(record, entity))
  //  return false;

  aim_point     = new_aim_point;
  target        = player;
  lc_record     = record;
  best_priority = p;

  most_lost_hp  = hp_loss;
  return true;
};

bool c_aimbot_filter_autorepair::handle(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr)
    return false;

  // We can't repair or speed up the building progress of this object.
  if(entity->disposable_building())
    return false;

  // Do we have enough metal to repair anything?
  // Update: Make sure the builing isn't being sapped.
  if(localplayer->get_ammo_count(3) <= 0 && !(entity->sapped() || entity->building() || entity->placing()))
    return false;

  return true;
};

bool c_aimbot_filter_autorepair::store(s_lc_record* record, c_base_entity* entity){
  if(entity == nullptr || localplayer == nullptr || global->aimbot_settings == nullptr)
    return false;

  vec3 new_aim_point;
  if(!auto_repair_aimbot->get_best_aim_point(entity, new_aim_point))
    return false;

  i32 hp_loss = math::biggest(0, entity->object_max_health() - entity->object_health());

  // This was jsut an easy way to get the wrench to target buildings in these states.
  if(!auto_repair_aimbot->rescue_ranger_mode){
    if(entity->sapped() || entity->placing() || entity->building())
      hp_loss += entity->object_max_health();
  }
  else{
    // We can't repair a sentry being sapped and don't consider buildings being constructed as damaged.
     if(entity->sapped() || entity->placing() || entity->building())
       hp_loss = 0;
  }

  if(most_lost_hp <= 0 && hp_loss <= 0 && !auto_repair_aimbot->rescue_ranger_mode){
    // The sentry does not need any ammo / rockets.
    if(entity->is_sentry()){
      if(entity->shells() >= entity->max_shells() && entity->rockets() >= 20 && entity->upgrade_level() == 3)
        return false;
      else if(entity->shells() >= entity->max_shells() && entity->mini_building())
        return false;

      goto jmp_sentry_ammo_check;
    }

    // No upgrading left todo!
    if(entity->upgrade_level() == 3)
      return false;
  }
  else{
    if(hp_loss < most_lost_hp) // Prefer the most lost HP.
      return false;

    if(auto_repair_aimbot->rescue_ranger_mode && hp_loss == 0)
      return false;

    most_lost_hp = hp_loss;
  }

  jmp_sentry_ammo_check:
  aim_point   = new_aim_point;
  target_ent  = entity;

  return true;
};