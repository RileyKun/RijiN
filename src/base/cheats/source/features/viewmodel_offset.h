#pragma once

class c_viewmodel_offset_settings{
public:
  union{
    struct{
      bool offset_enabled = false;
      i32  offset_x       = 0;
      i32  offset_y       = 0;
      i32  offset_z       = 0;

      i32  pitch          = 0;
      i32  yaw            = 0;
      i32  roll           = 0;

      bool  sway_enabled   = false;
      float sway_amount   = 1.5f;

      bool silent_aim     = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};


class c_base_viewmodel_offset{
public:

  virtual void create_menu_settings(gcs_component_group* group, c_viewmodel_offset_settings* settings, bool add_weapon_sway = true){
    assert(group != nullptr);
    assert(settings != nullptr);
    gcs::toggle(group, WXOR(L"Offset"), &settings->offset_enabled);
    {
      gcs::slider(group, WXOR(L"Forward"), WXOR(L"hu"), -60, 60, GCS_SLIDER_TYPE_INT, &settings->offset_x)
        ->set_render_on(&settings->offset_enabled);
      gcs::slider(group, WXOR(L"Right"), WXOR(L"hu"), -60, 60, GCS_SLIDER_TYPE_INT, &settings->offset_y)
        ->set_render_on(&settings->offset_enabled);
      gcs::slider(group, WXOR(L"Up"), WXOR(L"hu"), -60, 60, GCS_SLIDER_TYPE_INT, &settings->offset_z)
        ->set_render_on(&settings->offset_enabled);

      gcs::slider(group, WXOR(L"Pitch"), WXOR(L"°"), -89, 89, GCS_SLIDER_TYPE_INT, &settings->pitch)
        ->set_render_on(&settings->offset_enabled);
      gcs::slider(group, WXOR(L"Yaw"), WXOR(L"°"), -180, 180, GCS_SLIDER_TYPE_INT, &settings->yaw)
        ->set_render_on(&settings->offset_enabled);
      gcs::slider(group, WXOR(L"Roll"), WXOR(L"°"), -180, 180, GCS_SLIDER_TYPE_INT, &settings->roll)
        ->set_render_on(&settings->offset_enabled);
    }

    gcs::add_space(group, &settings->offset_enabled);

    if(add_weapon_sway){
      gcs::toggle(group, WXOR(L"Weapon sway"), &settings->sway_enabled);
      {
        gcs::slider(group, WXOR(L"Scale"), L"", 1, 10, GCS_SLIDER_TYPE_FLOAT, &settings->sway_amount)
          ->set_render_on(&settings->sway_enabled);
      }

      gcs::add_space(group, &settings->sway_enabled);
    }

    gcs::toggle(group, WXOR(L"Show silent aim"), &settings->silent_aim);
  }

  virtual c_viewmodel_offset_settings* get_settings(u32 slot = 0){ // Slot is optional, incase you want unique settings per weapon.
    FUNCTION_OVERRIDE_NEEDED;
    return nullptr;
  }

  virtual void reset(){
    reset_sway_data();
  }

  virtual void reset_sway_data(){
    if(!reset_sway)
      return;

    #if !defined(GMOD_CHEAT)
    global_internal->cl_wpn_sway_interp->flt = old_cl_wpn_sway_interp;
    global_internal->cl_wpn_sway_scale->flt  = old_cl_wpn_sway_scale;
    #else
    assert(false && "no gmod support 2");
    #endif
    reset_sway = false;
  }

  virtual void calc_view_model(c_internal_base_entity* viewmodel, vec3& eye_pos, vec3& angles);
  virtual void post_calc_view_model(c_internal_base_entity* viewmodel, c_internal_base_player* owner, vec3& eye_pos, vec3& angles);
  virtual void pre_calc_view_model_lag(c_internal_base_entity* viewmodel, vec3& origin, vec3& angles);
  virtual void post_calc_view_model_lag(c_internal_base_entity* viewmodel, vec3& origin, vec3& angles);

  float old_cl_wpn_sway_interp;
  float old_cl_wpn_sway_scale;
  bool  reset_sway;
};