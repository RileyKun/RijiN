#pragma once

class c_base_log_manager{
public:
  c_gcs_component_log_window* infractions = nullptr; // Cheater detection.
  c_gcs_component_log_window* connections = nullptr; // Player join/leaves
  c_gcs_component_log_window* votes = nullptr;       // Votes.

  virtual void on_game_event(c_game_event* event);
  virtual void dispatch_user_message(void* usermsg, i32 msg_type, s_bf_read* bf_read);
  virtual void send_net_msg(c_net_message& net_msg, bool rel, bool voice);

  virtual void reset(){
    if(infractions != nullptr)
      infractions->clear_history();

    if(connections != nullptr)
      connections->clear_history();

    if(votes != nullptr)
      votes->clear_history();
  }

  virtual void unload(){
    #if defined(DEV_MODE)
     reset();
    #endif
  }
};