#pragma once

class c_base_weapon {
public:
  union{
    DATA(DT_LocalActiveWeaponData_m_flNextPrimaryAttack, float next_primary_attack);
    DATA(DT_BCCLocalPlayerExclusive_m_flNextAttack, float next_attack);
    DATA(DT_LocalActiveWeaponData_m_flNextSecondaryAttack, float next_secondary_attack);
    DATA(DT_BaseEntity_m_hOwnerEntity, i32 owner_handle);
    DATA(DT_LocalWeaponData_m_iPrimaryAmmoType, i32 primary_ammo_type);
    DATA(DT_LocalActiveWeaponData_m_nNextThinkTick, i32 next_think_tick);

    DATA(DT_LocalWeaponData_m_iClip1, i32 clip1);
    DATA(DT_LocalWeaponData_m_iClip2, i32 clip2);
    DATA(DT_WeaponPhysGun_m_hGrabbedEntity, i32 phys_gun_grab_entity_handle);
    DATA(DT_WeaponRPG_m_hMissile, i32 rpg_missile_handle);
  };

  VIRTUAL_TABLE_RW_IMPLEMENTS()

  ALWAYSINLINE void* networkable(){
    assert(this != nullptr);
    return (uptr)this + (sizeof(uptr) * 2);
  }

  i32 get_index(){
    return utils::virtual_call64<9, i32>(networkable());
  }

  ALWAYSINLINE s_client_class* get_client_class(){
    return utils::virtual_call64<2, s_client_class*>(networkable());
  }

  //(weapon_print_name) in client.dll
  std::wstring get_weapon_name();

  ALWAYSINLINE i8* get_name(){
    return utils::virtual_call64<367, i8*>(this);
  }

  ALWAYSINLINE i8* get_print_name(){
    return utils::virtual_call64<368, i8*>(this);
  }

  ALWAYSINLINE i32 get_ammo_type(){
    return utils::virtual_call64<378, i32>(this);
  }

  ALWAYSINLINE bool push_entity(){
    return utils::virtual_call64<172, bool>(this);
  }

  ALWAYSINLINE bool is_lua_weapon(){
    return utils::virtual_call64<170, bool>(this);
  }

  bool is_melee();
  ALWAYSINLINE bool is_physgun(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    u32 hash = HASH_RT(cc->name);
    return hash == HASH("CWeaponPhysGun");
  }


  bool is_harmless_weapon();

  bool is_rpg_guiding_missile(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    if(cc->id != CWeaponRPG)
      return false;

    return this->rpg_missile_handle > 0;
  }

  u32 get_melee_range(){
    return 180;
  }
  
  c_base_entity* get_physgun_grabbed_entity();
  c_base_entity* get_owner();
  u8             scripted_type();
};