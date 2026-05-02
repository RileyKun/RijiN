#include "../link.h"

void* c_base_entity::get_scripted_entity(){
  s_client_class* cc = get_client_class();
  if(cc == nullptr)
    return nullptr;

  switch(cc->id){
    default: break;
    case CSENT_point:
    {
      return (void*)((uptr)this + (uptr)DT_SENT_point_m_ScriptedEntity);
    }
    case CSENT_anim:
    {
      return (void*)((uptr)this + (uptr)DT_SENT_anim_m_ScriptedEntity);
    }
    case CSENT_AI:
    {
       return (void*)((uptr)this + (uptr)DT_SENT_AI_m_ScriptedEntity);
    }
    case CWeaponSWEP:
    {
      return (void*)((uptr)this + (uptr)DT_WeaponSWEP_m_ScriptedEntity);
    }
    case CLuaNextBot:
    {
      return (void*)((uptr)this + (uptr)DT_LuaNextBot_m_ScriptedEntity);
    }
  }

  return nullptr;
}

ALWAYSINLINE std::wstring c_base_entity::get_print_name(){
  s_client_class* cc = get_client_class();
  if(cc == nullptr)
    return L"";

  if(cc->id == CWeaponSWEP || is_combat_weapon()){

    i8* print_name = utils::virtual_call64<367, i8*>(this);
    if(print_name == nullptr)
      return L"";

    assert(global->localization_addr != nullptr);
    wchar_t* delocalized_text = utils::virtual_call64<2, wchar_t*, i8*>(global->localization_addr, print_name);
    if(delocalized_text != nullptr)
      return delocalized_text;

    return convert::str2wstr(print_name);
  }

  return L"";
}

ALWAYSINLINE c_base_entity* c_base_entity::get_vehicle(){
  if(player_vehicle == 0)
    return nullptr;

  return (c_base_entity*)global->entity_list->get_entity_handle(player_vehicle);
}

ALWAYSINLINE c_base_entity* c_base_entity::get_owner(){
  if(owner_handle == 0)
    return nullptr;

  return (c_base_entity*)global->entity_list->get_entity_handle(owner_handle);
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

  invalidate_setup_bones();

  bool in_vehicle = is_in_vehicle();
  vec3 old_abs_origin = get_abs_origin();
  vec3 old_abs_angles = get_abs_angles();
  
  // FRAME_NET_UPDATE_POSTDATAUPDATE_END does not yet call set_abs_origin
  // If we don't do this, we are setting up bones for the last abs origin instead of the new networked one
  if(get_index() != global->localplayer_index && !in_vehicle){
    vec3 t = origin(true);
    set_abs_origin(t);

    if(this->is_player()){
      c_base_player* p = (c_base_player*)this;

      vec3 a = p->viewangles();
      a.x = a.z = 0.f;
      set_abs_angles(a);
    }
  }

  bool result = utils::call_fastcall64<bool, matrix3x4*, i32, i32, float>(gen_internal->decrypt_asset(global->base_animating_setupbones_hook_trp), renderable(), matrix, math::smallest(hdr->num_bones, MAX_SETUP_BONES), used_by_anything ? 0x7FF00/*BONE_USED_BY_ANYTHING*/ : 0x100/*BONE_USED_BY_HITBOX*/, 0.f);

  if(!in_vehicle){
    set_abs_origin(old_abs_origin);
    set_abs_angles(old_abs_angles);
  }

  return result;
}

void c_base_entity::set_abs_origin(vec3 pos){
  assert(global->set_abs_origin_addr != nullptr);
  utils::call_fastcall64<void, vec3&>(global->set_abs_origin_addr, this, pos);
}

void c_base_entity::set_abs_angles(vec3 angles){
  assert(global->set_abs_angles_addr != nullptr);
  utils::call_fastcall64<void, vec3&>(global->set_abs_angles_addr, this, angles);
}

vec3& c_base_entity::get_abs_origin(){
  return utils::virtual_call64<9, vec3&>(this);
}

vec3& c_base_entity::get_abs_angles(){
  return utils::virtual_call64<10, vec3&>(this);
}

c_entity_data* c_base_entity::data(){
  return utils::get_entity_data(get_index());
}