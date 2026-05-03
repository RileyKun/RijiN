#pragma once

enum gcs_slider_type{
  GCS_SLIDER_TYPE_FLOAT = 0,
  GCS_SLIDER_TYPE_FLOAT_PRECISE,
  GCS_SLIDER_TYPE_INT,
};

class gcs_component_slider : public gcs_component_base{
  float slider_width;
  i32   slider_bar_height         = 4;
  bool  slider_active = false;

  float slider_fraction = 0.f;
  i32   value_min       = 0;
  i32   value_max       = 0;
  u8    value_type      = GCS_SLIDER_TYPE_FLOAT;
  void* value_ptr       = nullptr;

  std::wstring postfix;
public:

  void setup() override{
    set_size(vec3i(0, render->rescale(render->rescale(21))));
    slider_bar_height = render->rescale(4);

    info()->always_run_think = true;
  }

  ALWAYSINLINE void set_value(float new_value){
    if(value_ptr == nullptr)
      return;

    if(value_type == GCS_SLIDER_TYPE_INT)
      *(i32*)value_ptr    = (i32)new_value;
    else if(value_type == GCS_SLIDER_TYPE_FLOAT || value_type == GCS_SLIDER_TYPE_FLOAT_PRECISE)
      *(float*)value_ptr  = new_value;
    else{
      DBG("GCS: gcs_component_slider: value_type is not valid\n");
    }
  }

  ALWAYSINLINE void set_bounds(i32 min, i32 max){
    value_min = min;
    value_max = max;
  }

  ALWAYSINLINE void set_ptr(u8 type, void* ptr){
    value_type  = type;
    value_ptr   = ptr;
  }

  ALWAYSINLINE void set_postfix(std::wstring new_postfix){
    postfix = new_postfix;
  }

  void think(vec3i pos, vec3i size) override{
    slider_width = size.x - push_align_offset();
    if(is_in_rect())
      push_help_text_to_scene();
  }

  void draw(vec3i pos, vec3i size){
    if(value_ptr == nullptr)
      return;

    vec3i slider_pos = pos + vec3i(size.x - slider_width, render->rescale(4));
    vec3i slider_size = vec3i(slider_width, size.y);

    {
      i32 equal = (slider_pos.x-pos.x);

      if(parent() != nullptr && parent()->module() != nullptr)
        equal -= parent()->module()->info()->module_size;

      slider_size.x -= equal;
    }

    // Draw option name
    vec3i name_size = font()->draw(pos + vec3i(push_align_offset(), 0), 16, name().c_str(), scheme()->text, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_LEFT);

    // draw slider
    {
      // draw slider bar
      {
        vec3i slider_bar_size = vec3i(slider_size.x, slider_bar_height);
        vec3i slider_bar_pos = slider_pos + vec3i(0, name_size.y + math::floor((slider_size.y * 0.19f) - (slider_bar_size.y / 2) - render->rescale(4)));

        colour transparent_fill = scheme()->grey3;
        transparent_fill.a = 255.f * 0.25f;

        render->filled_rect(slider_bar_pos, slider_bar_size, transparent_fill * 0.25f);
        render->outlined_rect(slider_bar_pos, slider_bar_size, scheme()->grey3);

        // handle slider logic
        {
          if(is_in_rect() && input() & GCS_IN_M1_CLICK || slider_active && input() & GCS_IN_M1){
            if(input() & GCS_IN_M1_CLICK)
              emit_click_sound();

            slider_active = true;

            float cursor_delta = mouse_pos().x - (slider_bar_pos.x);
            slider_fraction = cursor_delta == 0.f ? 0.f : math::clamp(cursor_delta / slider_bar_size.x, 0.f, 1.f);

            if(value_type == GCS_SLIDER_TYPE_FLOAT_PRECISE)
              set_value(math::lerp((float)value_min, (float)value_max, slider_fraction));
            else
              set_value(math::floor(math::lerp((float)value_min, (float)value_max, slider_fraction)));
          }
          else{
            slider_active = false;

            // We need to make sure the slider is correctly aligned when any new values are set
            // (val-min)/(max-min) = normalized fraction
            if(value_type == GCS_SLIDER_TYPE_FLOAT || value_type == GCS_SLIDER_TYPE_FLOAT_PRECISE)
              slider_fraction = math::clamp((*(float*)value_ptr - (float)value_min) / ((float)value_max - (float)value_min), 0.f, 1.f);
            else if(value_type == GCS_SLIDER_TYPE_INT)
              slider_fraction = math::clamp(((float)*(i32*)value_ptr - (float)value_min) / ((float)value_max - (float)value_min), 0.f, 1.f);
          }
        }

        if(slider_fraction > 0.f){
          vec3i test = vec3i(slider_bar_size.x * slider_fraction, slider_bar_size.y);

          colour transparent_fill = scheme()->main;
          transparent_fill.a = 255.f * 0.25f;

          render->filled_rect(slider_bar_pos, test, transparent_fill);
          render->outlined_rect(slider_bar_pos, test, scheme()->main * 1.25f);
        }

        // draw slider text
        {
          if(value_type == GCS_SLIDER_TYPE_FLOAT){
            FORMATW(val_str, 11, XOR(L"%i%ls"), (i32)(*(float*)value_ptr), postfix.c_str());
            font()->draw(pos + vec3i(push_align_offset() + slider_bar_size.x, 0), 16, val_str, scheme()->text, TSTYLE_SHADOW, TALIGN_RIGHT, TALIGN_LEFT);
          }
          else if(value_type == GCS_SLIDER_TYPE_FLOAT_PRECISE){
            i8 ch[1024];
            I(wsprintfA)(ch, XOR("%s%ls"), convert::float_to_str(*(float*)value_ptr).c_str(), postfix.c_str());

            std::wstring val_str = convert::str2wstr(ch);
            font()->draw(pos + vec3i(push_align_offset() + slider_bar_size.x, 0), 16, val_str, scheme()->text, TSTYLE_SHADOW, TALIGN_RIGHT, TALIGN_LEFT);
            //FORMATW(val_str, 11, XOR(L"%i%ls"), (i32)(*(float*)value_ptr), postfix.c_str());
            //font()->draw(pos + vec3i(push_align_offset() + slider_bar_size.x, 0), 16, val_str, scheme()->text, TSTYLE_SHADOW, TALIGN_RIGHT, TALIGN_LEFT);
          }
          else{
            FORMATW(val_str, 11, XOR(L"%i%ls"), *(i32*)value_ptr, postfix.c_str());
            font()->draw(pos + vec3i(push_align_offset() + slider_bar_size.x, 0), 16, val_str, scheme()->text, TSTYLE_SHADOW, TALIGN_RIGHT, TALIGN_LEFT);
          }
        }
      }
    }
  }
};