#pragma once

class c_interface_engine;
class c_base_seed_prediction{
public:
  bool  execute_player_pref         = false;
  float execute_player_pref_timeout = 0.f;
  float last_current_time           = 0.f;
  float sent_time                   = 0.f;
  float predicted_seed_time         = 0.f;
  float predicted_time              = 0.f;
  float last_received_time          = 0.f;
  float last_sync_time              = 0.f;
  bool  synced_this_update          = false;
  float last_latency                = 0.f;
  float sync_offset                 = 0.f;
  
  virtual void reset(){
    execute_player_pref         = false;
    execute_player_pref_timeout = 0.f;
    last_current_time           = 0.f;
    sent_time                   = 0.f;
    predicted_seed_time         = 0.f;
    predicted_time              = 0.f;
    last_received_time          = 0.f;
    last_sync_time              = 0.f;
    synced_this_update          = false;
    last_latency                = 0.f;
    sync_offset                 = 0.f;
  }

  virtual bool active(){
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  virtual void   run_command();
  virtual void   predict_time();
  virtual void   process_info_panel(){
    FUNCTION_OVERRIDE_NEEDED;
  }
  virtual void   parse_player_pref(std::string str);
  virtual double plat_float_time();
  virtual i32    get_seed();
  virtual bool   is_synced();
  virtual bool   is_uptime_too_low();
  ALWAYSINLINE float get_current_uptime(){
    return last_current_time;
  }

  ALWAYSINLINE float get_mantissa_step(){
    float val     = last_current_time * 1000.f;
    i32 raw_value = *(i32*)&val;
    i32 exp       = (raw_value >> 23) & 0xFF;
    return math::powf(2, exp - 150);
  }

  virtual void pre_create_move(){
    process_info_panel();
  }

  virtual void engine_paint(){
    run_command();
    predict_time();
  }

  virtual bool dispatch_user_message(s_bf_read* bf_read);
};