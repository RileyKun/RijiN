#include "../link.h"

CLASS_ALLOC(c_tickbase_correction, tickbase_correction);

EXPORT void __fastcall run_command_hook(void* ecx, void* edx, c_base_player* player, c_user_cmd* cmd, void* move_helper){
  assert(ecx != nullptr);

  if(!global->is_calling_run_command){
    global->move_helper = move_helper;
    misc::fix_nospread_pred(player, cmd);
  }

  c_base_player* localplayer = utils::localplayer();
  if(localplayer != nullptr && global->speedhack_active && player == localplayer && localplayer->is_alive()){
    static i32 last_client_tick     = 0;
    const bool is_speedhack_command = last_client_tick == globaldata->tick_count;
    last_client_tick                = globaldata->tick_count;
  
    if(is_speedhack_command){
      localplayer->tick_base()--;
      //DBG("TEST\n");
    }
  }

/*
  // Set processing tick
  tickbase_correction->processing_tick = cmd->command_number;

  s_tb_record* tb_record = tickbase_correction->find_record(cmd->command_number);
  if(tb_record != nullptr){
    player->tick_base             = tb_record->corrected_tickbase + ((cmd->command_number - tb_record->command_number) + 1);
    player->final_predicted_tick  = player->tick_base;
    globaldata->cur_time          = math::ticks_to_time(player->tick_base);

    if(cmd->predicted)
      return;

    c_base_weapon* wep = player->get_weapon();
    DBG("%i %i %i\n", math::time_to_ticks(globaldata->cur_time), math::time_to_ticks(wep->next_primary_attack), globaldata->cur_time >= wep->next_primary_attack ? 1 : 0);
  }
*/

  utils::call_fastcall<void, c_base_player*, c_user_cmd*, void*>(gen_internal->decrypt_asset(global->run_command_hook_trp), ecx, edx, player, cmd, move_helper);
}

EXPORT void __fastcall run_simulation_hook(void* ecx, void* edx, i32 current_command, float curtime, c_user_cmd* cmd, c_base_player* localplayer){

/*
  s_tb_record* tb_record = tickbase_correction->find_record(current_command);
  if(false && tb_record != nullptr){
    localplayer->tick_base  = tb_record->corrected_tickbase + ((current_command - tb_record->command_number) + 1);
    curtime                 = math::ticks_to_time(localplayer->tick_base);

    if(cmd->predicted)
      return;

    DBG("%i %i %i\n", tb_record->corrected_tickbase, localplayer->tick_base, ((current_command - tb_record->command_number) + 1));
  }
*/

  utils::call_fastcall<void, i32, float, c_user_cmd*, c_base_player*>(gen_internal->decrypt_asset(global->run_simulation_hook_trp), ecx, edx, current_command, curtime, cmd, localplayer);
}