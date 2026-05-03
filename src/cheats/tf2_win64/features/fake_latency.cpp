#include "../link.h"

CLASS_ALLOC(c_fake_latency, fake_latency);

EXPORT void __fastcall process_packet_hook(c_net_channel* rcx, void* packet, bool header){
  assert(rcx != nullptr);

  utils::call_fastcall64<void, void*, bool>(gen_internal->decrypt_asset(global->process_packet_hook_trp), rcx, packet, header);

  fake_latency->process_packet_run(rcx);
}

EXPORT i32 __fastcall send_datagram_hook(c_net_channel* rcx, void* datagram){
  static s_packet_record original_packet_record;
  bool                   should_run = fake_latency->send_datagram_pre_run(rcx, &original_packet_record);
  i32                    ret        = utils::call_fastcall64<i32, void*>(gen_internal->decrypt_asset(global->send_datagram_hook_trp), rcx, datagram);

  // Didn't run our handler, return here
  if(!should_run)
    return ret;

  fake_latency->send_datagram_post_run(rcx, &original_packet_record);

  return ret;
}