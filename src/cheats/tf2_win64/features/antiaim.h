#pragma once

struct s_batched_angle{
  vec3 angle;
  bool is_batched;
};

class c_antiaim{
public:
  void run();
  void run_on_fire();
  bool set_choke(bool choke);
  void setup_antiaims();
  void pitch(c_base_player* localplayer);
  void yaw();
  void adjust_real_rotate_angle(float& angle);
  void clamp_and_normalize_angle(float& angle);
  void auto_edge();
  void break_animations(bool pre_prediction);
  void thirdperson(u32 stage);
  bool should_antiaim();
  bool is_faking_yaw();
  void stabilize_yaw();
  void center_real_yaw();
  void pre_prediction();
  bool get_target_angle(i32 type, vec3* angle, vec3* pos = nullptr); // Returns true if the position parameter was filled.
  void find_headshot_classes();
  void process_input();
  void process_cycle();
  float get_best_pitch(c_base_player* localplayer);
  bool fake_duck(bool pre_prediction);
  bool get_fake_angle(vec3& fake_angle);

  bool get_auto_yaw_angle(c_base_player* player, float current_yaw, vec3 target_pos, float radius, std::vector<float> list, float& angle);

  ALWAYSINLINE void reset(){
    headshot_class   = 0;
    
    current_goal_yaw = 0.f;
    feet_yaw         = 0.f;

    using_fake_pitch = false;
    antiaim_toggle   = false;
    antiaim_enabled  = false;
    init_goal_yaw    = false;

    memset(&batched_angle, 0, sizeof(batched_angle));
  }

  void compute_aim_yaw();  
  void converge_yaw_angles(float goal_yaw, float& current_yaw);
  void add_to_batch(vec3 angle, bool sending);

  i32 headshot_class       = 0;
  bool using_fake_pitch      = false;
  bool antiaim_toggle        = false;
  bool antiaim_enabled       = false;
  bool antiaim_is_first_time = false;
  bool antiaim_send_packet   = false; // Used to alert us upon modifying send packet values after yaw anti-aim was done.
  bool antiaim_on_fire       = false; // We're firing a shot.

  bool init_goal_yaw     = false;
  float current_goal_yaw = 0.f;
  float feet_yaw         = 0.f;
  s_batched_angle batched_angle[26];
  float antiaim_active_headshot_threat;
};

CLASS_EXTERN(c_antiaim, antiaim);