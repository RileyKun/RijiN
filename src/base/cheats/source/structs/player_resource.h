#pragma once

class c_player_resource{
public:
  ALWAYSINLINE i32 get_health(i32 index){
    gen_read_array(i32, "DT_PlayerResource_m_iHealth", index);
  }

  ALWAYSINLINE i32 get_max_health(i32 index){
    #if defined(SOURCE_2018)
      gen_read_array(i32, "DT_PlayerResource_m_maxHealth", index);
    #else
      gen_read_array(i32, "DT_PlayerResource_m_iMaxHealth", index);
    #endif
  }

  ALWAYSINLINE i32 get_ping(i32 index){
    gen_read_array(i32, "DT_PlayerResource_m_iPing", index);
  }

  ALWAYSINLINE i32 get_team(i32 index){
    gen_read_array(i32, "DT_PlayerResource_m_iTeam", index);
  }

#if !defined(SOURCE_2018)
  ALWAYSINLINE i32 get_account_id(i32 index){
    gen_read_array(i32, "DT_PlayerResource_m_iAccountID", index);
  }
#endif

  #if defined(TF2_CHEAT)
  ALWAYSINLINE i32 get_damage(i32 index){
    gen_read_array(i32, "DT_PlayerResource_m_iDamage", index);
  }

  ALWAYSINLINE i32 get_player_class(i32 index){
    gen_read_array(i32, "DT_PlayerResource_m_iPlayerClass", index);
  }

  ALWAYSINLINE i32 get_max_buffed_health(i32 index){
    gen_read_array(i32, "DT_PlayerResource_m_iMaxBuffedHealth", index);
  }
  #endif
};