#pragma once

#define AUTO_UBER_MAX2CHARGE 175.f
#define AUTO_UBER_PLAYER_MAXDIST 24
#define AUTO_KRITZ_PLAYER_MAXDIST 48
#define AUTO_UBER_PROJ_MAXDIST 8
#define AUTO_UBER_CLOSE_RANGE 3

#define AUTO_UBER_DEBUG

struct s_auto_uber_data{
  // Our threat score.
  float threat;

  // How much enemies are close by?
  i32 enemies_close_by;

  // How much enemies can we see?
  i32 vis_enemies;

  // Visible enemy health near by.
  i32 total_enemy_health;

  // How many heal sources are near by? Like resupply lockers, friendly dispensers, health kits, etc...
  i32 heal_sources_near_by;

  bool deploy_uber;
  i32 blast_projectile_near_by;
};

class c_auto_uber{
public:
  void run();

  void process_player_threats(c_base_weapon* medigun, c_base_player* player, c_base_player* protect, s_auto_uber_data* uberdata);
  void process_entity_threats(c_base_weapon* medigun, c_base_entity* ent, c_base_player* protect, s_auto_uber_data* uberdata);
  void process_data(c_base_weapon* medigun, c_base_player* heal_target, s_auto_uber_data* uberdata);
  void handle_medigun_attack2(c_base_weapon* wep);
  float calc_uber_cost(c_base_player* protect);
  void on_deploy_charge(s_auto_uber_data* uberdata, std::string reason);
  bool is_visible(c_base_entity* ent, c_base_player* protect, bool& in_blast_radius);
};

CLASS_EXTERN(c_auto_uber, auto_uber);