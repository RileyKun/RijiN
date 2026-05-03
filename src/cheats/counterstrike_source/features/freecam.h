#pragma once

class c_freecam : public c_base_free_camera{
public:
   c_freecam_settings* get_settings() override{
    return &config->freecam;
  }
};

CLASS_EXTERN(c_freecam, freecam);