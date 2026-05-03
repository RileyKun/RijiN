#pragma once

class c_freecamera{
public:
  c_key_control key;
  bool          pos_setup;
  vec3          pos;
  vec3          freeze_angle;
  bool          was_freecam_active;

  void reset(){
    memset(this, 0, sizeof(c_freecamera));
  }

  bool is_active();
  bool run(s_view_setup* view_setup, i32* what_to_draw);
  bool run_createmove(s_user_cmd* cmd);
};

CLASS_EXTERN(c_freecamera, freecam);