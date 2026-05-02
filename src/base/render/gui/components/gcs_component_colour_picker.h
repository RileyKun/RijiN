#pragma once

class gcs_component_colour_picker : public gcs_component_base{
public:
  bool  picker_open = false;

  vec3i picker_pos;
  vec3i picker_size = vec3i(192, 192);
  vec3i picker_padding = vec3i(4, 4);
  bool  skip_picker_click_test = false;

  // colour specific data
  float* output_colour;

  // hue selection
  float   selected_hue = 0.f;
  bool    is_selecting_hue = false;

  // alpha selection
  float   selected_alpha = 1.f;
  bool    is_selecting_alpha = false;
  bool    alpha_enabled      = false;

  // colour selection
  vec3 selected_colour_frac;
  bool  is_selecting_color = false;

  bool is_module() override{
    return true;
  }

  void setup() override{
    info()->module_size         = render->rescale(20);
    info()->always_run_think    = true;

    picker_size    = render->rescale(vec3i(192, 192));
    picker_padding = render->rescale(vec3i(4, 4));
  }

  gcs_component_colour_picker* set_output_colour_ptr(float* colour_vec){
    output_colour = colour_vec;

    return this;
  }

  gcs_component_colour_picker* enable_alpha_bar(){
    alpha_enabled = true;

    return this;
  }

  colour get_output_colour(bool include_alpha = true){
    assert(output_colour != nullptr);

    return rgba(output_colour[0], output_colour[1], output_colour[2], include_alpha ? output_colour[3] : 255);
  }

  ALWAYSINLINE void set_output_colour(colour new_col){
    assert(output_colour != nullptr);

    output_colour[0] = new_col.r;
    output_colour[1] = new_col.g;
    output_colour[2] = new_col.b;
    output_colour[3] = new_col.a;
  }

  void export_to_clipboard(){
    i8 hex_value[1024];
    wsprintfA(hex_value, XOR("%08x"), get_output_colour(alpha_enabled).tohex_rgba());
    clipboard::set_text(hex_value);
  }

  void import_from_clipboard(){
    i8* hex_code = clipboard::get_text_alloc();
    if(hex_code != nullptr){
      if(hex_code[0] == '#')
        *hex_code++;

      if(str_utils::is_hex(hex_code)){
        colour value = convert::hex_str_to_colour(hex_code);

        if(value.r == 0 && value.g == 0 && value.b == 0 && value.a == 0)
          return;

        if(!alpha_enabled)
          value.a = 255;

        set_output_colour(value);
        calculate_hue_and_offset_from_colour();
      }
    }
  }

  void think(vec3i pos, vec3i size) override{
    if(!picker_open && is_in_rect() && input_system->pressed(VK_LBUTTON)){
      picker_open             = !picker_open;
      skip_picker_click_test  = picker_open;

      scene()->info()->set_final_draw(picker_open ? this : nullptr);
      scene()->info()->input_frozen = picker_open;
      emit_click_sound(1);

      calculate_hue_and_offset_from_colour();
    }

    if(picker_open)
      calculate_picker_pos(pos, size);
  }

  void calculate_picker_pos(vec3i pos, vec3i size){
    pos        += size + picker_padding;
    pos.x       = math::clamp(pos.x, picker_padding.x, render->screen_size.x - (picker_size.x + picker_padding.x));
    pos.y       = math::clamp(pos.y, picker_padding.y, render->screen_size.y - (picker_size.y + picker_padding.y));
    picker_pos  = pos;
  }

  void calculate_output_colour(){
    if(output_colour == nullptr)
      return;

    colour wish_output_colour = math::hsv_2_rgb(hsv(selected_hue, selected_colour_frac.x, 1.f - selected_colour_frac.y));
    wish_output_colour.a      = selected_alpha > 0.f ? math::clamp(255.f * selected_alpha, 0.f, 255.f) : 0.f;

    set_output_colour(wish_output_colour);
  }

  void calculate_hue_and_offset_from_colour(){
    if(output_colour == nullptr)
      return;

    colour output           = get_output_colour(true);
    hsv hsv_col             = math::rgb_2_hsv(output);

    selected_hue            = hsv_col.h;
    selected_colour_frac.x  = hsv_col.s;
    selected_colour_frac.y  = 1.f - hsv_col.v;
    selected_alpha          = output.a > 0.f ? output.a / 255.f : 0.f;
  }

  void draw(vec3i pos, vec3i size) override{
    size.y = math::smallest(size.y, render->rescale(13));

    render->filled_rect(pos, size, get_output_colour(false));
    render->outlined_rect(pos, size, picker_open ? (scheme()->main * 1.25f) : scheme()->grey3);
  }

  void draw_last(vec3i pos, vec3i size) override{
    if(!picker_open)
      return;

    if(input_system->held(VK_CONTROL)){
      if(input_system->held('C'))
        export_to_clipboard();
      else if(input_system->held('V'))
        import_from_clipboard();
    }

    // check if we should close
    {
      if(!skip_picker_click_test && !is_in_rect(picker_pos, picker_size, true) && input_system->pressed(VK_LBUTTON)){
        picker_open = false;
        scene()->info()->set_final_draw(nullptr);
        scene()->info()->input_frozen = false;
        scene()->info()->last_input_frozen = false;

        emit_click_sound(1);
      }
      else
        skip_picker_click_test = false;
    }

    // draw background
    {
      colour alpha_background = scheme()->background * 0.75f;
      alpha_background.a = 255.f * 0.95f;

      render->filled_rect(picker_pos, picker_size, alpha_background);
      render->outlined_rect(picker_pos - vec3i(1,1), picker_size + vec3i(2,2), scheme()->main * 1.25f);
    }

    // draw elements
    {
      vec3i element_padding = render->rescale(vec3i(2,2));
      vec3i scrollbar_leway = render->rescale(vec3i(14, 14));
      vec3i button_size     = render->rescale(vec3i(0, 18));

      // draw colour selection box
      vec3i colour_selection_box_pos;
      vec3i colour_selection_box_size;
      {
        colour_selection_box_pos  = picker_pos + element_padding;
        vec3i total_extra_height = element_padding + vec3i(0, scrollbar_leway.y + element_padding.y); // alpha + copy button
        colour_selection_box_size = picker_size - (scrollbar_leway + element_padding + (total_extra_height * 2) + element_padding);


        render->gradient_rounding_hack = true;

        if(render->mode() == RENDER_MODE_D2D1){
          render->gradient_rect_linear(colour_selection_box_pos, colour_selection_box_size, vec3i(0, 0), vec3i(0, colour_selection_box_size.y), rgb(255,255,255), rgb(0,0,0), false);
          render->gradient_rect_linear(colour_selection_box_pos, colour_selection_box_size, vec3i(colour_selection_box_size.x, 0), vec3i(0, colour_selection_box_size.y), math::hsv_2_rgb(hsv(selected_hue, 1.f, 1.f)), rgba(0,0,0,0), false);
        }
        else
          render->gradient_rect(colour_selection_box_pos, colour_selection_box_size, rgb(255,255,255), math::hsv_2_rgb(hsv(selected_hue, 1.f, 1.f)), rgb(0,0,0), rgb(0,0,0));

        render->outlined_rect(colour_selection_box_pos, colour_selection_box_size, scheme()->grey3);


        if((is_selecting_color = is_in_rect(colour_selection_box_pos, colour_selection_box_size, true) && input_system->pressed(VK_LBUTTON) || is_selecting_color && input_system->held(VK_LBUTTON))){
          selected_colour_frac.x = math::clamp((float)(mouse_pos().x - colour_selection_box_pos.x) / colour_selection_box_size.x, 0.f, 1.f);
          selected_colour_frac.y = math::clamp((float)(mouse_pos().y - colour_selection_box_pos.y) / colour_selection_box_size.y, 0.f, 1.f);

          calculate_output_colour();
        }

        // draw selection circle
        {
          render->push_clip_region(colour_selection_box_pos + vec3i(1, 1), colour_selection_box_size - vec3i(2, 2));
          {
            float radius = 4.f;

            render->filled_circle(colour_selection_box_pos + (vec3i(colour_selection_box_size.x * selected_colour_frac.x, colour_selection_box_size.y * selected_colour_frac.y)), radius,   10.f, rgba(230,230,230,80));
            render->outlined_circle(colour_selection_box_pos + (vec3i(colour_selection_box_size.x * selected_colour_frac.x, colour_selection_box_size.y * selected_colour_frac.y)), radius, 10.f, rgb(230,230,230));
          }
          render->pop_clip_region();
        }
      }

      // draw hue selection bar
      {
        vec3i hue_selection_bar_pos = colour_selection_box_pos + vec3i(colour_selection_box_size.x + element_padding.x, 0);
        vec3i hue_selection_bar_size = vec3i(scrollbar_leway.x, colour_selection_box_size.y);

        const colour hue_colour_spectrum[] = {
          rgb(255, 0, 0),
          rgb(255, 255, 0),
          rgb(0, 255, 0),
          rgb(0, 255, 255),
          rgb(0, 0, 255),
          rgb(255, 0, 255),
          rgb(255, 0, 0)
        };

        u32 hue_colour_spectrum_size = (sizeof(hue_colour_spectrum) / sizeof(colour)) - 1;
        vec3i size_per_spectrum      = vec3i(hue_selection_bar_size.x, (hue_selection_bar_size.y) / hue_colour_spectrum_size);
        vec3i hue_pos                = hue_selection_bar_pos;
        for(u32 i = 0; i < hue_colour_spectrum_size; i++){
          if(render->mode() == RENDER_MODE_D2D1)
            render->gradient_rect_linear(hue_pos, size_per_spectrum, vec3i(0, 0), vec3i(0, size_per_spectrum.y), hue_colour_spectrum[i], hue_colour_spectrum[i + 1], false);
          else
            render->gradient_rect(hue_pos, size_per_spectrum, hue_colour_spectrum[i], hue_colour_spectrum[i], hue_colour_spectrum[i + 1], hue_colour_spectrum[i + 1]);

          hue_pos.y += size_per_spectrum.y;
        }

        render->outlined_rect(hue_selection_bar_pos, hue_selection_bar_size, scheme()->grey3);

        // handle hue selection
        {
          if((is_selecting_hue = is_in_rect(hue_selection_bar_pos, hue_selection_bar_size, true) && input_system->pressed(VK_LBUTTON) || is_selecting_hue && input_system->held(VK_LBUTTON))){
            selected_hue = math::clamp((float)(mouse_pos().y - hue_selection_bar_pos.y) / hue_selection_bar_size.y, 0.f, 1.f);

            calculate_output_colour();
          }

          // draw hue bar
          {
            vec3i hue_bar_pos = hue_selection_bar_pos + vec3i(0, math::clamp((i32)(hue_selection_bar_size.y * selected_hue), 2, hue_selection_bar_size.y - 3));

            render->filled_rect(hue_bar_pos - vec3i(1,1), vec3i(hue_selection_bar_size.x + 2, 3), rgba(230,230,230,80));
            render->outlined_rect(hue_bar_pos - vec3i(1,1), vec3i(hue_selection_bar_size.x + 2, 3), rgb(230,230,230));
          }
        }
      }

      // draw alpha selection bar
      vec3i alpha_selection_bar_pos  = colour_selection_box_pos + vec3i(0, colour_selection_box_size.y + element_padding.y);
      vec3i alpha_selection_bar_size = vec3i(colour_selection_box_size.x, scrollbar_leway.x);
      {
        render->push_clip_region(alpha_selection_bar_pos, alpha_selection_bar_size);

        // draw checkered boxes
        {
          vec3i box_size = vec3i(scrollbar_leway.x / 2, scrollbar_leway.y / 2);
          bool  swap = false;

          for(u32 i = 0; i < alpha_selection_bar_size.x; i += box_size.x){
            render->filled_rect(alpha_selection_bar_pos + vec3i(i, 0), box_size, swap ?  rgba(188,188,188,127) : rgba(255,255,255,127));
            render->filled_rect(alpha_selection_bar_pos + vec3i(i, box_size.y), box_size, swap ? rgba(255,255,255,127) :  rgba(188,188,188,127));

            swap = !swap;
          }
        }

        if(render->mode() == RENDER_MODE_D2D1)
          render->gradient_rect_linear(alpha_selection_bar_pos, alpha_selection_bar_size, vec3i(0, 0), vec3i(alpha_selection_bar_size.x, 0), get_output_colour(false), rgba(0,0,0,0), false);
        else
          render->gradient_rect(alpha_selection_bar_pos, alpha_selection_bar_size, rgba(0,0,0,0), get_output_colour(false), rgba(0,0,0,0), get_output_colour(false));

        render->outlined_rect(alpha_selection_bar_pos, alpha_selection_bar_size, scheme()->grey3);

        render->pop_clip_region();

        // handle alpha selection
        if(alpha_enabled)
        {
          if((is_selecting_alpha = is_in_rect(alpha_selection_bar_pos, alpha_selection_bar_size, true) && input_system->pressed(VK_LBUTTON) || is_selecting_alpha && input_system->held(VK_LBUTTON))){
            selected_alpha = math::clamp((float)(mouse_pos().x - alpha_selection_bar_pos.x) / alpha_selection_bar_size.x, 0.f, 1.f);

            calculate_output_colour();
          }

          // draw hue bar
          {
            vec3i alpha_bar_pos = alpha_selection_bar_pos + vec3i(math::clamp((i32)(alpha_selection_bar_size.x * selected_alpha), 2, alpha_selection_bar_size.x - 3), 0);

            render->filled_rect(alpha_bar_pos - vec3i(1,1), vec3i(3, alpha_selection_bar_size.y + 2), rgba(230,230,230,80));
            render->outlined_rect(alpha_bar_pos - vec3i(1,1), vec3i(3, alpha_selection_bar_size.y + 2), rgb(230,230,230));
          }
        }
      }

      vec3i col_output_pos  = colour_selection_box_pos + vec3i(colour_selection_box_size.x + element_padding.x, colour_selection_box_size.y + element_padding.y);
      vec3i col_output_size = vec3i(scrollbar_leway.x, scrollbar_leway.y);
      // draw final colour output
      {
        render->filled_rect(col_output_pos, col_output_size, get_output_colour(true));
        render->outlined_rect(col_output_pos, col_output_size, scheme()->grey3);
      }

      // Info
      {
        colour current_clr = get_output_colour(alpha_enabled);
        wchar_t clr_buf[1024];
        wsprintfW(clr_buf, WXOR(L"R: %i, G: %i, B: %i, A: %i"), (i32)current_clr.r, (i32)current_clr.g, (i32)current_clr.b, (i32)current_clr.a);
        vec3i info_size = font()->draw(alpha_selection_bar_pos + vec3i(element_padding.x, alpha_selection_bar_size.y + element_padding.y), 14, clr_buf, scheme()->text, TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_CHEAP);

        wsprintfW(clr_buf, WXOR(L"Hex: %08x"), current_clr.tohex_rgba());
        font()->draw(alpha_selection_bar_pos + vec3i(element_padding.x, alpha_selection_bar_size.y + element_padding.y + info_size.y + element_padding.y), 14, clr_buf, scheme()->text, TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_CHEAP);
      }

      // Copy button
      vec3i copy_button_pos  = col_output_pos + vec3i(0, col_output_size.y + element_padding.y);
      vec3i copy_button_size = vec3i(scrollbar_leway.x, scrollbar_leway.y);
      {
        bool in_rect = is_in_rect(copy_button_pos, copy_button_size, true);

        colour draw_col = in_rect ? scheme()->main * 1.20f : scheme()->main;
        render->outlined_rect(copy_button_pos, copy_button_size, draw_col);
        draw_col.a = 255.f * 0.5f;
        render->filled_rect(copy_button_pos, copy_button_size, draw_col);

        font()->draw(copy_button_pos + (copy_button_size / 2), 10, WXOR(L"📤"), scheme()->error, TSTYLE_OUTLINE, TALIGN_CENTER, TALIGN_CENTER);

        if(in_rect){
          font()->draw(mouse_pos() - vec3i(0, 12), 12, WXOR(L"Copy to clipboard"), scheme()->text, TSTYLE_OUTLINE, TALIGN_CENTER, TALIGN_CENTER);

          if(input_system->pressed(VK_LBUTTON))
            export_to_clipboard();
        }
      }

      // Import button
      {
        vec3i import_button_pos  = copy_button_pos + vec3i(0, col_output_size.y + element_padding.y);
        vec3i import_button_size = vec3i(scrollbar_leway.x, scrollbar_leway.y);

        bool in_rect = is_in_rect(import_button_pos, import_button_size, true);

        colour draw_col = in_rect ? scheme()->main * 1.20f : scheme()->main;
        render->outlined_rect(import_button_pos, import_button_size, draw_col);
        draw_col.a = 255.f * 0.5f;
        render->filled_rect(import_button_pos, import_button_size, draw_col);

        font()->draw(import_button_pos + (import_button_size / 2), 10, WXOR(L"📥"), scheme()->success, TSTYLE_OUTLINE, TALIGN_CENTER, TALIGN_CENTER);

        if(in_rect){
          font()->draw(mouse_pos() - vec3i(0, 12), 12, WXOR(L"Import from clipboard"), scheme()->text, TSTYLE_OUTLINE, TALIGN_CENTER, TALIGN_CENTER);
          if(input_system->pressed(VK_LBUTTON))
            import_from_clipboard();
        }
      }

    }
  }


};