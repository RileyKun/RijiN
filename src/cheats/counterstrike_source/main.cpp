#include "link.h"

bool setup(void* device){
  if(gen_internal->loaded)
    return true;

  GLOBAL_SETUP();
  RENDER_SETUP();
  RENDER_DEBUG_SETUP();
  CREATE_NOTIFICATION_SYSTEM();
  CREATE_BASE_INTERFACES();

  notify->setup_fonts();
  render_debug->register_ws2(&math::world2screen);

  // Setup trampoline addresses
  {
    global->d3d9_hook_trp                                  = GET_PKG("d3d9_hook");
    global->rtl_dispatch_exception_hook_trp                = GET_PKG("rtl_dispatch_exception_hook");
    global->create_move_hook_trp                           = GET_PKG("create_move_hook");
    global->override_view_hook_trp                         = GET_PKG("override_view_hook");
    global->engine_paint_hook_trp                          = GET_PKG("engine_paint_hook");
    global->cnetchan_sendnetmsg_hook_trp                   = GET_PKG("cnetchan_sendnetmsg_hook");
    global->cinput_get_user_cmd_hook_trp                   = GET_PKG("cinput_get_user_cmd_hook");
    global->run_command_hook_trp                           = GET_PKG("run_command_hook");
    global->post_think_hook_trp                            = GET_PKG("post_think_hook");
    global->base_animating_setupbones_hook_trp             = GET_PKG("base_animating_setupbones_hook");
    global->frame_stage_notify_hook_trp                    = GET_PKG("frame_stage_notify_hook");
    global->draw_model_execute_hook_trp                    = GET_PKG("draw_model_execute_hook");
    global->set_dormant_hook_trp                           = GET_PKG("set_dormant_hook");
    global->paint_traverse_hook_trp                        = GET_PKG("paint_traverse_hook");
    global->util_impacttrace_trp                           = GET_PKG("util_impacttrace");
    global->fire_event_hook_trp                            = GET_PKG("fire_event_hook");
    //global->cbaseclientstate_process_set_convar_hook_trp   = GET_PKG("cbaseclientstate_process_set_convar_hook");
    global->cbaseviewmodel_calcviewmodelview_hook_trp      = GET_PKG("cbaseviewmodel_calcviewmodelview_hook");
    global->check_for_pure_server_whitelist_hook_trp       = GET_PKG("check_for_pure_server_whitelist_hook");
    global->hud_chatline_insert_and_colorize_text_hook_trp = GET_PKG("hud_chatline_insert_and_colorize_text_hook");
    global->engine_get_player_info_hook_trp                = GET_PKG("engine_get_player_info_hook");
    global->s_startsound_hook_trp                          = GET_PKG("s_startsound_hook");
    global->ccsviewrender_perform_flashbangeffect_hook_trp = GET_PKG("ccsviewrender_perform_flashbangeffect_hook");
    global->clientmodecsnormal_canrecorddemo_hook_trp      = GET_PKG("clientmodecsnormal_canrecorddemo_hook");
    global->chlclient_createmove_push_rdi                  = GET_PKG("chlclient_createmove_push_rdi");
    global->cbaseentity_baseinterpolatepart1_hook_trp      = GET_PKG("cbaseentity_baseinterpolatepart1_hook");
    global->cbaseentity_estimateabsvelocity_hook_trp       = GET_PKG("cbaseentity_estimateabsvelocity_hook");
    global->crenderview_renderview_hook_trp                = GET_PKG("crenderview_renderview_hook");
    global->input_cam_is_thirdperson_hook_trp              = GET_PKG("input_cam_is_thirdperson_hook");
    global->is_blacklisted_hook_trp                        = GET_PKG("is_blacklisted_hook");
  }

  // Setup interfaces
  {
    global->client          = utils::get_interface(WXOR(L"client.dll"),         HASH("VClient017"));
    global->engine          = utils::get_interface(WXOR(L"engine.dll"),         HASH("VEngineClient014"));
    global->prediction      = utils::get_interface(WXOR(L"client.dll"),         HASH("VClientPrediction001"));
    global->engine_tool     = utils::get_interface(WXOR(L"engine.dll"),         HASH("VENGINETOOL003"));
    global->entity_list     = utils::get_interface(WXOR(L"client.dll"),         HASH("VClientEntityList003"));
    global->model_info      = utils::get_interface(WXOR(L"engine.dll"),         HASH("VModelInfoClient006"));
    global->cvar            = utils::get_interface(WXOR(L"vstdlib.dll"),        HASH("VEngineCvar004"));
    global->material_system = utils::get_interface(WXOR(L"materialsystem.dll"), HASH("VMaterialSystem080"));
    global->render_view     = utils::get_interface(WXOR(L"engine.dll"),         HASH("VEngineRenderView014"));
    global->engine_model    = utils::get_interface(WXOR(L"engine.dll"),         HASH("VEngineModel016"));
    global->trace           = utils::get_interface(WXOR(L"engine.dll"),         HASH("EngineTraceClient003"));
    global->panel           = utils::get_interface(WXOR(L"vgui2.dll"),          HASH("VGUI_Panel009"));
  }

  global->init(global->trace);

  // Copy signature addresses and pointers
  {
    global->global_data                                 = GET_PKG("globaldata");
    global->client_state                                = (s_client_state*)GET_PKG("client_state");

    global->input                                       = *(s_input**)(GET_PKG("iinput"));

    global->cl_runprediction                            = GET_PKG("cl_runprediction");
    global->md5_pseudorandom                            = GET_PKG("md5_pseudorandom");
    global->util_cliptrace_to_players                   = GET_PKG("util_cliptrace_to_players");

    global->keyvalues_init                     = GET_PKG("keyvalues_init");
    global->keyvalues_setname                  = GET_PKG("keyvalues_setname");
    global->keyvalues_setint                   = GET_PKG("keyvalues_setint");
    global->keyvalues_setfloat                 = GET_PKG("keyvalues_setfloat");
    global->keyvalues_setstring                = GET_PKG("keyvalues_setstring");
    global->vgui_drawhud_retaddr               = GET_PKG("vgui_drawhud_retaddr");
    global->fire_bullet                        = GET_PKG("fire_bullet");
    global->look_weapon_info_slot              = GET_PKG("look_weapon_info_slot");
    global->file_weapon_info_from_handle       = GET_PKG("file_weapon_info_from_handle");
    global->get_class_name_addr                = GET_PKG("get_class_name");
    global->set_abs_origin_addr                = GET_PKG("set_abs_origin");
    global->set_abs_angles_addr                = GET_PKG("set_abs_angles");
    global->intersect_inf_ray_with_sphere_addr = GET_PKG("intersect_inf_ray_with_sphere_addr");

    void* steam3client           = GET_PKG("steam3client_addr");
    assert(steam3client != nullptr);
    global->steamcontext      = steam3client;

    global->hud_element_addr     = GET_PKG("hud_element_addr");
    global->get_hud_element_addr = GET_PKG("get_hud_element_addr");
    assert(global->hud_element_addr != nullptr);
    global->hud_chat                  = utils::call_fastcall64<void*, i8*>(global->get_hud_element_addr, global->hud_element_addr, XOR("CHudChat"));

    global->chatprintf_retaddr1                     = GET_PKG("chatprintf_retaddr1");
    global->ctrace_filter_simple                    = GET_PKG("ctrace_filter_simple");
    global->baseentity_calculate_abs_velocity_addr  = GET_PKG("baseentity_calculate_abs_velocity_addr");
  }

  // Cvar
  {
    assert(global->cvar != nullptr);
    global->cvar->setup();
  }

  // Setup additional fonts
  {
    if(global->gui_font == nullptr)
      global->gui_font  = render->create_font(WXOR(L"Segoe UI"), FW_MEDIUM, CLEARTYPE_QUALITY);

    if(global->gui_font_emoji == nullptr)
      global->gui_font_emoji  = render->create_font(WXOR(L"Segoe UI Emoji"), FW_MEDIUM, CLEARTYPE_QUALITY);

    if(global->esp_font_small == nullptr)
      global->esp_font_small = render->create_font(WXOR(L"Small Fonts"), FW_MEDIUM, DEFAULT_QUALITY);

    if(global->esp_font_large == nullptr)
      global->esp_font_large = render->create_font(WXOR(L"Tahoma"), FW_SEMIBOLD, ANTIALIASED_QUALITY);

    render_debug->setup_fonts();
    assert(global->gui_font != nullptr);
    assert(global->esp_font_small != nullptr);
    assert(global->esp_font_large != nullptr);
  }

  lag_compensation = new c_lag_compensation;
  lag_compensation->setup();

  engine_prediction            = new c_engine_prediction;
  interp_manager               = new c_interpolation_manager;
  movement                     = new c_movement;
  command_manager              = new c_command_manager;
  packet_manager               = new c_packet_manager;
  cheat_detection              = new c_cheat_detection;
  thirdperson                  = new c_thirdperson;
  freecam                      = new c_freecam;
  acm                          = new c_anti_cheat_manager;
  privacy_mode                 = new c_privacy_mode;

  global->base_packet_manager    = (c_base_packet_manager*)packet_manager;
  global->base_lag_compensation  = (c_base_lag_compensation*)lag_compensation;
  global->base_cheat_detection   = (c_base_cheat_detection*)cheat_detection;
  global->base_acm               = (c_base_anti_cheat_manager*)acm;

  //global->base_log               = (c_base_log_manager*)logs;
  //global->base_dispatch_user_msg = (c_dispatch_user_message_features*)dispatch_user_msg;

  // Create settings pointer
  config = new c_settings;
  player_list_handler->alloc();

  // Create guis
  {
    render->screen_size = global->engine->get_screen_size();

    if(global->info_panel == nullptr){
      global->info_panel = gcs::add_scene<gcs_info_panel>();
      assert(global->info_panel != nullptr);
    }

    // Setup target window
    if(global->target_window == nullptr)
      global->target_window = FindWindowW(WXOR(L"Valve001"), WXOR(L"Counter-Strike Source - Direct3D 9 - 64 Bit"));

    // Main GUI
    global->menu = gcs::add_scene<gcs_menu>();
  }

  if(!gen_internal->setup())
    return false;

  notify->create(NOTIFY_TYPE_SUCCESS, WXOR(L"RijiN has successfully loaded! Press insert, fn+insert, or F3 to open the menu."), colour(255, 255, 255, 255));
  return true;
}

EXPORT HRESULT __fastcall d3d9_hook(void* device, RECT* source_rect, RECT* dest_rect, HWND window_override, void* region){
  if(!setup(device))
    return D3D_OK;

  if(render->handle_device_states(device, global->engine->get_screen_size()))
    return utils::call_fastcall64<HRESULT, RECT*, RECT*, HWND, void*>(gen_internal->decrypt_asset(global->d3d9_hook_trp), device, source_rect, dest_rect, window_override, region);

  if(!gen_internal->has_sub_expired()){
    IDirect3DStateBlock9* state_block = render->begin();
    {
      render->screen_size = global->engine->get_screen_size();

      // Update input
      input_system->update();
      cheat_detection->d3d9_hook();
      render_debug->render_objects();

      // Our code
      if(global->valid()){
        world_esp->run();
        player_esp->run();
      }

      cheat_detection->fetch_server_info();

      notify->update_theme(flt_array2clr(config->visual.menu_foreground_colour), flt_array2clr(config->visual.menu_background_colour));
      notify->draw_notifications();

      // Render our menu
      gcs::render_scene(global->info_panel, math::mouse_pos(global->target_window), global->gui_font);
      gcs::render_scene(global->menu, math::mouse_pos(global->target_window), global->gui_font, global->gui_font_emoji);

      global->menu->scheme()->main                 = colour(config->visual.menu_foreground_colour);
      global->menu->scheme()->background           = colour(config->visual.menu_background_colour);

      global->info_panel->scheme()->main           = colour(config->visual.menu_foreground_colour);
      global->info_panel->scheme()->background     = colour(config->visual.menu_background_colour);

      if(global->client_state != nullptr)
        render->draw_watermark(global->gui_font, !utils::is_in_game() || global->menu_open);
    }
    render->end(state_block);
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

  return utils::call_fastcall64<HRESULT, RECT*, RECT*, HWND, void*>(gen_internal->decrypt_asset(global->d3d9_hook_trp), device, source_rect, dest_rect, window_override, region);
}