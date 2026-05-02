#include "../link.h"

#if defined(DEV_MODE) || defined(STAGING_MODE)
  #define DEBUG_PROJECTILE_AIMBOT
#endif

bool c_projectile_aimbot::should_predict_entity(s_current_target_data* proj_ctx){
  if(proj_ctx->entity == nullptr)
    return false;

  // This is meant for as of right now for gravity weapons.
  if(!proj_ctx->wep->is_sticky_launcher())
    return true;

  vec3  obb_center    = proj_ctx->entity->obb_center();
  float dist_to_local = proj_ctx->localplayer->shoot_pos().distance_meter(obb_center);

  i32 max_distance = proj_ctx->wep->is_quickie_bomb_launcher() ? 32 : 28;

  float current_charge = math::clamp((globaldata->cur_time - proj_ctx->wep->charge_begin_time()), 0.f, proj_ctx->wep->get_sticky_launcher_max_charge());
  if(proj_ctx->wep->charge_begin_time() > 0.0f)
    max_distance = (i32)(float)(max_distance * (1.f + current_charge));
  
  // We're going to assume at our current AOA the entity isn't going to be hittable. Lets ignore them.
  return dist_to_local <= max_distance;
}

bool c_projectile_aimbot::performs_trajectory_cast(s_current_target_data* proj_ctx){
  if(proj_ctx == nullptr)
    return false;

  if(proj_ctx->wep->is_wrangler() || proj_ctx->wep->is_flamethrower() || proj_ctx->wep->is_dragons_fury())
    return false;
    
  return true;
}

bool c_projectile_aimbot::aimbot_only_mode_active(c_base_player* player){
  if(global->aimbot_settings == nullptr)
    return false;

  if(player == nullptr)
    return false;

  c_player_data* data = player->data();
  if(data == nullptr)
    return false;

  if(global->aimbot_settings->aimbot_only_enabled){
    if(!global->aimbot_settings->aimbot_only_use_key || input_system->held(global->aimbot_settings->aimbot_only_key)){

      bool fail = true;
      if(global->aimbot_settings->aimbot_only_prioritized && (data->playerlist.priority_mode && data->playerlist.priority > 0))
        fail = false;
      else if(global->aimbot_settings->aimbot_only_cheaters && cheat_detection->is_cheating(player->get_index()))
        fail = false;
      else if(global->aimbot_settings->aimbot_only_bots && cheat_detection->is_cheating_bot(player->get_index()))
        fail = false;

      return fail;
    }
  }

  return false;
}

bool c_projectile_aimbot::should_target_entity(c_base_entity* entity){
  if(global->aimbot_settings == nullptr)
    return false;

  if(entity == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return false;

  bool truce_active = utils::is_truce_active();
  if(entity->is_player()){
    c_base_player* player = entity->get_player();
    if(!player->is_valid())
      return false;

    if(auto_heal_arrow->force_enable_heal_arrow()){
      if(player->get_index() != auto_heal_arrow->target_index_lock)
        return false;
    }

    bool bypass_bonk_check       = false;
    bool bypass_ubercharge_check = false;
    bool bypass_truce_check      = false;

    if(wep->is_crossbow() && localplayer->team() == player->team()){
      bypass_bonk_check  = true;
      bypass_truce_check = true;
    }
    else if(wep->is_loose_cannon()){
      if(global->aimbot_settings->loose_cannon_target_ubercharged)
        bypass_ubercharge_check = true;

      if(global->aimbot_settings->loose_cannon_target_bonked)
        bypass_bonk_check = true;
    }

    if(heal_arrow_team_mates_only){
      // Only apply this logic on our team-mates.
      if(player->team() == localplayer->team()){
        // Only target friendly players that are damaged.
        if(player->health()>= player->max_health())
          return false;

        // Only target our steam friends.
        if(global->aimbot_settings->heal_arrow_friendsonly && !player->is_steam_friend())
          return false;
      }
      else{

        // Don't target enemies if this is true.
        if(global->aimbot_settings->heal_arrow_priority_team){
          if(!global->aimbot_settings->heal_arrow_priority_team_inview && !global->aimbot_settings->heal_arrow_priority_team_any)
            global->aimbot_settings->heal_arrow_priority_team_inview = true;

          if(global->aimbot_settings->heal_arrow_priority_team_any || global->aimbot_settings->heal_arrow_priority_team_inview && heal_arrow_team_mates_within_view)
            return false;
        }

        // We're only targeting a certain player in this case.
        if(aimbot_only_mode_active(player))
          return false;

        // -- Readded check, because people got butthurt that this was disabled. Cannot make up their minds.
        if(!localplayer->is_considered_enemy(player))
          return false;

        return true; // Probably shouldn't target enemies in this case.
      }
    }
    else{

      // We're only targeting a certain player in this case.
      if(aimbot_only_mode_active(player))
        return false;

      if(!localplayer->is_considered_enemy(player))
        return false;
    }

    if(player->is_bonked() && !bypass_bonk_check || player->is_ubercharged() && !bypass_ubercharge_check || truce_active && !bypass_truce_check)
      return false;

    return true;
  }
  else{
    if(wep->is_throwables())
      return false;

    if(entity->is_dormant())
      return false;

    if(entity->team() == localplayer->team())
      return false;

    // We're running our heal arrow logic. Ignore objects for now.
    if(heal_arrow_team_mates_only)
      return false;

    // Once we can predict movement on non players we can re-enable this.
    //if(entity->is_boss() && global->aimbot_settings->target_boss)
    //  return true;

    // This happens in the special halloween maps. We can no longer damage any enemy buildings or stickies.
    if(truce_active)
      return false;

    if(entity->is_sentry() && global->aimbot_settings->target_sentries || entity->is_dispenser() && global->aimbot_settings->target_dispenser || entity->is_teleporter() && global->aimbot_settings->target_teleporter)
      return entity->object_health() > 0;
  }

  return false;
}

bool c_projectile_aimbot::crusaders_crossbow_logic_enabled(){
  if(auto_heal_arrow->force_enable_heal_arrow())
    return true;

  if(!global->aimbot_settings->heal_arrow_enabled)
    return false;

  if(global->aimbot_settings->heal_arrow_use_key){
    if(!input_system->held(global->aimbot_settings->heal_arrow_key))
      return false;
  }

  return true;
}

void c_projectile_aimbot::crusaders_crossbow_logic(){
  heal_arrow_team_mates_only        = false;
  heal_arrow_team_mates_within_view = false;
  if(!crusaders_crossbow_logic_enabled())
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  if(!weapon->is_crossbow())
    return;

  {
    for(i32 index = 1; index <= globaldata->max_clients; index++){
      if(index == global->localplayer_index)
        continue;

      c_base_player* player = global->entity_list->get_entity(index);
      if(player == nullptr)
        continue;

      if(!player->is_valid())
        continue;

      if(player->team() != localplayer->team())
        continue;

      if(!player->is_steam_friend() && global->aimbot_settings->heal_arrow_friendsonly)
        continue;

      float dist = localplayer->origin().distance(player->origin());
      if(dist > 4800.f)
        continue;

      // Don't count this entity if they aren't with in our max travel time.
      // Obvious this one doesn't use the correct shoot position. But we aren't using this for aiming at players so it doesn't matter.
      float t = 0.f;
      if(!calc_travel_time(localplayer->shoot_pos(), player->obb_center(), weapon->get_projectile_speed(), t))
        continue;

      if(t > get_max_travel_time())
        continue;

      // Are they with in the heal arrow FOV Limit?
      float fov = math::get_pixel_fov_delta(player->obb_center());
      if(global->aimbot_settings->heal_arrow_max_fov > 0.f && global->aimbot_settings->heal_arrow_max_fov < 180.f){
        if(fov > global->aimbot_settings->heal_arrow_max_fov)
          continue;
      }

      if(player->health()>= player->max_health())
        continue;

      heal_arrow_team_mates_only = true;
      if(fov < 90.f)
        heal_arrow_team_mates_within_view = true;

      break;
    }
  }
}

float c_projectile_aimbot::get_max_travel_time(){
  cookie_block_check_return_val(0.75f);
  assert(global->aimbot_settings != nullptr);

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return 0.25f;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return 0.25f;

  float max_travel_time = math::abs(global->aimbot_settings->projectile_prediction_max_travel_time);
  if(max_travel_time < 250.f)
    return 0.25f;

  float speed = wep->get_projectile_speed();

  max_travel_time /= 1000.f;
  switch(localplayer->player_class()){
    default:
    {
      max_travel_time = math::clamp(max_travel_time, 0.25f, 2.5f);
      break;
    }
    case TF_CLASS_DEMOMAN:
    {
      if(wep->is_grenade_launcher())
        max_travel_time = math::clamp(max_travel_time, 0.25f, 1.25f);
      else if(wep->is_loose_cannon())
        max_travel_time = math::clamp(max_travel_time, 0.25f, 0.75f);
      else{
        max_travel_time = math::clamp(max_travel_time, 0.25f, 2.5f);
      }

      break;
    }
  }

  // The grenade has a life of around 2.0 seconds.
  if(wep->is_grenade_launcher() && max_travel_time > 1.75f)
    max_travel_time = 1.75f;
  else if(wep->is_loose_cannon() && max_travel_time > 1.f - math::ticks_to_time(2)) // Has a max fuse of 1 second.
    max_travel_time = 1.f - math::ticks_to_time(2);
  else if(wep->is_jarate_jar() || wep->is_mad_milk_jar() || wep->is_gas_jar())
    max_travel_time = 0.75f;
  else if(wep->is_cleaver()) // Originally I was going to make this 0.75, but I decided to make it 1 second. I kept the code the same for a reason.
    max_travel_time = 1.f;
  

  return max_travel_time;
}

float c_projectile_aimbot::get_min_hit_chance(float travel_time){
  assert(global->aimbot_settings != nullptr);

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return 75.f;

  float hit_chance = math::clamp(global->aimbot_settings->projectile_prediction_min_hit_chance, 1.f, 100.f) / 100.f;
  switch(localplayer->player_class()){
    default:
    {
      if(localplayer->player_class() == TF_CLASS_MEDIC && travel_time < 1.f)
        break;

      // I've tested this against fakelagging players. It doesn't have any issues.
      // I'm going to force people to use this for the time being. Since I don't see why it would be bad to scale hit-chance up based on distance.
      if(global->aimbot_settings->aim_mode_automatic){
        float ratio = travel_time / 2.5f;
        hit_chance = math::smallest(0.1f, math::biggest((ratio * 100) / 100.f, hit_chance));
      }     

      break;
    }
  }

  return hit_chance;
}

bool c_projectile_aimbot::calc_travel_time(vec3 shoot_pos, vec3 aim_point, float v, float& time){
  // calculate t=(d/v)
  float d = shoot_pos.distance(aim_point);
  if(v == 0.f || d == 0.f)
    return false;
  
  time = math::clamp(d / v, 0.f, 10.f);
  if(time > get_max_travel_time())
    return false;

  return true;
}

float c_projectile_aimbot::get_splash_bot_blast_radius(c_base_weapon* wep){
  if(wep == nullptr)
    return 0.f;

  if(wep->is_direct_hit())
    return 0.f;

  float blast_radius = wep->get_blast_radius();
  if(wep->is_sticky_launcher()){
    if(auto_sticky->is_active())
      blast_radius *= (auto_sticky->get_blast_radius_ratio() / 100.f);
  }

  return blast_radius;
}

bool c_projectile_aimbot::get_correct_shoot_pos_info(s_current_target_data* proj_ctx, bool skip_trajectory_vis_check, std::string str){
  if(proj_ctx->wep == nullptr)
    return false;

  if(!skip_trajectory_vis_check){
    if(proj_ctx->performed_trajectory_vis_check){
      //DBG("[-] c_projectile_aimbot::get_correct_shoot_pos_info call not allowed (%s)\n", str.c_str());
      return false;
    }
  }

  vec3 weapon_offset;
  if(!proj_ctx->wep->get_weapon_offset(weapon_offset)){ // This means the weapon doesn't have an offset we need to compensate for.
    proj_ctx->proj_shoot_pos = proj_ctx->shoot_pos;
    proj_ctx->aim_angle      = math::calc_view_angle(proj_ctx->proj_shoot_pos, proj_ctx->target_aim_point);
    return true;
  }

  vec3 new_target_pos = proj_ctx->target_aim_point - vec3(0.f, 0.f, proj_ctx->proj_max.z * 2.f);

  if(proj_ctx->wep->is_pomson())
    new_target_pos.z += 13.f;
  
  proj_ctx->proj_shoot_pos = proj_ctx->shoot_pos;
  proj_ctx->aim_angle      = math::calc_view_angle(proj_ctx->proj_shoot_pos, new_target_pos);

  vec3 fwd, right, up;
  bool vphysics = (proj_ctx->wep->is_grenade_launcher() || proj_ctx->wep->is_loose_cannon() || proj_ctx->wep->is_sticky_launcher() || proj_ctx->wep->is_throwables() || proj_ctx->wep->is_cleaver() || proj_ctx->wep->is_ball_bat());

  if(!vphysics){
    vec3 old_angle = proj_ctx->localplayer->last_viewangle();
    proj_ctx->localplayer->last_viewangle() = proj_ctx->aim_angle;
    proj_ctx->wep->get_projectile_fire_setup(proj_ctx->localplayer, weapon_offset, &proj_ctx->proj_shoot_pos, &proj_ctx->proj_angle);
    proj_ctx->localplayer->last_viewangle() = old_angle;
  }
  else{
    math::angle_2_vector(proj_ctx->aim_angle, &fwd, &right, &up);

    s_trace tr = global->trace->ray_obb(proj_ctx->shoot_pos, proj_ctx->shoot_pos + (fwd * weapon_offset.x) + (right * weapon_offset.y) + (up * weapon_offset.z), vec3(-8.f, -8.f, -8.f), vec3(8.f, 8.f, 8.f), mask_solid_brushonly, TRACE_EVERYTHING, nullptr, PROJ_TRACE_FILTER);
    if(tr.started_solid)
      return false;

    // This is how the game setups the pill launcher shoot position.
    proj_ctx->proj_shoot_pos = tr.end;
    proj_ctx->proj_angle     = math::calc_view_angle(proj_ctx->proj_shoot_pos, new_target_pos);
  }

  vec3 delta = math::clamp_angles((proj_ctx->proj_angle - proj_ctx->aim_angle));
  if(!vphysics)
    delta.y  = 0.f;

  delta.z    = 0.f;
  proj_ctx->aim_angle += delta;

  return true;
}

NEVERINLINE bool c_projectile_aimbot::calc_gravity_offset(s_current_target_data* proj_ctx, float* travel_time = nullptr){
  bool        success       = false;
  const vec3  shoot_pos     = proj_ctx->shoot_pos;
  const vec3  dir_to_target = (proj_ctx->target_aim_point - shoot_pos);
  const float len_sqr       = dir_to_target.length_sqr();

  vec3        best_point      = proj_ctx->target_aim_point; // Logging the best position.
  vec3        new_aim_point   = proj_ctx->target_aim_point; // Used for calculating gravity offset.
  i32         samples         = 0; // Tracking the max samples we can collect.

  float       last_offset_len = 0.f;

  // Used for the projectile simulation context.
  vec3        aim_angle       = math::calc_view_angle(shoot_pos, proj_ctx->target_aim_point);
  float       dist            = shoot_pos.distance(proj_ctx->target_aim_point);
  bool        vphysics        = proj_simulate->is_vphysics_weapon(proj_ctx->wep);

  // no idea what the heck todo with this yet. but we gonna turn off the XY offset when close to a target.
  bool dont_xy_offset = dist <= 192.f || shoot_pos.distance_meter(proj_ctx->target_aim_point) <= 6;

  s_projectile_simulation_context ctx;
  while(true && samples <= 32){
    ctx = s_projectile_simulation_context(proj_ctx->wep, aim_angle, false, false, proj_ctx->wep->is_throwables());
    if(!ctx.valid){
      assert(false && "Why did this occur? Did you forget to setup projectile simulation context?");
      return false;
    }

    samples++;
    for(i32 i = 0; i <= math::time_to_ticks(3.f); i++){
      const vec3  current_dir = (ctx.predicted_origin - shoot_pos);
      const float dot         = current_dir.dot(dir_to_target);

      if(!proj_simulate->predict_tick(&ctx) || dot >= len_sqr)
        break;
    }

    proj_ctx->highest_simulated_pos = ctx.highest_origin;

    const vec3 offset = (ctx.predicted_origin - proj_ctx->target_aim_point);
    float cur_offset  = math::abs(offset.length());

    // If we get the same offset then break, there is no samples left to receive.
    if((i32)cur_offset == (i32)last_offset_len)
      break;

    // If the cur offset is somehow bigger than the last one. Then we're moving away from the target somehow.
    if(cur_offset > last_offset_len && last_offset_len > 0.0f)
      break;

    last_offset_len  = cur_offset;

    if(!dont_xy_offset){
      for(i32 i = 0; i <= 1; i++)
        new_aim_point[i] -= offset[i];
    }

    new_aim_point.z -= offset.z;
    aim_angle        = math::calc_view_angle(shoot_pos, new_aim_point); // Never check the pitch angle for optimization tricks.

    // There really should be a better way at detecting a good point...
    if(cur_offset <= 100.f){
      if(travel_time != nullptr)
        calc_travel_time(shoot_pos, ctx.predicted_origin, ctx.predicted_velocity.length(), *travel_time);

      best_point = new_aim_point;
      success    = true;
    }
  }

  proj_ctx->target_aim_point = best_point;
  return success;
}

bool c_projectile_aimbot::calc_aim_offset(s_current_target_data* proj_ctx){
  if(proj_ctx->entity == nullptr || proj_ctx->wep == nullptr)
    return false;

  c_base_player* player = proj_ctx->entity->get_player();
  vec3 obb_size = proj_ctx->entity->obb_mins() + proj_ctx->entity->obb_maxs();

  vec3 top    = obb_size * 0.75f;
  vec3 center = obb_size * 0.5f;
  vec3 bottom = obb_size * 0.1f;

  if(proj_ctx->localplayer->entity_flags() & FL_DUCKING)
    bottom = obb_size * 0.35f;

   // A default of aiming at the center of the entity.
  proj_ctx->aim_offset    = obb_size * 0.5f;

  bool vphysics = proj_ctx->wep->is_grenade_launcher() || proj_ctx->wep->is_loose_cannon() || proj_ctx->wep->is_sticky_launcher();

  if(proj_ctx->wep->is_grenade_launcher() || proj_ctx->wep->is_loose_cannon()){
    bottom     = obb_size * 0.35f;
    proj_ctx->aim_offset = bottom; // The prediction will always be off by 1 - 3 ticks due to imperfections with drag calculations. This will for the most part stop it from hitting the ground just barely hitting the player.
    if(player != nullptr){
      if(!(player->entity_flags() & FL_ONGROUND))
        proj_ctx->aim_offset = center;
    }
  }
  else if(proj_ctx->wep->is_sticky_launcher()){
    bottom = vec3();
    proj_ctx->aim_offset = bottom;
  }
  else if(proj_ctx->wep->is_scorch_shot() || proj_ctx->wep->is_detonator()){
    bottom = vec3();
    proj_ctx->aim_offset = bottom;
    if(player != nullptr){
      if(!(player->entity_flags() & FL_ONGROUND))
        proj_ctx->aim_offset = center;
    }
  }
  else if(proj_ctx->wep->is_rocket_launcher() || proj_ctx->wep->is_scorch_shot() || proj_ctx->wep->is_detonator()){
    proj_ctx->aim_offset = bottom * 0.5f;
    if(player != nullptr){
      if(!(player->entity_flags() & FL_ONGROUND))
        proj_ctx->aim_offset = center;
    }
  }
  else if(proj_ctx->wep->is_dragons_fury()){
    proj_ctx->aim_offset = center;
    if(player != nullptr){
      if(player->entity_flags() & FL_ONGROUND) // Launch people in the air when they're on fire.
        proj_ctx->aim_offset = player->is_on_fire() ? bottom : center;
    }
  }
  else if(proj_ctx->wep->is_huntsman()){
    proj_ctx->aim_offset = player != nullptr ? (obb_size * 0.9f) : top;
    if(player == nullptr)
      proj_ctx->aim_offset = center;
  }
  else if(proj_ctx->wep->is_jarate_jar() || proj_ctx->wep->is_gas_jar() || proj_ctx->wep->is_mad_milk_jar()){
    proj_ctx->aim_offset = bottom;
  }
  else if(proj_ctx->wep->is_cleaver()){
    proj_ctx->aim_offset = center;
  }
  else
    proj_ctx->aim_offset = center;

  if(proj_ctx->wep->get_projectile_gravity() > 0.f && !proj_ctx->wep->is_huntsman()){
    if(player != nullptr){
      if(player->entity_flags() & FL_ONGROUND){
        // Same code for vphysics. Didn't use a goto cause we might wanna change it for flares or whatever.
        float height = math::abs(proj_ctx->entity->origin().z - proj_ctx->localplayer->origin().z);
        if(proj_ctx->entity->origin().z > proj_ctx->localplayer->origin().z){
          if(height >= 64.f)
            proj_ctx->aim_offset = center;

          if(height >= 128.f)
            proj_ctx->aim_offset = top;
        }
      }
    }
  }

  if(global->aimbot_settings->obb_head){
    proj_ctx->aim_offset = top;
  }
  else if(global->aimbot_settings->obb_body){
    proj_ctx->aim_offset = center;
  }
  else if(global->aimbot_settings->obb_feet){
    proj_ctx->aim_offset = bottom;
    if(proj_ctx->wep->is_syringe_gun()) // Game's pasted and needles hit ammo packs.
      proj_ctx->aim_offset = center * 0.5f;
  }

  // Only run this on non players.
  if(player == nullptr){
    if(proj_ctx->in_water){
      if(proj_ctx->wep->is_huntsman() || proj_ctx->wep->is_crossbow() || proj_ctx->wep->is_rescue_ranger()){
        proj_ctx->aim_offset = bottom;
      }
    }
  }

  return true;
}

bool c_projectile_aimbot::predict_projectile(s_entity_predict_ctx* ctx, s_current_target_data* proj_ctx){
  if(ctx == nullptr || ctx->predicted_entity == nullptr || ctx->prediction_broken)
    return false;

  c_player_data* data = ctx->predicted_entity->data();
  if(ctx->predicted_entity->velocity().length() <= 0.0f && ctx->predicted_entity->entity_flags() & FL_ONGROUND){
    if(data != nullptr){
      data->pred_flags     = 0;
      data->pred_real_time = globaldata->interval_per_tick;
      data->pred_track_array.reset();
    }
    
    ctx->predicted_ticks       = 1;
    ctx->has_been_predicted    = true; 


    //ctx->predicted_origin -= vec3(0.f, 0.f, 4.f); // why?
    proj_ctx->target_aim_point = ctx->predicted_origin + proj_ctx->aim_offset;
    return true;
  }

  float v = proj_ctx->proj_speed;

  // calculate our heuristic time
  i32 ticks_to_predict = math::time_to_ticks(proj_ctx->travel_time);
  if(ticks_to_predict < 1)
    ticks_to_predict = 1;

  // calculate our additional ping delay
  float ping_delay = utils::get_latency();

  // account for the fact we are sending our packet next tick
  if(global->aimbot_settings->silent_aim_serverside)
    ping_delay += math::ticks_to_time(1);

  // to correct the prediction since our t=(d/v) calculation is just an estimation,
  // we are going to add an extra second of prediction to the heuristic
  ticks_to_predict += math::time_to_ticks(1.f);

  // fetch the offset of our wish aim point
  vec3 aim_offset = proj_ctx->target_aim_point - ctx->predicted_origin;

  float smallest_delta = 0.f;
  float real_time      = 0.f;
  vec3  correct_origin;

  if(entity_prediction->is_cached(proj_ctx->entity)){
    s_entity_predict_cache_ctx* pred_cache = entity_prediction->get_cache(proj_ctx->entity);
    if(pred_cache != nullptr){
      memcpy(ctx, &pred_cache->ctx, sizeof(s_entity_predict_ctx)); 

      correct_origin = pred_cache->correct_origin;
      real_time      = pred_cache->real_time;
    }
  }
  else{

    if(data != nullptr && !ctx->has_been_predicted)
      data->pred_track_array.reset();

    for(u32 i = 0; i < ticks_to_predict; i++){
      if(!entity_prediction->predict_tick(ctx))
        return false;
      
      // add prediction track record
      if(data != nullptr)
        data->pred_track_array.add(ctx->predicted_origin);
  
      // solve the correct time to predict by taking our goal time and matching it against our predicted time
      {
        vec3 wish_correct_origin = ctx->predicted_origin + aim_offset;
  
        float simulation_time       = math::ticks_to_time(i);
        float goal_simulation_time  = math::ticks_to_time(math::time_to_ticks((proj_ctx->proj_shoot_pos.distance(wish_correct_origin) / v) + ping_delay));
        float simulation_time_delta = math::ticks_to_time(math::time_to_ticks(math::abs(goal_simulation_time - simulation_time)));
  
        if(i == 0 || simulation_time_delta < smallest_delta){
          smallest_delta = simulation_time_delta;
          correct_origin = wish_correct_origin;
          real_time      = goal_simulation_time - ping_delay;
        }
        else if(simulation_time >= goal_simulation_time)
          break;
      }
    }
   
    entity_prediction->store_cache(ctx, proj_ctx->entity, real_time, correct_origin);
  }

  // Correct the prediction track size to the actual predicted ticks
  data->pred_track_array.size = math::clamp(ctx->predicted_ticks - 1, 0, MAX_PRED_TRACK_ENTRIES);

  // Store the real time for this prediction
  data->pred_real_time = real_time;

  data->pred_flags = ctx->flags;

  proj_ctx->target_aim_point = correct_origin;
  return true;
}

bool c_projectile_aimbot::post_predict_projectile(s_entity_predict_ctx* ctx, s_current_target_data* proj_ctx){
  if(ctx == nullptr || ctx->predicted_entity == nullptr || ctx->prediction_broken)
    return false;

  if(!ctx->predicted_entity->is_player())
    return true;

  if(proj_ctx->proj_gravity <= 0.0f)
    return true;

  c_base_player* target_player = ctx->predicted_entity->get_player();
  c_player_data* player_data   = target_player->data();
  if(player_data == nullptr)
    return false;

  // This player is not moving, there for our special projectile should hit them without issue.
  if(target_player->velocity().length() <= 0.0f && target_player->entity_flags() & FL_ONGROUND)
    return true;


  // Post-prediction, the projectiles slowing down due to the curve of the trajectory and the drag is not.
  // The code below attempts to compare the original trajectory time to target against our simulated pill -
  // trajectory time to target at the predicted position and compensates by predicting the player the required -
  // ticks forward to compensate for the drag and curve time added on by the gravity solver.
  // -senator
  if(ctx->has_been_predicted){
    // Compares corrected real travel time (via solve) against original travel_time (t=d/v) to figure out if the player is coming towards us
    bool  towards                 = (player_data->pred_real_time - proj_ctx->travel_time) < 0.f;
          proj_ctx->travel_time   = player_data->pred_real_time;
    float new_travel_time         = 0.f;

    // Simulate projectile hitting end position and return the travel time taken for the pill to reach that point
    if(calc_gravity_offset(proj_ctx, &new_travel_time)){
      float travel_time_delta = (new_travel_time - proj_ctx->travel_time);

      // When the player is coming towards us (by comparing corrected travel time and travel time)
      // The player is walking towards us so the time delta is solved %50 in every situation
      // The delta check is intented to avoid unwanted issues
      if(towards){
        travel_time_delta *= 0.5f;
        if(math::abs(travel_time_delta) >= 0.5f){
          //DBG("[!] travel_time_delta too large (%f) max (0.5)\n", travel_time_delta);
          return false;
        }
      }

      i32 ticks_to_adjust   = math::time_to_ticks(math::abs(travel_time_delta));
          ticks_to_adjust  *= (travel_time_delta < 0.f ? -1 : 1);

      if(ticks_to_adjust > 0){
        //DBG("[!] adjusting ticks: %i\n", ticks_to_adjust);
        //DBG("[!] %s ticks_to_adjust: %i, travel_time_delta: %f, \t new_travel_time: %f, \t travel_time: %f\n", target_player->info().name, ticks_to_adjust, travel_time_delta, new_travel_time, proj_ctx->travel_time);

        float predicted_time_budget = math::abs(math::ticks_to_time(ctx->predicted_ticks + ticks_to_adjust) - 1.f); // Remove the sample time.
        if(predicted_time_budget >= get_max_travel_time())
          return false;
        
        float smallest_delta = 0.f;
        for(i32 i = 0; i < ticks_to_adjust; i++){
          if(!entity_prediction->predict_tick(ctx))
            return false;

          player_data->pred_track_array.add(ctx->predicted_origin);

          float simulation_time       = math::ticks_to_time(i);
          float goal_simulation_time  = math::ticks_to_time(math::time_to_ticks(math::abs(travel_time_delta)));
          float simulation_time_delta = math::ticks_to_time(math::time_to_ticks(math::abs(goal_simulation_time - simulation_time)));

          if(i == 0 || simulation_time_delta < smallest_delta)
            smallest_delta = simulation_time_delta;
          else if(simulation_time >= goal_simulation_time){
            ticks_to_adjust = i;
            break;
          }
        }

        // Fix pred track array size
        player_data->pred_track_array.size = math::clamp(player_data->pred_track_array.size - 1, 0, MAX_PRED_TRACK_ENTRIES);
      }

      // Update the aim point and travel time to correct values
      proj_ctx->target_aim_point  = ctx->predicted_origin + proj_ctx->aim_offset;
      proj_ctx->travel_time       = new_travel_time;
    }
  }
  else
    proj_ctx->travel_time = player_data->pred_real_time;

  return true;
}

bool c_projectile_aimbot::predict_evasion(s_current_target_data* proj_ctx, bool simulate = false){
  if(!global->aimbot_settings->predict_evasion)
    return false;
  
  const u32 max_trigger = 20;
  if(proj_ctx->entity == nullptr)
    return false;

  if(!proj_ctx->entity->is_player())
    return false;

  c_base_player* player = proj_ctx->entity->get_player();
  if(player == nullptr)
    return false;

  if(proj_ctx->localplayer != nullptr){
    if(player->origin().distance_meter(proj_ctx->localplayer->origin()) <= 6)
      return false;
  }

  s_lc_data*     lc_data  = lag_compensation->get_data(player);
  if(lc_data == nullptr)
    return false;

  c_player_data* data     = player->data();
  if(data == nullptr)
    return false;

  data->bypass_hit_chance = false;


  vec3 origin       = player->origin();
  u32  travel_ticks = math::smallest(lc_data->max_records, math::biggest(2, math::time_to_ticks(proj_ctx->travel_time)));

  s_lc_record* oldest_record = lc_data->get_record(travel_ticks - 1);
  if(oldest_record == nullptr)
    return false;

  float time        = math::time();
  u32   samples     = 0;
  float aoa_diff    = 0.f;
  vec3  avg_origin;
  for(u32 i = 1; i < travel_ticks; i++){
    s_lc_record* cur_record = lc_data->get_record(i);
    s_lc_record* last_record = lc_data->get_record(i - 1);
    if(cur_record == nullptr || last_record == nullptr)
      continue;

    vec3 prev_aoa, cur_aoa;
    math::vector_2_angle(&cur_record->velocity, &cur_aoa);
    math::vector_2_angle(&last_record->velocity, &prev_aoa);
    cur_aoa.x = prev_aoa.x = 0.f;

    aoa_diff   += (cur_aoa - prev_aoa).length();
    avg_origin += cur_record->origin;
    samples++;
  }

  if(!samples)
    return false;
  
  avg_origin /= (float)samples;
  aoa_diff /= (float)samples;
  if(aoa_diff < 8.f){
    if(data->evasion_triggers > 0 && data->evasion_last_sim_time <= player->simulation_time())
      data->evasion_triggers--;

    data->evasion_last_sim_time      = player->simulation_time();
    if(data->evasion_triggers < max_trigger || data->evasion_trigger_reset_time <= time)
      return false;
  }

  if(data->evasion_trigger_reset_time <= time)
    data->evasion_triggers = 0;

  if(data->evasion_last_sim_time <= player->simulation_time() && aoa_diff > 5.f){
    data->evasion_trigger_reset_time = time + 1.f;
    data->evasion_triggers++;
  }

  data->evasion_last_sim_time      = player->simulation_time();
  if(data->evasion_triggers < max_trigger)
    return false;

  if((oldest_record->origin - origin).length() > 48.f)
    return false;

  if(simulate)
    return true;

  data->bypass_hit_chance    = true;
  proj_ctx->aim_point        = avg_origin + proj_ctx->aim_offset;
  proj_ctx->target_aim_point = proj_ctx->aim_point;
  if(!get_correct_shoot_pos_info(proj_ctx, false, XOR("c_projectile_aimbot::predict_evasion")))
    return false;

  #if defined(DEV_MODE) || defined(STAGING_MODE)
    const float show_time = 1.f;
    //render_debug->draw_3dbox(avg_origin, proj_ctx->entity->obb_mins(), proj_ctx->entity->obb_maxs(), proj_ctx->entity->get_abs_angles(), colour(255, 255, 255, 64), colour(0, 0, 0, 64), show_time);
    render_debug->draw_text(avg_origin, colour(255, 255, 255, 255), XOR("predict_evasion"), 1, show_time);
  #endif
  
  return true;
}

bool c_projectile_aimbot::predict_splash_damage(s_current_target_data* proj_ctx){
  if(!global->aimbot_settings->splash_assist)
    return false; 

  if(!proj_ctx->wep->is_rocket_launcher() || proj_ctx->wep->is_direct_hit())
    return false;

  // Only run on players that are in the air.
  if(proj_ctx->entity->entity_flags() & FL_ONGROUND)
    return false;

  s_trace tr = global->trace->player_move(proj_ctx->predicted_pos, proj_ctx->predicted_pos - vec3(0.f, 0.f, 2000.f), proj_ctx->entity);

  vec3 original_aim_point        = proj_ctx->aim_point;
  vec3 original_target_aim_point = proj_ctx->target_aim_point;

  proj_ctx->aim_point        = tr.end;
  proj_ctx->target_aim_point = proj_ctx->aim_point;
  if(!get_correct_shoot_pos_info(proj_ctx, true, XOR("c_projectile_aimbot::predict_splash_damage")))
    return false;

  //(vec3 pos, colour face_col, i8* text, i32 offset, float life_time)
  global->skip_trajectory_check = false;
  if(is_projectile_trajectory_point_visible(proj_ctx, PROJ_TRAJECTORY_SPLASH_BOT_TEST)){
    global->skip_trajectory_check = true;
    return true;
  }

  // Restore previous if we fail.
  proj_ctx->aim_point        = original_aim_point;
  proj_ctx->target_aim_point = original_target_aim_point;

  if(!get_correct_shoot_pos_info(proj_ctx, true, XOR("c_projectile_aimbot::predict_splash_damage2")))
    return false;


  return false;
}

bool c_projectile_aimbot::get_adjusted_aim_point(s_current_target_data* proj_ctx, s_trace* tr){
  if(proj_ctx == nullptr)
    return false;

  if(proj_ctx->entity == nullptr || proj_ctx->proj_gravity > 0.0f)
    return false;

  // This fucking sucks
  return false;

  vec3 old_abs_origin  = proj_ctx->entity->get_abs_origin();
  float max_height     = math::abs(proj_ctx->predicted_pos.z + (proj_ctx->entity->obb_mins().z + proj_ctx->entity->obb_maxs().z));
  float step_size      = max_height / 6.f;
  bool  exceeded_point = false;

  vec3 start_pos = vec3(proj_ctx->predicted_obb.x, proj_ctx->predicted_obb.y, proj_ctx->predicted_pos.z);
  proj_ctx->entity->set_abs_origin(proj_ctx->predicted_pos);
  for(float h = 0.f; h < max_height; h += step_size){
    proj_ctx->aim_point = start_pos + vec3(0.f, 0.f, h);
    if(proj_ctx->aim_point.z >= max_height){
      proj_ctx->aim_point.z = max_height;
      if(exceeded_point) // We've been here before.. Sadly, lets go ahead and fail this function.
        return false;

      exceeded_point = true;
    }

    proj_ctx->target_aim_point = proj_ctx->aim_point;
    if(!get_correct_shoot_pos_info(proj_ctx, true))
      continue;

    if(proj_ctx->proj_max.z != 0.0f){
      if(!is_projectile_trajectory_visible(proj_ctx, PROJ_TRAJECTORY_SKIP_OBB_SCAN))
        continue;
    }
    else{
      *tr = global->trace->ray(proj_ctx->shoot_pos, proj_ctx->target_aim_point, mask_bullet);
      if(tr->started_solid || !tr->vis(proj_ctx->entity))
        continue;

      if(found_sentry){
        *tr = global->trace->ray(sentry_shoot_pos, proj_ctx->target_aim_point, mask_bullet);
        if(tr->started_solid || !tr->vis(proj_ctx->entity))
          continue;
      }
    }


    proj_ctx->entity->set_abs_origin(old_abs_origin);
    return true;
  }

  proj_ctx->entity->set_abs_origin(old_abs_origin);
  return false;
}

bool c_projectile_aimbot::get_best_aim_point(s_current_target_data* proj_ctx){
  if(proj_ctx->entity == nullptr  || global->aimbot_settings == nullptr)
    return false;

  proj_ctx->aim_angle = vec3();
  proj_ctx->shoot_pos = proj_ctx->localplayer->shoot_pos();
  vec3 old_abs_origin = proj_ctx->entity->get_abs_origin();

  // Distance checks for certain weapons
  {
    if(proj_ctx->wep->is_dragons_fury()){
      if(proj_ctx->shoot_pos.distance_meter(proj_ctx->entity->obb_center()) > 13)
        return false;
    }
    else if(proj_ctx->wep->is_flamethrower()){
      if(proj_ctx->shoot_pos.distance_meter(proj_ctx->entity->obb_center()) >= 9)
        return false;
    }
  }

  // Since we don't wanna base it on the predicted origin, we can use this function.
  const float fov = utils::get_ideal_pixel_fov(proj_ctx->entity);

  // Are they in our fov?
  if(proj_ctx->wep->is_crossbow() && crusaders_crossbow_logic_enabled() && heal_arrow_team_mates_only){ // Heal arrows has it's own FOV limit.
    if(global->aimbot_settings->heal_arrow_max_fov > 0 && global->aimbot_settings->heal_arrow_max_fov < 180){
      if(fov > (float)global->aimbot_settings->heal_arrow_max_fov)
        return false;
    }
  }
  else{ // Do other projectile weapon FOV check.
    if(global->aimbot_settings->max_fov > 0 && global->aimbot_settings->max_fov < 180)
      if(fov > (float)global->aimbot_settings->max_fov)
        return false;
  }

  s_trace water_tr = global->trace->ray(proj_ctx->entity->obb_center(), proj_ctx->entity->obb_center(), mask_bullet | mask_water);
  proj_ctx->in_water = water_tr.contents & contents_water;

  if(!proj_ctx->wep->get_projectile_size(proj_ctx->proj_min, proj_ctx->proj_max))
    return false;

  proj_ctx->predicted_pos       = proj_ctx->entity->origin();
  proj_ctx->predicted_obb       = proj_ctx->entity->obb_center();
  bool  height_adjust = true;

  // Calculate aim offset
  if(!calc_aim_offset(proj_ctx))
    return false;
  
  if(proj_ctx->entity->is_player()){
    c_base_player* target_player = proj_ctx->entity->get_player();
    if(target_player == nullptr)
      return false;

    proj_ctx->aim_point        = (proj_ctx->entity->origin() + proj_ctx->aim_offset);
    proj_ctx->target_aim_point = proj_ctx->aim_point;
    if(!get_correct_shoot_pos_info(proj_ctx, false))
      return false;

    proj_ctx->proj_speed   = proj_ctx->wep->get_projectile_speed(proj_ctx->aim_angle);
    proj_ctx->proj_gravity = proj_ctx->wep->get_projectile_gravity();
    if(!calc_travel_time(proj_ctx->proj_shoot_pos, proj_ctx->target_aim_point, proj_ctx->proj_speed, proj_ctx->travel_time))
      return false;

    // The prediction isn't needed unless we want to support aimbotting bosses.
    s_entity_predict_ctx ctx(target_player, EP_FLAG_PREDICT_YAW_TURN);
    if(!predict_projectile(&ctx, proj_ctx))
      return false;

    if(!post_predict_projectile(&ctx, proj_ctx))
      return false;

    proj_ctx->predicted_pos = ctx.predicted_origin;
    proj_ctx->predicted_obb = proj_ctx->predicted_pos + (ctx.predicted_obb_mins + ctx.predicted_obb_maxs) * 0.5f;

    if(predict_evasion(proj_ctx)){
      proj_ctx->predicted_pos = proj_ctx->entity->origin();
      proj_ctx->predicted_obb = proj_ctx->entity->obb_center();
    }

    if(predict_splash_damage(proj_ctx))
      return true;
  }
  else{
    proj_ctx->target_aim_point = proj_ctx->entity->origin() + proj_ctx->aim_offset;

    // Use predict_projectile when it supports non players.
    {
      if(!get_correct_shoot_pos_info(proj_ctx, false))
        return false;

      proj_ctx->proj_speed   = proj_ctx->wep->get_projectile_speed(proj_ctx->aim_angle);
      proj_ctx->proj_gravity = proj_ctx->wep->get_projectile_gravity();
      if(!calc_travel_time(proj_ctx->proj_shoot_pos, proj_ctx->target_aim_point, proj_ctx->proj_speed, proj_ctx->travel_time))
        return false;
    }
  }

  if(!get_correct_shoot_pos_info(proj_ctx, false))
    return false;

  proj_ctx->obb_scan            = false;
  global->skip_trajectory_check = false;

  // If we're using the wrangler. Ensure the sentry can also see it.
  // On the server it will lock on to players once you "hit" their bounding box and it'll force the sentry to aim at the center of the bounding box.
  // See CObjectSentrygun::GetEnemyAimPosition for more information.
  // If m_hAutoAimTarget is set to any entity then the sentry will force lock on.
  if(found_sentry){
    proj_ctx->entity->set_abs_origin(proj_ctx->predicted_pos);
    s_trace tr  = global->trace->ray(sentry_shoot_pos, proj_ctx->predicted_obb, mask_bullet);
    s_trace tr2 = global->trace->ray(proj_ctx->shoot_pos, proj_ctx->predicted_obb, mask_bullet);

    if(tr.vis(proj_ctx->entity) && !tr.started_solid && tr2.vis(proj_ctx->entity) && !tr2.started_solid){
      proj_ctx->entity->set_abs_origin(old_abs_origin);
      return true;
    }

    if(get_adjusted_aim_point(proj_ctx, &tr)){
      proj_ctx->entity->set_abs_origin(old_abs_origin);
      return tr.vis(proj_ctx->entity) && !tr.started_solid;
    }

    proj_ctx->entity->set_abs_origin(old_abs_origin);
    return false;
  }
  else if(!performs_trajectory_cast(proj_ctx)){
    proj_ctx->entity->set_abs_origin(proj_ctx->predicted_pos);
    s_trace tr = global->trace->ray_obb(proj_ctx->shoot_pos, proj_ctx->target_aim_point, proj_ctx->proj_min, proj_ctx->proj_max, PROJ_TRACE_MASK, TRACE_EVERYTHING, proj_ctx->localplayer, PROJ_TRACE_FILTER);
    proj_ctx->entity->set_abs_origin(old_abs_origin);

    if(tr.vis(proj_ctx->entity) && !tr.started_solid)
      return true;
    
    return false;
  }

  if(proj_ctx->proj_gravity > 0.f){
    if(!calc_gravity_offset(proj_ctx))
      return false;

    return true;
  }

  proj_ctx->obb_scan = true;
  return true;
}

bool c_projectile_aimbot::is_projectile_trajectory_point_visible(s_current_target_data* proj_ctx, i32 flags){
  if(global->skip_trajectory_check || !performs_trajectory_cast(proj_ctx))
    return true;

  vec3 old_abs_origin = proj_ctx->entity->get_abs_origin();
  proj_ctx->entity->set_abs_origin(proj_ctx->predicted_pos);

  if(!get_correct_shoot_pos_info(proj_ctx, true))
    return false;

  proj_ctx->performed_trajectory_vis_check = true;
  s_projectile_simulation_context ctx(proj_ctx->wep, proj_ctx->aim_angle, true, proj_ctx->wep->is_throwables(), proj_ctx->wep->is_throwables(), true, proj_ctx->aim_point);
  ctx.single_step();
  for(u32 ticks = 1; ticks <= math::time_to_ticks(5.f); ticks++){
    if(!proj_simulate->predict_tick(&ctx))
      break;
  } 

  if(ctx.predicted_ticks <= 0){
    DBG("[-] c_projectile_aimbot::is_projectile_trajectory_point_visible: No ticks were predicted returning\n");
    return false;
  }

  // We don't check if the result is valid. We just continue on, like normal because we just want the position.

  proj_ctx->entity->set_abs_origin(old_abs_origin);
  return on_projectile_trajectory_hit(proj_ctx, &ctx.tr, flags);
}

bool c_projectile_aimbot::is_projectile_trajectory_visible(s_current_target_data* proj_ctx, i32 flags){
  if(global->skip_trajectory_check || !performs_trajectory_cast(proj_ctx))
    return true;

  vec3 old_abs_origin = proj_ctx->entity->get_abs_origin();
  proj_ctx->entity->set_abs_origin(proj_ctx->predicted_pos);

  if(!get_correct_shoot_pos_info(proj_ctx, true)){
    proj_ctx->entity->set_abs_origin(old_abs_origin);
    return false;
  }

  proj_ctx->performed_trajectory_vis_check = true;

  s_projectile_simulation_context ctx(proj_ctx->wep, proj_ctx->aim_angle, true, proj_ctx->wep->is_throwables(), proj_ctx->wep->is_throwables());
  ctx.single_step();

  for(u32 ticks = 1; ticks <= math::time_to_ticks(5.f); ticks++){
    if(!proj_simulate->predict_tick(&ctx))
      break;
  }

  if(!ctx.valid){
    proj_ctx->entity->set_abs_origin(old_abs_origin);
    return false;
  }

  if(ctx.tr.hit_entity != nullptr){
    proj_ctx->entity->set_abs_origin(old_abs_origin);
    return on_projectile_trajectory_hit(proj_ctx, &ctx.tr, flags);
  }
  else
    DBG("[-] is_projectile_trajectory_visible failed to hit target entity! Increase prediction size or review aim point!\n");

  proj_ctx->entity->set_abs_origin(old_abs_origin);
  return false;
}

bool c_projectile_aimbot::on_projectile_trajectory_hit(s_current_target_data* proj_ctx, s_trace* tr, i32 flags){
  if(tr == nullptr)
    return false;

  if(tr->started_solid)
    return false;

  // Sticky launcher hack done.
  float blast_radius     = get_splash_bot_blast_radius(proj_ctx->wep);
  if(flags & PROJ_TRAJECTORY_SPLASH_BOT_TEST){
    if(!utils::is_in_blast_range(proj_ctx->entity, proj_ctx->predicted_pos, tr->end, blast_radius)) // 04.3.2024 removed the (* 0.88) mod
      return false;

    u8 type = EXPLOSION_ROCKET;
    if(proj_ctx->wep->is_grenade_launcher() || proj_ctx->wep->is_sticky_launcher() || proj_ctx->wep->is_loose_cannon())
      type = EXPLOSION_DEMO;
    else if(proj_ctx->wep->is_throwables())
      type = EXPLOSION_THROWABLE;
    else if(type == proj_ctx->wep->is_scorch_shot())
      type = EXPLOSION_PYRO;

    vec3 pos;
    if(!utils::correct_position_for_blast(type, tr->end, proj_ctx->entity))
      return false;

    vec3 old_abs_origin = proj_ctx->entity->get_abs_origin();
    vec3 nearest_point;
    
    // We need to test against the predicted position.
    proj_ctx->entity->set_abs_origin(proj_ctx->predicted_pos);
    proj_ctx->entity->calc_nearest_point(tr->end, &nearest_point);


    //CTFRadiusDamageInfo::ApplyToEntity
    s_trace ex_tr = global->trace->ray(tr->end, proj_ctx->predicted_obb, mask_shot & ~(0x40000000), TRACE_EVERYTHING, nullptr, PROJ_TRACE_FILTER);
    if(ex_tr.started_solid && ex_tr.hit_entity != nullptr)
      ex_tr = global->trace->ray(tr->end, nearest_point, mask_shot & ~(0x40000000), TRACE_EVERYTHING, ex_tr.hit_entity, PROJ_TRACE_FILTER);

    if(!ex_tr.started_solid && ex_tr.vis(proj_ctx->entity)){
      ex_tr = global->trace->ray(tr->end, nearest_point, mask_shot & ~(0x40000000), TRACE_EVERYTHING, nullptr, PROJ_TRACE_FILTER);
      if(ex_tr.vis(proj_ctx->entity) && !ex_tr.started_solid){
        proj_ctx->entity->set_abs_origin(old_abs_origin);

        // This obviously won't account for the blast range difference.
        // Goal of this is to avoid positions that can offer cover against splash and thus make us do no damage.
        // So if our predicted position can see the target and the non predicted position can also. Then use that point instead.
        {
          // Redo traces again.
          proj_ctx->entity->calc_nearest_point(tr->end, &nearest_point);
          ex_tr = global->trace->ray(tr->end, proj_ctx->entity->obb_center(), mask_shot & ~(0x40000000), TRACE_EVERYTHING, nullptr, PROJ_TRACE_FILTER);
          if(ex_tr.started_solid && ex_tr.hit_entity != nullptr)
            ex_tr = global->trace->ray(tr->end, nearest_point, mask_shot & ~(0x40000000), TRACE_EVERYTHING, ex_tr.hit_entity, PROJ_TRACE_FILTER);

          if(ex_tr.vis(proj_ctx->entity) && !ex_tr.started_solid)
            return true;
        }
        return false;
      }
    }

    proj_ctx->entity->set_abs_origin(old_abs_origin);
    return false;
  }

  // We hit the entity we were looking for.
  if(tr->hit_entity != nullptr && tr->hit_entity == proj_ctx->entity)
    return true;

  if(process_splash_bot(tr, proj_ctx, flags))
    return true;

  // OBB SCAN
  if(!(flags & PROJ_TRAJECTORY_SKIP_OBB_SCAN)){
    if(proj_ctx->obb_scan){
      if(get_adjusted_aim_point(proj_ctx, tr))
        return true;
    }
  
    proj_ctx->obb_scan = false;
  }

  return false;
}

bool c_projectile_aimbot::process_splash_bot(s_trace* tr, s_current_target_data* proj_ctx, i32 flags){
  if(!is_first_processed_entity)
    return false;

  if(!global->aimbot_settings->splash_bot)
    return false;

  if(global->aimbot_settings->splash_bot_use_key){
    if(!splash_key_bind.is_toggled())
      return false;
  }

  if(tr == nullptr || proj_ctx == nullptr){
    DBG("[-] c_projectile_aimbot::process_splash_bot bad data\n");
    return false;
  }

  if(!proj_ctx->wep->is_splash_weapon())
    return false;

  if(!splash_point->get_valid_points_to_entity(proj_ctx->entity, proj_ctx->predicted_pos, proj_ctx->predicted_obb))
    return false;

  vec3* splash_points      = nullptr;
  i32   splash_point_count = 0;

  c_base_player* player = proj_ctx->entity->get_player();
  if(player != nullptr){
    c_player_data* data = player->data();
    if(data == nullptr)
      return false;

    splash_points      = data->splash_points;
    splash_point_count = data->splash_point_count;
  }
  else{
    c_entity_data* data = proj_ctx->entity->entity_data();
    if(data == nullptr)
      return false;

    splash_points      = data->splash_points;
    splash_point_count = data->splash_point_count;
  }

  if(splash_points == nullptr || !splash_point_count)
    return false;

  assert(splash_point_count <= MAX_ENTITY_DATA_SPLASH_POINTS);

  // Edit of June 12th, 2024.
  /* Since the list is sorted by default, we'll give the entity with the best conditions the most splash points. Then we'll slowly reduce it by 75% until its zero.
     This works really well without making the splash bot 'bad'. 
  */

  i32 max_points = (i32)math::round((float)math::smallest(MAX_ENTITY_DATA_SPLASH_POINTS, splash_point_count));
  if(max_points <= 0)
    return false;

  float splash_radius = get_splash_bot_blast_radius(proj_ctx->wep);
  vec3  correct_aim_angle;
  for(i32 i = 0; i < max_points; i++){
    if(!utils::is_in_blast_range(proj_ctx->entity, proj_ctx->predicted_pos, splash_points[i], splash_radius))
      continue;

    proj_ctx->aim_point        = splash_points[i];
    proj_ctx->target_aim_point = proj_ctx->aim_point;

    if(!get_correct_shoot_pos_info(proj_ctx, true))
      continue;

    if(!calc_travel_time(proj_ctx->proj_shoot_pos, proj_ctx->aim_point, proj_ctx->proj_speed, proj_ctx->travel_time))
      continue;

    if(proj_ctx->proj_gravity > 0.f){
      if(!calc_gravity_offset(proj_ctx))
        continue;
    }

    global->skip_trajectory_check = false;
    if(is_projectile_trajectory_point_visible(proj_ctx, PROJ_TRAJECTORY_SPLASH_BOT_TEST))
      return true;

    //splash_bot_reduction *= 0.75f;
  }
  return false;
}

bool c_projectile_aimbot::get_target(c_base_entity*& target, vec3& aim_point, s_current_target_data& td){
  if(global->aimbot_settings == nullptr)
    return false;

  s_current_target_data* target_data = new s_current_target_data;
  target_data->localplayer = utils::localplayer();
  if(target_data->localplayer == nullptr)
    return false;

  target_data->wep = target_data->localplayer->get_weapon();
  if(target_data->wep == nullptr)
    return false;

  target_data->shoot_pos = target_data->localplayer->shoot_pos();

  set_aimbot_filter();
  crusaders_crossbow_logic();

  if(filter == nullptr)
    return false;

  {
    is_first_processed_entity = true;
    for(u32 i = target_list->get_loop_start_index(); i <= target_list->get_loop_end_index(); i++){
      i32 index = target_list->get_sorted_target_index(i);
      if(index == global->localplayer_index)
        continue;

      c_base_entity* entity = global->entity_list->get_entity(index);
      if(entity == nullptr)
        continue;

      if(entity->is_player()){
        c_player_data* data = utils::get_player_data(index);
        if(data == nullptr)
          continue;

        data->bypass_hit_chance = false;
      }

      if(!should_target_entity(entity))
        continue;

      target_data->entity         = entity;
      target_data->old_abs_origin = entity->get_abs_origin();
      target_data->performed_trajectory_vis_check = false;

      if(!should_predict_entity(target_data))
        continue;

      if(!filter->sanity((s_lc_record*)target_data, entity)){
        is_first_processed_entity = false;
        entity->set_abs_origin(target_data->old_abs_origin);
        continue;
      }

      if(!filter->handle((s_lc_record*)target_data, entity)){
        is_first_processed_entity = false;
        entity->set_abs_origin(target_data->old_abs_origin);
        continue;
      }

      if(filter->store((s_lc_record*)target_data, entity)){
        is_first_processed_entity = false;
        memcpy(&td, target_data, sizeof(s_current_target_data));
      }

      is_first_processed_entity = false;
      entity->set_abs_origin(target_data->old_abs_origin);
    }

    aim_point                 = filter->aim_point;
    target                    = filter->target_ent;
  }

  return target != nullptr;
}

bool c_projectile_aimbot::should_release_weapon(c_base_player* target, vec3 shoot_pos, vec3 aim_point){
  if(target == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  ctf_weapon_info* wep_info = weapon->get_weapon_info();
  if(wep_info == nullptr)
    return false;

  // If it's not a release weapon then always return true.
  if(!weapon->is_release_weapon())
    return true;

  // We're currently stepping to bypass aimbot detections.
  //  if(acp->is_aim_step_active())
  //    return false;

  if(weapon->is_huntsman()){
    if(global->aimbot_settings->sniper_wait_for_headshot){
      if(weapon->charge_begin_time() > 0.f)
        return true;

      return false;
    }

    return localplayer->can_huntsman_kill(target) || weapon->charge_begin_time() > 0.f && !(global->original_cmd.buttons & IN_ATTACK) && global->aimbot_settings->aim_mode_on_attack;
  }
  else if(weapon->is_sticky_launcher()){
    float charge_time     = (globaldata->cur_time - weapon->charge_begin_time());
    return weapon->charge_begin_time() > 0.f && charge_time > 0.f;
  }
  else if(weapon->is_loose_cannon()){
    vec3  aim_angle   = math::calc_view_angle(shoot_pos, aim_point);
    float time_to_hit = shoot_pos.distance(aim_point) / weapon->get_projectile_speed(aim_angle);
    float reach_time  = 1.f - math::clamp(time_to_hit, 0.f, 1.f);
    float charge_time = 1.f - math::clamp(globaldata->cur_time - weapon->get_loose_cannon_charge_begin_time(), 0.f, 1.f);

    if(global->aimbot_settings->loose_cannon_lethal){
      float damage = wep_info->damage * 0.5f;
      bool  lethal = (target->get_health() <= (i32)damage);

      if(lethal){ // If we gotta charge and where no longer attacking with (auto shoot disabled or on_attack aim mode) then return true.
        if(!global->aimbot_settings->auto_shoot && global->aimbot_settings->aim_mode_on_key || global->aimbot_settings->aim_mode_on_attack){
          if(charge_time > 0.f && !(global->current_cmd->buttons & IN_ATTACK))
            return true;
        }
        else
          return charge_time > 0.f;
      }
    }

    // Since loose cannon balls don't explode on impact. They'll pass through the sentry, dispenser or teleporter.
    // Make it so it'll only shoot if the cannon ball is going to blow up next to the object.
    if(global->aimbot_settings->double_donk_above_ms > 0.f && charge_time > 0.f && time_to_hit <= (global->aimbot_settings->double_donk_above_ms / 1000.f) && target->is_player() || charge_time > 0.f && charge_time >= reach_time - math::ticks_to_time(1))
      return true;

    // If we gotta charge and where no longer attacking with (auto shoot disabled or on_attack aim mode) then return true.
    if(!global->aimbot_settings->auto_shoot && global->aimbot_settings->aim_mode_on_key || global->aimbot_settings->aim_mode_on_attack){
      if(charge_time > 0.f && !(global->current_cmd->buttons & IN_ATTACK))
        return true;
    }
  }

  return false;
}

bool c_projectile_aimbot::should_force_release_weapon(c_base_player* target, vec3 shoot_pos, vec3 aim_point){
  if(global->aimbot_settings->aim_mode_on_attack && global->aimbot_settings->auto_release)
    return true;

  if(target == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  // If it's not a release weapon then always return true.
  if(!weapon->is_release_weapon())
    return false;

  // Fixes an issue where it would shoot where your cross
  if(weapon->is_sticky_launcher()){
    float charge_time     = (globaldata->cur_time - weapon->charge_begin_time());
    float max_charge_time = weapon->get_sticky_launcher_max_charge();
    return weapon->charge_begin_time() > 0.f && charge_time >= max_charge_time;
  }

  return false;
}

bool c_projectile_aimbot::should_bypass_hit_chance(c_base_player* entity, vec3 shoot_pos, vec3 angles, vec3& aim_point){
  if(entity == nullptr)
    return false;

  // Don't need hit chance for non players.
  if(!entity->is_player())
    return true;

  bool should_bypass = false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  {
    if(  weapon->is_beggars()
      || weapon->is_throwables()
      || weapon->is_cleaver()
      || weapon->is_ball_bat()
      || weapon->is_flamethrower()
      || weapon->is_wrangler()
      || weapon->is_sticky_launcher()
      || weapon->is_syringe_gun()
      || weapon->is_loose_cannon())
        return true;
  }

  c_base_player* player = entity->get_player();
  if(player == nullptr)
    return false;

  c_player_data* data = player->data();
  if(data == nullptr)
    return false;

  if(global->untouched_cmd.buttons & IN_ATTACK || global->aimbot_settings->aim_mode_on_attack)
    should_bypass = true;

  if(data->bypass_hit_chance)
    should_bypass = true;

  vec3  pos = entity->obb_center();
  float t   = 0.f;
  if(!calc_travel_time(shoot_pos, pos, weapon->get_projectile_speed(angles), t))
    return false;

  // A special condition to ignore hit chance on friendly heavies with the crossbow.
  if(player->is_player_class(TF_CLASS_HEAVY) && player->team() == localplayer->team() && weapon->is_crossbow())
    should_bypass = (t <= player->has_condition(TF_COND_AIMING) ? 0.4f : 0.1f);

  return should_bypass;
}

bool c_projectile_aimbot::do_beggars_logic(c_base_weapon* weapon){
  if(weapon == nullptr)
    return false;

  if(!weapon->is_beggars())
    return false;

  // We've reached the limit. 
  if(weapon->clip1() >= 3)
    return true;

  bool is_manually_pressing_attack = (global->original_cmd.buttons & IN_ATTACK);

  if(!weapon->clip1() && !global->aimbot_settings->auto_reload_beggars)
    return !is_manually_pressing_attack;

  // We're now firing?
  if(weapon->clip1() > 0){
    // The weapon is now firing.
    if(weapon->reload_mode() == 0)
      return true;
      
    if(is_manually_pressing_attack)
      return false;

    return true;
  }

  return !(weapon->clip1() == 0 && weapon->reload_mode() == 0 && (weapon->next_primary_attack() <= globaldata->cur_time));
}

bool c_projectile_aimbot::should_auto_shoot(){
  if(auto_heal_arrow->force_enable_heal_arrow())
    return true;
  
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(global->aimbot_settings->aim_mode_automatic){
    if(do_beggars_logic(weapon)){
      global->current_cmd->buttons &= ~IN_ATTACK;
      return false;
    }

     return true;
  }

  if(!global->aimbot_settings->auto_shoot)
    return false;

  if(global->aimbot_settings->aim_mode_on_attack)
    return false;

  if(weapon->is_beggars()){
    global->current_cmd->buttons &= ~IN_ATTACK;
    return !do_beggars_logic(weapon);
  }
  
  return true;
}

bool c_projectile_aimbot::should_disallow_crits(c_base_entity* target, c_base_weapon* wep){
  if(target == nullptr || wep == nullptr)
    return false;

  // Disallow forcing crits when targeting team-mates.
  if(wep->is_crossbow() && target->team() == global->localplayer_team)
    return true;
  else{
    // Don't force crits if were not aiming at players or bosses.
    if(!target->is_player() && !target->is_boss())
      return true;
  }

  return false;
}

bool c_projectile_aimbot::process_attack_buttons(c_base_weapon* wep, u32 type){
  bool is_manually_pressing_attack  = (global->original_cmd.buttons & IN_ATTACK);
  bool wants_unrelease_if_possible  = (global->original_cmd.buttons & IN_ATTACK2);
  switch(type){
    default: break;
    case -1: // Aim key not pressed.
    {
      if(!global->aimbot_settings->aim_mode_on_key)
        return false;

      if(wep->is_huntsman()){
        if(wep->charge_begin_time() > 0.f && !is_manually_pressing_attack && proj_aimbot_ran){
          global->current_cmd->buttons &= ~IN_ATTACK;
          global->current_cmd->buttons |= IN_ATTACK2;
        }
      }
      else if(wep->is_sticky_launcher()){
        if(wep->charge_begin_time() > 0.f && !is_manually_pressing_attack && proj_aimbot_ran){
          DBG("[!] Canceling demoman charge due to aimkey not being pressed anymore. (Aimbot ran)\n");
          global->current_cmd->buttons |= IN_ATTACK;
          auto_weapon_swap->swap_and_return_to(2, false);
        }
      }
      else if(wep->is_loose_cannon()){
        float charge_time = 1.f - math::clamp(globaldata->cur_time - wep->get_loose_cannon_charge_begin_time(), 0.f, 1.f);
        if(charge_time > 0.f && !is_manually_pressing_attack && proj_aimbot_ran){
           DBG("[!] Canceling demoman loose cannon charge due to aimkey not being pressed anymore. (Aimbot ran)\n");
          global->current_cmd->buttons |= IN_ATTACK;
          auto_weapon_swap->swap_and_return_to(2, false);
        }
      }

      proj_aimbot_ran = false;
      break;
    }
    case 0: // Before get_target is called
    {
      // Handle on attack code.
      if(global->aimbot_settings->aim_mode_on_attack && !is_manually_pressing_attack){
        if(!wep->is_release_weapon())
          return true;
        else{

          // The huntsman have no charge. Lets go ahead and not run.
          if((wep->is_huntsman() || wep->is_sticky_launcher()) && wep->charge_begin_time() <= 0.f)
            return true;
          else if(wep->is_loose_cannon() && wep->get_loose_cannon_charge() <= 0.f) // Also check for loose cannon charge.
            return true;
        }
      }

      if(wep->is_huntsman())
        global->current_cmd->buttons |= IN_ATTACK2;
      else if(wep->is_sticky_launcher() && wep->charge_begin_time() <= 0.f && should_auto_shoot()){
        global->current_cmd->buttons |= IN_ATTACK; // Lets see if we can hit somebody.
        return false;
      }
      else if(wep->is_loose_cannon()){
        // Lets make sure we keep this charge.
        float charge_time = 1.f - math::clamp(globaldata->cur_time - wep->get_loose_cannon_charge_begin_time(), 0.f, 1.f);
        if(charge_time > 0.f){
          global->current_cmd->buttons |= IN_ATTACK;
        }
      }

      break;
    }
    case 1: // After get_target is called and is successful
    {
      // This will prevent our weapon from firing.
      if(wep->is_huntsman() && !wants_unrelease_if_possible)
        global->current_cmd->buttons &= ~IN_ATTACK2;

      if(wep->is_beggars() && wep->clip1() > 0)
        global->current_cmd->buttons &= ~IN_ATTACK;

      break;
    }
    case 2: // on target fail
    case 3: // Hitchance fail. (When the hitchance is lower than our setting)
    {
      // Do not prevent attacking at all with on attack mode.
      // People may use on attack mode for playing legit and having hit chance or other factors-
      // Preventing them from shooting or releasing their wepaon will not look legit.
      if(global->aimbot_settings->aim_mode_on_attack){
        if(wep->is_huntsman() && !wants_unrelease_if_possible)
          global->current_cmd->buttons &= ~IN_ATTACK2;

        return false;
      }

      if(wep->is_huntsman() && !global->aimbot_settings->aim_mode_on_attack && !wants_unrelease_if_possible){
        if(wep->charge_begin_time() > 0.f){ // We have charge lets check it.
          // If we have been pulling back the bow for more than 5 seconds pull it back to avoid.
          // The spread caused by holding it for too long.
          if(math::abs(wep->charge_begin_time() - globaldata->cur_time) > 5.0f){
            global->current_cmd->buttons &= ~IN_ATTACK;
            global->current_cmd->buttons |= IN_ATTACK2;
            DBG("[!] Huntsman: Charge held for too long\n");
          }
          else{ // Keep our bow charged.
            global->current_cmd->buttons |= IN_ATTACK;
            global->current_cmd->buttons &= ~IN_ATTACK2;
          }
        }
        else{ // We have no charge. Don't begin pulling back the bow.

          if(!is_manually_pressing_attack)
            global->current_cmd->buttons &= ~IN_ATTACK;
          else
            global->current_cmd->buttons &= ~IN_ATTACK2;
        }
      }
      else if(wep->is_sticky_launcher() && !global->aimbot_settings->aim_mode_on_attack){
        // Okay, so if we're about to over charge our sticky launcher. Lets switch our weapon to something else and then back.
        // This will cancel our charge.
        // Predict our latency and subtract 22 ticks (the max amount of commands we can batch) to ensure we don't magically shoot where we're looking at.
        float charge_time = (globaldata->cur_time - wep->charge_begin_time()) + utils::get_latency();
        if(wep->charge_begin_time() > 0.f && charge_time >= wep->get_sticky_launcher_max_charge() - math::ticks_to_time(22)){
          auto_weapon_swap->swap_and_return_to(2, false);
          // Continue to hold IN_ATTACK down to prevent releasing our sticky during the switch.
          global->current_cmd->buttons |= IN_ATTACK;
        }
        else if(wep->charge_begin_time() > 0.f && should_auto_shoot()) // Keep building the charge.
          global->current_cmd->buttons |= IN_ATTACK;
      }
      else if(wep->is_loose_cannon() && !global->aimbot_settings->aim_mode_on_attack){
        // Same thing for the sticky.
        float charge_time = 1.f - math::clamp(globaldata->cur_time - wep->get_loose_cannon_charge_begin_time(), 0.f, 1.f);
        if(charge_time >= 0.8f){
          auto_weapon_swap->swap_and_return_to(2, false);
          global->current_cmd->buttons |= IN_ATTACK;
        }
      }
      else if(!wep->is_release_weapon() && type == 3){
        global->current_cmd->buttons &= ~IN_ATTACK;
      }

      // Try and prevent an unwanted attack.
      if(wep->is_beggars() && wep->clip1() == 0 && type == 2 && !is_manually_pressing_attack)
        global->current_cmd->buttons &= ~IN_ATTACK;
      

      return false;
    }
  }

  return false;
}

bool c_projectile_aimbot::crithack_force_wrapper(bool b){
  return crit_hack->force(b);
}

bool c_projectile_aimbot::run(){
  found_sentry         = false;
  sentry_fires_rockets = false;
  if(global->aimbot_settings == nullptr)
    return false;

  if(!global->aimbot_settings->enabled)
    return false;

  if(!splash_key_bind.is_valid())
    splash_key_bind = c_key_control(&global->aimbot_settings->splash_bot_key, &global->aimbot_settings->splash_bot_key_held, &global->aimbot_settings->splash_bot_key_toggle, &global->aimbot_settings->splash_bot_key_double_click);

  // Called to allow the state to be updated.
  splash_key_bind.is_toggled();

  if(target_list->is_being_spectated() && global->aimbot_settings->disable_on_spectated){
    DBG("[!] c_projectile_aimbot::run() - being spectated not running.\n");
    return false;
  }

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(localplayer->is_bonked() || localplayer->is_taunting())
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  // If something player related requires trigger counters and time delays run them in here per tick.
  for(u32 i = 1; i <= globaldata->max_clients; i++){
    c_base_player* entity = global->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    static s_current_target_data tmp;
    memset(&tmp, 0, sizeof(s_current_target_data));
    tmp.entity = entity;

    if(!calc_travel_time(localplayer->shoot_pos(), entity->obb_center(), weapon->get_projectile_speed(), tmp.travel_time))
      continue;

    predict_evasion(&tmp, true);
  }

  if(!global->aimbot_settings->silent_aim_none && !localplayer->can_fire())
    return false;
  
  ctf_weapon_info* weapon_info = weapon->get_weapon_info();
  if(weapon_info == nullptr)
    return false;

  static float allow_rocket_fire_time = 0.f;

  if(weapon->is_wrangler()){
    for(u32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
      c_base_entity* entity = global->entity_list->get_entity(i);
      if(entity == nullptr)
        continue;

      if(entity->is_dormant())
        continue;

      if(entity->team() != localplayer->team())
        continue;

      if(!entity->is_sentry())
        continue;

      if(entity->disposable_building()) // We can't use the wrangler on this.
        continue;

      c_base_player* owner = entity->get_building_owner();
      if(owner == nullptr)
        continue;

      if(owner->get_index() != global->engine->get_local_player())
        continue;

      s_trace trace    = global->trace->ray(localplayer->shoot_pos(), entity->obb_center(), mask_bullet, TRACE_EVERYTHING, localplayer, TR_CUSTOM_FILTER_WRANGLER_AIMBOT_TEST);
      found_sentry     = trace.vis(entity);

      if(entity->upgrade_level() == 3 && entity->rockets() > 0)
        sentry_fires_rockets = true;

      sentry_shoot_pos = entity->get_sentry_shoot_pos();
      break; // Break on the first sentry we found.
    }

    if(!found_sentry)
      return false;
  }
  else{

    // Since swapping from the wrench to the wranlger can cause you to mistakenly pickup your sentry. This should "Hopefully" stop this.
    allow_rocket_fire_time = math::time();

    //  Don't run if we're not using a valid projectile weapon.
    if(!weapon->is_projectile_weapon() && !weapon->is_ball_bat() || weapon->is_blast_jumper_weapon())
      return false;

    #if !defined(DEV_MODE)
     if(weapon->is_ball_bat())
      return false;
    #endif
  }

  // If we using the aim key mode. Don't run unless we're holding our key down.
  if(!auto_heal_arrow->force_enable_heal_arrow()){
    if(global->aimbot_settings->aim_mode_on_key && !input_system->held(global->aimbot_settings->aim_hotkey)){
      process_attack_buttons(weapon, -1);
      return false;
    }
  }

  proj_aimbot_ran = true;
  if(process_attack_buttons(weapon, 0))
    return false;

  s_current_target_data  target_data;
  vec3                   aim_point;
  c_base_entity*         target;

  if(!get_target(target, aim_point, target_data)){
    process_attack_buttons(weapon, 2);
    throwable_target_confirms = 0;
    global->aimbot_target_reset_time = -1.f;
    return false;
  }

  if(weapon->is_throwables()){
    throwable_target_confirms++;
    if(throwable_target_confirms <= 10)
      return false;
  }
  else
    throwable_target_confirms = 0;

  if(process_attack_buttons(weapon, 1))
    return false;

  global->on_aimbot_target(target->get_index(), aim_point, weapon);

  if(should_disallow_crits(target, weapon))
    crithack_force_wrapper(false);

  vec3 shoot_pos = target_data.proj_shoot_pos;
  vec3 aim_angle = target_data.aim_angle;

  if(!should_bypass_hit_chance(target, shoot_pos, aim_angle, aim_point)){
    float min_hit_chance = get_min_hit_chance(target_data.travel_time);
    float hit_chance     = entity_prediction->calculate_hit_chance(target);

    if(min_hit_chance > hit_chance){
      global->info_panel->add_entry(INFO_PANEL_AIMBOT, WXOR(L"HIT-CHANCE"), colour(255, 255, 0, 255));
      process_attack_buttons(weapon, 3);
      return false;
    }
  }

  global->info_panel->clear(INFO_PANEL_AIMBOT);

  bool will_fire = false;

  global->current_cmd->view_angles = aim_angle;

  // default impl
  if(!weapon->is_release_weapon()){
    global->current_cmd->view_angles = aim_angle;

    if(should_auto_shoot()){
      global->current_cmd->buttons |= IN_ATTACK;
      if(weapon->is_wrangler() && sentry_fires_rockets && math::abs(allow_rocket_fire_time - math::time()) > 5.0f)
          global->current_cmd->buttons |= IN_ATTACK2;
    }

    if(global->aimbot_settings->silent_aim_none)
      global->engine->set_view_angles(aim_angle);
    else if(!weapon->is_wrangler() && !weapon->is_flamethrower() && !weapon->is_throwables() && global->aimbot_settings->silent_aim_serverside)
      packet_manager->set_choke(true);

    will_fire = true;
  }
  else{
    if(should_release_weapon(target, shoot_pos, aim_point)){
      global->current_cmd->view_angles = aim_angle;

      if(should_auto_shoot() || should_force_release_weapon(target, shoot_pos, aim_point))
        global->current_cmd->buttons    &= ~IN_ATTACK;

      if(global->aimbot_settings->silent_aim_none)
        global->engine->set_view_angles(aim_angle);
      else if(global->aimbot_settings->silent_aim_serverside && !weapon->is_throwables())
        packet_manager->set_choke(true);

      will_fire = true;
    }
    else{
      if(should_auto_shoot())
        global->current_cmd->buttons |= IN_ATTACK;
    }
  }

  if(will_fire){
    if(target->is_player()){
      c_base_player* player = (c_base_player*)target;
      c_player_data* data   = player->data();
      if(data != nullptr){
        global->last_shot_predicted_player        = target->get_index();
        global->last_shot_predicted_player_expire = math::time() + data->pred_real_time + 1.f;
      }
    }

    #if defined(DEBUG_PROJECTILE_AIMBOT_SPECIAL)
      if(global->aimbot_settings->auto_shoot && !double_tap->is_shifting){
        float life_time = weapon_info->time_fire_delay + target_data.travel_time + 2.f;

        render_debug->draw_line(target_data.proj_shoot_pos, aim_point, colour(255, 0, 255, 255), true, life_time);
        render_debug->draw_line(shoot_pos, aim_point, colour(255, 255, 255, 255), true, life_time);
        render_debug->draw_3dbox(target->origin(), target->obb_mins(), target->obb_maxs(), vec3(0.f, 0.f, 0.f), colour(255, 0, 0, 32), colour(0, 0, 0, 255), life_time);
        render_debug->draw_3dbox(target_data.predicted_pos, target->obb_mins(), target->obb_maxs(), vec3(0.f, 0.f, 0.f), colour(100, 0, 255, 32), colour(0, 0, 0, 255), life_time);
        render_debug->draw_3dbox(aim_point, target_data.proj_min, target_data.proj_max, vec3(0.f, 0.f, 0.f), colour(255, 255, 255, 32), colour(0, 0, 0, 255), life_time);
        render_debug->draw_3dbox(target_data.aim_point, target_data.proj_min, target_data.proj_max, vec3(0.f, 0.f, 0.f), colour(255, 255, 0, 32), colour(0, 0, 0, 255), life_time);
        render_debug->draw_3dbox(target_data.target_aim_point, target_data.proj_min, target_data.proj_max, vec3(0.f, 0.f, 0.f), colour(255, 100, 255, 32), colour(0, 0, 0, 255), life_time);

        vec3 last_pos;
        s_projectile_simulation_context ctx(weapon, aim_angle, true, false, false, true, aim_point);
        for(u32 ticks = 0; ticks < math::time_to_ticks(1800.f); ticks++){
          if(!proj_simulate->predict_tick(&ctx))
            break;

          if(ticks)
            render_debug->draw_line(last_pos, ctx.predicted_origin, colour(255, 255, 255, 255), true, life_time);

          last_pos = ctx.predicted_origin;
        }

        render_debug->draw_3dbox(ctx.start_pos, ctx.proj_min, ctx.proj_max, vec3(0.f, 0.f, 0.f), colour(255, 255, 255, 32), colour(0, 0, 0, 255), life_time);
        render_debug->draw_3dbox(ctx.end_pos, ctx.proj_min, ctx.proj_max, vec3(0.f, 0.f, 0.f), colour(255, 255, 255, 32), colour(0, 0, 0, 255), life_time);

        {
          render_debug->draw_text(target_data.predicted_pos, colour(100, 0, 255, 255), XOR("predicted_pos"), 6, life_time);
        }

        i8 buf[IDEAL_MIN_BUF_SIZE];
        {
          formatA(buf, XOR("speed: %i"), (i32)target_data.proj_speed);
          render_debug->draw_text(target->origin(), colour(255, 255, 255, 255), buf, 1, life_time);
        }

        {
          formatA(buf, XOR("time %ims"), (i32)(target_data.travel_time * 1000.f));
          render_debug->draw_text(target->origin(), colour(255, 255, 255, 255), buf, 2, life_time);
        }

      }
    #endif
  }

  return will_fire;
}

CLASS_ALLOC(c_projectile_aimbot, projectile_aimbot);