#include "link.h"

bool OW_Init();
void OW_Unload();

bool setup(IDXGISwapChain* swap_chain){
  if(gen_internal->loaded)
    return true;

  // Hooks
  {
    global->d3d11_hook_trp = gen_internal->get_pkg(HASH("d3d11_hook"));
    global->access_level   = gen_internal->get_pkg(HASH("gen_access_level"));
  }

  // Signatures
  {
    global->user_cmd_func_address   = gen_internal->get_pkg(HASH("user_cmd_func_address"));
    global->key_func_address        = gen_internal->get_pkg(HASH("key_func_address"));
    global->key_func2_address       = gen_internal->get_pkg(HASH("key_func2_address"));
    global->glow_handle_player      = gen_internal->get_pkg(HASH("glow_handle_player"));
    global->mouse_move_func_address = gen_internal->get_pkg(HASH("mouse_move_func_address"));
    global->sens_func_address       = gen_internal->get_pkg(HASH("sens_func_address"));
    global->ray_trace               = gen_internal->get_pkg(HASH("ray_trace"));
    global->ray_add_filter          = gen_internal->get_pkg(HASH("ray_add_filter"));
    global->skill_struct            = gen_internal->get_pkg(HASH("skill_struct"));
  }

  // Easy way of checking if we are in the main menu or fully loaded
  
  if (global->user_cmd_func_address == 0)
  	return false;
  
  if(!gen_internal->setup())
    return false;

  RENDER_SETUP();
  render->setup(swap_chain);

  // Create fonts
  {
    global->font_default = render->create_font_from_memory(segoeui_compressed_data_base85, 18);
    global->font_esp_big = render->create_font_from_memory(tahomabd_compressed_data_base85, 12);

    render->build_all_fonts();
  }

  // Create settings pointer
  config = new c_settings;

  // Create guis
  {
    global->menu = gcs::add_scene<gcs_menu>();
  }

  return true;
}

EXPORT HRESULT __fastcall d3d11_hook(void* rcx, void* rdx, UINT SyncInterval, UINT Flags){
  if(!setup((IDXGISwapChain*)rcx))
    return global->d3d11_hook_trp != 0 ? utils::call_fastcall<HRESULT, UINT, UINT>(gen_internal->decrypt_asset(global->d3d11_hook_trp), rcx, rdx, SyncInterval, Flags) : 0;

  input_system->update();
  if(gen_internal->should_unload()){
    OW_Unload();

    gcs::free_memory_stack();
    gen_internal->unload();

    return 0;
  }

  render->begin();
  if(!gen_internal->has_sub_expired()){
    OW_Init();
    
    if (g_ow->init())
      g_ow->think();

    // Render our menu
    gcs::render_scene(global->menu, math::mouse_pos(render->desc.OutputWindow), global->font_default);
    global->menu->scheme()->main       = colour(config->visual.menu_foreground_colour);
    global->menu->scheme()->background = colour(config->visual.menu_background_colour);

    render->draw_watermark(global->font_default, true);
  }
  render->end();

  return utils::call_fastcall<HRESULT, UINT, UINT>(gen_internal->decrypt_asset(global->d3d11_hook_trp), rcx, rdx, SyncInterval, Flags);
}