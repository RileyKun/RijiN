#pragma once

class c_entity_sphere_query{
public:
  c_entity_sphere_query(vec3& center, float radius, i32 flag_mask = 0, i32 partition_mask = (1 << 7));

  i32 list_index;
  i32 list_count;
  c_base_entity* list[256];
};