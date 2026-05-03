#pragma once

enum e_entity_predict_flags{
  EP_FLAG_PREDICT_YAW_TURN = 1 << 0, // Avoids tangents by predicting movement based on turning estimations
};

struct s_entity_predict_ctx{
  u32             flags;
  c_base_player*  predicted_entity   = nullptr;  // entity to predict
  bool            has_been_predicted = false;    // has the entity been predicted
  bool            prediction_broken  = false;    // has the prediction been broken?
  u32             predicted_ticks    = 0;        // how many ticks has the entity been predicted
  vec3            predicted_origin;              // the resulting predicted origin
  vec3            predicted_velocity;            // the resulting predicted velocity
  vec3            predicted_obb_mins;            // the resulting predicted object bounds (mins)
  vec3            predicted_obb_maxs;            // the resulting predicted object bounds (maxs)
  u8              predicted_flags;               // predicted entity flags, such as FL_ONGROUND
  float           predicted_surface_friction;    // predicted surface friction
  float           predicted_wish_speed;          // our wish speed (used by air/ground acceleration)
  float           predicted_wish_yaw_turn;       // EP_FLAG_PREDICT_YAW_TURN: the predicted yaw turn per tick
  float           predicted_wish_yaw_turn_delta; // EP_FLAG_PREDICT_YAW_TURN: the predicted yaw turn delta per tick

  s_entity_predict_ctx(c_base_entity* entity_to_predict, u32 new_flags = EP_FLAG_PREDICT_YAW_TURN){
    assert(entity_to_predict != nullptr);
    memset(this, 0, XOR32(sizeof(*this)));

    // fill the context with the required info
    {
      // Don't predict noclipping players
      if(entity_to_predict->move_type == MOVETYPE_NOCLIP)
        return;

      bool predicting_local       = entity_to_predict->get_index() == global->engine->get_local_player();

      flags                       = new_flags;
      predicted_entity            = entity_to_predict;
      predicted_origin            = predicted_entity->origin();
      predicted_velocity          = predicted_entity->velocity();
      predicted_obb_mins          = predicted_entity->obb_mins();
      predicted_obb_maxs          = predicted_entity->obb_maxs();
      predicted_flags             = predicted_entity->entity_flags;
      predicted_surface_friction  = predicted_entity->surface_friction;
      predicted_wish_speed        = predicted_entity->max_speed;

      s_lc_data* lc_data = lag_compensation->get_data(predicted_entity);
      if(lc_data != nullptr){
        s_lc_record* record1 = lc_data->get_record(0);
        s_lc_record* record2 = lc_data->get_record(1);

        if(record1 != nullptr && record2 != nullptr){
          vec3 correct_origin = record1->origin;

          // Fix the infamous ducking/unducking velocity spike
          // This happens because when you duck your collision obb is set to 62 from 82
          // When that happens your origin (which is based on your collision obb) is shifted up 20 in 1 tick
          // Causing essentially 20 z units per tick and therefor the bug that causes velocity spiking
          // Same thing also happens when you unduck but downwards
          if(!predicting_local){
            // when bounds change we need to correct
            correct_origin += (record1->obb_mins + record1->obb_maxs) - (record2->obb_mins + record2->obb_maxs);

            // ignore huge x/y/z
            vec3 delta = (correct_origin - record2->origin) / math::time_to_ticks(math::abs(record1->simulation_time - record2->simulation_time));
            if(math::abs(delta.x) >= 100.f ||
               math::abs(delta.y) >= 100.f ||
               math::abs(delta.z) >= 100.f ||
               math::abs(delta.z) >= 20.f){
              DBG("[!] ENTITY_PREDICTION: IGNORING HUGE X/Y/Z (%f %f %f)\n", delta.x, delta.y, delta.z);
              predicted_entity  = nullptr;
              prediction_broken = true;
              return;
            }
          }

          if(record1->simulation_time <= record2->simulation_time){
            DBG("[!] ENTITY_PREDICTION: SIMTIME IS THE SAME (THIS SHOULD NEVER HAPPEN)\n");
            predicted_entity = nullptr;
            return;
          }

          c_player_data* data = predicted_entity->data();
          if(data == nullptr){
            DBG("[!] ENTITY_PREDICTION: C_PLAYER_DATA INVALID\n");
            predicted_entity = nullptr;
            return;
          }

          // EP_FLAG_PREDICT_YAW_TURN is responsible for predicting circular based motion
          // It works by creating a velocity angle from the velocity and then estimating the yaw change per tick
          if(flags & EP_FLAG_PREDICT_YAW_TURN)
            predicted_wish_yaw_turn = data->pred_change;

          if(!predicting_local)
            predicted_velocity = (correct_origin - record2->origin) / math::abs(record1->simulation_time - record2->simulation_time);
        }
        else{
          DBG("[!] ENTITY_PREDICTION: NOT ENOUGH DATA\n");
          predicted_entity  = nullptr;
          prediction_broken = true;
          return;
        }
      }
    }
  }
};

class c_entity_prediction{
public:

  void compute_pred_yaw_change(c_base_player* player){
    if(player == nullptr)
      return;

    c_player_data* data = player->data();
    if(data == nullptr)
      return;

    s_lc_data* lc_data = lag_compensation->get_data(player);
    if(lc_data == nullptr)
      return;

    s_lc_record* record1 = lc_data->get_record(0);
    s_lc_record* record2 = lc_data->get_record(1);

    if(record1 == nullptr || record2 == nullptr)
      return;

    float yaw1 = math::rad_2_deg(math::atan2(record1->velocity.y, record1->velocity.x));
    float yaw2 = math::rad_2_deg(math::atan2(record2->velocity.y, record2->velocity.x));

    float yaw_delta   = math::normalize_angle(yaw1 - yaw2);
    float tick_delta  = (float)math::biggest(math::time_to_ticks(record1->simulation_time - record2->simulation_time), 1);

    // If a player holds ticks (with fakelag)
    // We would have a huge delta of say 30 deg per tick
    // If they have a huge delta we would want to divide that by there tick delta to get the true per tick turn
    yaw_delta /= tick_delta;

    // We'll need the actual yaw delta for another calculation.
    float actual_yaw_delta = yaw_delta;

    // Clamp to 330
    yaw_delta = math::clamp(yaw_delta, -5.f, 5.f);

    float lerp_mod = 0.05f * tick_delta;

    data->pred_change = math::lerp(data->pred_change, yaw_delta, lerp_mod);
    {
      if(tick_delta >= 6.f) // We'll assume at this level of tick delta the player is fakelagging.
        data->pred_change_delta = math::normalize_angle(actual_yaw_delta - data->pred_last_yaw_delta) * globaldata->interval_per_tick;
      else
        data->pred_change_delta = math::normalize_angle(data->pred_change - data->last_pred_change);
    }
    data->last_pred_change    = data->pred_change;
    data->pred_last_yaw_delta = actual_yaw_delta;

    if(math::abs(yaw_delta) < 0.01f)
      data->pred_change = 0.f;

    // Was last ticks velocity low? if so zero pred change
    if(record1->velocity.length_2d() < 0.1f || record2->velocity.length_2d() < 0.1f)
      data->pred_change = 0.f;

    //DBG("%f\t\t%f\n", data->pred_change, data->pred_change_delta);
  }

  float calculate_hit_chance(c_base_player* player){
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
    return hc;
  }

  // Gravity is applied half before simulation and half after simulation
  ALWAYSINLINE void apply_gravity(s_entity_predict_ctx* ctx){
    if(ctx == nullptr || ctx->predicted_flags & FL_ONGROUND || ctx->predicted_entity->water_level > 1)
      return;

    float gravity = ctx->predicted_entity->gravity > 0.f ? ctx->predicted_entity->gravity : 1.f;
          // TODO: Apply TFGameRules()->GetGravityMultiplier() to gravity
          gravity *= global->sv_gravity->flt();
          gravity *= 0.5f;
          gravity *= globaldata->interval_per_tick;

    ctx->predicted_velocity.z -= gravity;

    fix_velocity(ctx);
  }

  ALWAYSINLINE void handle_acceleration(s_entity_predict_ctx* ctx){
    if(ctx == nullptr)
      return;

    c_player_data* data = ctx->predicted_entity->data();

    if(data == nullptr)
      return;

    return;

    if(!(ctx->predicted_flags & FL_ONGROUND)){
      float speed           = ctx->predicted_velocity.length_2d();
      float ideal_max       = speed > 0.f ? math::clamp(math::rad_2_deg(math::asin(15.f / speed)), 0.f, 90.f) : 0.f;
      float side_move       = 0.f;
      float velocity_angle  = math::rad_2_deg(math::atan2(ctx->predicted_velocity.y, ctx->predicted_velocity.x));

      if(math::abs(data->pred_change) >= ideal_max)
        side_move = data->pred_change > 0.f ? -450.f : 450.f;

      if(side_move != 0.f)
        air_accelerate(ctx, velocity_angle, 0.f, side_move);
    }
  }

  ALWAYSINLINE void air_accelerate(s_entity_predict_ctx* ctx, float velocity_angle, float forward_move, float side_move){
    vec3 fwd, right, up;
    math::angle_2_vector(vec3(0.f, velocity_angle, 0.f), &fwd, &right, &up);
    {
      fwd.z   = 0.f;
      right.z = 0.f;

      fwd.normalize();
      right.normalize();
    }

    // Calculate wish velocity
    vec3 wish_dir   = fwd * forward_move + right * side_move;
         wish_dir.z = 0.f;

    float wish_speed = wish_dir.normalize();

    DBG("%f\n", wish_speed);

    if(wish_speed != 0.f && wish_speed > ctx->predicted_wish_speed)
      wish_dir *= (ctx->predicted_wish_speed / wish_speed);

    float speed_cap = 30.f; //ctx->predicted_entity->get_air_speed_cap(); (was a tf2 function) no idea if this is in other games.
    if(wish_speed > speed_cap)
      wish_speed = speed_cap;

    float current_speed = ctx->predicted_velocity.dot(wish_dir);

    float add_speed = wish_speed - current_speed;

    if(add_speed <= 0.f)
      return;

    float accel_speed = global->sv_airaccelerate->flt() * wish_speed * globaldata->interval_per_tick * ctx->predicted_entity->surface_friction;

    if(accel_speed > add_speed)
      accel_speed = add_speed;

    ctx->predicted_velocity += wish_dir * accel_speed;
  }

  // Applied when the player would jump
  ALWAYSINLINE void apply_jump_impulse(s_entity_predict_ctx* ctx){
    if(ctx == nullptr || ctx->predicted_flags & FL_ONGROUND)
      return;

    // TODO: multiply this by m_pSurfaceData->game.jumpFactor in CTFGameMovement::CheckJumpButton
    ctx->predicted_velocity.z = math::sqrt(2.f * global->sv_gravity->flt() * 45.f); // 72000
  }

  ALWAYSINLINE void apply_friction(s_entity_predict_ctx* ctx){
    if(ctx == nullptr || !(ctx->predicted_flags & FL_ONGROUND))
      return;

    float len = ctx->predicted_velocity.length_2d();

    if(len == 0.f)
      return;

    float friction  = global->sv_friction->flt() * ctx->predicted_surface_friction;
    float new_speed = math::biggest(0.f, len - math::biggest(len, global->sv_stopspeed->flt()) * (friction * globaldata->interval_per_tick));

    if(new_speed != len){
      ctx->predicted_velocity.x *= new_speed / len;
      ctx->predicted_velocity.y *= new_speed / len;

      fix_velocity(ctx);
    }
  }

  ALWAYSINLINE void update_on_ground(s_entity_predict_ctx* ctx){
    if(ctx == nullptr)
      return;

    s_trace move_tr = global->trace->player_move(ctx->predicted_origin, ctx->predicted_origin - vec3(0.f, 0.f, ctx->predicted_entity->step_size), ctx->predicted_entity);
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
        ctx->predicted_origin.z = move_tr.end.z;
    }
  }

  ALWAYSINLINE void fix_velocity(s_entity_predict_ctx* ctx){
    if(ctx == nullptr)
      return;

    //for(u32 i = 0; i < 3; i++)
    //  ctx->predicted_velocity[i] = math::clamp(ctx->predicted_velocity[i], -global->sv_maxvelocity->flt(), global->sv_maxvelocity->flt());
  }

  ALWAYSINLINE void clip_ground_velocity(s_entity_predict_ctx* ctx){
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
  ALWAYSINLINE bool resolve_stuck(s_entity_predict_ctx* ctx){
    s_trace stuck_tr = global->trace->player_move(ctx->predicted_origin, ctx->predicted_origin, ctx->predicted_entity);

    if(!stuck_tr.started_solid)
      return false;

    // This looks like alot but in reality %99 of cases would be corrected on the first 4 entries
    float offset = ctx->predicted_entity->step_size * 0.1f;
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

  ALWAYSINLINE void physical_move(s_entity_predict_ctx* ctx){
    if(ctx == nullptr)
      return;

    // Apply friction
    //apply_friction(ctx);

    // If we are on the ground ensure the velocity is not over the max speed
    //clip_ground_velocity(ctx);

    s_trace move_tr;
    {
      // Causes prediction to fail on thin surfaces, push players through walls, and messes up movement on sloped surfaces.
      if(resolve_stuck(ctx))
        return;

      // handle circular prediction if we have EP_FLAG_PREDICT_YAW_TURN enabled
      if((ctx->flags & EP_FLAG_PREDICT_YAW_TURN)){
        if(math::abs(ctx->predicted_wish_yaw_turn) > 0.f){
          float vel_len               = ctx->predicted_velocity.length_2d();

          // Take our current turn angle, and add our
          float velocity_angle        = math::normalize_angle(math::rad_2_deg(math::atan2(ctx->predicted_velocity.y, ctx->predicted_velocity.x)) + ctx->predicted_wish_yaw_turn);
          ctx->predicted_velocity.x   = math::cos(math::deg_2_rad(velocity_angle)) * vel_len;
          ctx->predicted_velocity.y   = math::sin(math::deg_2_rad(velocity_angle)) * vel_len;
        }
      }

      // predict one tick of movement
      vec3 end = ctx->predicted_origin + (ctx->predicted_velocity * globaldata->interval_per_tick);

      // start obb trace for physical move
      move_tr = global->trace->player_move(ctx->predicted_origin, end, ctx->predicted_entity);

      // Have we hit something and does it allow us to go over it?
      // If so then teleport our predicted origin to above that surface
      if((ctx->predicted_flags & FL_ONGROUND) && (move_tr.fraction != 1.f || move_tr.started_solid || move_tr.solid)){
        s_trace obstacle_trace = global->trace->player_move(end + vec3(0.f, 0.f, ctx->predicted_entity->step_size), end, ctx->predicted_entity);

        if(!obstacle_trace.started_solid){
          end.z   = obstacle_trace.end.z;
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
          end                   = ctx->predicted_origin + (ctx->predicted_velocity * (globaldata->interval_per_tick * (1.f - move_tr.fraction)));
          move_tr               = global->trace->player_move(ctx->predicted_origin, end, ctx->predicted_entity);
        }
      }

      ctx->predicted_origin = move_tr.end;
    }
  }

  bool predict_tick(s_entity_predict_ctx* ctx){
    if(ctx == nullptr || ctx->predicted_entity == nullptr || ctx->prediction_broken)
      return false;

    // perform prediction
    {
      // Apply half of the gravity
      apply_gravity(ctx);

      // Apply accelerations
      handle_acceleration(ctx);

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

  bool simple(s_entity_predict_ctx* ctx, u32 ticks_to_predict){
    assert(ticks_to_predict > 0);
    assert(ctx != nullptr);

    for(u32 i = 0; i < ticks_to_predict; i++)
      if(!predict_tick(ctx))
        break;

    return ctx->has_been_predicted;
  }
};

CLASS_EXTERN(c_entity_prediction, entity_prediction);