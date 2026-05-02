#pragma once

// Keep game only related functions outta here.

namespace gcsf{
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

  inline void add_key_bind_control(gcs_component_group* group,
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
};