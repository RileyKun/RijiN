#pragma once

class c_viewmodel_offset : public c_base_viewmodel_offset{
public:
  c_viewmodel_offset_settings* get_settings(u32 slot = 0) override{
    return &config->vm;
  }
};

CLASS_EXTERN(c_viewmodel_offset, viewmodel);