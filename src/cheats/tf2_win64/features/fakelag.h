#pragma once

class c_fakelag{
public:
  void run();
  void anti_backtrack();
  void peek_fakelag();
  void desync_sequences();
  void find_headshot_classes();
  bool active();
  bool run_method();
  void anti_backtrack_fail(c_base_player* player);
  bool can_fakelag();

  i32  ticks_to_break_lc;
  bool headshot_threat;
};

CLASS_EXTERN(c_fakelag, fakelag);