#include "../link.h"

c_engine_prediction* engine_prediction = nullptr;

EXPORT void __fastcall post_think_hook(c_base_player* ecx, void* edx){
  assert(ecx != nullptr);

  if(global->is_calling_run_command)
    return;

  utils::call_fastcall<void>(gen_internal->decrypt_asset(global->post_think_hook_trp), ecx, edx);
}