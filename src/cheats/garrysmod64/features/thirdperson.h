#pragma once

class c_thirdperson{
public:

  ALWAYSINLINE void reset(){
    memset(this, 0, sizeof(c_thirdperson));
  }

  void calculate_view_status(s_view_setup* view){
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    is_view_from_prop_camera = false;
    s_trace tr = global->trace->ray(view->origin, view->origin, mask_shot);
    if(tr.hit_entity != nullptr){
      i8* name = tr.hit_entity->get_scripted_class_name();
      if(name != nullptr){
        u32 hash = HASH_RT(name);
        is_view_from_prop_camera = (hash == HASH("gmod_cameraprop"));
        if(!is_view_from_prop_camera){
          DBG("[!] calculate_view_status entity found: %s\n", name);
        }     
      }
    }

    float view_dist = (localplayer->shoot_pos() - view->origin).length();
    is_view_thirdperson = (view_dist > 24.f);
  }

  bool is_in_thirdperson_view(){
    return is_view_thirdperson;
  }

  bool is_in_view_from_camera(){
    return is_view_from_prop_camera;
  }

  bool is_active(){
    return thirdperson_toggle;
  }

  void run(s_view_setup* view_setup, i32* what_to_draw){
    if(utils::is_screen_shot_taken() || acp->is_blocked(FUNC_SCREEN_GRAB))
      return;
    
    if(is_view_thirdperson)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(localplayer->player_vehicle > 0)
      return;

    if(!localplayer->is_alive()){
      thirdperson_toggle = false;
      return;
    }

    // Toggle thirdperson!
    if(config->visual.thirdperson_usekey){
      if(!key.is_valid())
        key = c_key_control(&config->visual.thirdperson_key, &config->visual.thirdperson_key_held, &config->visual.thirdperson_key_toggle, &config->visual.thirdperson_key_double_click);

      thirdperson_toggle = key.is_toggled();

      if(!config->visual.thirdperson)
        thirdperson_toggle = false;
    }
    else
      thirdperson_toggle = config->visual.thirdperson;

 
    float right_offset = math::clamp((float)config->visual.thirdperson_right_offset, -64.f, 64.f);
    if(config->visual.thirdperson_right_offset_enabled){
      bool apply_offset = true;

      if(!right_master_key.is_valid())
        right_master_key = c_key_control(&config->visual.thirdperson_right_offset_key, &config->visual.thirdperson_right_offset_held, &config->visual.thirdperson_right_offset_toggle, &config->visual.thirdperson_right_offset_double_click);

      if(config->visual.thirdperson_right_offset_use_key && !right_master_key.is_toggled())
        apply_offset = false;

      // Fix any config errors.
      if(!config->visual.thirdperson_right_offset_invert_held && !config->visual.thirdperson_right_offset_invert_toggle && !config->visual.thirdperson_right_offset_invert_double_click)
        config->visual.thirdperson_right_offset_invert_held = true;
      else if(config->visual.thirdperson_right_offset_invert_held && config->visual.thirdperson_right_offset_invert_toggle && config->visual.thirdperson_right_offset_invert_double_click){
        config->visual.thirdperson_right_offset_invert_held   = true;
        config->visual.thirdperson_right_offset_invert_toggle = config->visual.thirdperson_right_offset_invert_double_click = false;
      }

      if(!invert_key.is_valid())
      invert_key = c_key_control(&config->visual.thirdperson_right_offset_invert_key, &config->visual.thirdperson_right_offset_invert_held, &config->visual.thirdperson_right_offset_invert_toggle, &config->visual.thirdperson_right_offset_invert_double_click);

      if(apply_offset){
        if(invert_key.is_toggled())
          right_offset *= -1.f;

        cam_desired_right = right_offset;
      }
      else
        cam_desired_right = 0.f;
    }
    else
      cam_desired_right = 0.f;

    cam_desired_up       = math::clamp((float)config->visual.thirdperson_up_offset, -64.f, 64.f);
    cam_desired_distance = math::biggest((float)config->visual.thirdperson_distance, 30.f);

    if(!thirdperson_toggle)
      return;
    
    if(what_to_draw != nullptr){
      if(*what_to_draw & RENDERVIEW_DRAWVIEWMODEL)
        *what_to_draw &= ~RENDERVIEW_DRAWVIEWMODEL;
    }

    vec3 angles = global->engine->get_view_angles();

    vec3 fwd, right, up;
    math::angle_2_vector(angles, &fwd, &right, &up);

    view_setup->origin += (fwd * -cam_desired_distance);
    view_setup->origin += (right * cam_desired_right);
    view_setup->origin += (up * cam_desired_up);
    global->info_panel->add_entry(INFO_PANEL_THIRDPERSON, WXOR(L"THIRDPERSON"), WXOR(L"RUNNING"), INFO_PANEL_LEGIT_CLR);
  }

  float cam_desired_right    = 0.f;
  float cam_desired_up       = 0.f;
  float cam_desired_distance = 0.f;

  c_key_control key;
  c_key_control right_master_key;
  c_key_control invert_key;

  bool thirdperson_toggle;

  bool is_view_thirdperson;
  bool is_view_from_prop_camera;
};

CLASS_EXTERN(c_thirdperson, thirdperson);