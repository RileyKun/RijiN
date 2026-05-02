#pragma once


#define ASSUMED_PREDICTED_HEALTH 72
class c_auto_heal_arrow{
public:
  i32   target_index_lock                = 0;
  float next_allow_swap_time             = 0.f;
  float last_vaccinator_uber_charge_time[3];
  
  ALWAYSINLINE void reset(){
    memset(this, 0, sizeof(c_auto_heal_arrow));
  }

  ALWAYSINLINE void invalidate_target(){
    target_index_lock = -1;
  }

  ALWAYSINLINE c_aimbot_settings* get_proj_aim_settings(){
    return &config->aimbot[TF_CLASS_MEDIC - 1][AIMBOT_WEAPON_SETTING_PROJ];
  }

  // Get target via heal target.
  void find_target();
  bool has_valid_target();
  bool force_enable_heal_arrow();
  bool should_run();

  // This function should always return array safe values.
  i32 get_resist_type_from_cond(u32 condition);

  // Vaccinator logic functions.
  void knowledge_vaccinator_charge_released(c_base_player* player, u32 condition);
  bool is_handling_vaccinator_logic(c_base_player* localplayer, c_base_weapon* crossbow, c_base_weapon* wep);

  // Generic functions.
  void run();
  void cancel_automation();

  // Event functions
  void on_death(i32 attacker, i32 victim);

};

CLASS_EXTERN(c_auto_heal_arrow, auto_heal_arrow);