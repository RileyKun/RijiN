#include "../link.h"

c_cheat_detection* cheat_detection = nullptr;

bool c_cheat_detection::is_cheating(i32 index, bool exclude_playerlist){
  bool result = c_base_cheat_detection::is_cheating(index, exclude_playerlist);
  if(exclude_playerlist)
    return result;

  c_player_data* data = utils::get_player_data(index);
  if(data == nullptr)
    return result;

  if(data->playerlist.mark_as_cheater || data->playerlist.mark_as_bot)
      return true;

  if(data->playerlist.mark_as_legit || data->playerlist.mark_as_suspicious)
    return false;
  

  return result;
}

bool c_cheat_detection::is_cheating_bot(i32 index, bool exclude_playerlist){
  bool result = c_base_cheat_detection::is_cheating_bot(index, exclude_playerlist);
  if(exclude_playerlist)
    return result;

  c_player_data* data = utils::get_player_data(index);
  if(data == nullptr)
    return result;

  if(data->playerlist.mark_as_bot)
    return true;

  if(data->playerlist.mark_as_legit || data->playerlist.mark_as_suspicious)
    return false;
  

  return result;
}

bool c_cheat_detection::is_friend(i32 index, bool exclude_playerlist){
  bool result = c_base_cheat_detection::is_friend(index, exclude_playerlist);
  if(exclude_playerlist)
    return result;

  c_player_data* data = utils::get_player_data(index);
  if(data == nullptr)
    return result;

  utils::check_for_party_member(index);
  if(data->playerlist.mark_as_friend || data->party_member)
    return true;

  return result;
}

bool c_cheat_detection::is_weapon_allowed_for_angles(c_internal_base_weapon* weapon){
  c_base_weapon* wep = (c_base_weapon*)weapon;
  if(wep == nullptr)
    return false;

  return wep->is_hitscan_weapon();
}

bool c_cheat_detection::is_weapon_allowed_for_repeat(c_internal_base_weapon* weapon){
  c_base_weapon* wep = (c_base_weapon*)weapon;
  if(wep == nullptr)
    return false;

  if(wep->is_harmless_item() || wep->is_flamethrower() || wep->is_medigun() || wep->is_wrangler())
    return false;

  return true;
}