#pragma once

class c_interface_steamfriends{
public:

  // WARNING: Do not store this string.
  ALWAYSINLINE i8* get_persona_name(){
    return utils::internal_virtual_call<i8*>(base_interface->get_persona_name_index, this);
  }

  // WARNING: Do not store this string.
  ALWAYSINLINE i8* get_persona_name_other(c_steamid steamid){
    return utils::internal_virtual_call<i8*, c_steamid>(base_interface->get_persona_name_other_index, this, steamid);
  }

  ALWAYSINLINE void game_overlay_to_web_page(std::string url){
    utils::internal_virtual_call<void, const i8*>(base_interface->game_overlay_to_web_page_index, this, url.c_str());
  }

  ALWAYSINLINE bool has_friend(c_steamid steamid, i32 flags = 0x4){
    return utils::internal_virtual_call<bool, c_steamid, i32>(base_interface->has_friend_index, this, steamid, flags);
  }
};