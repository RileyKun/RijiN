#include "../link.h"

ALWAYSINLINE c_base_entity* c_base_entity::get_owner(){
  if(owner_handle == 0)
    return nullptr;

  return (c_base_entity*)global->entity_list->get_entity_handle(owner_handle);
}

// C_BaseEntity::FirstMoveChild
ALWAYSINLINE c_base_entity* c_base_entity::first_move_child(){
  return global->entity_list->get_entity(read<u32>(0x184/*m_pMoveChild*/) & 0xFFF);
}

// C_BaseEntity::NextMovePeer
ALWAYSINLINE c_base_entity* c_base_entity::next_move_peer(){
  return global->entity_list->get_entity(read<u32>(0x188/*m_pMovePeer*/) & 0xFFF);
}

s_player_info c_base_entity::info(){
  s_player_info info{};
  global->engine->get_player_info(get_index(), &info);

  return info;
}

ALWAYSINLINE s_studio_hdr* c_base_entity::get_studio_hdr(){
  void* model = get_model();

  if(model == nullptr)
    return nullptr;

  return global->model_info->get_studio_hdr(model);
}

bool c_base_entity::setup_bones(matrix3x4* matrix, bool used_by_anything){
  s_studio_hdr* hdr = get_studio_hdr();

  if(hdr == nullptr)
    return false;

  // invalidate the bone cache
  last_bone_setup_time              = -(3.402823e+38f);
  model_bone_counter                = -1;

  // FRAME_NET_UPDATE_POSTDATAUPDATE_END does not yet call set_abs_origin
  // If we don't do this, we are setting up bones for the last abs origin instead of the new networked one
  {
    vec3 t = origin();
    //DBG("[!] setup_bones set set_abs_origin\n");
    set_abs_origin(t);

    if(this->is_player()){
      c_base_player* p = (c_base_player*)this;

      vec3 a = p->viewangles();
      a.x = a.z = 0.f;
      set_abs_angles(a);
    }
  }

  return utils::call_thiscall<bool, matrix3x4*, i32, i32, float>(gen_internal->decrypt_asset(global->base_animating_setupbones_hook_trp), renderable(), matrix, math::smallest(hdr->num_bones, 128), used_by_anything ? 0x7FF00/*BONE_USED_BY_ANYTHING*/ : 0x100/*BONE_USED_BY_HITBOX*/, 0.f);
}

ALWAYSINLINE std::wstring c_base_entity::get_grenade_name(){
  s_client_class* cc = get_client_class();
  if(cc == nullptr){
    assert(false);
    return L"";
  }

  if(cc->id != CBaseGrenadeProjectile)
    return L"";

  uptr* model = get_model();
  if(model == nullptr)
    return L"";

  assert(global->model_info != nullptr);
  i8* model_name = global->model_info->get_model_name(model);
  if(model_name == nullptr)
    return L"";

  u32 name_hash = HASH_RT(model_name);
  return L"Frag Grenade";
}

ALWAYSINLINE std::wstring c_base_entity::get_objective_name(){
  s_client_class* cc = get_client_class();
  if(cc == nullptr){
    assert(false);
    return L"";
  }

  switch(cc->id){
    default: break;
    case CRatRaceWaypoint: return L"Race waypoint";
    case CBriefcaseCaptureZone: return L"Capture zone";
    case CBriefcase: return L"Briefcase";
  }

  return L"";
}

ALWAYSINLINE bool c_base_entity::is_objective(){
  s_client_class* cc = get_client_class();
  if(cc == nullptr){
    assert(false);
    return false;
  }

  return cc->id == CRatRaceWaypoint || cc->id == CBriefcaseCaptureZone || cc->id == CBriefcase;
}

bool c_base_entity::is_considered_enemy(c_base_entity* target){
  if(target == nullptr || target == this)
    return false;

  // should target friends
  if(target->is_player()){
    c_base_player* me = (c_base_player*)this;
    if(!config->aimbot.target_friends && me->is_steam_friend((c_base_player*)target))
      return false;
  }

  return true;
}

ALWAYSINLINE void c_base_entity::set_abs_origin(vec3 pos){
  assert(global->set_abs_origin_addr != nullptr);
  assert(this != nullptr);
  utils::call_thiscall<void, vec3&>(global->set_abs_origin_addr, this, pos);
}

ALWAYSINLINE void c_base_entity::set_abs_angles(vec3 angles){
  assert(global->set_abs_angles_addr != nullptr);
  assert(this != nullptr);
  utils::call_thiscall<void, vec3&>(global->set_abs_angles_addr, this, angles);
}