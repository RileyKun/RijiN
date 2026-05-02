#pragma once

class c_entity_data;
class c_base_weapon;
class c_base_player : public c_base_entity{
public:
  ALWAYSINLINE bool is_valid(bool check_dormant = true){
    return check_dormant ? !is_dormant() && is_alive() : is_alive();
  }

  c_base_weapon* get_weapon();

  bool can_fire();
  bool can_shove();
  float last_shove_delta();
  bool will_fire_this_tick();
  bool is_steam_friend(c_base_player* player);
};