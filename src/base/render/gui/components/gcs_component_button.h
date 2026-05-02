#pragma once

class gcs_component_button : public gcs_component_base{
  void*   param               = nullptr;
  bool    has_override_colour = false;
  colour  override_colour;
public:

  void setup() override{
  }

  void think(vec3i pos, vec3i size) override{
  }

  void pre_run() override{
  }

  void post_run() override{
  }

  void set_param(void* p){
    param = p;
  }

  void set_colour_override(colour col){
    has_override_colour = true;
    override_colour     = col;
  }

  void remove_colour_override(){
    has_override_colour = false;
    override_colour     = rgb(0, 0, 0);
  }

  void draw(vec3i pos, vec3i size){
    gcs_colour_scheme* _scheme = scheme();
    gcs_scene_info* _scene = scene()->info();

    colour draw_col;

    if(has_override_colour)
      draw_col = override_colour;
    else
      draw_col = is_in_rect() ? _scheme->main * 1.20f : _scheme->main;

    render->outlined_rect(pos, size, draw_col);
    draw_col.a = 255.f * 0.5f;
    render->filled_rect(pos, size, draw_col);

    u32 font_size = render->mode() != RENDER_MODE_D2D1 ? 16 : size.y / 2;

#if defined(INTERNAL_CHEAT)
    if(render->mode() == RENDER_MODE_D2D1)
      font_size += 8;
#endif

    _scene->font->draw(pos + (size / 2), font_size, name(), _scheme->text, TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_CENTER);

    if(is_in_rect() && input() & GCS_IN_M1_CLICK){
      call_hook(GCS_HOOK_CLICK, param);

      emit_click_sound(1);
    }

    render->push_clip_region(vec3i(), vec3i());
    render->pop_clip_region();
  }
};