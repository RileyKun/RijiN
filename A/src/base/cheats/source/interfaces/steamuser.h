#pragma once

class c_interface_steamuser{
public:

  ALWAYSINLINE bool is_logged_in(){
    return utils::internal_virtual_call<bool>(base_interface->is_logged_in_index, this);
  }

  ALWAYSINLINE bool get_steam_id(void* out){
    assert(out != nullptr);
    if(!is_logged_in()) // We might get an invalid id from this.
      return false;

    static i8 buf[8];
    memset(buf, 0, sizeof(buf));
    utils::internal_virtual_call<void*>(base_interface->get_steam_id_index, this, buf);
    memcpy(out, buf, sizeof(buf));

    return true;
  }
};