#pragma once

#define MAX_REDIRECT_POSITIONS 3
#define MAX_REDIRECT_HIGHEST_POS 3

#define REDIRECT_DESTROYS_PROJ 1
#define REDIRECT_HEALS_TEAM 2
#define REDIRECT_HEADSHOTS 3

class c_auto_airblast{
public:
  bool run();
  bool is_target_entity(c_base_entity* entity, c_base_player* localplayer, c_base_weapon* weapon, i32& priority);
  // To be only used in find_redirect_target!
  c_base_entity* find_redirect_target_healing(c_base_player* localplayer, vec3 target_angles);
  vec3 find_redirect_target(c_base_player* localplayer, vec3 target_angles, i32 entity_type, i32 wanted_position, bool& success);
  bool is_redirect_target(c_base_player* localplayer, c_base_entity* entity, i32 entity_type);
  vec3 get_target_position(c_base_entity* entity);
  i32 get_best_redirect_position(c_base_entity* entity);
  bool get_redirect_position(c_base_entity* entity, i32 i, vec3& pos);
};

CLASS_EXTERN(c_auto_airblast, auto_airblast);