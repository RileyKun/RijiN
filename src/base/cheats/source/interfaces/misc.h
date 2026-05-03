class c_base_hud_chat{
public:
  ALWAYSINLINE void chat_print(i8* arg){
    utils::internal_virtual_call<void, i32, i32, i8*>(base_interface->chat_print_index, this, 1, 0, arg);
  }
};
#pragma once

class c_sfx_table{
public:
  ALWAYSINLINE i8* get_name(){
    return utils::internal_virtual_call<i8*>(base_interface->sfx_table_get_name_index, this);
  }
};

class c_achievement{
public:
  ALWAYSINLINE i32 get_id(){
    return utils::internal_virtual_call<i32>(base_interface->achievement_get_id_index, this);
  }
};

class c_achievement_mgr{
public:
  ALWAYSINLINE c_achievement* get_achievement_by_index(i32 index){
    return utils::internal_virtual_call<c_achievement*, i32>(base_interface->get_achievement_by_index_index, this, index);
  }

  ALWAYSINLINE i32 get_achievement_count(){
    return utils::internal_virtual_call<i32>(base_interface->get_achievement_count_index, this);
  }

  ALWAYSINLINE void award_achievement(i32 id){
    utils::internal_virtual_call<void, i32>(base_interface->award_achievement_index, this, id);
  }
};