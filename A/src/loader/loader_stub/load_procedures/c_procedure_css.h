#pragma once

class c_procedure_css : public c_procedure_base{
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

      u64 global_data_ptr = (u64)find_signature(HASH("client.dll"), HASH(".text"), SIG("\x48\x8B\x05\x00\x00\x00\x00\x8B\x50\x00\x41\x89\x90"), true);

      while(true){
        u64 global_data = global_data_ptr;

        if(!inject->read_memory(global_data, &global_data, XOR32(sizeof(u64)))){
          SHOW_ERROR_STR(ERR_GLOBALDATA_FETCH_FAILED, "1: Possible anti-virus issue or game update", true);
          return false;
        }

        if(!inject->read_memory(global_data + XOR32(0xC), &cur_time, XOR32(sizeof(float)))){
          SHOW_ERROR_STR(ERR_GLOBALDATA_FETCH_FAILED, "1: Possible anti-virus issue or game update", true);
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
    DBG("[!] disable_vac\n");

    // Disable bit that determins if game is a valve game and therefor enables vac hooks to report to vac
    // This bit is checked in combination with hooks that makes the game report pages to valve in some cases
    set_module(HASH("gameoverlayrenderer64.dll"));
    {
      // To find me look in gameoverlayrender and see how they place hooks in the entrypoint
      // You should be able to easily figure out the if statement used in basically every hook to see if the game is a valve title
      // Set it to false and you will avoid all there hooks

      // Update: Avoid getting a signature in functions where valve is likely to change code not in terms of anti-cheat stuff but likely fixing overlay bugs.
      u64 is_valve_game_ptr = (u64)find_signature(HASH("gameoverlayrenderer64.dll"), HASH(".text"), SIG("\x80\x3D\x00\x00\x00\x00\x00\x41\x8B\xE8\x4C\x8B\xF2\x48\x8B\xD9\x74\x00\x48\x8B\x7C\x24\x00\x48\x8B\xD1\x4C\x8B\xC7\x48\x8D\x0D"), true);

      DBG("[!] disable_vac: is_valve_game_ptr: %p\n", is_valve_game_ptr);
      u64 read_value = false;
      if(inject->read_memory(is_valve_game_ptr, &read_value, sizeof(bool))){
        DBG("[!] disable_vac: before patch is_valve_title: %i\n", read_value);
      }
      else{
        SHOW_ERROR_STR(ERR_VAC_BYPASS_SIG_FAIL, "Failed to execute (1)", true);
        return false;
      }

      bool new_value = false;
      inject->write_memory(is_valve_game_ptr, &new_value, sizeof(bool));

      if(inject->read_memory(is_valve_game_ptr, &read_value, sizeof(bool))){
        DBG("[!] disable_vac: after patch is_valve_title: %i\n", read_value);
      }
      else{
        SHOW_ERROR_STR(ERR_VAC_BYPASS_SIG_FAIL, "Failed to execute (2)", true);
        return false;
      }

      // This likely means the signature is valid but it's not disabling the is_valve_title check.
      if(read_value != false){
        SHOW_ERROR_STR(ERR_VAC_BYPASS_SIG_FAIL, "Failed to execute (3)", true);
        return false;
      }
    }

    DBG("[!] disable_vac ok\n");
    return true;
  }

  bool write_signatures() override{
    set_section(HASH(".text"));

    // Easy way to ensure the loading screen is passed
   set_module(HASH("serverbrowser.dll"));

   // Search for // @ for unfixed sigs.
   // Ones like // !! are to be left alone since we have a better way of doing something.

   set_module(HASH("client.dll"));
   {
     // Find CHLClient::CreateMove and sig the instruction below push rdi
     // We use an exception handler do dump RSP to get stack position of bSendPacket
     add_signature(HASH("chlclient_createmove_push_rdi"), SIG("\x48\x83\xEC\x00\x8B\xF2\x0F\x29\x74\x24"));

     add_signature(HASH("iinput"), SIG("\x48\x8B\x0D\x00\x00\x00\x00\x48\x8B\x01\xFF\x90\x00\x00\x00\x00\x48\x85\xDB\x74\x00\x48\x8B\x03\x33\xD2"), true);

     add_signature(HASH("keyvalues_init"), SIG("\x40\x53\x48\x83\xEC\x00\x48\x8B\xD9\xFF\x15\x00\x00\x00\x00\x8B\xD3"), false);

     add_signature(HASH("keyvalues_setname"), SIG("\x40\x53\x48\x83\xEC\x00\x48\x8B\xD9\xC7\x01"), false);
     add_signature(HASH("keyvalues_setint"), SIG("\x40\x53\x48\x83\xEC\x00\x41\x8B\xD8\x41\xB0"), false);
     add_signature(HASH("keyvalues_setfloat"), SIG("\x48\x83\xEC\x00\x0F\x29\x74\x24\x00\x41\xB0"), false);

     // Get the return address of the call do_post_screen_space_effects
     add_signature(HASH("vgui_drawhud_retaddr"), SIG("\x45\x0F\xB6\xC4\x48\x8B\xD6\x41\x80\xE0"), false);
     // Found inside of FX_FireBullets
     add_signature(HASH("fire_bullet"), SIG("\x48\x8B\xC4\x48\x89\x50\x00\x48\x89\x48\x00\x55\x53\x56\x41\x54"), false);

     add_signature(HASH("look_weapon_info_slot"), SIG("\x48\x8B\xD1\x48\x8D\x0D\x00\x00\x00\x00\xE9\x00\x00\x00\x00\xCC\x48\x89\x5C\x24\x00\x48\x89\x6C\x24"), false);
     add_signature(HASH("file_weapon_info_from_handle"), SIG("\x66\x3B\x0D"), false);
     add_signature(HASH("get_class_name"), SIG("\x40\x56\x48\x83\xEC\x00\xC6\x05"), false);
     add_signature(HASH("set_abs_origin"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xFA\x48\x8B\xD9\xE8\x00\x00\x00\x00\xF3\x0F\x10\x83\x00\x00\x00\x00\x0F\x2E\x07"), false);
     add_signature(HASH("set_abs_angles"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\xFA\x48\x8B\xD9\xE8\x00\x00\x00\x00\xF3\x0F\x10\x83"), false);

     add_signature(HASH("md5_pseudorandom"), SIG("\x89\x4C\x24\x00\x55\x48\x8B\xEC\x48\x81\xEC"), false);

     // Find inside the CSPlayer FireBullet or also C_CSPlayer::UpdateIDTarget
     add_signature(HASH("util_cliptrace_to_players"), SIG("\x48\x8B\xC4\x48\x89\x70\x00\x48\x89\x78\x00\x55\x41\x54"), false);

     //sub_1022A060(&dword_10C681B8, "CHudChat"); Sig the function CHudChat is passed into along with the dword_XXXXXX(which is gHud pointer)
     add_signature(HASH("get_hud_element_addr"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x33\xDB\x48\x8B\xEA\x48\x8B\xF9\x39\x59"), false);
     add_signature(HASH("hud_element_addr"), SIG("\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x48\x8D\x15\x00\x00\x00\x00\x48\x89\x46"), true);

     // Found by searching for "%c(%s) %s%c: %s" or "#Voice"
     // Get the return address of the call to ChatPrintf
     // ChatPrintf: (*(v19 + 76))(v12, v20, 0, "%c(%s) %s%c: %s", 3, v29, v21);
     add_signature(HASH("chatprintf_retaddr1"), SIG("\x4C\x8B\xB4\x24\x00\x00\x00\x00\xC6\x87"), false);


     // Found inside of C_CSPlayer::UpdateIDTarget
     add_signature(HASH("intersect_inf_ray_with_sphere_addr"), SIG("\x48\x83\xEC\x00\xF3\x0F\x10\x61\x00\xF3\x0F\x10\x4A"), false);
    
     // Very easy to find you should remember.
     add_signature(HASH("prediction_random_seed"), SIG("\x8B\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\xC8"), true);

     add_signature(HASH("ctrace_filter_simple"), SIG("\x48\x8D\x05\x00\x00\x00\x00\x48\x89\x51\x00\x48\x89\x01\x48\x8B\xC1\x44\x89\x41"), false);

     // v53 = current_dist + (v110 * dist);
     // real_damage = pow(a8, (v53 * 0.0020000001)) * v131;
     // current_damage0 = real_damage;
     // if ( v53 > v17 && penetration > 0 ) // <-- Placed here.
     //   penetration = 0;
     add_signature(HASH("fire_bullets_damage_rbp"), SIG("\x44\x0F\x4F\xF8\x40\x84\xFF"));


      // Look for "Clamping ApplyAbsVelocityImpulse"
      /*
          Warning("Clamping ApplyAbsVelocityImpulse(%f,%f,%f) on %s\n", *a2);
          }
          if ( *(a1 + 548) == 6 )
          {
            (*(**(a1 + 288) + 416i64))(*(a1 + 288), &v8, 0i64);
          }
          else
          {
            C_BaseEntity::CalcAbsoluteVelocity(a1);
            *&v6 = *(&v8 + 1) + *(a1 + 476);
            *v10 = *&v8 + *(a1 + 472);
            *&v7 = v9 + *(a1 + 480);
            v10[1] = v6;
            v10[2] = v7;
            sub_7FF9E5F23E20(a1, v10);
      */
      add_signature(HASH("baseentity_calculate_abs_velocity_addr"), SIG("\x40\x57\x48\x81\xEC\x00\x00\x00\x00\xF7\x81"), false);
   }

   set_module(HASH("engine.dll"));
   {
      // search for "CL_RunPrediction" and compare to this code: https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/engine/cl_pred.cpp#L41-L72
     add_signature(HASH("client_state"), SIG("\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x48\x8D\x54\x24\x00\x48\x8D\x0D\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x48\x8B\x0D"), true);

     // Search for "CL_RunPrediction"
     add_signature(HASH("cl_runprediction"), SIG("\x4C\x8B\xDC\x49\x89\x5B\x00\x57\x48\x83\xEC\x00\x48\x8B\x1D\x00\x00\x00\x00\x33\xFF\x49\x89\x7B\x00\x48\x8B\x03\x48\x85\xC0\x74\x00\x48\x8D\x0D\x00\x00\x00\x00\x45\x33\xC9\x49\x89\x4B\x00\x49\x8D\x53\x00\x48\x8D\x0D\x00\x00\x00\x00\x45\x33\xC0\x49\x89\x4B\x00\x48\x8D\x0D\x00\x00\x00\x00\x49\x89\x4B\x00\x48\x8D\x0D\x00\x00\x00\x00\xC7\x44\x24\x00\x00\x00\x00\x00\x49\x89\x4B\x00\x48\x8B\xC8\x49\x89\x7B\x00\xFF\x90\x00\x00\x00\x00\x48\x8B\x7C\x24\x00\x48\x8B\x1D\x00\x00\x00\x00\x83\x3D"), false);

     add_signature(HASH("keyvalues_setstring"), SIG("\xE8\x00\x00\x00\x00\xF3\x0F\x10\x57\x00\x48\x8D\x15\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\xF3\x0F\x10\x57\x00\x48\x8D\x15\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\xF3\x0F\x10\x57\x00\x48\x8D\x15\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\xF3\x0F\x10\x57\x00\x48\x8D\x15\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\xF3\x0F\x10\x57\x00\x48\x8D\x15\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\xF3\x0F\x10\x57"), true);

     // Search for "Software\\Valve\\Steam" and when inside function it is there.
     add_signature(HASH("steam3client_addr"), SIG("\x48\x8D\x05\x00\x00\x00\x00\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x40\x53\x48\x83\xEC\x00\x48\x8B\xD9\x48\x8D\x15\x00\x00\x00\x00\x33\xC9"), true);

     // it'll be the instruction after ClientDLL->CreateMove is called in cl_move
     add_signature(HASH("cl_move_test_dil_addr"), SIG("\x40\x84\xFF\x0F\x84\x00\x00\x00\x00\x8B\x05"));

     // Why? Don't need sig for nextcmdtime & two vtable indexes.
     /*
       if ( (!(*(*net_channel + 48i64))() || *(host_limitlocal + 88))
       && (*&net_time < *&m_flNextCmdTime || !(*(*net_channel + 424i64))(net_channel) || !final_tick) )
       {
         send_packet = 0; // <-- this here.
       }
     */
     add_signature(HASH("cl_move_send_packet_false_addr"), SIG("\x40\x32\xFF\x83\x3D"));
   }

   set_module(HASH("server.dll"));
   {
     // @ add_signature(HASH("get_server_animating_addr"), SIG("\x55\x8B\xEC\x8B\x55\x00\x85\xD2\x7E\x00\xA1"), false);
     // @ add_signature(HASH("draw_server_hitbox_addr"), SIG("\x55\x8B\xEC\x83\xEC\x00\x57\x8B\xF9\x80\xBF\x00\x00\x00\x00\x00\x0F\x85\x00\x00\x00\x00\x83\xBF\x00\x00\x00\x00\x00\x75\x00\xE8\x00\x00\x00\x00\x85\xC0\x74\x00\x8B\xCF\xE8\x00\x00\x00\x00\x8B\x97"), false);
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

    set_module(HASH("ntdll.dll"));
    {
      u64 module = inject->get_module_address(HASH("ntdll.dll"));
      u64 export_ptr = inject->get_export(module, HASH("KiUserExceptionDispatcher"));

      void* _asm = malloc(XOR32(256));
      inject->read_memory(export_ptr, _asm, XOR32(256));

      for(u64 i = _asm;; i < XOR32(256)){
        hde64s h;
        hde64_disasm(i, &h);

        if(h.opcode == XOR32(0xE8) && (h.flags & HDE64_F_IMM32)){
          add_hook(HASH("rtl_dispatch_exception_hook"), (export_ptr + (i - (u64)_asm) + (i32)h.imm.imm32 + h.len) - module);
          break;
        }

        DBG("%i\n", h.len);

        i += h.len;
      }

      free(_asm);
    }

    set_module(HASH("client.dll"));
    {
      // look for "WARNING! User command buffer overflow(%i %i), last cmd was %i bits long\n" and xref 2 up in vtable list
      add_hook(HASH("create_move_hook"), SIG("\x48\x85\xD2\x74\x00\x48\x81\xC1\x00\x00\x00\x00\x48\x3B\xCA\x74\x00\x8B\x42\x00\x89\x41\x00\x8B\x42\x00\x89\x41\x00\x8B\x42\x00\x89\x41\x00\x8B\x42\x00\x89\x41\x00\x8B\x42\x00\x89\x41\x00\x8B\x42\x00\x89\x41\x00\x8B\x42\x00\x89\x41\x00\x8B\x42\x00\x89\x41\x00\x8B\x42\x00\x89\x41\x00\x0F\xB6\x42\x00\x88\x41\x00\x8B\x42\x00\x89\x41\x00\x8B\x42\x00\x89\x41\x00\x8B\x42\x00\x89\x41\x00\x0F\xB7\x42\x00\x66\x89\x41\x00\x0F\xB7\x42\x00\x66\x89\x41\x00\x0F\xB6\x42\x00\x88\x41\x00\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x48\x89\x74\x24"), false);

      add_hook(HASH("override_view_hook"), SIG("\x48\x89\x5C\x24\x00\x55\x48\x8D\x6C\x24\x00\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\xDA"), false);

       // Follow up call to PostThink with debugger from RunCommand
      add_hook(HASH("run_command_hook"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x4C\x89\x74\x24\x00\x41\x57\x48\x83\xEC\x00\x0F\x29\x74\x24"), false);
      add_hook(HASH("post_think_hook"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\x1D\x00\x00\x00\x00\x48\x8B\xF9\x48\x8B\xCB\x48\x8B\x03\xFF\x90\x00\x00\x00\x00\x48\x8B\x07"), false);
      add_hook(HASH("process_impacts_hook"), SIG("\x41\x56\x48\x83\xEC\x00\x48\x8B\x05\x00\x00\x00\x00\x4C\x8B\xF1\x83\x78"), false);

      // Search for "C_BaseAnimating::SetupBones"
      add_hook(HASH("base_animating_setupbones_hook"), SIG("\x48\x8B\xC4\x44\x89\x40\x00\x48\x89\x50\x00\x55\x53"), false);


      //// Search for this cvar and look below
      ////  && *(mat_motion_blur_enabled + 48)
      ////  && (*(*g_pMaterialSystemHardwareConfig + 128))(g_pMaterialSystemHardwareConfig) >= 90 )
      ////v24 = sub_101C61D0();
      //// (*(*v24 + 156))(v24, a2); <-----
      add_hook(HASH("do_post_screen_space_effects_hook"), SIG("\xB0\x00\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x8B\x02"), false);

      add_hook(HASH("set_dormant_hook"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\x01\x0F\xB6\xFA\x48\x8B\xD9\x88\x91\x00\x00\x00\x00\xFF\x90\x00\x00\x00\x00\x48\x8D\x8B"), false);

      add_hook(HASH("util_impacttrace"), SIG("\x4C\x8B\x51\x00\x4D\x85\xD2"), false);

      // The function CViewRender::SetUpViews calls the CBasePlayer version which calls the function we're looking for.
      // You can use the string OnRenderStart->CViewRender::SetUpView to find this function.
      add_hook(HASH("cbaseviewmodel_calcviewmodelview_hook"), SIG("\x48\x89\x5C\x24\x00\x56\x48\x83\xEC\x00\xF2\x41\x0F\x10\x01"), false);

      // search for "%8.4f : %30s : %5.3f : %4.2f  +\n" and sig the function is it in.
      add_hook(HASH("maintain_sequence_transition_hook"), SIG("\x4C\x89\x4C\x24\x00\x41\x56"), false);

      // Use the game's code to find it.
      add_hook(HASH("input_cam_is_thirdperson_hook"), SIG("\x0F\xB6\x81\x00\x00\x00\x00\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x48\x8B\x05\x00\x00\x00\x00\x8B\x48"), false);


      add_hook(HASH("cinput_get_user_cmd_hook"), SIG("\x44\x8B\xCA\x4C\x8B\xC1\xB8"), false);

      // Had to break point in ChatPrintf and step into the function.
      add_hook(HASH("hud_chatline_insert_and_colorize_text_hook"), SIG("\x44\x89\x44\x24\x00\x55\x53\x56\x57"));

      // effects/flashbang
      add_hook(HASH("ccsviewrender_perform_flashbangeffect_hook"), SIG("\x48\x89\x54\x24\x00\x48\x89\x4C\x24\x00\x55\x56\x41\x55"), false);

      // Search: "Cannot record demos while blind" or "Cannot record demos while a smoke grenade is active"
      add_hook(HASH("clientmodecsnormal_canrecorddemo_hook"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x41\x8B\xF8\x48\x8B\xF2\xE8"), false);

      add_hook(HASH("cbaseentity_estimateabsvelocity_hook"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xFA\x48\x8B\xD9\xE8\x00\x00\x00\x00\x48\x3B\xD8"), false);

      // You can find it using s_bInterpolate which can be easily found in C_BaseEntity::InterpolateServerEntities.
      add_hook(HASH("cbaseentity_baseinterpolatepart1_hook"), SIG("\x48\x89\x5C\x24\x00\x56\x57\x41\x55\x41\x56\x41\x57\x48\x83\xEC\x00\x4C\x8B\xBC\x24"), false);

      //
      add_hook(HASH("crenderview_renderview_hook"), SIG("\x48\x8B\xC4\x44\x89\x48\x00\x44\x89\x40\x00\x48\x89\x50\x00\x48\x89\x48\x00\x55\x53"), false);
    }

    set_module(HASH("vgui2.dll"));
    {
      add_hook(HASH("paint_traverse_hook"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\x01\x41\x0F\xB6\xD9"), false);
    }

    set_module(HASH("engine.dll"));
    {
      // search for "CEngineVGui::Paint" and sig the function it is in
      add_hook(HASH("engine_paint_hook"), SIG("\x4C\x8B\xDC\x41\x54\x41\x57\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x05\x00\x00\x00\x00\x45\x33\xE4\x49\x89\x5B\x00\x48\x8D\x1D\x00\x00\x00\x00\x49\x89\x6B\x00\x48\x8B\xE9"), false);

      // search for "PureServerWhitelist" and sig the function is is in!
      add_hook(HASH("check_for_pure_server_whitelist_hook"), SIG("\x40\x56\x48\x83\xEC\x00\x83\x3D\x00\x00\x00\x00\x00\x48\x8B\xF1\x0F\x8E"), false);

      // Search for "NetMsg" and sig the function it is in.
      add_hook(HASH("cnetchan_sendnetmsg_hook"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56\x48\x83\xEC\x00\x48\x8B\xF1\x45\x0F\xB6\xF1"), false);

      // Search for "ClientDLL_FrameStageNotify" and sig the function it is in.
      add_hook(HASH("frame_stage_notify_hook"), SIG("\x4C\x8B\xDC\x56\x48\x83\xEC"), false);

      add_hook(HASH("cl_client_interp_amount_hook"), SIG("\x48\x83\xEC\x00\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x75"), false);

      add_hook(HASH("draw_model_execute_hook"), SIG("\x4C\x89\x4C\x24\x00\x48\x89\x4C\x24\x00\x55\x53\x56\x57\x41\x54"), false);

      add_hook(HASH("fire_event_hook"), SIG("\x41\xB1\x00\x45\x33\xC0\xE9"), false);

      // finding this was kinda aids.
      add_hook(HASH("engine_get_player_info_hook"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x8D\x7A"), false);

      // Search for "StartStaticSound" and sig the function it is in
      add_hook(HASH("s_startsound_hook"), SIG("\x40\x53\x48\x83\xEC\x00\x48\x83\x79\x00\x00\x48\x8B\xD9\x75\x00\x33\xC0"), false);

      // Look for "Auto-disconnect in %.1f seconds" and sig the function it is in.
      add_hook(HASH("cl_move_hook"), SIG("\x40\x55\x53\x48\x8D\xAC\x24\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x48\x2B\xE0\x83\x3D"), false);
    }

    set_module(HASH("serverbrowser.dll"));
    {
      add_hook(HASH("is_blacklisted_hook"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xD9\x48\x8B\xFA\x48\x81\xC1"), false);
    }

    return true;
  }
};