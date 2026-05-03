#include "link.h"

bool setup(void* device){
  if(gen_internal->loaded)
    return true;

  GLOBAL_SETUP();
  RENDER_SETUP();
  RENDER_DEBUG_SETUP();
  CREATE_NOTIFICATION_SYSTEM();

  notify->setup_fonts();
  render_debug->register_ws2(&math::world2screen);
  render_debug->setup_fonts();

  // Setup trampoline addresses
  {
    global->d3d9_hook_trp                                          = gen_internal->get_pkg(HASH("d3d9_hook"));
    global->create_move_hook_trp                                   = gen_internal->get_pkg(HASH("create_move_hook"));
    global->override_view_hook_trp                                 = gen_internal->get_pkg(HASH("override_view_hook"));
    global->engine_paint_hook_trp                                  = gen_internal->get_pkg(HASH("engine_paint_hook"));
    global->cnetchan_sendnetmsg_hook_trp                           = gen_internal->get_pkg(HASH("cnetchan_sendnetmsg_hook"));
    global->cinput_get_user_cmd_hook_trp                           = gen_internal->get_pkg(HASH("cinput_get_user_cmd_hook"));
    global->run_command_hook_trp                                   = gen_internal->get_pkg(HASH("run_command_hook"));
    global->post_think_hook_trp                                    = gen_internal->get_pkg(HASH("post_think_hook"));
    global->base_animating_setupbones_hook_trp                     = gen_internal->get_pkg(HASH("base_animating_setupbones_hook"));
    global->frame_stage_notify_hook_trp                            = gen_internal->get_pkg(HASH("frame_stage_notify_hook"));
    global->draw_model_execute_hook_trp                            = gen_internal->get_pkg(HASH("draw_model_execute_hook"));
    global->set_dormant_hook_trp                                   = gen_internal->get_pkg(HASH("set_dormant_hook"));
    global->paint_traverse_hook_trp                                = gen_internal->get_pkg(HASH("paint_traverse_hook"));
    global->fire_event_hook_trp                                    = gen_internal->get_pkg(HASH("fire_event_hook"));
    global->cbaseclientstate_process_print_hook_trp                = gen_internal->get_pkg(HASH("cbaseclientstate_process_print_hook"));
    global->cbaseclientstate_process_set_convar_hook_trp           = gen_internal->get_pkg(HASH("cbaseclientstate_process_set_convar_hook"));
    global->lua_gamemode_callwithargs_hook_trp                     = gen_internal->get_pkg(HASH("lua_gamemode_callwithargs_hook"));
    global->lua_gamemode_callfinish_hook_trp                       = gen_internal->get_pkg(HASH("lua_gamemode_callfinish_hook"));
    global->lua_gamemode_callwithargs_str_hook_trp                 = gen_internal->get_pkg(HASH("lua_gamemode_callwithargs_str_hook"));
    global->lua_gamemode_call_str_hook_trp                         = gen_internal->get_pkg(HASH("lua_gamemode_call_str_hook"));
    global->lua_gamemode_call_hook_trp                             = gen_internal->get_pkg(HASH("lua_gamemode_call_hook"));
    global->do_post_screen_space_effects_hook_trp                  = gen_internal->get_pkg(HASH("do_post_screen_space_effects_hook"));
    global->crenderview_renderview_hook_trp                        = gen_internal->get_pkg(HASH("crenderview_renderview_hook"));
    global->render_capture_hook_trp                                = gen_internal->get_pkg(HASH("render_capture_hook"));
    global->should_draw_localplayer_hook_trp                       = gen_internal->get_pkg(HASH("should_draw_localplayer_hook"));
    global->fire_bullets_hook_trp                                  = gen_internal->get_pkg(HASH("fire_bullets_hook"));
    global->set_render_target_hook_trp                             = gen_internal->get_pkg(HASH("set_render_target_hook"));
    global->engine_client_cmd_hook_trp                             = gen_internal->get_pkg(HASH("engine_client_cmd_hook"));
    global->client_entity_list_on_add_entity_hook_trp              = gen_internal->get_pkg(HASH("client_entity_list_on_add_entity_hook"));
    global->client_entity_list_on_remove_entity_hook_trp           = gen_internal->get_pkg(HASH("client_entity_list_on_remove_entity_hook"));
    global->maintain_sequence_transition_hook_trp                  = gen_internal->get_pkg(HASH("maintain_sequence_transition_hook"));
    global->check_for_sequence_change_hook_trp                     = gen_internal->get_pkg(HASH("check_for_sequence_change_hook"));
    global->copy_render_target_to_texture_ex_hook_trp              = gen_internal->get_pkg(HASH("copy_render_target_to_texture_ex_hook"));
    global->copy_render_target_to_texture_hook_trp                 = gen_internal->get_pkg(HASH("copy_render_target_to_texture_hook"));
    global->get_render_target_hook_trp                             = gen_internal->get_pkg(HASH("get_render_target_hook"));
    global->rtl_dispatch_exception_hook_trp                        = gen_internal->get_pkg(HASH("rtl_dispatch_exception_hook"));
    global->push_render_target_and_viewport_p5_hook_trp            = gen_internal->get_pkg(HASH("push_render_target_and_viewport_p5_hook"));
    global->push_render_target_and_viewport_p1_hook_trp            = gen_internal->get_pkg(HASH("push_render_target_and_viewport_p1_hook"));
    global->get_local_view_angles_hook_trp                         = gen_internal->get_pkg(HASH("get_local_view_angles_hook"));
    global->cmutliplayeranimstate_update_hook_trp                  = gen_internal->get_pkg(HASH("cmutliplayeranimstate_update_hook"));
    global->cmutliplayeranimstate_restartgesture_hook_trp          = gen_internal->get_pkg(HASH("cmutliplayeranimstate_restartgesture_hook"));
    global->ivideowriter_addframe_hook_trp                         = gen_internal->get_pkg(HASH("ivideowriter_addframe_hook"));
    global->record_game_hook_trp                                   = gen_internal->get_pkg(HASH("record_game_hook"));
    global->shaderapi_read_pixels_hook_trp                         = gen_internal->get_pkg(HASH("shaderapi_read_pixels_hook"));
  }

  // Setup interfaces
  {
    global->client             = utils::get_interface(WXOR(L"client.dll"),         XOR("VClient017"));
    global->engine             = utils::get_interface(WXOR(L"engine.dll"),         XOR("VEngineClient015"));
    global->prediction         = utils::get_interface(WXOR(L"client.dll"),         XOR("VClientPrediction001"));
    global->engine_tool        = utils::get_interface(WXOR(L"engine.dll"),         XOR("VENGINETOOL003"));
    global->entity_list        = utils::get_interface(WXOR(L"client.dll"),         XOR("VClientEntityList003"));
    global->model_info         = utils::get_interface(WXOR(L"engine.dll"),         XOR("VModelInfoClient006"));
    global->cvar               = utils::get_interface(WXOR(L"vstdlib.dll"),        XOR("VEngineCvar007"));
    global->material_system    = utils::get_interface(WXOR(L"materialsystem.dll"), XOR("VMaterialSystem080"));
    global->render_view        = utils::get_interface(WXOR(L"engine.dll"),         XOR("VEngineRenderView014"));
    global->engine_model       = utils::get_interface(WXOR(L"engine.dll"),         XOR("VEngineModel016"));
    global->trace              = utils::get_interface(WXOR(L"engine.dll"),         XOR("EngineTraceClient003"));
    global->panel              = utils::get_interface(WXOR(L"vgui2.dll"),          XOR("VGUI_Panel009"));
    global->physics_surface    = utils::get_interface(WXOR(L"vphysics.dll"),       XOR("VPhysicsSurfaceProps001"));
    global->lua_shared         = utils::get_interface(WXOR(L"lua_shared.dll"),     XOR("LUASHARED003"));
    global->game_event_manager = utils::get_interface(WXOR(L"engine.dll"),         XOR("GAMEEVENTSMANAGER002"));
  }

  // Copy signature addresses and pointers
  {
    global->base_address_start  = gen_internal->get_pkg(HASH("base_address_start64"));
    global->base_address_end    = gen_internal->get_pkg(HASH("base_address_end64"));
    global->global_data         = gen_internal->get_pkg(HASH("globaldata"));
    global->client_state        = gen_internal->get_pkg(HASH("client_state"));

    global->cl_runprediction    = gen_internal->get_pkg(HASH("cl_runprediction"));

    global->keyvalues_init                   = gen_internal->get_pkg(HASH("keyvalues_init"));
    global->keyvalues_setname                = gen_internal->get_pkg(HASH("keyvalues_setname"));
    global->keyvalues_setint                 = gen_internal->get_pkg(HASH("keyvalues_setint"));
    global->keyvalues_setfloat               = gen_internal->get_pkg(HASH("keyvalues_setfloat"));
    global->keyvalues_setstring              = gen_internal->get_pkg(HASH("keyvalues_setstring"));
    global->vgui_drawhud_retaddr             = gen_internal->get_pkg(HASH("vgui_drawhud_retaddr"));
    global->localization_addr                = *(void**)gen_internal->get_pkg(HASH("localization_addr"));
    global->set_abs_origin_addr              = gen_internal->get_pkg(HASH("set_abs_origin"));
    global->set_abs_angles_addr              = gen_internal->get_pkg(HASH("set_abs_angles"));
    global->movie_name_addr                  = gen_internal->get_pkg(HASH("movie_name_addr"));
    global->prediction_random_seed           = gen_internal->get_pkg(HASH("prediction_random_seed"));
    global->md5_pseudorandom                 = gen_internal->get_pkg(HASH("md5_pseudorandom"));
    global->engine_client_cmd_retaddr        = gen_internal->get_pkg(HASH("engine_client_cmd_retaddr"));
    global->cviewrender_renderview_ret_addr1 = gen_internal->get_pkg(HASH("cviewrender_renderview_ret_addr1"));
    global->chlclient_createmove_push_rdi    = gen_internal->get_pkg(HASH("chlclient_createmove_push_rdi"));
    global->gmod_model_material_override     = gen_internal->get_pkg(HASH("gmod_model_material_override"));

    void* steam3client           = gen_internal->get_pkg(HASH("steam3client_addr"));
    assert(steam3client != nullptr);
    global->steamcontext         = utils::call_fastcall64_raw<void*>(steam3client);
  }

  // Cvar
  {
    global->cl_updaterate              = global->cvar->find_var(XOR("cl_updaterate"));
    global->cl_interp                  = global->cvar->find_var(XOR("cl_interp"));
    global->cl_interp_ratio            = global->cvar->find_var(XOR("cl_interp_ratio"));
    global->sv_client_min_interp_ratio = global->cvar->find_var(XOR("sv_client_min_interp_ratio"));
    global->sv_client_max_interp_ratio = global->cvar->find_var(XOR("sv_client_max_interp_ratio"));
    global->sv_gravity                 = global->cvar->find_var(XOR("sv_gravity"));

    global->sv_airaccelerate  = global->cvar->find_var(XOR("sv_airaccelerate"));
    global->sv_friction       = global->cvar->find_var(XOR("sv_friction"));
    global->sv_stopspeed      = global->cvar->find_var(XOR("sv_stopspeed"));
    global->sv_maxvelocity    = global->cvar->find_var(XOR("sv_maxvelocity"));
  }

  // Setup config.
  {
    // network
    global->engine->client_cmd(XOR("sv_maxcmdrate 66"));
    global->engine->client_cmd(XOR("sv_maxupdaterate 66"));
    global->engine->client_cmd(XOR("sv_client_min_interp_ratio 0"));

    global->engine->client_cmd(XOR("cl_updaterate 66"));
    global->engine->client_cmd(XOR("cl_cmdrate 66"));

    global->engine->client_cmd(XOR("rate 1048576"));
    global->engine->client_cmd(XOR("cl_interp 0.0"));
    global->engine->client_cmd(XOR("cl_interp_ratio 0"));
    global->engine->client_cmd(XOR("cl_smooth 0"));

    global->engine->client_cmd(XOR("cl_allowupload 0"));

    // rendering
    global->engine->client_cmd(XOR("gmod_mcore_test 0")); // if you find this, don't go around enabling it and then complaining about issues
    global->engine->client_cmd(XOR("mat_queue_mode -1"));
  }

  kernel_time_expired_return();

  // Setup additional fonts
  {
    if(global->gui_font == nullptr)
      global->gui_font  = render->create_font(WXOR(L"Segoe UI"), FW_MEDIUM, CLEARTYPE_QUALITY);

    if(global->gui_font_emoji == nullptr)
      global->gui_font_emoji = render->create_font(WXOR(L"Segoe UI Emoji"), FW_MEDIUM, CLEARTYPE_QUALITY);

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

  player_list_handler->alloc();
  entity_list_data->reset();

  // Create guis
  {
    render->screen_size = global->engine->get_screen_size();
    if(global->info_panel == nullptr){
      global->info_panel = gcs::add_scene<gcs_info_panel>();
      assert(global->info_panel != nullptr);
    }

    // Setup target window
    if(global->target_window == nullptr)
      global->target_window = FindWindowW(WXOR(L"Valve001"), WXOR(L"Garry's Mod (x64)"));

    // Main GUI
    global->menu = gcs::add_scene<gcs_menu>();
  }

  if(!gen_internal->setup())
    return false;

  notify->create(NOTIFY_TYPE_INFO, WXOR(L"Loaded."), colour(255, 255, 255, 255), 36);
  return true;
}

EXPORT HRESULT __fastcall d3d9_hook(void* device, RECT* source_rect, RECT* dest_rect, HWND window_override, void* region){
  if(!setup(device))
    return D3D_OK;

  if(!gen_internal->has_sub_expired()){
    IDirect3DStateBlock9* state_block = render->begin();

    // I don't know if this would be detectable or not.
    // But gmod lua plugins can abuse the SetRenderTarget function to prevent this hook from being able to render anything.
    // I have the plugin that does this if you wish to fix it out right so we don't have to this.

    c_texture* old_render_target = nullptr;
    if(global->material_system != nullptr){
      c_render_context* rc = global->material_system->get_render_context();
      if(rc != nullptr){
        rc->begin_render();
        old_render_target = rc->get_render_target();
        rc->set_render_target(nullptr);
        rc->end_render();
      }
    }

    {
      render->screen_size = global->engine->get_screen_size();

      // Update input
      input_system->update();

      // Our code
      if(global->valid()){
        render_debug->render_objects();
      
        player_esp->run();
        world_esp->run();
      
        if(config->aimbot.enabled && global->engine->is_in_game()){
          c_base_player* localplayer = utils::localplayer();

          if(localplayer != nullptr && localplayer->is_alive()){
            c_base_weapon* weapon = localplayer->get_weapon();
            if(weapon != nullptr){
              if(!weapon->is_melee() && !weapon->is_harmless_weapon()){
                float fov_rad = config->aimbot.max_fov;
                      fov_rad = math::clamp(fov_rad / global->game_fov, 0.f, 1.5f) * (render->screen_size.x * 0.5f);
  
                if(fov_rad > 0.f){
                  if(config->visual.aimbot_fov_circle_enabled){
                    if(config->visual.aimbot_fov_circle_outlines){
                      render->outlined_circle(vec3i(render->screen_size.x * 0.5f, render->screen_size.y * 0.5f), fov_rad + 1.f, 150.f, rgb(0,0,0));
                      render->outlined_circle(vec3i(render->screen_size.x * 0.5f, render->screen_size.y * 0.5f), fov_rad - 1.f, 150.f, rgb(0,0,0));
                    }
  
                    render->outlined_circle(vec3i(render->screen_size.x * 0.5f, render->screen_size.y * 0.5f), fov_rad, 150.f, flt_array2clr(config->visual.aimbot_fov_circle_colour));
                  }
                }
              }
            }
          }
        }
      }

      notify->update_theme(flt_array2clr(config->visual.menu_foreground_colour), flt_array2clr(config->visual.menu_background_colour));
      notify->draw_notifications();

      // Render our menu
      gcs::render_scene(global->info_panel, math::mouse_pos(global->target_window), global->gui_font);  
      gcs::render_scene(global->menu, math::mouse_pos(global->target_window), global->gui_font, global->gui_font_emoji);

      global->menu->scheme()->main       = colour(config->visual.menu_foreground_colour);
      global->menu->scheme()->background = colour(config->visual.menu_background_colour);

      global->info_panel->scheme()->main           = colour(config->visual.menu_foreground_colour);
      global->info_panel->scheme()->background     = colour(config->visual.menu_background_colour);

      render->draw_watermark(global->gui_font, !global->engine->is_in_game() || global->menu_open);
    }
    render->end(state_block);

    if(global->material_system != nullptr){
      c_render_context* rc = global->material_system->get_render_context();
      if(rc != nullptr){
        rc->begin_render();
        rc->set_render_target(old_render_target);
        rc->end_render();
      }
    }
  }

#if defined(DEV_MODE)
  if(gen_internal->should_unload()){
    global->unloading = true;
    utils::lock_mouse(false);

    player_list_handler->dealloc();
    gen_internal->unload();
    gcs::free_memory_stack();
    lag_compensation->unload();
    render->unload();

    if(global->game_event_manager != nullptr)
      global->game_event_manager->remove_listener();
  }
#endif

  return utils::call_fastcall64<HRESULT, RECT*, RECT*, HWND, void*>(gen_internal->decrypt_asset(global->d3d9_hook_trp), device, source_rect, dest_rect, window_override, region);
}