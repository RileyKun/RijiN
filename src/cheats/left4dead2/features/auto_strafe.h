#pragma once

class c_auto_strafe{
public:
  float mod;
  float old_yaw;
  float vel_yaw;

  bool will_hit_obstacle_in_future(float wish_yaw, float current_yaw, float predict_time, float step, vec3* hit_plane = nullptr){
    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr)
      return false;

    vec3  velocity                = localplayer->velocity();
    float velocity_len            = velocity.length_2d();
    float velocity_angle          = math::rad_2_deg(math::atan2(velocity.y, velocity.x));
    float gravity_per_half_tick   = global->sv_gravity->flt * 0.5f * globaldata->interval_per_tick;
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
        velocity.z = math::sqrt(2.f * global->sv_gravity->flt * 45.f); // 72000

      // physical move
      s_trace move_tr;
      {
        // predict one tick of movement
        vec3 end = start + (velocity * globaldata->interval_per_tick);
        move_tr = global->trace->ray_obb(start, end, mins, maxs, mask_playersolid);

        // If we hit something and then plane is higher than -1 and lower than 0.7, we have hit a wall
        // If the plane is -1 then we are hitting a ceiling, anything over 0.7 is considered a walkable slope
        // Calculate its adjustment and re-start a trace from there so we will trace up the plane
        if(move_tr.fraction != 1.f && move_tr.plane.normal.z >= 0.7f){
          velocity -= move_tr.plane.normal * velocity.dot(move_tr.plane.normal);

          float adjust = velocity.dot(move_tr.plane.normal);
          if(adjust < 0.f)
            velocity -= move_tr.plane.normal * adjust;

          start   = move_tr.end;
          end     = start + (velocity * (globaldata->interval_per_tick * (1.f - move_tr.fraction)));
          move_tr = global->trace->ray_obb(start, end, mins, maxs, mask_playersolid);
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
        s_trace gc_tr = global->trace->ray_obb(move_tr.end, move_tr.end - vec3(0.f, 0.f, 2.f), mins, maxs, mask_playersolid);
        on_ground = (gc_tr.fraction != 1.f || gc_tr.started_solid || gc_tr.solid) && gc_tr.plane.normal.z >= 0.70f;
      }
    }

    return false;
  }

  bool should_avoid_wall(float looking_yaw, float wish_yaw, float current_yaw, float predict_time, float& new_yaw, float ideal){
    vec3 plane;
    if(!will_hit_obstacle_in_future(wish_yaw, current_yaw, predict_time, ideal, &plane))
      return false;

    new_yaw  = math::normalize_angle(math::rad_2_deg(math::atan2(plane.y, plane.x)));
    new_yaw  = math::normalize_angle(new_yaw + ((math::normalize_angle(new_yaw - looking_yaw) > 0.f) ? -90.f : 90.f));

    return true;
  }

  void run(){
    if(!config->misc.auto_strafe)
      return;

    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr)
      return;

    if(localplayer->water_level() > 1)
      return;

    if(localplayer->move_type() != MOVETYPE_WALK)
      return;

    // do prespeed for jump to gain instant velocity
    {
      static u32 jump_ticks = 0;
      jump_ticks = (global->original_cmd.buttons & IN_JUMP) ? (jump_ticks + 1) : 0;

      if(jump_ticks > 0 && jump_ticks <= math::time_to_ticks(0.125f)/*5 TICKS on 66*/ && global->original_cmd.move.length_2d() == 0.f){
        global->current_cmd->move.x = 450.f;
        global->current_cmd->buttons &= ~IN_JUMP;
      }
    }

    if(localplayer->entity_flags() & FL_ONGROUND)
      return;

    vec3  velocity    = localplayer->velocity();
          velocity.z  = 0.f;

    float original_yaw    = global->original_cmd.view_angles.y;
    float velocity_angle  = math::rad_2_deg(math::atan2(velocity.y, velocity.x));
    float speed           = velocity.length_2d();
    float retrack         = 15.f;
    float max_retrack     = 90.f;
    float ideal_max       = speed > 0.f ? math::clamp(math::rad_2_deg(math::asin(15.f / speed)), 0.f, 90.f) : 0.f;
    float ideal2_max      = speed > 0.f ? math::clamp(math::rad_2_deg(math::asin(30.f / speed)), 0.f, 90.f) : 0.f;
    float ideal           = ideal_max;
    float ideal_turn      = ideal2_max;
          mod             = (mod == 0.f) ? 1.f : (mod * -1.f);

    if(config->misc.auto_strafe_type_directional){
      if(global->original_cmd.move.x != 0.f || global->original_cmd.move.y != 0.f){
        float dir = math::rad_2_deg(math::atan2(-global->original_cmd.move.y, global->original_cmd.move.x));

        if(dir != 0.f){
          // apply smoothing for retrack
          if(config->misc.auto_strafe_retrack > 0.f){
            float mod = math::clamp(config->misc.auto_strafe_retrack / 100.f, 0.f, 1.f);
            ideal_turn *= mod;
          }

          global->original_cmd.view_angles.y = math::normalize_angle(global->original_cmd.view_angles.y + dir);
        }
      }
    }

    if(!config->misc.auto_strafe_avoid_walls_disabled){
      float new_yaw;
      float ideal_step = config->misc.auto_strafe_avoid_walls_adv ? ideal_turn : 0.f;
      if(ideal_turn > 0.f && should_avoid_wall(original_yaw, global->original_cmd.view_angles.y, velocity_angle, 2.f, new_yaw, ideal_step)){

        // If we are moving backwards but towards a wall, invert our direction
        // For example; if we are moving towards a wall and looking to the left of it, we will move to the left
        // If we are moving to the left of the wall but we are behind, we are going to go to the right
        // This code will correct this problem
        float dir_correct = math::abs(math::normalize_angle(original_yaw - global->original_cmd.view_angles.y)) > 90.f ? 180.f : 0.f;
        new_yaw += dir_correct;

        // now we know we will hit a wall, lets calculate the ticks we have to move out of the way
        // + 1 = to move out of the way for next tick
        // + 1 + 1 = because its going to be the next actual tick we could start
        i32 ticks_to_move = math::clamp((i32)math::floor(math::abs(math::normalize_angle(new_yaw - velocity_angle)) / ideal2_max), 1, math::time_to_ticks(2.f)) + 1 + 1;

        // now we predict the ticks required to move out of the way from the plane
        // if we have to move we should do it as soon as possible
        if(should_avoid_wall(original_yaw, global->original_cmd.view_angles.y, velocity_angle, math::ticks_to_time(ticks_to_move), new_yaw, ideal_step)){
          new_yaw                            += dir_correct;
          ideal_turn                          = ideal2_max;
          global->original_cmd.view_angles.y  = new_yaw;

          //DBG("%f\n", global->original_cmd.view_angles.y);
          //vec3 out;
          //math::angle_2_vector(vec3(0.f, global->original_cmd.view_angles.y, 0.f), &out, nullptr, nullptr);
          //bullet_tracers->add(localplayer->shoot_pos(), localplayer->shoot_pos() + (out * 8192.f), rgb(255,255, 0), globaldata->interval_per_tick);
        }
      }
    }

    vec3  wish_view   = global->original_cmd.view_angles;
    float yaw_delta   = math::normalize_angle(wish_view.y - old_yaw);
          old_yaw     = wish_view.y;

    float velocity_delta = math::normalize_angle(wish_view.y - velocity_angle);

    if(math::abs(velocity_delta) > ideal_turn){
      float ideal_mod = math::clamp(velocity_delta, -ideal_turn, ideal_turn);

      global->original_cmd.view_angles.y = velocity_angle + ideal_mod;
      global->current_cmd->move.y        = ideal_mod > 0.f ? -450.f : 450.f;
    }
    else{
      global->original_cmd.view_angles.y  = wish_view.y + (ideal * mod);
      global->current_cmd->move.y         = 450.f * mod;
    }

    global->current_cmd->move.x = 0.f;
  }
};

CLASS_EXTERN(c_auto_strafe, auto_strafe);