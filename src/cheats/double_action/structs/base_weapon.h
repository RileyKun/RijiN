#pragma once

class c_base_weapon {
public:
  union{
    DATA(DT_LocalActiveWeaponData_m_flNextPrimaryAttack, float next_primary_attack);
    DATA(DT_LocalActiveWeaponData_m_flNextSecondaryAttack, float next_secondary_attack);
    DATA(DT_BaseEntity_m_hOwnerEntity, i32 owner_handle);
    DATA(DT_LocalWeaponData_m_iPrimaryAmmoType, i32 primary_ammo_type);

    DATA(DT_LocalWeaponData_m_iClip1, i32 clip1);
    DATA(DT_WeaponSDKBase_m_flSpread, float spread);
    DATA(DT_WeaponSDKBase_m_flViewPunchMultiplier, float view_punch_mult);
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
      case CAkimboM1911: return L"AKIMBO  M1911";
      case CAkimboBeretta: return L"AKIMBO  BERETTA";
      case CWeaponBeretta: return L"BERETTA";
      case CWeaponBrawl: return L"FISTS";
      case CWeaponCrowbar: return L"CROWBAR";
      case CWeaponFAL: return L"FAL";
      case CWeaponGrenade: return L"GRENADE";
      case CWeaponM16: return L"M16";
      case CWeaponM1911: return L"M1911";
      case CWeaponMAC10: return L"MAC10";
      case CWeaponMossberg: return L"MOSSBERG";
      case CWeaponMP5K: return L"MP5";
      case CWeaponSawnoff: return L"SAWNOFF  SHOTGUN";
      case CWeaponShotgun: return L"SHOTGUN";
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

    return cc->id == CWeaponBrawl;
  }


  ALWAYSINLINE bool is_pistol(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }

    switch(cc->id){
      default: break;
      case CAkimboBeretta:
      case CAkimboM1911:
      case CWeaponBeretta:
      case CWeaponM1911:
        return true;
    }

    return false;
  }

  ALWAYSINLINE bool is_shotgun(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }

    return cc->id == CWeaponSawnoff || cc->id == CWeaponShotgun;
  }

  ALWAYSINLINE bool is_semi_auto(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }

    return is_pistol() || cc->id == CWeaponM16;
  }

  float get_spread();

  c_base_entity* get_owner();
  i8*            get_class_name();
  css_weapon_file_info*          get_weapon_info();
};