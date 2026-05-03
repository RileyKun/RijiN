#include "../link.h"

ALWAYSINLINE c_base_entity* c_base_weapon::get_physgun_grabbed_entity(){
  if(!is_physgun())
    return nullptr;

  if(this->phys_gun_grab_entity_handle == -1)
    return nullptr;
  
  return global->entity_list->get_entity_handle(this->phys_gun_grab_entity_handle);
}

ALWAYSINLINE c_base_entity* c_base_weapon::get_owner(){
  return (c_base_entity*)global->entity_list->get_entity_handle(owner_handle);
}

// Look for (weapon_print_name) or (weapon_name) to find the offset. (367)
ALWAYSINLINE std::wstring c_base_weapon::get_weapon_name(){
  // Don't use cache if you're getting weapons by slot rather than active weapon.
  i8* print_name            = get_print_name();
  if(print_name == nullptr)
    return L"";

  assert(print_name != nullptr);
  assert(global->localization_addr != nullptr);

  wchar_t* delocalized_text = utils::virtual_call64<11, wchar_t*, i8*>(global->localization_addr, print_name);
  if(delocalized_text != nullptr)
    return utils::vis_upper_string(delocalized_text);

  return utils::vis_upper_string(convert::str2wstr(print_name).c_str());
}

ALWAYSINLINE u8 c_base_weapon::scripted_type(){
  i8* name  = get_name();

  if(name == nullptr)
    return weapon_type_unk;

  u32 hash  = FNV1A_STR_RT(name, true);
  u32 h3    = FNV1A_RT(name, 3, true);
  u32 h4    = FNV1A_RT(name, 4, true);

  if(hash == HASH("weapon_357") || hash == HASH("weapon_pistol") || 
     hash == HASH("weapon_crossbow") || hash == HASH("weapon_crowbar") || 
     hash == HASH("weapon_frag") || hash == HASH("weapon_physcannon") ||
     hash == HASH("weapon_ar2") || hash == HASH("weapon_rpg") ||
     hash == HASH("weapon_slam") || hash == HASH("weapon_shotgun") ||
     hash == HASH("weapon_smg1") || hash == HASH("weapon_stunstick") || 
     hash == HASH("weapon_bugbait") || hash == HASH("weapon_mp5_hl1") ||
     hash == HASH("weapon_357_hl1") || hash == HASH("weapon_glock_hl1") ||
     hash == HASH("weapon_crossbow_hl1") || hash == HASH("weapon_shotgun_hl1") ||
     hash == HASH("weapon_gauss"))
    return weapon_type_hl2;

  if(h4 == HASH("m9k_"))
    return weapon_type_m9k;

  if(h3 == HASH("cw_") || h4 == HASH("khr_") || FNV1A_RT(name, 7, true) == HASH("garnet_"))
    return weapon_type_cw;

  if(FNV1A_RT(name, 5, true) == HASH("fas2_"))
    return weapon_type_fas;

  if(h4 == HASH("tfa_"))
    return weapon_type_tfa;

  return weapon_type_unk;
}

bool c_base_weapon::is_melee(){
  c_base_player* owner = get_owner();
  if(owner == nullptr)
    return false;

  if(!owner->is_player())
    return false;

  c_player_data* data = owner->data();
  if(data != nullptr){
    if(data->gmod_data.is_melee)
      return true;
  }

  s_client_class* cc = get_client_class();
  if(cc == nullptr){
    assert(false);
    return false;
  }

  i8* name = get_name();
  if(name == nullptr)
    return false;

  u32 hash = HASH_RT(name);
  if(  hash == HASH("m9k_damascus")
    || hash == HASH("m9k_fists")
    || hash == HASH("gmod_fists")
    || hash == HASH("weapon_fists")
    || hash == HASH("m9k_knife")
    || hash == HASH("m9k_machete")
    || hash == HASH("fas2_machete")
    || hash == HASH("weapon_zm_improvised")
    || hash == HASH("weapon_policebaton"))
    return true;

  return cc->id == CWeaponCrowbar || cc->id == CWeaponStunStick;
}

bool c_base_weapon::is_harmless_weapon(){
  c_base_player* owner = get_owner();
  if(owner == nullptr)
    return false;

  if(!owner->is_player())
    return false;

  c_player_data* data = owner->data();
  if(data != nullptr){
    if(data->gmod_data.is_harmless)
      return true;
  }

  s_client_class* cc = get_client_class();
  if(cc == nullptr)
    return false;

  i8* name = get_name();
  if(name == nullptr)
    return false;

  u32 hash = HASH_RT(name);
  if(  hash == HASH("weapon_physcannon") 
    || hash == HASH("weapon_physgun")
    || hash == HASH("gmod_camera") 
    || hash == HASH("gmod_tool") 
    || hash == HASH("weapon_medkit")
    || hash == HASH("roleplay_fists")
    || hash == HASH("roleplay_keys")
    || hash == HASH("weapon_bugbait")
    || hash == HASH("weapon_slam")
    || hash == HASH("m9k_ied_detonator")
    || hash == HASH("m9k_suicide_bomb")
    || hash == HASH("m9k_nerve_gas")
    || hash == HASH("m9k_sticky_grenade")
    || hash == HASH("m9k_m61_frag")
    || hash == HASH("manhack_welder")
    || hash == HASH("fas2_m67")
    || hash == HASH("fas2_ifak")
    || hash == HASH("fas2_ammobox")
    || hash == HASH("weapon_ttt_disguiser")
    || hash == HASH("weapon_ttt_wtester")
    || hash == HASH("weapon_ttt_binoculars")
    || hash == HASH("weapon_ttt_health_station")
    || hash == HASH("weapon_ttt_defuser")
    || hash == HASH("item_ttt_armor")
    || hash == HASH("weapon_ttt_radar")
    || hash == HASH("weapon_ttt_push")
    || hash == HASH("weapon_ttt_ump")
    || hash == HASH("lockpick")
    || hash == HASH("keypad_cracker")
    || hash == HASH("weapon_defibrillator"))
    return true;

  if(cc->id == CWeaponFrag)
    return true;
  
  //DBG("[!] get_name: %s\n", get_name());
  return false;
}