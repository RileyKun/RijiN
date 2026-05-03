#pragma once

class c_interface_prediction{
public:
  PAD(0x4);
  u32   last_ground;
  bool  in_prediction;
  bool  first_time_predicted;
  bool  old_cl_predict_value;
  bool  engine_paused;
  i32   previous_start_frame;
  i32   commands_predicted;

  ALWAYSINLINE void setup_move(void* ent, s_user_cmd* cmd, void* move_helper, u8* move_data){
    utils::virtual_call<18, void, void*, s_user_cmd*, void*, u8*>(this, ent, cmd, move_helper, move_data);
  }

  ALWAYSINLINE void run_command(c_base_player* p, s_user_cmd* cmd, void* move_helper){
    utils::virtual_call<17, void, c_base_player*, s_user_cmd*, void*>(this, p, cmd, move_helper);
  }

  ALWAYSINLINE void set_local_view_angles(vec3 angle){
    utils::virtual_call<13, void, vec3&>(this, angle);
  }
};