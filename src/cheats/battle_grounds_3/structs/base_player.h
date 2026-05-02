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
    return read<vec3>(DT_SDKLocalPlayerExclusive_m_angEyeAngles0);
  }

  ALWAYSINLINE vec3 velocity(){
    return read<vec3>(DT_LocalPlayerExclusive_m_vecVelocity0);
  }

  ALWAYSINLINE void set_viewangles(vec3& new_ang){
    write<vec3>(DT_SDKLocalPlayerExclusive_m_angEyeAngles0, new_ang);
  }

  ALWAYSINLINE vec3& last_viewangle(){
    return read<vec3>(DT_BasePlayer_pl + DT_PlayerState_deadflag + 0x4);
  }

  ALWAYSINLINE vec3 punchangle(){
    return read<vec3>(DT_LocalPlayerExclusive_m_Local + DT_Local_m_vecPunchAngle);
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

  ALWAYSINLINE s_studio_bbox* get_studio_bbox(i32 hitbox){
    s_studio_hdr* hdr = get_studio_hdr();

    if(hdr == nullptr)
      return nullptr;

    if(hitbox > hdr->num_bones){
      DBG("[-] get_studio_bbox: hitbox bone id %i too large\n", hitbox);
      return nullptr;
    }

    s_studio_hitbox_set* set = hdr->get_hitbox_set(hitbox_set);

    if(set == nullptr)
      return nullptr;

    if(hitbox >= set->num_hitboxes){
      DBG("[-] get_studio_bbox: hitbox id %i too large\n", hitbox);
      return nullptr;
    }

    s_studio_bbox* box = set->get_hitbox(hitbox);

    if(box == nullptr)
      return nullptr;

    return box;
  }

  ALWAYSINLINE vec3 shoot_pos(){
    vec3 shoot_pos;

    // We should only call weapon_shootpos on the localplayer -senator
    if(utils::localplayer() == this)
      utils::virtual_call<227, void, vec3&>(this, shoot_pos);
    else
      shoot_pos = eye_pos();

    return shoot_pos;
  }

  bool is_steam_friend(c_base_player* player);

  c_base_weapon* get_weapon();

  bool can_fire();
  bool should_attack2();
  bool will_fire_this_tick();
};