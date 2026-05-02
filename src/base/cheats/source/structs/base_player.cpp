#include "../../../link.h"

NEVERINLINE s_player_info c_internal_base_player::info(){
  s_player_info info;
  info.friends_id = 0;
  info.name[0]    = 0;
  info.guid[0]    = 0;
  global_internal->engine->get_player_info(get_index(), &info);
  return info;
}

NEVERINLINE void c_internal_base_player::update_clientside_animations(){
  assert(global_internal->update_clientside_animations != nullptr);
  #if defined(__x86_64__)
    utils::call_fastcall64<void>(global_internal->update_clientside_animations, this);
  #else
    utils::call_thiscall<void>(global_internal->update_clientside_animations, this);
  #endif
}

NEVERINLINE c_internal_base_weapon* c_internal_base_player::get_weapon(){
  if(weapon_handle() <= -1)
    return nullptr;

  return (c_internal_base_weapon*)global_internal->entity_list->get_entity_handle(weapon_handle());
}

NEVERINLINE c_internal_base_weapon* c_internal_base_player::get_weapon_from_belt(i32 id, bool id_check){
  create_offset_hash("DT_BaseCombatCharacter_m_hMyWeapons");
  #if defined(__x86_64__)
    for(u32 i = 0; i < 47; i++){
      c_internal_base_weapon* wep = (c_internal_base_weapon*)global_internal->entity_list->get_entity_handle(*(u64*)((uptr)this + (uptr)offset + 4 * i));
      if(wep == nullptr)
        continue;

      if(wep->get_id() == id && id_check || wep->get_slot() == id && !id_check)
        return wep;
    }
  #else
    u32** belt = (u32**)((u32)this + offset);
  
    for(u32 i = 0; belt[i] != nullptr; i++){
      c_internal_base_weapon* wep = (c_internal_base_weapon*)global_internal->entity_list->get_entity_handle(belt[i]);
  
      if(wep == nullptr)
        continue;
  
      if(wep->get_id() == id && id_check || wep->get_slot() == id && !id_check)
        return wep;
    }
  #endif

  return nullptr;
}

bool c_internal_base_player::is_steam_friend(){
  return (get_index() == global_internal->localplayer_index) ? true : global_internal->base_cheat_detection->is_friend(get_index());
}
  
// For TF2: Find inside of CTFWeaponBaseGun::FireBullet x-ref FX_FireBullets
vec3 c_internal_base_player::shoot_pos(){
  vec3 shoot_pos;

  create_offset_hash("shoot_pos");
  if(global_internal->localplayer == this){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, vec3&>(offset, this, shoot_pos);
    STACK_CHECK_END;
  }
  else
    shoot_pos = eye_pos();

  return shoot_pos;
}

c_internal_base_entity* c_internal_base_player::get_ground_entity(){
  return ground_entity() > 0 ? global_internal->entity_list->get_entity_handle(ground_entity()) : nullptr;
}