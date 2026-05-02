#pragma once

#define AUTO_VACCINATOR_MAX2CHARGE 26
#define AUTO_VACCINATOR_PLAYER_MAXDIST 32
#define AUTO_VACCINATOR_PROJ_MAXDIST 8
#define AUTO_VACCINATOR_CLOSE_RANGE 4
#define AUTO_VACCINATOR_PROJ_DANGER 6
#define AUTO_VACCINATOR_MAGIC_THREAT_VALUE 6924
#define AUTO_VACCINATOR_MAGIC_THREAT_FVALUE 6924.f

//#define AUTO_VACCINATOR_DEBUG

// These can be used to make the auto vaccinator instantly prefer damage types.
enum auto_charge_flags{

  // Charge this resist type now! But, it'll only work if auto vaccinator is on full control.
  AUTO_CHARGE_FLAG_BULLET = (1 << 1),
  AUTO_CHARGE_FLAG_BLAST = (1 << 2),
  AUTO_CHARGE_FLAG_FIRE = (1 << 3),

  // For passive vaccinator reactions and uber costs. (This will run in passive/full control.)
  AUTO_CHARGE_FLAG_BULLET_INSTANT_KILL = (1 << 4),
  AUTO_CHARGE_FLAG_BLAST_INSTANT_KILL  = (1 << 5),
  AUTO_CHARGE_FLAG_FIRE_INSTANT_KILL   = (1 << 6),

  // It will prevent any ubering.
  AUTO_CHARGE_CANNOT_UBER              = (1 << 9),
};

// Should this be here or moved into structs -Rud
struct s_auto_vaccinator_data{
  // Used for knowing when to pop charges but also over-ride the passive healing resist.
  i32 bullet;
  i32 blast;
  i32 fire;

  // This will keep track of how many players are with in range of us. (Does not check if they're visible.)
  // Used for the passive healing resist. It will be taken into account if the 3 variables above are all equal or less than 1.
  i32 overall_bullet;
  i32 overall_blast;
  i32 overall_fire;

  float bullet_dmg;
  float blast_dmg;
  float fire_dmg;

  // Happens when we're on fire or our heal target is on fire.
  bool after_burn;

  i32 flags;
  i32 blast_projectile_near_by;
  i32 stickies_near_by;
};

class c_auto_vaccinator{
public:
  void run();
  bool is_visible(c_base_entity* ent, c_base_player* protect, bool& in_blast_radius, bool predict_players = false);
  void handle_player(c_base_player* protect, c_base_player* player, s_auto_vaccinator_data* data);
  void handle_entity(c_base_player* protect, c_base_entity* ent, s_auto_vaccinator_data* data);
  void process_data(c_base_player* protect, s_auto_vaccinator_data* data);
  void force_uber_charge(s_auto_vaccinator_data* data, std::wstring reason, i32 add_flags);
  bool is_wanted_cycle(i32 resist);
  void perform_cycle();

  ALWAYSINLINE void set_wanted_resist(i32 resist){
    wanted_resist_cycle = resist;
  }

  bool should_predict_other_players(c_base_player* protect, c_base_player* player);
  i32 get_unknown_reaction_range();

  void on_damage(c_game_event* event);
  bool process_manual_charge(c_base_player* protect);
  bool process_manual_charge_cycle();
  void handle_medigun_attack2(c_base_weapon* wep);
  bool is_hitscan_weapon(i32 weaponid);
  i32 get_weapon_resist_type(c_base_weapon* weapon);
  i32 calc_uber_cost(c_base_player* protect);
  void medigun_itempostframe_run(c_base_player* player, c_base_weapon* wep, bool reload, i32 buttons);
  void int_decode_run(c_base_entity* entity, i32* data);

  c_key_control key;
  i32 wanted_resist_cycle = -1;
};

CLASS_EXTERN(c_auto_vaccinator, auto_vaccinator);