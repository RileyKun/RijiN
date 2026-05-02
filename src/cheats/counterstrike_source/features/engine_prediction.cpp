#include "../link.h"

c_engine_prediction* engine_prediction = nullptr;

EXPORT void __fastcall run_command_hook(void* rcx, c_base_player* player, c_user_cmd* cmd, void* move_helper){
  assert(rcx != nullptr);

  if(!global->is_calling_run_command)
    global->move_helper = move_helper;

  utils::call_fastcall64<void, c_base_player*, c_user_cmd*, void*>(gen_internal->decrypt_asset(global->run_command_hook_trp), rcx, player, cmd, move_helper);
}

EXPORT void __fastcall post_think_hook(c_base_player* rcx){
  assert(rcx != nullptr);

  if(global->is_calling_run_command)
    return;

  utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->post_think_hook_trp), rcx);
}

EXPORT void __fastcall process_impacts_hook(void* rcx){
  if(global->is_calling_run_command)
    return;

   utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->process_impacts_hook_trp), rcx);
}