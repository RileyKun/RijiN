#pragma once

class c_movement : public c_base_movement{
public:

  c_movement_settings* get_settings() override{
    return &config->movement;
  }

  bool can_perform_double_jump(c_internal_base_player* localplayer) override;
};

CLASS_EXTERN(c_movement, movement);