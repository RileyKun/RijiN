#pragma once

class c_accuracy : public c_base_accuracy{
public:

  bool has_custom_server_random_seed() override {
    return true;
  }

  void set_prediction_random_seed(i32 seed) override;
  bool is_seed_predicting() override;
  bool set_command(i32 command_number) override;
  i32  get_seed() override;
  bool get_best_bullet_with_seed(vec3 current_angle, i32* bullet) override;
  bool get_spread(vec3* out, u32& math_type, i32 seed = -1) override;
  bool is_nospread_active() override;
  bool should_modify_command_number(c_internal_base_weapon* weapon) override;
  bool setup_good_seed_data(c_good_seed_data* data, c_internal_base_weapon* weapon);
  bool is_good_seed(i32 seed, c_good_seed_data* data);
  bool should_force_command_number() override;
  bool calculate_command_number(c_user_cmd* cmd, c_internal_base_weapon* weapon) override;
  bool on_received_command_number(i32 command_number, c_internal_base_weapon* weapon) override;
  void apply_spread_to_angle(vec3* angle, bool add = false, i32 seed_override = -1) override;
  bool create_move_run(c_user_cmd* cmd, bool pre_prediction) override;
};

CLASS_EXTERN(c_accuracy, accuracy);