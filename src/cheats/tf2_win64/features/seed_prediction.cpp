#include "../link.h"

c_seed_prediction* seed_prediction = nullptr;

bool c_seed_prediction::active(){
  if(!utils::is_in_game())
    return false;

  if(global->aimbot_settings == nullptr)
    return false;

  if(!global->aimbot_settings->predict_server_random_seed)
    return false;

  if(utils::is_safe_mode_enabled())
    return false;
  
  return true;
}

void c_seed_prediction::process_info_panel(){
  if(!active())
    return;

  float last_sync_delta = math::abs(plat_float_time() - last_sync_time);

  if(utils::is_listen_server()){
    global->info_panel->add_entry(INFO_PANEL_SEED_PRED, WXOR(L"LISTEN SERVER"), INFO_PANEL_WARNING_CLR);
    return;
  }
  else if(is_uptime_too_low()){
    global->info_panel->add_entry(INFO_PANEL_SEED_PRED, WXOR(L"UPTIME TOO LOW"), INFO_PANEL_WARNING_CLR);
    return;
  }

  float latency = utils::get_latency();
  if(last_latency > 0.0f){
    float delta = math::abs(latency - last_latency);
    if(delta > globaldata->interval_per_tick * 2.f){
      global->info_panel->add_entry(INFO_PANEL_SEED_PRED, WXOR(L"UNRELIABLE CONNECTION"), INFO_PANEL_WARNING_CLR);
      last_latency = latency;
      return;
    }
  }

  last_latency = latency;

  i32 hours = (i32)math::round(predicted_time / 3600.f);
  i32 days  = (i32)math::round(predicted_time / 86400.f);

  wchar_t buf[64] = {};
  if(days > 0){
    formatW(buf, WXOR(L"UPTIME %iD"), days);
  }
  else if(hours > 0){
    formatW(buf, WXOR(L"UPTIME %iH"), hours);
  }
 
  global->info_panel->add_entry(INFO_PANEL_SEED_PRED, buf, last_sync_delta <= 5.f ? INFO_PANEL_LEGIT_CLR : INFO_PANEL_WARNING_CLR);
}