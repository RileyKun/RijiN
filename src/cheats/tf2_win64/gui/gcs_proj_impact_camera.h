#pragma once

#include "settings.h"

class gcs_proj_impact_camera_panel : public gcs_scene{
public:
  vec3i window_size = vec3i(200, 128);

  bool setup() override {
    vec3i menu_pos = vec3i(4 + (window_size.x / 2), render->screen_size.y / 2);

    render_cam->set_camera_size(window_size);
    gcs_component_window* window = gcs::window(this, WXOR(L"Projectile Impact Camera"), menu_pos, render->rescale(window_size));
    {
      assert(window != nullptr);
      window->set_pos_ptr(&config->fake_latency_panel_pos);

      if(config->fake_latency_panel_pos == vec3i())
        config->fake_latency_panel_pos = menu_pos - (window_size / 2);

      window->add_hook(GCS_HOOK_SHOULD_ENABLE, [](gcs_component_base* c, void* p){
        if(!config->visual.projectile_trajectory_visualizer || !config->visual.projectile_trajectory_impact_camera)
          return false;

        if(global->menu_open)
          return true;

        if(!render_cam->should_render_camera())
          return false;

        return true;
      });

      //window->add_hook(GCS_HOOK_SHOULD_PAINT, [](gcs_component_base* c, void* p){
      //  return true;
      //});

      window->add_hook(GCS_HOOK_INPUT_ENABLED, [](gcs_component_base* c, void* p){
        return global->menu_open;
      });

      window->add_hook(GCS_HOOK_POST_PAINT, [](gcs_component_base* c, void* p){
        if(!utils::is_in_game() && !global->menu_open)
          return true;

        render_cam->set_screen_position(*c->info()->pos_ptr);
        return true;
      });

    }

    return true;
  }
};