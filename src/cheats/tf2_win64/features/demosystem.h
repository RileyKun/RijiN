#pragma once

class c_demosystem{
public:
  void createmove();
  void begin(i8* name);
  void end();
  void apply_settings();
  void format_dir(bool bypass_check);
  void record_player_event(c_base_player* player, i8* info);
  std::string get_match_name();
};

CLASS_EXTERN(c_demosystem, demosystem);