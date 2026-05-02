#pragma once

struct dropdown_entry{
  std::wstring  name;
  bool*         ptr = nullptr;
  bool          quick_swap = false;
};

#define GCS_DROPDOWN_SELECTED_TEXT_UPDATE_TIME 0.5f
class gcs_component_dropdown : public gcs_component_base{
  std::vector<dropdown_entry> options;
  i32                         dropdown_width;
  i32                         dropdown_height;
  bool                        dropdown_open               = false;
  bool                        skip_dropdown_in_rect_test  = false;
  bool                        multi_select                = false;
  float                       next_selected_text_update   = 0.f;
  std::wstring                selected_text;
  bool                        full_wide                   = false;

  // HACK for getting correct coordinates for the dropdown
  vec3i                       cache_pos;

  vec3i                       cache_size;
public:
  i32                         last_selected_index         = 0;

  gcs_component_dropdown* add_option(std::wstring name, bool* ptr){
    dropdown_entry entry;
    {
      entry.name  = name;
      entry.ptr   = ptr;
    }

    options.push_back(entry);

    return this;
  }

  ALWAYSINLINE void invalidate_selected_text(){
    next_selected_text_update = 0.f;
  }

  ALWAYSINLINE void update_selected_text(){
    if(scene()->info()->curtime < next_selected_text_update)
      return;

    selected_text.clear();

    bool set_entry = false;
    for(dropdown_entry entry : options){
        if(entry.ptr == nullptr)
          continue;

        if(*entry.ptr){
          if(set_entry)
            selected_text += WXOR(L", ");

          selected_text += entry.name;
          set_entry = true;
        }
    }

    if(!set_entry)
      selected_text = WXOR(L"None");

    next_selected_text_update = scene()->info()->curtime + GCS_DROPDOWN_SELECTED_TEXT_UPDATE_TIME;
  }

  ALWAYSINLINE u32 get_last_selected_index(){
    return last_selected_index;
  }

  ALWAYSINLINE void set_full_wide(bool new_full_wide){
    full_wide = new_full_wide;
  }

  ALWAYSINLINE void set_multi_select(bool new_multi_select){
    multi_select = new_multi_select;
  }

  ALWAYSINLINE void set_selected(u32 new_selected){
    if(new_selected >= options.size())
      return;

    dropdown_entry selected_entry = options[new_selected];

    if(selected_entry.ptr == nullptr)
      return;

    if(multi_select)
      *selected_entry.ptr = !*selected_entry.ptr;
    else{
      for(dropdown_entry entry : options){
        if(entry.ptr == nullptr)
          continue;

        *entry.ptr = entry.ptr == selected_entry.ptr;
      }

      dropdown_open               = false;
      skip_dropdown_in_rect_test  = false;

      scene()->info()->set_final_draw(nullptr);
      scene()->info()->input_frozen = false;
      scene()->info()->last_input_frozen = false;
    }

    last_selected_index = new_selected;

    invalidate_selected_text();
    update_selected_text();
    emit_click_sound();
  }

  void setup() override{
    dropdown_open = false;
    set_size(vec3i(0, render->rescale(34)));
    dropdown_height = render->rescale(16);

    invalidate_selected_text();

    info()->always_run_think = true;
  }

  void think(vec3i pos, vec3i size) override{
    if(full_wide)
      dropdown_width = size.x;
    else
      dropdown_width = size.x - push_align_offset();

    if(is_in_rect() && input() & GCS_IN_M1_CLICK){
        dropdown_open               = !dropdown_open;
        skip_dropdown_in_rect_test  = dropdown_open;

        scene()->info()->set_final_draw(dropdown_open ? this : nullptr);
        scene()->info()->input_frozen = dropdown_open;

        emit_click_sound(1);
    }

    update_selected_text();

    if(dropdown_open){
      cache_pos = pos;
      cache_size = size;
    }

    if(is_in_rect())
      push_help_text_to_scene();
  }

  void draw_last(vec3i pos, vec3i size) override{
    pos = cache_pos;
    size = cache_size;

    // Draw dropdown options
    {
      i32   option_padding = render->rescale(2);
      i32   option_size = (dropdown_height + option_padding);
      vec3i dropdown_pos = pos + vec3i(size.x - dropdown_width, size.y - 1);
      //vec3i dropdown_pos = pos + vec3i(size.x - dropdown_width, dropdown_height - 1);
      vec3i dropdown_size = vec3i(dropdown_width, (option_padding + (option_size * options.size())) + option_padding);
      vec3i checkbox_size = render->rescale(vec3i(16, 16));

      if(multi_select)
        dropdown_size.y += (option_padding * 2) + (render->rescale(16) * 2);


      if(!full_wide){
        i32 equal = (dropdown_pos.x-pos.x);

        if(parent() != nullptr && parent()->module() != nullptr)
          equal -= parent()->module()->info()->module_size;

        dropdown_size.x -= equal;
      }

      // Small logic hack
      if(!skip_dropdown_in_rect_test && !is_in_rect(dropdown_pos, dropdown_size, true) && input() & GCS_IN_M1_CLICK){
        dropdown_open = false;

        scene()->info()->set_final_draw(nullptr);

        // We need to set last_input_frozen so that the draw_last handler
        // Handles the change
        scene()->info()->input_frozen = false;
        scene()->info()->last_input_frozen = false;

        emit_click_sound(1);
      }
      else
        skip_dropdown_in_rect_test = false;

      colour transparent_fill = scheme()->background;
      transparent_fill.a = 255.f * 0.80f;

      render->filled_rect(dropdown_pos, dropdown_size, transparent_fill);
      render->outlined_rect(dropdown_pos, dropdown_size, scheme()->main * 1.25f);

      dropdown_pos += render->rescale(vec3i(-1, 1));

      u32 entry_count = 0;
      for(i32 i = 0; i < options.size(); i++){
        dropdown_entry* entry = &options[i];
        if(entry->ptr == nullptr)
          continue;

        vec3i option_pos   = dropdown_pos + vec3i(0, option_padding);
        bool is_selected  = *entry->ptr;

        bool is_in_entry = is_in_rect(option_pos, vec3i(dropdown_size.x, option_size), true);

        bool shift = input_system->held(VK_LSHIFT);
        if(!shift)
          entry->quick_swap = false;

        // handle logic for selection
        {
          if(is_in_entry && (input() & GCS_IN_M1_CLICK || shift && input_system->held(VK_LBUTTON) && !entry->quick_swap)){
            set_selected(entry_count);
            entry->quick_swap = true;
          }
          else if(is_in_entry){
            colour transparent_fill = scheme()->grey3;
            transparent_fill.a = 255.f * 0.5f;
            transparent_fill *= 1.10f;

            render->filled_rect(option_pos + vec3i(2, -1), vec3i(dropdown_size.x - 2, option_size), transparent_fill);
          }
        }

        font()->draw(option_pos + vec3i(5, checkbox_size.y / 2), 16, entry->name, scheme()->text * 0.90f, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);

        // draw checkbox
        if(multi_select || is_selected)
        {
          vec3i checkbox_pos = option_pos + vec3i(dropdown_size.x - checkbox_size.x - option_padding, 0);

          if(is_selected){
            colour transparent_fill = scheme()->main;
            transparent_fill.a = 255.f * 0.25f;

            if(is_in_entry)
              transparent_fill *= 1.25f;

            render->filled_rect(checkbox_pos, checkbox_size, transparent_fill);
            render->outlined_rect(checkbox_pos, checkbox_size, scheme()->main * 1.25f);

            u32 checkbox_font_size = 16;
#if defined(IMGUI_API)
            render->tick_mark(checkbox_pos + render->rescale(vec3i(3, 2)), scheme()->main * 1.25f, checkbox_font_size - render->rescale(4));
#else
            emoji()->draw(checkbox_pos + (checkbox_size / 2) - render->rescale(vec3i(1,0)), checkbox_font_size - render->rescale(2), L" ✔️", scheme()->main * 1.25f, TSTYLE_NONE, TALIGN_CENTER, TALIGN_CENTER);
#endif
          }
          else{
            colour transparent_fill = scheme()->grey3;
            transparent_fill.a = 255.f * 0.25f;
            transparent_fill *= 0.25f;

            render->filled_rect(checkbox_pos, checkbox_size, transparent_fill);
            render->outlined_rect(checkbox_pos, checkbox_size, scheme()->grey3);
          }
        }

        dropdown_pos.y += option_size;
        entry_count++;
      }

      if(multi_select){
        // Setup button position.
        dropdown_pos += vec3i(option_padding * 2, 0);
        dropdown_pos.y += option_padding;

        {
          vec3i  set_all_size = vec3i(dropdown_size.x - (option_padding * 3), render->rescale(16));
          bool   in_rect  = is_in_rect(dropdown_pos, set_all_size, true);
          colour draw_col = in_rect ? scheme()->main * 1.20f : scheme()->main;

          render->outlined_rect(dropdown_pos, set_all_size, draw_col);
          draw_col.a = 255.f * 0.5f;
          render->filled_rect(dropdown_pos, set_all_size, draw_col);
          font()->draw(dropdown_pos + (set_all_size / 2), 14, WXOR(L"Enable all"), scheme()->text, TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_CENTER);

          if(in_rect && input() & GCS_IN_M1_CLICK){
            for(dropdown_entry entry : options){
              *entry.ptr = 1;
            }
          }
          dropdown_pos.y += set_all_size.y + option_padding;
        }


        {
          vec3i  reset_all_size = vec3i(dropdown_size.x - (option_padding * 3), render->rescale(16));
          bool   in_rect  = is_in_rect(dropdown_pos, reset_all_size, true);
          colour draw_col = in_rect ? scheme()->main * 1.20f : scheme()->main;

          render->outlined_rect(dropdown_pos, reset_all_size, draw_col);
          draw_col.a = 255.f * 0.5f;
          render->filled_rect(dropdown_pos, reset_all_size, draw_col);

          font()->draw(dropdown_pos + (reset_all_size / 2), 14, WXOR(L"Disable all"), scheme()->text, TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_CENTER);

          if(in_rect && input() & GCS_IN_M1_CLICK){
            for(dropdown_entry entry : options){
              *entry.ptr = 0;
            }
          }
        }
      }

    }
  }

  void draw(vec3i pos, vec3i size){

    // Draw dropdown
    {
      vec3i dropdown_pos = pos + vec3i(size.x - dropdown_width, size.y - dropdown_height);
      vec3i dropdown_size = vec3i(dropdown_width, dropdown_height);

      if(!full_wide){
        i32 equal = (dropdown_pos.x-pos.x);

        if(parent() != nullptr && parent()->module() != nullptr)
          equal -= parent()->module()->info()->module_size;

        dropdown_size.x -= equal;
      }

      if(dropdown_open){
        colour transparent_fill = scheme()->main;
        transparent_fill.a = 255.f * 0.25f;

        render->filled_rect(dropdown_pos, dropdown_size, transparent_fill);
        render->outlined_rect(dropdown_pos, dropdown_size, scheme()->main * 1.25f);

        {
          render->push_clip_region(dropdown_pos, dropdown_size - render->rescale(vec3i(18, 0)));

          if(!selected_text.empty())
            font()->draw(dropdown_pos + vec3i(4, dropdown_size.y / 2), 16, selected_text, scheme()->text * 0.90f, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);

          render->pop_clip_region();
        }

        // Draw the arrow (open)
        {
          vec3i broad_box = vec3i(16, dropdown_size.y);
          vec3i inner_box = vec3i(10, 5);
          vec3i broad_pos = dropdown_pos + vec3i(dropdown_size.x - broad_box.x, 0) - vec3i(2, 0);
          vec3i inner_pos = broad_pos + ((broad_box / 2) - (inner_box / 2));

          // left to top
          render->solid_line(inner_pos + vec3i(1, inner_box.y - 1), inner_pos + vec3i(inner_box.x / 2, 0), scheme()->main * 1.25f);

          render->solid_line(inner_pos + vec3i(inner_box.x / 2, 0), inner_pos + inner_box, scheme()->main * 1.25f);
        }
      }
      else{
        colour transparent_fill;

        if(is_in_rect()){
          transparent_fill = scheme()->background * 1.2f;
          transparent_fill.a = 255.f * 0.75f;
        }
        else{
          transparent_fill = scheme()->grey3;
          transparent_fill.a = 255.f * 0.25f;
          transparent_fill *= 0.25f;
        }

        render->filled_rect(dropdown_pos, dropdown_size, transparent_fill);
        render->outlined_rect(dropdown_pos, dropdown_size, scheme()->grey3);

        {
          render->push_clip_region(dropdown_pos, dropdown_size - render->rescale(vec3i(18, 1)));

          if(!selected_text.empty())
            font()->draw(dropdown_pos + vec3i(4, dropdown_size.y / 2), 16, selected_text, scheme()->text * 0.90f, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);

          render->pop_clip_region();
        }

        // Draw the arrow
        {
          vec3i broad_box = vec3i(16, dropdown_size.y);
          vec3i inner_box = vec3i(10, 5);
          vec3i broad_pos = dropdown_pos + vec3i(dropdown_size.x - broad_box.x, 0) - vec3i(2, 0);
          vec3i inner_pos = broad_pos + ((broad_box / 2) - (inner_box / 2));

          // 1
          render->solid_line(inner_pos + vec3i(1,1), inner_pos + vec3i(inner_box.x / 2, inner_box.y), scheme()->grey3 * 1.25f);

          // 2
          render->solid_line(inner_pos + vec3i(inner_box.x / 2, inner_box.y), inner_pos + vec3i(inner_box.x, 0), scheme()->grey3 * 1.25f);
        }
      }
    }

    // Draw option name
    if(!full_wide)
      font()->draw(pos + vec3i(push_align_offset(), 0), 16, name().c_str(), scheme()->text, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_LEFT);
  }
};