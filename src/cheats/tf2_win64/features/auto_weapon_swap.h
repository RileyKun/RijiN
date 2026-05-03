#pragma once

class c_auto_weapon_swap{
public:
  ALWAYSINLINE void reset(){
    memset(this, 0, sizeof(c_auto_weapon_swap));
    switch_weapon_slot = -1;
    return_weapon_slot = -1;
  }

  void think(){
    static i32 currrent_tick = 0;
    if(currrent_tick == globaldata->tick_count)
      return;

    // for dev mode when unloading / loading the cheat.
    if(math::abs(currrent_tick - globaldata->tick_count) > 5){
      DBG("[!] auto_weapon_swap fixing slots and timers..\n");
      reset();
    }

    currrent_tick = globaldata->tick_count;
    if(switch_weapon_slot != -1){
      if(utils::switch_to_weapon_from_slot(switch_weapon_slot)){
        DBG("[!] Switched weapon to slot %i\n", switch_weapon_slot);
        if(switch_weapon_apply_ticks-- <= 0)
          switch_weapon_slot = -1;
      }
      else{
        DBG("[!] Switching to weapon slot %i\n", switch_weapon_slot);
      }
    }
    else if(return_weapon_slot != -1){
      if(has_to_fire){
        if(has_fired || has_to_fire_timeout <= math::time()){
          if(utils::switch_to_weapon_from_slot(return_weapon_slot)){
            DBG("[!] Switched back to original slot %i\n", return_weapon_slot);
            if(return_weapon_apply_ticks-- <= 0)
              return_weapon_slot = -1;
          }
          else{
            DBG("[!] Switching back to original slot %i\n", return_weapon_slot);
          }
        }
      }
      else{
        if(utils::switch_to_weapon_from_slot(return_weapon_slot)){
          DBG("[!] Switched back to original slot %i\n", return_weapon_slot);
          if(return_weapon_apply_ticks-- <= 0)
            return_weapon_slot = -1;
        }
        else{
          DBG("[!] Switching back to original slot %i\n", return_weapon_slot);
        }
      }
    }
  }

  void on_weapon_fired(c_base_weapon* wep = nullptr){
    if(!return_weapon_slot || !has_to_fire || !in_progress)
      return;

    if(wep != nullptr){
      if(wep->get_slot() != wanted_weapon_slot){
        DBG("Fired: Not the right weapon %i - %i (%ls)\n", wep->get_slot(), switch_weapon_slot, wep->get_weapon_name().c_str());
        return;
      }
    }

    has_fired = true;
    DBG("[!] Weapon has fired\n");
  }

  bool swap_and_return_to(i32 slot, bool must_fire, float timeout = 5.f){
    if(slot < 0 || slot > 10)
      return false;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return false;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return false;

    // Always force this on.
    timeout += global->time_between_shots;

    i32 current_slot = wep->get_slot();
    if(current_slot == slot) // Already on this weapon..
      return false;

    switch_weapon_apply_ticks = 1;
    return_weapon_apply_ticks = math::time_to_ticks(utils::get_latency());

    wanted_weapon_slot  = slot;
    return_weapon_slot  = current_slot;
    switch_weapon_slot  = slot;
    has_to_fire         = must_fire;
    has_to_fire_timeout = math::time() + timeout;
    in_progress         = true;
    has_fired           = false;
    return true;
  }

  bool extend_timeout(float extend_amount){
    if(has_fired || !return_weapon_slot || !has_to_fire || !in_progress)
      return false; 

    extend_amount       = math::clamp(extend_amount, 0.0f, 5.f);
    has_to_fire_timeout += extend_amount;
    return true;
  }

  ALWAYSINLINE void force_return(){
    has_to_fire_timeout = -1.f;
    has_to_fire         = true;
  }

  i32   switch_weapon_slot        = -1;
  i32   return_weapon_slot        = -1;
  i32   switch_weapon_apply_ticks = 0;
  i32   return_weapon_apply_ticks = 0;
  i32   wanted_weapon_slot        = -1;
  bool  has_to_fire               = false;
  bool  has_fired                 = false;
  bool  in_progress               = false;
  float has_to_fire_timeout       = 0.f;
};

CLASS_EXTERN(c_auto_weapon_swap, auto_weapon_swap);