#pragma once

class c_interface_model_info{
public:
  ALWAYSINLINE u8* get_model_name(uptr* model){
    assert(model != nullptr);
    return utils::virtual_call<3, u8*, uptr*>(this, model);
  }

  ALWAYSINLINE s_studio_hdr* get_studio_hdr(void* model){
    assert(model != nullptr);
    return utils::virtual_call<28, s_studio_hdr*, void*>(this, model);
  }
};