#pragma once

//#define DOUBLE_TAP_DEBUG

enum dt_state{
  DT_STATE_READY = 0,
  DT_STATE_READY_ATTACK2_ONLY,
  DT_STATE_READY_ATTACK1_2,
  DT_STATE_CHARGING,
  DT_STATE_NOT_ENOUGH_CHARGE,
  DT_STATE_NO_CHARGE,
  DT_STATE_DT_IMPOSSIBLE,
  DT_STATE_GROUND_ONLY,
};

class c_double_tap{
public:
  bool should_shift  = false;
  bool is_shifting   = false;
  bool is_warping    = false;
  bool is_charging   = false;
  bool force_charge  = false;
  i32  shift_end     = 0;
  i32  charged_ticks = 0;

  ALWAYSINLINE void reset(){
    memset(this, 0, sizeof(*this));
  }

  u32  get_wanted_charge_ticks();
  void adjust_player_time_base(i32 simulation_ticks);
  void auto_stop();
  bool is_shift_tick();
  bool wants_to_shift();
  bool is_dt_active();
  bool has_any_charged_ticks();
  dt_state get_charge_state();
  bool can_weapon_dt();
  bool shift(bool aimbot_running);
  bool should_be_passive_charging();
  bool handle_charge(float extra_samples);
  bool cl_move_run(float extra_samples, bool final_tick);
  bool cl_move_run_warp(float extra_samples, bool final_tick);
};

CLASS_EXTERN(c_double_tap, double_tap);