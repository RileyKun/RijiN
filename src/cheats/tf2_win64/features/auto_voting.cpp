#include "../link.h"

CLASS_ALLOC(c_auto_voting, auto_vote);

void c_auto_voting::run_auto_cast(){
  if(!config->misc.auto_vote_enabled || config->misc.auto_vote_autocast_disabled)
    return;

  global->info_panel->add_entry(INFO_PANEL_AUTO_VOTE, WXOR(L"INACTIVE"));
  float current_time = math::time(false);

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  // No servers allow us to kick from spectator.
  if(localplayer->team() == 0)
    return;

  i32 priority = 0;
  s_player_info kick_victim;
  kick_victim.user_id = -1;
  for(i32 i = 1; i <= globaldata->max_clients; i++){
    c_base_player* player = global->entity_list->get_entity( i );
    if(player == nullptr)
      continue;

    // We can actually votekick ourself.
    if(i == global->engine->get_local_player())
      continue;

    if(player->team() != localplayer->team())
      continue;

    if(player->is_steam_friend())
      continue;

    c_player_data* data = player->data();
    if(data == nullptr)
      continue;

    if(!data->friends_id)
      continue;

    if(!config->misc.auto_vote_autocast_defensive || config->misc.auto_vote_autocast_defensive && data->attempted_votekicks <= 0){
      if(!cheat_detection->is_cheating_bot(i) && config->misc.auto_vote_autocast_botsonly)
        continue;

      if(!cheat_detection->is_cheating(i) && config->misc.auto_vote_autocast_cheatersonly)
        continue;
    }

    u32 p = 0;
    if(cheat_detection->is_cheating(i))
      p++;

    if(cheat_detection->is_cheating_bot(i))
      p++;

    p += data->attempted_votekicks;
    if(p >= priority){
      kick_victim = player->info();
      priority = p;
    }
  }

  if(kick_victim.user_id == -1)
    return;

  if(global->next_vote_kick_time > current_time){
    global->info_panel->add_entry(INFO_PANEL_AUTO_VOTE, WXOR(L"COOLDOWN"), INFO_PANEL_WARNING_CLR);
    return;
  }

  if(global->cant_start_vote_kick){
    global->info_panel->add_entry(INFO_PANEL_AUTO_VOTE, WXOR(L"CANT CALL VOTE"), INFO_PANEL_WARNING_CLR);
    return;
  }

  if(kick_victim.user_id != -1){
    global->info_panel->add_entry(INFO_PANEL_AUTO_VOTE,  WXOR(L"CALLING VOTE"), INFO_PANEL_RAGE_CLR);

    DBG("[!] Kicking %i\n", kick_victim.user_id);
    i8 buf[IDEAL_MIN_BUF_SIZE];
    formatA(buf, XOR("callvote kick \"%i cheating\""), kick_victim.user_id);
    global->engine->client_cmd(buf);
    global->next_vote_kick_time = current_time + 1.0f;
  }
  else
    global->next_vote_kick_time = current_time + 0.1f;
}

i32 c_auto_voting::resolve_votekick_victim(wchar_t* victim_name){
  if(victim_name == nullptr)
    return 0;

  s_player_info info;
  for(i32 i = 1; i <= globaldata->max_clients; i++){
    global->skip_get_player_info_hook = true;
    if(!global->engine->get_player_info(i, &info)){
      global->skip_get_player_info_hook = false;
      continue;
    }
    global->skip_get_player_info_hook = false;

    static wchar_t buff[128];
    convert::str2wstr(info.name, buff, sizeof(buff));

    if(str_utils::equal(victim_name, buff))
      return i;
  }

  return 0;
}

void c_auto_voting::handle_user_message(u8 caster_index, u8 team, u32 vote_index, wchar_t* victim_name){
  if(victim_name == nullptr)
    return;

  i32 victim = resolve_votekick_victim(victim_name);
  if(victim == global->localplayer_index)
    localplayer_being_kicked = true;

  if(!config->misc.auto_vote_enabled)
    return;

  float current_time = math::time(false);
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  // We called this vote man.
  if(caster_index == global->localplayer_index){
    global->next_vote_kick_time = current_time + 10.f;
    return;
  }

  // This vote doesn't matter to us!
  if(team == 0 || team != localplayer->team())
    return;

  i8 yes_buff[64];
  i8 no_buff[64];

  formatA(yes_buff, XOR("vote %i option1"), vote_index);
  formatA(no_buff, XOR("vote %i option2"), vote_index);

  c_base_player* player = global->entity_list->get_entity(caster_index);
  if(player == nullptr){
    DBG("[-] Couldn't votekick caster! Voting no if option is enabled!\n");
    if(config->misc.auto_vote_castno)
      global->engine->client_cmd(no_buff);

    return;
  }

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  // If we know who's getting votekicked lets prefer them over the caster.
  if(victim != 0){
    c_base_player* victim_player = global->entity_list->get_entity(victim);
    if(victim_player == nullptr)
      return;

    DBG("[!] Prefering votekick victim for auto-vote\n");
    if(victim_player->is_steam_friend() || victim == global->engine->get_local_player()){
      if(config->misc.auto_vote_castno){
        global->engine->client_cmd(no_buff);
        DBG("[!] Auto F2\n");
      }

      data->attempted_votekicks++;
    }
    else{
      if(config->misc.auto_vote_castyes){
        global->engine->client_cmd(yes_buff);
        DBG("[!] Auto F1\n");
      }
    }
  }
  else{

    DBG("[!] Prefering votekick caster for auto-vote\n");
    if(player->is_steam_friend()){
      if(config->misc.auto_vote_castyes){
        global->engine->client_cmd(yes_buff);
        DBG("[!] Auto F1\n");
      }
    }
    else{
      if(config->misc.auto_vote_castno){
        global->engine->client_cmd(no_buff);
        DBG("[!] Auto F2\n");
      }
    }

  }
}

void c_auto_voting::dispatch_user_message_run(u32 name_hash, i32 msg_type, s_bf_read* bf_read){
  if(bf_read == nullptr){
    assert(bf_read != nullptr);
    return;
  }

  if(name_hash == HASH("VoteStart")){
    u8  team_to_vote         = bf_read->read();
    i32 vote_index       = bf_read->read<i32>();
    u8  entity_holding_vote  = bf_read->read();
    bf_read->read_str();
    i8* player_name          = bf_read->read_str(false);
    bf_read->reset();
    assert(player_name != nullptr);

    static wchar_t buff[128];
    convert::str2wstr(player_name, buff, sizeof(buff));

    handle_user_message(entity_holding_vote, team_to_vote, vote_index, buff);
    global->cant_start_vote_kick = true;
  }
  else if(name_hash == HASH("VoteFailed") || name_hash == HASH("VotePass")){
    global->cant_start_vote_kick = false;
    localplayer_being_kicked     = false;
  }
  else if(name_hash == HASH("CallVoteFailed")){
    i32   reason = bf_read->read();
    float time   = (float)bf_read->read();
    global->next_vote_kick_time = math::time() + math::abs(time);
    bf_read->reset();
  }
}