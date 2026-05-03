#pragma once

class c_interface_steamfriends{
public:

  // WARNING: Do not store this string.
  ALWAYSINLINE i8* get_persona_name(){
    return utils::virtual_call<0, i8*>(this);
  }

  // WARNING: Do not store this string.
  ALWAYSINLINE i8* get_persona_name_other(c_steamid steamid){
    return utils::virtual_call<7, i8*, c_steamid>(this, steamid);
  }

  ALWAYSINLINE bool has_friend(c_steamid steamid, i32 flags = 0x4){
    return utils::virtual_call<17, bool, c_steamid, i32>(this, steamid, flags);
  }
};