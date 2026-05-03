#pragma once

class c_auto_detonator{
public:
  void run();
  bool is_target_entity(c_base_entity* entity, c_base_player* localplayer);
  bool is_local_flare(c_base_entity* entity);
};

CLASS_EXTERN(c_auto_detonator, auto_detonator);