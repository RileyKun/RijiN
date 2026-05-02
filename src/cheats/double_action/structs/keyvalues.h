#pragma once

class c_keyvalues{
public:
  static c_keyvalues* create(i8* name);

  void set_int(i8* name, i32 value);
  void set_bool(i8* name, bool value);
  void set_float(i8* name, float value);
  void set_string(i8* name, i8* value);
  void transmit();
};