#include "../../../link.h"

CFLAG_O0 bool c_base_privacy_mode::engine_get_player_info_hook(i32 entity_id, s_player_info* info, bool sid_write_allowed){
  if(entity_id == 0 || info == nullptr)
    return false;

  if(info->fake_player || info->friends_id == 0)
    return false;

  c_misc_privacy_mode_settings* settings = get_settings();
  if(settings == nullptr)
    return false;

  if(!settings->enabled)
    return false;

  c_entity_info* cache = global_internal->base_cheat_detection->get_entity_info(entity_id);
  if(cache == nullptr)
    return false;

  bool local_player = global_internal->localplayer_index == entity_id;
  bool is_friend    = global_internal->base_cheat_detection->is_friend(entity_id);
  bool is_public    = !local_player && !is_friend;
  bool is_cheater   = global_internal->base_cheat_detection->is_cheating(entity_id);
  bool is_bot       = global_internal->base_cheat_detection->is_cheating_bot(entity_id);
  bool is_team      = false;

  // Some data is not ready for us until they are spawned in.
  c_internal_base_player* player = global_internal->entity_list->get_entity(entity_id);
  if(player != nullptr)
    is_team    = player->team() == global_internal->localplayer_team;
  

  i8 buf[1024] = {0};
  if(local_player && !settings->censor_local_player || is_friend && !settings->censor_friends || is_public && !settings->censor_public)
    return false;

  // Some parts of the game can be killed if not added in properly.
  if(sid_write_allowed){
    if(info->friends_id)
      info->friends_id = (info->friends_id + entity_id) + (info->friends_id % 128) + global_internal->localplayer_index;
  }

  if(local_player)
    I(wsprintfA)(buf, XOR("You"));
  else if(is_friend){
    if(settings->friends_use_nick_names && cache->info->steam_name[0] != 0)
      I(wsprintfA)(buf, XOR("%s"), cache->info->steam_name);
    else
      I(wsprintfA)(buf, XOR("Friend"));
  }
  else if(is_public)
    I(wsprintfA)(buf, XOR("%s%s"), is_team ? "" : XOR("Enemy "), is_cheater ? XOR("Cheater") : XOR("Player"));
    
  strcpy(info->name, buf);

  return true;
}

bool c_base_privacy_mode::paint_traverse_hook(uptr panel){
  cookie_block_check_return_val(false);

  if(!panel)
    return false;

  c_misc_privacy_mode_settings* settings = get_settings();
  if(settings == nullptr)
    return false;

  if(!settings->enabled || !settings->hide_revealing_ui_panels)
    return false;

  i8* name = global_internal->panel->get_name(panel);
  if(name == nullptr)
    return false;

  // This does have game only panels, but it shouldn't really matter at all.
  const u32 hash = HASH_RT(name);
  switch(hash){
    default: break;
    case HASH("avatar"):
    case HASH("SteamFriendsList"):
    case HASH("RankPanel"):
    case HASH("TooltipPanel"):
    case HASH("ServerLabelNew"):
    case HASH("ServerLabel"):
    case HASH("mapname"):
    case HASH("itempanel"):
    case HASH("DifficultyContainer"):
    case HASH("PopFileLabel"):
    case HASH("MvMPlayerList"):
    case HASH("RedLeaderAvatar"):
    case HASH("BLueLeaderAvatar"):
    case HASH("PlayerAvatar"):
    case HASH("Player1Avatar"):
    case HASH("Player2Avatar"):
    case HASH("Player3Avatar"):
    case HASH("Player4Avatar"):
    case HASH("KillStreakPlayer1Avatar"):
    case HASH("KillStreakPlayer2Avatar"):
    case HASH("KillStreakPlayer3Avatar"):
    case HASH("KillStreakPlayer4Avatar"):
      return true; 
  }

  return false;
}

// Should be the last call in fire_even_hook
bool c_base_privacy_mode::handle_game_event(c_game_event* event){
  if(event == nullptr)
    return false;

  if(!is_enabled())
    return false;

  u32 hash = HASH_RT(event->get_name());
  
  switch(hash){
    default: break;
    case HASH("party_chat"):
      return true;
    case HASH("player_team"):
    case HASH("player_connect_client"):
    case HASH("player_connect"):
    case HASH("player_disconnect"):
    {
      event->set_string(XOR("name"), XOR("STREAMER-MODE"));
      if(hash == HASH("player_disconnect"))
        event->set_string(XOR("reason"), XOR("STREAMER-MODE"));

      return false;
    }
  }

  return false;
}

// Should be the last call in dispatch_user_message_hook
bool c_base_privacy_mode::dispatch_user_message(u32 name_hash, i32 msg_type, s_bf_read* bf_read){
  if(bf_read == nullptr)
    return false;

  if(!is_enabled())
    return false;

  switch(name_hash){
    default: break;
    case HASH("VoteStart"):
    {
      bf_read->read();       // team vote index (number of the team)
      bf_read->read<i32>();  // vote type
      bf_read->read();       // entity_holding_vote
      bf_read->read_str();   
      i8* player_name       = bf_read->read_str(false);
      assert(player_name != nullptr);

      // Reset data.
      bf_read->reset();
      {
        bf_read->read();
        bf_read->read<i32>();
        bf_read->read();
        bf_read->read_str();
      }

      DBG("[!] c_base_privacy_mode::dispatch_user_message censoring vote caller name \"%s\"\n", player_name);
      for(u32 i = 0; i < strlen(player_name); i++)
        *(i8*)(bf_read->data + i) = '*'; 

      bf_read->reset();

      return true;
    }
  }

  return false;
}

bool c_base_privacy_mode::should_censor_chat(i32 index){
  if(!is_enabled())
    return false;

  c_misc_privacy_mode_settings* settings = get_settings();
  if(settings == nullptr)
    return false;

  // If not spawned in then censor name.
  if(global_internal->entity_list->get_entity(index) == nullptr)
    return true;

  bool is_localplayer = global_internal->localplayer_index == index;
  bool is_friend      = global_internal->base_cheat_detection->is_friend(index) && !is_localplayer;

  if(is_localplayer && settings->censor_local_player)
    return true;

  if(is_friend && settings->censor_friends)
    return true;

  if(!is_localplayer && !is_friend && settings->censor_public)
    return true;

  return false;
}