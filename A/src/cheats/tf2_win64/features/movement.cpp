#include "../link.h"

c_movement* movement = nullptr;


bool c_movement::can_perform_double_jump(c_internal_base_player* localplayer){
  
  c_base_player* tf_player = (c_base_player*)localplayer;

  bool using_base_jumper = false;
  for(i32 i = 0; i <= 1; i++){
    c_base_weapon* base_jumper = localplayer->get_weapon_from_belt(i, false);
    if(base_jumper == nullptr)
      continue;

    if(!base_jumper->is_base_jumper())
      continue;

    using_base_jumper = true;
    break;
  }
  
  return tf_player->is_player_class(TF_CLASS_SCOUT) || tf_player->is_saxton_hale() || using_base_jumper;
}