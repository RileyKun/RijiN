#pragma once

class c_internal_base_weapon;
class c_internal_base_player : public c_internal_base_entity{
public:

  ALWAYSINLINE i32& tick_base(){
    gen_read(i32, "DT_LocalPlayerExclusive_m_nTickBase");
  }
#if defined(SOURCE_2018)
  ALWAYSINLINE i32& final_predicted_tick(){
    gen_read_offset(i32, "DT_LocalPlayerExclusive_m_nTickBase", 0x4);
  }
#endif

  ALWAYSINLINE i32& ground_entity(){
    #if defined(SOURCE_2018)
    gen_read(i32, "DT_BasePlayer_m_hGroundEntity");
    #else
    gen_read(i32, "DT_LocalPlayerExclusive_m_hGroundEntity");
    #endif
  }

  ALWAYSINLINE i8 water_level(){
    #if defined(SOURCE_2018)
    gen_read(i8, "DT_BasePlayer_m_nWaterLevel");
    #else
    gen_read(i8, "DT_LocalPlayerExclusive_m_nWaterLevel");
    #endif
  }

  ALWAYSINLINE i32 observer_target(){
    gen_read(i32, "DT_BasePlayer_m_hObserverTarget");
  }

  ALWAYSINLINE i32 observer_mode(){
    gen_read(i32, "DT_BasePlayer_m_iObserverMode");
  }

  ALWAYSINLINE i32& health(){
    gen_read(i32, "DT_BasePlayer_m_iHealth");
  }

  ALWAYSINLINE u8 life_state(){
    gen_read(u8, "DT_BasePlayer_m_lifeState");
  }

  ALWAYSINLINE i32& entity_flags(){
    gen_read(i32, "DT_BasePlayer_m_fFlags");
  }

  ALWAYSINLINE float& max_speed(){
    gen_read(float, "DT_BasePlayer_m_flMaxspeed");
  }

  ALWAYSINLINE i32& fov(){
    gen_read(i32, "DT_BasePlayer_m_iFOV");
  }

  ALWAYSINLINE float& fov_time(){
    gen_read(float, "DT_BasePlayer_m_flFOVTime");
  }

  ALWAYSINLINE i32 weapon_handle(){
    gen_read(i32, "DT_BaseCombatCharacter_m_hActiveWeapon");
  }

  ALWAYSINLINE vec3& velocity(){
    gen_read(vec3, "DT_LocalPlayerExclusive_m_vecVelocity0");
  }

  ALWAYSINLINE void* local(){
    gen_read_pointer(void*, "DT_LocalPlayerExclusive_m_Local");
  }

  ALWAYSINLINE float step_size(){
    gen_read_other(local(), float, "DT_Local_m_flStepSize");
  }

  ALWAYSINLINE bool ducking(){
    gen_read_other(local(), bool, "DT_Local_m_bDucking");
  }

  ALWAYSINLINE bool ducked(){
    gen_read_other(local(), bool, "DT_Local_m_bDucked");
  }

  NEVERINLINE vec3 punch_angle(){
    gen_read_other(local(), vec3, "DT_Local_m_vecPunchAngle");
  }

  NEVERINLINE bool is_base_alive(){
    return team() > 0 && life_state() == 0;
  }

  ALWAYSINLINE vec3& view_offset(){
    gen_read(vec3, "DT_LocalPlayerExclusive_m_vecViewOffset0");
  }

  // This might need to be redone, but if you crash here, then its because it uses SPlayer instead of localplayerexclusive
  NEVERINLINE vec3& viewangles(){
    #if defined(SOURCE_2018) || defined(CSS_CHEAT)
      gen_read(vec3, "DT_SPlayer_m_angEyeAngles0");
    #else
      gen_read(vec3, "DT_LocalPlayerExclusive_m_angEyeAngles0");
    #endif
  }

  ALWAYSINLINE void* pl(){
    gen_read_pointer(void*, "DT_BasePlayer_pl");
  }

  ALWAYSINLINE vec3& last_viewangle(){
    gen_read_other_offset(pl(), vec3, "DT_PlayerState_deadflag", 0x4);
  }

  NEVERINLINE vec3 offset_shoot_pos(){
    return origin() + view_offset();
  }

  ALWAYSINLINE vec3 eye_pos(){
    return offset_shoot_pos();
  }

  vec3                    shoot_pos();
  s_player_info           info();
  void                    update_clientside_animations();
  c_internal_base_weapon* get_weapon();
  c_internal_base_weapon* get_weapon_from_belt(i32 id, bool id_check = true);
  bool                    is_steam_friend();
  c_internal_base_entity* get_ground_entity();
};