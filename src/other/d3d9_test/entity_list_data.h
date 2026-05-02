#pragma once

struct s_cheater_data{
  bool is_cheater;
  bool is_bot;
  bool is_known_actor;
  wchar_t known_alias[33];
};

struct s_entity_list_data{
  i32     entity_index;
  i32     team;

  u32     class_name_hash;
  u32     print_name_hash;
  wchar_t class_name[1028];
  wchar_t print_name[1028];
  bool    set = false;
};

#define MAX_ENTITY_LIST_DATA_ENTRIES 128
class c_entity_list_data{
public:

  void reset(){
    memset(this, 0, sizeof(c_entity_list_data));
    selected_index = -1;
  }

  i32 get_max_entries(){
    return MAX_ENTITY_LIST_DATA_ENTRIES;
  }

  i32 get_list_count(){
    return list_count;
  }

  i32 max_team_process_count(){
    return 4;
  }

  void calculate_item_count(){
    list_count = 0;
    for(i32 i = 0; i < MAX_ENTITY_LIST_DATA_ENTRIES; i++){
      if(!data[i].set)
        continue;

      list_count++;
    }
  }

  bool add_entry(i32 entity_index, i32 team, wchar_t* class_name, wchar_t* print_name){
    assert(entity_index > 0);
    if(class_name == nullptr){
      assert(class_name != nullptr);
      return false;
    }

    bool added = false;
    u32 class_hash = WHASH_RT(class_name);
    u32 print_hash = WHASH_RT(print_name);

    // Lets avoid having the same class in the list. Detect duped entries and ignore them.
    for(i32 i = 0; i < MAX_ENTITY_LIST_DATA_ENTRIES; i++){
      if(!data[i].set)
        continue;

      if(data[i].class_name_hash == class_hash){
        added = true;
        break;
      }
    }

    if(added)
      return true;

    // Find an empty entry and add it.
    for(i32 i = 0; i < MAX_ENTITY_LIST_DATA_ENTRIES; i++){
      if(data[i].set)
        continue;

      data[i].print_name_hash = print_hash;
      data[i].class_name_hash = class_hash;
      wsprintfW(data[i].class_name, WXOR(L"%ls"), class_name);
      if(print_name != nullptr)
        wsprintfW(data[i].print_name, WXOR(L"%ls"), print_name);

      data[i].team         = team;
      data[i].entity_index = entity_index;
      data[i].set          = true;
      added = true;
      break;
    }

    assert(added == true);
    calculate_item_count();
    return added;
  }

  bool remove_entry_from_entity_index(i32 entity_index){
    for(i32 i = 0; i < MAX_ENTITY_LIST_DATA_ENTRIES; i++){
      if(!data[i].set)
        continue;

      if(i == selected_index)
        selected_index = -1;

      data[i].class_name_hash = 0;
      data[i].entity_index    = 0;
      data[i].set             = false;
    }

    calculate_item_count();
    return true;
  }

  ALWAYSINLINE s_entity_list_data* get_data_from_pos(i32 i){
    assert(i >= 0 && i < MAX_ENTITY_LIST_DATA_ENTRIES);
    return &data[i];
  }

  s_entity_list_data* get_data_from_entity_index(i32 entity_index){
    for(i32 i = 0; i < MAX_ENTITY_LIST_DATA_ENTRIES; i++){
      if(!data[i].set)
        continue;

      if(data[i].entity_index != entity_index)
        continue;

      return &data[i];
    }

    return nullptr;
  }

  s_entity_list_data selected_item;
  i32                selected_index = -1;
  i32                list_count     = 0;
  s_entity_list_data data[128];
};


CLASS_EXTERN(c_entity_list_data, entity_list_data);