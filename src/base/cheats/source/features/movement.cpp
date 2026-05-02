#include "../../../link.h"

bool c_base_movement::pre_predicted_create_move(c_internal_base_player* localplayer, c_internal_base_weapon* wep, c_user_cmd* cmd){
  if(localplayer == nullptr)
    return false;

  bunnyhop(localplayer, cmd);
  auto_strafe(localplayer, cmd);

  return false;
}

bool c_base_movement::can_perform_movement(c_internal_base_player* player, bool skip_button_check){
  if(player == nullptr)
    return false;

  if(!player->is_base_alive() || player->is_dormant())
    return false;

  if(player->water_level() > 1)
    return false;

  if(player->move_type() != MOVETYPE_WALK)
    return false;

  if(player->entity_flags() & FL_ONGROUND || !(global_internal->original_cmd.buttons & IN_JUMP) && !skip_button_check)
    return false;

  return true;
}

bool c_base_movement::can_perform_double_jump(c_internal_base_player* localplayer){
  return false;
}

void c_base_movement::bunnyhop(c_internal_base_player* localplayer, c_user_cmd* cmd){
  global_internal->double_jumping = false;

  c_movement_settings* settings = get_settings();
  if(settings == nullptr)
    return;

  if(!settings->bunnyhop)
    return;

  if(!can_perform_movement(localplayer))
    return;

  if(can_perform_double_jump(localplayer)){
    global_internal->double_jumping = !(global_internal->last_cmd.buttons & IN_JUMP) && (global_internal->original_cmd.buttons & IN_JUMP);
    if(global_internal->double_jumping)
      return;
  }

  cmd->buttons &= ~IN_JUMP;
}


void c_base_movement::auto_strafe(c_internal_base_player* localplayer, c_user_cmd* cmd){
  c_movement_settings* settings = get_settings();
  if(settings == nullptr)
    return;

  if(!settings->auto_strafe)
    return;

  auto_strafe_pre_speed(localplayer, cmd);
  if(!can_perform_movement(localplayer, true) || global_internal->double_jumping)
    return;

  static c_auto_strafer_data data;
  data.init(localplayer, &global_internal->original_cmd);

  auto_strafe_init_base();
  auto_strafe_directional_smoothing(localplayer, cmd, &data);
  auto_strafe_avoid_walls(localplayer, cmd, &data);
  
  vec3  wish_view    = global_internal->original_cmd.view_angles;
  float yaw_delta    = math::normalize_angle(wish_view.y - old_yaw);
        old_yaw = wish_view.y;

  float velocity_delta = math::normalize_angle(wish_view.y - data.velocity_angle);

  if(math::abs(velocity_delta) > data.ideal_turn){
    float ideal_mod = math::clamp(velocity_delta, -data.ideal_turn, data.ideal_turn);

    global_internal->original_cmd.view_angles.y = data.velocity_angle + ideal_mod;
    cmd->move.y        = ideal_mod > 0.f ? -450.f : 450.f;
  }
  else{
    global_internal->original_cmd.view_angles.y  = wish_view.y + (data.ideal * mod);
    cmd->move.y         = 450.f * mod;
  }

   cmd->move.x = 0.f;
}

void c_base_movement::auto_strafe_pre_speed(c_internal_base_player* localplayer, c_user_cmd* cmd){
  c_movement_settings* settings = get_settings();
   if(settings == nullptr)
    return;

  if(!settings->auto_strafe_pre_speed)
    return;

  if(!(localplayer->entity_flags() & FL_ONGROUND))
    return;

  static u32 jump_ticks = 0;
  jump_ticks = (global_internal->original_cmd.buttons & IN_JUMP) ? (jump_ticks + 1) : 0;

  if(jump_ticks > 0 && jump_ticks <= math::time_to_ticks(0.125f)/*5 TICKS on 66*/ && global_internal->original_cmd.move.length_2d() == 0.f){
    cmd->move.x   = 450.f;
    cmd->buttons &= ~IN_JUMP;
  }
}

void c_base_movement::auto_strafe_directional_smoothing(c_internal_base_player* localplayer, c_user_cmd* cmd, c_auto_strafer_data* data){
  c_movement_settings* settings = get_settings();
  if(settings == nullptr)
    return;

  if(!settings->auto_strafe_type_directional)
    return;

  if(global_internal->original_cmd.move.x == 0.0f && global_internal->original_cmd.move.y == 0.0f)
    return;

  const float dir = math::rad_2_deg(math::atan2(-global_internal->original_cmd.move.y, global_internal->original_cmd.move.x));
  if(dir == 0.0f)
    return;

  if(settings->auto_strafe_retrack > 0.f)
    data->ideal_turn *= math::clamp(settings->auto_strafe_retrack / 100.f, 0.f, 1.f);

  global_internal->original_cmd.view_angles.y = math::normalize_angle(global_internal->original_cmd.view_angles.y + dir);
}

void c_base_movement::auto_strafe_avoid_walls(c_internal_base_player* localplayer, c_user_cmd* cmd, c_auto_strafer_data* data){
  c_movement_settings* settings = get_settings();
  if(settings == nullptr)
    return;

  if(settings->auto_strafe_avoid_walls_disabled)
    return;

  float new_yaw;
  float ideal_step = settings->auto_strafe_avoid_walls_adv ? data->ideal_turn : 0.f;
  if(data->ideal_turn > 0.f && should_avoid_wall(data->original_yaw, global_internal->original_cmd.view_angles.y, data->velocity_angle, 2.f, new_yaw, ideal_step)){

    // If we are moving backwards but towards a wall, invert our direction
    // For example; if we are moving towards a wall and looking to the left of it, we will move to the left
    // If we are moving to the left of the wall but we are behind, we are going to go to the right
    // This code will correct this problem
    float dir_correct = math::abs(math::normalize_angle(data->original_yaw - global_internal->original_cmd.view_angles.y)) > 90.f ? 180.f : 0.f;
    new_yaw += dir_correct;

    // now we know we will hit a wall, lets calculate the ticks we have to move out of the way
    // + 1 = to move out of the way for next tick
    // + 1 + 1 = because its going to be the next actual tick we could start
    i32 ticks_to_move = math::clamp((i32)math::floor(math::abs(math::normalize_angle(new_yaw - data->velocity_angle)) / data->ideal2_max), 1, math::time_to_ticks(2.f)) + 1 + 1;

    // now we predict the ticks required to move out of the way from the plane
    // if we have to move we should do it as soon as possible
    if(should_avoid_wall(data->original_yaw, global_internal->original_cmd.view_angles.y, data->velocity_angle, math::ticks_to_time(ticks_to_move), new_yaw, ideal_step)){
      new_yaw          += dir_correct;
      data->ideal_turn = data->ideal2_max;
      global_internal->original_cmd.view_angles.y  = new_yaw;
    }
  }
}

bool c_base_movement::will_hit_obstacle_in_future(float wish_yaw, float current_yaw, float predict_time, float step, vec3* hit_plane){
  c_internal_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_generic_world_only_trace_filter filter;
  filter.ignore_entity = localplayer;

  vec3  velocity                = localplayer->velocity();
  float velocity_len            = velocity.length_2d();
  float velocity_angle          = math::rad_2_deg(math::atan2(velocity.y, velocity.x));
  float gravity_per_half_tick   = global_internal->sv_gravity->flt * 0.5f * global_internal->global_data->interval_per_tick;
  bool  on_ground               = localplayer->entity_flags() & FL_ONGROUND;
  vec3  start                   = localplayer->origin();
  vec3  mins                    = localplayer->obb_mins();
  vec3  maxs                    = localplayer->obb_maxs();
        maxs.z                  *= 0.75f;

  for(i32 i = 0; i < math::time_to_ticks(predict_time); i++){
    if(step > 0.f){
      float velocity_delta = math::normalize_angle(wish_yaw - velocity_angle);

      if(math::abs(velocity_delta) > step){
        float ideal_mod = math::clamp(velocity_delta, -step, step);
        current_yaw += ideal_mod;
      }
      else
        current_yaw = wish_yaw;
    }

    velocity.x = math::cos(math::deg_2_rad(current_yaw)) * velocity_len;
    velocity.y = math::sin(math::deg_2_rad(current_yaw)) * velocity_len;

    // startgravity
    if(!on_ground)
      velocity.z -= gravity_per_half_tick;
    else
      // TODO: multiply this by m_pSurfaceData->game.jumpFactor in CTFGameMovement::CheckJumpButton
      velocity.z = math::sqrt(2.f * global_internal->sv_gravity->flt * 45.f); // 72000

    // physical move
    s_trace move_tr;
    {
      // predict one tick of movement
      vec3 end = start + (velocity * global_internal->global_data->interval_per_tick);
      move_tr = global_internal->base_trace->internal_ray_obb(filter, start, end, mins, maxs, mask_playersolid);

      // If we hit something and then plane is higher than -1 and lower than 0.7, we have hit a wall
      // If the plane is -1 then we are hitting a ceiling, anything over 0.7 is considered a walkable slope
      // Calculate its adjustment and re-start a trace from there so we will trace up the plane
      if(move_tr.fraction != 1.f && move_tr.plane.normal.z >= 0.7f){
        velocity -= move_tr.plane.normal * velocity.dot(move_tr.plane.normal);

        float adjust = velocity.dot(move_tr.plane.normal);
        if(adjust < 0.f)
          velocity -= move_tr.plane.normal * adjust;

        start   = move_tr.end;
        end     = start + (velocity * (global_internal->global_data->interval_per_tick * (1.f - move_tr.fraction)));
        move_tr = global_internal->base_trace->internal_ray_obb(filter, start, end, mins, maxs, mask_playersolid);
      }

      // If we hit something and then plane is higher than -1 and lower than 0.7, we have hit a wall
      // If the plane is -1 then we are hitting a ceiling, anything over 0.7 is considered a walkable slope
      if(move_tr.fraction != 1.f && move_tr.plane.normal.z > -1.f && move_tr.plane.normal.z < 0.7f || move_tr.started_solid || move_tr.solid){
        if(hit_plane != nullptr)
          *hit_plane = move_tr.plane.normal;

        return true;
      }

      start = move_tr.end;
    }

    // finishgravity
    if(!on_ground)
      velocity.z -= gravity_per_half_tick;

    // recalculate onground
    // TODO: fix this
    {
      s_trace gc_tr = global_internal->base_trace->internal_ray_obb(filter, move_tr.end, move_tr.end - vec3(0.f, 0.f, 2.f), mins, maxs, mask_playersolid);
      on_ground = (gc_tr.fraction != 1.f || gc_tr.started_solid || gc_tr.solid) && gc_tr.plane.normal.z >= 0.70f;
    }
  }

  return false;
}

bool c_base_movement::should_avoid_wall(float looking_yaw, float wish_yaw, float current_yaw, float predict_time, float& new_yaw, float ideal){
  vec3 plane;
  if(!will_hit_obstacle_in_future(wish_yaw, current_yaw, predict_time, ideal, &plane))
    return false;

  new_yaw  = math::normalize_angle(math::rad_2_deg(math::atan2(plane.y, plane.x)));
  new_yaw  = math::normalize_angle(new_yaw + ((math::normalize_angle(new_yaw - looking_yaw) > 0.f) ? -90.f : 90.f));

  return true;
}
