#include "../link.h"

ALWAYSINLINE float c_base_weapon::get_spread(){
  return this->spread;
}

ALWAYSINLINE c_base_entity* c_base_weapon::get_owner(){
  return (c_base_entity*)global->entity_list->get_entity_handle(owner_handle);
}

ALWAYSINLINE i8* c_base_weapon::get_class_name(){
  assert(global->get_class_name_addr != nullptr);
  return utils::call_thiscall<i8*>(global->get_class_name_addr, this);
}

ALWAYSINLINE css_weapon_file_info* c_base_weapon::get_weapon_info(){
  //assert(global->look_weapon_info_slot != nullptr);
  //assert(global->file_weapon_info_from_handle != nullptr);
//
  //i8* class_name = get_class_name();
  //if(class_name == nullptr)
  //  return nullptr;
//
  //i32 slot  = utils::call_cdecl<i32, i8*>(global->look_weapon_info_slot, get_class_name());
  //if(slot == -1)
  //  return nullptr;
//
  //return utils::call_cdecl<css_weapon_file_info*, i32>(global->file_weapon_info_from_handle, slot);
  return nullptr;
}