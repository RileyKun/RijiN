#include "link.h"

EXPORT bool __stdcall rtl_dispatch_exception_hook(EXCEPTION_RECORD* exception_record, CONTEXT* context_frame){
  if(exception_record->ExceptionCode == EXCEPTION_SINGLE_STEP){
    // Dumps stack pointer at moment in time when CHLClient::CreateMove pushes RDI to RSP+0
    // We can write to this pointer which when restored after pop rdi will allow us to manipulate bSendPacket
   if(exception_record->ExceptionAddress == global->cl_move_test_dil_addr ||
      exception_record->ExceptionAddress == global->cl_move_send_packet_false_addr){
      packet_manager->handle_eh(context_frame, exception_record->ExceptionAddress == global->cl_move_send_packet_false_addr ? EH_CL_MOVE_SP_LOCATION_ZEROING : EH_CL_MOVE_SP_LOCATION_SET_CHOKE);
      return true;
    }
  }
  else{
    if(exceptions::from_eh(exception_record, context_frame, XOR("TF2"))){
      utils::nt_continue(context_frame, false);
      return true;
    }
  }

  return utils::call_stdcall<bool, EXCEPTION_RECORD*, CONTEXT*>(gen_internal->decrypt_asset(global->rtl_dispatch_exception_hook_trp), exception_record, context_frame);
}