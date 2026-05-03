#include "../../../link.h"

bool gcs_spectator_list_panel::setup(){
  vec3i menu_pos = vec3i(4, render->screen_size.y / 4);

  window = gcs::window(this, WXOR(L"Spectators"), menu_pos, window_size);
  assert(window != nullptr);
  {
    window->info()->disable_auto_positioning();

    window->add_hook(GCS_HOOK_SHOULD_ENABLE, [](gcs_component_base* c, void* p){
      gcs_spectator_list_panel* panel = ( gcs_spectator_list_panel*)c->scene();
      if(!panel->is_enabled())
        return false;

      if(!global_internal->menu_open && !utils::is_in_game())
        return false;

      return true;
    });

    window->add_hook(GCS_HOOK_SHOULD_PAINT, [](gcs_component_base* c, void* p){
      return global_internal->menu_open || utils::is_in_game();
    });

    window->add_hook(GCS_HOOK_INPUT_ENABLED, [](gcs_component_base* c, void* p){
      return global_internal->menu_open;
    });

    // Not thread safe for game functions.
    window->add_hook(GCS_HOOK_MAIN_PAINT, [](gcs_component_base* c, void* p){
      gcs_spectator_list_panel* panel = (gcs_spectator_list_panel*)c->scene();

      const bool is_in_game = utils::is_in_game();
      static bool was_in_game = is_in_game;
      if(!is_in_game){
        gcs_spectator_list_panel* panel = (gcs_spectator_list_panel*)c->scene();
        if(was_in_game){
          panel->reset_data();
          was_in_game = false;
        }
      }
      else
        was_in_game = true;

      if(!is_in_game && !global_internal->menu_open)
        return true;

      i32 padding = 4;
      c->info()->size = vec3i(150, 0);
      for(i32 i = 1; i <= global_internal->global_data->max_clients; i++){
        s_spectator_data* data = &panel->data[i];
        if(!data->valid)
          continue;

        static wchar_t buf[1024];
        I(wsprintfW)(buf, WXOR(L"(%ls) %s → %s"), data->observer_str, data->spectator_name, data->target_name);

        colour clr = c->scheme()->text;
        if(data->spectating_localplayer)
          clr = data->observer_mode == OBS_MODE_IN_EYE ? c->scheme()->error : c->scheme()->warning;

        vec3i size = c->font()->draw(c->pos() + vec3i(padding, padding + c->size().y), 14, buf, clr, TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_LEFT);

        if(size.x > c->info()->size.x)
          c->info()->size.x = size.x;

        c->info()->size.y += size.y;
      }


      if(c->info()->size.x != 150)
        c->info()->size.x += padding * 2;

      if(c->info()->size.y > 0)
        c->info()->size.y += padding * 2;

      return true;
    });
  }

  return true;
}