#include "../../../link.h"
#include "seed_prediction.h"

void c_base_seed_prediction::run_command(){
  if(!active())
    return;

  float time = math::time();
  if(execute_player_pref && time <= execute_player_pref_timeout)
    return;

  // Send the command that leaks the server uptime.
  global_internal->engine->server_cmd(XOR("playerperf"), false);

  sent_time = plat_float_time();
  execute_player_pref         = true;
  execute_player_pref_timeout = time + global_internal->engine->get_latency(true) + 0.5f;
}

void c_base_seed_prediction::predict_time(){
  if(!active())
    return;

  // Just winging it here, no idea if it works. Doesn't matter if it doesn't tbh.
  if(utils::is_listen_server()){
    predicted_time = predicted_seed_time = last_received_time = plat_float_time() + global_internal->global_data->interval_per_tick;
    return;
  }

  if(last_received_time <= 0.0f)
    return;

  float predicted_step = (plat_float_time() - last_received_time);
  if(predicted_step < sync_offset || sync_offset <= 0.000f)
    return;

  // This is used for the actual seed so we want to make sure this is predicted.
  predicted_seed_time += sync_offset;
  last_received_time = plat_float_time();
}

void c_base_seed_prediction::parse_player_pref(std::string str){
  execute_player_pref = false;
  if(!active())
    return;

  if(str.empty()){
    DBG("[-] c_seed_prediction::parse_player_pref error\n");
    return;
  }

  float new_time = convert::str_to_float((i8*)str.c_str());
  if(new_time <= last_current_time) // Only accept newer times.
    return;

  float delta = new_time - last_current_time;
  last_current_time = new_time;

  float response_delay = (float)(plat_float_time() - sent_time);

  if(predicted_time > 0.0f){
    if(predicted_time == new_time){
      sync_offset        = delta;
      last_sync_time     = plat_float_time();
      synced_this_update = true;
    }
    else
     synced_this_update = false;
  }

  predicted_time      = new_time + response_delay;
  predicted_seed_time = predicted_time + delta;
  last_received_time  = plat_float_time();
}

double c_base_seed_prediction::plat_float_time(){
  #if defined(__x86_64__)
  static void* plat_float_time_ptr = utils::get_proc_address(utils::get_module_handle(HASH("tier0.dll")), HASH("Plat_FloatTime"));
  return utils::call_fastcall64_raw<double>(plat_float_time_ptr);
  #else
  static void* plat_float_time_ptr = utils::get_proc_address(utils::get_module_handle(HASH("tier0.dll")), HASH("Plat_FloatTime"));
  return utils::call_cdecl<double>(plat_float_time_ptr);
  #endif
}

i32 c_base_seed_prediction::get_seed(){
  float time_now = (float)((predicted_seed_time) * 1000.f);
  return *(i32*)((i8*)&time_now); 
}

bool c_base_seed_prediction::is_synced(){
  if(last_current_time <= 0.0f || last_sync_time <= 0.0f || is_uptime_too_low())
    return false;

  return true;//math::abs(plat_float_time() - last_sync_time) <= 10.f; // the '10.f' value is made up. 
}

bool c_base_seed_prediction::is_uptime_too_low(){
  return get_mantissa_step() < 1.0f;
}

bool c_base_seed_prediction::dispatch_user_message(s_bf_read* bf_read){
  if(bf_read == nullptr)
    return false;

  i32 dest         = bf_read->read();
  i8* textmsg_name = bf_read->read_str(false);

  if(textmsg_name == nullptr)
    return false;

  //u32 textmsg_hash = HASH_RT(textmsg_name);

  // Very important!
  bf_read->reset();

  if(dest != 2) // Must be two
    return false;

  u32 len = strlen(textmsg_name);
  if(!len || textmsg_name[0] == '#')
    return true; // Make sure this is the correct message we're looking for.

  if(!str_utils::contains(textmsg_name, XOR("vel")))
    return true;

  // Grab the uptime float value from the string.
  std::string number = "";
  for(i32 i = 0; i < len; i++){
    if(textmsg_name[i] == ' ' || textmsg_name[i] == '\n')
      break;

    number += textmsg_name[i];
  }

  parse_player_pref(number);

  return true;
}