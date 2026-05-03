#pragma once

// NOTE:
// It would be real nice to just call localplayer->PhysicsSimulate since its doing exactly what we want in here anyway
// Edit: better to rebuild it instead since the command context will always be 1 tick behind, we could copy it but I just dont want to bother
// https://github.com/TheAlePower/TeamFortress2/blob/1b81dded673d49adebf4d0958e52236ecc28a956/tf2_src/game/client/c_baseplayer.cpp#L2304

class c_engine_prediction{
public:
  void run(){
    if(global->current_cmd == nullptr){
      DBG("[-] c_engine_prediction::run(): global->current_cmd is nullptr\n");
      return;
    }

    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr){
      DBG("[-] c_engine_prediction::run(): localplayer is nullptr\n");
      return;
    }

    if(gen_internal->decrypt_asset(global->run_command_hook_trp) == 0){
      DBG("[-] c_engine_prediction::run(): gen_internal->decrypt_asset(global->run_command_hook_trp) is nullptr\n");
      return;
    }

    if(global->cl_runprediction == 0){
      DBG("[-] c_engine_prediction::run(): global->cl_runprediction is nullptr\n");
      return;
    }

    if(global->prediction_random_seed == nullptr){
      DBG("[-] c_engine_prediction::run(): prediction_random_seed is nullptr\n");
      return;
    }

    if(global->move_helper == nullptr)
      return;

    // Backup data
    {
      memcpy(&global->old_global_data, globaldata, sizeof(s_global_data));
      global->old_prediction_seed = *global->prediction_random_seed;
    }

    u32 old_tick_base = localplayer->tick_base;
    utils::call_cdecl<void, u32>(global->cl_runprediction, 0);
    {
      global->prediction->in_prediction         = true;
      global->prediction->first_time_predicted  = false;

      // invoke runcommand
      {
        global->is_calling_run_command = true;
        global->prediction->run_command(localplayer, global->current_cmd, global->move_helper);
        global->is_calling_run_command = false;
      }

      global->did_run_prediction = true;

      global->prediction->in_prediction         = false;
      global->prediction->first_time_predicted  = false;
    }
    localplayer->tick_base = old_tick_base;

    // Runcommand does do this, but at finishcommand it sets this again to NULL (aka -1)
    *global->prediction_random_seed   = global->current_cmd->random_seed;
  }

  void finish(){
    if(!global->did_run_prediction)
      return;

    // Restore the globaldata after createmove
    memcpy(globaldata, &global->old_global_data, sizeof(s_global_data));

    // Restore prediction seed.
    *global->prediction_random_seed = global->old_prediction_seed;

    global->did_run_prediction = false;
  }
};

CLASS_EXTERN(c_engine_prediction, engine_prediction);