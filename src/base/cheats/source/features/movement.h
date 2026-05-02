#pragma once

class c_movement_settings{
public:
  union{
    struct{
      bool bunnyhop                     = false;
      bool auto_strafe                  = false;
      bool auto_strafe_pre_speed        = false;
      bool auto_strafe_type_normal      = false;
      bool auto_strafe_type_directional = true;
      float auto_strafe_retrack               = 75.f;
      bool auto_strafe_avoid_walls_disabled   = true;
      bool auto_strafe_avoid_walls_normal     = false;
      bool auto_strafe_avoid_walls_adv        = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_auto_strafer_data{
public:
  vec3  velocity;
  float original_yaw;
  float velocity_angle;
  float speed;
  float retrack;
  float max_retrack;
  float ideal_max;
  float ideal2_max;
  float ideal;
  float ideal_turn;

  ALWAYSINLINE void reset(){
    memset(this, 0, sizeof(c_auto_strafer_data));
  }

  ALWAYSINLINE void init(c_internal_base_player* player, c_user_cmd* cmd){
    reset();
    if(player == nullptr)
      return;

    original_yaw   = cmd->view_angles.y;
    velocity       = player->velocity();
    velocity_angle = math::rad_2_deg(math::atan2(velocity.y, velocity.x));
    speed          = velocity.length_2d();
    retrack        = 15.f;
    max_retrack    = 90.f;
    ideal_max      = speed > 0.f ? math::clamp(math::rad_2_deg(math::asin(15.f / speed)), 0.f, 90.f) : 0.f;
    ideal2_max     = speed > 0.f ? math::clamp(math::rad_2_deg(math::asin(30.f / speed)), 0.f, 90.f) : 0.f;
    ideal          = ideal_max;
    ideal_turn     = ideal2_max;
  }
};

class c_base_movement{
public:
  float mod;
  float old_yaw;
  float vel_yaw;

  virtual void create_menu_settings(gcs_component_group* group, c_movement_settings* settings){
    assert(settings != nullptr && "movement settings are fucked");
    assert(group != nullptr);
    if(settings == nullptr)
      return;

    gcs::toggle(group, WXOR(L"Bunnyhop"), &settings->bunnyhop);
    gcs::add_space(group);

    gcs::toggle(group, WXOR(L"Auto-strafe"), &settings->auto_strafe);
    {
      gcs::toggle(group, WXOR(L"Prespeed"), &settings->auto_strafe_pre_speed)
        ->set_render_on(&settings->auto_strafe);

      gcs::slider(group, WXOR(L"Retrack speed"), WXOR(L"%"), 1, 100, GCS_SLIDER_TYPE_FLOAT, &settings->auto_strafe_retrack)
       ->set_render_on(&settings->auto_strafe);

      gcs::dropdown(group, WXOR(L"Strafe type"), false)
        ->add_option(WXOR(L"Normal"), &settings->auto_strafe_type_normal)
        ->add_option(WXOR(L"Directional (WASD)"), &settings->auto_strafe_type_directional)
          ->set_render_on(&settings->auto_strafe);

      gcs::dropdown(group, WXOR(L"Avoid walls"), false)
        ->add_option(WXOR(L"Disabled"), &settings->auto_strafe_avoid_walls_disabled)
        ->add_option(WXOR(L"Simple"), &settings->auto_strafe_avoid_walls_normal)
        ->add_option(WXOR(L"Advanced"), &settings->auto_strafe_avoid_walls_adv)
          ->set_render_on(&settings->auto_strafe);
    }
  }

  virtual c_movement_settings* get_settings(){
    FUNCTION_OVERRIDE_NEEDED;
    return nullptr;
  }

  virtual bool pre_predicted_create_move(c_internal_base_player* localplayer, c_internal_base_weapon* wep, c_user_cmd* cmd);
  virtual bool can_perform_movement(c_internal_base_player* player, bool skip_button_check = false);

  // bunny hop
  virtual bool can_perform_double_jump(c_internal_base_player* localplayer);
  virtual void bunnyhop(c_internal_base_player* localplayer, c_user_cmd* cmd);

  // auto strafer
  virtual void auto_strafe_init_base(){
     mod = (mod == 0.f) ? 1.f : (mod * -1.f);
  }

  virtual void auto_strafe(c_internal_base_player* localplayer, c_user_cmd* cmd);
  virtual void auto_strafe_pre_speed(c_internal_base_player* localplayer, c_user_cmd* cmd);
  virtual void auto_strafe_directional_smoothing(c_internal_base_player* localplayer, c_user_cmd* cmd, c_auto_strafer_data* data);

  virtual void auto_strafe_avoid_walls(c_internal_base_player* localplayer, c_user_cmd* cmd, c_auto_strafer_data* data);

  virtual bool will_hit_obstacle_in_future(float wish_yaw, float current_yaw, float predict_time, float step, vec3* hit_plane = nullptr);
  virtual bool should_avoid_wall(float looking_yaw, float wish_yaw, float current_yaw, float predict_time, float& new_yaw, float ideal);
};