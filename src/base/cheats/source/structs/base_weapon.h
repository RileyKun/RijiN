#pragma once

class c_internal_base_weapon{
public:
  ALWAYSINLINE float& next_primary_attack(){
    gen_read(float, "DT_LocalActiveWeaponData_m_flNextPrimaryAttack");
  }

  ALWAYSINLINE float& next_secondary_attack(){
    gen_read(float, "DT_LocalActiveWeaponData_m_flNextSecondaryAttack");
  }

  ALWAYSINLINE i32 clip1(){
    #if defined(SOURCE_2018)
    gen_read(i32, "DT_BaseCombatWeapon_m_iClip1");
    #else
    gen_read(i32, "DT_LocalWeaponData_m_iClip1");
    #endif
  }

  ALWAYSINLINE i32 clip2(){
    gen_read(i32, "DT_LocalWeaponData_m_iClip2");
  }

  ALWAYSINLINE i32 primary_ammo_type(){
    gen_read(i32, "DT_LocalWeaponData_m_iPrimaryAmmoType");
  }

  ALWAYSINLINE i32 get_slot(){
    create_offset_hash("get_slot");
    return utils::internal_virtual_call<i32>(offset, this);
  }

  ALWAYSINLINE i32 get_id(){
    create_offset_hash("get_id");
    return utils::internal_virtual_call<i32>(offset, this);
  }
};