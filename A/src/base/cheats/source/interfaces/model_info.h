#pragma once

class c_interface_model_info{
public:
  ALWAYSINLINE i8* get_model_name(uptr* model){
    assert(model != nullptr);
    return utils::internal_virtual_call<i8*, uptr*>(base_interface->get_model_name_index, this, model);
  }

  ALWAYSINLINE s_studio_hdr* get_studio_hdr(void* model){
    assert(model != nullptr);
    return utils::internal_virtual_call<s_studio_hdr*, void*>(base_interface->get_studio_hdr_index, this, model);
  }
};