#include "../link.h"

c_entity_sphere_query::c_entity_sphere_query(vec3& center, float radius, i32 flag_mask, i32 partition_mask){
  assert(global->entity_sphere_query_addr != nullptr);
  utils::call_fastcall64<void, vec3&, float, i32, i32>(global->entity_sphere_query_addr, this, center, radius, flag_mask, partition_mask);
}