#pragma once

#define ROCKET_JUMPER_TRACE_SIZE 96.f
#define ROCKET_JUMPER_OBSTRUCTION_ABOVE_SIZE 272.f

class c_auto_rocket_jumper{
public:

  void run();
  bool should_target_angle(float& pitch, float& yaw, i32 scan = -1);

  ALWAYSINLINE void null_fire_buttons(){
    global->current_cmd->buttons &= ~IN_ATTACK;
    global->current_cmd->buttons &= ~IN_ATTACK2;
  }

  bool can_weapon_blast_jump();

  bool should_auto_jump();

  ALWAYSINLINE vec3 create_trace_point(vec3 trace_pos, float pitch, float yaw, float length){
    vec3 fwd;
    math::angle_2_vector(vec3(pitch, yaw, 0.f), &fwd, nullptr, nullptr);
    return (fwd * length) + trace_pos;
  }

  float target_angle_time   = 0.f;
  float best_pitch          = 0.f;
  float best_yaw            = 0.f;

  bool  is_performing_jump  = false;
};

CLASS_EXTERN(c_auto_rocket_jumper, auto_rocket_jumper);