#pragma once

namespace gcs{
  // Example usage:
  // gcs::text(panel, L"test");
  inline gcs_component_text* text(gcs_component_base* base, std::wstring name, u32 font_size = 22){
    gcs_component_text* new_comp = base->add<gcs_component_text>();
    assert(new_comp != nullptr);

    new_comp->set_name(name);
    new_comp->set_font_size(font_size);

    return new_comp;
  }

  inline gcs_component_label* label(gcs_component_base* base, std::wstring text){
    gcs_component_label* new_comp = base->add<gcs_component_label>();
    assert(new_comp != nullptr);

    new_comp->set_text(text);

    return new_comp;
  }

  inline gcs_component_divider* divider(gcs_component_base* base){
    gcs_component_divider* new_comp = base->add<gcs_component_divider>();
    assert(new_comp != nullptr);
    return new_comp;
  }

  inline gcs_component_button* button(gcs_component_base* base, std::wstring text, void* param = nullptr){
    gcs_component_button* new_comp = base->add<gcs_component_button>();
    assert(new_comp != nullptr);

    new_comp->set_name(text);
    new_comp->set_size(vec3i(base->info()->size.x, 24));

    return new_comp;
  }

#if defined(RENDERING_MODE_D2D1)
  inline gcs_component_cheat_select_option* cheat_select_option(gcs_component_base* base, std::wstring text, void* param = nullptr){
    gcs_component_cheat_select_option* new_comp = base->add<gcs_component_cheat_select_option>();
    assert(new_comp != nullptr);

    new_comp->set_name(text);

    return new_comp;
  }
#endif

  // Example usage:
  // gcs::slider(panel, L"test", -360, 360, GCS_SLIDER_TYPE_FLOAT, &test);
  inline gcs_component_slider* slider(gcs_component_base* base, std::wstring name, std::wstring postfix, i32 min, i32 max, u8 type, void* ptr){
    gcs_component_slider* new_comp = base->add<gcs_component_slider>();
    assert(new_comp != nullptr);

    new_comp->set_name(name);
    new_comp->set_bounds(min, max);
    new_comp->set_ptr(type, ptr);
    new_comp->set_postfix(postfix);

    return new_comp;
  }

  inline gcs_component_slider* slider(gcs_component_base* base, std::wstring name, i32 min, i32 max, u8 type, void* ptr){
    gcs_component_slider* new_comp = base->add<gcs_component_slider>();
    assert(new_comp != nullptr);

    new_comp->set_name(name);
    new_comp->set_bounds(min, max);
    new_comp->set_ptr(type, ptr);

    return new_comp;
  }


  // Example usage:
  // gcs::toggle(panel, L"Enabled", &test);
  inline gcs_component_toggle* toggle(gcs_component_base* base, std::wstring name, bool* bool_ptr){
    gcs_component_toggle* new_comp = base->add<gcs_component_toggle>();
    assert(new_comp != nullptr);

    new_comp->set_name(name);
    new_comp->set_bool(bool_ptr);

    return new_comp;
  }

  // Example usage:
  // gcs::dropdown(panel, L"Test", true) <- true = multi select
  // ->add_option(L"No-Spread", &test[0])
  // ->add_option(L"No-Recoil", &test[1]);
  inline gcs_component_dropdown* dropdown(gcs_component_base* base, std::wstring name, bool multi_select = false){
    gcs_component_dropdown* new_comp = base->add<gcs_component_dropdown>();
    assert(new_comp != nullptr);

    new_comp->set_name(name);
    new_comp->set_multi_select(multi_select);

    return new_comp;
  }

  // Example usage:
  // gcs_component_canvas* panel = gcs::canvas(comp, L"test", pos, size, true); <- true = auto positioning children
  inline gcs_component_canvas* canvas(gcs_component_base* base, std::wstring name, vec3i pos, vec3i size, bool auto_positioning = false){
    gcs_component_canvas* new_comp = base->add<gcs_component_canvas>();
    assert(new_comp != nullptr);

    new_comp->set_name(name);
    new_comp->set_pos(pos);
    new_comp->set_size(size);

    if(auto_positioning)
      new_comp->info()->enable_auto_positioning();

    return new_comp;
  }

  inline gcs_component_group_host* group_host(gcs_component_base* base, std::wstring name, vec3i pos, vec3i size){
    gcs_component_group_host* new_comp = base->add<gcs_component_group_host>();
    assert(new_comp != nullptr);

    new_comp->set_name(name);
    new_comp->set_pos(pos);
    new_comp->set_size(size);

    return new_comp;
  }

  inline gcs_component_window* window(gcs_scene* base, std::wstring name, vec3i pos, vec3i size, bool auto_positioning = false){
    gcs_component_window* new_comp = base->add<gcs_component_window>();
    assert(new_comp != nullptr);

    new_comp->set_name(name);
    new_comp->set_pos(pos);
    new_comp->set_size(size);

    if(auto_positioning)
      new_comp->info()->enable_auto_positioning();

    return new_comp;
  }

  inline void add_space(gcs_component_group* group, bool* should_render_pointer = nullptr){
    assert(group != nullptr);
    gcs::divider(group)
      ->set_render_on(should_render_pointer);
  }

  inline void add_maximum_fov(gcs_component_group* group, i32* fov, bool* enabled = nullptr, bool* enabled2 = nullptr){
    assert(fov != nullptr);
    assert(group != nullptr);

    *fov = math::clamp(*fov, 1, XOR32(180));

    gcs::slider(group, WXOR(L"Maximum FOV"), WXOR(L"°"), 1, XOR32(180), GCS_SLIDER_TYPE_INT, fov)
      ->set_render_on(enabled)
      ->set_render_on(enabled2);
  }

  static void add_key_bind_control(gcs_component_group* group,
    std::wstring base_name,
      bool* enabled,
      bool* use_key,
      bool* key_held,
      bool* key_toggle,
      bool* key_dc,
      u8* key){
    assert(enabled != nullptr);
    assert(use_key != nullptr);
    assert(key_held != nullptr);
    assert(key_toggle != nullptr);
    assert(key_dc != nullptr);
    assert(key != nullptr);
    assert(group != nullptr);

    // Create the master toggle.
    gcs::toggle(group, base_name, enabled)
      ->add_module<gcs_component_hotkey>()
      ->set_hotkey_ptr(key)
      ->set_render_on(enabled)
      ->set_render_on(use_key);

    gcs::toggle(group, WXOR(L"Use key"), use_key)
      ->set_render_on(enabled);

    gcs::dropdown(group, WXOR(L"Key control"), false)
      ->add_option(WXOR(L"Held"), key_held)
      ->add_option(WXOR(L"Toggle"), key_toggle)
      ->add_option(WXOR(L"Double click"), key_dc)
      ->set_render_on(enabled)
      ->set_render_on(use_key);
  }

  static void add_key_bind_control(gcs_component_group* group, c_key_setting* key, std::wstring name){
    add_key_bind_control(group,
      name,
      &key->enabled,
      &key->use_key,
      &key->held_state,
      &key->toggle_state,
      &key->double_click_state,
      &key->key);
  }

  static std::pair<gcs_component_group_host*, gcs_component_canvas*> fetch_playerlist_settings_comp(gcs_component_base* comp, std::wstring name, std::wstring other_window, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_canvas* main_canvas = gcs::canvas(comp, name, pos, size, false);
    gcs_component_canvas* canvas      = gcs::canvas(main_canvas, name, vec3i(), size, false);
    canvas->info()->disable_auto_positioning();

    return std::make_pair(gcs::group_host(canvas, WXOR(L"Settings"), vec3i(), size), main_canvas);
  }

  template<typename T>
  inline gcs_component_canvas* fetch_playerlist_window_comp(gcs_component_canvas* main_canvas, std::wstring name, vec3i size){
    gcs_component_canvas* canvas = gcs::canvas(main_canvas, name, vec3i(size.x / 2, 2), vec3i(size.x / 2, size.y - 4), true);
    assert(canvas != nullptr);
    canvas->add<T>()
      ->info()->disable_alpha_vis_fade();

    canvas->add_module<gcs_component_scrollbar>();
    canvas->info()->disable_alpha_vis_fade();

    canvas->add_hook(GCS_HOOK_POST_PAINT, [](gcs_component_base* c, void* p){
      colour background_transparent = c->scheme()->grey3;
      background_transparent.a      = 255.f * 0.5f;
      render->outlined_rect(c->pos(true), c->size(), background_transparent);
      return true;
    });

    return canvas;
  }

  template<typename T>
  inline gcs_component_canvas* create_notify_history(gcs_component_base* comp, std::wstring name, vec3i pos, vec3i size){
    assert(comp != nullptr);
    gcs_component_canvas* tab_canvas   = gcs::canvas(comp, name, pos, size, false);
    gcs_component_canvas* draw_canvas  = gcs::canvas(tab_canvas, WXOR(L"List Window"), vec3i(2, 2), size - vec3i(0, 4), true);

    draw_canvas->add<T>()
      ->info()->disable_alpha_vis_fade();
    draw_canvas->add_module<gcs_component_scrollbar>();
    draw_canvas->info()->disable_alpha_vis_fade();
    {
      draw_canvas->add_hook(GCS_HOOK_POST_PAINT, [](gcs_component_base* c, void* p){
        colour background_transparent = c->scheme()->grey3;
        background_transparent.a = 255.f * 0.5f;

        render->outlined_rect(c->pos(true), c->size(), background_transparent);
        return true;
      });
    }

    return tab_canvas;
  }

  inline gcs_component_canvas* create_log_window(gcs_component_base* comp, c_gcs_component_log_window*& p, std::wstring name, vec3i pos, vec3i size){
    assert(comp != nullptr);
    gcs_component_canvas* tab_canvas   = gcs::canvas(comp, name, pos, size, false);
    gcs_component_canvas* draw_canvas  = gcs::canvas(tab_canvas, WXOR(L"List Window"), vec3i(2, 2), size - vec3i(0, 4), true);

    p = draw_canvas->add<c_gcs_component_log_window>();
    p->info()->disable_alpha_vis_fade();

    draw_canvas->add_module<gcs_component_scrollbar>();
    draw_canvas->info()->disable_alpha_vis_fade();
    {
      draw_canvas->add_hook(GCS_HOOK_POST_PAINT, [](gcs_component_base* c, void* p){
        colour background_transparent = c->scheme()->grey3;
        background_transparent.a = 255.f * 0.5f;


        render->outlined_rect(c->pos(true), c->size(), background_transparent);

        return true;
      });
    }

    return tab_canvas;
  }


}