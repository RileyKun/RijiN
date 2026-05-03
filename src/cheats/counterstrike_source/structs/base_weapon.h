#pragma once

class c_base_weapon : public c_internal_base_weapon, public c_base_entity{
public:
  float smack_time(){
    gen_read(float, "smack_time");
  }

  bool glock_burst_mode(){
    gen_read(bool, "DT_WeaponGlock_m_bBurstMode");
  }

  i32 glock_burst_shots(){
    gen_read(i32, "DT_WeaponGlock_m_iBurstShotsRemaining");
  }

  float glock_next_burst_shot(){
    gen_read_offset(float, "DT_WeaponGlock_m_iBurstShotsRemaining", 0x4);
  }

  float famas_burst_mode(){
    gen_read(float, "DT_WeaponFamas_m_bBurstMode");
  }

  i32 famas_burst_shots(){
    gen_read(i32, "DT_WeaponFamas_m_iBurstShotsRemaining");
  }

  float famas_next_burst_shot(){
    gen_read(float, "famas_next_burst_shot");
  }

  // GRENADES!
  bool pin_pulled(){
    gen_read(bool, "DT_BaseCSGrenade_m_bPinPulled");
  }

  float throw_time(){
    gen_read(float, "DT_BaseCSGrenade_m_fThrowTime");
  }

  // Nothing else

  std::wstring get_weapon_name(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return L"";
    }

    switch(cc->id){
      default: break;
      case CWeaponXM1014: return L"XM1014";
      case CWeaponUSP: return L"USP";
      case CWeaponUMP45: return L"UMP45";
      case CWeaponTMP: return L"TMP";
      case CWeaponSG552: return L"SG552";
      case CWeaponSG550: return L"SG550";
      case CWeaponScout: return L"SCOUT";
      case CWeaponP90: return L"P90";
      case CWeaponP228: return L"P228";
      case CWeaponMP5Navy: return L"MP5";
      case CWeaponMAC10: return L"MAC10";
      case CWeaponM4A1: return L"M4A1";
      case CWeaponM3: return L"M3";
      case CWeaponM249: return L"M249";
      case CKnife: return L"KNIFE";
      case CSmokeGrenade: return L"SMOKE  GRENADE";
      case CHEGrenade: return L"GRENADE";
      case CWeaponGlock: return L"GLOCK";
      case CWeaponGalil: return L"GALIL";
      case CWeaponG3SG1: return L"G3SG1";
      case CFlashbang: return L"FLASH  BANG";
      case CWeaponFiveSeven: return L"FIVE  SEVEN";
      case CWeaponFamas: return L"FAMAS";
      case CWeaponElite: return L"ELITE";
      case CDEagle: return L"DEAGLE";
      case CC4: return L"C4";
      case CWeaponAWP: return L"AWP";
      case CWeaponAug: return L"AUG";
      case CAK47: return L"AK47";
    }

    DBG("[!] %s: %i\n", cc->name, cc->id);
    return L"UNK";
  }

  ALWAYSINLINE bool is_burst_mode_active(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    if(cc->id == CWeaponFamas)
      return this->famas_burst_mode();
    else if(cc->id == CWeaponGlock)
      return this->glock_burst_mode();

    return false;
  }

  ALWAYSINLINE i32 get_burst_shots_remaining(){
    if(!is_burst_mode_active())
      return 0;

    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return 0;

    if(cc->id == CWeaponFamas)
      return this->famas_burst_shots();
    else if(cc->id == CWeaponGlock)
      return this->glock_burst_shots();

    return 0;
  }

  ALWAYSINLINE float get_next_burst_shot_time(){
    if(!is_burst_mode_active())
      return -1.f;

    if(get_burst_shots_remaining() <= 0)
      return -1.f;

    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    if(cc->id == CWeaponFamas)
      return this->famas_next_burst_shot();
    else if(cc->id == CWeaponGlock)
      return this->glock_next_burst_shot();

    return -1.f;
  }

  ALWAYSINLINE bool is_sniper_rifle(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    switch(cc->id){
      default: break;
      case CWeaponAWP:
      case CWeaponScout:
        return true;
    }

    return is_auto_sniper_rifle();
  }

  ALWAYSINLINE bool is_auto_sniper_rifle(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    switch(cc->id){
      default: break;
      case CWeaponG3SG1:
      case CWeaponSG550:
        return true;
    }
    return false;
  }

  ALWAYSINLINE bool is_rifle(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    switch(cc->id){
      default: break;
      case CAK47:
      case CWeaponAug:
      case CWeaponFamas:
      case CWeaponM249:
      case CWeaponM4A1:
      case CWeaponGalil:
      case CWeaponSG552:
        return true;
    }

    return false;
  }

  ALWAYSINLINE bool is_smg(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    switch(cc->id){
      default: break;
      case CWeaponFamas:
      case CWeaponMAC10:
      case CWeaponMP5Navy:
      case CWeaponP90:
      case CWeaponTMP:
      case CWeaponUMP45:
        return true;
    }

    return false;
  }

  ALWAYSINLINE bool is_shotgun(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    switch(cc->id){
      default: break;
      case CWeaponM3:
      case CWeaponXM1014:
        return true;
    }

    return false;
  }

  ALWAYSINLINE bool is_melee(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }

    return cc->id == CKnife;
  }


  ALWAYSINLINE bool is_pistol(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }

    switch(cc->id){
      default: break;
      case CWeaponUSP:
      case CWeaponFiveSeven:
      case CWeaponElite:
      case CDEagle:
      case CWeaponGlock:
      case CWeaponP228:
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

    switch(cc->id){
      default: break;
      case CHEGrenade:
      case CFlashbang:
      case CSmokeGrenade:
        return true;
    }

    return false;
  }

  ALWAYSINLINE bool is_holding_bomb(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    switch(cc->id){
      default: break;
      case CC4:
        return true;
    }

    return false;
  }

  ALWAYSINLINE bool is_semi_auto(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }

    return is_pistol();
  }

  ALWAYSINLINE void update_accuracy_penalty(){
    if(is_melee() || is_holding_bomb())
      return;

    utils::virtual_call<384, void>(this);
  }

  ALWAYSINLINE float get_spread(){
    return utils::virtual_call<382, float>(this);
  }

  ALWAYSINLINE float get_inaccuracy(){
    return utils::virtual_call<383, float>(this);
  }

  c_base_entity* get_owner();
  i8*            get_class_name();
  css_weapon_file_info*          get_weapon_info();
};