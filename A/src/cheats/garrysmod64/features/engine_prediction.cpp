#include "../link.h"

CLASS_ALLOC(c_engine_prediction, engine_prediction);

void c_engine_prediction::run(){
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

    // READ ME:
    // This function doesn't exist in x64 gmod. Also might be detectable from lua.
    
    //if(global->cl_runprediction == 0){
    //  DBG("[-] c_engine_prediction::run(): global->cl_runprediction is nullptr\n");
    //  return;
    //}

    if(global->prediction_random_seed == nullptr){
      DBG("[-] c_engine_prediction::run(): prediction_random_seed is nullptr\n");
      return;
    }

    if(global->move_helper == nullptr)
      return;

    global->current_cmd->random_seed = math::md5_pseudo_random(global->current_cmd->command_number) & 0xFF;

    //utils::call_fastcall64_raw<void, u32>(global->cl_runprediction, 0);

    // Backup data
    {
      memcpy(&global->old_global_data, globaldata, sizeof(s_global_data));
      global->old_prediction_seed = *global->prediction_random_seed;
    }

    u32 old_tick_base = localplayer->tick_base;
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

      if(!accuracy->is_dumping_spread)
        global->predicted_tick_base = localplayer->tick_base;
    }
    localplayer->tick_base = old_tick_base;

    // Runcommand does do this, but at finishcommand it sets this again to NULL (aka -1)
    *global->prediction_random_seed   = global->current_cmd->random_seed;
}

EXPORT void __fastcall run_command_hook(void* rcx, c_base_player* player, s_user_cmd* cmd, void* move_helper){
  assert(rcx != nullptr);

  if(!global->is_calling_run_command)
    global->move_helper = move_helper;

  vec3 view_angle = global->engine->get_view_angles();

  utils::call_fastcall64<void, c_base_player*, s_user_cmd*, void*>(gen_internal->decrypt_asset(global->run_command_hook_trp), rcx, player, cmd, move_helper);

  if(config->aimbot.no_recoil)
    global->engine->set_view_angles(view_angle);
}

EXPORT void __fastcall post_think_hook(c_base_player* rcx){
  assert(rcx != nullptr);

  if(global->is_calling_run_command && !accuracy->is_dumping_spread)
    return;

  utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->post_think_hook_trp), rcx);
}