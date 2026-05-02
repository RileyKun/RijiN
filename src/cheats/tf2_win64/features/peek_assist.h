#pragma once

#define PEEK_ASSIST_NEXT_TIME 0.6f
class c_peek_assist{
public:
  bool  has_set_return_position = false;
  bool  has_shot_weapon         = false;
  vec3  return_position;
  float next_effect_time;

  void handle_effects(vec3 pos, bool first_time){
    if(!first_time && math::time() < next_effect_time)
      return;

    next_effect_time = math::time() + PEEK_ASSIST_NEXT_TIME;

    utils::dispatch_particle_effect(XOR("ping_circle"), pos);

    if(!first_time)
      return;

    utils::dispatch_particle_effect(XOR("drain_effect"), pos);
    utils::dispatch_particle_effect(XOR("xms_ornament_glitter"), pos);
  }

  void auto_stop(){
    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr)
      return;

    vec3 vel = localplayer->velocity();
    vel.z = 0.f;

    float speed = vel.length_2d();

    if(speed < 1.f){
      global->current_cmd->move.x = 0.f;
      global->current_cmd->move.y = 0.f;
      return;
    }

    vec3 negative_dir = (vel * -1.f);
    vec3 negative_ang;
    math::vector_2_angle(&negative_dir, &negative_ang);

    negative_ang.y = global->current_cmd->view_angles.y - negative_ang.y;
    math::angle_2_vector(negative_ang, &negative_dir, nullptr, nullptr);
    negative_dir.normalize();

    global->current_cmd->move.x = negative_dir.x * 450.f;
    global->current_cmd->move.y = negative_dir.y * 450.f;
  }

  vec3 frictioned_velocity(){
    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr)
      return vec3();

    if(!(localplayer->entity_flags() & FL_ONGROUND))
      return vec3();

    vec3  vel = localplayer->velocity();
    float len = localplayer->velocity().length_2d();

    if(len == 0.f)
      return vec3();

    float friction  = global->sv_friction->flt * localplayer->surface_friction();
    float new_speed = math::biggest(0.f, len - math::biggest(len, global->sv_stopspeed->flt) * (friction * globaldata->interval_per_tick));

    if(new_speed != len){
      vel.x *= new_speed / len;
      vel.y *= new_speed / len;

      for(u32 i = 0; i < 3; i++)
        vel[i] = math::clamp(vel[i], -global->sv_maxvelocity->flt, global->sv_maxvelocity->flt);
    }

    return vel;
  }

  bool can_use(){
    if(!config->hvh.peek_assist)
      return false;

    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr)
      return false;

    c_base_weapon* wep = localplayer->get_weapon();

    if(wep == nullptr)
      return false;

    if(wep->is_medigun() || wep->is_flamethrower() || wep->is_dragons_fury())
      return false;

    return true;
  }

  void pre_run(){
    if(!can_use())
      return;

    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr)
      return;

    static bool last_is_pressing_key;
    bool        is_pressing_key = input_system->held(config->hvh.peek_assist_hotkey);

    if(has_set_return_position && !is_pressing_key)
      has_set_return_position = false;
    else if(has_set_return_position){
      handle_effects(return_position, false);

      if(has_shot_weapon){
        float d = (localplayer->origin() - return_position).length_2d();
        float v = frictioned_velocity().length_2d();
        float t = math::clamp(d / v, 0.f, 1.f);

        // when we get in the region of the goal area then cancel
        if(math::time_to_ticks(t) <= 1)
          has_set_return_position = false;
        else{
          // move in that direction
          global->original_cmd.view_angles.y = math::calc_view_angle(localplayer->shoot_pos(), return_position).y;
          global->current_cmd->move          = vec3(450.f, 0.f, 0.f);
          global->current_cmd->buttons      &= ~IN_JUMP;
        }
      }
    }
    else if(is_pressing_key && !last_is_pressing_key){
      // trace to ground and get our return position
      s_trace to_ground = global->trace->player_move(localplayer->origin(), localplayer->origin() - vec3(0.f, 0.f, 8129.f), localplayer);

      // create particle effect
      handle_effects(to_ground.end, true);

      has_set_return_position = true;
      has_shot_weapon         = false;
      return_position         = to_ground.end;
    }

    last_is_pressing_key = is_pressing_key;
  }

  void post_run(){
    if(!has_set_return_position)
      return;

    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr)
      return;

    if(localplayer->can_fire() && (global->current_cmd->buttons & IN_ATTACK))
      has_shot_weapon = true;
  }
};

CLASS_EXTERN(c_peek_assist, peek_assist);