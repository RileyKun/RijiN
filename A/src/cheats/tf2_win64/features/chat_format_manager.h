#pragma once

class c_chat_format_manager : public c_base_chat_format_manager{
public:
  c_chat_format_settings* get_settings() override{
    return &config->chat;
  }
};

CLASS_EXTERN(c_chat_format_manager, chat_format_manager);