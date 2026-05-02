#pragma once

class c_freecam_settings{
public:
  union{
    struct{
      bool enabled             = false;
      i32  speed               = 5;
      u8   key                 = VK_XBUTTON2;
      bool key_held            = true;
      bool key_toggle          = false;
      bool key_double_click    = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_base_free_camera{
public:
  c_key_control key;
  bool pos_setup;
  vec3 pos;
  vec3 freeze_angle;

  virtual void create_menu_settings(gcs_component_group* group, c_freecam_settings* settings){
    assert(group != nullptr);
    assert(settings != nullptr)
    if(settings == nullptr)
      return;

    gcs::toggle(group, WXOR(L"Free camera"), &settings->enabled)
      ->add_module<gcs_component_hotkey>()
      ->set_hotkey_ptr(&settings->key)
      ->set_render_on(&settings->enabled);
    {
      gcs::slider(group, WXOR(L"Speed"), L"", 5, 15, GCS_SLIDER_TYPE_INT, &settings->speed)
        ->set_render_on(&settings->enabled);

      gcs::dropdown(group, WXOR(L"Key control"), false)
        ->add_option(WXOR(L"Held"), &settings->key_held)
        ->add_option(WXOR(L"Toggle"), &settings->key_toggle)
        ->add_option(WXOR(L"Double click"), &settings->key_double_click)
        ->set_render_on(&settings->enabled);
    }
  }

  void reset(){
    pos_setup    = false;
    pos          = vec3();
    freeze_angle = vec3();
    memset(&key, 0, sizeof(c_key_control));
  }

  virtual c_freecam_settings* get_settings(){
    FUNCTION_OVERRIDE_NEEDED;
    return nullptr;
  }

  virtual bool get_shoot_pos(vec3& _pos){
    c_internal_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return false;

    _pos = localplayer->shoot_pos();
    if(!is_active() || !pos_setup)
      return true;

    _pos = pos;
    return true;
  }

  virtual bool is_active(){
    c_freecam_settings* settings = get_settings();
    if(settings == nullptr)
      return false;

    if(!settings->enabled)
      return false;

    if(!key.is_valid())
      key = c_key_control(&settings->key, &settings->key_held, &settings->key_toggle, &settings->key_double_click);

    return key.is_toggled();
  }

  virtual float get_freecam_speed(){
    c_freecam_settings* settings = get_settings();
    if(settings == nullptr)
      return 0.f;

    return (float)settings->speed;
  }

  virtual float get_frame_time(){
    assert(global_internal != nullptr);
    assert(global_internal->global_data != nullptr);
    return global_internal->global_data->frame_time;
  }

  virtual void on_free_camera_begin(){

  }

  virtual void on_free_camera_end(){

  }

  virtual bool render_view(s_view_setup* view_setup, i32* what_to_draw);
  virtual bool pre_create_move_prediction(c_user_cmd* cmd);
};