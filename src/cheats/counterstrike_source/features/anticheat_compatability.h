#pragma once

class c_anti_cheat_manager : public c_base_anti_cheat_manager{
public:
  bool  is_anti_aim_allowed() override;
  bool  has_to_angle_repeat() override;
  u32   get_play_style(bool cache = false) override;
  bool  can_back_track() override;
  bool  should_allow_unclamped_angles() override;
  bool  should_allow_command_number_changes() override;
  bool  can_hide_angle_snaps()                override;
};

CLASS_EXTERN(c_anti_cheat_manager, acm);