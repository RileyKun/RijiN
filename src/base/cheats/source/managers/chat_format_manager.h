#pragma once

class c_chat_format_settings{
public:
  union{
    struct{
      bool block_server_messages = false;
      bool chat_tags             = false;
      bool chat_tags_cheaters    = false;
      bool chat_tags_friends     = false;
      bool chat_tags_localplayer = false;
      bool auto_mute_bots_chats  = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

enum e_chat_format_return_type{
  e_chat_format_type_none = 0,
  e_chat_format_type_replace_msg = 1,
  e_chat_format_type_no_call = 2,
};

class c_base_chat_format_manager{
public:
  virtual c_chat_format_settings* get_settings(){
    FUNCTION_OVERRIDE_NEEDED;
    return nullptr;
  }

  virtual void create_menu_settings(gcs_component_group* group, c_chat_format_settings* settings){
    assert(settings != nullptr);
    assert(group != nullptr);

    gcs::dropdown(group, WXOR(L"Tags"), true)
      ->add_option(WXOR(L"Enabled"), &settings->chat_tags)
      ->add_option(WXOR(L"Localplayer"), &settings->chat_tags_localplayer)
      ->add_option(WXOR(L"Friends"), &settings->chat_tags_friends)
      ->add_option(WXOR(L"Cheaters"), &settings->chat_tags_cheaters);

    gcs::toggle(group, WXOR(L"Block server messages"), &settings->block_server_messages)
      ->set_help_text(WXOR(L"Chat messages that belong to the server will be blocked."));

    gcs::toggle(group, WXOR(L"Block cheater bot messages"), &settings->auto_mute_bots_chats)
      ->set_help_text(WXOR(L"Will mute players who are marked as cheating bots."));
  }

  virtual i32 hud_chatline_insert_and_colorize_text(wchar_t* buf, i32 index, wchar_t* new_buf, u32 new_buf_len);
  virtual std::wstring get_tag(i32 index);

  virtual u32 get_tag_colour(i32 index);
};