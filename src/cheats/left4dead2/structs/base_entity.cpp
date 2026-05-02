#include "../link.h"

c_entity_data* c_base_entity::data(){
  return utils::get_entity_data(get_index());
}

i8* c_base_entity::get_sequence_name(){
  STACK_CHECK_START;
  auto r = utils::call_thiscall<i8*, i32>(global->get_sequence_name, this, sequence());
  STACK_CHECK_END;
  return r;
}

std::wstring c_base_entity::get_dropped_weapon_name(){
  s_entity_info* info = entity_info->get_data(get_index());
  if(info == nullptr)
    return L"";

  switch(info->model_hash){
    case HASH("models/w_models/weapons/w_grenade_launcher.mdl"): return L"Grenade launcher";
    case HASH("models/w_models/weapons/w_rifle_m16a2.mdl"): return L"M16A2";
    case HASH("models/w_models/weapons/w_desert_rifle.mdl"): return L"SCAR";
    case HASH("models/w_models/weapons/w_rifle_sg552.mdl"): return L"SG552";
    case HASH("models/w_models/weapons/w_sniper_scout.mdl"): return L"Scout sniper";
    case HASH("models/w_models/weapons/w_sniper_awp.mdl"): return L"AWP";
    case HASH("models/w_models/weapons/w_sniper_military.mdl"): return L"Auto sniper";
    case HASH("models/w_models/weapons/w_rifle_ak47.mdl"): return L"AK47";
    case HASH("models/w_models/weapons/w_smg_uzi.mdl"): return L"UZI";
    case HASH("models/w_models/weapons/w_autoshot_m4super.mdl"): return L"M4";
    case HASH("models/w_models/weapons/w_shotgun_spas.mdl"): return L"SPAS";
    case HASH("models/w_models/weapons/w_sniper_mini14.mdl"): return L"Hunting rifle";
    case HASH("models/w_models/weapons/w_smg_a.mdl"): return L"SMG";
    case HASH("models/w_models/weapons/w_shotgun.mdl"): return L"Shotgun";
    case HASH("models/w_models/weapons/w_pumpshotgun_A.mdl"): return L"Pump shotgun";
    case HASH("models/w_models/weapons/w_smg_mp5.mdl"): return L"MP5";
    case HASH("models/w_models/weapons/w_desert_eagle.mdl"): return L"Desert eagle";
    case HASH("models/weapons/melee/w_fireaxe.mdl"): return L"Fireaxe";
    case HASH("models/w_models/weapons/w_pistol_B.mdl"): return L"Glock";
    case HASH("models/weapons/melee/w_cricket_bat.mdl"):
    case HASH("models/weapons/melee/w_bat.mdl"): return L"Bat";
    case HASH("models/weapons/melee/w_katana.mdl"): return L"Katana";
    case HASH("models/weapons/melee/w_golfclub.mdl"): return L"Golfclub";
    case HASH("models/w_models/weapons/w_knife_t.mdl"): return L"Knife";
    case HASH("models/weapons/melee/w_crowbar.mdl"): return L"Crowbar";
    case HASH("models/weapons/melee/w_chainsaw.mdl"): return L"Chainsaw";
    case HASH("models/weapons/melee/w_electric_guitar.mdl"): return L"Electric guitar";
    case HASH("models/weapons/melee/w_shovel.mdl"): return L"Shovel";
    case HASH("models/weapons/melee/w_machete.mdl"): return L"Machete";
    case HASH("models/w_models/weapons/w_m60.mdl"): return L"M60";
    case HASH("models/weapons/melee/w_frying_pan.mdl"): return L"Frying pan";
    case HASH("models/weapons/melee/w_tonfa.mdl"): return L"Night stick";
    case HASH("models/weapons/melee/w_pitchfork.mdl"): return L"Pitch fork";
    case HASH("models/weapons/melee/w_riotshield.mdl"): return L"Riot shield";
    case HASH("models/w_models/weapons/w_minigun.mdl"): return L"Mounted minigun";
    case HASH("models/w_models/weapons/50cal.mdl"): return L"Mounted 50CAL";
  }

  return L"";
}

std::wstring c_base_entity::get_dropped_item_name(){
  s_entity_info* info = entity_info->get_data(get_index());
  if(info == nullptr)
    return L"";

  switch(info->model_hash){
    default: break;
    case HASH("models/w_models/weapons/w_eq_bile_flask.mdl"): return L"Vomit";
    case HASH("models/w_models/weapons/w_eq_adrenaline.mdl"): return L"Adrenaline";
    case HASH("models/w_models/Weapons/w_eq_pipebomb.mdl"): return L"Pipebomb";
    case HASH("models/w_models/weapons/w_eq_defibrillator.mdl"): return L"Defibrillator";
    case HASH("models/w_models/weapons/w_eq_painpills.mdl"): return L"Painpills";
    case HASH("models/w_models/weapons/w_eq_Medkit.mdl"): return L"Medkit";
    case HASH("models/w_models/weapons/w_eq_molotov.mdl"): return L"Molotov";
    case HASH("models/props/terror/exploding_ammo.mdl"):
    case HASH("models/w_models/weapons/w_eq_explosive_ammopack.mdl"): return L"Explosive ammopack";
    case HASH("models/props/terror/incendiary_ammo.mdl"):
    case HASH("models/w_models/weapons/w_eq_incendiary_ammopack.mdl"): return L"Incendiary ammopack";
    case HASH("models/w_models/Weapons/w_laser_sights.mdl"): return L"Laser sights";
    case HASH("models/props_junk/dieselcan.mdl"):
    case HASH("models/props_junk/gascan001a.mdl"): return L"Gas can";
    case HASH("models/props_unique/wooden_barricade_gascans.mdl"): return L"Gas cans";
    case HASH("models/props_junk/explosive_box001.mdl"): return L"Explosive box";
    case HASH("models/props_junk/propanecanister001a.mdl"): return L"Propane tank";
    case HASH("models/props_equipment/oxygentank01.mdl"): return L"Oxygen tank";
  }

  if(info->class_id == CGasCan)
    return L"Gas can";
  else if(info->class_id == CPropaneTank)
    return L"Propane tank";

  if(info->class_id == CItemAmmoPack || info->class_id == CWeaponAmmoSpawn)
    return L"Ammo";

  return L"";
}

ALWAYSINLINE bool c_base_entity::is_health_item(){
  s_entity_info* info = entity_info->get_data(get_index());
  if(info == nullptr)
    return false;

  switch(info->model_hash){
    default: break;
    case HASH("models/w_models/weapons/w_eq_Medkit.mdl"):
    case HASH("models/w_models/weapons/w_eq_adrenaline.mdl"):
    case HASH("models/w_models/weapons/w_eq_painpills.mdl"):
      return true;
  }

  return false;
}

ALWAYSINLINE bool c_base_entity::is_grenade_item(){
  s_entity_info* info = entity_info->get_data(get_index());
  if(info == nullptr)
    return false;

  switch(info->model_hash){
    default: break;
    case HASH("models/w_models/weapons/w_eq_bile_flask.mdl"):
    case HASH("models/w_models/Weapons/w_eq_pipebomb.mdl"):
    case HASH("models/w_models/weapons/w_eq_molotov.mdl"):
      return true;
  }

  return false;
}

ALWAYSINLINE bool c_base_entity::is_ammo_item(){
  s_entity_info* info = entity_info->get_data(get_index());
  if(info == nullptr)
    return false;

  if(info->class_id == CItemAmmoPack || info->class_id == CWeaponAmmoSpawn)
    return false;

  if(info->model_ptr == nullptr)
    return false;

  switch(info->model_hash){
    default: break;
    case HASH("models/w_models/weapons/w_eq_explosive_ammopack.mdl"):
    case HASH("models/w_models/weapons/w_eq_incendiary_ammopack.mdl"):
      return true;
  }

  return false;
}

ALWAYSINLINE bool c_base_entity::is_objective(){
  s_client_class* cc = get_client_class();
  if(cc == nullptr)
    return false;

  //if(cc->id != CDynamicProp && cc->id != CPhysicsProp && cc->id != CColaBottles && cc->id != CGasCan)
  //  return false;

  c_entity_data* data = this->data();
  if(data == nullptr)
    return false;

  return data->is_objective;
}

ALWAYSINLINE bool c_base_entity::is_dropped_weapon(){
  s_entity_info* info = entity_info->get_data(get_index());
  if(info == nullptr)
    return false;

  switch(info->model_hash){
    case HASH("models/w_models/weapons/w_grenade_launcher.mdl"):
    case HASH("models/w_models/weapons/w_rifle_m16a2.mdl"):
    case HASH("models/w_models/weapons/w_desert_rifle.mdl"):
    case HASH("models/w_models/weapons/w_rifle_sg552.mdl"):
    case HASH("models/w_models/weapons/w_sniper_scout.mdl"):
    case HASH("models/w_models/weapons/w_sniper_awp.mdl"):
    case HASH("models/w_models/weapons/w_sniper_military.mdl"):
    case HASH("models/w_models/weapons/w_rifle_ak47.mdl"):
    case HASH("models/w_models/weapons/w_smg_uzi.mdl"):
    case HASH("models/w_models/weapons/w_autoshot_m4super.mdl"):
    case HASH("models/w_models/weapons/w_shotgun_spas.mdl"):
    case HASH("models/w_models/weapons/w_sniper_mini14.mdl"):
    case HASH("models/w_models/weapons/w_smg_a.mdl"):
    case HASH("models/w_models/weapons/w_shotgun.mdl"):
    case HASH("models/w_models/weapons/w_pumpshotgun_A.mdl"):
    case HASH("models/w_models/weapons/w_smg_mp5.mdl"):
    case HASH("models/w_models/weapons/w_desert_eagle.mdl"):
    case HASH("models/weapons/melee/w_fireaxe.mdl"):
    case HASH("models/w_models/weapons/w_pistol_B.mdl"):
    case HASH("models/weapons/melee/w_cricket_bat.mdl"):
    case HASH("models/weapons/melee/w_bat.mdl"):
    case HASH("models/weapons/melee/w_katana.mdl"):
    case HASH("models/weapons/melee/w_golfclub.mdl"):
    case HASH("models/w_models/weapons/w_knife_t.mdl"):
    case HASH("models/weapons/melee/w_crowbar.mdl"):
    case HASH("models/weapons/melee/w_chainsaw.mdl"):
    case HASH("models/weapons/melee/w_electric_guitar.mdl"):
    case HASH("models/weapons/melee/w_shovel.mdl"):
    case HASH("models/weapons/melee/w_machete.mdl"):
    case HASH("models/w_models/weapons/w_m60.mdl"):
    case HASH("models/weapons/melee/w_frying_pan.mdl"):
    case HASH("models/weapons/melee/w_tonfa.mdl"):
    case HASH("models/weapons/melee/w_pitchfork.mdl"):
    case HASH("models/weapons/melee/w_riotshield.mdl"):
    case HASH("models/w_models/weapons/w_minigun.mdl"):
    case HASH("models/w_models/weapons/50cal.mdl"):
      return true;
  }

  return false;
}

ALWAYSINLINE bool c_base_entity::is_dropped_item(){
  s_entity_info* info = entity_info->get_data(get_index());
  if(info == nullptr)
    return false;

  switch(info->model_hash){
    default: break;
    case HASH("models/w_models/weapons/w_eq_bile_flask.mdl"):
    case HASH("models/w_models/weapons/w_eq_adrenaline.mdl"):
    case HASH("models/w_models/Weapons/w_eq_pipebomb.mdl"):
    case HASH("models/w_models/weapons/w_eq_defibrillator.mdl"):
    case HASH("models/w_models/weapons/w_eq_painpills.mdl"):
    case HASH("models/w_models/weapons/w_eq_Medkit.mdl"):
    case HASH("models/w_models/weapons/w_eq_molotov.mdl"):
    case HASH("models/w_models/weapons/w_eq_explosive_ammopack.mdl"):
    case HASH("models/w_models/weapons/w_eq_incendiary_ammopack.mdl"):
    case HASH("models/w_models/Weapons/w_laser_sights.mdl"):
    case HASH("models/props_junk/gascan001a.mdl"):
    case HASH("models/props_junk/propanecanister001a.mdl"):
    case HASH("models/props_junk/dieselcan.mdl"):
    case HASH("models/props_unique/wooden_barricade_gascans.mdl"):
    case HASH("models/props_junk/explosive_box001.mdl"):
    case HASH("models/props/terror/exploding_ammo.mdl"):
    case HASH("models/props/terror/incendiary_ammo.mdl"):
    case HASH("models/props_equipment/oxygentank01.mdl"):
      return true;
  }

  if(info->class_id == CGasCan || info->class_id == CPropaneTank || info->class_id == CItemAmmoPack || info->class_id == CWeaponAmmoSpawn)
    return true;

  return false;
}

ALWAYSINLINE bool c_base_entity::can_entity_give_ammo(){
  s_client_class* cc = get_client_class();
  if(cc == nullptr)
    return false;

  return cc->id == CItemAmmoPack || cc->id == CWeaponAmmoSpawn;
}

bool c_base_entity::is_alive(){
  if(team() == 4)
    return false;

  if(type() & TYPE_REAL_PLAYER)
    return is_base_alive();

  if(solid_type() == 0 || solid_flags() & 0x4)
    return false;

  c_entity_data* ed = data();
  if(ed != nullptr && str_utils::contains(ed->sequence_name, XOR("Death")))
    return false;

  return true;
}