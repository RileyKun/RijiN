#pragma once

struct s_text_msg{
  ALWAYSINLINE s_text_msg(){
    dest      = 0;
    name      = nullptr;
    name_hash = 0;
    bf_read   = nullptr;
  }

  i32 dest;
  i8* name;
  u32 name_hash;
  s_bf_read* bf_read;
};

class c_dispatch_user_message_feature_settings{
public:
  union{
    struct{
      bool disable_motd     = true;
      bool no_fade          = false;
      bool no_shake         = false;
      bool no_rumble        = false;
      bool no_angle_snap    = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_base_dispatch_user_message_features{
public:

  virtual c_dispatch_user_message_feature_settings* get_settings(){
    FUNCTION_OVERRIDE_NEEDED;
    return nullptr;
  }

  virtual void create_menu_settings(gcs_component_group* group, c_dispatch_user_message_feature_settings* settings){
    assert(settings != nullptr && "dispatch_user_message features are dead.");
    assert(group != nullptr);
    if(settings == nullptr)
      return;

    gcs::toggle(group, WXOR(L"Disable MOTD popups"), &settings->disable_motd);
    gcs::toggle(group, WXOR(L"No fade"), &settings->no_fade);
    gcs::toggle(group, WXOR(L"No shake"), &settings->no_shake);
    gcs::toggle(group, WXOR(L"No rumble"), &settings->no_rumble);
    gcs::toggle(group, WXOR(L"No angle-snap"), &settings->no_angle_snap);
  }

  virtual i8* get_msg_name(void* usermsg, i32 msg_type){
    assert(usermsg != nullptr);

    #if defined(__x86_64__)
      void* beast_list = *(uptr*)((uptr)usermsg + (uptr)8) + (uptr)32 * msg_type;
      if(beast_list == nullptr)
        return nullptr;

      assert(beast_list != nullptr);

      i8* name = *(i8**)((uptr)beast_list + (uptr)0x10);
      return name;
    #else
      assert(false && "Remove me to begin testing this code.");
      // This has not been tested.
      void* beast_list = *(uptr*)((uptr)usermsg + (uptr)4) + (uptr)24 * msg_type;
      if(beast_list == nullptr)
        return nullptr;

      i8* name = *(i8**)((uptr)beast_list+ (uptr)0x10);
      return name;
    #endif
  }

  virtual bool dispatch_user_message(void* usermsg, i32 msg_type, s_bf_read* bf_read);

  virtual bool handle_text_msg_features(s_text_msg* msg);
};