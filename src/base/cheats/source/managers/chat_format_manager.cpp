#include "../../../link.h"

i32 c_base_chat_format_manager::hud_chatline_insert_and_colorize_text(wchar_t* buf, i32 index, wchar_t* new_buf, u32 new_buf_len){
  if(buf == nullptr || new_buf == nullptr)
    return e_chat_format_type_none;

  if(global_internal->skip_chatline_insert_hook || buf == nullptr)
    return e_chat_format_type_none;

  c_chat_format_settings* settings = get_settings();
  if(settings == nullptr)
    return e_chat_format_type_none;

  if(settings->block_server_messages && index == 0)
    return e_chat_format_type_no_call;

  if(settings->auto_mute_bots_chats){
    if(global_internal->base_cheat_detection != nullptr){
      if(global_internal->base_cheat_detection->is_cheating_bot(index))
        return e_chat_format_type_no_call;
    }
  }

  if(!index)
    return e_chat_format_type_none;

  global_internal->skip_get_player_info_hook = true;
  s_player_info info;
  bool result = global_internal->engine->get_player_info(index, &info);
  global_internal->skip_get_player_info_hook = false;
  if(!result)
    return e_chat_format_type_none;

  wchar_t name_buf[128];
  convert::str2wstr(info.name, name_buf, sizeof(name_buf));

  u32 buf_len  = wcslen(buf);
  u32 name_len = wcslen(name_buf);

  i32 start = -1;
  i32 end   = -1;
  if(!utils::find_start_and_end(buf, name_buf, &start, &end))
    return e_chat_format_type_none;

  if(buf[0] == L'\x2')
    buf[0] = L'\x3';

  // Prevent team colour from "leaking" into the chat message.
  {
    i32 x1_pos = (start == 1) ? ((name_len + 1 < buf_len) ? name_len + 1 : 0) : (name_len + start < buf_len) ? (name_len + start) : 0;
    if(!x1_pos)
      return false;

    buf[x1_pos] = L'\x1';
  }

  if(!settings->chat_tags)
    return e_chat_format_type_none;

  std::wstring tag = get_tag(index);
  u32          clr = get_tag_colour(index);

  if(tag.empty())
    return e_chat_format_type_none;

  i32 write_amount =  (i32)buf_len + tag.length() + 8; // buf len, tag len, hex clr len, plus '\x?'
  assert(write_amount < 1024);

  if(write_amount >= new_buf_len){
    assert(false && "new_buf_len is too small");
    return e_chat_format_type_none;
  }

  memset(new_buf, 0,  new_buf_len);
  wsprintfW(new_buf, WXOR(L"\x7%06X%ls\x3%ls"), clr, tag.c_str(), buf);

  return e_chat_format_type_replace_msg;
}

std::wstring c_base_chat_format_manager::get_tag(i32 index){
  if(global_internal->base_cheat_detection == nullptr)
    return L"";

  c_chat_format_settings* settings = get_settings();
  if(settings == nullptr)
    return L"";

  if(index == global_internal->localplayer_index)
    return settings->chat_tags_localplayer ? WXOR(L"(You) ") : L"";

  if(global_internal->base_cheat_detection->is_friend(index) && settings->chat_tags_friends)
    return settings->chat_tags_friends ? WXOR(L"(Friend) ") : L"";

  if(settings->chat_tags_cheaters){
    if(global_internal->base_cheat_detection->is_cheating_bot(index))
      return WXOR(L"(Bot) ");

    if(global_internal->base_cheat_detection->is_cheating(index))
      return WXOR(L"(Cheater) ");
  }

  return L"";
}

u32 c_base_chat_format_manager::get_tag_colour(i32 index){
  if(index == global_internal->localplayer_index || global_internal->base_cheat_detection->is_friend(index))
    return 0x60FF64;

  if(global_internal->base_cheat_detection->is_cheating(index) || global_internal->base_cheat_detection->is_cheating_bot(index))
    return 0xFF2D1E;

  return 0xFFFFFF;
}