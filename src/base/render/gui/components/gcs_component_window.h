#pragma once

class gcs_component_window : public gcs_component_base{
private:
  bool  border            = true;
  bool  draggable         = true;
  bool  clamp_window      = true;
  bool  is_dragging       = false;
  vec3i drag_offset;
  i32   border_top_height = 24;
public:
  bool  wants_icon        = false;
  void setup() override{
    info()->always_run_think = true;
    border_top_height = render->rescale(24);
  }

  //vec3i size() override{
  //  return render->rescale(gcs_component_base::size());
  //}

  void set_size(vec3i new_size) override{
    info()->size = new_size;

    set_pos(pos() - (size() / 2));
  }

  void set_pos(vec3i new_pos) override{
    if(clamp_window){
      i32 padding = (4);
      new_pos.x = math::clamp(new_pos.x, padding, render->screen_size.x - (size().x + padding));
      new_pos.y = math::clamp(new_pos.y, padding + border_top_height, render->screen_size.y - (size().y + (padding + 1)));
    }

    if(info()->pos_ptr != nullptr)
      *info()->pos_ptr  = new_pos;
    else
      info()->pos       = new_pos;
  }

  void think(vec3i pos, vec3i size) override{
    if(!border || !draggable)
      return;

    vec3i border_pos   = pos - vec3i(0, border_top_height);
    vec3i border_size  = vec3i(size.x, border_top_height);
    bool is_in_border = is_in_rect(border_pos, border_size);

    if(is_in_border && input() & GCS_IN_M1_CLICK){
      drag_offset = mouse_pos() - pos;

      emit_click_sound(0);
    }

    bool last_is_dragging = is_dragging;
    is_dragging = (is_dragging || (input() & GCS_IN_M1_CLICK) && is_in_border) && input() & GCS_IN_M1;

    if(is_dragging)
      set_pos(mouse_pos() - drag_offset);
    else if(last_is_dragging)
      emit_click_sound(1);
    else
      set_pos(pos);
  }

  void draw_anywhere(vec3i pos, vec3i size) override{
    if(border){
      render->outlined_rect(pos - vec3i(1, border_top_height + 1), size + vec3i(2, border_top_height + 2), scheme()->main);

      colour alpha_border = scheme()->main;
      alpha_border.a = 255.f * 0.5f;
      render->filled_rect(pos - vec3i(0, border_top_height), vec3i(size.x, border_top_height), alpha_border);

      if(!name().empty()){
        vec3i title_pos = pos - vec3i(wants_icon ? 0 : -4, (border_top_height / 2));

        #if defined(RENDERING_MODE_D3D9)
          #if defined(INTERNAL_CHEAT) || defined(DEBUG_APPLICATION)
            if(wants_icon){
              colour logo_clr = scheme()->main * 0.95f;
              logo_clr.a      = scheme()->main.a;

              vec3i icon_size = render->rescale(vec3i(24, 26));

              render->render_logo(pos - vec3i(0, border_top_height + render->rescale(2)) + render->rescale(vec3i(1, 1)), icon_size, colour(0, 0, 0, logo_clr.a * 0.2f));
              render->render_logo(pos - vec3i(0, border_top_height + render->rescale(2)), icon_size, logo_clr);
              title_pos.x += icon_size.x + render->rescale(6);
            }
          #endif
        #endif

        font()->draw(title_pos, (18), name(), scheme()->text, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);
      }
    }
    else
      render->outlined_rect(pos - vec3i(1, 1), size + vec3i(2, 2), scheme()->main);
  }

  void draw(vec3i pos, vec3i size){
    if(scheme()->disable_window_background)
      return;

    colour alpha_background = scheme()->background * 0.75f;
    alpha_background.a = 255.f * 0.95f;
    render->filled_rect(pos, size, alpha_background);
  }
};