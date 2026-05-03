#pragma once

class c_auto_medigun_shield{
public:
  void run();

  // find targets that are both to collide with our shield.
  bool found_target_near_by(c_base_player* localplayer, vec3& best_angle);

  // find the yaw angle that covers the most enemies.
  bool found_best_cover_target(c_base_player* localplayer, vec3& best_angle, u32& total_entities);
  bool is_valid_entity(c_base_entity* entity);

  // rebuilt CTFMedigunShield::UpdateShieldPosition
  bool get_predicted_shield_position(vec3 eye_angles, vec3& pos);
};

CLASS_EXTERN(c_auto_medigun_shield, auto_shield);