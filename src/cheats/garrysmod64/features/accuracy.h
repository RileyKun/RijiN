#pragma once

class c_accuracy{
public:
  bool                is_dumping_spread = false;
  s_fire_bullets_info dump;

  bool do_spread_dump();
  bool norecoil();
  bool nospread();
  bool run();
};

CLASS_EXTERN(c_accuracy, accuracy);