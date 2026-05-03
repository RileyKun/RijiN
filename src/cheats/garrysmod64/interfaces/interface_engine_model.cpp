#include "../link.h"

void c_interface_engine_model::force_material_override(void* material){
  global->override_material_call = true;
  utils::virtual_call64<1, void, void*, i32>(this, material, 0);
  global->override_material_call = false;
}