#pragma once

class c_cheat_detection : public c_base_cheat_detection{
public:
  virtual u32 game_hash() override{
    return HASH("L4D2");
  }

  bool is_cheating(i32 index, bool exclude_playerlist = false) override;
  bool is_cheating_bot(i32 index, bool exclude_playerlist = false) override;
  bool is_friend(i32 index, bool exclude_playerlist = false) override;

  bool is_weapon_allowed_for_angles(c_internal_base_weapon* weapon) override;
};

CLASS_EXTERN(c_cheat_detection, cheat_detection);