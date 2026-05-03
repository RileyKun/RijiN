#pragma once

class c_anti_screengrab{
public:
  bool         is_screen_grab_taking_place = false;
  bool         is_rendering_new_view       = false;
  c_texture*   view_texture                = nullptr;
  c_material*  view_material               = nullptr;

  c_material*  read_pixels_material        = nullptr;
  c_texture*   read_pixels_texture         = nullptr;

  bool         has_pushed                  = false;
  float        last_screenshot_taken_time  = 0.f;
  float        last_update_time            = 0.f;
  vec3         last_knowledged_origin;
  vec3         last_viewangle;
  u32          blacklisted_render_targets[128];
  u32          whitelisted_render_targets[128];
  u32          whitelisted_cycle                = 0;
  u32          blacklisted_cycle                = 0;
  c_texture*   original_render_texture          = nullptr;
  float        original_render_texture_set_time = 0.f;
  float        last_video_add_frame_time        = 0.f;
  bool         video_adding_frame               = false;
  bool         calcuating_viewport              = false;

  ALWAYSINLINE void on_video_add_frame_start(){
    video_adding_frame = true;
    float time = math::time();
    if(last_video_add_frame_time <= time)
      notify->create(NOTIFY_TYPE_WARNING, WXOR(L"A script is capturing your screen with video.addframe"));

    global->info_panel->add_entry(INFO_PANEL_ANTISCREENGRAB, WXOR(L"ANTI-SCREENGRAB"), WXOR(L"IVIDEOWRITER METHOD"), globaldata->tick_count % 66 <= 33 ? INFO_PANEL_RAGE_CLR : INFO_PANEL_WARNING_CLR);
    last_video_add_frame_time = time + 1.f;
  }

  ALWAYSINLINE void on_video_add_frame_finish(){
    video_adding_frame = false;
  }


  void init_materials();
  void check_game_screenshot_states();
  bool is_screenshot_taken();
  bool is_creating_spoofed_view();
  void on_screen_grab_start();
  void on_screen_grab_end();
  void on_screen_recording_start();
  void on_screen_recording_end();
  void on_render_target_override(c_texture* texture);
  bool should_update_frame();
  bool should_override_set_render_target(c_texture* texture);
  bool is_rijin_texture(c_texture* texture);

  void create_spoofed_view(void* rcx, s_view_setup* view, i32 flags, i32 what_to_draw);

  void push();
  void pop();

  void set_original_texture(c_texture* texture){
    assert(is_rijin_texture(texture) == false); // It's not supposed to be a RijiN texture.
    original_render_texture          = texture;
    original_render_texture_set_time = math::time() + 1.f;
  }

  c_texture* get_original_texture(){
    // The game SHOULD never de-alloc a texture from memory that fast. But just incase this game's retarded we'll keep this alive for 10 seconds.
    if(original_render_texture_set_time + 10.f <= math::time()){
      original_render_texture = nullptr;
      return nullptr;
    }

    return original_render_texture;
  }

  void debug_draw(){
    #if defined(DEV_MODE)
      if(view_material == nullptr || view_texture == nullptr)
        return;
      
      c_render_context* rc = global->material_system->get_render_context();
      if(rc == nullptr)
        return;

      rc->draw_screen_space_rectangle(view_material, 0, 0, 800, 600, 0, 0, 800.f, 600.f, view_texture->get_actual_width(), view_texture->get_actual_height(), nullptr, 1, 1);
    #endif
  }
};

CLASS_EXTERN(c_anti_screengrab, anti_screengrab);