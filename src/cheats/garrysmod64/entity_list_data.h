#pragma once

struct s_entity_list_data{
  bool          set = false;
  i32           entity_index;
  u32           class_name_hash;
  u32           print_name_hash;
  wchar_t       class_name[1028];
  wchar_t       print_name[1028];
  c_entity_list_data entity_data;
  bool is_valid(){
    return (set && class_name_hash > 0);
  }
};

#define MAX_ENTITY_LIST_DATA_ENTRIES 256
class c_entity_list_manager{
public:

  void reset(){
    DBG("[!] reset entity list data\n");
    memset(this, 0, sizeof(c_entity_list_data));
    selected_index = -1;
  }

  void reset_selected_item(){
    DBG("[!] reset_selected_item\n");
    sync_selected_item_entity_data();
    selected_index = -1;
    memset(&selected_item, 0, sizeof(s_entity_list_data));
  }

  void sync_selected_item_entity_data(){
    if(selected_index == -1)
      return;
    
    if(!selected_item.class_name_hash)
      return;
    
    s_entity_list_data* entry = get_data_from_hash(selected_item.class_name_hash);
    if(entry == nullptr)
      return;

    memcpy(&entry->entity_data, &selected_item.entity_data, sizeof(c_entity_list_data));
  }

  i32 get_max_entries(){
    return MAX_ENTITY_LIST_DATA_ENTRIES;
  }

  i32 get_list_count(){
    return list_count;
  }

  void calculate_item_count(){
    i32 c = 0;
    for(i32 i = 0; i < MAX_ENTITY_LIST_DATA_ENTRIES; i++){
      if(!data[i].is_valid())
        continue;

      c++;
    }

    list_count = c;
  }

  bool add_entry(i32 entity_index, wchar_t* class_name, wchar_t* print_name){
    assert(entity_index > 0);
    if(class_name == nullptr){
      assert(class_name != nullptr);
      return false;
    }

    if(wcslen(class_name) == 0)
      return false;

    u32 class_hash = WHASH_RT(class_name);
    u32 print_hash = print_name != nullptr ? WHASH_RT(print_name) : 0;

    bool skip = false;

    // Lets avoid having the same class in the list. Detect duped entries and ignore them.
    for(i32 i = 0; i < MAX_ENTITY_LIST_DATA_ENTRIES; i++){
      s_entity_list_data* entry = &data[i];
      if(!entry->is_valid())
        continue;

      if(entry->class_name_hash == class_hash){
        skip = true;
        break;
      }
    }

    if(skip)
      return true;
  
    // Find an empty entry and add it.
    for(i32 j = 0; j < MAX_ENTITY_LIST_DATA_ENTRIES; j++){
      s_entity_list_data* entry = &data[j];
      if(entry->is_valid())
        continue;

      entry->print_name_hash = print_hash;
      entry->class_name_hash = class_hash;

      wsprintfW(entry->class_name, WXOR(L"%ls"), class_name);
      if(print_name != nullptr)
        wsprintfW(entry->print_name, WXOR(L"%ls"), print_name);
      else
        memset(entry->print_name, 0, sizeof(entry->print_name));


      entry->entity_index = entity_index; 
      
      // Copy global world esp settings over to entity data settings.
      memcpy(&entry->entity_data.esp_settings, &config->world_esp, sizeof(c_world_esp_settings));
      memcpy(&entry->entity_data.chams_settings, &config->world_chams, sizeof(c_world_chams_settings));
      memcpy(&entry->entity_data.glow_settings, &config->world_glow, sizeof(c_world_glow_settings));

      entry->set          = true;
      break;
    }

    calculate_item_count();
    return true;
  }

  ALWAYSINLINE s_entity_list_data* get_data_from_hash(u32 hash){
    if(!hash){
      DBG("[-] get_data_from_hash: hash was zero\n");
      return nullptr;
    }
    
    for(i32 i = 0; i < MAX_ENTITY_LIST_DATA_ENTRIES; i++){
      if(!data[i].is_valid())
        continue;

      if(data[i].class_name_hash != hash)
        continue;

      return &data[i];
    }

    return nullptr;
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
private:
  s_entity_list_data data[MAX_ENTITY_LIST_DATA_ENTRIES];
};

CLASS_EXTERN(c_entity_list_manager, entity_list_data);