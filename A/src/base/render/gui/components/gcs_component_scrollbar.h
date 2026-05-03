#pragma once

class gcs_component_scrollbar : public gcs_component_base{
  float scroll_size_fraction   = 0.f;
  float scroll_fraction        = 0.f;
  bool  requires_scrolling     = false;
  bool  scrollbar_active       = false;
  i32   scrollbar_offset       = 0;
  vec3i scrollbar_pos;
  vec3i scrollbar_size;
  vec3i scrollbar_clicked_offset;
  i32   last_total_render_height = 0;
public:

  bool is_module() override{
    return true;
  }

  void setup() override{
    info()->module_size = render->rescale(8);

    info()->always_run_think = true;

    //info()->clip_size_offset = vec3i(10,0);
  }

  void think(vec3i pos, vec3i size) override{
    if(parent() == nullptr)
      return;

    i32 parent_box_height = parent()->size().y;
    i32 render_height     = info()->total_render_height;

    if(parent_box_height == 0.f || render_height == 0.f)
      return;

    scroll_size_fraction = (float)render_height / (float)parent_box_height;
    requires_scrolling = scroll_size_fraction > 1.f;

    if(requires_scrolling){
      scroll_size_fraction = math::clamp(1.f - (scroll_size_fraction - 1.f), 0.25f, 1.f);

      // fix scrolling when sizes change
      {
        i32 correct_render_height = info()->total_render_height - parent()->size().y;

        if(math::abs(parent()->info()->auto_positioning_override_height) > correct_render_height){
          i32 correct_height                                  = (i32)((float)size.y * (1.f - scroll_size_fraction));
          scrollbar_offset                                    = (i32)((float)((float)correct_height * scroll_fraction));
          parent()->info()->auto_positioning_override_height  = -((i32)((float)correct_render_height * (float)scroll_fraction));
        }
      }

      scrollbar_pos = pos + vec3i(0, scrollbar_offset);

      if(is_in_rect(pos, size, true) || scrollbar_active){
        if(input() & GCS_IN_M1_CLICK){
          scrollbar_active = true;
          scrollbar_clicked_offset = mouse_pos() - scrollbar_pos;

          emit_click_sound(0);
        }

        if(scrollbar_active && input() & GCS_IN_M1){
          i32 offset_height   = (i32)((float)math::clamp(mouse_pos().y - pos.y - scrollbar_clicked_offset.y, 0, size.y));
          i32 correct_height  = (i32)((float)size.y * (1.f - scroll_size_fraction));

          if(offset_height > 0 && correct_height > 0)
            scroll_fraction     = math::clamp((float)offset_height / (float)correct_height, 0.f, 1.f);
          else
            scroll_fraction     = 0.f;

          scrollbar_offset    = (i32)((float)((float)correct_height * scroll_fraction));

          i32 correct_render_height = info()->total_render_height - parent()->size().y;
          parent()->info()->auto_positioning_override_height = -((i32)((float)correct_render_height * (float)scroll_fraction));
        }
        else{
          if(scrollbar_active)
            emit_click_sound(1);

          scrollbar_active = false;
        }
      }
    }
    else{
      parent()->info()->auto_positioning_override_height = 0.f;
      scroll_fraction = 0.f;
      scrollbar_offset = 0;
      scrollbar_offset = 0;
    }
  }

  void draw(vec3i pos, vec3i size){
    if(!requires_scrolling)
      return;

    // Scrollbar background
    {
      colour background_transparent = scheme()->grey3;
      background_transparent.a = 255.f * 0.5f;
      render->filled_rect(pos, size, background_transparent, 0.f);
    }

    // Scrollbar itself
    {
      vec3i padding = vec3i(1,0);
      vec3i scrollbar_pos = pos + vec3i(0, scrollbar_offset) + padding;
      vec3i scrollbar_size = vec3i(size.x, (i32)((float)size.y * scroll_size_fraction)) - (padding * 2);
      bool is_in_scrollbar = is_in_rect(scrollbar_pos, scrollbar_size, true);

      colour scrollbar_transparent;

      if(scrollbar_active){
        scrollbar_transparent = scheme()->grey3 * 2.f;
        scrollbar_transparent.a = 255.f * 0.5f;
      }
      else if(is_in_scrollbar){
        scrollbar_transparent = scheme()->grey3 * 1.5f;
        scrollbar_transparent.a = 255.f * 0.5f;
      }
      else{
        scrollbar_transparent = scheme()->grey3 * 1.25f;
        scrollbar_transparent.a = 255.f * 0.5f;
      }

      scrollbar_pos.y = math::clamp(scrollbar_pos.y, scrollbar_pos.y + 1, (scrollbar_pos.y + scrollbar_size.y) - 2);
      render->filled_rect(scrollbar_pos, scrollbar_size, scrollbar_transparent, 0.f);
    }
  }
};