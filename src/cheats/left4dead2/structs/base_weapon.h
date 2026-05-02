#pragma once

class c_base_weapon : public c_internal_base_weapon, public c_base_entity{
public:

  bool has_spread(){
    return !(is_melee() || is_throwable() || is_item() || is_grenade_launcher());
  }

  float& spread(){
    gen_read(float, "spread");
  }

  NEVERINLINE bool is_shotgun(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    switch(cc->id){
      default: break;
      case CShotgun_SPAS:
      case CShotgun_Chrome:
      case CPumpShotgun:
      case CBaseShotgun:
      case CBaseAutoShotgun:
      case CAutoShotgun:
        return true;
    }

    return false;
  }

  NEVERINLINE bool is_claw(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    switch(cc->id){
      default: break;
      case CSpitterClaw:
      case CTankClaw:
      case CSmokerClaw:
      case CJockeyClaw:
      case CHunterClaw:
      case CClaw:
      case CChargerClaw:
      case CBoomerClaw:
        return true;
    }

    return false;
  }

  NEVERINLINE bool is_melee(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    if(is_claw())
      return true;

    switch(cc->id){
      default: break;
      case CTerrorMeleeWeapon:
      case CChainsaw:
        return true;
    }

    return false;
  }

  NEVERINLINE bool is_item(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    switch(cc->id){
      default: break;
      case CItemUpgradePackIncendiary:
      case CItemUpgradePackExplosive:
      case CItem_Adrenaline:
      case CFirstAidKit:
      case COxygenTank:
      case CPainPills:
      case CPropaneTank:
      case CGasCan:
      case CFireworkCrate:
      case CColaBottles:
      case CGnome:
        return true;
    }

    return false;
  }

  NEVERINLINE bool is_throwable(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    switch(cc->id){
      default: break;
      case CPipeBomb:
      case CMolotov:
      case CItem_VomitJar:
        return true;
    }

    return false;
  }

  NEVERINLINE bool is_grenade_launcher(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    switch(cc->id){
      default: break;
      case CGrenadeLauncher:
        return true;
    }

    return false;
  }

  NEVERINLINE bool is_semi_auto(){
    if(is_shotgun())
      return true;

    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    switch(cc->id){
      default: break;
      case CMagnumPistol:
      case CPistol:
      case CSniper_Scout:
      case CSniperRifle:
      case CSniper_Military:
      case CSniper_AWP:
        return true;
    }

    return false;
  }

  NEVERINLINE bool is_gun() {
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;
    
    return cc->id == WEAPON_GRENADE_LAUNCHER || cc->id == WEAPON_M60 || cc->id == WEAPON_PUMP_SHOTGUN ||
           cc->id == WEAPON_CHROME_SHOTGUN || cc->id == WEAPON_AUTO_SHOTGUN || cc->id == WEAPON_SPAS12_SHOTGUN ||
           cc->id == WEAPON_MILITARY_SNIPER || cc->id == WEAPON_HUNTING_RIFLE || cc->id == WEAPON_SCAR ||
           cc->id == WEAPON_M16A1 || cc->id == WEAPON_AK47 || cc->id == WEAPON_UZI || cc->id == WEAPON_MAC10 ||
           cc->id == WEAPON_PISTOL || cc->id == WEAPON_DEAGLE || cc->id == WEAPON_AWP || cc->id == WEAPON_SG552 ||
           cc->id == WEAPON_SCOUT || cc->id == WEAPON_MP5;
  }

  NEVERINLINE bool can_oneshot(i32 ent_type){
    if(!(ent_type & TYPE_INFECTED) && !(ent_type & TYPE_BOOMER))
      return false;

    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;
    
    return is_shotgun() || 
           cc->id == WEAPON_AK47 || cc->id == WEAPON_DEAGLE || 
           cc->id == WEAPON_MILITARY_SNIPER || cc->id == WEAPON_HUNTING_RIFLE || 
           cc->id == WEAPON_AWP || cc->id == WEAPON_SCOUT || cc->id == WEAPON_M60 || cc->id == WEAPON_M16A1;
  }

  NEVERINLINE i32 get_gun_id(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return 0;

    return cc->id;
  }
};