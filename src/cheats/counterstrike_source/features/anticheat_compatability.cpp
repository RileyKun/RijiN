#include "../link.h"

c_anti_cheat_manager* acm = nullptr;

bool c_anti_cheat_manager::is_anti_aim_allowed(){
  return false;
}

bool c_anti_cheat_manager::has_to_angle_repeat(){
  // Shouldn't be a problem in CS:S :TM:
  return false;
}

u32 c_anti_cheat_manager::get_play_style(bool cache){
  static u32 cached_result = ACM_UNKNOWN;

  return cached_result;
}

bool c_anti_cheat_manager::can_back_track(){
  return true;
}

bool c_anti_cheat_manager::should_allow_unclamped_angles(){
  return false;
}

bool c_anti_cheat_manager::should_allow_command_number_changes(){
  return true;
}

bool c_anti_cheat_manager::can_hide_angle_snaps(){
  return true;
}