#pragma once

class c_procedure_hlsdm : public c_procedure_base{
public:

  bool setup() override{
    if(global->dxgi_offset_data == nullptr)
      return false;

    return true;
  }

  bool handle_globaldata(){
    DBG("[!] fetching globaldata\n");

    set_module(HASH("client.dll"));
    {
      float cur_time = 0.f;

      u32 global_data_ptr = (u32)find_signature(HASH("client.dll"), HASH(".text"), SIG("\xA1\x00\x00\x00\x00\xD9\x40\x0C\xD9\x9E\x00\x00\x00\x00\xC6\x86\x00\x00\x00\x00\x00\xF3\x0F\x2C\x8E\x00\x00\x00\x00"), false, 0x1);

      while(true){
        u32 global_data = global_data_ptr;

        for(u8 i = 0; i < XOR32(2); i++)
          if(!inject->read_memory(global_data, &global_data, XOR32(sizeof(u32)))){
            SHOW_ERROR(ERR_GLOBALDATA_FETCH_FAILED, true);
            return false;
          }

        if(!inject->read_memory(global_data + XOR32(0xC), &cur_time, XOR32(sizeof(float)))){
          SHOW_ERROR(ERR_GLOBALDATA_FETCH_FAILED, true);
          return false;
        }

        if(cur_time >= 5.0f){
          DBG("[+] globaldata fetched! (%f)\n", cur_time);
          add_ptr(HASH("globaldata"), global_data);
          return true;
        }
        else
          I(Sleep)(XOR32(100));
      }
    }

    return false;
  }

  bool disable_vac(){

    // Disable bit that determins if game is a valve game and therefor enables vac hooks to report to vac
    // This bit is checked in combination with hooks that makes the game report pages to valve in some cases
    set_module(HASH("gameoverlayrenderer.dll"));
    {
      // To find me look in gameoverlayrender and see how they place hooks in the entrypoint
      // You should be able to easily figure out the if statement used in basically every hook to see if the game is a valve title
      // Set it to false and you will avoid all there hooks
      u32 is_valve_game_ptr = (u32)find_signature(HASH("gameoverlayrenderer.dll"), HASH(".text"), SIG("\xC6\x05\x00\x00\x00\x00\x00\x84\x00\x74\x00"), false, 0x2);
      assert(inject->read_memory(is_valve_game_ptr, &is_valve_game_ptr, sizeof(u32)));

      bool new_value = false;
      inject->write_memory(is_valve_game_ptr, &new_value, sizeof(bool));
    }

    return true;
  }

  bool write_signatures() override{
    set_section(HASH(".text"));

    // Easy way to ensure the loading screen is passed
   set_module(HASH("serverbrowser.dll"));

   set_module(HASH("client.dll"));
   {
     //add_signature(HASH("iinput"), SIG("\x7B\x12\x8B\x0D\x00\x00\x00\x00\xF3\x0F\x10\x41\x00"), false, 0x16);

     add_signature(HASH("keyvalues_init"), SIG("\x55\x8B\xEC\xFF\x15\x00\x00\x00\x00\xFF\x75\x00\x8B\xC8\x8B\x10\xFF\x52\x00\x5D\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x55\x8B\xEC\x8B\x51"), false);
     add_signature(HASH("keyvalues_setname"), SIG("\xFF\x15\x00\x00\x00\x00\x83\xC4\x00\x89\x06\x8B\xC6"), false);
     add_signature(HASH("keyvalues_setint"), SIG("\x55\x8B\xEC\x6A\x01\xFF\x75\x08\xE8\x00\x00\x00\x00\x85\xC0\x74\x0A\x8B\x4D\x0C\x89\x48\x0C\xC6\x40\x10\x02"), false);
     add_signature(HASH("keyvalues_setfloat"), SIG("\x55\x8B\xEC\x6A\x00\xFF\x75\x00\xE8\x00\x00\x00\x00\x85\xC0\x74\x00\xF3\x0F\x10\x45"), false);

     // Get the return address of the call do_post_screen_space_effects
     add_signature(HASH("vgui_drawhud_retaddr"), SIG("\x8A\x45\x00\x8B\xCE\x24"), false);

     //add_signature(HASH("look_weapon_info_slot"), SIG("\x55\x8B\xEC\x8B\x45\x00\x83\xEC\x00\x85\xC0\x74\x00\x89\x45\x00\xB9"), false);
     //add_signature(HASH("file_weapon_info_from_handle"), SIG("\x55\x8B\xEC\x66\x8B\x45\x00\x66\x3B\x05"), false);
     add_signature(HASH("get_class_name"), SIG("\x53\x8B\xD9\xC6\x05"), false);
     add_signature(HASH("set_abs_origin"), SIG("\x55\x8B\xEC\x56\x57\x8B\xF1\xE8\x00\x00\x00\x00\x8B\x7D"), false);
     add_signature(HASH("set_abs_angles"), SIG("\x55\x8B\xEC\x83\xEC\x00\x56\x57\x8B\xF1\xE8\x00\x00\x00\x00\x8B\x7D"), false);

     add_signature(HASH("prediction_random_seed"), SIG("\xA1\x00\x00\x00\x00\x89\x45\x00\x8D\x45\x00\x56\x50\xE8\x00\x00\x00\x00\x6A\x00\x8D\x45\x00\x50\x8D\x45\x00\x50\xE8\x00\x00\x00\x00\x6A\x00\x8D\x45\x00\x50\x8D\x45\x00\x50\xE8\x00\x00\x00\x00\x8B\x55\x00\x83\xC4\x00\x8B\xCA\x8D\x71\x00\x8A\x01\x41\x84\xC0\x75\x00\x2B\xCE\x8D\x45\x00\x51\x52\x50\xE8\x00\x00\x00\x00\x8D\x45\x00\x50\xE8\x00\x00\x00\x00\xFF\x75\x00\xFF\x15\x00\x00\x00\x00\xFF\x75"), false, 0x1);
   }

   set_module(HASH("engine.dll"));
   {
     // search for "CL_RunPrediction" and compare to this code: https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/engine/cl_pred.cpp#L41-L72
     add_signature(HASH("client_state"), SIG("\xB9\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8D\x85\x00\x00\x00\x00\x50\x68"), false, 0x1);

     add_signature(HASH("clc_move_rtti"), SIG("\xC7\x45\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8D\x4D\x00\xE8\x00\x00\x00\x00\x6A\x00\x6A\x00\x68\x00\x00\x00\x00\x8D\x85\x00\x00\x00\x00\xC6\x45\x00\x00\x50\x8D\x4D\x00\xE8\x00\x00\x00\x00\xA1"), false);

     // Search for "CL_RunPrediction"
     add_signature(HASH("cl_runprediction"), SIG("\x55\x8B\xEC\x83\xEC\x00\x8B\x0D\x00\x00\x00\x00\x33\xD2\x53\x33\xDB\x89\x55\x00\x89\x5D\x00\x8B\x01\x89\x55\x00\x56"), false);

     add_signature(HASH("keyvalues_setstring"), SIG("\x55\x8B\xEC\x57\x6A\x00\xFF\x75\x00\xE8\x00\x00\x00\x00\x8B\xF8\x85\xFF\x74\x00\x80\x7F"), false);

     // Search for "Software\\Valve\\Steam" and when inside function it is there.
     add_signature(HASH("steam3client_addr"), SIG("\xE8\x00\x00\x00\x00\x8B\x78\x08"), true);
   }

   //set_module(HASH("server.dll"));
   //{
   //  // Good way to find this is get_cvar_ptr("sv_clockcorrection_msecs") + 0x2C, find out what accesses address and thats the func
   //  // Looks for a call to gpGlobals, we are basically getting the servers globaldata so we can write to it and restore
   //  //add_signature(HASH("server_globaldata"), SIG("\x8B\x0D\x00\x00\x00\x00\x39\xBE"), false, 0x2);
   //}

    return true;
  }

  bool write_pointers() override{
    if(!handle_globaldata())
      return false;

    if(!disable_vac())
      return false;

    return true;
  }

  bool write_hooks() override{
    set_section(HASH(".text"));

    set_module(HASH("d3d9.dll"));
    {
      add_hook(HASH("d3d9_hook"), global->dxgi_offset_data->d3d9_present);
    }

    set_module(HASH("client.dll"));
    {
      // look for "WARNING! User command buffer overflow(%i %i), last cmd was %i bits long\n" and xref 2 up in vtable list
      add_hook(HASH("create_move_hook"), SIG("\x55\x8B\xEC\x8B\x45\x00\x85\xC0\x74\x00\x89\x45"), false);

      // use CViewRender::SetUpViews as a ref use the convar cl_demoviewoverride to get into this function.
      add_hook(HASH("override_view_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\xE8\x00\x00\x00\x00\x85\xC0\x0F\x84\x00\x00\x00\x00\x8B\x10"), false);

      //add_hook(HASH("cinput_get_user_cmd_hook"), SIG("\x55\x8B\xEC\x8B\x45\x08\x56\x8B\xF1\x83\xF8\xFF\x75\x10\x8B\x0D\x00\x00\x00\x00\x8B\x01\x8B\x90\x00\x00\x00\x00\xFF\xD2\x8B\x4D\x0C"), false);

      add_hook(HASH("run_command_hook"), SIG("\x55\x8B\xEC\x51\x53\x56\x8B\xF1\x57\x89\x75\x00\xE8"), false);

      //add_hook(HASH("run_simulation_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x56\x8B\xF1\x8B\x0D\x00\x00\x00\x00\x8B\x01\x8B\x90\x00\x00\x00\x00\x57\x89\x75"), false);

      // Follow up call to PostThink with debugger from RunCommand
      add_hook(HASH("post_think_hook"), SIG("\x53\x56\x8B\x35\x00\x00\x00\x00\x8B\xD9\x8B\xCE\x8B\x06"), false);

      //// Search for "C_BaseAnimating::SetupBones"
      add_hook(HASH("base_animating_setupbones_hook"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x53\x56\x8B\x35\x00\x00\x00\x00\x8B\xD9\x33\xC9\x33\xD2"), false);

      add_hook(HASH("client_interp_amount_hook"), SIG("\xE8\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\xD9\x5D\xF4"), true);

      ////// Search for this cvar and look below
      //////  && *(mat_motion_blur_enabled + 48)
      //////  && (*(*g_pMaterialSystemHardwareConfig + 128))(g_pMaterialSystemHardwareConfig) >= 90 )
      //////v24 = sub_101C61D0();
      ////// (*(*v24 + 156))(v24, a2); <-----
      add_hook(HASH("do_post_screen_space_effects_hook"), SIG("\xB0\x00\xC2\x00\x00\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x56"), false);

      add_hook(HASH("set_dormant_hook"), SIG("\x55\x8B\xEC\x53\x8B\x5D\x00\x56\x8B\xF1\x8B\x06"), false);

      ////add_hook(HASH("util_impacttrace"), SIG("\x55\x8B\xEC\x8B\x55\x00\x8B\x4A"), false);

      // Use the convar "mat_autoexposure_min / mat_autoexposure_max"
      add_hook(HASH("get_exposure_range_hook"), SIG("\x55\x8B\xEC\x80\x3D\x00\x00\x00\x00\x00\xF3\x0F\x10\x05"), false);

      // use the convar mat_bloomscale
      add_hook(HASH("get_bloom_amount_hook"), SIG("\x55\x8B\xEC\x51\x8B\x0D\x00\x00\x00\x00\x8B\x01\xFF\x90"), false);

      add_hook(HASH("maintain_sequence_transition_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x56\x57\x8B\x7D\x00\x8B\xF1\x8B\xCF"), false);

      add_hook(HASH("fire_bullets_hook"), SIG("\x53\x8B\xDC\x83\xEC\x00\x83\xE4\x00\x83\xC4\x00\x55\x8B\x6B\x00\x89\x6C\x24\x00\x8B\xEC\x81\xEC\x00\x00\x00\x00\x56\x8B\xF1\x57\x89\x75\x00\xE8"), false);
    }

    set_module(HASH("vgui2.dll"));
    {
      add_hook(HASH("paint_traverse_hook"), SIG("\x55\x8B\xEC\x8B\x01\xFF\x75\x00\xFF\x90\x00\x00\x00\x00\xFF\x75\x00\x8B\xC8\xFF\x75\x00\x8B\x10\xFF\x52\x00\x5D\xC2\x00\x00\xCC\x55\x8B\xEC\x8B\x01\xFF\x75\x00\xFF\x90\x00\x00\x00\x00\x8B\xC8"), false);
    }

    set_module(HASH("engine.dll"));
    {
      // search for "CEngineVGui::Paint" and sig the function it is in
      add_hook(HASH("engine_paint_hook"), SIG("\x55\x8B\xEC\x83\xEC\x2C\x53\x8B\xD9\x8B\x0D\x00\x00\x00\x00\x56"), false);

      //add_hook(HASH("cl_sendmove_hook"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x8D\x4D"), false);

      // Search for "NetMsg" and sig the function it is in.
      add_hook(HASH("cnetchan_sendnetmsg_hook"), SIG("\x55\x8B\xEC\x57\x8B\xF9\x8D\x8F"), false);

      // Search for "ClientDLL_FrameStageNotify" and sig the function it is in.
      add_hook(HASH("frame_stage_notify_hook"), SIG("\x55\x8B\xEC\x8B\x0D\x00\x00\x00\x00\x83\xEC\x00\x85\xC9\x74"), false);

      // Search for "CL_Move" or "WARNING: Connection issues"
      //add_hook(HASH("cl_move_hook"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\x00\x56\xE8\x00\x00\x00\x00\x8B\xF0\x83\x7E"), false);

      add_hook(HASH("cl_client_interp_amount_hook"), SIG("\x55\x8B\xEC\x8B\x0D\x00\x00\x00\x00\x83\xEC\x00\x85\xC9\x75\x00\x8B\x0D"), false);

      add_hook(HASH("draw_model_execute_hook"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x53\x57\x8B\x7D\x0C\x89\x4D\xFC"), false);

      add_hook(HASH("fire_event_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x8B\x5D\x00\x57\x8B\xF9\x89\x7D"), false);
      //// Be in-game first
      //// Set a break point on the tier0 function "ConMsg"
      //// Go on a community server and type "sm plugins"
      //// Go through the callstack until you find the function that called ConMsg
      //// Should look like this. Also if offset changes from 20 to something else update the offset in the s_svc_print struct.
      ///*
      //  ConMsg("%s", *(a1 + 20));
      //  return 1;
      //*/
      add_hook(HASH("cbaseclientstate_process_print_hook"), SIG("\x55\x8B\xEC\x8B\x45\x00\xFF\x70"), false);
    }

    set_module(HASH("crashhandler.dll"));
    {
      add_hook(HASH("steam_exception_handler_hook"), SIG("\x55\x8B\xEC\x53\x56\x8B\xF1\x57\x8B\x7D\x00\x8B\x06"), false);
    }

    set_module(HASH("server.dll"));
    {
      // Good way to find this is get_cvar_ptr("sv_clockcorrection_msecs") + 0x2C, find out what accesses address and thats the func
      //add_hook(HASH("server_adjust_player_time_base_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x83\x7D\x00\x00\x56\x8B\xF1\x0F\x8C"), false);
    }

    return true;
  }
};