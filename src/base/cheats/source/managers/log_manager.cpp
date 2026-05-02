#include "../../../link.h"

void c_base_log_manager::on_game_event(c_game_event* event){
  if(event == nullptr)
    return;

  const float  time = math::time();

  u32     name_hash = HASH_RT(event->get_name());
  wchar_t buf[1024];

  wchar_t player_name[1024];
  wchar_t steamid[1024];
  memset(steamid, 0, sizeof(steamid));
  memset(player_name, 0, sizeof(player_name));

  switch(name_hash){
    default: break;
    case HASH("player_connect_client"):
    case HASH("player_disconnect"):
    {
      if(event->get_bool(XOR("bot")))
        return;

      i8* name      = event->get_string(XOR("name"));
      i32 index     = event->get_int(XOR("index"));
      i8* networkid = event->get_string(XOR("networkid"));

      if(name == nullptr || networkid == nullptr){
        DBG("[-] c_base_log_manager::on_game_event received nullptr name or networkid\n");
        return;
      }

      convert::str2wstr(name, player_name, sizeof(player_name));
      convert::str2wstr(networkid, steamid, sizeof(steamid));

      u32 sid = 0;
      convert::str_sid3_to_u32(networkid, &sid);

      I(wsprintfW)(buf, WXOR(L"[%i] %ls %ls as \"%ls\""), index, player_name, name_hash == HASH("player_disconnect") ? WXOR(L"disconnected") : WXOR(L"connecting"), steamid);
      if(connections != nullptr){
        connections->add(buf)
        ->set_type(LOG_WINDOW_STEAM_URL)
        ->set_value(sid);
      }

      break;
    }
    case HASH("server_spawn"):
    {
      wchar_t server_name[1024];
      wchar_t address[1024];
      wchar_t map_name[1024];
      i32 port = event->get_int(XOR("port"));

      i8* i8_map_name  = event->get_string(XOR("mapname"));
      i8* i8_host_name = event->get_string(XOR("hostname"));
      i8* i8_address   = event->get_string(XOR("address"));
      if(i8_map_name == nullptr || i8_host_name == nullptr || i8_address == nullptr){
        DBG("[-] server_spawn variables nullptr\n");
        return;
      }

      convert::str2wstr(i8_map_name, map_name, sizeof(map_name));
      convert::str2wstr(i8_host_name, server_name, sizeof(server_name));
      convert::str2wstr(i8_address, address, sizeof(address));

      I(wsprintfW)(buf, WXOR(L"%ls - %ls - %ls"), server_name, address, map_name);
      if(connections != nullptr)
        connections->add(buf);

      break;
    }
    case HASH("client_beginconnect"):
    {
      i8* addr = event->get_string(XOR("address"));
      i32 port = event->get_int(XOR("port"));

      if(addr == nullptr)
        return;

      i8 full_addr[1024];
      I(wsprintfA)(full_addr, XOR("%s:%i"), addr, port);

      wchar_t address[1024];
      convert::str2wstr(addr, address, sizeof(address));

      I(wsprintfW)(buf, WXOR(L"Client connecting to: %ls\n"), address);
      if(connections != nullptr){
        if(!str_utils::contains(address, WXOR(L"unknown"))){
          connections->add(buf)
            ->set_type(LOG_WINDOW_SERVER_IP)
            ->set_text_buffer(full_addr);
        }
        else
          connections->add(buf);
      }

      break;
    }
    case HASH("client_disconnect"):
    {
      static float last_sent_time = 0.f;
      if(last_sent_time > time)
        return;

      last_sent_time = time + 0.1f;

      if(connections != nullptr)
        connections->add(WXOR(L"Client disconnected."));

      break;
    }
    case HASH("vote_cast"):
    {
      i32 index   = event->get_int(XOR("entityid"));
      i32 option  = event->get_int(XOR("vote_option"));
      i32 team    = event->get_int(XOR("team"));

      s_player_info info;
      memset(&info, 0, sizeof(s_player_info));

      if(!global_internal->engine->get_player_info(index, &info))
        I(wsprintfA)(info.name, XOR("Unk player %i:"), index);

      convert::str2wstr(info.name, player_name, sizeof(player_name));
      I(wsprintfW)(steamid, WXOR(L"[U:1:%i]"), info.friends_id); // Should be safe.

      if(option <= 1)
        I(wsprintfW)(buf, WXOR(L"Vote: %ls %ls as \"%ls\" voted %ls."), utils::get_team_relation(team), steamid, player_name, option == 0 ? WXOR(L"Yes") : WXOR(L"No"));
      else
        I(wsprintfW)(buf, WXOR(L"Vote: %ls %ls as \"%ls\" voted option %i."), utils::get_team_relation(team), steamid, player_name, option);

      if(votes != nullptr){
        votes->add(buf)
          ->set_type(LOG_WINDOW_STEAM_URL)
          ->set_value(info.friends_id);
      }

      break;
    }
  }
}

void c_base_log_manager::dispatch_user_message(void* usermsg, i32 msg_type, s_bf_read* bf_read){
  if(usermsg == nullptr)
    return;

  assert(global_internal->base_dispatch_user_msg != nullptr);

  i8* name = global_internal->base_dispatch_user_msg->get_msg_name(usermsg, msg_type);
  if(name == nullptr)
    return false;

  static wchar_t buf[1024];
  const u32 name_hash = HASH_RT(name);


  wchar_t player_name[128];
  wchar_t steamid[1024];

  switch(name_hash){
    default: break;
    case HASH("VoteStart"):
    {
      bf_read->read();
      bf_read->read<i32>();
      u8 caller = bf_read->read();
      bf_read->read_str();
      bf_read->read_str();
      bool yes_no = bf_read->read<bool>();
      u8   target = bf_read->read();

      bf_read->reset();

      wchar_t caller_name[1024];
      wchar_t caller_steamid[1024];
      s_player_info caller_info;
      if(global_internal->engine->get_player_info((i32)caller, &caller_info)){
        convert::str2wstr(caller_info.name, caller_name, sizeof(caller_name));
        convert::str2wstr(caller_info.guid, caller_steamid, sizeof(caller_steamid));
      }
      else{
        I(wsprintfW)(caller_name, WXOR(L"Server/Unknown"));
        I(wsprintfW)(caller_steamid, WXOR(L"N/A"));
      }

      s_player_info info;
      memset(&info, 0, sizeof(s_player_info));

      if(!global_internal->engine->get_player_info((i32)target, &info)){
        I(wsprintfA)(info.name, XOR("Invalid/Disconnected"));
        I(wsprintfW)(steamid, WXOR(L"N/A"));
      }
      else{
        convert::str2wstr(info.name, player_name, sizeof(player_name));
        convert::str2wstr(info.guid, steamid, sizeof(steamid));
      }

      if(yes_no)
        I(wsprintfW)(buf, WXOR(L"%ls as \"%ls\" started vote against %ls who is \"%ls\"."),
          caller_steamid,
          caller_name,
          steamid,
          player_name);
      else
        I(wsprintfW)(buf, WXOR(L"%ls as \"%ls\" created a vote."), caller_steamid, caller_name);

      if(votes != nullptr)
        votes->add(buf);

      break;
    }
    case HASH("VoteFailed"):
    {
      if(votes != nullptr)
        votes->add(WXOR(L"A vote has failed."));

      break;
    }
    case HASH("VotePass"):
    {
      if(votes != nullptr)
        votes->add(WXOR(L"A vote has passed."));

      break;
    }
  }
}

void c_base_log_manager::send_net_msg(c_net_message& msg, bool rel, bool voice){

}