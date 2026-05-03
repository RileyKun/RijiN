#include "link.h"

bool setup(void* device){
  if(gen_internal->loaded)
    return true;

  GLOBAL_SETUP();
  RENDER_SETUP();
  RENDER_DEBUG_SETUP();
  CREATE_NOTIFICATION_SYSTEM();
  CREATE_BASE_INTERFACES();
 
  render_debug->register_ws2(&math::world2screen);

  notify->setup_fonts();
  render_debug->setup_fonts();

  // Setup trampoline addresses
  {
    global->d3d9_hook_trp                                   = gen_internal->get_pkg(HASH("d3d9_hook"));
    global->d3d9_reset_trp                                  = gen_internal->get_pkg(HASH("d3d9_reset_hook"));
    global->create_move_hook_trp                            = gen_internal->get_pkg(HASH("create_move_hook"));
    global->override_view_hook_trp                          = gen_internal->get_pkg(HASH("override_view_hook"));
    global->engine_paint_hook_trp                           = gen_internal->get_pkg(HASH("engine_paint_hook"));
    global->cnetchan_sendnetmsg_hook_trp                    = gen_internal->get_pkg(HASH("cnetchan_sendnetmsg_hook"));
    global->cinput_get_user_cmd_hook_trp                    = gen_internal->get_pkg(HASH("cinput_get_user_cmd_hook"));
    global->run_command_hook_trp                            = gen_internal->get_pkg(HASH("run_command_hook"));
    global->post_think_hook_trp                             = gen_internal->get_pkg(HASH("post_think_hook"));
    global->base_animating_setupbones_hook_trp              = gen_internal->get_pkg(HASH("base_animating_setupbones_hook"));
    global->frame_stage_notify_hook_trp                     = gen_internal->get_pkg(HASH("frame_stage_notify_hook"));
    global->server_adjust_player_time_base_hook_trp         = gen_internal->get_pkg(HASH("server_adjust_player_time_base_hook"));
    global->run_simulation_hook_trp                         = gen_internal->get_pkg(HASH("run_simulation_hook"));
    global->clock_drift_manager_set_server_tick_hook_trp    = gen_internal->get_pkg(HASH("clock_drift_manager_set_server_tick_hook"));
    global->draw_model_execute_hook_trp                     = gen_internal->get_pkg(HASH("draw_model_execute_hook"));
    global->studio_render_forced_material_override_hook_trp = gen_internal->get_pkg(HASH("studio_render_forced_material_override_hook"));
    global->render_context_set_stencil_state_hook_trp       = gen_internal->get_pkg(HASH("render_context_set_stencil_state_hook"));
    global->set_dormant_hook_trp                            = gen_internal->get_pkg(HASH("set_dormant_hook"));
    global->fire_event_hook_trp                             = gen_internal->get_pkg(HASH("fire_event_hook"));
    global->paint_traverse_hook_trp                         = gen_internal->get_pkg(HASH("paint_traverse_hook"));
    global->do_post_screen_space_effects_hook_trp           = gen_internal->get_pkg(HASH("do_post_screen_space_effects_hook"));
    global->int_decode_hook_trp                             = gen_internal->get_pkg(HASH("int_decode_hook"));
    global->apply_entity_glow_effects_hook_trp              = gen_internal->get_pkg(HASH("apply_entity_glow_effects_hook"));
    global->trace_ray_hook_trp                              = gen_internal->get_pkg(HASH("trace_ray_hook"));
    global->cmd_executecommand_hook_trp                     = gen_internal->get_pkg(HASH("cmd_executecommand_hook"));
    global->crenderview_renderview_hook_trp                 = gen_internal->get_pkg(HASH("crenderview_renderview_hook"));
    global->should_draw_localplayer_hook_trp                = gen_internal->get_pkg(HASH("should_draw_localplayer_hook"));
    global->cam_isthirdperson_hook_trp                      = gen_internal->get_pkg(HASH("cam_isthirdperson_hook"));
    global->cbaseviewmodel_calcviewmodelview_hook_trp       = gen_internal->get_pkg(HASH("cbaseviewmodel_calcviewmodelview_hook"));
    global->cbaseentity_baseinterpolatepart1_hook_trp       = gen_internal->get_pkg(HASH("cbaseentity_baseinterpolatepart1_hook"));


    global->set_abs_origin                                  = gen_internal->get_pkg(HASH("set_abs_origin"));
    global->set_abs_angles                                  = gen_internal->get_pkg(HASH("set_abs_angles"));
    global->baseentity_calculate_abs_velocity_addr          = gen_internal->get_pkg(HASH("baseentity_calculate_abs_velocity_addr"));
  }

  // Setup interfaces
  {
    global->client          = utils::get_interface(WXOR(L"client.dll"), HASH("VClient016"));
    global->engine          = utils::get_interface(WXOR(L"engine.dll"), HASH("VEngineClient013"));
    global->prediction      = utils::get_interface(WXOR(L"client.dll"), HASH("VClientPrediction001"));
    global->engine_tool     = utils::get_interface(WXOR(L"engine.dll"), HASH("VENGINETOOL003"));
    global->entity_list     = utils::get_interface(WXOR(L"client.dll"), HASH("VClientEntityList003"));
    global->model_info      = utils::get_interface(WXOR(L"engine.dll"), HASH("VModelInfoClient004"));
    global->cvar            = utils::get_interface(WXOR(L"vstdlib.dll"), HASH("VEngineCvar007"));
    global->material_system = utils::get_interface(WXOR(L"materialsystem.dll"), HASH("VMaterialSystem080"));
    global->render_view     = utils::get_interface(WXOR(L"engine.dll"), HASH("VEngineRenderView013"));
    global->engine_model    = utils::get_interface(WXOR(L"engine.dll"), HASH("VEngineModel016"));
    global->trace           = utils::get_interface(WXOR(L"engine.dll"), HASH("EngineTraceClient003"));
    global->panel           = utils::get_interface(WXOR(L"vgui2.dll"), HASH("VGUI_Panel009"));
  }

  // Copy signature addresses and pointers
  {
    global->server_global_data                      = **(s_global_data***)((u32)gen_internal->get_pkg(HASH("server_globaldata")));
    global->client_state                            = (s_client_state*)((u32)(**(void***)(gen_internal->get_pkg(HASH("client_state")))) + 0x8);
    global->clc_move_rtti                           = *(void**)((u32)gen_internal->get_pkg(HASH("clc_move_rtti")) + 0x6); //*(void**)(gen_internal->get_pkg(HASH("clc_move_rtti")) + 0x3);
    global->input                                   = **(s_input***)gen_internal->get_pkg(HASH("iinput"));
    global->get_sequence_name                       = gen_internal->get_pkg(HASH("get_sequence_name"));
    global->cl_runprediction                        = gen_internal->get_pkg(HASH("cl_runprediction"));

    global->shared_random_float_addr                = gen_internal->get_pkg(HASH("shared_random_float_addr"));

    global->test_swing_collision                    = gen_internal->get_pkg(HASH("test_swing_collision"));
    global->cam_isthirdperson_retaddr_override_view = gen_internal->get_pkg(HASH("cam_isthirdperson_retaddr_override_view"));
  }

  global->init(global->trace);
  global->cvar->setup();

  global->steamcontext                            = utils::call_stdcall<void*>(global->steam3client_addr);

  // Setup additional fonts
  if(global->gui_font == nullptr)
    global->gui_font  = render->create_font(WXOR(L"Segoe UI"), FW_MEDIUM, CLEARTYPE_QUALITY);

  if(global->gui_font_emoji == nullptr)
    global->gui_font_emoji = render->create_font(WXOR(L"Segoe UI Emoji"), FW_MEDIUM, CLEARTYPE_QUALITY);

  if(global->esp_font_small == nullptr)
    global->esp_font_small = render->create_font(WXOR(L"Small Fonts"), FW_MEDIUM, DEFAULT_QUALITY);

  if(global->esp_font_large == nullptr)
    global->esp_font_large = render->create_font(WXOR(L"Tahoma"), FW_SEMIBOLD, ANTIALIASED_QUALITY);

  lag_compensation  = new c_lag_compensation;
  engine_prediction = new c_engine_prediction;
  packet_manager    = new c_packet_manager;
  thirdperson       = new c_thirdperson;
  viewmodel         = new c_viewmodel_offset;
  cheat_detection   = new c_cheat_detection;
  interp_manager    = new c_interpolation_manager;

  lag_compensation->setup();

  global->base_lag_compensation = lag_compensation;

  player_list_handler->alloc();

  // Create settings pointer
  config = new c_settings;

  // Create guis
  {
    render->screen_size = global->engine->get_screen_size();

    // Setup target window
    if(global->target_window == nullptr)
      global->target_window = FindWindowW(WXOR(L"Valve001"), WXOR(L"Left 4 Dead 2 - Direct3D 9"));

    // Main GUI
    global->menu = gcs::add_scene<gcs_menu>();
  }

  if(!gen_internal->setup())
    return false;

  DBG("[+] finished setup\n");
  notify->create(NOTIFY_TYPE_INFO, WXOR(L"Loaded."));
  return true;
}

EXPORT HRESULT __fastcall d3d9_hook(void* ecx, void* edx, void* device, RECT* source_rect, RECT* dest_rect, HWND window_override, void* region){
  if(!setup(device))
    return utils::call_fastcall<HRESULT, void*, RECT*, RECT*, HWND, void*>(gen_internal->decrypt_asset(global->d3d9_hook_trp), ecx, edx, device, source_rect, dest_rect, window_override, region);

  kernel_time_expired_close();
  if(render->handle_device_states(device, global->engine->get_screen_size()))
    return utils::call_fastcall<HRESULT, void*, RECT*, RECT*, HWND, void*>(gen_internal->decrypt_asset(global->d3d9_hook_trp), ecx, edx, device, source_rect, dest_rect, window_override, region);

  if(!gen_internal->has_sub_expired()){
    IDirect3DStateBlock9* state_block = render->begin();
    if(state_block != nullptr){
      render->screen_size = global->engine->get_screen_size();

      // Update input
      input_system->update();
      cheat_detection->d3d9_hook();
      cheat_detection->fetch_server_info();

      render_debug->render_objects();

      // Our code
      world_esp->run();
      player_esp->run();

      notify->update_theme(flt_array2clr(config->visual.menu_foreground_colour), flt_array2clr(config->visual.menu_background_colour));
      notify->draw_notifications();

      // Render our menu
      gcs::render_scene(global->menu, math::mouse_pos(global->target_window), global->gui_font, global->gui_font_emoji);
      global->menu->scheme()->main       = colour(config->visual.menu_foreground_colour);
      global->menu->scheme()->background = colour(config->visual.menu_background_colour);

      render->draw_watermark(global->gui_font, !global->engine->is_in_game() || global->menu_open);
    }

    render->end(state_block);
    kernel_time_24h_close();
  }

#if defined(DEV_MODE)
  if(gen_internal->should_unload()){
    utils::lock_mouse(false);

    gen_internal->unload();
    gcs::free_memory_stack();
    lag_compensation->unload();
    render->unload();
    player_list_handler->dealloc();
  }
#endif

  return utils::call_fastcall<HRESULT, void*, RECT*, RECT*, HWND, void*>(gen_internal->decrypt_asset(global->d3d9_hook_trp), ecx, edx, device, source_rect, dest_rect, window_override, region);
}

EXPORT HRESULT __fastcall d3d9_reset_hook(void* ecx, void* edx, void* param, void* fullscreen_display_mode){
  DBG("[!] d3d9 reset!\n");

  // The source engine is fucking retarded.
  // The 2 render targets we're creating are causing issues. (For glow)

  render->is_device_resetting = true;
  render->on_device_lost();
  HRESULT result = utils::call_fastcall<HRESULT, void*, void*>(gen_internal->decrypt_asset(global->d3d9_reset_trp), ecx, edx, param, fullscreen_display_mode);
  render->on_device_reset();
  render->is_device_resetting = false;

  if(result == D3D_OK){
    DBG("[!] d3d9::resetex OK\n");
    render->needs_to_wait = false;
  }
  else if(result == D3DERR_DEVICELOST){
    DBG("[!] d3d9::resetex (D3DERR_DEVICELOST)\n");
  }
  else if(result == D3DERR_DEVICEHUNG){
    DBG("[!] d3d9::resetex (D3DERR_DEVICEHUNG)\n");
  }
  else if(result == D3DERR_INVALIDCALL){
    DBG("[!] d3d9::resetex (D3DERR_INVALIDCALL)\n");
  }
  else{
    DBG("[-] d3d9::resetex failed: 0x%X\n", result);
  }

  render->device_reset = result == D3D_OK;
  render->needs_to_wait = result != D3D_OK;
  return result;
}