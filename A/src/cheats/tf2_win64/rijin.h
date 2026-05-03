#pragma once

class c_rijin_product : public c_base_rijin_source1{
public:
  bool is_valid() override;
  bool bug_check();
  bool d3d9_input(c_render_d3d9* render_base) override;
  bool d3d9_hook_game_specific(c_render_d3d9* render_base) override;
  bool d3d9_hook_ui(c_render_d3d9* render_base) override;

  bool pre_predicted_create_move(c_base_player* localplayer, c_base_weapon* wep, c_user_cmd* cmd);
  bool on_predicted_create_move(c_base_player* localplayer, c_base_weapon* wep, c_user_cmd* cmd);
  bool on_post_prediction_create_move(c_base_player* localplayer, c_base_weapon* wep, c_user_cmd* cmd);

  // can be called anywhere.
  bool on_engine_frame() override;

  void on_1000ms() override;
  void on_5000ms() override;
};

CLASS_EXTERN(c_rijin_product, rijin);