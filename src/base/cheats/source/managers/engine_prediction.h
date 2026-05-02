#pragma once

class c_interface_prediction;
class c_base_engine_prediction{
public:
  bool is_calling_run_command = false;
  u32  old_tick_base          = 0;

  virtual void set_host(c_internal_base_player* player){
    #if defined(SOURCE_2018) || defined(__x86_64__) // Well if you're crashing, this might be why, Make sure the x64 source engine game supports this.
      if(global_internal->move_helper == nullptr){
        DBG("[-] c_base_engine_prediction::set_host move_helper is nullptr\n");
        return;
      }

      create_offset_hash("move_helper_set_host");
      utils::internal_virtual_call<void, c_internal_base_player*>(offset, global_internal->move_helper, player);
    #endif
  }

  virtual bool can_run_prediction(c_user_cmd* cmd){
    if(cmd == nullptr)
      return false;

    if(cmd->command_number == 0)
      return false;

    c_internal_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return false;

    if(!localplayer->is_base_alive()){
      DBG("[-] c_engine_prediction::can_run_prediction - not alive or in a valid state\n");
      return false;
    }

    if(gen_internal->decrypt_asset(global_internal->run_command_hook_trp) == 0){
      DBG("[-] c_engine_prediction::can_run_prediction - missing run_command_hook hook\n");
      return false;
    }

    if(global_internal->cl_runprediction == 0){
      DBG("[-] c_engine_prediction::can_run_prediction - global_internal->cl_runprediction is nullptr\n");
      return false;
    }

    if(global_internal->prediction_random_seed == nullptr){
      DBG("[-] c_engine_prediction::can_run_prediction - is prediction random seed is nullptr\n");
      return false;
    }

    if(global_internal->move_helper == nullptr){
      DBG("[-] c_base_engine_prediction - global_internal->move_helper is nullptr\n");
      return false;
    }

    return true;
  }

  virtual void cl_run_prediction(){
    #if defined(__x86_64__)
      utils::call_fastcall64_raw<void>(global_internal->cl_runprediction);
    #else
      utils::call_cdecl<void, u32>(global_internal->cl_runprediction, 0);
    #endif
  }

  // Call backs functions I guess?
  virtual void on_pre_prediction(c_user_cmd* cmd){
    c_internal_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    memcpy(&global_internal->old_global_data, global_internal->global_data, sizeof(s_global_data));
    global_internal->old_prediction_seed = *global_internal->prediction_random_seed;

    old_tick_base                        = localplayer->tick_base();
  }

  virtual void on_post_prediction(c_user_cmd* cmd){
    global_internal->did_run_prediction = true;
    c_internal_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
       return;

    global_internal->predicted_tick_base = localplayer->tick_base();
    localplayer->tick_base()             = old_tick_base;
    *global_internal->prediction_random_seed = cmd->random_seed;
  }

  virtual bool is_frozen(){
    c_internal_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return false;

    return localplayer->entity_flags() & FL_FROZEN;
  }

  virtual void clear_movement(c_user_cmd* cmd, bool remove_buttons = false){
    if(cmd == nullptr)
      return;

    cmd->move = vec3(0.f, 0.f, 0.f);

    if(remove_buttons){
      cmd->buttons = 0;
      cmd->impulse = 0;
    }
  }

  virtual bool start_prediction(c_internal_base_player* localplayer, c_user_cmd* cmd);

  virtual void end_prediction(c_user_cmd* cmd){
    if(cmd == nullptr || !global_internal->did_run_prediction)
      return;

    global_internal->did_run_prediction = false;

    // Restore the globaldata after createmove
    memcpy(global_internal->global_data, &global_internal->old_global_data, sizeof(s_global_data));

    // Restore prediction seed.
    *global_internal->prediction_random_seed = global_internal->old_prediction_seed;
  }
};