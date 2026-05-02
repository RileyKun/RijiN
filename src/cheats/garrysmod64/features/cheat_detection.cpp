#include "../link.h"

CLASS_ALLOC(c_cheat_detection, cheat_detection);

void c_cheat_detection::transmit_steamid(){
  float time = math::time();

  static float next_update = 0.f;
  if(next_update > time)
    return;

  next_update = time + 60.f;
  if(!global->current_steamid){
    if(global->steamcontext->steamuser == nullptr)
      return;

    c_steamid id;
    global->steamcontext->steamuser->get_steam_id(&id);
    global->current_steamid = id.m_steamid.comp.account_id;

    if(!global->current_steamid){
      DBG("[-] c_cheat_detection::transmit_steamid - failed to get current steamid.\n");
      return;
    }
  }

  static u32* data = malloc(sizeof(u32));
  if(data == nullptr)
    return;

  utils::create_worker([](void*){
    transmit_identifier(gen_internal->cloud_license_key, global->current_steamid);
    return 0;
  }, nullptr);
}

void c_cheat_detection::think(u32 stage){
  if(stage != FRAME_NET_UPDATE_END)
    return;

  for(u32 i = 1; i <= globaldata->max_clients; i++){
    if(i == global->engine->get_local_player())
      continue;

    c_base_player* player = global->entity_list->get_entity(i);
    if(player == nullptr)
      continue;

    run(player);
  }
}

void c_cheat_detection::run(c_base_player* player){
  if(player == nullptr)
    return;

  float time = math::time();
  c_player_data* data = player->data();
  if(data->friends_id == 0)
    return;

  if(should_run_on_player(player)){
    check_for_known_cheater(player);
  }
}

void c_cheat_detection::fire_bullets(c_base_entity* entity, s_fire_bullets_info* info, bool post){
  #if defined(DEV_MODE)
  if(entity == nullptr || info == nullptr)
    return;

  if(!entity->is_player())
    return;
  
  if(accuracy->is_dumping_spread && entity == utils::localplayer())
    return;

  c_base_player* player = (c_base_player*)entity;

  if(post){  
    s_trace spread_tr = global->trace->ray(info->src, info->src + info->dir * info->distance, mask_shot);

    vec3 angles = player->viewangles();
    vec3 fwd;
    math::angle_2_vector(angles, &fwd, nullptr, nullptr);
    s_trace normal_tr = global->trace->ray(info->src, info->src + fwd * info->distance, mask_shot);

    float dist = (spread_tr.end - normal_tr.end).length();
  }
  #endif
}

bool c_cheat_detection::should_run_on_player(c_base_player* player, bool ignore_marked_cheaters){
  if(player == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(localplayer == player)
    return false;

  c_player_data* data = player->data();
  if(data == nullptr)
    return false;

  if(data->friends_id == 0 || player->info().friends_id == 0)
    return false;

  if(player->info().fake_player)
    return false;

  return true;
}

void c_cheat_detection::check_for_known_cheater(c_base_player* player){
  if(player == nullptr)
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  if(!should_run_on_player(player))
    return;

  s_player_info info;
  if(!global->engine->get_player_info(player->get_index(), &info))
    return;

  // Since gmod has a common place of having load of people in it, we are going to rate-limit this because otherwise the person will literally ddos our server with alot of steamids.
  float        time              = math::time();
  static float rate_limit_global = 0.f;
  static u32   requests_sent     = 0;
  if(time > rate_limit_global){
    requests_sent     = 0;
    rate_limit_global = time + 1.f;
  }

  if(requests_sent >= 32){
    DBG("[+] c_cheat_detection::check_for_known_cheater rate-limiting requests for a second.\n");
    return;
  }

  if(data->checked_cheater_list)
    return;
 
  data->checked_cheater_list = true;

  s_known_cheater_data* kc_data = malloc(sizeof(s_known_cheater_data));
  kc_data->player_index         = player->get_index();
  kc_data->name_hash            = 0;
  kc_data->sid                  = info.friends_id;

  requests_sent++;
  if(!utils::create_worker([](s_known_cheater_data* kc_data){
    while(global->is_talking_to_tfcl)
      Sleep(XOR32(250));

    c_net_request_tf_cheater_sid32_response* response = nullptr;
    if(utils::get_cheater_data_from_network(kc_data->sid, kc_data->name_hash, response)){
      cheat_detection->on_known_cheater_data_received(kc_data->player_index, response);
      cheat_detection->setup_known_alias(kc_data->player_index, response);
   }

    free(response);
    free(kc_data);
    kc_data = nullptr;

    return 0;
  }, kc_data))
    free(kc_data);
}

void c_cheat_detection::on_known_cheater_data_received(i32 player_index, c_net_request_tf_cheater_sid32_response* response){
  if(response == nullptr)
    return;

  if(player_index <= 0)
    return;
  
  c_base_player* player = global->entity_list->get_entity(player_index);
  if(player == nullptr)
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  if(!should_run_on_player(player))
    return;

  DBG("[+] on_known_cheater_data_received: Processing %s\n", player->info().name);
  if(!is_response_cheater(response->cheater_flags)){
    on_known_person_received(player_index, response);
    process_steam_group_data(player_index, response);
    return;
  } 

  data->is_dev_esp = response->cheater_flags & CHEATER_FLAG_DEV_ESP;
  data->is_dev_ignored = response->cheater_flags & CHEATER_FLAG_DEV_IGNORE;

  wchar_t buf[2048];
  wchar_t player_name[1024];
  convert::str2wstr(player->info().name, player_name, sizeof(player_name));

  bool already_marked = false;

  bool should_send = should_notify(player, &already_marked);
  if(should_send || already_marked){

    std::wstring known_who = WXOR(L"known cheater");
    if(response->cheater_flags & CHEATER_FLAG_IS_KNOWN_BOT)
      known_who = WXOR(L"known bot");

    std::wstring alias = convert::str2wstr(response->alias);
    std::wstring group = convert::str2wstr(response->group);

    if(response->alias_len > 0){
      if(response->group_len > 0)
        I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is a %ls and is commonly referred to as \"%ls\" and is related to the group \"%ls\"."), player_name, known_who.c_str(), alias.c_str(), group.c_str());
      else
        I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is a %ls and is commonly referred to as \"%ls\"."), player_name, known_who.c_str(), alias.c_str());
    }
    else{
      if(response->group_len > 0)
        I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is a %ls and is related to the group \"%ls\"."), player_name, known_who.c_str(), group.c_str());
      else
        I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is a %ls."), player_name, known_who.c_str());
    }

    notify->create(NOTIFY_TYPE_WARNING, buf, colour(255, 0, 0, 255));
  }

  if(response->cheater_steam_group_count > 0 || response->bot_steam_group_count > 0){
    DBG("[!] \n\nCHEATER STEAM GROUP!\n\n\n");
  }

  on_cheater_detected(player, L"", DETECTION_REASON_INVALID, false, true);
}

void c_cheat_detection::setup_known_alias(i32 player_index, c_net_request_tf_cheater_sid32_response* response){
  if(response == nullptr)
    return;

  if(player_index <= 0)
    return;

  c_player_data* data = utils::get_player_data(player_index);
  if(data == nullptr)
    return;

  s_player_info info;
  if(!global->engine->get_player_info(player_index, &info))
    return;

  if(!response->alias_len){
    DBG("[!] setup_known_alias: \"%s\" doesn't have a known alias.\n", info.name);
    return;
  }

  data->has_alias = true;
  memset(data->alias, 0, sizeof(data->alias));
  strcpy(data->alias, response->alias);
  DBG("[+] Setting known alias for #%i (%s - %s)\n", player_index, data->alias, response->alias);
}

void c_cheat_detection::on_known_person_received(i32 player_index, c_net_request_tf_cheater_sid32_response* response){
  if(response == nullptr)
    return;

  if(player_index <= 0)
    return;

  c_base_player* player = global->entity_list->get_entity(player_index);
  if(player == nullptr)
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  if(!should_run_on_player(player))
    return;

  DBG("[+] on_known_person_received: Processing %s\n", player->info().name);
  if(!response->alias_len && !response->group_len){
    process_steam_group_data(player_index, response);
    return;
  }

  data->cheater_groups = response->cheater_steam_group_count;
  data->bot_groups     = response->bot_steam_group_count;

  wchar_t buf[1024];
  wchar_t player_name[1024];
  convert::str2wstr(player->info().name, player_name, sizeof(player_name));

  std::wstring alias = convert::str2wstr(response->alias);
  std::wstring group = convert::str2wstr(response->group);

  if(should_notify(player)){
    if(response->cheater_steam_group_count <= 0 && response->bot_steam_group_count <= 0){
      if(response->alias_len > 0){
        if(response->group_len > 0)
          I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is not a cheater but is commonly referred to as \"%ls\" and is related to the group \"%ls\"."), player_name, alias.c_str(), group.c_str());
        else
          I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is not a cheater but is commonly referred to as \"%ls\"."), player_name, alias.c_str());
      }
      else
        I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is not a cheater but is related to the group \"%ls\"."), player_name, group.c_str());
    }
    else{

      if(response->cheater_steam_group_count > 0){
        if(response->alias_len > 0){
          if(response->group_len > 0)
            I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is not a cheater but is commonly referred to as \"%ls\" and is related to the group \"%ls\" and is also in \"%i\" known cheater steam groups."), player_name, alias.c_str(), group.c_str(), response->cheater_steam_group_count);
          else
            I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is not a cheater but is commonly referred to as \"%ls\" and is also in \"%i\" known cheater steam groups."), player_name, alias.c_str(), response->cheater_steam_group_count);
        }
        else
          I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is not a cheater but is related to the group \"%ls\" and is also in \"%i\" known cheater steam groups."), player_name, group.c_str(), response->cheater_steam_group_count);
      }
      else if(response->bot_steam_group_count > 0){
        if(response->alias_len > 0){
          if(response->group_len > 0)
            I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is not a cheater but is commonly referred to as \"%ls\" and is related to the group \"%s\" and is also in %i bot steam groups."), player_name, alias.c_str(), group.c_str(), response->bot_steam_group_count);
          else
            I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is not a cheater but is commonly referred to as \"%ls\" and is also in %i bot steam groups."), player_name, alias.c_str(), response->bot_steam_group_count);
        }
        else
          I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is not a cheater but is related to the group \"%ls\" and is also in %i bot steam groups."), player_name, group.c_str(), response->bot_steam_group_count);
      }
      else{
        if(response->alias_len > 0){
          if(response->group_len > 0)
            I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is not a cheater but is commonly referred to as \"%ls\" and is related to the group \"%ls\" and is also in %i cheater and is in %i bot steam groups."), player_name, alias.c_str(), group.c_str(), response->cheater_steam_group_count, response->bot_steam_group_count);
          else
            I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is not a cheater but is commonly referred to as \"%ls\" and is also in %i cheater and is in %i bot steam groups."), player_name, alias.c_str(), response->cheater_steam_group_count, response->bot_steam_group_count);
        }
        else
          I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is not a cheater but is related to the group \"%ls\" and is also in %i cheater and is in %i bot steam groups."), player_name, group.c_str(), response->cheater_steam_group_count, response->bot_steam_group_count);
      }
    }

    notify->create(NOTIFY_TYPE_WARNING, buf, colour(255, 244, 164, 255));
  }
}

void c_cheat_detection::process_steam_group_data(i32 player_index, c_net_request_tf_cheater_sid32_response* response){
  if(response == nullptr)
    return;

  if(player_index <= 0)
    return;

  c_base_player* player = global->entity_list->get_entity(player_index);
  if(player == nullptr)
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  if(!should_run_on_player(player))
    return;

  if(!response->cheater_steam_group_count && !response->bot_steam_group_count)
    return;

  if(data->cheater_groups || data->bot_groups)
    return;

  data->cheater_groups = response->cheater_steam_group_count;
  data->bot_groups     = response->bot_steam_group_count;

  if(!should_notify(player))
    return;

  wchar_t buf[1024];
  wchar_t player_name[1024];
  convert::str2wstr(player->info().name, player_name, sizeof(player_name));

  if(response->cheater_steam_group_count > 0)
    I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is not a cheater but is in %i cheater steam group(s)."), player_name, response->cheater_steam_group_count);
  else if(response->bot_steam_group_count > 0)
    I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is not a cheater but is in %i bot steam group(s)."), player_name, response->bot_steam_group_count);
  else{
    I(wsprintfW)(buf, WXOR(L"The player \"%ls\" is not a cheater but is in %i cheater and %i bot steam groups."), player_name, response->cheater_steam_group_count, response->bot_steam_group_count);
  }

  notify->create(NOTIFY_TYPE_WARNING, buf, colour(255, 0, 0, 255));
} 

bool c_cheat_detection::is_cheating(c_base_player* player, bool exclude_playerlist){
  if(player == nullptr)
    return false;

  c_player_data* data = player->data();
  if(data == nullptr)
    return false;

  if(!exclude_playerlist){
    if(data->playerlist.mark_as_cheater)
      return true;

    //if(data->playerlist.mark_as_legit || data->playerlist.mark_as_suspicious)
    //  return false;
  }

  return data->is_cheater;
}

bool c_cheat_detection::on_playerlist_cfg_load(c_base_player* player){
  if(player == nullptr)
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  if(data->playerlist_notify)
    return;
  
  if(!data->friends_id || !data->checked_cheater_list)
    return;

  wchar_t player_name[128];
  wchar_t buf[1024];
  convert::str2wstr(player->info().name, player_name, sizeof(player_name));

  if(data->playerlist.mark_as_friend){
    I(wsprintfW)(buf, WXOR(L"You have the player \"%ls\" marked as a \"Friend\" in the playerlist."), player_name);
    notify->create(NOTIFY_TYPE_INFO, buf);
  }

  if(data->playerlist.ignore_player){
    I(wsprintfW)(buf, WXOR(L"You have the player \"%ls\" ignored on aimbot in the playerlist."), player_name);
    notify->create(NOTIFY_TYPE_INFO, buf);
  }

  if(!data->is_cheater){
    I(wsprintfW)(buf, WXOR(L"You have the player \"%ls\" marked as a \"Cheater\" in the playerlist."), player_name);
    notify->create(NOTIFY_TYPE_WARNING, buf);
  }

  std::string resolver_override = "";
  if(data->playerlist.resolver_disallow)
    resolver_override = XOR("Disallow");
  else if(data->playerlist.resolver_force)
    resolver_override = XOR("Force enabled");

  if(resolver_override.length() > 0){
    I(wsprintfW)(buf, WXOR(L"The player \"%ls\" has a resolver override set to \"%s\" in the playerlist."), player_name, resolver_override.c_str());
    notify->create(NOTIFY_TYPE_INFO, buf);
  }

  data->playerlist_notify = true;
}

bool c_cheat_detection::should_notify(c_base_player* player, bool* already_marked){
  if(player == nullptr)
    return false;

  if(!should_run_on_player(player))
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(localplayer->is_steam_friend(player))
    return false;

  if(cheat_detection->is_cheating(player, true)){
    if(already_marked != nullptr)
      *already_marked = true;

    return false;
  }

  return true;
}

void c_cheat_detection::angle_snapped(c_base_player* player, c_player_data* data, float trigger_delta, float angle_repeat_degree){
  if(player == nullptr)
    return;

  if(!player->is_valid())
    return;

  if(!should_run_on_player(player))
    return;

  if(data == nullptr)
    return;

  // Don't bother doing angle based detections on these servers.
  i32 tick_rate = math::time_to_ticks(1.f);
  if(tick_rate <= 16){
    DBG("[!] Tick rate of server is too low not checking angle snaps on player: %s\n", player->info().name);
    return;
  }

  s_lc_data* lc = lag_compensation->get_data(player);
  if(lc == nullptr)
    return;

  for(i32 i = 0; i < lc->max_records - 2; i++){
    s_lc_record* record1 = lc->get_record(i);
    s_lc_record* record2 = lc->get_record(i + 1);
    s_lc_record* record3 = lc->get_record(i + 2);

    if(record1 == nullptr || record2 == nullptr || record3 == nullptr)
      continue;

    float delta = math::abs(math::normalize_angle(record1->viewangles.y - record2->viewangles.y));
    if(delta >= trigger_delta && trigger_delta > 0.f)
      on_infraction(player);
    else if(delta >= angle_repeat_degree){
      if((i32)record1->viewangles.y == (i32)record3->viewangles.y && (i32)record1->viewangles.y != (i32)record2->viewangles.y){
        on_infraction(player);
      }
    }
  }
}

void c_cheat_detection::snap_check(c_base_player* player){
  if(player == nullptr)
    return;

  if(!player->is_valid())
    return;

  c_base_weapon* weapon = player->get_weapon();
  if(weapon == nullptr)
    return;

  if(weapon->is_harmless_weapon() || weapon->is_melee())
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  angle_snapped(player, data, 180.f, 2.5f);
}

void c_cheat_detection::on_death(i32 attacker, i32 victim){
  if(attacker == victim || attacker > globaldata->max_clients)
    return;

  c_base_player* player_attacker = global->entity_list->get_entity(attacker);
  if(player_attacker == nullptr)
    return;

  if(!player_attacker->is_player())
    return;

  if(!should_run_on_player(player_attacker))
    return;

  c_base_weapon* weapon = player_attacker->get_weapon();
  if(weapon == nullptr)
    return;

  if(weapon->is_harmless_weapon() || weapon->is_melee())
    return;

  c_player_data* data = player_attacker->data();
  if(data == nullptr){
    assert(data != nullptr)
    return;
  }

  angle_snapped(player_attacker, data, 45.f);
}

void c_cheat_detection::on_infraction(c_base_player* player){
  if(player == nullptr)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(localplayer->is_steam_friend(player))
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  float time = math::time(false);
  data->infractions++;
  if(data->infractions >= CHEAT_DETECTION_INFRACTION_MAX){
    on_cheater_detected(player, WXOR(L"(Heuristics)"), DETECTION_TOO_MANY_INFRACTIONS, true);
  }
  else if(data->infractions >= CHEAT_DETECTION_INFRACTION_SUSPECT){
    if(data->suspicious_timer < time){
      if(should_notify(player)){
        wchar_t buf[1024];
        wchar_t player_name[1024];

        convert::str2wstr(player->info().name, player_name, sizeof(player_name));
        wsprintfW(buf, WXOR(L"The player \"%ls\" is suspicious in regards to cheating."), player_name);
        notify->create(NOTIFY_TYPE_WARNING, buf);
      }
    }
  }
  data->suspicious_timer = time + CHEAT_DETECTION_SUSPECT_TIME;
}

void c_cheat_detection::on_cheater_detected(c_base_player* player, std::wstring reason, u32 reason_code, bool submit_cheater_to_network, bool disable_notification){
  if(player == nullptr)
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  if(data->friends_id == 0)
    return;

  if(player->get_index() == global->engine->get_local_player())
    return;

  if(player->info().fake_player)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_steam_friend(player) && !is_cheating(player, true)) {
    if(should_notify(player) && !disable_notification){
      wchar_t buf[1024];
      wchar_t player_name[1024];
      convert::str2wstr(player->info().name, player_name, sizeof(player_name));

      if(reason.length() != 0)
        wsprintfW(buf, WXOR(L"Detected player \"%ls\" as a cheater. [Reason: %ls]"), player_name, reason.c_str());
      else
        wsprintfW(buf, WXOR(L"Detected player \"%ls\" as a cheater."), player_name);

      notify->create(NOTIFY_TYPE_WARNING, buf);
    }
    data->is_cheater = true;
  }

  // submit this id to the cheater network
  if(submit_cheater_to_network && !data->has_submitted_cheater_to_network && reason_code != DETECTION_REASON_INVALID){
    utils::submit_cheater_to_network(player->info().friends_id, reason_code);
    data->has_submitted_cheater_to_network = true;
  }
}