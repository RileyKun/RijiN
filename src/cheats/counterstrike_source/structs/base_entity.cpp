#include "../link.h"

// C_BaseEntity::FirstMoveChild
ALWAYSINLINE c_base_entity* c_base_entity::first_move_child(){
  return nullptr;//global->entity_list->get_entity(read<u32>(0x184/*m_pMoveChild*/) & 0xFFF);
}

// C_BaseEntity::NextMovePeer
ALWAYSINLINE c_base_entity* c_base_entity::next_move_peer(){
  return nullptr;//global->entity_list->get_entity(read<u32>(0x188/*m_pMovePeer*/) & 0xFFF);
}

ALWAYSINLINE std::wstring c_base_entity::get_grenade_name(){
  s_client_class* cc = get_client_class();
  if(cc == nullptr){
    assert(false);
    return L"";
  }

  if(cc->id != CBaseCSGrenadeProjectile)
    return L"";

  uptr* model = get_model();
  if(model == nullptr)
    return L"";

  assert(global->model_info != nullptr);
  i8* model_name = global->model_info->get_model_name(model);
  if(model_name == nullptr)
    return L"";

  u32 name_hash = HASH_RT(model_name);
  switch(name_hash){
    default: break;
    case HASH("models/Weapons/w_eq_flashbang_thrown.mdl"):
      return L"Flash bang";
    case HASH("models/Weapons/w_eq_fraggrenade_thrown.mdl"):
      return L"Grenade";
    case HASH("models/Weapons/w_eq_smokegrenade_thrown.mdl"):
      return L"Smoke grenade";
  }
  return L"";
}

ALWAYSINLINE bool c_base_entity::is_objective(){
  s_client_class* cc = get_client_class();
  if(cc == nullptr){
    assert(false);
    return false;
  }

  if(cc->id == CHostage || cc->id == CPlantedC4)
    return true;

  uptr* model = get_model();
  if(model == nullptr)
    return false;

  assert(global->model_info != nullptr);
  i8* model_name = global->model_info->get_model_name(model);
  if(model_name == nullptr)
    return false;

  u32 name_hash = HASH_RT(model_name);
  return false;
}

bool c_base_entity::is_considered_enemy(c_base_entity* target){
  if(global->aimbot_settings == nullptr)
    return false;
  
  if(target == nullptr || target == this)
    return false;

  // should target friends
  if(target->is_player()){
    c_base_player* me = (c_base_player*)this;
    if(!global->aimbot_settings->target_friends && target->is_steam_friend())
      return false;
  }

  // should target team.
  if(global->aimbot_settings->target_friendlies && team() == target->team())
    return true;

  // should target enemies
  if(global->aimbot_settings->target_enemies && team() != target->team())
    return true;

  return false;
}

bool c_base_entity::is_smoke_entity_alive(){
  if(!is_smoke_entity())
    return false;

  return globaldata->cur_time <= this->spawn_time() + this->end_time();
}