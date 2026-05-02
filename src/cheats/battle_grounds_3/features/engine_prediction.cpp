#include "../link.h"

CLASS_ALLOC(c_engine_prediction, engine_prediction);

EXPORT void __fastcall run_command_hook(void* ecx, void* edx, c_base_player* player, s_user_cmd* cmd, void* move_helper){
  assert(ecx != nullptr);

  if(!global->is_calling_run_command)
    global->move_helper = move_helper;

  utils::call_fastcall<void, c_base_player*, s_user_cmd*, void*>(gen_internal->decrypt_asset(global->run_command_hook_trp), ecx, edx, player, cmd, move_helper);
}

EXPORT void __fastcall post_think_hook(c_base_player* ecx, void* edx){
  assert(ecx != nullptr);

  if(global->is_calling_run_command)
    return;

  utils::call_fastcall<void>(gen_internal->decrypt_asset(global->post_think_hook_trp), ecx, edx);
}