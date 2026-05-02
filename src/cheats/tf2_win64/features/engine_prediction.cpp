#include "../link.h"

c_engine_prediction* engine_prediction = nullptr;

EXPORT void __fastcall run_command_hook(void* rcx, c_base_player* player, c_user_cmd* cmd, void* move_helper){
  assert(rcx != nullptr);

  // C_TFPlayer::ClientThink()
  // Game thinks its funny in special modes to constantly turn off prediction?
  // It doesn't even bother to turn it back on, so we will do it
  if(global->cl_predict != nullptr && global->cl_predict->val != 1)
    global->cl_predict->set_value(1);

  // Prevent people from capping their fps really low with the fps_max command.
  if(global->fps_max != nullptr && global->fps_max->val > 0 && global->fps_max->val < 60)
    global->fps_max->set_value(0); // Unrestrict their framerate.

  if(!engine_prediction->is_calling_run_command)
    global->move_helper = move_helper;

  utils::call_fastcall64<void, c_base_player*, c_user_cmd*, void*>(gen_internal->decrypt_asset(global->run_command_hook_trp), rcx, player, cmd, move_helper);
}

EXPORT void __fastcall process_impacts_hook(void* rcx){
  if(engine_prediction->is_calling_run_command)
    return;

  utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->process_impacts_hook_trp), rcx);
}

EXPORT void __fastcall post_think_hook(void* rcx){
  assert(rcx != nullptr);

  if(engine_prediction->is_calling_run_command)
    return;

  utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->post_think_hook_trp), rcx);
}