#pragma once

class c_packet_manager{
public:
  bool force_next_send_packet   = false;
  bool should_force_send_packet = false;
  bool is_locked                = false;

  bool set_choke(bool choke){
    if(global->send_packet == nullptr)
      return false;

    if(is_locked)
      return *global->send_packet;

    return (*global->send_packet = !(!should_force_send_packet && allow_choking(choke) && choke));
  }

  // This function allows us to make it so that createmove cannot choke or unchoke any packets, this is desigined to be used in conjunction with doubletap
  void lock(bool locked){
    is_locked = locked;
  }

  // This allows us to trick the cheat into always ensuring the next command is sent to the server
  // This could be called for example in the case where you choke a packet but you want to ensure it is sent as soon as possible
  // Calling this function will make our cheat think that we always want to send the next tick, we cannot override it and everything will respect it
  void force_send_packet(bool next_tick){
    if(is_locked)
      return;

    if(next_tick)
      force_next_send_packet = true;
    else{
      should_force_send_packet = true;
      set_choke(false);
    }
  }

  void pre_run(){
    // senator; This should be the best way of detecting ping spikes that could cause command rejection
    // All we are doing here is taking our ping delta, checking if its increased and if its over 5ms
    // Then just send the next packet
   //i32 ping            = (i32)(global->engine->get_avg_latency(true) * 1000.f);
   //i32 ping_delta      = math::abs(ping - global->last_ping);

   //global->network_lag = (ping_delta > 0 && ping > global->last_ping) && ping_delta >= 5;
   //global->last_ping   = ping;

    // If we force the next packet to be sent
    // Force the packet to be sent this tick instead of last instead
    if(force_next_send_packet){
      should_force_send_packet  = true;
      force_next_send_packet    = false;
    }
  }

  void post_run(){
    if(!should_force_send_packet)
      return;

    set_choke(false);
    should_force_send_packet = false;
  }

  i32 get_max_choke_ticks();
  bool allow_choking(bool predicted = false);

  CFLAG_O0 bool choking(){
    if(global->send_packet == nullptr)
      return false;

    return !should_force_send_packet && *global->send_packet == 0;
  }
};

CLASS_EXTERN(c_packet_manager, packet_manager);