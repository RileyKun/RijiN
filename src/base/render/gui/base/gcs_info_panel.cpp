#include "../../../link.h"

#if !defined(LOADER_CLIENT)
bool gcs_base_info_panel::setup(){
  vec3i menu_pos = vec3i(4, render->screen_size.y / 2);

  window = gcs::window(this, WXOR(L"Information"), menu_pos, window_size);
  assert(window != nullptr);
  {
    window->info()->disable_auto_positioning();
    window->add_hook(GCS_HOOK_SHOULD_ENABLE, [](gcs_component_base* c, void* p){
      gcs_base_info_panel* panel = (gcs_base_info_panel*)c->scene();
      assert(panel != nullptr);
      if(!panel->is_enabled())
        return false;

      if(!panel->is_menu_open() && !panel->is_in_game())
        return false;

      return true;
    });

    window->add_hook(GCS_HOOK_SHOULD_PAINT, [](gcs_component_base* c, void* p){
      gcs_base_info_panel* panel = (gcs_base_info_panel*)c->scene();
      assert(panel != nullptr);
      return panel->is_menu_open() || panel->is_in_game();
    });

    window->add_hook(GCS_HOOK_INPUT_ENABLED, [](gcs_component_base* c, void* p){
      gcs_base_info_panel* panel = (gcs_base_info_panel*)c->scene();
      assert(panel != nullptr);
      return panel->is_menu_open();
    });

    window->add_hook(GCS_HOOK_MAIN_PAINT, [](gcs_component_base* c, void* p){
      gcs_base_info_panel* panel = (gcs_base_info_panel*)c->scene();
      assert(panel != nullptr);
      c->info()->size = vec3i(100, 0);

      i32   processed_entries = 0;
      i32   name_offset       = 0;
      i32   state_offset      = 0;
      i32   padding           = 4;

      vec3i pos   = c->pos();

      for(i32 i = 0; i < 128; i++){
        s_info_panel_table* entry = &panel->table[i];
        if(entry == nullptr)
          continue;

        if(!entry->valid || entry->name[0] == L'\0')
          continue;

        if(entry->enabled_ptr != nullptr){
          if(*entry->enabled_ptr == false)
            continue;
        }

        colour clr    = entry->is_active() ? c->scheme()->text : c->scheme()->text * 0.75f;
        i32    offset = processed_entries > 0 ? c->size().y + 4 : 0;
        bool   s      = false;

        // Keep track of what has been processed so far.
        processed_entries++;

        // Name.
        vec3i size = c->font()->get_size(INFO_PANEL_FONT_SIZE, entry->name, &s);

        render->push_clip_region(pos + vec3i(padding, padding + offset), vec3i(112 - padding, size.y));
        {
          c->font()->draw(pos + vec3i(padding, padding + offset),
            INFO_PANEL_FONT_SIZE,
            entry->name,
            clr,
            TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_LEFT);
        }
        render->pop_clip_region();

        name_offset = size.x = 112 + padding;//math::biggest(size.x, 112);

        if(entry->key != nullptr){
          size.x += padding;
          vec3i state_size = c->font()->draw(pos + vec3i(size.x, offset + padding),
            INFO_PANEL_FONT_SIZE,
            entry->key->get_toggle_state_str().c_str(),
            c->scheme()->text,
            TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_LEFT);

          state_offset = size.x += math::biggest(state_size.x, 48);

          vec3i key_size = c->font()->draw(pos + vec3i(size.x + padding, padding + offset),
            INFO_PANEL_FONT_SIZE,
            entry->key->get_key_str().c_str(),
            entry->key->is_key_held() ? c->scheme()->success : clr,
            TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_LEFT);

          size.x += key_size.x + padding;
        }
        else{
          size.x += padding;
          vec3i state_size = c->font()->draw(pos + vec3i(size.x, offset + padding),
            INFO_PANEL_FONT_SIZE,
            entry->enabled ? WXOR(L"Active") : WXOR(L"Inactive"),
            c->scheme()->text,
            TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_LEFT);

          state_offset = size.x += math::biggest(state_size.x, 48);

          vec3i key_size = c->font()->draw(pos + vec3i(size.x + padding, padding + offset),
            INFO_PANEL_FONT_SIZE,
            WXOR(L"N/A"),
            clr,
            TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_LEFT);

          size.x += key_size.x + (padding);
        }

        if(size.x > c->info()->size.x)
          c->info()->size.x = size.x;

        c->info()->size.y += size.y;
      }

      if(c->info()->size.y > 0)
        c->info()->size.y += padding * 2;


      render->solid_line(pos + vec3i(name_offset, 0), pos + vec3i(name_offset, c->info()->size.y), c->scheme()->main);
      render->solid_line(pos + vec3i(state_offset, 0), pos + vec3i(state_offset, c->info()->size.y), c->scheme()->main);
      return true;
    });
  }

  return true;
}
#endif