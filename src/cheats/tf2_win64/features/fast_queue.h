#pragma once

class c_fast_queue : public c_base_fast_queue{
public:
  c_fast_queue_settings* get_settings() override{
    return &config->fq;
  }
};

CLASS_EXTERN(c_fast_queue, fast_queue);