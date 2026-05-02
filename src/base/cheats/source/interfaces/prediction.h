#pragma once

class c_interface_prediction{
public:
  void* rtti;
  u32   last_ground;
  bool  in_prediction;
  bool  first_time_predicted;
  bool  old_cl_predict_value;
  bool  engine_paused;
  i32   previous_start_frame;
  i32   commands_predicted;

  ALWAYSINLINE void run_command(c_internal_base_player* p, c_user_cmd* cmd, void* move_helper){
    assert(p != nullptr);
    assert(cmd != nullptr);
    assert(move_helper != nullptr);
    utils::internal_virtual_call<void, c_internal_base_player*, c_user_cmd*, void*>(base_interface->run_command_index, this, p, cmd, move_helper);
  }
};