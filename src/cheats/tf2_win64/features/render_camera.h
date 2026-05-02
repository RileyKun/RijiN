#pragma once

// This class should be renamed to impact camera or something.
class c_render_camera{
public:
  c_texture*    camera_texture  = nullptr;
  c_material*   camera_material = nullptr;
  s_view_setup  view;
  vec3i         screen_pos;
  vec3i         camera_size     = vec3i(200, 128);
  vec3          camera_pos;
  vec3          camera_angle;
  vec3          impact_position;
  matrix4x4     view_matrix;
  bool          is_rendering_view;
  void init_material();

  ALWAYSINLINE void reset_position(){
    camera_pos   = vec3();
    camera_angle = vec3();
  }

  ALWAYSINLINE void set_camera_position(vec3 pos){
    camera_pos = pos;
  }

  ALWAYSINLINE void set_camera_angles(vec3 angles){
    camera_angle = angles;
  }

  ALWAYSINLINE void set_screen_position(vec3i pos){
    screen_pos = pos;
  }

  ALWAYSINLINE void set_camera_size(vec3i size){
    camera_size = size;
  }

  ALWAYSINLINE vec3i get_camera_size(){
    return render->rescale(camera_size);
  }

  ALWAYSINLINE void set_impact_position(vec3 pos){
    if(pos == vec3(0.f, 0.f, 0.f))
      return;

    impact_position = pos;
  }

  bool should_render_camera(){
    if(!config->visual.projectile_trajectory_impact_camera)
      return false;

    if(!config->visual.projectile_trajectory_visualizer)
      return false;

    if(!utils::is_in_game())
      return false;

    if(global->engine->is_playing_demo())
      return false;

    if(camera_pos == vec3(0.f, 0.f, 0.f))
      return false;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return false;

    if(!localplayer->is_alive())
      return false;

    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return false;

    if(!wep->is_projectile_weapon() && !wep->is_ball_bat())
      return false;

    if(wep->is_flamethrower() || wep->is_dragons_fury())
      return false;

    #if !defined(DEV_MODE)
      if(wep->is_ball_bat())
        return false;
    #endif

    // Disable camera
    if((localplayer->origin() - camera_pos).length() <= 200.f)
      return false;

    return true;
  }

  void setup_view(s_view_setup* current_view){
    if(!should_render_camera())
      return;

    view = *current_view;

    vec3i size = get_camera_size();

    // Setup our camera view.
    view.origin       = camera_pos;
    view.angles       = camera_angle;
    view.width        = size.x;
    view.height       = size.y;
    view.aspect_ratio = (float)size.x / (float)size.y;
    view.fov          = 90.f;
  }

  void render_view(void* rcx, s_view_setup* current_view){
    if(!should_render_camera())
      return;

    init_material();
    if(camera_texture == nullptr || camera_material == nullptr)
      return;

    setup_view(current_view);

    c_render_context* rc = global->material_system->get_render_context();
    if(rc == nullptr)
      return;

    // Be warned that this function will call DoScreenSpaceEffects.
    is_rendering_view = true;
    rc->push_render_target_and_viewport();
    rc->set_render_target(camera_texture);
    {
      utils::call_fastcall64<void, s_view_setup*, u32, i32>(gen_internal->decrypt_asset(global->crenderview_renderview_hook_trp), rcx, &view, VIEW_CLEAR_COLOR | VIEW_CLEAR_DEPTH | VIEW_CLEAR_STENCIL, 0);
    }
    rc->pop_render_target_and_viewport();
    rc->release();
    is_rendering_view = false;
  }

  // This will draw the camera on the screen.
  void draw(){
    if(!should_render_camera())
      return;

    if(camera_texture == nullptr || camera_material == nullptr)
      return;

    c_render_context* rc = global->material_system->get_render_context();
    if(rc == nullptr)
      return;

    rc->draw_screen_space_rectangle(camera_material, screen_pos.x, screen_pos.y, view.width + 1, view.height + 1, 0, 0, (float)view.width, (float)view.height, camera_texture->get_actual_width(), camera_texture->get_actual_height(), nullptr, 1, 1);
    rc->release();
  }

  // Calculates a view matrix for our camera.
  void update_view_matrix(){
    if(!should_render_camera())
      return;

    global->engine_tool->get_world_to_screen_matrix_for_view(&view, &view_matrix);
  }

  void render_visuals_in_camera(){
    if(!should_render_camera())
      return;

    const float frame_rate = (1.f / 60.f);
    float time             = math::time(false, true);
    static float last_time = time;
    static float yaw_angle = 0.f;
    {
      matrix4x4 old_view_matrix  = global->view_matrix;
      vec3i old_screen_size      = render->screen_size;

      float e_time = math::abs(last_time - time);
      while(e_time > frame_rate){ // Give the 3d box a nice spinning effect.
        yaw_angle += 8.f;
        if(yaw_angle > 180.f)
          yaw_angle -= 360.f;

        e_time -= frame_rate;
        last_time = time;
      }

      // A hacky solution to render a 3dbox with in the camera.
      global->ws2_position_offset  = screen_pos;
      global->view_matrix          = view_matrix;
      render->screen_size          = get_camera_size();

      render->push_clip_region(screen_pos, render->screen_size); // Could this be a cause of the flickering? Who knows.
      render_debug->draw_static_3dbox(impact_position, vec3(-4.f, -4.f, -4.f), vec3(4.f, 4.f, 4.f), vec3(0.f, yaw_angle, 0.f), colour(255, 0, 0, 64), colour(200, 0, 0, 255));
      render->pop_clip_region();

      global->ws2_position_offset  = vec3i(0, 0, 0);
      global->view_matrix          = old_view_matrix;
      render->screen_size          = old_screen_size;
    }
  }
};

CLASS_EXTERN(c_render_camera, render_cam);