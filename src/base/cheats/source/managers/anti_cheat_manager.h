#pragma once

enum anti_cheat_manager_tick_info{
  ACM_LAST_TICK = 0,
  ACM_HISTORY_TICK1,
  ACM_HISTORY_TICK2,
  ACM_MAX_COUNT,
};

enum anti_cheat_manager_trigger_info{
  ACM_NO_TRIGGER = 0,
  ACM_TRIGGER_ON_SET, // Like if we add a button it'll trigger a detection.
  ACM_TRIGGER_WITHOUT_SET, // If we don't add a button it'll trigger a detection.
  ACM_TRIGGER_AUTO_BAN,
};

enum anti_cheat_manager_cheating_style{
  ACM_UNKNOWN = 0,
  ACM_RAGE = 1,
  ACM_SEMI_RAGE = 2,
  ACM_LEGIT = 3
};

class c_anti_cheat_manager_settings{
public:
  union{
    struct{
      bool allow_command_number_manip = false;
      bool allow_unclamped_angles     = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_base_anti_cheat_manager{
public:
  c_user_cmd untouched_cmd;
  c_user_cmd history_cmd[ACM_MAX_COUNT];
  bool       silent_aim_last_tick;


  virtual void reset(){
    DBG("[!] c_base_anti_cheat_manager::reset\n");
    memset(&untouched_cmd, 0, sizeof(c_user_cmd));
    memset(&history_cmd, 0, sizeof(history_cmd));
    silent_aim_last_tick      = false;
  }

  virtual bool is_anti_aim_allowed(){
    return false;
  }

  // You should put features in here that are considered rage hack like and what not and use the returns from anti_cheat_manager_cheating_style.
  virtual u32 get_play_style(bool cache = false){
    FUNCTION_OVERRIDE_NEEDED;
    return ACM_UNKNOWN;
  }

  virtual bool can_back_track(){
    return true;
  }

  virtual bool wants_aim_stepping(){
    return false;
  }

  virtual bool has_to_angle_repeat(){
    return false;
  }

  virtual bool should_allow_unclamped_angles(){
    #if defined(TF2_CHEAT)
      return true;
    #else
      return false;
    #endif
  }

  virtual bool should_allow_command_number_changes(){
    #if defined(TF2_CHEAT) || defined(L4D_CHEAT)
      return true;
    #else
      return false;
    #endif
  }

  virtual float get_maximum_movement_size(){
    #if defined(GMOD_CHEAT)
      return 10000.f;
    #else
      return 450.f;
    #endif
  }


  virtual bool can_hide_angle_snaps(){
    return true;
  }

  i32 is_single_tick_input(c_user_cmd* cmd, i32 flag){
    if(cmd->buttons & flag && !(history_cmd[ACM_LAST_TICK].buttons & flag) && history_cmd[ACM_HISTORY_TICK1].buttons & flag)
      return ACM_TRIGGER_ON_SET;

    if(!(cmd->buttons & flag) && history_cmd[ACM_LAST_TICK].buttons & flag && !(history_cmd[ACM_HISTORY_TICK1].buttons & flag))
      return ACM_TRIGGER_WITHOUT_SET;

    return ACM_NO_TRIGGER;
  }

  void counter_single_tick_presses(c_user_cmd* cmd, i32 bit_flag){
    assert(cmd != nullptr);

    i32 input = is_single_tick_input(cmd, bit_flag);
    if(input == ACM_TRIGGER_ON_SET){
      DBG("[!] counter_single_tick_presses buttons bit flag: %i would trigger single tick on set, removing flag\n", bit_flag);
      cmd->buttons &= ~bit_flag;
    }
    else if(input == ACM_TRIGGER_WITHOUT_SET){
      DBG("[!] counter_single_tick_presses buttons bit flag: %i would trigger single tick if not set, adding flag\n", bit_flag);
      cmd->buttons |= bit_flag;
    }
  }

  float get_angle_snap(c_user_cmd* cmd){
    assert(cmd != nullptr);
    return math::abs(math::normalize_angle((math::clamp_angles(cmd->view_angles) - math::clamp_angles(untouched_cmd.view_angles)).length()));
  }

  i32 is_angle_repeat(c_user_cmd* cmd){
    assert(cmd != nullptr);

    vec3 ang1 = math::round_vector(math::clamp_angles(cmd->view_angles));
    vec3 ang2 = math::round_vector(math::clamp_angles(history_cmd[ACM_LAST_TICK].view_angles));
    vec3 ang3 = math::round_vector(math::clamp_angles(history_cmd[ACM_HISTORY_TICK1].view_angles));

    if(ang1.y == ang3.y && ang1.y != ang2.y)
      return ACM_TRIGGER_WITHOUT_SET;

    return ACM_NO_TRIGGER;
  }

  bool is_silent_aim_active(c_user_cmd* cmd){
    return get_angle_snap(cmd) >= 1.f; // anti-cheats check for a delta equal or greater than one degree for angle-repeats.
  }

  void pre_create_move(c_user_cmd* cmd);
  void post_create_move(c_user_cmd* cmd, bool post_movement_fix);
  bool on_process_print_hook(c_svc_print* msg);
  bool on_process_query_convar_hook(c_clc_respond_cvar_value* msg);
  virtual void on_angle_modified(c_user_cmd* cmd);
};