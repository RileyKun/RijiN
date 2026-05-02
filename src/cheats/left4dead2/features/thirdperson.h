#pragma once

class c_thirdperson : public c_base_third_person{
public:
  c_third_person_settings* get_settings() override{
    return &config->tp;
  }
};

CLASS_EXTERN(c_thirdperson, thirdperson);