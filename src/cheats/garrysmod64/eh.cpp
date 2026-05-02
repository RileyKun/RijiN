#include "link.h"

EXPORT bool __stdcall rtl_dispatch_exception_hook(EXCEPTION_RECORD* exception_record, CONTEXT* context_frame){
  if(exception_record->ExceptionCode == EXCEPTION_SINGLE_STEP){
    // Dumps stack pointer at moment in time when CHLClient::CreateMove pushes RDI to RSP+0
    // We can write to this pointer which when restored after pop rdi will allow us to manipulate bSendPacket
    if(exception_record->ExceptionAddress == global->chlclient_createmove_push_rdi){
      DBG("[!] bSendPacket RSP: %p\n", context_frame->Rsp);

      // Store RSP
      global->bsendpacket_rdi_rsp = context_frame->Rsp;

      // Clear DR0 and L0BPDR0 FLAG
      context_frame->Dr0 = 0;
      context_frame->Dr7 = 0;
  
      // Resume
      context_frame->EFlags |= (1 << 16); // RF Flag
      utils::nt_continue(context_frame, false);
    }
  }
  else{
    utils::on_exception_internal(exception_record, context_frame, XOR("GMOD-64bit"));
  }

  return utils::call_stdcall<bool, EXCEPTION_RECORD*, CONTEXT*>(gen_internal->decrypt_asset(global->rtl_dispatch_exception_hook_trp), exception_record, context_frame);
}