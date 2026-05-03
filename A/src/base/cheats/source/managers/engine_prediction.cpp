#include "../../../link.h"

bool c_base_engine_prediction::start_prediction(c_internal_base_player* localplayer, c_user_cmd* cmd){
  if(!can_run_prediction(cmd)){
    DBG("[-] c_base_engine_prediction::start_prediction can't run\n");
    return false;
  }

  global_internal->prediction->in_prediction        = true;
  global_internal->prediction->first_time_predicted = false;

  on_pre_prediction(cmd);
  cl_run_prediction();
  {
    global_internal->is_calling_run_command = true;
    is_calling_run_command = true;
    set_host(localplayer);
    global_internal->prediction->run_command(localplayer, cmd, global_internal->move_helper);
    set_host(nullptr);
    is_calling_run_command = false;
    global_internal->is_calling_run_command = false;
  }
  on_post_prediction(cmd);
  global_internal->prediction->in_prediction        = false;
  global_internal->prediction->first_time_predicted = false;

  return true;
} 