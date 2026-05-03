#include "../../../link.h"

s_studio_hdr* c_internal_base_entity::get_studio_hdr(){
  void* model = get_model();
  if(model == nullptr)
    return nullptr;

  return global_internal->model_info->get_studio_hdr(model);
}

NEVERINLINE bool c_internal_base_entity::setup_bones(matrix3x4* matrix, bool used_by_anything, bool force_animation_update){
  if(matrix == nullptr)
    return false;

  s_studio_hdr* hdr = get_studio_hdr();
  if(hdr == nullptr)
    return false;

  if(hdr->num_bones <= 0){
    DBG("[-] entity %s - %i (%i) has an invalid amount of bones.\n", get_client_class()->name, get_index(), hdr->num_bones);
    return false;
  }

  const i32  index               = get_index();
  const bool is_player           = (index >= 1 && index <= global_internal->global_data->max_clients);
  c_internal_base_player* player = is_player ? (c_internal_base_player*)this : nullptr;

  if(player != nullptr && force_animation_update){
    const float old_frametime = global_internal->global_data->frame_time;

    global_internal->global_data->frame_time = 0.f;
    player->update_clientside_animations();
    global_internal->global_data->frame_time = old_frametime;
  }

  // invalidate bone cache
  last_bone_setup_time()  = -(3.402823e+38f);
  model_bone_counter()    = -1;

  DBG_IF_NULL(global_internal->localplayer_index, "localplayer_index is not setup and hence will fail important check in setup bones!\n");
  if(index != global_internal->localplayer_index){
    vec3 uninterp_pos = origin();
    if(uninterp_pos != vec3())
      set_abs_origin(uninterp_pos);

    if(is_player){
      vec3 a = player->viewangles();
      a.x = a.y = 0.f;
      set_abs_angles(a);
    }
  }

  create_offset_hash("setup_bones");
  return utils::internal_virtual_call<bool, matrix3x4*, i32, i32, float>(offset, renderable(), matrix, math::smallest(hdr->num_bones, MAX_BONES), used_by_anything ? 0x7FF00/*BONE_USED_BY_ANYTHING*/ : 0x100/*BONE_USED_BY_HITBOX*/, 0.f);
}

NEVERINLINE void c_internal_base_entity::set_abs_origin(vec3& pos, bool allow_empty_vector){
  if(pos == vec3() && !allow_empty_vector){
    DBG("[!] c_internal_base_entity::set_abs_origin blocking %s set pos due to zero pos parameter\n", get_client_class()->name);
    return;
  }

#if !defined(__x86_64__)
  utils::call_thiscall<void, vec3&>(global_internal->set_abs_origin, this, pos);
#else
  utils::call_fastcall64<void, vec3&>(global_internal->set_abs_origin, this, pos);
#endif
}

NEVERINLINE void c_internal_base_entity::set_abs_angles(vec3& angles){
#if !defined(__x86_64__)
  utils::call_thiscall<void, vec3&>(global_internal->set_abs_angles, this, angles);
#else
  utils::call_fastcall64<void, vec3&>(global_internal->set_abs_angles, this, angles);
#endif
}

NEVERINLINE c_internal_base_entity* c_internal_base_entity::get_owner(){
  return owner_handle() > 0 ? global_internal->entity_list->get_entity_handle(owner_handle()) : nullptr;
}