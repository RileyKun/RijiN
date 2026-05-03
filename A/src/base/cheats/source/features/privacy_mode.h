#pragma once

class c_misc_privacy_mode_settings{
public:
  union{
    struct{
      bool enabled                  = false;
      bool censor_local_player      = true;
      bool censor_friends           = true;
      bool friends_use_nick_names   = true;
      bool censor_public            = false;
      bool hide_revealing_ui_panels = true;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_base_privacy_mode{
public:
  // Literally do the first member of c_misc_privacy_mode_settings &settings->misc;
  virtual c_misc_privacy_mode_settings* get_settings(){
    FUNCTION_OVERRIDE_NEEDED;
    return nullptr;
  }

  virtual void create_menu_settings(gcs_component_group* group, c_misc_privacy_mode_settings* settings){
    assert(group != nullptr);
    assert(settings != nullptr && "Privacy settings fucked!");
    gcs::toggle(group, WXOR(L"Privacy mode"), &settings->enabled);
    gcs::dropdown(group, WXOR(L"Censoring"), true)
      ->add_option(WXOR(L"Hide localplayer"), &settings->censor_local_player)
      ->add_option(WXOR(L"Hide friends"), &settings->censor_friends)
      ->add_option(WXOR(L"Hide public"), &settings->censor_public)
      ->set_render_on(&settings->enabled);

    gcs::toggle(group, WXOR(L"Use steam nickname"), &settings->friends_use_nick_names)
      ->set_help_text(WXOR(L"Use the set nickname for your friends if possible."))
        ->set_render_on(&settings->enabled);

    gcs::toggle(group, WXOR(L"Hide Revealing UI panels"), &settings->hide_revealing_ui_panels)
      ->set_render_on(&settings->enabled);

  }

  virtual bool engine_get_player_info_hook(i32 entity_id, s_player_info* info, bool sid_write_allowed = true);
  virtual bool paint_traverse_hook(uptr panel);
  virtual bool handle_game_event(c_game_event* event);
  virtual bool dispatch_user_message(u32 name_hash, i32 msg_type, s_bf_read* bf_read);
  virtual bool should_censor_chat(i32 index);

  virtual bool is_enabled(){
    c_misc_privacy_mode_settings* settings = get_settings();
    if(settings == nullptr)
      return false;

    return settings->enabled;
  }
};