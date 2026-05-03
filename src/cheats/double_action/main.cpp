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
    global->client_state        = *(s_client_state**)gen_internal->get_pkg(HASH("client_state"));
    //global->clc_move_rtti       = *(void**)(gen_internal->get_pkg(HASH("clc_move_rtti")) + 0x3);
    //global->input               = **(s_input***)gen_internal->get_pkg(HASH("iinput"));
    //global->get_sequence_name   = gen_internal->get_pkg(HASH("get_sequence_name"));
    global->cl_runprediction    = gen_internal->get_pkg(HASH("cl_runprediction"));

    global->keyvalues_init               = gen_internal->get_pkg(HASH("keyvalues_init"));
    global->keyvalues_setname            = gen_internal->get_pkg(HASH("keyvalues_setname"));
    global->keyvalues_setint             = gen_internal->get_pkg(HASH("keyvalues_setint"));
    global->keyvalues_setfloat           = gen_internal->get_pkg(HASH("keyvalues_setfloat"));
    global->keyvalues_setstring          = gen_internal->get_pkg(HASH("keyvalues_setstring"));

    global->look_weapon_info_slot        = gen_internal->get_pkg(HASH("look_weapon_info_slot"));
    global->file_weapon_info_from_handle = gen_internal->get_pkg(HASH("file_weapon_info_from_handle"));
    global->get_class_name_addr          = gen_internal->get_pkg(HASH("get_class_name"));
    global->set_abs_origin_addr          = gen_internal->get_pkg(HASH("set_abs_origin"));
    global->set_abs_angles_addr          = gen_internal->get_pkg(HASH("set_abs_angles"));
    global->prediction_random_seed       = *(i32*)gen_internal->get_pkg(HASH("prediction_random_seed"));

    void* steam3client           = gen_internal->get_pkg(HASH("steam3client_addr"));
    assert(steam3client != nullptr);
    global->steamcontext         = utils::call_stdcall<void*>(steam3client);

    global->vgui_drawhud_retaddr  = gen_internal->get_pkg(HASH("vgui_drawhud_retaddr"));
    global->cweaponpistol_updatepenaltytime_addr      = gen_internal->get_pkg(HASH("cweaponpistol_updatepenaltytime_addr"));
  }

  // Cvar
  {
    // network / interp settings.
    global->cvar->enforce_var(XOR("rate"), XOR("1048576"), true, 80000.f, 1048576.f);
    global->cvar->enforce_var(XOR("cl_cmdrate"), XOR("66"), true, 66.f, 100.f);
    global->cvar->enforce_var(XOR("cl_updaterate"), XOR("66"), true, 66.f, 100.f);
    global->cvar->enforce_var(XOR("cl_interp_ratio"), XOR("1"), true, 1.f, 1.f);
    global->cvar->enforce_var(XOR("cl_interp"), XOR("0.0"), true, 0.f, 0.f);
    global->cvar->enforce_var(XOR("cl_extrapolate"), XOR("0"), true, 0.f, 0.f);
    global->cvar->enforce_var(XOR("cl_extrapolate_amount"), XOR("0"), true, 0.f, 0.f);
    global->cvar->enforce_var(XOR("cl_smooth"), XOR("0"), true, 0.f, 0.f);
    global->cvar->enforce_var(XOR("cl_predict"), XOR("1"), true, 1, 1);

    // net commands.
    global->cvar->enforce_var(XOR("net_blockmsg"), "");
    global->cvar->enforce_var(XOR("net_splitrate"), XOR("1"), true, 1.f, 1.f);
    global->cvar->enforce_var(XOR("net_compresspackets"), XOR("1"), true, 1.f, 1.f);
    global->cvar->enforce_var(XOR("net_compresspackets_minsize"), XOR("1024"), true, 1024.f, 1024.f);
    global->cvar->enforce_var(XOR("net_maxroutable"), XOR("1260"), true, 1260.f, 1260.f);
    global->cvar->enforce_var(XOR("net_maxfragments"), XOR("1260"), true, 1260.f, 1260.f);
    global->cvar->enforce_var(XOR("net_splitpacket_maxrate"), XOR("80000"), true, 80000.f, 80000.f);
    global->cvar->enforce_var(XOR("net_maxcleartime"), XOR("4.0"), true, 4.0f, 4.0f);
    global->cvar->enforce_var(XOR("net_maxpacketdrop"), XOR("5000"), true, 5000.f, 5000.f);

    // client comands.
    global->cvar->enforce_var(XOR("cl_ask_favorite_opt_out"), XOR("1"));
    global->cvar->enforce_var(XOR("cl_pred_optimize"), XOR("2"));
    global->cvar->enforce_var(XOR("cl_vote_ui_active_after_voting"), XOR("1"), true, 1.f, 1.f);
    global->cvar->enforce_var(XOR("cl_ragdoll_fade_time"), XOR("1"), true, 1.f, 30.f); // Required.
    global->cvar->enforce_var(XOR("cl_ragdoll_forcefade"), XOR("0"), true, 0.f, 0.f); // Required.
    global->cvar->enforce_var(XOR("cl_ragdoll_physics_enable"), XOR("1"), true, 1.f, 1.f); // Required.

    // server commands.
    global->cvar->enforce_var(XOR("sv_lan"), XOR("1"));
    global->cvar->enforce_var(XOR("sv_maxcmdrate"), XOR("66"));
    global->cvar->enforce_var(XOR("sv_maxupdaterate"), XOR("66"));
    global->cvar->enforce_var(XOR("sv_maxupdaterate"), XOR("66"));
    global->cvar->enforce_var(XOR("sv_client_min_interp_ratio"), XOR("-1"));
    global->cvar->enforce_var(XOR("sv_maxrate"), XOR("0"));
    global->cvar->enforce_var(XOR("sv_minrate"), XOR("80000"));
    //global->cvar->enforce_var(XOR("sv_showimpacts"), XOR("0"), true, 0.f, 1.f); // Autowall will cause issues.

    // Rendering commands.
    global->cvar->enforce_var(XOR("r_drawtracers_firstperson"), XOR("1"), true, 1.f, 1.f);
    global->cvar->enforce_var(XOR("r_drawtracers"), XOR("1"), true, 1.f, 1.f);
    global->cvar->enforce_var(XOR("r_drawmodeldecals"), XOR("0"));

    // Threading commands.
    global->cvar->enforce_var(XOR("mat_queue_mode"), XOR("2"), true, 2.f, 2.f);
    global->cvar->enforce_var(XOR("r_threaded_particles"), XOR("1"));
    global->cvar->enforce_var(XOR("r_threaded_renderables"), XOR("1"));
    global->cvar->enforce_var(XOR("cl_threaded_bone_setup"), XOR("1"), false, 0.f, 1.f, true);
    global->cvar->enforce_var(XOR("r_hunkalloclightmaps"), XOR("0"));

    // misc commands.
    global->cvar->enforce_var(XOR("ragdoll_sleepaftertime"), XOR("1.0f"), true, 1.0f, 5.0f); // Ragdolls will sleep after 1 second of being simulated.
    global->cvar->enforce_var(XOR("glow_outline_effect_enable"), XOR("0"), true, 0.f, 0.f);
    global->cvar->enforce_var(XOR("cl_lagcompensation"), XOR("1"), true, 1.f, 1.f); // We have idiots turning off lag compensation now like what?
    global->cvar->enforce_var(XOR("da_data_enabled"), XOR("0"), true, 0.f, 0.f);
    global->cl_updaterate              = global->cvar->find_var(XOR("cl_updaterate"));
    global->cl_interp                  = global->cvar->find_var(XOR("cl_interp"));
    global->cl_interp_ratio            = global->cvar->find_var(XOR("cl_interp_ratio"));
    global->sv_client_min_interp_ratio = global->cvar->find_var(XOR("sv_client_min_interp_ratio"));
    global->sv_client_max_interp_ratio = global->cvar->find_var(XOR("sv_client_max_interp_ratio"));
  }

  // Setup additional fonts
  {
    if(global->gui_font == nullptr)
      global->gui_font  = render->create_font(WXOR(L"Segoe UI"), FW_MEDIUM, CLEARTYPE_QUALITY);

    if(global->gui_font_emoji == nullptr){
      global->gui_font_emoji = render->create_font(XOR("Segoe UI Emoji"), 14, FW_MEDIUM, CLEARTYPE_QUALITY);
      assert(global->gui_font_emoji != nullptr);
    }

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
      global->target_window = FindWindowW(WXOR(L"Valve001"), WXOR(L"Double Action: Boogaloo"));

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
      gcs::render_scene(global->menu, math::mouse_pos(global->target_window), global->gui_font, global->gui_font_emoji);
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