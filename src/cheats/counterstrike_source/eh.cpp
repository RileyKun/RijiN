#include "link.h"

EXPORT bool __stdcall rtl_dispatch_exception_hook(EXCEPTION_RECORD* exception_record, CONTEXT* context_frame){
  if(exception_record->ExceptionCode == EXCEPTION_SINGLE_STEP){
    if(exception_record->ExceptionAddress == global->cl_move_test_dil_addr ||
      exception_record->ExceptionAddress == global->cl_move_send_packet_false_addr){
      packet_manager->handle_eh(context_frame, exception_record->ExceptionAddress == global->cl_move_send_packet_false_addr ? EH_CL_MOVE_SP_LOCATION_ZEROING : EH_CL_MOVE_SP_LOCATION_SET_CHOKE);
      return true;
    }
  }
  else{
    utils::on_exception_internal(exception_record, context_frame, XOR("CS:S-64bit"));
  }

  return utils::call_stdcall<bool, EXCEPTION_RECORD*, CONTEXT*>(gen_internal->decrypt_asset(global->rtl_dispatch_exception_hook_trp), exception_record, context_frame);
}