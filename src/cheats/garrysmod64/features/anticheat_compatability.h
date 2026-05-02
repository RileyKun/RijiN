#pragma once


enum feature_unsupport_flags{
  FUNC_SILENT_AIM = (1 << 1),
  FUNC_MOVE_FIX = (1 << 2),
  FUNC_SCREEN_GRAB = (1 << 3),
  FUNC_SCREEN_GRAB_NO_2D = (1 << 4)
};

class c_acp{
public:
  bool should_fix_move;
  u32  support_flag;

  ALWAYSINLINE void reset(){
    memset(this, 0, sizeof(c_acp));
  }

  void calculate_feature_support();
  void calculate_should_move_fix(s_user_cmd* cmd);
  void post_create_move(s_user_cmd* cmd);
  void clamp_viewangles(s_user_cmd* cmd);
  void clamp_movement(s_user_cmd* cmd);
  void set_mouse(s_user_cmd* cmd);

  ALWAYSINLINE bool is_blocked(u32 i){
    return support_flag & (i << 1);
  }

  ALWAYSINLINE void toggle_from_bool(u32 i, bool r){
    if(r)
      support_flag |= (i << 1);
    else
      support_flag &= ~(i << 1);
  }

  ALWAYSINLINE void add_flag(u32 i){
    support_flag |= (i << 1);
  }

  ALWAYSINLINE void remove_flag(u32 i){
    support_flag &= ~(i << 1);
  }

  bool should_fix_movement(){
    return should_fix_move && !is_blocked(FUNC_MOVE_FIX);
  }
};

CLASS_EXTERN(c_acp, acp);