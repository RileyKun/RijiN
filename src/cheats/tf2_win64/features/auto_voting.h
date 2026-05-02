#pragma once

class c_auto_voting{
public:
  bool localplayer_being_kicked;

  ALWAYSINLINE void reset(){
    memset(this, 0, sizeof(c_auto_voting));
  }

  void run_auto_cast();
  i32  resolve_votekick_victim(wchar_t* victim_name);
  void handle_user_message(u8 caster_index, u8 team, u32 vote_index, wchar_t* victim_name);
  void dispatch_user_message_run(u32 name_hash, i32 msg_type, s_bf_read* bf_read);
};

CLASS_EXTERN(c_auto_voting, auto_vote);