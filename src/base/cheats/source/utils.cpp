#include "../../link.h" // base link
#include "link.h" // cheats/source/link.h

void utils::lock_mouse(const bool lock){
  if(!global_internal->focus_overlay_panel_id)
    return;

  if(global_internal->panel == nullptr)
    return;

  global_internal->panel->set_mouse_input_enabled(global_internal->focus_overlay_panel_id, lock);
}

bool utils::is_in_game(){
  assert(global_internal->client_state != nullptr);
  return global_internal->client_state->signon_state() == SIGNONSTATE_FULL || global_internal->engine->is_playing_demo();
}

c_internal_base_player* utils::localplayer(){
  return global_internal->entity_list->get_entity(global_internal->engine->get_local_player());
}

bool utils::is_match_making_server(){
  if(global_internal->cl_connectmethod == nullptr)
    return false;

  return global_internal->cl_connectmethod->str_len != 0 && HASH_RT(global_internal->cl_connectmethod->str) == HASH("matchmaking");
}

bool utils::is_listen_server(){
  if(global_internal->cl_connectmethod == nullptr)
    return false;

  return global_internal->cl_connectmethod->str_len != 0 && HASH_RT(global_internal->cl_connectmethod->str) == HASH("listenserver");
}

bool utils::is_community_server(){
  return !is_match_making_server() && !is_listen_server();
}

void utils::update_map_hash(i8* map_name){
  if(map_name == nullptr)
    return;

  if(!strlen(map_name))
    return;

  memset(global_internal->current_map, 0, sizeof(global_internal->current_map));

  if(!str_utils::contains(map_name, XOR("maps/")))
    wsprintfA(global_internal->current_map, XOR("maps/%s.bsp"), map_name);
  else
    strcpy(global_internal->current_map, map_name);

  global_internal->current_map_hash = HASH_RT(global_internal->current_map);
  DBG("[!] utils::update_map_hash %s - %X\n", global_internal->current_map, global_internal->current_map_hash);
}

void* utils::get_interface(std::wstring module, u32 name_hash, bool find_list){
  if(module.empty() || name_hash == 0)
    return nullptr;

  uptr create_interface_ptr = (uptr)utils::get_proc_address(utils::get_module_handle(WHASH_RT(module.c_str())), HASH("CreateInterface"));
  if(create_interface_ptr == 0){
    DBG("[-] %ls is invalid\n", module.c_str());
    return nullptr;
  }

  DBG("[!] %ls create interface addr 0x%p\n", module.c_str(), create_interface_ptr);

  #if defined(GMOD_CHEAT)
    void* result = utils::call_stdcall<void*, i8*, i32*>(create_interface_ptr, name.c_str(), nullptr);
    if(result != nullptr){
      DBG("[+] interface: %ls %s (%p)\n", module.c_str(), name.c_str(), result);
      return result;
    }
  #else
    uptr jmp               = 0;
    #if defined(__x86_64__)
    if(find_list){
      jmp = create_interface_ptr + (uptr)XOR32(3);   // mov r9, cs:?
      jmp = (jmp + *(u32*)jmp) + (uptr)XOR32(7);     // jmp eip + jmp rel + IMM32
    }
    else
      jmp = create_interface_ptr;
  
    // This fixed it, 
    jmp -= (uptr)0x3; // prob the + 7 should be + 4
  
    // Also only de-ref once.
    s_interface_list* list = *(s_interface_list**)jmp;
    do{
      if(name_hash == HASH_RT(list->name) && list->init_interface != nullptr){
        DBG("[+] interface: %ls %s\n", module.c_str(), list->name);
        return utils::call_stdcall<void*>(list->init_interface);
      } 
    } while(list = list->next);
    #else
    if(find_list){
      jmp = create_interface_ptr + XOR32(5);   // PUSH EBP, MOV EBP, ESP, POP EBP, JMP
      jmp = (jmp + *(i32*)jmp) + XOR32(4);     // jmp eip + jmp rel + IMM32
    }
    else
      jmp = create_interface_ptr;
  
    s_interface_list* list = **(s_interface_list***)(jmp + XOR32(6)); // PUSH EBP, MOV EBP, ESP, PUSH ESI, MOV ESI, DWORD_00000000
    do{
      if(name_hash == HASH_RT(list->name) && list->init_interface != nullptr){
        DBG("[+] interface: %ls %s\n", module.c_str(), list->name);
        return utils::call_stdcall<void*>(list->init_interface);
      }  
    } while(list = list->next);
    #endif
  #endif
  DBG("[-] ERROR: Couldn't find interface address for %ls %p\n", module.c_str(), name_hash);
  assert(false);

  return nullptr;
}

c_steamid utils::get_user_steamid(u32 friends_id){
  c_steamid steamid;
  steamid.m_steamid.comp.account_id = 0;
  if(friends_id == 0)
    return steamid;

   steamid.m_steamid.comp.account_id       = friends_id;
   steamid.m_steamid.comp.universe         = 1;
   steamid.m_steamid.comp.account_type     = 1;
   steamid.m_steamid.comp.account_instance = 1;
   steamid.m_steamid.bits                  = steamid.m_steamid.comp.account_id + 76561197960265728;

  return steamid;
}

void utils::send_con_notification(std::string str){
  if(str.empty())
    return;

  cookie_block_check_return();

  #if defined(__x86_64__)
    typedef void (__fastcall* msg_fn)(const i8* text, ...);
  #else
    typedef void (__cdecl* msg_fn)(const i8* text, ...);
  #endif

  static uptr tier0   = 0;
  static uptr msg_ptr = 0;
  static msg_fn msg   = nullptr;
  if(!msg){
    tier0 = utils::get_module_handle(HASH("tier0.dll"));
    if(tier0){
      msg_ptr = utils::get_proc_address(tier0, HASH("Msg"));
      msg     = (msg_fn)msg_ptr;
    }
  }

  i8 buf[1024];
  wsprintfA(buf, XOR("[RijiN] %s\n"), str.c_str());

  assert(msg != nullptr);
  msg(buf);
}

void utils::draw_high_ping_warning(){
  cookie_block_check_return();

  static float show_until;
  static float old_ping = global_internal->engine->get_latency(true);
         float new_ping = global_internal->engine->get_latency(true);
         float time     = math::time();
         float delta    = new_ping - old_ping;
         bool  too_high = math::abs(delta) >= math::ticks_to_time(1);

  if(too_high)
    show_until = time + 5.f;

  old_ping = new_ping;
  if(time > show_until)
    return;

  vec3i draw_pos  = vec3i(10, render->screen_size.y * 0.25f);
  vec3i size      = global_internal->gui_font_emoji->draw(draw_pos, 36, XOR(L"⚠"), too_high ? rgb(255, 0, 0) : rgb(241, 196, 15), TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_LEFT);

  wchar_t ping_str[14];
  wsprintfW(ping_str, XOR(L"%i"), math::clamp((i32)(new_ping * 1000.f), 0, 1000));

  global_internal->esp_font_large->draw(draw_pos + vec3i(size.x * 0.5f, size.y + render->rescale(14)), 12, ping_str, too_high ? rgb(255, 0, 0) : rgb(241, 196, 15), TSTYLE_OUTLINE, TALIGN_CENTER, TALIGN_TOP);
}

void utils::draw_hitbox_custom_matrix(i32 index, matrix3x4* bone_matrix, colour clr, float duration){
  assert(bone_matrix != nullptr);
  if(bone_matrix == nullptr || index <= 0 || clr.w == 0 || duration <= 0.0f)
    return;

  c_internal_base_player* player = global_internal->entity_list->get_entity(index);
  if(player == nullptr)
    return;

  if(!player->is_base_alive())
    return;

  s_studio_hdr* hdr = player->get_studio_hdr();
  if(hdr == nullptr)
    return;

  s_studio_hitbox_set* set = hdr->get_hitbox_set(player->hitbox_set());
  if(set == nullptr)
    return;

  assert(set->num_hitboxes > 0);
  if(set->num_hitboxes <= 0)
    return;

  vec3 angles, pos;
  for(i32 i = 0; i < set->num_hitboxes; i++){
    s_studio_bbox* bbox = player->get_studio_bbox(i);
    if(bbox == nullptr)
      continue;

    math::matrix_angles(bone_matrix[bbox->bone], angles, pos);
    render_debug->draw_3dbox(pos, bbox->mins * player->model_scale(), bbox->maxs * player->model_scale(), angles, clr, colour(clr.x, clr.y, clr.z, 255), duration);
  }
}

bool utils::get_cheater_data_from_network(u32 sid, u32 game_hash, c_net_request_tf_cheater_sid32_response*& _response){
  cookie_block_check_return();

  c_net_request_tf_cheater_sid32 msg;
  memcpy(&msg.config_key, gen_internal->cloud_license_key, XOR32(32));
  msg.sid       = sid;
  msg.name_hash = 0;
  msg.game      = game_hash;

  DBG("[!] Submitting [U:1:%i] to network for cheater check\n", sid);

  global_internal->is_talking_to_tfcl = true;
  c_net_request_tf_cheater_sid32_response* response = (c_net_request_tf_cheater_sid32_response*)msg.transmit(true);
  global_internal->is_talking_to_tfcl = false;

  if(response == nullptr)
    return false;

  if(!response->alias_len)
    memset(&response->alias[0], 0, XOR32(sizeof(response->alias)));

  if(!response->group_len)
    memset(&response->group[0], 0, XOR32(sizeof(response->group)));

  // This will never happen but it's good to be safe!
  if(response->alias_len > XOR32(32)){
    memset(&response->alias[0], 0, XOR32(sizeof(response->alias)));
    DBG("[-] Received invalid alias length size from server!\n");
    response->alias_len = 0;
  }

  if(response->group_len > XOR32(32)){
    memset(&response->group[0], 0, XOR32(sizeof(response->group)));
    DBG("[-] Received invalid group length size from server!\n");
    response->group_len = 0;
  }

  _response = response;
  return true;
}

struct s_submit_cheater_data{
  u32 sid;
  u32 reason;
  u32 server_type;
  u32 game;
};

void utils::submit_cheater_to_network(u32 sid, u32 reason, u32 game_hash){
  cookie_block_check_return();

  s_submit_cheater_data* data = malloc(sizeof(s_submit_cheater_data));
  if(data == nullptr)
    return;

  data->sid    = sid;
  data->reason = reason;
  data->game   = game_hash;

  if(is_listen_server())
    data->server_type = 1;
  else if(is_community_server())
    data->server_type = 2;
  else if(is_match_making_server())
    data->server_type = 3;
  else if(global_internal->engine->is_playing_demo())
    data->server_type = 4;

  utils::create_worker([](s_submit_cheater_data* data){
    while(global_internal->is_talking_to_tfcl)
      I(Sleep)(XOR32(100));

    c_net_submit_tf_cheater_sid32 msg;
    memcpy(&msg.config_key, gen_internal->cloud_license_key, XOR32(32));
    msg.sid         = data->sid;
    msg.reason      = data->reason;
    msg.server_type = data->server_type;
    msg.game        = data->game;

    global_internal->is_talking_to_tfcl = true;
    msg.transmit(true);
    global_internal->is_talking_to_tfcl = false;


    free(data);
    data = nullptr;
    return 0;
  }, data);
}

void utils::submit_bot_to_network(u32 sid){
  cookie_block_check_return();
  utils::create_worker([](u32 sid){
    while(global_internal->is_talking_to_tfcl)
      Sleep(XOR32(250));

    c_net_submit_tf_bot_sid32 msg;
    memcpy(&msg.config_key, gen_internal->cloud_license_key, XOR32(32));
    msg.sid         = sid;

    global_internal->is_talking_to_tfcl = true;
    msg.transmit(true);
    global_internal->is_talking_to_tfcl = false;

    return 0;
  }, sid);
}

i32 utils::get_entity_index_from_sid(u32 sid){
  if(!sid)
    return -1;

  for(i32 i = 1; i <= global_internal->global_data->max_clients; i++){
    c_internal_base_player* player = global_internal->entity_list->get_entity(i);
    if(player == nullptr)
      continue;

    if(player->info().friends_id != sid)
      continue;

    return i;
  }


  return -1;
}

bool utils::is_server_lagging(){
  if(global_internal->engine->is_playing_demo())
    return false;
  
  return global_internal->engine->get_avg_packets(false) < (float)math::time_to_ticks(0.9f);
}

bool utils::is_tick_rate_low(){
  if(global_internal->engine->is_playing_demo())
    return false;

  const i32 ticks = math::time_to_ticks(1.f);
  return ticks < 30;
}

bool utils::should_filter_server_ip(i8* ip, s_game_server_item* item, bool server_browser){
  if(ip == nullptr || item == nullptr){
    DBG("[-] received nullptr ip/item\n");
    return false;
  }

  if(item->max_players > 128)
    return true;

  const u32 ip_hash = HASH_RT(ip);
  switch(ip_hash){
    default: break;
    case HASH("185.232.171.145"):
    case HASH("23.140.40.110"):
    case HASH("91.217.76.232"):
    case HASH("193.162.143.150"):
    case HASH("185.23.236.207"):
    case HASH("51.68.152.142"):
    case HASH("51.68.143.140"):
      return true;
  }

  i8 name[1024];
  wsprintfA(name, XOR("%s"), item->server_name);
  str_utils::tolower(name);

  i8 game_desc[1024];
  wsprintfA(game_desc, XOR("%s"), item->game_desc);
  str_utils::tolower(game_desc);

  if(str_utils::contains(game_desc, XOR("█")))
    return true;

  // Server browser abuse.
  if(str_utils::contains(name, XOR("sgaming.ru")) ||
    str_utils::contains(name, XOR("mЯcopУБka"))   ||
    str_utils::contains(name, XOR("ПepeДoboЙ"))   ||
    str_utils::contains(name, XOR("ПepectpeЛka")) ||
    str_utils::contains(name, XOR("БoЙЦobckИЙ"))  ||
    str_utils::contains(name, XOR("beceЛЫЙ"))     ||
    str_utils::contains(name, XOR("zombie paradise [hunting/escape]")) ||
    str_utils::contains(name, XOR("perfect fox")) ||
    str_utils::contains(name, XOR("[dd2] = 100-tick")) ||
    str_utils::contains(name, XOR("[alcoholics]"))     ||
    str_utils::contains(name, XOR("ПОКОЛЕНИЕ 90-Х"))   ||
    str_utils::contains(name, XOR("fabrika [deathmatch]")) ||
    str_utils::contains(name, XOR("bc18go.ru")) ||
    str_utils::contains(name, XOR("defusedome")) ||
    str_utils::contains(name, XOR("cs-ds.ru")) ||
    str_utils::contains(name, XOR("4played.de")) ||
    str_utils::contains(name, XOR("c2play.de")) ||
    str_utils::contains(name, XOR("saigns.de")) ||
    str_utils::contains(name, XOR("valhalla | public")) ||
    str_utils::contains(name, XOR("go-meat.ru")) ||
    str_utils::contains(name, XOR("d90x.ru")) ||
    str_utils::contains(name, XOR("g a m e s h o t")) ||
    str_utils::contains(name, XOR("avg.rus")) ||
    str_utils::contains(name, XOR("soclan")) ||
    str_utils::contains(name, XOR("90-x 18+")) ||
    str_utils::contains(name, XOR("| free vip |")) ||
    str_utils::contains(name, XOR("@ no pro")) ||
    str_utils::contains(name, XOR("$2000$")) ||
    str_utils::contains(name, XOR("★")) ||
    str_utils::contains(name, XOR("█")) ||
    str_utils::contains(name, XOR("sex with bhop")) ||
    str_utils::contains(name, XOR("alkahi90.myarena.site")) ||
    str_utils::contains(name, XOR("game4ex.ru")) ||
    str_utils::contains(name, XOR("free vip")) ||
    str_utils::contains(name, XOR("csborn.ru")) ||
    str_utils::contains(name, XOR("ЧЕСТНЫЙ СЕРВЕР")) ||
    str_utils::contains(name, XOR(":cyberteam:")) ||
    str_utils::contains(name, XOR("Медвежий Фарш")) ||
    str_utils::contains(name, XOR("[cs:go] nirvana")) ||
    str_utils::contains(name, XOR("Покажи на что способен")) ||
    str_utils::contains(name, XOR("promotion.ru")) ||
    str_utils::contains(name, XOR("[18+]")) ||
    str_utils::contains(name, XOR("[15+]")) ||
    str_utils::contains(name, XOR("L E G E N D A DUST2")) ||
    str_utils::contains(name, XOR("alpha meat paradise")) ||
    str_utils::contains(name, XOR("МУЖСКОЙ ХАРАКТЕР")) ||
    str_utils::contains(name, XOR("international [public] [without mat]")) ||
    str_utils::contains(name, XOR("atmmix.ru")))
    return true;

  return false;
}

// Returns the smallest FOV from certain points on the entity from center of screen.
float utils::get_ideal_pixel_fov(c_internal_base_entity* entity){
  if(entity == nullptr)
    return 0.f;

  vec3  best_pos;
  float best_fov = -1.f;
  for(float mod = 0.f; mod <= 1.f; mod += 0.25f){
    const vec3 pos  = entity->obb_custom(mod);
    const float fov = math::get_pixel_fov_delta(pos);
    if(best_fov != -1.f && fov > best_fov)
      continue;

    best_pos = pos;
    best_fov = fov;
  }

#if defined(DEV_MODE)
  //render_debug->draw_3dbox(best_pos, colour(255, 0, 0, 32), global_internal->global_data->interval_per_tick * 2.f);
#endif
  return best_fov;
}

i8* utils::get_steam_name_from_sid(u32 sid){
  DBG("[!] utils::get_steam_name_from_sid: %i\n", sid);
  assert(global_internal->steamcontext != nullptr);
  assert(global_internal->steamcontext->steamfriends != nullptr);
  return global_internal->steamcontext->steamfriends->get_persona_name_other(utils::get_user_steamid(sid));
}

bool utils::is_steam_friend(u32 sid){
  DBG("[!] utils::is_steam_friend: %i\n", sid);
  assert(global_internal->steamcontext != nullptr);
  assert(global_internal->steamcontext->steamfriends != nullptr);
  return global_internal->steamcontext->steamfriends->has_friend(utils::get_user_steamid(sid));
}

wchar_t* utils::get_team_relation(i32 team){
  static wchar_t buffer[13];
  if(team <= 0){
    wcscpy(buffer, WXOR(L"[Unassigned]"));
    return buffer;
  }

  wcscpy(buffer, (global_internal->localplayer_team == team) ? WXOR(L"[Team]") : WXOR(L"[Other]"));
  return buffer;
}