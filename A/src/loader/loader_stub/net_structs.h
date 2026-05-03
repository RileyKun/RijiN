#pragma once

#pragma pack(push,1)

struct cpu_extensions{
  bool sse;
  bool sse2;
  bool sse3;
  bool ssse3;
  bool sse41;
  bool sse42;
  bool avx;
  bool avx2;
  bool avx512;

  void fill(){
    __builtin_cpu_init();

    sse     = __builtin_cpu_supports("sse");
    sse2    = __builtin_cpu_supports("sse2");
    sse3    = __builtin_cpu_supports("sse3");
    ssse3   = __builtin_cpu_supports("ssse3");
    sse41   = __builtin_cpu_supports("sse4.1");
    sse42   = __builtin_cpu_supports("sse4.2");
    avx     = __builtin_cpu_supports("avx");
    avx2    = __builtin_cpu_supports("avx2");
    avx512  = __builtin_cpu_supports("avx512f");
  }
};

#define c_net_request_cheat_info_cmd MAKE_CMD("NET_REQUEST_CHEAT_INFO")
class c_net_request_cheat_info : public c_net_fragment {
public:
  ALWAYSINLINE inline c_net_request_cheat_info(){
    u32 new_len = XOR32(sizeof(c_net_request_cheat_info)) - XOR32(sizeof(c_net_fragment));
      
    memset((uptr)this + XOR32(sizeof(c_net_fragment)), 0, new_len);
    cmd = c_net_request_cheat_info_cmd;
    len = new_len;
  } 

  i8 key[32];
  cpu_extensions extensions;
  u32 object_pos;
};

class c_net_receive_cheat_info : public c_net_fragment{
public:
  bool x86;
  u32 sizeof_image;
  u32 imports_count;
  s_gen_required_import imports[0];
};

#define c_net_request_dxgidmp_cmd MAKE_CMD("NET_REQUEST_DXGI")
class c_net_request_dxgidmp : public c_net_fragment{
public:
  ALWAYSINLINE inline c_net_request_dxgidmp(){
    u32 new_len = XOR32(sizeof(c_net_request_dxgidmp)) - XOR32(sizeof(c_net_fragment));

    memset((uptr)this + XOR32(sizeof(c_net_fragment)), 0, new_len);
    cmd = c_net_request_dxgidmp_cmd;
    len = new_len;
  }

  i8 key[32];
  bool x86;
};

class c_net_receive_dxgidmp : public c_net_fragment{
public:
  u32 buffer_len;
  u8  buffer[0];
};

#define c_net_request_cheat_cmd MAKE_CMD("NET_REQUEST_CHEAT")
class c_net_request_cheat : public c_net_fragment {
public:
  ALWAYSINLINE inline c_net_request_cheat(){
    u32 new_len = XOR32(sizeof(c_net_request_cheat)) - XOR32(sizeof(c_net_fragment));
        
    memset((uptr)this + XOR32(sizeof(c_net_fragment)), 0, new_len);
    cmd = c_net_request_cheat_cmd;
    len = new_len;
  } 
  
  i8 key[32];
  cpu_extensions extensions;
  u32 object_pos;
  s_gen_input input;
};

// Small wrapper to receive gen output
class c_net_receive_cheat : public c_net_fragment, public s_gen_output {
public:
};

#pragma pack(pop)