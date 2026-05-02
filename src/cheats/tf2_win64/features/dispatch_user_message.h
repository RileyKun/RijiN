#pragma once

class c_dispatch_user_message_features : public c_base_dispatch_user_message_features{
public:

  c_dispatch_user_message_feature_settings* get_settings() override{
    return &config->dum;
  }

  bool dispatch_user_message(void* usermsg, i32 msg_type, s_bf_read* bf_read) override;
  bool handle_text_msg_features(s_text_msg* msg) override;
};

CLASS_EXTERN(c_dispatch_user_message_features, dispatch_user_msg);