#pragma once

class c_third_person_settings{
public:
  union{
    struct{
    bool thirdperson          = false;
    i32  thirdperson_distance = 150;
    u8   thirdperson_key        = VK_F5;
    bool thirdperson_usekey     = false;
    bool thirdperson_key_held   = true;
    bool thirdperson_key_toggle = false;
    bool thirdperson_key_double_click     = false;

    bool thirdperson_right_offset_enabled             = false;
    bool thirdperson_right_offset_use_key             = false;
    i32  thirdperson_right_offset                     = 0;
    u8   thirdperson_right_offset_key                 = VK_XBUTTON2;
    u8   thirdperson_right_offset_invert_key          = 0x56;
    bool thirdperson_right_offset_invert_held         = false;
    bool thirdperson_right_offset_invert_toggle       = false;
    bool thirdperson_right_offset_invert_double_click = true;

    bool thirdperson_right_offset_held         = true;
    bool thirdperson_right_offset_toggle       = false;
    bool thirdperson_right_offset_double_click = false;

    i32  thirdperson_up_offset                        = 0;
    bool thirdperson_collision  = true;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_base_third_person{
public:

  virtual void create_menu_settings(gcs_component_group* group, c_third_person_settings* settings){
    assert(settings != nullptr && "Thirdperson is fucked");
    assert(group != nullptr);
    if(settings == nullptr)
      return;

    gcs::toggle(group, WXOR(L"Thirdperson"), &settings->thirdperson)
      ->add_module<gcs_component_hotkey>()
      ->set_hotkey_ptr(&settings->thirdperson_key)
      ->set_render_on(&settings->thirdperson)
      ->set_render_on(&settings->thirdperson_usekey);
    {
      gcs::toggle(group, WXOR(L"Camera collision"), &settings->thirdperson_collision)
        ->set_render_on(&settings->thirdperson);

      gcs::toggle(group, WXOR(L"Use key"), &settings->thirdperson_usekey)
        ->set_render_on(&settings->thirdperson);

      gcs::dropdown(group, WXOR(L"Key control"), false)
        ->add_option(WXOR(L"Held"), &settings->thirdperson_key_held)
        ->add_option(WXOR(L"Toggle"), &settings->thirdperson_key_toggle)
        ->add_option(WXOR(L"Double click"), &settings->thirdperson_key_double_click)
        ->set_render_on(&settings->thirdperson)
        ->set_render_on(&settings->thirdperson_usekey);

      gcs::slider(group, WXOR(L"Max distance"), WXOR(L"hu"), 30, 400, GCS_SLIDER_TYPE_INT, &settings->thirdperson_distance)
        ->set_render_on(&settings->thirdperson);

      gcs::slider(group, WXOR(L"Up offset"), WXOR(L"hu"), -64, 64, GCS_SLIDER_TYPE_INT, &settings->thirdperson_up_offset)
        ->set_render_on(&settings->thirdperson);

      gcs::toggle(group, WXOR(L"Right offset"), &settings->thirdperson_right_offset_enabled)
        ->set_render_on(&settings->thirdperson)
        ->add_module<gcs_component_hotkey>()
        ->set_hotkey_ptr(&settings->thirdperson_right_offset_key)
        ->set_render_on(&settings->thirdperson_right_offset_enabled)
        ->set_render_on(&settings->thirdperson_right_offset_use_key);

      gcs::toggle(group, WXOR(L"Use key"), &settings->thirdperson_right_offset_use_key)
        ->set_render_on(&settings->thirdperson)
        ->set_render_on(&settings->thirdperson_right_offset_enabled);

        gcs::dropdown(group, WXOR(L"Key control"), false)
        ->add_option(WXOR(L"Held"), &settings->thirdperson_right_offset_held)
        ->add_option(WXOR(L"Toggle"), &settings->thirdperson_right_offset_toggle)
        ->add_option(WXOR(L"Double click"), &settings->thirdperson_right_offset_double_click)
        ->set_render_on(&settings->thirdperson)
        ->set_render_on(&settings->thirdperson_right_offset_enabled);

      gcs::label(group, WXOR(L"Invert offset key"))
        ->set_render_on(&settings->thirdperson)
        ->set_render_on(&settings->thirdperson_right_offset_enabled)
          ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&settings->thirdperson_right_offset_invert_key);

      gcs::dropdown(group, WXOR(L"Invert key control"), false)
        ->add_option(WXOR(L"Held"), &settings->thirdperson_right_offset_invert_held)
        ->add_option(WXOR(L"Toggle"), &settings->thirdperson_right_offset_invert_toggle)
        ->add_option(WXOR(L"Double click"), &settings->thirdperson_right_offset_invert_double_click)
        ->set_render_on(&settings->thirdperson)
        ->set_render_on(&settings->thirdperson_right_offset_enabled);

      gcs::slider(group, WXOR(L"Offset"), WXOR(L"hu"), -64, 64, GCS_SLIDER_TYPE_INT, &settings->thirdperson_right_offset)
        ->set_render_on(&settings->thirdperson)
        ->set_render_on(&settings->thirdperson_right_offset_enabled);
    }
  }

  virtual c_third_person_settings* get_settings(){
    FUNCTION_OVERRIDE_NEEDED;
    return nullptr;
  }

  virtual void reset(){
    cam_desired_right    = 0.f;
    cam_desired_up       = 0.f;
    cam_desired_distance = 0.f;

    thirdperson_toggle  = false;
    is_view_thirdperson = false;

    memset(&key, 0, sizeof(c_key_control));
    memset(&right_master_key, 0, sizeof(c_key_control));
    memset(&invert_key, 0, sizeof(c_key_control));
  }

  virtual bool is_view_third_person(s_view_setup* view, c_internal_base_player* localplayer);
  virtual void calculate_view_state(s_view_setup* view_setup);
  virtual void render_view(s_view_setup* view_setup, i32* what_to_draw);

  virtual bool is_in_thirdperson_view(){
    return is_view_thirdperson;
  }

  virtual bool is_active(){
    return thirdperson_toggle;
  }

private:
  float cam_desired_right    = 0.f;
  float cam_desired_up       = 0.f;
  float cam_desired_distance = 0.f;

  c_key_control key;
  c_key_control right_master_key;
  c_key_control invert_key;

  bool thirdperson_toggle;
  bool is_view_thirdperson;
};