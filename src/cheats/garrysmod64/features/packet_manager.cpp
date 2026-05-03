#include "../link.h"

i32 c_packet_manager::get_max_choke_ticks(){
  // when double tap is ready, dont let the engine choke more than 2+1 commands
  // if we do the server will jolt our tickbase forwards and waste all of our charge ticks

  // clearly the clc_move thingy isn't working properly atm.
  // - aug 20th 2024.
  return 14;//22;//return double_tap->has_any_charged_ticks() ? 3 : 22;
}

// Put checks in here to prevent packet choking under certain conditions...
bool c_packet_manager::allow_choking(bool predicted = false){
  c_base_player* localplayer = utils::localplayer();

  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  //if(!fake_latency->is_synced())
  //  return false;

  return (global->client_state->choked_commands + (predicted ? 1 : 0)) < get_max_choke_ticks();
}

CLASS_ALLOC(c_packet_manager, packet_manager);