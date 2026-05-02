#pragma once

class c_trigger_bot{
public:
  void reset(){
    memset(this, 0, sizeof(c_trigger_bot));
  }

  bool has_valid_weapon(c_base_player* localplayer, c_base_weapon* weapon);

  void run();
  bool is_valid_entity(c_base_player* localplayer, c_base_entity* entity);
  bool process_hitbox(c_base_player* localplayer, c_base_entity* entity, i32 hitbox);

  bool  found_target   = false;
  float trigger_delay = 0.f;
  float fire_for_time = 0.f; // How long we should keep firing after we lost a target.
};

CLASS_EXTERN(c_trigger_bot, trigger_bot);