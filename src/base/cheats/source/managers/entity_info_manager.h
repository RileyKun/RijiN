#pragma once

class s_entity_info{
public:
  uptr id;
  u32  class_id;
  u32  class_name_hash;
  u32  model_hash;

  vec3  origin;
  vec3  abs_origin;
  vec3  abs_angles;
  vec3  obb_min;
  vec3  obb_max;
  void* model_ptr;

  bool  is_on_team;
  i32   meter_dist;
  float vector_dist;
};

class c_base_entity_info_manager{
public:
  s_entity_info* table   = nullptr;
  u32 max_entities_count = 0;

  // Will there ever be overrides? No idea.
  virtual u32 size_of_struct(){
    return sizeof(s_entity_info);
  }

  virtual void reset(){
    if(!max_entities_count)
      return;

    memset(table, 0, size_of_struct() * max_entities_count);
  }

  virtual void init(u32 max_entities = 2048);
  virtual void think(u32& stage);
  virtual s_entity_info* get_data(const i32 index){
    if(!max_entities_count)
      return nullptr;

    if(index < 0 || index >= max_entities_count){
      return nullptr;
    }

    return &table[index];
  }

  virtual void unload(){
    if(table == nullptr)
      return;

    DBG("[!] c_base_entity_info_manager::reset\n");
    max_entities_count = 0;
    free(table);
    table = nullptr;
  }
};

CLASS_EXTERN(c_base_entity_info_manager, entity_info);