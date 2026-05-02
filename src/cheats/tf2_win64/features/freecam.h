#pragma once

class c_freecamera : public c_base_free_camera{
public:
  c_freecam_settings* get_settings() override{
    return &config->freecam;
  }
};

CLASS_EXTERN(c_freecamera, freecam);