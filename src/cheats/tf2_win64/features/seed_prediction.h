#pragma once
class c_seed_prediction : public c_base_seed_prediction{
public:
  bool active() override;
  void process_info_panel() override;
};

CLASS_EXTERN(c_seed_prediction, seed_prediction);