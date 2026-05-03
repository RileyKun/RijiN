#pragma once


class c_base_entity: public c_internal_base_player{
public:
  // FOR THE FUCKING SMOKE GRENADES NOTHING ELSE!!
  float spawn_time(){
    gen_read(float, "DT_ParticleSmokeGrenade_m_flSpawnTime");
  }

  float end_time(){
    gen_read(float, "DT_ParticleSmokeGrenade_m_FadeEndTime");
  }

  float& flash_bang_time(){
    gen_read(float, "flash_bang_time");
  }

  bool has_defuser(){
    gen_read(bool, "DT_SPlayer_m_flFlashDuration");
  }

  bool is_defusing(){
    gen_read(bool, "DT_SPlayer_m_bIsDefusing");
  }

  bool is_using_nv(){
    gen_read(bool, "DT_SPlayer_m_bNightVisionOn");
  }

  bool has_nv(){
    gen_read(bool, "DT_SPlayer_m_bHasNightVision");
  }

  bool has_helmet(){
    gen_read(bool, "DT_SPlayer_m_bHasHelmet");
  }

  i32 armor_value(){
    gen_read(i32, "DT_SPlayer_m_ArmorValue");
  }

  NEVERINLINE bool is_considered_spawn_protected(){
    if(this->clr_render_a() > 0 && this->clr_render_a() < 254)
      return true;

    return false;
  }

  c_base_entity* first_move_child();
  c_base_entity* next_move_peer();

  bool setup_bones(matrix3x4* matrix, bool used_by_anything);

  ALWAYSINLINE bool is_active_grenade(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CBaseCSGrenadeProjectile;
  }

  std::wstring get_grenade_name();

  ALWAYSINLINE bool is_dropped_weapon(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }

    switch(cc->id){
      default: break;
      case CWeaponXM1014:
      case CWeaponUSP:
      case CWeaponUMP45:
      case CWeaponTMP:
      case CWeaponSG552:
      case CWeaponSG550:
      case CWeaponScout:
      case CWeaponP90:
      case CWeaponP228:
      case CWeaponMP5Navy:
      case CWeaponMAC10:
      case CWeaponM4A1:
      case CWeaponM3:
      case CWeaponM249:
      case CKnife:
      case CWeaponGlock:
      case CWeaponGalil:
      case CWeaponG3SG1:
      case CWeaponFiveSeven:
      case CWeaponFamas:
      case CWeaponElite:
      case CDEagle:
      case CC4:
      case CWeaponAWP:
      case CWeaponAug:
      case CAK47:
        return true;
    }

    return false;
  }

  // Copied from base weapon cause they use the same classes
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
      case CWeaponScout: return L"Scout";
      case CWeaponP90: return L"P90";
      case CWeaponP228: return L"P228";
      case CWeaponMP5Navy: return L"MP5";
      case CWeaponMAC10: return L"MAC10";
      case CWeaponM4A1: return L"M4A1";
      case CWeaponM3: return L"M3";
      case CWeaponM249: return L"M249";
      case CKnife: return L"Knife";
      case CSmokeGrenade: return L"Smoke Grenade";
      case CHEGrenade: return L"Grenade";
      case CWeaponGlock: return L"Glock";
      case CWeaponGalil: return L"Galil";
      case CWeaponG3SG1: return L"G3SG1";
      case CFlashbang: return L"Flash Bang";
      case CWeaponFiveSeven: return L"Five Seven";
      case CWeaponFamas: return L"Famas";
      case CWeaponElite: return L"Elite";
      case CDEagle: return L"Deagle";
      case CC4: return L"C4";
      case CWeaponAWP: return L"AWP";
      case CWeaponAug: return L"AUG";
      case CAK47: return L"AK47";
    }

    DBG("[!] %s: %i\n", cc->name, cc->id);
    return L"UNK";
  }

  ALWAYSINLINE bool is_player(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }

    return cc->id == CCSPlayer;
  }

  ALWAYSINLINE bool is_smoke_entity(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    return cc->id == ParticleSmokeGrenade;
  }

  bool is_smoke_entity_alive();

  bool is_objective();
  bool is_considered_enemy(c_base_entity* target);
};