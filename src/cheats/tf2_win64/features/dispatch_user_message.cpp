#include "../link.h"

c_dispatch_user_message_features* dispatch_user_msg = nullptr;

bool c_dispatch_user_message_features::dispatch_user_message(void* usermsg, i32 msg_type, s_bf_read* bf_read){
  if(c_base_dispatch_user_message_features::dispatch_user_message(usermsg, msg_type, bf_read))
    return true;

  i8* name = get_msg_name(usermsg, msg_type);
  if(name == nullptr)
    return false;

  const u32 name_hash = HASH_RT(name);
  if(name_hash == HASH("VoiceSubtitle")){
    i32 client = bf_read->read();
    i32 menu   = bf_read->read();
    i32 item   = bf_read->read();
    bf_read->reset();

    if(menu == 1 && item == 6)
      misc::uber_on_active_charge(client);

    return false;
  }

  auto_vote->dispatch_user_message_run(name_hash, msg_type, bf_read);
  privacy_mode->dispatch_user_message(name_hash, msg_type, bf_read);

  return false;
}

bool c_dispatch_user_message_features::handle_text_msg_features(s_text_msg* msg){
  if(c_base_dispatch_user_message_features::handle_text_msg_features(msg))
    return true;

  if(seed_prediction->dispatch_user_message(msg->bf_read))
    return true;

  if(config->misc.auto_retry_teambalance){
    if(msg->name_hash == HASH("#TF_Autobalance_TeamChangePending")){
      if(!auto_vote->localplayer_being_kicked)
        global->engine->client_cmd(XOR("retry"));
    }
  }

  return false;
}