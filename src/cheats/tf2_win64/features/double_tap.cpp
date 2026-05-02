#include "../link.h"

CLASS_ALLOC(c_double_tap, double_tap);

#define CALL_CL_MOVE(extra_samples, final_tick) \
  packet_manager->compute_send_packet_val(final_tick); \
  packet_manager->set_breakpoint(GetCurrentThread(), EH_CL_MOVE_SP_LOCATION_ZEROING); \
  packet_manager->set_breakpoint(GetCurrentThread(), EH_CL_MOVE_SP_LOCATION_SET_CHOKE); \
  utils::call_fastcall64_raw<void, float, bool>(gen_internal->decrypt_asset(global->cl_move_hook_trp), extra_samples, final_tick);

EXPORT void __fastcall cl_move_hook(float extra_samples, bool final_tick){
  if(double_tap->cl_move_run(extra_samples, final_tick) || double_tap->cl_move_run_warp(extra_samples, final_tick))
    return;

  CALL_CL_MOVE(extra_samples, final_tick);

  if(final_tick){
    c_base_player* localplayer = utils::localplayer();

    if(localplayer != nullptr)
      global->last_sent_tickbase = localplayer->tick_base();
  }
}

u32 c_double_tap::get_wanted_charge_ticks(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return 22;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return 22;

  if(!weapon->is_able_to_dt() || weapon->is_minigun() || weapon->is_sniper_rifle() || weapon->is_sniper_smg() || weapon->is_melee() || weapon->is_projectile_weapon() || weapon->is_flamethrower() || weapon->is_short_circuit())
    return 22;

  if(config->misc.double_tap_always_charge_max_ticks)
    return 22;

  // never calculate this value here, fetch it after prediction has been completed, store it and use it
  return math::clamp(math::time_to_ticks(global->time_between_shots), 1, 22);
}

/* AdjustPlayerTimeBase attemps to roll your tickbase backwards so that you essentially land up on the same tickbase next PhysicsSimulate
  Basically the server will take our tick base and it will subtract our total simulation ticks, because of this the server simulates our commands individually
  Incrementing the tickbase each time, when PhysicsSimulate is called next frame and a commands are received as usual the tickbase will start as if it only incremented by 1
  So in reality it goes like this as an example
  1. TICKBASE = 1000
  2. SIMULATION_TICKS = 20
  3. ADJUST_PLAYER_TIME_BASE_RESULT = 1000-20 = 980 (THE SERVER HAS RETIMED THE PLAYER TO SYNC WITH THE SERVER BECAUSE IT IS TOO SLOW OR TOO FAST)
  4. TICKBASE IS NOW 1980
  5. SERVER SIMULATES ALL 20 SIMULATION_TICKS
  6. TICKBASE IS NOW 980+20 = 1000 BECAUSE THE SERVER HAS SIMULATED 20 COMMANDS
  7. NEXT SERVER FRAME
  8. ADJUST_PLAYER_TIME_BASE_RESULT = 1001 (THIS TIME THE SERVER HAS NOT RETIMED THE PLAYER, SO IT IS NORMAL)
  REAL EXAMPLE OF THIS:
  [!] AdjustPlayerTimeBase BEFORE: simulation_ticks: 21 tick_base: 80485
  [!] AdjustPlayerTimeBase AFTER: simulation_ticks: 21 tick_base: 80465
  [!] AdjustPlayerTimeBase BEFORE: simulation_ticks: 1 tick_base: 80486
  [!] AdjustPlayerTimeBase AFTER: simulation_ticks: 1 tick_base: 80486*/
void c_double_tap::adjust_player_time_base(i32 simulation_ticks){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  localplayer->tick_base() -= simulation_ticks;
}

void c_double_tap::auto_stop(){
  if(!is_shifting || is_warping)
    return;

  c_base_player* localplayer  = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!(localplayer->entity_flags() & FL_ONGROUND))
    return;

  vec3 vel = localplayer->velocity();
  vel.z = 0.f;

  float speed = vel.length_2d();
  if(speed < 1.f){
    global->current_cmd->move.x = 0.f;
    global->current_cmd->move.y = 0.f;
    return;
  }

  vec3 negative_dir = (vel * -1.f);
  vec3 negative_ang;
  math::vector_2_angle(&negative_dir, &negative_ang);

  negative_ang.y = global->current_cmd->view_angles.y - negative_ang.y;
  math::angle_2_vector(negative_ang, &negative_dir, nullptr, nullptr);
  negative_dir.normalize();

  global->current_cmd->move.x = negative_dir.x * 450.f;
  global->current_cmd->move.y = negative_dir.y * 450.f;
}

bool c_double_tap::is_shift_tick(){
  return is_shifting || shift_end >= global->original_cmd.command_number;
}

bool c_double_tap::wants_to_shift(){
  if(!can_weapon_dt() || !has_any_charged_ticks())
    return false;

  if(!config->misc.double_tap_shift_on_key)
    return true;

  return input_system->held(config->misc.double_tap_shift_hotkey);
}

bool c_double_tap::is_dt_active(){
  return is_shift_tick() || wants_to_shift();
}

bool c_double_tap::has_any_charged_ticks(){
  return config->misc.double_tap_enabled && charged_ticks > 0;
}

dt_state c_double_tap::get_charge_state(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer != nullptr){
    if(localplayer->is_valid()){
      if(config->misc.double_tap_on_ground_only && !(localplayer->entity_flags() & FL_ONGROUND))
        return DT_STATE_GROUND_ONLY;
    }
  }

  if(can_weapon_dt()){
    if(localplayer != nullptr){
      c_base_weapon* weapon = localplayer->get_weapon();
      if(weapon != nullptr){
        if(weapon->is_flamethrower() || weapon->is_sniper_rifle())
          return DT_STATE_READY_ATTACK2_ONLY;
        else if(weapon->is_short_circuit())
          return DT_STATE_READY_ATTACK1_2;
      }
    }

    return DT_STATE_READY;
  }

  if(is_charging)
    return DT_STATE_CHARGING;

  if(charged_ticks > 0 && charged_ticks < get_wanted_charge_ticks())
    return DT_STATE_NOT_ENOUGH_CHARGE;

  if(charged_ticks < 1)
    return DT_STATE_NO_CHARGE;

  return DT_STATE_DT_IMPOSSIBLE;
}

bool c_double_tap::can_weapon_dt(){
  if(is_shift_tick())
    return true;

  c_base_player* localplayer = utils::localplayer();

  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();

  if(weapon == nullptr)
    return false;

  // can the weapon even dt?
  if(!weapon->is_able_to_dt())
    return false;

  if(is_charging || charged_ticks < get_wanted_charge_ticks())
    return false;


  bool can_dt = true;

  float old_curtime     = globaldata->cur_time;
  globaldata->cur_time  = math::ticks_to_time(localplayer->tick_base());

  // When we dt, the server will roll our tickbase back by how many ticks we want to shift
  // Imagine that we have a curtime of 1.2s and our next primaryattack is 1.3s, but we want to shift 200ms
  // The server is going to make us start from 1s and by the end of our batch we will land back to where we started at 1.2
  // But the server already set our nextprimaryattack to 1.3s so even though we shifted 200ms, we still have to wait 100ms
  if(!weapon->is_sniper_rifle()){
    if(weapon->is_flamethrower() || weapon->is_short_circuit()){
      if(weapon->next_secondary_attack() + math::ticks_to_time(get_wanted_charge_ticks()) >= globaldata->cur_time){
        can_dt = false;
      }
    }
    else{
      if(weapon->is_flamethrower() || weapon->is_sniper_rifle() || !localplayer->can_fire() || weapon->next_primary_attack() + math::ticks_to_time(get_wanted_charge_ticks()) >= globaldata->cur_time)
        can_dt = false;
    }
  }

  globaldata->cur_time  = old_curtime;

  return can_dt;
}

bool c_double_tap::shift(bool aimbot_running){
  if(!config->misc.double_tap_enabled || is_shift_tick() || !can_weapon_dt())
    return false;

  // this is used for things like object aimbot where we never want to double tap
  if(!global->allow_double_tap)
    return false;

  if(config->misc.double_tap_on_aimbot_only && !aimbot_running)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  // only dt if we have more than 1 bullet
  if(!weapon->is_melee() && (localplayer->get_ammo_count(weapon->primary_ammo_type()) <= 1 && weapon->clip1() == -1 || (weapon->clip1() <= 1 && weapon->clip1() != -1)))
    return false;

  if(config->misc.double_tap_shift_on_key && !input_system->held(config->misc.double_tap_shift_hotkey))
    return false;

  if(config->misc.double_tap_on_ground_only && !(localplayer->entity_flags() & FL_ONGROUND))
    return false;

  if(weapon->is_sniper_rifle() && localplayer->is_scoped())
    return false;

  if(!weapon->is_melee() && !weapon->is_flamethrower() && !weapon->is_sniper_rifle() && !localplayer->will_fire_this_tick() && !(weapon->is_short_circuit() && localplayer->can_fire_secondary()))
    return false;

  if((weapon->is_flamethrower() || weapon->is_sniper_rifle()) && !(global->current_cmd->buttons & IN_ATTACK2))
    return false;

  should_shift   = true;
  packet_manager->force_send_packet(false);
  global->current_cmd->buttons &= ~IN_ATTACK;
  if(weapon->is_flamethrower() || weapon->is_short_circuit())
    global->current_cmd->buttons &= ~IN_ATTACK2;

  DBG("[!] DT: waiting for next tick...\n");

  return true;
}

bool c_double_tap::should_be_passive_charging(){
  if(!config->misc.double_tap_passive_charging || config->misc.double_tap_passive_charge_every_ms == 0.f)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  static u32  pkt_count = 0;
              pkt_count = ++pkt_count % (INT_MAX - 1);

  float time_since_fire = math::ticks_to_time(localplayer->tick_base()) - weapon->last_fire_time();
  if(time_since_fire < 1.f)
    return false;

  float charge_every_x_ms = math::clamp(config->misc.double_tap_passive_charge_every_ms / 1000.f, 0.100f, 1.f);
  if(charged_ticks >= get_wanted_charge_ticks() && ((pkt_count % math::time_to_ticks(2.f)) <= 1)){
    force_charge = true;
    return true;
  }

  return (pkt_count % math::time_to_ticks(charge_every_x_ms)) <= 1;
}

bool c_double_tap::handle_charge(float extra_samples){
  if(is_shift_tick())
    return false;

  static bool old_charge_key_held = false;
  bool charge_key_held            = input_system->held(config->misc.double_tap_charge_hotkey);
  bool charge_key_pressed         = !is_charging && (charge_key_held && !old_charge_key_held);

  if((should_be_passive_charging() || (is_charging = (charge_key_pressed || is_charging))) && (force_charge || charged_ticks < get_wanted_charge_ticks())){
    // when the charge key is pressed
    // send off a command to the server instructing that we have stopped firing
    if(global->client_state->choked_commands() > 0 || charge_key_pressed){
      packet_manager->lock(true);
      CALL_CL_MOVE(extra_samples, true);
      packet_manager->lock(false);

      DBG("[!] DT: sending off outstanding command(s)\n");
    }
    else{
      // call cl_runprediction to maintain the correct tickbase
      utils::call_fastcall64_raw<void>(global->cl_runprediction);

      // clamp charged ticks
      charged_ticks = math::smallest(++charged_ticks, 22);
    }

    should_shift        = false;
    force_charge        = false;
    old_charge_key_held = charge_key_held;
    return true;
  }

  is_charging         = false;
  old_charge_key_held = charge_key_held;

  return false;
}

bool c_double_tap::cl_move_run(float extra_samples, bool final_tick){
  if(!config->misc.double_tap_enabled){
    reset();
    return false;
  }

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(handle_charge(extra_samples))
    return true;

  if(!weapon->is_able_to_dt())
    return false;

  if(can_weapon_dt() && should_shift && global->client_state->choked_commands() == 0){
    is_shifting = true;
    packet_manager->lock(true);

    i32 wish_charge = get_wanted_charge_ticks();

    // adjust the tickbase corrected for the batch
    adjust_player_time_base(wish_charge);

    shift_end = global->original_cmd.command_number + wish_charge + 1 + 1;
    for(i32 i = 0; i < wish_charge; i++){
      CALL_CL_MOVE(extra_samples, i == (wish_charge - 1));
    }

    // adjust the tickbase again to account for the next batch
    adjust_player_time_base(wish_charge);

    packet_manager->lock(false);

    charged_ticks -= wish_charge;
    should_shift  = false;
    is_shifting   = false;
    return true;
  }

  should_shift = false;
  return false;
}

bool c_double_tap::cl_move_run_warp(float extra_samples, bool final_tick){
  if(!config->misc.double_tap_enabled){
    reset();
    return false;
  }

  if(!config->misc.double_tap_boost || !input_system->held(config->misc.double_tap_boost_key))
    return false;

  // Let cl_move_run one for dt to call handle_charge

  if(charged_ticks > 0 && global->client_state->choked_commands() == 0){
    is_shifting = true;
    is_warping  = true;
    packet_manager->lock(true);


    i32 wish_charge = math::smallest(math::clamp(config->misc.double_tap_boost_ticks, 2, 22), charged_ticks);

    // adjust the tickbase corrected for the batch
    adjust_player_time_base(wish_charge);

    shift_end = global->original_cmd.command_number + wish_charge + 1 + 1;
    for(i32 i = 0; i < wish_charge; i++){
      CALL_CL_MOVE(extra_samples, i == (wish_charge - 1));
    }

    // adjust the tickbase again to account for the next batch
    adjust_player_time_base(wish_charge);
    packet_manager->lock(false);

    charged_ticks -= wish_charge;
    should_shift  = false;
    is_shifting   = false;
    is_warping    = false;
    return true;
  }
  else{
    if(charged_ticks > 0){
      DBG("[!] c_double_tap::cl_move_run_warp unable to warp due to choked commands: %i > 0", global->client_state->choked_commands());
    }
  }

  should_shift = false;
  return false;
}

// Server side hooks.
#if defined(DEV_MODE)
EXPORT void __fastcall server_adjust_player_time_base_hook(void* ecx, void* edx, i32 simulation_ticks){
#if defined(DEV_MODE) && defined(DOUBLE_TAP_DEBUG)
  static bool did_dt = false;

  if(simulation_ticks > 1 || did_dt)
    DBG("[!] AdjustPlayerTimeBase BEFORE: simulation_ticks: %i tick_base: %i\n", simulation_ticks, *(i32*)((u32)ecx + 3624));

  //utils::call_fastcall<void, i32>(gen_internal->decrypt_asset(global->server_adjust_player_time_base_hook_trp), ecx, edx, simulation_ticks);

  if(simulation_ticks > 1 || did_dt)
    DBG("[!] AdjustPlayerTimeBase AFTER: simulation_ticks: %i tick_base: %i\n", simulation_ticks, *(i32*)((u32)ecx + 3624));

  if(did_dt)
    did_dt = false;

  if(simulation_ticks > 1)
    did_dt = true;
#else
 // utils::call_fastcall<void, i32>(gen_internal->decrypt_asset(global->server_adjust_player_time_base_hook_trp), ecx, edx, simulation_ticks);
#endif
}

EXPORT void __fastcall server_fire_bullet_hook(void* ecx, void* edx, c_base_weapon* wep, s_fire_bullets_info& info, bool do_effects, i32 damage_type, i32 custom_damage_type){
#if defined(DEV_MODE) && defined(DOUBLE_TAP_DEBUG)
  if(utils::is_listen_server()){
    s_trace trace = global->trace->ray(info.src, info.src + info.dir * info.distance, mask_bullet);
    bullet_tracers->add(trace.start, trace.end, rgb(0,0,255), 2.f);
  }
#endif

  //utils::call_fastcall<void, c_base_weapon*, s_fire_bullets_info&, bool, i32, i32>(gen_internal->decrypt_asset(global->server_fire_bullet_hook_trp), ecx, edx, wep, info, do_effects, damage_type, custom_damage_type);
}
#endif