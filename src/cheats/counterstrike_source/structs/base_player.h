#pragma once

class c_player_data;
class c_base_weapon;
class c_base_player : public c_base_entity{
public:
  c_player_data* data();

  ALWAYSINLINE bool is_valid(bool check_dormant = true){
    return check_dormant ? !is_dormant() && is_alive() : is_alive();
  }

  ALWAYSINLINE bool is_alive(){
    return is_base_alive();
  }

  bool can_fire();
  bool will_fire_this_tick();
};