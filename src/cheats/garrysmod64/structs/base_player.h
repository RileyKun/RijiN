#pragma once

class c_player_data;
class c_base_weapon;
class c_base_player : public c_base_entity{
public:
  c_player_data* data();

  ALWAYSINLINE bool is_valid(bool check_dormant = true){
    return check_dormant ? !is_dormant() && is_alive() : is_alive();
  }

  ALWAYSINLINE vec3 viewangles(){
    return read<vec3>(DT_HL2MPLocalPlayerExclusive_m_angEyeAngles0);
  }

  ALWAYSINLINE vec3 velocity(){
    return read<vec3>(DT_BaseEntity_m_vecVelocity0);
  }

  ALWAYSINLINE void set_viewangles(vec3& new_ang){
    write<vec3>(DT_HL2MPLocalPlayerExclusive_m_angEyeAngles0, new_ang);
  }

  ALWAYSINLINE vec3& last_viewangle(){
    return read<vec3>(DT_BasePlayer_pl + DT_PlayerState_deadflag + 0x4);
  }

  ALWAYSINLINE vec3 punchangle(){
    return read<vec3>(DT_LocalPlayerExclusive_m_Local + DT_Local_m_vecPunchAngle);
  }

  ALWAYSINLINE void set_punchangle(vec3& new_ang){
    write<vec3>(DT_LocalPlayerExclusive_m_Local + DT_Local_m_vecPunchAngle, new_ang);
  }

  ALWAYSINLINE vec3 view_offset(){
    return read<vec3>(DT_LocalPlayerExclusive_m_vecViewOffset0);
  }

  ALWAYSINLINE vec3 eye_pos(){
    return origin() + view_offset();
  }

  ALWAYSINLINE bool is_alive(){
    return this->life_state == 0;
  }

  ALWAYSINLINE vec3 shoot_pos(){
    vec3 shoot_pos;

    // We should only call weapon_shootpos on the localplayer -senator
    if(utils::localplayer() == this)
      utils::virtual_call64<287, void, vec3&>(this, shoot_pos);
    else
      shoot_pos = eye_pos();

    return shoot_pos;
  }

  ALWAYSINLINE bool is_frozen(){
    return this->entity_flags & FL_ATCONTROLS || this->entity_flags & FL_FROZEN;
  }
  
  bool is_perpheads_police();
  bool is_perpheads_medic();
  bool is_perpheads();
  bool is_god_moded();
  bool is_considered_enemy(c_base_player* target);

  ALWAYSINLINE bool is_demi_god_mode(){
    return this->health > 1000;
  }

  bool is_staff();
  bool is_vip();
  bool is_steam_friend(c_base_player* player);

  c_base_weapon* get_weapon();
  bool is_can_fire_supported();
  bool can_fire();
  bool will_fire_this_tick();
  bool is_holding_semi_auto();

};