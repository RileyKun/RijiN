#include "link.h"

bool setup(void* device){
  if(gen_internal->loaded)
    return true;

  GLOBAL_SETUP();
  RENDER_SETUP();

  // Setup trampoline addresses
  {
    global->d3d9_hook_trp                             = gen_internal->get_pkg(HASH("d3d9_hook"));
    global->create_move_hook_trp                      = gen_internal->get_pkg(HASH("create_move_hook"));
    global->override_view_hook_trp                    = gen_internal->get_pkg(HASH("override_view_hook"));
    global->engine_paint_hook_trp                     = gen_internal->get_pkg(HASH("engine_paint_hook"));
    global->paint_traverse_hook_trp                   = gen_internal->get_pkg(HASH("paint_traverse_hook"));
    global->run_command_hook_trp                      = gen_internal->get_pkg(HASH("run_command_hook"));
    global->post_think_hook_trp                       = gen_internal->get_pkg(HASH("post_think_hook"));
    global->base_animating_setupbones_hook_trp        = gen_internal->get_pkg(HASH("base_animating_setupbones_hook"));
    global->set_dormant_hook_trp                      = gen_internal->get_pkg(HASH("set_dormant_hook"));
    global->cnetchan_sendnetmsg_hook_trp              = gen_internal->get_pkg(HASH("cnetchan_sendnetmsg_hook"));
    global->frame_stage_notify_hook_trp               = gen_internal->get_pkg(HASH("frame_stage_notify_hook"));
    global->draw_model_execute_hook_trp               = gen_internal->get_pkg(HASH("draw_model_execute_hook"));
    global->fire_event_hook_trp                       = gen_internal->get_pkg(HASH("fire_event_hook"));
    global->cbaseclientstate_process_print_hook_trp   = gen_internal->get_pkg(HASH("cbaseclientstate_process_print_hook"));
    global->csdkplayer_firebullet_hook_trp            = gen_internal->get_pkg(HASH("csdkplayer_firebullet_hook"));
    global->csdkplayershared_setrecoil_hook_trp       = gen_internal->get_pkg(HASH("csdkplayershared_setrecoil_hook"));
  }

  // Setup interfaces
  {
    global->client          = utils::get_interface(WXOR(L"client.dll"), HASH("VClient017"));
    global->engine          = utils::get_interface(WXOR(L"engine.dll"), HASH("VEngineClient014"));
    global->prediction      = utils::get_interface(WXOR(L"client.dll"), HASH("VClientPrediction001"));
    global->engine_tool     = utils::get_interface(WXOR(L"engine.dll"), HASH("VENGINETOOL003"));
    global->entity_list     = utils::get_interface(WXOR(L"client.dll"), HASH("VClientEntityList003"));
    global->model_info      = utils::get_interface(WXOR(L"engine.dll"), HASH("VModelInfoClient006"));
    global->cvar            = utils::get_interface(WXOR(L"vstdlib.dll"), HASH("VEngineCvar004"));
    global->material_system = utils::get_interface(WXOR(L"materialsystem.dll"), HASH("VMaterialSystem080"));
    global->render_view     = utils::get_interface(WXOR(L"engine.dll"), HASH("VEngineRenderView014"));
    global->engine_model    = utils::get_interface(WXOR(L"engine.dll"), HASH("VEngineModel016"));
    global->trace           = utils::get_interface(WXOR(L"engine.dll"), HASH("EngineTraceClient003"));
    global->panel           = utils::get_interface(WXOR(L"vgui2.dll"), HASH("VGUI_Panel009"));
  }

  // Copy signature addresses and pointers
  {
    global->global_data         = gen_internal->get_pkg(HASH("globaldata"));
  }

  // Setup additional fonts
  {
    if(global->gui_font == nullptr)
      global->gui_font  = render->create_font(WXOR(L"Segoe UI"), FW_MEDIUM, CLEARTYPE_QUALITY);

    if(global->esp_font_small == nullptr)
      global->esp_font_small = render->create_font(WXOR(L"Small Fonts"), FW_MEDIUM, DEFAULT_QUALITY);

    if(global->esp_font_large == nullptr)
      global->esp_font_large = render->create_font(WXOR(L"Tahoma"), FW_SEMIBOLD, ANTIALIASED_QUALITY);

    assert(global->gui_font != nullptr);
    assert(global->esp_font_small != nullptr);
    assert(global->esp_font_large != nullptr);
  }


  lag_compensation->setup();

  // Create settings pointer
  config = new c_settings;

  // Create guis
  {
    render->screen_size = global->engine->get_screen_size();

    // Setup target window
    if(global->target_window == nullptr)
      global->target_window = FindWindowW(WXOR(L"Valve001"), WXOR(L"Battle Grounds III"));

    // Main GUI
    global->menu = gcs::add_scene<gcs_menu>();
  }

  if(!gen_internal->setup())
    return false;

  return true;
}

EXPORT HRESULT __fastcall d3d9_hook(void* ecx, void* edx, void* device, RECT* source_rect, RECT* dest_rect, HWND window_override, void* region){
  if(!setup(device))
    return D3D_OK;

  if(!gen_internal->has_sub_expired()){
    IDirect3DStateBlock9* state_block = render->begin();
    {
      render->screen_size = global->engine->get_screen_size();

      // Update input
      input_system->update();

      // Our code
      if(global->valid()){
        player_esp->run();
        world_esp->run();
      }

      // Render our menu
      gcs::render_scene(global->menu, math::mouse_pos(global->target_window), global->gui_font);
      global->menu->scheme()->main       = colour(config->visual.menu_foreground_colour);
      global->menu->scheme()->background = colour(config->visual.menu_background_colour);

      render->draw_watermark(global->gui_font, !global->engine->is_in_game() || global->menu_open);
    }
    render->end(state_block);
  }

  if(gen_internal->should_unload()){
    utils::lock_mouse(false);

    gen_internal->unload();
    gcs::free_memory_stack();
    lag_compensation->unload();
    render->unload();
  }

  return utils::call_fastcall<HRESULT, void*, RECT*, RECT*, HWND, void*>(gen_internal->decrypt_asset(global->d3d9_hook_trp), ecx, edx, device, source_rect, dest_rect, window_override, region);
}