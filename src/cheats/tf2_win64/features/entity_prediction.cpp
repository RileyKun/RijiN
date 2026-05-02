#include "../link.h"

CLASS_ALLOC(c_entity_prediction, entity_prediction);

s_entity_predict_ctx::s_entity_predict_ctx(c_base_entity* entity_to_predict, u32 new_flags){
  assert(entity_to_predict != nullptr);
  memset(this, 0, XOR32(sizeof(*this)));

  // fill the context with the required info
  if(entity_to_predict->is_player()){
    // Don't predict noclipping players
    if(entity_to_predict->move_type() == MOVETYPE_NOCLIP)
      return;

    bool predicting_local       = entity_to_predict->get_index() == global->engine->get_local_player();

    flags                       = new_flags;
    predicted_entity            = entity_to_predict;
    predicted_origin            = predicted_entity->origin() + vec3(0.f, 0.f, 0.09375);
    predicted_velocity          = predicted_entity->velocity();
    predicted_obb_mins          = predicted_entity->obb_mins();
    predicted_obb_maxs          = predicted_entity->obb_maxs();
    predicted_flags             = predicted_entity->entity_flags();
    predicted_surface_friction  = predicted_entity->surface_friction();
    predicted_wish_speed        = predicted_entity->get_max_speed();
    
    c_player_data* data = predicted_entity->data();
    if(data == nullptr){
      DBG("[-] s_entity_predict_ctx::s_entity_predict_ctx - player data nullptr\n", data);
      predicted_entity  = nullptr;
      prediction_broken = true;
      return;
    }

    // EP_FLAG_PREDICT_YAW_TURN is responsible for predicting circular based motion
    // It works by creating a velocity angle from the velocity and then estimating the yaw change per tick
    if(flags & EP_FLAG_PREDICT_YAW_TURN)
      predicted_wish_yaw_turn = data->pred_change;

    vec3 prev_velocity;
    if(!entity_prediction->calc_frame_velocity(predicted_entity, &predicted_velocity) || !entity_prediction->calc_frame_velocity(predicted_entity, &prev_velocity, 1)){
      DBG("[-] s_entity_predict_ctx::s_entity_predict_ctx - calc_frame_velocity\n");
      predicted_entity  = nullptr;
      prediction_broken = true;
      return;
    }

    const float yaw1      = math::normalize_angle(math::rad_2_deg(math::atan2(predicted_velocity.y, predicted_velocity.x)));
    const float yaw2      = math::normalize_angle(math::rad_2_deg(math::atan2(prev_velocity.y, prev_velocity.x)));
    float yaw_delta       = math::abs(math::normalize_angle(yaw1 - yaw2));

    if(data->pre_pred_flags & PRE_PREDICTION_APPLY_FRICTION)
      flags |= EP_FLAG_PREDICT_FRICTION;

    if(data->pre_pred_flags & PRE_PREDICITON_ZERO_VEL){
      predicted_velocity.x = 0.f;
      predicted_velocity.y = 0.f;
    }

    if(predicted_flags & FL_ONGROUND){
      while(yaw_delta > 90.f){
        predicted_velocity.x *= 0.5f;
        predicted_velocity.y *= 0.5f;

        yaw_delta -= 45.f;
      }
    }
  }
  else{
    DBG("[-] s_entity_predict_ctx: NO SUPPORT FOR NON PLAYERS!\n");
  }
}

bool c_entity_prediction::calc_frame_velocity(c_base_player* player, vec3* vel_out, u32 tick_offset, u32 flags){
  if(player == nullptr || vel_out == nullptr)
    return false;

  if(!player->is_player())
    return false;

  if(player->get_index() == global->localplayer_index && !tick_offset){
    *vel_out = player->velocity();
    return true;
  }

  c_player_data* data = player->data();
  if(data == nullptr)
    return false;

  s_lc_data* lc_data = lag_compensation->get_data(player);
  if(lc_data == nullptr)
    return false;

  s_lc_record* cur_record    = lc_data->get_record(tick_offset);
  s_lc_record* last_record   = lc_data->get_record(tick_offset + 1);
  if(cur_record == nullptr || last_record == nullptr){
    //DBG("[-] c_entity_prediction::calc_frame_velocity: not enough data!\n");
    return false;
  }

  if(cur_record->simulation_time <= last_record->simulation_time){
    DBG("[-] c_entity_prediction::calc_frame_velocity: simulation_time is equal\n");
    return false;
  }

  // Fix the infamous ducking/unducking velocity spike
  // This happens because when you duck your collision obb is set to 62 from 82
  // When that happens your origin (which is based on your collision obb) is shifted up 20 in 1 tick
  // Causing essentially 20 z units per tick and therefor the bug that causes velocity spiking
  // Same thing also happens when you unduck but downwards
  vec3 size_change    = (cur_record->obb_mins + cur_record->obb_maxs) - (last_record->obb_mins + last_record->obb_maxs);
  vec3 correct_origin = player->entity_flags() & FL_ONGROUND ? cur_record->origin : cur_record->origin + size_change;

  vec3 delta = (correct_origin - last_record->origin) / math::biggest(math::time_to_ticks(math::abs(cur_record->simulation_time - last_record->simulation_time)), 1);
  if(math::abs(delta.z) >= 20.f){
    DBG("[-] (%i) c_entity_prediction::calc_frame_velocity: z spike (%2.2f)!\n", flags, delta.z);
    return false;
  }

  for(i32 i = 0; i <= 1; i++){
    if(math::abs(delta[i]) >= 100.f){
     DBG("[-] c_entity_prediction::calc_frame_velocity: velocity axis %i spike\n");
      return false;
    }
  }

  float max_speed = player->get_max_speed();
  vec3 predicted_velocity = (correct_origin - last_record->origin) / math::biggest(math::abs(cur_record->simulation_time - last_record->simulation_time), globaldata->interval_per_tick);

  // Scale velocity by tens. Reduce noise in prediction.
  bool should_scale = false;//flags & EV_FLAG_SCALE_BY_TENS;

  // If its going more than 3500 hu per tick then we're too fast, fail the function.
  float cur_speed = predicted_velocity.length_2d();
  if(cur_speed > 3500.f){
    DBG("[-] c_entity_prediction::calc_frame_velocity velocity too fast %2.2f > 3500\n", cur_speed);
    return false;
  }

  // Make sure we have some form of speed.
  if(cur_speed > 0.0f){
    // Calculate a ratio we need to scale towards.
    float ratio = (cur_speed / max_speed) * 100.f;

    // Round to the nearest ten '%' of the velocity.
    // Meaning if we're at 34% it should get rounded to the nearest number and then floored.
    // 34% -> 30% 
    float rounded_ratio = math::floor(math::round(ratio / 10.f)) * 10.f;
    float scale_value = math::clamp(math::abs((rounded_ratio / 100.f)), 0.0f, 1.f);
    
    // Check the speed of the player, if it's equal or less than 10% of their max speed lets just zero it out.
    // Alot of players who move at these speeds will be doing really bad evasive movement
    // However, Ideally we should probably do a check to see if they're actually moving anywhere or not.
    if(predicted_velocity.length_2d() <= max_speed * 0.1f){
      predicted_velocity.x = 0.f;
      predicted_velocity.y = 0.f;
    }
    
    if(should_scale){
      predicted_velocity.x *= scale_value;
      predicted_velocity.y *= scale_value;
    }
  }

  // If it's not on these maps then clamp velocity to the max speed of the player.
  if(global->current_map_hash != HASH("maps/tr_walkway_rc2.bsp") && global->current_map_hash != HASH("maps/tr_walkway_rc.bsp") && player->entity_flags() & FL_ONGROUND){
    float len       = predicted_velocity.length_2d();

    if(len != 0.f && len > max_speed){
      predicted_velocity.x *= (max_speed / len);
      predicted_velocity.y *= (max_speed / len);
    }
  }

  *vel_out                      = predicted_velocity;
  return true;
}

bool c_entity_prediction::draw_prediction_track(){
  if(!config->visual.projectile_prediction_show_path)
    return false;

  float time = math::time();
  if(time >= global->last_shot_predicted_player_expire)
    return false;

  c_base_player* player = global->entity_list->get_entity(global->last_shot_predicted_player);

  if(player == nullptr)
    return false;

  c_player_data* data = player->data();

  if(data == nullptr)
    return false;

  if(data->pred_track_array.size < 2)
    return false;

  const float  delta      = math::abs(time - global->last_shot_predicted_player_expire);

  #if defined(DEV_MODE) || defined(STAGING_MODE)

  static float last_delta = 0.f;
  static vec3 last_pos   = vec3();
  if(delta != last_delta){

    if(math::abs(last_delta - delta) > 1.f || (last_pos - player->origin()).length() > 64.f){
      last_pos   = vec3();
      last_delta = 0.f;
    }

    if(last_delta > 0.f)
      render_debug->draw_line(last_pos, player->origin(), colour(255, 255, 0, 255), false, delta);

    last_pos   = player->origin();
    last_delta = delta;
  }
  #endif



  bool render_tick = true;
  for(u32 i = 1; i <= data->pred_track_array.size; i++){
    s_pred_track_entry* last_entry = &data->pred_track_array.entries[i-1];
    s_pred_track_entry* this_entry = &data->pred_track_array.entries[i];

    vec3i last_pos_scr, this_pos_scr;
    if(!math::world2screen(last_entry->pos, last_pos_scr) || !math::world2screen(this_entry->pos, this_pos_scr))
      continue;

    if(config->visual.projectile_prediction_show_path_outline){
      render->solid_line(last_pos_scr - vec3i(0, 1), this_pos_scr - vec3i(0, 1), rgba(0,0,0,180), false, 0.f);
      render->solid_line(last_pos_scr + vec3i(0, 1), this_pos_scr + vec3i(0, 1), rgba(0,0,0,180), false, 0.f);
      render->solid_line(last_pos_scr - vec3i(1, 0), this_pos_scr - vec3i(1, 0), rgba(0,0,0,180), false, 0.f);
      render->solid_line(last_pos_scr + vec3i(1, 0), this_pos_scr + vec3i(1, 0), rgba(0,0,0,180), false, 0.f);
    }
 
    render->solid_line(last_pos_scr, this_pos_scr, rgba(config->visual.projectile_prediction_show_path_col[0], config->visual.projectile_prediction_show_path_col[1], config->visual.projectile_prediction_show_path_col[2], 255), false, 0.f);

    if(config->visual.projectile_prediction_show_path_ticks){
      colour box_face = rgba(config->visual.projectile_prediction_show_path_col[0], config->visual.projectile_prediction_show_path_col[1], config->visual.projectile_prediction_show_path_col[2], 32);
      colour box_outline = rgba(config->visual.projectile_prediction_show_path_col[0], config->visual.projectile_prediction_show_path_col[1], config->visual.projectile_prediction_show_path_col[2], 180);

      if(config->visual.projectile_prediction_show_path_outline)
        box_outline = rgba(0, 0, 0, 180);


      if(render_tick)
        render_debug->draw_static_3dbox(this_entry->pos, vec3(-1.f, -1.f, -1.f), vec3(1.f, 1.f, 1.f), vec3(0.f, 0.f, 0.f), box_face, box_outline);

      render_tick = !render_tick;
    }

    #if defined(DEV_MODE) || defined(STAGING_MODE)
    if(i == data->pred_track_array.size){
      if(data->pred_flags & EP_FLAG_PREDICT_FRICTION && player->entity_flags() & FL_ONGROUND)
        render_debug->draw_text(this_entry->pos, colour(255, 255, 255, 255), XOR("PRED FRICTION"), 2, 0.01f);

      if(data->pred_flags & EP_FLAG_PREDICT_YAW_TURN)
        render_debug->draw_text(this_entry->pos, colour(255, 255, 255, 255), XOR("PRED STRAFE"), 3, 0.01f);
    }
    #endif
  }
  //render->flush_to_gpu(render->current_primitive_type);
  //render->device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, false);

  return true;
}

static float calculate_max_turn_radius(c_base_player* player){
  if(player == nullptr)
    return 0.f;

  if(!(player->entity_flags() & FL_ONGROUND))
    return 4.f;

  return 6.f;
}

void c_entity_prediction::clean_yaw_deltas(float& raw_yaw_delta, float& yaw_delta, float& last_raw_yaw_delta){
  if(raw_yaw_delta < 0.0f)
    raw_yaw_delta += COMPRESSION_GARBAGE_VALUE;
  else if(raw_yaw_delta > 0.0f)
    raw_yaw_delta -= COMPRESSION_GARBAGE_VALUE;

  if(raw_yaw_delta > -YAW_DELTA_NOT_ENOUGH && raw_yaw_delta < YAW_DELTA_NOT_ENOUGH)
    raw_yaw_delta = 0.f;

  // A real strafe should have a small constant that never constantly jitters from a pos to a neg number.
  if(last_raw_yaw_delta != raw_yaw_delta && math::abs(last_raw_yaw_delta) == math::abs(raw_yaw_delta)){
    last_raw_yaw_delta = raw_yaw_delta;
    raw_yaw_delta = 0.f;
  }
  else
    last_raw_yaw_delta = raw_yaw_delta;

  yaw_delta    = raw_yaw_delta;
  raw_yaw_delta = math::abs(raw_yaw_delta);
}

void c_entity_prediction::compute_pred_movement_change(c_base_player* player){
  if(player == nullptr)
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  data->pre_pred_flags = 0;
  vec3 cur_vel;
  vec3 last_vel;

  const bool is_on_ground     = player->entity_flags() & FL_ONGROUND;
  const i32  required_samples = 4;
  float last_raw_yaw_delta    = 0.f;
  float last_yaw_delta        = 0.f;

  i32   samples            = 0;
  float evasion_movement   = 0.f;
  float evasion_high_delta = 0.f;

  for(i32 i = required_samples; i >= 0; i--){
    if(!calc_frame_velocity(player, &cur_vel, i, EV_FLAG_NONE))
      continue;

    if(i == required_samples){
      last_vel = cur_vel;
      continue;
    }

    float raw_yaw_delta = compute_yaw_delta(cur_vel, last_vel);
    float yaw_delta     = 0.f;

    clean_yaw_deltas(raw_yaw_delta, yaw_delta, last_raw_yaw_delta);

    if(last_yaw_delta > 0.f && yaw_delta < 0.f || yaw_delta > 0.f && last_yaw_delta < 0.f || raw_yaw_delta >= 15.f)
      evasion_movement += 1.f;

    if(raw_yaw_delta >= 25.f)
      evasion_high_delta += 1.f;

    last_yaw_delta = yaw_delta;
    last_vel = cur_vel;
    samples++;
  }

  if(!samples)
    return;

  if(evasion_movement >= (float)required_samples && is_on_ground)
    data->pre_pred_flags |= PRE_PREDICTION_APPLY_FRICTION;

  if(evasion_high_delta >= (float)required_samples && is_on_ground)
    data->pre_pred_flags |= PRE_PREDICITON_ZERO_VEL;
}

void c_entity_prediction::compute_pred_yaw_change(c_base_player* player){
  if(player == nullptr)
    return;

  if(player->get_index() == global->localplayer_index)
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  compute_pred_movement_change(player);

  const float tick_delta = (float)(math::biggest(math::time_to_ticks(math::abs(player->simulation_time() - player->old_simulation_time())), 1));
  const float max_turn_angle    = calculate_max_turn_radius(player); // was 5 in early july 2025, and was 6th in aug 1st and now is 15.
  i32         samples_collected = 0;

  const i32 required_samples = 1;

  vec3  cur_vel;
  vec3  last_vel;
  float last_raw_yaw_delta = 0.f;
  float last_yaw_delta     = 0.f;

  // Compute strafe prediction data.
  // --
  float s_yaw_delta     = 0.f;
  float s_raw_yaw_delta = 0.f;
  for(i32 i = required_samples; i >= 0; i--){
    if(!calc_frame_velocity(player, &cur_vel, i, EV_FLAG_NONE)){
      last_raw_yaw_delta = 0.f;
      continue;
    }

    if(samples_collected){
      float raw_yaw_delta = compute_yaw_delta(cur_vel, last_vel);
      float yaw_delta     = 0.f;

      clean_yaw_deltas(raw_yaw_delta, yaw_delta, last_raw_yaw_delta);

      if( cur_vel.length_2d() < PRED_CHANGE_VEL_LEN_LOW || raw_yaw_delta > PRED_CHANGE_YAW_DELTA_TOO_HIGH ||
          yaw_delta > 0.0f && last_yaw_delta < 0.0f || yaw_delta < 0.0f && last_yaw_delta > 0.0f)
        yaw_delta = 0.f;


      s_yaw_delta     += yaw_delta;
      s_raw_yaw_delta += raw_yaw_delta;
      last_yaw_delta = yaw_delta;
    }

    samples_collected++;
    last_vel = cur_vel;
  }

  if(samples_collected < required_samples){
    clear_pred_yaw_change(player);
    return;
  }
  
  s_yaw_delta     /= (float)samples_collected;
  s_raw_yaw_delta /= (float)samples_collected;

 // DBG("[!] %s [yd: %f, ryd: %f, %i, %f]\n", player->info().name, s_yaw_delta, s_raw_yaw_delta, samples_collected, tick_delta);

  const float mod = (float)samples_collected > tick_delta ? (float)samples_collected : tick_delta;

  const float lerp_mod = (0.05f * mod);

  data->pred_change = math::lerp(data->pred_change, s_yaw_delta, lerp_mod);
  {
    //if(tick_delta >= 6.f) // We'll assume at this level of tick delta the player is fakelagging.
    //  data->pred_change_delta = math::normalize_angle(s_yaw_delta - data->pred_last_yaw_delta) * globaldata->interval_per_tick;
    //else
    data->pred_change_delta = math::normalize_angle(data->pred_change - data->last_pred_change);
  }

  data->last_pred_change    = data->pred_change;
  data->pred_last_yaw_delta = s_yaw_delta;
}

void c_entity_prediction::clear_pred_yaw_change(c_base_player* player){
  if(player == nullptr)
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  data->pred_change         = 0.f;
  data->pred_change_delta   = 0.f;
  data->last_pred_change    = 0.f;
}

float c_entity_prediction::calculate_hit_chance(c_base_player* player){
  if(player == nullptr)
    return 0.f;

  c_player_data* data = player->data();
  if(data == nullptr)
    return 0.f;

  float hit_chance      = 0.f;
  float max_turn_angle  = 15.f * globaldata->interval_per_tick;

  if(data->pred_change_delta == 0.f)
    return 1.f;

  float hc = 1.f - math::clamp(math::abs(data->pred_change_delta) / max_turn_angle, 0.f, 1.f);
  return math::biggest(hc, 0.01f);
}

// Gravity is applied half before simulation and half after simulation
void c_entity_prediction::apply_gravity(s_entity_predict_ctx* ctx){
  if(ctx == nullptr || ctx->predicted_flags & FL_ONGROUND || ctx->predicted_entity->water_level() > 1)
    return;

  float gravity = ctx->predicted_entity->gravity() > 0.f ? ctx->predicted_entity->gravity() : 1.f;
        // TODO: Apply TFGameRules()->GetGravityMultiplier() to gravity
        gravity *= global->sv_gravity->flt;
        gravity *= 0.5f;
        gravity *= globaldata->interval_per_tick;

  ctx->predicted_velocity.z -= gravity;

  fix_velocity(ctx);
}

void c_entity_prediction::handle_parachute_gravity(s_entity_predict_ctx* ctx){
  if(ctx == nullptr || ctx->predicted_flags & FL_ONGROUND || ctx->predicted_entity->water_level() > 1)
    return;

  if(!ctx->predicted_entity->has_condition(TF_COND_PARACHUTE_ACTIVE) || ctx->predicted_velocity.z >= 0.f)
    return;

  ctx->predicted_velocity.z = math::biggest(ctx->predicted_velocity.z, global->tf_parachute_maxspeed_z->flt);

  float drag        = global->tf_parachute_maxspeed_xy->flt;
  float speed_x     = math::abs(ctx->predicted_velocity.x);
  float speed_y     = math::abs(ctx->predicted_velocity.y);
  float reduction_x = speed_x > drag ? (speed_x - drag) / 3.f - 10.f : 0.f;
  float reduction_y = speed_y > drag ? (speed_y - drag) / 3.f - 10.f : 0.f;

  ctx->predicted_velocity.x = math::clamp(ctx->predicted_velocity.x, -drag - reduction_x, drag + reduction_x);
  ctx->predicted_velocity.y = math::clamp(ctx->predicted_velocity.y, -drag - reduction_y, drag + reduction_y);
}

ALWAYSINLINE void c_entity_prediction::apply_friction(s_entity_predict_ctx* ctx){
  if(ctx == nullptr || !(ctx->predicted_flags & FL_ONGROUND))
    return;

  if(!(ctx->flags & EP_FLAG_PREDICT_FRICTION))
    return;

  float len = ctx->predicted_velocity.length_2d();
  if(len == 0.f)
    return;

  float friction  = global->sv_friction->flt * ctx->predicted_surface_friction;
  float new_speed = math::biggest(0.f, len - math::biggest(len, global->sv_stopspeed->flt) * (friction * globaldata->interval_per_tick));

  if(new_speed != len){
    ctx->predicted_velocity.x *= new_speed / len;
    ctx->predicted_velocity.y *= new_speed / len;

    fix_velocity(ctx);
  }
}

ALWAYSINLINE void c_entity_prediction::update_on_ground(s_entity_predict_ctx* ctx){
  if(ctx == nullptr)
    return;

  s_trace move_tr = global->trace->player_move(ctx->predicted_origin, ctx->predicted_origin - vec3(0.f, 0.f, ctx->predicted_entity->step_size()), ctx->predicted_entity);
  {
    // update onground flag
    if((move_tr.fraction != 1.f || move_tr.started_solid || move_tr.solid) && move_tr.plane.normal.z >= 0.7f){
      if(!(ctx->predicted_flags & FL_ONGROUND)){
        ctx->predicted_flags |= FL_ONGROUND;
        clip_ground_velocity(ctx);
      }
    }
    else
      ctx->predicted_flags &= ~FL_ONGROUND;

    // If we have touched the floor, set our predicted origin z to the trace result end position (hit)
    if(ctx->predicted_flags & FL_ONGROUND)
      ctx->predicted_origin.z = move_tr.end.z + EPSILON;
  }
}

void c_entity_prediction::clip_ground_velocity(s_entity_predict_ctx* ctx){
  if(ctx == nullptr || !(ctx->predicted_flags & FL_ONGROUND))
    return;

  float len       = ctx->predicted_velocity.length_2d();
  float max_speed = ctx->predicted_wish_speed;

  if(len != 0.f && len > max_speed){
    ctx->predicted_velocity.x *= max_speed / len;
    ctx->predicted_velocity.y *= max_speed / len;
  }
}

// It seems to be that maybe network compression (?) of the origin position causes a player in some rare cases to be stuck in a wall
// To solve this problem we need to trace around the player and try and find a position that we arent stuck on
// Once we have a position that hasnt started solid, we can trace back to our position we are at and get the correct origin from that
bool c_entity_prediction::resolve_stuck(s_entity_predict_ctx* ctx){
  s_trace stuck_tr = global->trace->player_move(ctx->predicted_origin, ctx->predicted_origin, ctx->predicted_entity);

  if(!stuck_tr.started_solid)
    return false;

  // This looks like alot but in reality %99 of cases would be corrected on the first 4 entries
  float offset = ctx->predicted_entity->step_size() * 0.1f;
  vec3 stuck_tests[] = {
    vec3(-offset, 0.f,      0.f),
    vec3(offset,  0.f,      0.f),
    vec3(0.f,     -offset,  0.f),
    vec3(0.f,     offset,   0.f),
    vec3(-offset, 0.f,      offset),
    vec3(offset,  0.f,      offset),
    vec3(0.f,     -offset,  offset),
    vec3(0.f,     offset,   offset),
    vec3(-offset, 0.f,      -offset),
    vec3(offset,  0.f,      -offset),
    vec3(0.f,     -offset,  -offset),
    vec3(0.f,     offset,   -offset),
    vec3(-offset, -offset,  offset),
    vec3(offset,  offset,   offset),
    vec3(offset,  -offset,  offset),
    vec3(-offset, offset,   offset),
    vec3(-offset, -offset,  -offset),
    vec3(offset,  offset,   -offset),
    vec3(offset,  -offset,  -offset),
    vec3(-offset, offset,   -offset),
  };

  for(u32 i = 0; i < sizeof(stuck_tests) / sizeof(vec3); i++){
    vec3 test_offset  = stuck_tests[i];
    stuck_tr          = global->trace->player_move(ctx->predicted_origin + test_offset, ctx->predicted_origin, ctx->predicted_entity);

    if(!stuck_tr.started_solid){
      ctx->predicted_origin = stuck_tr.end;
      //DBG("[!] ENTITY_PREDICTION: Resolved stuck player (%f %f %f)\n", test_offset.x, test_offset.y, test_offset.z);
      return false;
    }
  }

  DBG("[!] ENTITY_PREDICTION: Failed to resolve stuck player\n");
  return (ctx->prediction_broken = true);
}

void c_entity_prediction::physical_move(s_entity_predict_ctx* ctx){
  if(ctx == nullptr)
    return;

  // Apply friction
  apply_friction(ctx);

  // If we are on the ground ensure the velocity is not over the max speed
  //clip_ground_velocity(ctx);

  s_trace move_tr;
  {
    // Causes prediction to fail on thin surfaces, push players through walls, and messes up movement on sloped surfaces.
    if(resolve_stuck(ctx))
      return;

    // handle circular prediction if we have EP_FLAG_PREDICT_YAW_TURN enabled
    if((ctx->flags & EP_FLAG_PREDICT_YAW_TURN) && !ctx->predicted_entity->has_condition(TF_COND_PARACHUTE_ACTIVE)){
      if(math::abs(ctx->predicted_wish_yaw_turn) > 0.f){
        float vel_len = ctx->predicted_velocity.length_2d();

        // Take our current turn angle, and add our
        float velocity_angle        = math::normalize_angle(math::rad_2_deg(math::atan2(ctx->predicted_velocity.y, ctx->predicted_velocity.x)) + ctx->predicted_wish_yaw_turn);
        ctx->predicted_velocity.x   = math::cos(math::deg_2_rad(velocity_angle)) * vel_len;
        ctx->predicted_velocity.y   = math::sin(math::deg_2_rad(velocity_angle)) * vel_len;

        // This is kinda very important
        // You can 'move' in the air. Without this it just turns into a guessing game and many people always mash their keys in air.
        if(!(ctx->predicted_flags & FL_ONGROUND)){
          // the ideal check has a few issues with it. So I removed the code and commented the variable out. So if you look at V1, you'll know it was tried.
         // float ideal     = math::clamp(vel_len != 0.f ? math::rad_2_deg(math::asin(15.f / vel_len)) : 90.f, 0.f, 90.f);
          float side_move = ctx->predicted_wish_yaw_turn > 0.f ? -450.f : 450.f;
          #if defined(STAGING_MODE) || defined(DEV_MODE)
          //air_accelerate(ctx, vec3(0.f, velocity_angle, 0.f), 0.f, side_move);
          #endif
        }
        else{
          // I mean, should we? I don't know if this is a good idea or not.
          // I suppose we can look into it later. Like if they're fakelagging don't predict it.
        }    
      }
    }

    // predict one tick of movement
    vec3 end = ctx->predicted_origin + (ctx->predicted_velocity * globaldata->interval_per_tick);

    // start obb trace for physical move
    move_tr = global->trace->player_move(ctx->predicted_origin, end, ctx->predicted_entity);

    // Have we hit something and does it allow us to go over it?
    // If so then teleport our predicted origin to above that surface
    if((ctx->predicted_flags & FL_ONGROUND) && (move_tr.fraction != 1.f || move_tr.started_solid || move_tr.solid)){
      s_trace obstacle_trace = global->trace->player_move(end + vec3(0.f, 0.f, ctx->predicted_entity->step_size()), end, ctx->predicted_entity);

      if(!obstacle_trace.started_solid){
        end.z   = obstacle_trace.end.z + EPSILON;
        move_tr = obstacle_trace;
      }
    }

    // If we hit something and then plane is higher than -1 and lower than 0.7, we have hit a wall
    // If the plane is -1 then we are hitting a ceiling, anything over 0.7 is considered a walkable slope
    // Calculate its adjustment and re-start a trace from there so we will trace up the plane
    if((move_tr.fraction != 1.f || move_tr.started_solid || move_tr.solid)){
      if(move_tr.plane.normal.z < 0.7f)
        ctx->flags &=  ~EP_FLAG_PREDICT_YAW_TURN;

      for(u32 i = 0; i < 2; i++){
        ctx->predicted_velocity -= move_tr.plane.normal * ctx->predicted_velocity.dot(move_tr.plane.normal);

        float adjust = ctx->predicted_velocity.dot(move_tr.plane.normal);
        if(adjust < 0.f)
          ctx->predicted_velocity -= move_tr.plane.normal * adjust;

        ctx->predicted_origin = move_tr.end;
        end                   = ctx->predicted_origin + (ctx->predicted_velocity * (globaldata->interval_per_tick * (1.f - move_tr.fraction))) + vec3(0.f, 0.f, EPSILON);
        move_tr               = global->trace->player_move(ctx->predicted_origin, end, ctx->predicted_entity);
      }
    }

    ctx->predicted_origin = move_tr.end;
  }
}

void c_entity_prediction::air_accelerate(s_entity_predict_ctx* ctx, vec3 angle, float fmove, float smove){
  vec3 fwd, right;
  math::angle_2_vector(angle, &fwd, &right, nullptr);

  fwd.z   = 0.f;
  right.z = 0.f;

  fwd.normalize();
  right.normalize();

  vec3 wish_vel = fwd * fmove + right * smove;
  wish_vel.z = 0.f;

  vec3  wish_dir   = wish_vel;
  float wish_speed = wish_dir.length();
  wish_dir.normalize();

  // The ctx->predicted_wish_speed is the entity's max speed that's all.
  if(ctx->predicted_wish_speed != 0.f && wish_speed > ctx->predicted_wish_speed)
    wish_speed = ctx->predicted_wish_speed;

  float calc_wish_speed = math::smallest(wish_speed, 30.f);
  float cur_speed       = ctx->predicted_velocity.dot(wish_dir);
  float add_speed       = calc_wish_speed - cur_speed;
  if(add_speed <= 0.f)
    return;

  float accel_speed = global->sv_airaccelerate->flt * wish_speed * globaldata->interval_per_tick;
  if(accel_speed > add_speed)
    accel_speed = add_speed;

  ctx->predicted_velocity += (wish_dir * accel_speed);
}

bool c_entity_prediction::predict_tick(s_entity_predict_ctx* ctx){
  if(ctx == nullptr || ctx->predicted_entity == nullptr || ctx->prediction_broken)
    return false;

  // perform prediction
  {
    // Handle parachute gravity
    handle_parachute_gravity(ctx);

    // Apply half of the gravity
    apply_gravity(ctx);

    /*
    if (player->GetWaterJumpTime()) {
      WaterJump();
      TryPlayerMove();
      CheckWater();
      return;
    }
    */

    /*
    if (InWater() || (player && (player->InCond(TF_COND_HALLOWEEN_GHOST_MODE) || player->InCond(TF_COND_SWIMMING_NO_EFFECTS)))) {
      FullWalkMoveUnderwater();
      return;
    }
    */

    // Apply accelerations
    //

    // Physically move the entity
    physical_move(ctx);

    // Apply the other half of the gravity
    apply_gravity(ctx);

    // Recaculate FL_ONGROUND
    update_on_ground(ctx);
  }

  // set result information
  {
    ctx->has_been_predicted = true;
    ctx->predicted_ticks++;
  }

  return !ctx->prediction_broken;
}

bool c_entity_prediction::simple(s_entity_predict_ctx* ctx, u32 ticks_to_predict){
  assert(ticks_to_predict > 0);
  assert(ctx != nullptr);

  for(u32 i = 0; i < ticks_to_predict; i++)
    if(!predict_tick(ctx))
      break;

  return ctx->has_been_predicted;
}


s_entity_predict_cache_ctx* c_entity_prediction::get_cache(c_base_entity* entity){
  if(entity == nullptr)
    return nullptr;

  i32 index = entity->get_index();
  if(index < 0 || index >= MAXPLAYERS)
    return nullptr;

  return &cached_pred_data[index];
}

bool c_entity_prediction::is_cached(c_base_entity* entity){
  if(entity == nullptr)
    return false;

  s_entity_predict_cache_ctx* cache = get_cache(entity);
  if(cache == nullptr)
    return false;

  if(cache->ctx.prediction_broken || cache->ctx.predicted_entity == nullptr)
    return false;
  
  return cache->ctx.cached;
}

bool c_entity_prediction::store_cache(s_entity_predict_ctx* ctx, c_base_entity* entity, float real_time, vec3 correct_origin){
  if(ctx == nullptr || entity == nullptr)
    return false;

  s_entity_predict_cache_ctx* cache = get_cache(entity);
  if(cache == nullptr)
    return false;

  if(cache->ctx.cached)
    return true;

  memcpy(&cache->ctx, ctx, sizeof(s_entity_predict_ctx));
  cache->correct_origin = correct_origin;
  cache->real_time      = real_time;
  cache->ctx.cached = true;

  return true;
}

void c_entity_prediction::delete_cache(c_base_entity* entity){
  if(entity == nullptr)
    return;

  s_entity_predict_cache_ctx* cache = get_cache(entity);
  if(cache == nullptr)
    return;

  cache->ctx.cached = false;
}