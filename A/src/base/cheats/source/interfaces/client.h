#pragma once

class c_interface_client{
public:

  ALWAYSINLINE bool write_user_cmd_delta_to_buffer(s_bf_write* buf, i32 from, i32 to, bool isnewcmd){
    assert(buf != nullptr);
    return utils::internal_virtual_call<bool, s_bf_write*, i32, i32, bool>(base_interface->write_user_cmd_delta_to_buffer_index, this, buf, from, to, isnewcmd);
  }
};