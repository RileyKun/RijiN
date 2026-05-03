#pragma once

#pragma pack(push,1)
class license_object{
public:
  // core
  u32   pos;
  i8    name[128];
  u32   name_hash;
  i8    short_name[128];
  u32   target_hash;
  u32   expire;
  i8    expire_str[128];
  bool  disabled;
  bool  beta;
  bool  hypervisor;
  bool  spoofer;
  bool  vac_mode;
  u32   vac_pos;
  u32   appid;
  u32   d3dx;
  bool  runs_on_stub;
};

class c_stub_data{
public:
  bool loaded                 = false;
  i8 key[32];
  u32 license_expire_seconds  = 0;
  license_object object;
};
#pragma pack(pop)