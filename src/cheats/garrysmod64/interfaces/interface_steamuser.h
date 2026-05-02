#pragma once

class c_interface_steamuser{
public:

  ALWAYSINLINE bool is_logged_in(){
    return utils::virtual_call64<1, bool>(this);
  }

  ALWAYSINLINE bool get_steam_id(void* out){
    if(!is_logged_in()) // We might get an invalid id from this.
      return false;

    static i8 buf[8];
    memset(buf, 0, sizeof(buf));
    utils::virtual_call64<2, void*>(this, buf);
    memcpy(out, buf, sizeof(buf));

    return true;
  }

};