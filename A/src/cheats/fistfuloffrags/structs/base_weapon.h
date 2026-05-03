#pragma once

class c_base_weapon {
public:
  union{
    DATA(DT_LocalActiveWeaponData_m_flNextPrimaryAttack, float next_primary_attack);
    DATA(DT_LocalActiveWeaponData_m_flNextSecondaryAttack, float next_secondary_attack);
    DATA(DT_BaseEntity_m_hOwnerEntity, i32 owner_handle);
    DATA(DT_LocalWeaponData_m_iPrimaryAmmoType, i32 primary_ammo_type);

    DATA(DT_LocalWeaponData_m_iClip1, i32 clip1);
  };

  VIRTUAL_TABLE_RW_IMPLEMENTS()

  ALWAYSINLINE void* networkable(){
    assert(this != nullptr);
    return (uptr)this + 0x8;
  }

  i32 get_index(){
    return utils::virtual_call<9, i32>(networkable());
  }

  ALWAYSINLINE s_client_class* get_client_class(){
    return utils::virtual_call<2, s_client_class*>(networkable());
  }

  std::wstring get_weapon_name(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return L"";
    }

    switch(cc->id){
      default: break;
      case CWhiskey1:
      case CWhiskey2:
        return L"WHISKEY";
      case CColtNavy1:
      case CColtNavy2:
        return L"COLT  NAVY";
      case CWalker1:
      case CWalker2:
        return L"WALKER";
      case CVolcanic1:
      case CVolcanic2:
        return L"VOLCANIC";
      case CWeaponSpencer:
        return L"SPENCER";
      case CWeaponShotgunFoF:
        return L"SHOTGUN";
      case CWeaponSharps1874:
        return L"SHARPS";
      case CSchofield1:
      case CSchofield2:
        return L"SCHOFIELD";
      case CSawedShotgun1:
      case CSawedShotgun2:
        return L"SAWED  SHOTGUN";
      case CRemington_Army:
      case CRemington_Army2:
        return L"REMINGTON";
      case CPeacemaker1:
      case CPeacemaker2:
        return L"PEACE  MAKER";
      case CMauser1:
      case CMauser2:
        return L"MAUSER";
      case CMaresLeg1:
      case CMaresLeg2:
        return L"MARES  LEG";
      case CWeaponMachete:
        return L"MACHETE";
      case CHammerless1:
      case CHammerless2:
        return L"HAMMERLESS";
      case CGhostGun1:
      case CGhostGun2:
        return L"GHOST  GUN";
      case CWeaponFistsGhost:
      case CWeaponFists:
        return L"FISTS";
      case CWeaponDynamiteBlack:
        return L"DYNAMITE  BLACK";
      case CWeaponDynamiteBelt:
        return L"DYNAMITE  BELT";
      case CWeaponDynamite:
        return L"DYNAMITE";
      case CDeringer1:
      case CDeringer2:
        return L"DERINGER";
      case CWeaponCoachgun:
        return L"COACHGUN";
      case CWeaponCarbine:
        return L"CARBINE";
      case CWeaponBowie:
        return L"BOWIE";
      case CWeaponBowarrowBlack:
      case CWeaponBowarrow:
        return L"BOW";
      case CWeaponAxe:
        return L"AXE";
      case CWeaponYellowboy:
        return L"YELLOW  BOY";
    }

    DBG("[!] %s: %i\n", cc->name, cc->id);
    return L"UNK";
  }

  ALWAYSINLINE bool is_melee(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }

    return false;
  }


  ALWAYSINLINE bool is_pistol(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }

    switch(cc->id){
      default: break;
      case CColtNavy1:
      case CColtNavy2:
      case CVolcanic1:
      case CVolcanic2:
      case CWalker1:
      case CWalker2:
      case CRemington_Army:
      case CRemington_Army2:
      case CPeacemaker1:
      case CPeacemaker2:
      case CDeringer1:
      case CDeringer2:
        return true;
    }

    return false;
  }


  ALWAYSINLINE bool is_grenade(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }

    return cc->id == CWeaponDynamite || cc->id == CWeaponDynamiteBlack;
  }

  ALWAYSINLINE bool is_semi_auto(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }

    return is_pistol();
  }

  ALWAYSINLINE float get_spread(){
    return 0.f;//utils::virtual_call<376, float>(this);
  }

  ALWAYSINLINE float get_inaccuracy(){
    return 0.f;//utils::virtual_call<377, float>(this);
  }

  c_base_entity* get_owner();
  i8*            get_class_name();
  css_weapon_file_info*          get_weapon_info();
};