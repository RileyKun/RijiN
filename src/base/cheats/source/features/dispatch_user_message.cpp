#include "../../../link.h"

// Returning true will stop the original call.
bool c_base_dispatch_user_message_features::dispatch_user_message(void* usermsg, i32 msg_type, s_bf_read* bf_read){
  assert(msg_type >= 0);
  assert(bf_read != nullptr);
  if(bf_read == nullptr)
    return false;

  // Gotta be filled if we wish todo anything.
  c_dispatch_user_message_feature_settings* settings = get_settings();
  if(settings == nullptr)
    return false;

  i8* name = get_msg_name(usermsg, msg_type);
  if(name == nullptr)
    return false;

  const u32 name_hash = HASH_RT(name);
  switch(name_hash){
    default: break;
    case HASH("TextMsg"):
    {
      s_text_msg msg{};
      msg.bf_read     = bf_read;
      msg.dest        = bf_read->read();
      msg.name        = bf_read->read_str(false);
      if(msg.name != nullptr)
        msg.name_hash = HASH_RT(msg.name);

      bf_read->reset();
      return handle_text_msg_features(&msg);
    }
    case HASH("ForcePlayerViewAngles"): return settings->no_angle_snap;
    case HASH("Fade"): return settings->no_fade;
    case HASH("Rumble"): return settings->no_rumble;
    case HASH("Shake"): return settings->no_shake;
    case HASH("VGUIMenu"):
    {
      i8* panel_name = bf_read->read_str(false);
      if(panel_name == nullptr)
        return false;

      DBG("[!] VGUIMenu: %s\n", panel_name);

      const u32 hash = HASH_RT(panel_name);
      if(settings->disable_motd){
        if(hash == HASH("info")){
          global_internal->engine->client_cmd(XOR("closedwelcomemenu"));
          bf_read->reset();
          return true;
        }
      }

      bf_read->reset();
    }
  }

  return false;
}

bool c_base_dispatch_user_message_features::handle_text_msg_features(s_text_msg* msg){
  assert(msg != nullptr);

  return false;
}