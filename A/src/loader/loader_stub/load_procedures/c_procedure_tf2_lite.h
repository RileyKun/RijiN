#pragma once

class c_procedure_tf2_lite : public c_procedure_base{
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
          SHOW_ERROR_STR(ERR_GLOBALDATA_FETCH_FAILED, "1: 1. If your game crashes then wait 5 to 10 minutes and try again. This is a TF2 bug\n2. Possible anti-virus issue or game update", true);
          return false;
        }

        if(!inject->read_memory(global_data + XOR32(0xC), &cur_time, XOR32(sizeof(float)))){
          SHOW_ERROR_STR(ERR_GLOBALDATA_FETCH_FAILED, "2: 1. If your game crashes then wait 5 to 10 minutes and try again. This is a TF2 bug\n2. Possible anti-virus issue or game update", true);
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

    
    set_module(HASH("client.dll"));
    {
      // Find CHLClient::CreateMove and sig the instruction below push rdi
      // We use an exception handler do dump RSP to get stack position of bSendPacket
      add_signature(HASH("chlclient_createmove_push_rdi"), SIG("\x48\x83\xEC\x00\x8B\xF2\x0F\x29\x74\x24"));

      // search "Pitch: %6.1f   Yaw: %6.1f %38s"
      // compare code with https://github.com/ValveSoftware/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/mp/src/game/client/in_camera.cpp#L418-L439
      add_signature(HASH("input"), SIG("\x48\x8B\x0D\x00\x00\x00\x00\x48\x8D\x15\x00\x00\x00\x00\x48\x8B\x01\xFF\x50\x00\x0F\x2E\xC6\x74\x00\x48\x8B\x05\x00\x00\x00\x00\xF3\x44\x0F\x58\x58"), true);

      add_signature(HASH("get_team_check_scoreboard_retaddr1"), SIG("\x83\xF8\x00\x75\x00\x8B\x84\x24"));
      add_signature(HASH("get_team_status_panel_retaddr1"), SIG("\x4C\x8B\x07\x83\xF8"));

      // For the spy cloak / disguise hit markers fix.
      // v135 = (*(*a2 + 20))(a2, "showdisguisedcrit", 0);
      //LOBYTE(v6) = InCond((player + 6128), 3); <--- ret addr of this
      // if ( !v6 || v135 )
      add_signature(HASH("ctfplayer_fireevent_retaddr1"), SIG("\x84\xC0\x74\x00\x45\x84\xED\x0F\x84\x00\x00\x00\x00\x48\x8D\x8F"));

      // Search for "DamagedPlayer" once in the function scroll up until you see.
      //if ( sub_10405AC0(LODWORD(v10), 8) ) sig the retaddr of that.
      //in tf_hud_account.cpp (DisplayDamageFeedback)
      add_signature(HASH("displaydmgfeedback_retaddr1"), SIG("\x84\xC0\x0F\x84\x00\x00\x00\x00\x48\x8B\x06\x48\x8B\xCE\xFF\x90\x00\x00\x00\x00\x49\x8B\x16"));

      // The return address of UTIL_TraceLine call.
      add_signature(HASH("displaydmgfeedback_retaddr4"), SIG("\x48\x8B\x05\x00\x00\x00\x00\x83\x78\x00\x00\x74\x00\xF3\x0F\x10\x05\x00\x00\x00\x00\x48\x8D\x54\x24"));

      // Used for skipping "if ( Q_stricmp( panelname, PANEL_INFO ) == 0 && mode )" in __MsgFunc_VGUIMenu for the motd
      add_signature(HASH("msg_func_is_info_panel_allowed_retaddr1"), SIG("\x84\xC0\x74\x00\x48\x8B\x03\x48\x8B\xCB\xFF\x90\x00\x00\x00\x00\x48\x8B\x0D"));

      // Search for string "minigun_no_spin_sounds" it will land you inside C_TFMinigun::WindDown this is where the function will be at.
      // found inside TF2_SetSpeed
      add_signature(HASH("tf2_getmaxspeed"), SIG("\x88\x54\x24\x00\x53\x55"));

      // Search for "CHudUpgradePanel" you should be in C_TFPlayer::AvoidPlayers
      // if ( *&v2 != 0.0 )
      // v5 = sub_102294C0("CHudUpgradePanel");
      // x-ref C_TFPlayer:AvoidPlayers and you'll be in C_TFPlayer::CreateMove
      // Follow: https://github.com/TheAlePower/TeamFortress2/blob/1b81dded673d49adebf4d0958e52236ecc28a956/tf2_src/game/client/tf/c_tf_player.cpp#L6995

      // You'll see a function call with 7 passed as an arg.
       add_signature(HASH("ctfplayer_createmove_retaddr1"), SIG("\x84\xC0\x75\x00\xBA\x00\x00\x00\x00\x48\x8D\x8E\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x84\xC0\x75\x00\x45\x32\xFF"));

      // You'll see a function call with 82(TF_COND_HALLOWEEN_KART) passed as an arg.
      add_signature(HASH("ctfplayer_createmove_retaddr2"), SIG("\x84\xC0\x74\x00\x4C\x8B\xC3"));

      // You'll see a function call with 87(TF_COND_FREEZE_INPUT) passed as an arg.
      add_signature(HASH("ctfplayer_createmove_retaddr3"), SIG("\x33\xFF\x84\xC0\x74\x00\xF3\x0F\x10\x05"));

      // Required for unlocking the view for TF_COND_HALLOWEEN_KART.
      // "CTFInput::ApplyMouse" Found by using the convar "tf_halloween_kart_cam_follow"
      add_signature(HASH("ctfinput_applymouse_retaddr1"), SIG("\x84\xC0\x74\x00\xF3\x0F\x10\x9B"));

      // check CTFWearable::ShouldDraw() we're checking returnaddress of InCond( pOwner == C_TFPlayer::GetLocalTFPlayer() && pOwner->m_Shared.InCond( TF_COND_ZOOMED ) )
      add_signature(HASH("ctfwearable_shoulddraw_retaddr1"), SIG("\x84\xC0\x0F\x85\x00\x00\x00\x00\x41\xBF"));

      // sig: \x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x48\x8D\x71
      // C_TFPlayer::ShouldDraw
      // get the return address of InCondition.
      add_signature(HASH("ctfplayer_shoulddraw_retaddr1"), SIG("\x84\xC0\x74\x00\x32\xC0\x48\x8B\x74\x24"));

      // found in C_TFPlayer::CreateMove
      add_signature(HASH("ctfplayer_canmoveduringtaunt"), SIG("\x40\x53\x48\x83\xEC\x00\x48\x8B\xD9\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x74\x00\xE8"), false);

      // search for "sapper_sentry1_fx" x-ref the function you end up in. There should only be 1 function that's CTFPlayerShared::OnConditionAdded.
      // x-ref the function until you see something like this
      // *(this[2] + v6 + 4) = 0;
      // CTFPlayerShared::OnConditionAdded(this, a2);
      add_signature(HASH("ctfplayershared_addcond"), SIG("\x48\x8B\xC4\x55\x53\x48\x8D\x68\x00\x48\x81\xEC\x00\x00\x00\x00\xFF\xCA"), false);

      // search for "damage_prevented" and sig the function you're in.
      add_signature(HASH("ctfplayershared_removecond"), SIG("\x40\x56\x57\x41\x56\x48\x83\xEC\x00\x45\x0F\xB6\xF0"), false);

      // Look for "sprites/physbeam.vmt" scroll down to bottom of function that should be the address.
      add_signature(HASH("view_render_beams_interface"), SIG("\x48\x8B\x0D\x00\x00\x00\x00\xF3\x0F\x11\x45\x00\xF3\x0F\x10\x43"), true);

      // search for "CSecondaryTargetID"
      // xref that function to its vmt table and go down 3 and thats C_TFPlayer::IsPlayerOnSteamFriendsList
      // or use tf_hud_target_id_show_avatars
      add_signature(HASH("ctfplayer_playeronsteamfriends"), SIG("\x40\x57\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\xFA\xE8"), false);

      // ClientModeTFNormal::ShouldDrawViewModel
      // Can find this function by using "r_drawviewmodel" get the return address of the InCond function.
      add_signature(HASH("shoulddraw_viewmodel_retaddr1"), SIG("\x84\xC0\x74\x00\x32\xC0\x48\x83\xC4\x00\xC3"), false);

      // Search for string "use_action_slot_item_server"
      add_signature(HASH("keyvalues_init"), SIG("\x40\x53\x48\x83\xEC\x00\x48\x8B\xD9\xFF\x15\x00\x00\x00\x00\x8B\xD3"), false);
      add_signature(HASH("keyvalues_setname"), SIG("\xFF\x15\x00\x00\x00\x00\x89\x03\x48\x8B\xC3\x48\x83\xC4"), true);

      // Keyvalues:setint search for "achievementID"
      add_signature(HASH("keyvalues_setint"), SIG("\x40\x53\x48\x83\xEC\x00\x41\x8B\xD8\x41\xB0"), false);

      // Look for "DmeVoxelPositionInitializer" and the functions with centerx, centery, centerz will have it.
      add_signature(HASH("keyvalues_setfloat"), SIG("\x48\x83\xEC\x00\x0F\x29\x74\x24\x00\x41\xB0"), false);

      add_signature(HASH("get_client_voice_manager"), SIG("\x48\x83\xEC\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x0F\x85\x00\x00\x00\x00\xB9"), false);

      // search for "CVoiceStatus::SetPlayerBlockedState part 2" and that function should be the function to sig
      add_signature(HASH("set_player_blocked_state"), SIG("\x48\x89\x74\x24\x00\x57\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x05\x00\x00\x00\x00\x8B\xF2\x48\x8B\xF9\x83\x78\x00\x00\x74"), false);

      // search for "#TF_ScoreBoard_Context_UnMute" and look for call to "GetClientVoiceMgr()->IsPlayerBlocked"
      add_signature(HASH("is_player_blocked"), SIG("\x40\x53\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\xD9\x4C\x8D\x44\x24"), false);

      // Search for "mult_crit_when_health_is_below_percent"
      add_signature(HASH("is_crit_boosted"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xD9\x0F\x29\x7C\x24"), false);

      // Search for "WeaponMedigun_Vaccinator.Charged_tier_0%d" it gets passed into EmitSound. Get the return address of that function.
      add_signature(HASH("medigun_find_and_heal_target_retaddr1"), SIG("\x48\x8B\xAC\x24\x00\x00\x00\x00\x0F\x28\xBC\x24\x00\x00\x00\x00\x66\x0F\x7E\xF0"), false);

      // Found in CInput::Createmove
      add_signature(HASH("md5_pseudorandom"), SIG("\x89\x4C\x24\x00\x55\x48\x8B\xEC\x48\x81\xEC"), false);

      add_signature(HASH("prediction_random_seed"), SIG("\x8B\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\xC8"), true);

      // search for "engy_disposable_sentries" in client.dll
      // look for something like "v15 = sub_10168350(v4, 3);" and that's the function
      add_signature(HASH("get_ammo_count"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x63\xDA\x48\x8B\xF9\x83\xFB"), false);

      // Inside the IsSteamfriend function we use on players there will be a call to GetPlayerInfo that's what this retaddr is
      add_signature(HASH("is_steam_friend_get_player_info_retaddr1"), SIG("\x48\x8B\x9C\x24\x00\x00\x00\x00\x84\xC0\x74\x00\x83\x7C\x24"));

      // CClientScoreBoardDialog::UpdatePlayerAvatar can be found with "avatar" string which is the only string in the function.
      // Test it, there's 2 functions that do the same thing but one of them is the correct while the other is not.
      add_signature(HASH("update_player_avatar_get_player_info_retaddr2"), SIG("\x84\xC0\x0F\x84\x00\x00\x00\x00\x83\x7C\x24\x00\x00\x0F\x84\x00\x00\x00\x00\x48\x89\xB4\x24"), false);
      
      // set_abs_origin SIG IS UPDATED FOR 64BIT!!!
      add_signature(HASH("set_abs_origin"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xFA\x48\x8B\xD9\xE8\x00\x00\x00\x00\xF3\x0F\x10\x83"), false);
      add_signature(HASH("set_abs_angles"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\xFA\x48\x8B\xD9\xE8\x00\x00\x00\x00\xF3\x0F\x10\x83"), false);
      // Unused
      // ! add_signature(HASH("set_abs_velocity"), SIG("\xE8\x00\x00\x00\x00\x8B\x43\x04\x8B\x4D\xF4"), false);
      

      // Search for "mult_bullets_per_shot"
      add_signature(HASH("attrib_hook_value_float"), SIG("\x4C\x8B\xDC\x49\x89\x5B\x00\x49\x89\x6B\x00\x56\x57\x41\x54\x41\x56\x41\x57\x48\x83\xEC\x00\x48\x8B\x3D\x00\x00\x00\x00\x4C\x8D\x35"), false);

      // Search for "can_overload"
      add_signature(HASH("attrib_hook_value_int"), SIG("\x4C\x8B\xDC\x49\x89\x5B\x00\x49\x89\x6B\x00\x49\x89\x73\x00\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x83\xEC\x00\x48\x8B\x3D\x00\x00\x00\x00\x4C\x8D\x35"), false);

      add_signature(HASH("look_weapon_info_slot"), SIG("\x48\x8B\xD1\x48\x8D\x0D\x00\x00\x00\x00\xE9\x00\x00\x00\x00\xCC\x48\x89\x5C\x24\x00\x48\x89\x6C\x24"), false);
      add_signature(HASH("file_weapon_info_from_handle"), SIG("\x66\x3B\x0D"), false);

      add_signature(HASH("get_class_name"), SIG("\x40\x56\x48\x83\xEC\x00\xC6\x05"), false);

      // Search for "#TF_Voice_Unavailable" and above it should be
      //sub_1022A060(&dword_10C681B8, "CHudChat"); Sig the function CHudChat is passed into along with the dword_XXXXXX(which is gHud pointer)
      add_signature(HASH("get_hud_element_addr"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x33\xDB\x48\x8B\xEA\x48\x8B\xF9\x39\x59"), false);

      add_signature(HASH("hud_element_addr"), SIG("\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x48\x8B\xD8\x48\x85\xC0\x74\x00\x48\x8B\x0D\x00\x00\x00\x00\x48\x8D\x15"), true);

      // Found by using the convar "tf_max_health_boost"
      add_signature(HASH("get_max_buffed_health_addr"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xF1\x41\x0F\xB6\xD8"), false);

      // Found by searching for "%c(%s) %s%c: %s" or "#Voice"
      // Get the return address of the call to ChatPrintf
      // ChatPrintf: (*(v19 + 76))(v12, v20, 0, "%c(%s) %s%c: %s", 3, v29, v21);
      add_signature(HASH("chatprintf_retaddr1"), SIG("\x4C\x8B\xB4\x24\x00\x00\x00\x00\xC6\x87"), false);

      // Search for "Flesh" you'll end up in a function called Impact. The function it's passed into is IsTruceActive
      add_signature(HASH("gamerules_addr"), SIG("\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x74\x00\xE8\x00\x00\x00\x00\x84\xC0\x74\x00\x48\x8D\x15\x00\x00\x00\x00\x49\x8B\xCC"), true);

      // Finding m_pSoCache & FindBaseTypecache search for "Failed to find party shared object\n" and you'll find a function with the pointer and the function.
      // if ( m_pSOCache
      //    && (pTypeCache = FindBaseTypeCache(m_pSOCache, 2003)) != 0
      //    && (GetCount = *(_DWORD *)(pTypeCache + 24)) != 0
      //    && (PartyClient = *(_DWORD **)(*(_DWORD *)(pTypeCache + 4) + 4 * GetCount - 4)) != 0 )
      add_signature(HASH("shared_object_cache_addr"), SIG("\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x74\x00\xBA\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x48\x85\xC0\x74\x00\x8B\x48\x00\x85\xC9\x74\x00\x48\x8B\x40\x00\xFF\xC9"), true);
      add_signature(HASH("find_base_type_cache_addr"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x0F\xB7\x59\x00\xBE"), false);

      // Has "CFlaggedEntitiesEnum::`vftable'" inside the function.
      add_signature(HASH("c_entity_sphere_query"), SIG("\x40\x53\x48\x83\xEC\x00\x48\x8B\xD9\xC7\x44\x24\x00\x00\x00\x00\x00\x33\xC9"), false);

      // search for "centerfire_projectile"
      add_signature(HASH("get_projectile_fire_setup"), SIG("\x48\x8B\xC4\x55\x48\x8D\xA8\x00\x00\x00\x00\x48\x81\xEC\x00\x00\x00\x00\x0F\x29\x78"), false);

      // It's been inlined in 64-bit.
      // \xFF\x50\x00\x48\x8B\x05\x00\x00\x00\x00\x83\x78\x00\x00\x74\x00\xF3\x44\x0F\x11\x44\x24\x00\x48\x8D\x53\x00\xC6\x44\x24\x00\x00\x45\x33\xC9\x41\xB8\x00\x00\x00\x00\xC7\x44\x24\x00\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\x49\x83\x3F
      // ! add_signature(HASH("util_traceray"), SIG("\xE8\x00\x00\x00\x00\xF3\x0F\x10\x05\x00\x00\x00\x00\x83\xC4\x18\x0F\x2F\x85\x00\x00\x00\x00\x77\x12"), true);

      // CTraceFilterSimple
      add_signature(HASH("ctrace_filter_simple"), SIG("\x48\x8D\x05\x00\x00\x00\x00\x48\x89\x51\x00\x48\x89\x01\x48\x8B\xC1\x44\x89\x41"), false);

      // search for "worldNormalX" and follow the call where "ping_circle" is passed into a function, that is the function to hook
      add_signature(HASH("dispatch_particle_effect"), SIG("\x48\x8B\xC4\x48\x89\x58\x00\x48\x89\x70\x00\x48\x89\x78\x00\x4C\x89\x70\x00\x55\x48\x8D\x68\x00\x48\x81\xEC\x00\x00\x00\x00\x41\x8B\x70"), false);

      // Look for CWeaponMedigun::FindAndHealTargets can be found using "WeaponMedigun_Vaccinator.Charged_tier_0%d" this will call "CWeaponMedigun::MaintainTargetInSlot"
      // Can be found inside CWeaponMedigun::MaintainTargetInSlot
      add_signature(HASH("collision_property_calcnearestpoint"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x49\x8B\xD8\x48\x8B\xF9\x4C\x8D\x44\x24"), false);

      // Can be found inside the "CTFPlayer::CanDisguise" which can be found using the string "set_cannot_disguise"
      add_signature(HASH("ctfplayer_hastheflag"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56\x48\x83\xEC\x00\x44\x8B\x89"), false);

      // Search for UpdateClientSideAnimations in client.dll obviously...
      add_signature(HASH("update_clientside_animations"), SIG("\x4C\x8B\xDC\x49\x89\x5B\x00\x49\x89\x6B\x00\x56\x57\x41\x56\x48\x83\xEC\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x8D\x1D\x00\x00\x00\x00\x33\xED\x48\x8D\x3D\x00\x00\x00\x00\x49\x89\x6B\x00\x4C\x8B\x50\x00\x4D\x85\xD2\x74\x00\x49\x89\x5B\x00\x48\x8D\x05\x00\x00\x00\x00\x49\x89\x7B\x00\x49\x8D\x53\x00\x49\x89\x43\x00\x45\x33\xC9\x48\x8D\x05\x00\x00\x00\x00\x45\x33\xC0\x49\x89\x43\x00\x49\x8B\xCA\x48\x8D\x05\x00\x00\x00\x00\xC7\x44\x24\x00\x00\x00\x00\x00\x49\x89\x43\x00\x49\x89\x6B\x00\x41\xFF\x92\x00\x00\x00\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\xAC\x24\x00\x00\x00\x00\x48\x8B\x0D\x00\x00\x00\x00\x48\x8B\x70\x00\x44\x8B\xB1\x00\x00\x00\x00\x45\x85\xF6\x74\x00\xC7\x44\x24\x00\x00\x00\x00\x00\x4C\x8B\xCF\x45\x33\xC0\xC6\x44\x24\x00\x00\x48\x8B\xD3\xFF\x15\x00\x00\x00\x00\x48\x8B\x0D"), false);


      // Found inside "CSniperDot::ClientThink"
      // Get the return address of this call SetControlPoint(*(this + 340), 2, clr);
      add_signature(HASH("csniperdot_clientthink_retaddr1"), SIG("\x4C\x8D\x44\x24\x00\x48\x8B\xCE\x48\x8D\x15\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x4C\x8D\x44\x24\x00\x84\xC0"), false);

      // These 2 signatures are unused in the cheat.
      // Found looking for return addresses in UpdateCritBoostEffect
      // !@! add_signature(HASH("is_crit_boosted_material_retaddr1"), SIG("\x8B\xCF\x84\xC0\x74\x00\x6A\x00\xE8\x00\x00\x00\x00\x84\xC0"), false);

      // UpdateCritBoostEffect return from IsCritBoosted, search for "critgun_weaponmodel_red" and look for call to IsCritBoosted at the top
      // !@! add_signature(HASH("update_crit_boost_effect_is_crit_boost_retaddr1"), SIG("\x84\xC0\x75\x00\x6A\x00\x8D\x8B\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x84\xC0\x75\x00\x8B\x83"), false);

      // The return address of this call in DisplayDamageFeedback
      // pLocalPlayer->IsPlayerClass( TF_CLASS_MEDIC )
      add_signature(HASH("displaydmgfeedback_isplayerclass_retaddr1"), SIG("\x84\xC0\x0F\x84\x00\x00\x00\x00\x48\x8B\xCE\xE8\x00\x00\x00\x00\x49\x3B\xC7"), false);

      add_signature(HASH("displaydmgfeedback_medicgethealtarget_retaddr"), SIG("\x49\x3B\xC7\x0F\x85\x00\x00\x00\x00\x49\x8B\x06"), false);

      add_signature(HASH("shared_random_int_addr"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x8B\xFA\x41\x8B\xD8"), false);

      // CNewParticleEffect::DrawModel call to CNewParticleEffect::Render return address
      add_signature(HASH("cnewparticle_effect_render_retaddr"), SIG("\x48\x8B\x03\xBA\x00\x00\x00\x00\x48\x8B\xCB\xFF\x90\x00\x00\x00\x00\x48\x8B\x03\x48\x8B\xCB\xFF\x90\x00\x00\x00\x00\x8B\xAC\x24"), false);

      // You can use tf_tournament_hide_domination_icons to find this function.
      // This is inlined 32 bit unknown in 64 bit.
      // ! add_signature(HASH("should_show_dueling_icon_addr"), SIG("\x57\x8B\xF9\x8B\x0D\x00\x00\x00\x00\x85\xC9\x74\x00\xE8\x00\x00\x00\x00\x84\xC0\x74\x00\xA1\x00\x00\x00\x00\x83\x78\x00\x00\x74\x00\x32\xC0\x5F\xC3"), false);

      add_signature(HASH("viewportpanel_interface_addr"), SIG("\x48\x8B\x0D\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x85\xC0"), true);

      // Search for "disguisename" this will land you in CDisguiseStatus::CheckName
      // X-ref and find the function that matches CDisguiseStatus::Paint get the retaddr from the InCond call
      add_signature(HASH("cdisguisestatus_paint_retaddr"), SIG("\x84\xC0\x75\x00\x40\x32\xF6\x40\x38\xB3"), false);

      // Search for "disguisename" this will land you in CDisguiseStatus::CheckName
      // X-ref and find the function that matches CDisguiseStatus::ShowAndUpdateStatus
      // X-ref again and you'll end up in CDisguiseStatus::ShouldDraw.
      // Sig the retaddr from the InCond call.
      add_signature(HASH("cdisguisestatus_shoulddraw_retaddr"), SIG("\x84\xC0\x74\x00\x48\x83\xC3\x00\x48\x8B\xCB"), false);

      // Use cl_hud_playerclass_use_playermodel
      // Inside of the function "CTFHudPlayerClass::OnThink" get the return addresses of the InCond calls.
      add_signature(HASH("ctfhudplayerclass_onthink_retaddr"), SIG("\x84\xC0\x74\x00\xBA\x00\x00\x00\x00\x49\x8D\x8E\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x84\xC0\x75"), false);

      // Search for "#GameUI_HudPlayerClassUsePlayerModelDialogCancel"
      // Scroll down and look for a function like this
      //  v22 = InCond((v3 + 6392), 3);
      // Sig the return address.
      add_signature(HASH("ctfhudplayerclass_update_model_panel_retaddr"), SIG("\x44\x0F\xB6\xE0\x84\xC0\x74"), false);

      // return address of the InCondition call inside of load_itempreset_callback
      add_signature(HASH("load_itempreset_incond_retaddr"), SIG("\x84\xC0\x0F\x85\x00\x00\x00\x00\x83\x3B"), false);

      // function at the top in TracerCallback
      // Unused
      // ! add_signature(HASH("get_effect_data"), SIG("\xE8\x00\x00\x00\x00\x8B\xF8\x85\xFF\x0F\x84\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x83\x79\x00\x00\x0F\x84\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x56"), true);

      // The return address of the call InventoryManager()->UpdateInventoryEquippedState inside CTFPlayerInventory::VerifyLoadoutItemsAreValid
      // If you have a hook on updateInventoryEquippedState then you can simply just print the retaddr out and get the signature that way.
      add_signature(HASH("updateinvequippedstate_retaddr"), SIG("\xB0\x00\xE9\x00\x00\x00\x00\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x85\xD2"), false);
    
      // You can find this by searching for 'projectile_spread_angle'
      add_signature(HASH("get_spread_angles_addr"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x0F\x29\x74\x24\x00\x48\x8B\xDA\x48\x8B\xF9\xE8\x00\x00\x00\x00\x48\x8B\xC8"), false);
      
      // CTFPartyClient function
      add_signature(HASH("ctfpartyclient_func_addr"), SIG("\x48\x8B\x05\x00\x00\x00\x00\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56"), false);
      
      // 
      add_signature(HASH("ctf_firebullet_retaddr1"), SIG("\x48\x8B\xD8\x48\x85\xC0\x74\x00\x48\x8B\xC8\xE8\x00\x00\x00\x00\x48\x8B\x4B"), false);

      // Just print the retaddr inside the relationship function. Both of these checks are inside the UpdateActiveParty Function
      add_signature(HASH("iclientfriends_getfriendrelationship_retaddr1"), SIG("\x83\xF8\x00\x0F\x85\x00\x00\x00\x00\x83\x7C\x24"), false);
      add_signature(HASH("iclientfriends_getfriendrelationship_retaddr2"), SIG("\xBB\x00\x00\x00\x00\x83\xF8\x00\x74\x00\x41\x8B\xDC\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x48\x8B\xD0\x48\x8D\x0D\x00\x00\x00\x00\x44\x8B\xCB\x44\x8B\xC6\xFF\x15\x00\x00\x00\x00\x48\x8B\x54\x24"), false);
    
      // INside CStudioHdr::pSeqDesc look for &unk_10E2240;
      add_signature(HASH("nil_seq_desc_addr"), SIG("\x48\x8D\x05\x00\x00\x00\x00\x48\x8B\x74\x24\x00\x48\x83\xC4\x00\x5F\xC3\x44\x8B\xCE"), true);

      // *** ERROR
      add_signature(HASH("setup_bones_exception_handler_addr"), SIG("\x74\x00\x49\x8B\x46\x00\x49\x8D\x4E\x00\xFF\x90"), false);

      // Find by using StartContextAction using string "+context_action"
      add_signature(HASH("get_equip_wearable_for_loadout_slot_addr"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x54\x41\x56\x41\x57\x48\x83\xEC\x00\x44\x8B\xB9"), false);
    }

    set_module(HASH("engine.dll"));
    {
      add_signature(HASH("cl_runprediction"), SIG("\x4C\x8B\xDC\x49\x89\x5B\x00\x57\x48\x83\xEC\x00\x48\x8B\x1D\x00\x00\x00\x00\x33\xFF\x49\x89\x7B\x00\x48\x8B\x03\x48\x85\xC0\x74\x00\x48\x8D\x0D\x00\x00\x00\x00\x45\x33\xC9\x49\x89\x4B\x00\x49\x8D\x53\x00\x48\x8D\x0D\x00\x00\x00\x00\x45\x33\xC0\x49\x89\x4B\x00\x48\x8D\x0D\x00\x00\x00\x00\x49\x89\x4B\x00\x48\x8D\x0D\x00\x00\x00\x00\xC7\x44\x24\x00\x00\x00\x00\x00\x49\x89\x4B\x00\x48\x8B\xC8\x49\x89\x7B\x00\xFF\x90\x00\x00\x00\x00\x48\x8B\x7C\x24\x00\x48\x8B\x1D\x00\x00\x00\x00\x83\x3D"), false);

      //Search for "Recording to %s...\n" once in the function there will be text "demostart" that's the function.
      // Read the comment for cl_sendstringcommand it gets passed.

      // There are two ways to fetch a pointer to the client state
      // Method 1: Search for "Recording to %s...\n" and look for a call like "sub_10085B10(&dword_10463908, (int)"demorestart");"
      // Look in the assembly and look for mov ecx, 00000000 before that function call
      // Method 2: Search for "CDemo::Play: failed to create demo net channel\n" or "Playing demo from %s.\n" and look for a call like "v8 = sub_101A4920(0, 0, "DEMO", (int)&dword_10463908, 0, *((_DWORD *)Sourcea + 3));"
      // In those function parameters look for something like "(int)&dword_10463908"
      add_signature(HASH("client_state"), SIG("\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x48\x8D\x54\x24\x00\x48\x8D\x0D\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x48\x8B\x0D"), true);

      // The return address of Buf_AddtextWithMarkerers in BaseClientState::ProcessStringCmd
      add_signature(HASH("baseclientstate_processstringcmd_retaddr1"), SIG("\xB0\x00\x48\x83\xC4\x00\x5B\xC3\x48\x8B\x4A"), false);

      // Search for "StartSound" once in the function look for the string soundname sig the function the string is being passed into.
      add_signature(HASH("keyvalues_setstring"), SIG("\xE8\x00\x00\x00\x00\xF3\x0F\x10\x57\x00\x48\x8D\x15\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\xF3\x0F\x10\x57\x00\x48\x8D\x15\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\xF3\x0F\x10\x57\x00\x48\x8D\x15\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\xF3\x0F\x10\x57\x00\x48\x8D\x15\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\xF3\x0F\x10\x57\x00\x48\x8D\x15\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\xF3\x0F\x10\x57"), true);

      // Search for "Software\\Valve\\Steam" and when inside function it is there.
      add_signature(HASH("steam3client_addr"), SIG("\x48\x8D\x05\x00\x00\x00\x00\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x57"), false);
      
      // Search for the string "Current alias commands:"
      add_signature(HASH("cmd_alias_list"), SIG("\x48\x8B\x1D\x00\x00\x00\x00\x48\x85\xDB\x0F\x84\x00\x00\x00\x00\x0F\x1F\x40"), true);

      add_signature(HASH("staticpropmgr_drawstaticprop_retaddr1"), SIG("\x0F\xB6\xBC\x24\x00\x00\x00\x00\x84\xC0"), false);

      return true;
    }
#if defined(DEV_MODE)
    set_module(HASH("server.dll"));
    if(false){
      add_signature(HASH("get_server_animating_addr"), SIG("\x55\x8B\xEC\x8B\x55\x00\x85\xD2\x7E\x00\xA1"), false);
      add_signature(HASH("draw_server_hitbox_addr"), SIG("\xE8\x00\x00\x00\x00\x8B\x7D\xD4\x80\x7D\xF3\x00"), true);
    }
#endif

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

      add_hook(HASH("run_command_hook"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56\x48\x83\xEC\x00\x0F\x29\x74\x24\x00\x49\x8B\xE9"), false); 
      add_hook(HASH("post_think_hook"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\x1D\x00\x00\x00\x00\x48\x8B\xF9\x48\x8B\xCB\x48\x8B\x03\xFF\x90\x00\x00\x00\x00\x48\x8B\x07"), false);

 
      // look at CTFPlayerShared::OnAddBurning can be found by searching for "Fire.Engulf"
      // Step into view->SetScreenOverlayMaterial( pMaterial ); using a debugger.
      add_hook(HASH("cviewrender_setscreenoverlaymaterial_hook"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xF9\x48\x8B\xDA\x48\x8B\x09\x48\x3B\xCA"), false);

      // search for C_BaseAnimating::SetupBones and sig the function it is in.
      add_hook(HASH("base_animating_setupbones_hook"), SIG("\x48\x8B\xC4\x44\x89\x40\x00\x48\x89\x50\x00\x55\x53"), false);

      // Found by printing proxy address for m_bDormant and using cheat engine to find what writes to it
      /*
        __int64 __fastcall sub_7FFCA7C7C430(__int64 a1, unsigned __int8 a2)
        {
          if ( !(*(*a1 + 2872i64))(a1) )
          {
            if ( !(*(*(a1 + 16) + 64i64))(a1 + 16) && a2 && *(a1 + 4264) >= 2 )
              minigun_weapon_sound_update(a1); <- Can use this function to find the set_dormant function
            if ( !(*(*(a1 + 16) + 64i64))(a1 + 16) && a2 && *(a1 + 4264) )
            {
              sub_7FFCA7C7D270(a1);
              sub_7FFCA7C7D1B0(a1);
            }
          }
          return set_dormant(a1, a2);
        }
      */
      add_hook(HASH("set_dormant_hook"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\x01\x0F\xB6\xFA\x48\x8B\xD9\x88\x91\x00\x00\x00\x00\xFF\x90\x00\x00\x00\x00\x48\x8D\x8B"));

      // Search for "minigun_fire" and the function should be right below.
      // SIG UPDATED:
      add_hook(HASH("calc_is_attack_critical_hook"), SIG("\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xF9\xE8\x00\x00\x00\x00\x48\x8B\xC8\xC7\x44\x24\x00\x00\x00\x00\x00\x4C\x8D\x0D\x00\x00\x00\x00\x33\xD2\x4C\x8D\x05\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x48\x8B\xF0\x48\x85\xC0\x0F\x84\x00\x00\x00\x00\x48\x8B\x10"), false);

      // search for "minigun_no_spin_sounds" there should be 3 cases of this string in the function
      // there should be an if statement in the function that looks like this
      //if ( result == 1 && *(a1 + 1078) == 12 )
      add_hook(HASH("minigun_weapon_sound_update_hook"), SIG("\x40\x53\x56\x57\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\xF1\x44\x0F\x29\x44\x24"), false);

      // Search for the string "WeaponMedigun_Vaccinator.Toggle" and sig the functions the string gets passed in.
      add_hook(HASH("emit_sound_hook"), SIG("\x48\x89\x5C\x24\x00\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\x6C\x24\x00\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x3D"));

      // Search for the string "models/props_halloween/bumpercar_cage.mdl" and goto the function you're in.
      // Once inside look for the FollowEntity function. Sig that function.
      add_hook(HASH("cbaseentity_followentity_hook"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x41\x0F\xB6\xF8\x48\x8B\xD9\x48\x85\xD2"));

      // Search for "tf_viewmodels_offset_override" and sig the function it is in.
      add_hook(HASH("ctfviewmodel_calcviewmodelview_hook"), SIG("\x48\x89\x74\x24\x00\x55\x41\x56\x41\x57\x48\x8D\xAC\x24"), false);

      // search for "mat_motion_blur_enabled" in client.dll and follow the address from call "(*(void (__thiscall **)(int, _DWORD *))(*(_DWORD *)v24 + 156))(v24, a2)"
      add_hook(HASH("do_post_screen_space_effects_hook"), SIG("\x40\x53\x48\x83\xEC\x00\x48\x8B\xDA\xE8\x00\x00\x00\x00\x84\xC0\x74\x00\xE8"), false);

      // Use the "cl_bobup" convar once in the function with 'cl_bobup' x-ref out of that function.
      /*
        __int64 __fastcall sub_7FFD3C192240(__int64 a1, __int64 a2, __int64 a3, __int64 a4)
        {
          __int64 result; // rax
        
          result = sub_7FFD3BD6D0E0();
          if ( result == 1 )
          {
            result = (*(*(a1 + 8) + 72i64))(a1 + 8);
            if ( result )
            {
              sub_7FFD3C2C07B0(a2, (a1 + 3292));
              return AddViewmodelbobhelper(a3, a4, a1 + 3292);
            }
          }
          return result;
        }
      */

      // search for "CSecondaryTargetID"
      // xref that function to its vmt table and go down 3 and thats C_TFPlayer::IsPlayerOnSteamFriendsList
      add_hook(HASH("ctfplayer_playeronsteamfriends_hook"), SIG("\x40\x57\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\xFA\xE8"), false);

      // Can be found by using the cvar "tf_weapon_criticals_bucket_default" it's the cbasecombatweapon constructor.
      add_hook(HASH("cbase_combat_weapon_hook"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xD9\xE8\x00\x00\x00\x00\x33\xFF\xC7\x83"), false);

      // CTFPlayerShared::OnAddedCondition
      add_hook(HASH("ctfplayershared_onconditionadded_hook"), SIG("\x48\x8B\xC4\x55\x53\x48\x8D\x68\x00\x48\x81\xEC\x00\x00\x00\x00\xFF\xCA"), false);

      //  Called in CTFPlayerShared::RemoveCond you can find this function by looking for the string "damage_prevented"
      add_hook(HASH("ctfplayershared_onconditionremoved_hook"), SIG("\x40\x57\x48\x83\xEC\x00\xFF\xCA"), false);

      // called inside of the load_itempreset_callback function
      // String can be used: Loadout change denied. Changing presets too quickly.
      add_hook(HASH("inventorymanager_loadpreset_hook"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x41\x0F\xB6\xF0\x48\x8B\xF9"), false);

      // Use r_drawtracers or r_drawtracers_firstperson to find this function.
      /*
      The right function has something like this in it
      if ( v17 == 0.0 )
            v7 = 5000.0;
      */
      add_hook(HASH("tracer_callback_hook"), SIG("\x48\x89\x5C\x24\x00\x56\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\xD9\xE8"), false);

      // Search for "muzzle_sentry2" or "muzzle_sentry" in client.dll to find this function.
      add_hook(HASH("sentry_muzzle_flash_hook"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x8B\x71\x00\x8B\x59"), false);


      // TOSS_JAR_THINK
      add_hook(HASH("set_context_think_hook"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x48\x83\x7C\x24"), false);
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

      //Use the string "DPT_" to find these functions
      add_hook(HASH("int_decode_hook"), SIG("\x40\x53\x48\x83\xEC\x00\x48\x89\x74\x24\x00\x48\x8B\xD9\x48\x8B\x71"), false);
      
      // Search for "FireEvent: event '%s' not registered.\n" or "Game event \"%s\", Tick %i:\n"
      // Xref the function and look for "return sub_170B90(this, a1, 0, 1);" and thats the function to sig (not the return function)
      add_hook(HASH("fire_event_hook"), SIG("\x41\xB1\x00\x45\x33\xC0\xE9"), false);

      // search for "WARNING:  Connection Problem" and that is the function to hook
      add_hook(HASH("cl_move_hook"), SIG("\x40\x55\x53\x48\x8D\xAC\x24\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x48\x2B\xE0\x83\x3D"), false);

      // search for "UDP <- %s: sz=%i seq=%i ack=%i rel=%i ch=%d, tm=%f rt=%f wire=%i\n" and thats the function to hook
      add_hook(HASH("process_packet_hook"), SIG("\x44\x88\x44\x24\x00\x48\x89\x54\x24\x00\x53"), false);

      // search for "%s:send reliable stream overflow\n" or "CNetChan_TransmitBits->send" or "datagram" and thats the function to hook
      add_hook(HASH("send_datagram_hook"), SIG("\x40\x55\x57\x41\x56\x48\x8D\xAC\x24"), false);

      // finding this was kinda aids.
      add_hook(HASH("engine_get_player_info_hook"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x8D\x7A"), false);

      // Search for "StartStaticSound" and sig the function it is in
      add_hook(HASH("s_startsound_hook"), SIG("\x40\x53\x48\x83\xEC\x00\x48\x83\x79\x00\x00\x48\x8B\xD9\x75\x00\x33\xC0"), false);


      // search for "ClientDLL_FrameStageNotify"
      add_hook(HASH("frame_stage_notify_hook"), SIG("\x4C\x8B\xDC\x56\x48\x83\xEC"), false);

    }

    set_module(HASH("studiorender.dll"));
    {
      // Search for "Missing LODs for %s, lod index is %d.\n" this will land you in CStudioRender::DrawModel step out of the function and that will land you in CStudioRenderContext::DrawModel BOOM!
      add_hook(HASH("studio_render_context_drawmodel_hook"), SIG("\x4C\x89\x4C\x24\x00\x48\x89\x4C\x24\x00\x56"), false);

      // Go in engine.dll and search for the string "CModelRender::ForcedMaterialOverride" this function calls studio render one.
      // Use a debugger to break point in it.
      add_hook(HASH("studio_render_forced_material_override_hook"), SIG("\x48\x89\x91\x00\x00\x00\x00\x44\x89\x81"), false);
    }

    set_module(HASH("materialsystem.dll"));
    {

      // NOTE these in 32-bit tf2 never seemed to actually fix the black chams issue.
      //CMaterial::Uncache
      //You can find this function by searching for the string "materials/%s.vmt" the string will be at the bottom of the function.
      add_hook(HASH("cmaterial_uncache_hook"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\xF9"), false);

      // You can use cheat engine and check what accesses this address on the ref counter.
      // @ add_hook(HASH("cmaterial_deleteifunreferenced_hook"), SIG("\x56\x8B\xF1\x83\x7E\x00\x00\x7F\x00\x57"), false);
    }

    set_module(HASH("steamclient64.dll"));
    {
      // Inside CTFGCClientSystem::PingThink()
      // pUtils->GetDirectPingToPOP( id ) is called to measure ping to servers
      add_hook(HASH("get_direct_ping_to_pop_hook"), SIG("\x40\x53\x48\x83\xEC\x00\x33\xC9\x8B\xDA"), false);

      // No longer used.
      // Literally has this inside the function "IClientFriends", "GetFriendRelationship"
      // Incase you get a huge ass function cause IDA is being stupid.
      // You can look for IClientFriendsMap::`vftable
      // The 11th function should it be it and you'll see the strings mentioned above.

      //bypass_pkg_errors = true;
      //add_hook(HASH("iclientfriends_getfriendrelationship_hook"), SIG("\x40\x53\x56\x57\x48\x83\xEC\x00\x45\x33\xC9\x48\x8B\xDA\x48\x8B\xF9\x48\x8D\x4C\x24\x00\x41\x8D\x51\x00\x45\x8D\x41\x00\xE8\x00\x00\x00\x00\xB2\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\xB2\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x8B\x47\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\x89\x84\x24\x00\x00\x00\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\xC7\x84\x24\x00\x00\x00\x00\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x48\x8B\xD3\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\xC7\x84\x24\x00\x00\x00\x00\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x8B\x57\x00\x48\x8D\x35\x00\x00\x00\x00\x48\x8B\x4F\x00\x4C\x8D\x0D\x00\x00\x00\x00\x4C\x8D\x44\x24\x00\x48\x89\x74\x24\x00\xE8\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x48\x8B\xC8\x48\x8B\xD8\xE8\x00\x00\x00\x00\x84\xC0\x75\x00\x48\x8B\xD6\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x33\xDB\xEB\x00\x44\x0F\xB6\x84\x24\x00\x00\x00\x00\x41\x80\xF8\x00\x74\x00\x48\x8B\xD6\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x33\xDB\xEB\x00\x41\xB8\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\xA8\x00\x74\x00\x48\x8B\x4F\x00\x48\x8B\xD3\xE8\x00\x00\x00\x00\x8B\x9C\x24\x00\x00\x00\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x8B\xC3\x48\x83\xC4\x00\x5F\x5E\x5B\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x40\x53\x56\x57\x48\x83\xEC\x00\x45\x33\xC9\x48\x8B\xDA\x48\x8B\xF9\x48\x8D\x4C\x24\x00\x41\x8D\x51\x00\x45\x8D\x41\x00\xE8\x00\x00\x00\x00\xB2\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\xB2\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x8B\x47\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\x89\x84\x24\x00\x00\x00\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\xC7\x84\x24\x00\x00\x00\x00\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x48\x8B\xD3\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\xC7\x84\x24\x00\x00\x00\x00\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x8B\x57\x00\x48\x8D\x35\x00\x00\x00\x00\x48\x8B\x4F\x00\x4C\x8D\x0D\x00\x00\x00\x00\x4C\x8D\x44\x24\x00\x48\x89\x74\x24\x00\xE8\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x48\x8B\xC8\x48\x8B\xD8\xE8\x00\x00\x00\x00\x84\xC0\x75\x00\x48\x8B\xD6\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x33\xDB\xEB\x00\x44\x0F\xB6\x84\x24\x00\x00\x00\x00\x41\x80\xF8\x00\x74\x00\x48\x8B\xD6\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x33\xDB\xEB\x00\x41\xB8\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\xA8\x00\x74\x00\x48\x8B\x4F\x00\x48\x8B\xD3\xE8\x00\x00\x00\x00\x8B\x9C\x24\x00\x00\x00\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x8B\xC3\x48\x83\xC4\x00\x5F\x5E\x5B\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x48\x89\x5C\x24\x00\x55\x56\x57\x48\x83\xEC"), false);
      //add_hook(HASH("iclientfriends_getfriendrelationship_hook"), SIG("\x40\x53\x56\x57\x48\x83\xEC\x00\x45\x33\xC9\x48\x8B\xDA\x48\x8B\xF9\x48\x8D\x4C\x24\x00\x41\x8D\x51\x00\x45\x8D\x41\x00\xE8\x00\x00\x00\x00\xB2\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\xB2\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x8B\x47\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\x89\x84\x24\x00\x00\x00\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\xC7\x84\x24\x00\x00\x00\x00\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x48\x8B\xD3\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\xC7\x84\x24\x00\x00\x00\x00\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x8B\x57\x00\x48\x8D\x35\x00\x00\x00\x00\x48\x8B\x4F\x00\x4C\x8D\x0D\x00\x00\x00\x00\x4C\x8D\x44\x24\x00\x48\x89\x74\x24\x00\xE8\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x48\x8B\xC8\x48\x8B\xD8\xE8\x00\x00\x00\x00\x84\xC0\x75\x00\x48\x8B\xD6\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x33\xDB\xEB\x00\x44\x0F\xB6\x84\x24\x00\x00\x00\x00\x41\x80\xF8\x00\x74\x00\x48\x8B\xD6\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x33\xDB\xEB\x00\x41\xB8\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\xA8\x00\x74\x00\x48\x8B\x4F\x00\x48\x8B\xD3\xE8\x00\x00\x00\x00\x8B\x9C\x24\x00\x00\x00\x00\x83\x7C\x24\x00\x00\x7C\x00\x48\x8B\x54\x24\x00\x48\x85\xD2\x74\x00\x48\x8B\x0D\x00\x00\x00\x00\x45\x33\xC0\x48\x8B\x09\x4C\x8B\x09\x41\xFF\x51\x00\x8B\xC3\x48\x83\xC4\x00\x5F\x5E\x5B\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x40\x53\x56\x57\x48\x83\xEC\x00\x45\x33\xC9\x48\x8B\xDA\x48\x8B\xF9\x48\x8D\x4C\x24\x00\x41\x8D\x51\x00\x45\x8D\x41\x00\xE8\x00\x00\x00\x00\xB2\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\xB2\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x8B\x47\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\x89\x84\x24\x00\x00\x00\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\xC7\x84\x24\x00\x00\x00\x00\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x48\x8B\xD3\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\xC7\x84\x24\x00\x00\x00\x00\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x48\x8D\x4C\x24\x00\xE8\x00\x00\x00\x00\x8B\x57\x00\x48\x8D\x35\x00\x00\x00\x00\x48\x8B\x4F\x00\x4C\x8D\x0D\x00\x00\x00\x00\x4C\x8D\x44\x24\x00\x48\x89\x74\x24\x00\xE8\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x48\x8B\xC8\x48\x8B\xD8\xE8\x00\x00\x00\x00\x84\xC0\x75\x00\x48\x8B\xD6\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x33\xDB\xEB\x00\x44\x0F\xB6\x84\x24\x00\x00\x00\x00\x41\x80\xF8\x00\x74\x00\x48\x8B\xD6\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x33\xDB\xEB\x00\x41\xB8\x00\x00\x00\x00\x48\x8D\x94\x24\x00\x00\x00\x00\x48\x8B\xCB\xE8\x00\x00\x00\x00\xA8\x00\x74\x00\x48\x8B\x4F\x00\x48\x8B\xD3\xE8\x00\x00\x00\x00\x8B\x9C\x24\x00\x00\x00\x00\x83\x7C\x24\x00\x00\x7C\x00\x48\x8B\x54\x24\x00\x48\x85\xD2\x74\x00\x48\x8B\x0D\x00\x00\x00\x00\x45\x33\xC0\x48\x8B\x09\x4C\x8B\x09\x41\xFF\x51\x00\x8B\xC3\x48\x83\xC4\x00\x5F\x5E\x5B\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x55"), false);
      //bypass_pkg_errors = false;
    }


    set_module(HASH("crashhandler64.dll"));
    if(false){ // needs a new function this one doesn't catch exceptions.
      //add_hook(HASH("steam_exception_handler_hook"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xF9\x48\x8D\x0D"), false);
    }

    set_module(HASH("serverbrowser.dll"));
    {
      add_hook(HASH("is_blacklisted_hook"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xD9\x48\x8B\xFA\x48\x81\xC1"), false);
    }


    return true;
  }
};