#pragma once

class c_server{
public:
  
  ALWAYSINLINE inline c_net_receive_info* get_serverinfo(){
    c_net_fragment request{};
    request.cmd = XOR32(c_net_receive_info_cmd);

    return (c_net_receive_info*)request.transmit(); 
  }
  
  ALWAYSINLINE inline c_net_receive_cheat_info* request_cheat_info(u32 pos){
    c_net_request_cheat_info request{};

    I(memcpy)(request.key, global->stub_data->key, XOR32(sizeof(c_stub_data::key)));
    request.extensions.fill();
    request.object_pos = pos;

    return (c_net_receive_cheat_info*)request.transmit();
  }

  ALWAYSINLINE inline c_net_receive_dxgidmp* request_dxgidmp(bool x86){
    c_net_request_dxgidmp request{};

    I(memcpy)(request.key, global->stub_data->key, XOR32(sizeof(c_stub_data::key)));
    request.x86 = x86;

    return (c_net_receive_dxgidmp*)request.transmit();
  }

  ALWAYSINLINE inline c_net_receive_cheat* request_cheat(u32 pos, s_gen_input* input){
    c_net_request_cheat request{};

    I(memcpy)(request.key, global->stub_data->key, XOR32(sizeof(c_stub_data::key)));
    request.extensions.fill();
    request.object_pos = pos;
    I(memcpy)(&request.input, input, sizeof(s_gen_input));

    return (c_net_receive_cheat*)request.transmit();
  }
};

CLASS_EXTERN(c_server, server);