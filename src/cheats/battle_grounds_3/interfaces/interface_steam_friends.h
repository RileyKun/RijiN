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
    // Seems to have an extra arg in this game. It passes the steamid, some sort of hex value and then the flag.
    return utils::virtual_call<11, bool, i32, i32, i32>(this, steamid.m_steamid.comp.account_id, 0x1100001, flags);
  }
};