#include "../../link.h"

bool gcs::render_scene( gcs_scene* scene, vec3i mouse_pos, c_font_base* font, c_font_base* emoji_font ){
  if ( scene == nullptr )
    return false;

  if (!scene->valid())
    return false;

  if(scene->get_scale() == 0.0f)
    scene->set_scale(render->scale());
  else if(scene->get_scale() != render->scale()){

    // TODO
    // This will free up memory, but gcs_scene or something still leaks memory. Not as bad as before though.
    // Should try and fully fix it.
    gcs::free_memory_stack();

    // We have to call setup again since if we don't everything will be fucked.
    scene->setup();
    scene->set_scale(render->scale());
  }

  gcs_scene_info* _info = scene->info( );
  {
    _info->help_text  = L"";
    _info->mouse_pos  = mouse_pos;
    _info->font       = font;
    _info->font_emoji = emoji_font;
    _info->update_input();
    _info->curtime    = math::time(false);
  }

  gcs_component_base* root_comp = _info->root_component;
  {
    root_comp->set_parent(nullptr);
    root_comp->set_scene(scene);
  }

  float t1 = 0.f;
  if(_info->perf_enabled)
    t1 = _info->curtime;

  bool ret = root_comp->run( );

  scene->draw_help_text();

  // Increment scene framecount
  _info->framecount++;

  if(_info->perf_enabled)
    _info->frametime = abs(math::time(false) - t1);
  else
    _info->frametime = 0.f;

  return ret;
}