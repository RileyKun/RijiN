#pragma once

class c_interface_client{
public:

  ALWAYSINLINE void frame_stage_notify(u32 stage){
    utils::virtual_call64<35, void, u32>(this, stage);
  }

  ALWAYSINLINE bool write_user_cmd_delta_to_buffer(s_bf_write* buf, i32 from, i32 to, bool isnewcmd){
    return utils::virtual_call64<23, bool, s_bf_write*, i32, i32, bool>(this, buf, from, to, isnewcmd);
  }

  ALWAYSINLINE void render_view(s_view_setup& view, i32 flags, i32 draw){
    utils::virtual_call64<27, void,  s_view_setup&, i32, i32>(this, view, flags, draw);
  }

  bool get_player_view(s_view_setup& view){
    return utils::virtual_call64<59, bool, s_view_setup&>(this, view);
  }
};
