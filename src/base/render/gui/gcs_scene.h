#pragma once

class gcs_component_base;

enum gcs_input{
  GCS_IN_M1             = 1 << 1,
  GCS_IN_M1_CLICK       = 1 << 2,
  GCS_IN_INSERT         = 1 << 3,
  GCS_IN_INSERT_CLICK   = 1 << 4,
  GCS_IN_F3             = 1 << 5,
  GCS_IN_F3_CLICK       = 1 << 6,
  GCS_IN_M2             = 1 << 7,
  GCS_IN_M2_CLICK       = 1 << 8,
};

struct gcs_scene_info{

  // core information (such as fonts and mouse positions)
  c_font_base*    font                = nullptr;
  c_font_base*    font_emoji          = nullptr;
  vec3i           mouse_pos;

  // performance related
  bool   perf_enabled                   = false;
  float  frametime                      = 0.f;
  u32    framecount                     = 0;

  // other
  float  curtime                        = 0.f;

  // keyboard/mouse input related
  bool   last_input_frozen              = false;
  bool   input_frozen                   = false;
  u32    input                          = 0;

  // DPI Scaling
  bool  dpi_scale_enabled               = true;
  float dpi_scale                       = 1.5f;

  // This can be used to ensure a component is drawn last
  // This should be cleared when its not used
  gcs_component_base*   final_draw_comp = nullptr;

  // We received a help text entry that wants to be rendered. Done this way so we don't have to do any clip hacks.
  std::wstring help_text;

  ALWAYSINLINE void set_final_draw(gcs_component_base* comp){
    final_draw_comp = comp;
  }

  ALWAYSINLINE void update_input(){
    u32 old_input = input;
    input         = 0;

    i16 left_button = I(GetAsyncKeyState)(VK_LBUTTON);
    {
      if(left_button < 0 && !(old_input & GCS_IN_M1) )
        input |= GCS_IN_M1_CLICK;

      if(left_button < 0)
        input |= GCS_IN_M1;
    }

    i16 right_button = I(GetAsyncKeyState)(VK_RBUTTON);
    {
      if(right_button < 0 && !(old_input & GCS_IN_M2) )
        input |= GCS_IN_M2_CLICK;

      if(right_button < 0)
        input |= GCS_IN_M2;
    }

    i16 insert_key = I(GetAsyncKeyState)(VK_INSERT);
    {
      if(insert_key < 0 && !(old_input & GCS_IN_INSERT) )
        input |= GCS_IN_INSERT_CLICK;

      if(insert_key < 0)
        input |= GCS_IN_INSERT;
    }

    #if !defined(GMOD)
    i16 f3_key = I(GetAsyncKeyState)(VK_F3);
    {
      if(f3_key < 0 && !(old_input & GCS_IN_F3) )
        input |= GCS_IN_F3_CLICK;

      if(f3_key < 0)
        input |= GCS_IN_F3;
    }
    #endif
  }

  ALWAYSINLINE void clear_input(){
    input = 0;
  }

  gcs_component_base* root_component  = nullptr;
};

struct gcs_colour_scheme{
  colour main                      = rgb(25, 118, 210);
  colour background                = rgb(41,44,51);
  colour grey1                     = rgb(60, 60, 60);
  colour grey2                     = rgb(40, 40, 40);
  colour grey3                     = rgb(70,70,70);
  colour text                      = rgb(230,230,230);
  colour success                   = rgb(37, 161, 29);
  colour error                     = rgb(231, 76, 60);
  colour warning                   = rgb(241, 196, 15);
  bool   disable_window_background = false;
};

class gcs_scene{
private:
  gcs_scene_info    scene_info;
  gcs_colour_scheme colour_scheme;
  float             scale;
public:

  float get_scale(){
    return scale;
  }

  void set_scale(float new_scale){
    scale = new_scale;
  }

  virtual bool setup(){
    return false;
  }

  virtual bool think(){
    return false;
  }

  ALWAYSINLINE bool valid(){
    // unhandled
    return info()->root_component != nullptr;
  }

  ALWAYSINLINE gcs_colour_scheme* scheme(){
    return &colour_scheme;
  }

  ALWAYSINLINE gcs_scene_info* info(){
    return &scene_info;
  }

  template<typename T>
  T* add(){
    gcs_component_base* comp = gcs::alloc_memory<T>();

    comp->setup();

    return (T*)(info()->root_component = comp);
  }


  ALWAYSINLINE void draw_help_text(){
    if(info()->help_text.empty())
      return;

    if(info()->font == nullptr)
      return;
    
    u32   font_size = 14;
    vec3i size = info()->font->get_size(font_size, info()->help_text, nullptr);

    vec3i mpos = info()->mouse_pos;

    // Trying to avoid having the groupbox header draw over the box & text.
    mpos.x += font_size + 4;

    // Draw box outline.
    {
      colour bg = scheme()->background;
      bg.a = 255 * 0.8f;
    
      render->filled_rect(vec3i(mpos.x - 4, (mpos.y - size.y / 2) - 4), vec3i(size.x + 4, size.y + 4), bg);
      render->outlined_rect(vec3i(mpos.x - 4, (mpos.y - size.y / 2) - 4), vec3i(size.x + 4, size.y + 4), scheme()->main * 1.25f);
    }

    // Render text.
    {
      info()->font->draw(mpos - vec3i(1, 1), font_size, info()->help_text, scheme()->text, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);
    }
  }
};