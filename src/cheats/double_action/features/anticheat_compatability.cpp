#include "../link.h"

CLASS_ALLOC(c_acp, acp);

EXPORT bool __fastcall cbaseclientstate_process_print_hook(void* ecx, void* edx, s_svc_print* msg){
  assert(ecx != nullptr);
  assert(msg != nullptr);
  assert(msg->txt != nullptr);
  if(acp->on_server_print(msg->txt))
    return true;

  return utils::call_fastcall<bool, s_svc_print*>(gen_internal->decrypt_asset(global->cbaseclientstate_process_print_hook_trp), ecx, edx, msg);
}