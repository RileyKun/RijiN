#pragma once

class c_procedure_tf2 : public c_procedure_base{
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

      u32 global_data_ptr = (u32)find_signature(HASH("client.dll"), HASH(".text"), SIG("\xA1\x00\x00\x00\x00\xD9\x40\x0C\xD9\x9E\x00\x00\x00\x00\xC6\x86\x00\x00\x00\x00\x00\xF3\x0F\x2C\x8E\x00\x00\x00\x00"), false, 0x1 );

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

  bool patch_lobby(){
    set_module(HASH("client.dll"));

    return true;

    // Patch #1 (Allow tf_party_request_join_user to work on non steam friends)
    // Patches out the check that prevents you from joining non steam friends.
    u32 allow_lobby_joining = (u32)find_signature(HASH("client.dll"), HASH(".text"), SIG("\x75\x00\x8D\x4D\x00\xE8\x00\x00\x00\x00\x50\x68\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x83\xC4\x00\x5E\x8B\xE5\x5D\xC3\xFF\x75"), false);
    assert(allow_lobby_joining != 0);
    if(allow_lobby_joining != 0){
      u8  opcode          = XOR32(0xEB);
      u32 old_permissions = 0;
      if(inject->virtual_protect((void*)allow_lobby_joining, sizeof(u8), PAGE_EXECUTE_READWRITE, &old_permissions)){
        if(!inject->write_memory(allow_lobby_joining, &opcode, sizeof(u8))){
          DBG("[-] Failed party patch #1\n");
          return false;
        }

        DBG("[+] Applied party patch #1\n");
        inject->virtual_protect((void*)allow_lobby_joining, sizeof(u8), old_permissions, &old_permissions);
      }
    }

    // Patch #2 (Another check that prevents us from joining other people's lobbies)
    // Cause: invalid state v2
    u32 invalid_state_code_ptr = (u32)find_signature(HASH("client.dll"), HASH(".text"), SIG("\x75\x00\xE8\x00\x00\x00\x00\x50\x68\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x83\xC4\x00\x32\xC0\x5E\x8B\xE5\x5D\xC2\x00\x00\xFF\x76\x00\xFF\x76\x00\xE8\x00\x00\x00\x00\x50\x68\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x83\xC4\x00\x8D\x4D"), false);
    assert(invalid_state_code_ptr != 0);
    if(invalid_state_code_ptr != 0){
      u8  opcode          = XOR32(0xEB);
      u32 old_permissions = 0;
      if(inject->virtual_protect((void*)invalid_state_code_ptr, sizeof(u8), PAGE_EXECUTE_READWRITE, &old_permissions)){
        if(!inject->write_memory(invalid_state_code_ptr, &opcode, sizeof(u8))){
          DBG("[-] Failed party patch #2\n");
          return false;
        }

        DBG("[+] Applied party patch #2 (%X)\n", invalid_state_code_ptr);
        inject->virtual_protect((void*)invalid_state_code_ptr, sizeof(u8), old_permissions, &old_permissions);
      }
    }

    // Patch #3 (Another check that prevents us from joining other people's lobbies) (Related to send_join_request)
    // Cause: invalid state
    u32 invalid_state_code_ptr2 = (u32)find_signature(HASH("client.dll"), HASH(".text"), SIG("\x75\x00\xE8\x00\x00\x00\x00\x50\x68\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x83\xC4\x00\x32\xC0\x5E\x8B\xE5\x5D\xC2\x00\x00\xFF\x76\x00\xFF\x76\x00\xE8\x00\x00\x00\x00\x50\x68\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x83\xC4\x00\x8B\xCE"), false);
    assert(invalid_state_code_ptr2 != 0);
    if(invalid_state_code_ptr2 != 0){
      u8  opcode          = XOR32(0xEB);
      u32 old_permissions = 0;
      if(inject->virtual_protect((void*)invalid_state_code_ptr2, sizeof(u8), PAGE_EXECUTE_READWRITE, &old_permissions)){
        if(!inject->write_memory(invalid_state_code_ptr2, &opcode, sizeof(u8))){
          DBG("[-] Failed party patch #3\n");
          return false;
        }

        DBG("[+] Applied party patch #3\n");
        inject->virtual_protect((void*)invalid_state_code_ptr2, sizeof(u8), old_permissions, &old_permissions);
      }
    }

    return true;
  }

  bool write_signatures() override{
    set_section(HASH(".text"));

    // Easy way to ensure the loading screen is passed
    set_module(HASH("serverbrowser.dll"));

    set_module(HASH("client.dll"));
    {
      // search "Pitch: %6.1f   Yaw: %6.1f %38s"
      // compare code with https://github.com/ValveSoftware/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/mp/src/game/client/in_camera.cpp#L418-L439
      add_signature(HASH("input"), SIG("\x8B\x0D\x00\x00\x00\x00\x68\x00\x00\x00\x00\x8B\x01\x8B\x40\x00\xFF\xD0\xD9\x45\x00\xD9\xC9\xDF\xE9\xDD\xD8\x9F\xF6\xC4\x00\x7B\x00\xA1\x00\x00\x00\x00\xF3\x0F\x10\x4D"), false, 0x2);

      add_signature(HASH("get_team_check_scoreboard_retaddr1"), SIG("\x83\xF8\x00\x75\x00\x8B\x45\x00\x83\xF8\x00\x75"));
      add_signature(HASH("get_team_status_panel_retaddr1"), SIG("\x3B\xC6\x0F\x94\x45\xFF"));

      // For the spy cloak / disguise hit markers fix.
      // v135 = (*(*a2 + 20))(a2, "showdisguisedcrit", 0);
      //LOBYTE(v6) = InCond((player + 6128), 3); <--- ret addr of this
      // if ( !v6 || v135 )
      add_signature(HASH("ctfplayer_fireevent_retaddr1"), SIG("\x84\xC0\x74\x0A\x80\x7D\x0B\x00"));

      // Search for "DamagedPlayer" once in the function scroll up until you see.
      //if ( sub_10405AC0(LODWORD(v10), 8) ) sig the retaddr of that.
      //in tf_hud_account.cpp (DisplayDamageFeedback)
      add_signature(HASH("displaydmgfeedback_retaddr1"), SIG("\x84\xC0\x0F\x84\x00\x00\x00\x00\x8B\x06\x8B\xCE\x8B\x80\x00\x00\x00\x00\xFF\xD0\x8B\xF0\x8B\x43"));

      // The return address of UTIL_TraceLine call.
      add_signature(HASH("displaydmgfeedback_retaddr4"), SIG("\xF3\x0F\x10\x45\x00\x83\xC4\x00\x0F\x2F\x05\x00\x00\x00\x00\x72"));

      // Used for skipping "if ( Q_stricmp( panelname, PANEL_INFO ) == 0 && mode )" in __MsgFunc_VGUIMenu for the motd
      add_signature(HASH("msg_func_is_info_panel_allowed_retaddr1"), SIG("\xFF\xD0\x84\xC0\x74\x19\x8B\x06"), false, 0x2);

      // Search for string "minigun_no_spin_sounds" it will land you inside C_TFMinigun::WindDown this is where the function will be at.
      // found inside TF2_SetSpeed
      add_signature(HASH("tf2_getmaxspeed"), SIG("\x55\x8B\xEC\x83\xEC\x00\x83\x3D\x00\x00\x00\x00\x00\x56\x57\x8B\xF1"));

      // Search for "CHudUpgradePanel" you should be in C_TFPlayer::AvoidPlayers
      // if ( *&v2 != 0.0 )
      // v5 = sub_102294C0("CHudUpgradePanel");
      // x-ref C_TFPlayer:AvoidPlayers and you'll be in C_TFPlayer::CreateMove
      // Follow: https://github.com/TheAlePower/TeamFortress2/blob/1b81dded673d49adebf4d0958e52236ecc28a956/tf2_src/game/client/tf/c_tf_player.cpp#L6995

      // You'll see a function call with 7 passed as an arg.
      add_signature(HASH("ctfplayer_createmove_retaddr1"), SIG("\x84\xC0\x75\x15\x6A\x36"));

      // You'll see a function call with 82(TF_COND_HALLOWEEN_KART) passed as an arg.
      add_signature(HASH("ctfplayer_createmove_retaddr2"), SIG("\x84\xC0\x74\x00\xF3\x0F\x10\x05\x00\x00\x00\x00\x33\xC0"));

      // You'll see a function call with 87(TF_COND_FREEZE_INPUT) passed as an arg.
      add_signature(HASH("ctfplayer_createmove_retaddr3"), SIG("\x84\xC0\x74\x00\xF3\x0F\x10\x05\x00\x00\x00\x00\x33\xC0"));

      // Required for unlocking the view for TF_COND_HALLOWEEN_KART.
      // "CTFInput::ApplyMouse" Found by using the convar "tf_halloween_kart_cam_follow"
      add_signature(HASH("ctfinput_applymouse_retaddr1"), SIG("\x84\xC0\x0F\x84\x00\x00\x00\x00\xF3\x0F\x10\x8E\x00\x00\x00\x00\xF3\x0F\x10\x87\x00\x00\x00\x00"));

      // check CTFWearable::ShouldDraw() we're checking returnaddress of InCond( pOwner == C_TFPlayer::GetLocalTFPlayer() && pOwner->m_Shared.InCond( TF_COND_ZOOMED ) )
      add_signature(HASH("ctfwearable_shoulddraw_retaddr1"), SIG("\x84\xC0\x0F\x85\x00\x00\x00\x00\x6A\x00\x8B\xCB\xE8\x00\x00\x00\x00\x84\xC0\x0F\x84\x00\x00\x00\x00\x8B\x47"));

      // sig: \x56\x8B\xF1\x8D\x4E\xFC\xE8\x00\x00\x00\x00\x84\xC0\x74\x16
      // C_TFPlayer::ShouldDraw
      // get the return address of InCondition.
      add_signature(HASH("ctfplayer_shoulddraw_retaddr1"), SIG("\x84\xC0\x74\x00\x5E\x32\xC0\x5B\xC3"));

      // found in C_TFPlayer::CreateMove
      add_signature(HASH("ctfplayer_canmoveduringtaunt"), SIG("\xE8\x00\x00\x00\x00\x84\xC0\x75\x13\xC7\x46\x00\x00\x00\x00\x00"), true);

      // search for "sapper_sentry1_fx" x-ref the function you end up in. There should only be 1 function that's CTFPlayerShared::OnConditionAdded.
      // x-ref the function until you see something like this
      // *(this[2] + v6 + 4) = 0;
      // CTFPlayerShared::OnConditionAdded(this, a2);
      add_signature(HASH("ctfplayershared_addcond"), SIG("\xE8\x00\x00\x00\x00\xD9\x05\x00\x00\x00\x00\x6A\x00\x51\x8B\x8F"), true);

      // search for "damage_prevented" and sig the function you're in.
      add_signature(HASH("ctfplayershared_removecond"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x56\x8B\x75\x00\x8B\xD9\x56\xE8\x00\x00\x00\x00\x84\xC0"), false);

      add_signature(HASH("view_render_beams_interface"), SIG("\x8B\x0D\x00\x00\x00\x00\xF3\x0F\x11\x45\x00\xF3\x0F\x10\x46\x00\xF3\x0F\x11\x45\x00\xF3\x0F\x10\x46\x00\xF3\x0F\x58\x05"), false, 0x2);

      // search for "CUserMessages::GetUserMessageSize( %i ) out of range!!!\n" and sig the function it is in.
      // or if that function doesn't exist it'll likely be apart of the class.
      add_signature(HASH("usermessages_getmsgname"), SIG("\x55\x8B\xEC\x8B\x45\x00\x8B\xD1\x85\xC0\x78\x00\x3B\x42\x00\x7D\x00\x8D\x0C\x80\x8B\x42"));

      // search for "CSecondaryTargetID"
      // xref that function to its vmt table and go down 3 and thats C_TFPlayer::IsPlayerOnSteamFriendsList
      add_signature(HASH("ctfplayer_playeronsteamfriends"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x85\xC0\x74\x00\x8B\x4D"), false);

      // ClientModeTFNormal::ShouldDrawViewModel
      // Can find this function by using "r_drawviewmodel" get the return address of the InCond function.
      add_signature(HASH("shoulddraw_viewmodel_retaddr1"), SIG("\x84\xC0\x74\x00\x32\xC0\xC3\xA1"), false);

      // Search for string "use_action_slot_item_server"
      add_signature(HASH("keyvalues_init"), SIG("\x55\x8B\xEC\xFF\x15\x00\x00\x00\x00\xFF\x75\x08\x8B\xC8\x8B\x10\xFF\x52\x04"), false);
      add_signature(HASH("keyvalues_setname"), SIG("\xFF\x15\x00\x00\x00\x00\x83\xC4\x08\x89\x06\x8B\xC6"), false);

      // Keyvalues:setint search for "achievementID"
      add_signature(HASH("keyvalues_setint"), SIG("\x55\x8B\xEC\x6A\x01\xFF\x75\x08\xE8\x00\x00\x00\x00\x85\xC0\x74\x0A\x8B\x4D\x0C\x89\x48\x0C\xC6\x40\x10\x02"), false);
      add_signature(HASH("keyvalues_setfloat"), SIG("\x55\x8B\xEC\x6A\x01\xFF\x75\x08\xE8\x00\x00\x00\x00\x85\xC0\x74\x0E\xF3\x0F\x10\x45\x00\xF3\x0F\x11\x40\x00\xC6\x40\x10\x03\x5D\xC2\x08\x00"), false);

      add_signature(HASH("get_client_voice_manager"), SIG("\xE8\x00\x00\x00\x00\x85\xC0\x74\x15\x57"), true);

      // search for "CVoiceStatus::SetPlayerBlockedState part 2" and that function should be the function to sig
      add_signature(HASH("set_player_blocked_state"), SIG("\x55\x8B\xEC\xA1\x00\x00\x00\x00\x81\xEC\x00\x00\x00\x00\x83\x78\x00\x00\x53\x56\x8B\x35"), false);

      // search for "#TF_ScoreBoard_Context_UnMute" and look for call to "GetClientVoiceMgr()->IsPlayerBlocked"
      add_signature(HASH("is_player_blocked"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x8D\x95\x00\x00\x00\x00\x56\x8B\xF1\x8B\x0D"), false);

      // Search for "mult_crit_when_health_is_below_percent"
      add_signature(HASH("is_crit_boosted"), SIG("\x55\x8B\xEC\x83\xEC\x00\x56\x8B\xF1\x57\x6A\x00\x8D\x8E"), false);

      // Search for "WeaponMedigun_Vaccinator.Charged_tier_0%d" it gets passed into EmitSound. Get the return address of that function.
      add_signature(HASH("medigun_find_and_heal_target_retaddr1"), SIG("\x83\xC0\x00\x83\xC4\x00\x89\x45\x00\x83\xFE"), false);

      // Found in CInput::Createmove
      add_signature(HASH("md5_pseudorandom"), SIG("\xE8\x00\x00\x00\x00\x83\xC4\x04\x25\x00\x00\x00\x00"), true);

      add_signature(HASH("prediction_random_seed"), SIG("\xA1\x00\x00\x00\x00\x89\x45\x00\x8D\x45\x00\x56"), false, 0x1);

      // search for "engy_disposable_sentries" in client.dll
      // look for something like "v15 = sub_10168350(v4, 3);" and that's the function
      add_signature(HASH("get_ammo_count"), SIG("\x55\x8B\xEC\x56\x8B\x75\x00\x57\x8B\xF9\x83\xFE\x00\x75\x00\x5F"), false);

      add_signature(HASH("is_steam_friend_get_player_info_retaddr1"), SIG("\x5E\x84\xC0\x74\x00\x83\x7D\x00\x00\x74\x00\xE8\x00\x00\x00\x00\x88\x45\x00\xA1\x00\x00\x00\x00\x8B\x55\x00\x81\xE2\x00\x00\x00\x00\x6A\x00\x8B\x48\x00\x81\xCA\x00\x00\x00\x00\x52\xFF\x75\x00\x8B\x01\x8B\x40\x00\xFF\xD0\x84\xC0\x74\x00\xB0"), false);

      // CClientScoreBoardDialog::UpdatePlayerAvatar can be found with "avatar" string which is the only string in the function.
      add_signature(HASH("update_player_avatar_get_player_info_retaddr2"), SIG("\x84\xC0\x0F\x84\x00\x00\x00\x00\x83\x7D\x00\x00\x0F\x84\x00\x00\x00\x00\x53\x57\xE8\x00\x00\x00\x00\x8B\x4D\x00\x8D\xBE\x00\x00\x00\x00\x0F\xB7\x77\x00\x88\x45\x00\xB8\x00\x00\x00\x00\x8B\x5D\x00\x81\xE3\x00\x00\x00\x00\x89\x4D\x00\x81\xCB\x00\x00\x00\x00\x89\x4D\x00\x89\x5D\x00\x89\x5D\x00\x89\x4D\x00\x89\x5D\x00\x66\x3B\xF0\x74\x00\x8D\x49\x00\x0F\xB7\xC6\x8D\x1C\x80\x8B\x47\x00\x83\xC0\x00\xC1\xE3\x00\x03\xC3\x50\x8D\x45\x00\x50\x8B\x07\xFF\xD0\x83\xC4\x00\x84\xC0\x74\x00\xB8\x00\x00\x00\x00\x66\x3B\xF0\x74\x00\x8B\x47\x00\x0F\xB7\x34\x03\xEB\x00\x8B\x47\x00\x8D\x4D\x00\x83\xC0\x00\x03\xC3\x51\x50\x8B\x07\xFF\xD0\x83\xC4\x00\x84\xC0\x74\x00\xB8\x00\x00\x00\x00\x66\x3B\xF0\x74\x00\x8B\x47\x00\x0F\xB7\x74\x18\x00\xB8\x00\x00\x00\x00\x66\x3B\xF0\x75\x00\xEB\x00\x8B\xF0\x8B\x5D\x00\x0F\xB7\xC6\x3D\x00\x00\x00\x00\x75\x00\x6A\x00\xE8\x00\x00\x00\x00\x83\xC4\x00\x85\xC0\x74\x00\x8B\xC8\xE8\x00\x00\x00\x00\x8B\xF0\xEB\x00\x33\xF6\x6A\x00\x53\xFF\x75\x00\x8B\xCE\xE8\x00\x00\x00\x00\x6A\x00\x6A\x00\x8B\xCE\xE8\x00\x00\x00\x00\x8B\x5D\x00\x56"), false);
      
      add_signature(HASH("set_abs_origin"), SIG("\xE8\x00\x00\x00\x00\xEB\x32\xE8\x00\x00\x00\x00"), true);
      add_signature(HASH("set_abs_angles"), SIG("\x55\x8B\xEC\x83\xEC\x00\x56\x57\x8B\x7D\x00\x8B\xF1\xE8"), false);
      add_signature(HASH("set_abs_velocity"), SIG("\x55\x8B\xEC\x83\xEC\x00\x56\x8B\x75\x00\x57\x8B\xF9\xF3\x0F\x10\x06"), false);
      
      // Search for "mult_bullets_per_shot"
      add_signature(HASH("attrib_hook_value_float"), SIG("\xE8\x00\x00\x00\x00\xDA\x4D\xFC"), true);

      // Search for "can_overload"
      add_signature(HASH("attrib_hook_value_int"), SIG("\xE8\x00\x00\x00\x00\xB9\x00\x00\x00\x00\x83\xC4\x00\x3B\xC1"), true);

      add_signature(HASH("look_weapon_info_slot"), SIG("\x55\x8B\xEC\xFF\x75\x00\xB9\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x5D\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x55\x8B\xEC\x53\x56\x57"), false);
      add_signature(HASH("file_weapon_info_from_handle"), SIG("\x55\x8B\xEC\x66\x8B\x45\x08\x66\x3B\x05\x00\x00\x00\x00\x73\x1A"), false);

      //Ignoring bogus angles (%f,%f,%f) from vphysics! (entity %s)\n
      add_signature(HASH("get_class_name"), SIG("\x53\x8B\xD9\xC6\x05"), false);

      // Search for "#TF_Voice_Unavailable" and above it should be
      //sub_1022A060(&dword_10C681B8, "CHudChat"); Sig the function CHudChat is passed into along with the dword_XXXXXX(which is gHud pointer)
      add_signature(HASH("get_hud_element_addr"), SIG("\xE8\x00\x00\x00\x00\xC3\x53"), true);

      add_signature(HASH("hud_element_addr"), SIG("\xB9\x00\x00\x00\x00\x56\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\xF0\x85\xF6\x74\x32\x8B\x0D\x00\x00\x00\x00\x8D\x45\x9C\x6A\x64\x50"), false);

      // Found by using the convar "tf_max_health_boost"
      add_signature(HASH("get_max_buffed_health_addr"), SIG("\x55\x8B\xEC\x51\x53\x8B\xD9\x57\x8B\x8B"), false);

      // Found by searching for "%c(%s) %s%c: %s" or "#Voice"
      // Get the return address of the call to ChatPrintf
      // ChatPrintf: (*(v19 + 76))(v12, v20, 0, "%c(%s) %s%c: %s", 3, v29, v21);
      add_signature(HASH("chatprintf_retaddr1"), SIG("\x83\xC4\x24\xC6\x83\x00\x00\x00\x00\x00\x5F\x5E\x5B"), false);

      // Search for "Flesh" you'll end up in a function called Impact. The function it's passed into is IsTruceActive
      add_signature(HASH("gamerules_addr"), SIG("\x8B\x0D\x00\x00\x00\x00\x85\xC9\x74\x1F\xE8\x00\x00\x00\x00\x84\xC0\x74\x16"), false, 0x2);

      // You can find the function GTFGCClientSystem by searching for the string "MatchmakingBanPanel"
      //  GTFGCClientSystem();
      //    v7 = sub_104E7E90(v5, &v41, &v47);
      //    LOBYTE(v49) = v7;
      //    result = (*(*v48 + 904))(v48, "MatchmakingBanPanel", v49, 0);
      add_signature(HASH("gc_clientsystem_addr"), SIG("\xE8\x00\x00\x00\x00\x8B\x17\x83\xC4\x00\x8A\xD8"), true);


      // Finding m_pSoCache & FindBaseTypecache search for "Failed to find party shared object\n" and you'll find a function with the pointer and the function.
      // if ( m_pSOCache
      //    && (pTypeCache = FindBaseTypeCache(m_pSOCache, 2003)) != 0
      //    && (GetCount = *(_DWORD *)(pTypeCache + 24)) != 0
      //    && (PartyClient = *(_DWORD **)(*(_DWORD *)(pTypeCache + 4) + 4 * GetCount - 4)) != 0 )
      add_signature(HASH("shared_object_cache_addr"), SIG("\x8B\x0D\x00\x00\x00\x00\x85\xC9\x74\x24\x68\x00\x00\x00\x00"), false);
      add_signature(HASH("find_base_type_cache_addr"), SIG("\x55\x8B\xEC\x83\xEC\x00\x8B\x45\x00\x53\x56\x57\x8B\xF9\x89\x45\x00\xBB"), false);

      // Has "CFlaggedEntitiesEnum::`vftable'" inside the function.
      add_signature(HASH("c_entity_sphere_query"), SIG("\x55\x8B\xEC\x83\xEC\x00\xD9\x45\x00\x8D\x55"), false);

      // Return address of CTempEnts::SpawnTempModel
      // String: effects/christmas_bulb.vmt
      // The function layout in IDA: ((*off_10BE2388)[13])(off_10BE2388, v3, a1, v7, &dword_10CCD978, 2.0, 0x400000);
      add_signature(HASH("create_holiday_light_retaddr"), SIG("\x8B\xF0\x85\xF6\x0F\x84\x00\x00\x00\x00\x33\xC0"), false);

      // search for "centerfire_projectile"
      add_signature(HASH("get_projectile_fire_setup"), SIG("\xE8\x00\x00\x00\x00\x5D\xC2\x20\x00"), true);

      add_signature(HASH("util_traceray"), SIG("\x55\x8B\xEC\x83\xEC\x00\x8D\x4D\x00\x56\xFF\x75\x00\xFF\x75"), false);

      // search for "worldNormalX" and follow the call where "ping_circle" is passed into a function, that is the function to hook
      add_signature(HASH("dispatch_particle_effect"), SIG("\xE8\x00\x00\x00\x00\x83\xC4\x20\x8D\x4D\xCC"), true);

      // Can be found in CTFMinigun::RingOfFireAttack
      add_signature(HASH("collision_property_calcnearestpoint"), SIG("\x55\x8B\xEC\x83\xEC\x00\x8D\x45\x00\x56\x50\xFF\x75\x00\x8B\xF1"), false);

      // Can be found inside the "CTFPlayer::CanDisguise" which can be found using the string "set_cannot_disguise"
      add_signature(HASH("ctfplayer_hastheflag"), SIG("\x55\x8B\xEC\x56\x8B\xF1\x57\x8B\x86\x00\x00\x00\x00\x85\xC0\x0F\x84"), false);

      // Search for UpdateClientSideAnimations in client.dll obviously...
      add_signature(HASH("update_clientside_animations"), SIG("\xE8\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x6A\x00\x68\x00\x00\x00\x00\x8B\x01\xFF\x50\x00\x0F\xB7\x3D"), true);

      // !!!! ONCE IN-GAME SET cl_particle_show_bbox to 1 !!!!

      // Go into CNewParticleEffect::DrawModel and search for DebugDrawBbox
      // Go inside of CNewParticleEffect::DebugDrawBbox
      // Where the (****) is at break point on the call and step into it.
      // Booom you got GetEffectName
      // v15 = (*(this2->dword0 + 44))(this2, this2->m_nActiveParticles); (****) <--

      //result = (*(v14 + 60))(debugoverlay, &center, 0, 0.0, r, g, b, 64, "%s:(%d)", v15);
      //Sig to the function that is inside this one \x8B\x49\x00\x85\xC9\x74\x00\x81\xC1\x00\x00\x00\x00\xE9
      /*
        .text:5256AE30 sub_5256AE30    proc near
        .text:5256AE30                 add     ecx, 16      <-- Make sure we use the correct offset inside cnewparticle_effect_render_hook
        .text:5256AE33                 jmp     sub_52A6B090 <--- SIG THIS FUNCTION
        .text:5256AE33 sub_5256AE30    endp
      */
      add_signature(HASH("cnewparticle_get_effect_name_addr"), SIG("\x8B\x49\x00\x85\xC9\x74\x00\x81\xC1\x00\x00\x00\x00\xE9"), false);

      // Found inside "CSniperDot::ClientThink"
      // Get the return address of this call SetControlPoint(*(this + 340), 2, clr);
      add_signature(HASH("csniperdot_clientthink_retaddr1"), SIG("\x8D\x45\x00\x8B\xCB\x50\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x84\xC0"), false);

      // Found looking for return addresses in UpdateCritBoostEffect
      add_signature(HASH("is_crit_boosted_material_retaddr1"), SIG("\x8B\xCF\x84\xC0\x74\x00\x6A\x00\xE8\x00\x00\x00\x00\x84\xC0"), false);

      // UpdateCritBoostEffect return from IsCritBoosted, search for "critgun_weaponmodel_red" and look for call to IsCritBoosted at the top
      // ! add_signature(HASH("update_crit_boost_effect_is_crit_boost_retaddr1"), SIG("\x84\xC0\x75\x00\x6A\x00\x8D\x8B\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x84\xC0\x75\x00\x8B\x83"), false);

      // The return address of this call in DisplayDamageFeedback
      // pLocalPlayer->IsPlayerClass( TF_CLASS_MEDIC )
      add_signature(HASH("displaydmgfeedback_isplayerclass_retaddr1"), SIG("\x84\xC0\x0F\x84\x00\x00\x00\x00\x8B\xCE\xE8\x00\x00\x00\x00\x3B\x43"), false);
      add_signature(HASH("displaydmgfeedback_medicgethealtarget_retaddr"), SIG("\x3B\x43\x00\x0F\x85\x00\x00\x00\x00\x8B\x4B"), false);

      add_signature(HASH("shared_random_int_addr"), SIG("\x55\x8B\xEC\x83\xEC\x00\x8B\x45\x00\x89\x45\x00\xA1"), false);

      // CNewParticleEffect::DrawModel call to CNewParticleEffect::Render return address
      add_signature(HASH("cnewparticle_effect_render_retaddr"), SIG("\x8B\x06\x8B\xCE\x6A\x00\xFF\x50\x00\x8B\x06\x8B\xCE\xFF\x50\x00\x8D\x7B"), false);

      // You can use tf_tournament_hide_domination_icons to find this function. (INLINED FROM RECENT UPDATE)
      // - add_signature(HASH("should_show_dueling_icon_addr"), SIG("\x57\x8B\xF9\x8B\x0D\x00\x00\x00\x00\x85\xC9\x74\x00\xE8\x00\x00\x00\x00\x84\xC0\x74\x00\xA1\x00\x00\x00\x00\x83\x78\x00\x00\x74\x00\x32\xC0\x5F\xC3"), false);

      add_signature(HASH("viewportpanel_interface_addr"), SIG("\x8B\x0D\x00\x00\x00\x00\x8D\x95\x00\x00\x00\x00\x85\xC0"), false);

      add_signature(HASH("setup_current_view_addr"), SIG("\x55\x8B\xEC\x83\xEC\x00\x8B\x0D\x00\x00\x00\x00\x53\x57\x33\xFF\x33\xDB\x89\x7D\x00\x89\x5D\x00\x8B\x01\x85\xC0\x74\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x53\x53\x53\x53\x8D\x4D\x00\x51\x50\x8B\x40\x00\xFF\xD0\x8B\x5D\x00\x83\xC4\x00\x8B\x7D\x00\x8B\x0D\x00\x00\x00\x00\x8B\x01\x8B\x4D"), false);

      // Search for "disguisename" this will land you in CDisguiseStatus::CheckName
      // X-ref and find the function that matches CDisguiseStatus::Paint get the retaddr from the InCond call
      add_signature(HASH("cdisguisestatus_paint_retaddr"), SIG("\x84\xC0\x74\x00\x83\xC6\x00\x8B\xCE"), false);

      // Search for "disguisename" this will land you in CDisguiseStatus::CheckName
      // X-ref and find the function that matches CDisguiseStatus::ShowAndUpdateStatus
      // X-ref again and you'll end up in CDisguiseStatus::ShouldDraw.
      // Sig the retaddr from the InCond call.
      add_signature(HASH("cdisguisestatus_shoulddraw_retaddr"), SIG("\x84\xC0\x75\x00\x32\xDB\x38\x9E"), false);

      // Use cl_hud_playerclass_use_playermodel
      // Inside of the function "CTFHudPlayerClass::OnThink" get the return addresses of the InCond calls.
      add_signature(HASH("ctfhudplayerclass_onthink_retaddr"), SIG("\x84\xC0\x74\x00\x6A\x00\x8D\x8E\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x84\xC0\x75\x00\x8D\x8E"), false);

      // Search for "#GameUI_HudPlayerClassUsePlayerModelDialogCancel"
      // Scroll down and look for a function like this
      //  v22 = InCond((v3 + 6392), 3);
      // Sig the return address.
      add_signature(HASH("ctfhudplayerclass_update_model_panel_retaddr"), SIG("\x88\x45\x00\x84\xC0\x74\x00\x8D\x8F"), false);

      // return address of the InCondition call inside of load_itempreset_callback
      add_signature(HASH("load_itempreset_incond_retaddr"), SIG("\x84\xC0\x0F\x85\x00\x00\x00\x00\x83\x3E"), false);

      // function at the top in TracerCallback
      add_signature(HASH("get_effect_data"), SIG("\xE8\x00\x00\x00\x00\x8B\xF8\x85\xFF\x0F\x84\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x83\x79\x00\x00\x0F\x84\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x56"), true);

      // The return address of the call InventoryManager()->UpdateInventoryEquippedState inside CTFPlayerInventory::VerifyLoadoutItemsAreValid
      add_signature(HASH("updateinvequippedstate_retaddr"), SIG("\xEB\x00\x0B\xD8"), false);
    
      // You can find this by searching for 'projectile_spread_angle'
      add_signature(HASH("get_spread_angles_addr"), SIG("\x55\x8B\xEC\x83\xEC\x00\x56\x57\x6A\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x6A\x00\x8B\xF9\xE8\x00\x00\x00\x00\x50\xE8\x00\x00\x00\x00\x8B\xF0\x83\xC4\x00\x85\xF6"), false);
    }

    set_module(HASH("engine.dll"));
    {
      add_signature(HASH("cl_runprediction"), SIG("\xE8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xD9\x05\x00\x00\x00\x00\xD9\x1C\x24"), true);

      //Search for "Recording to %s...\n" once in the function there will be text "demostart" that's the function.
      // Read the comment for cl_sendstringcommand it gets passed.

      // There are two ways to fetch a pointer to the client state
      // Method 1: Search for "Recording to %s...\n" and look for a call like "sub_10085B10(&dword_10463908, (int)"demorestart");"
      // Look in the assembly and look for mov ecx, 00000000 before that function call
      // Method 2: Search for "CDemo::Play: failed to create demo net channel\n" or "Playing demo from %s.\n" and look for a call like "v8 = sub_101A4920(0, 0, "DEMO", (int)&dword_10463908, 0, *((_DWORD *)Sourcea + 3));"
      // In those function parameters look for something like "(int)&dword_10463908"
      add_signature(HASH("client_state"), SIG("\xB9\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8D\x85\x00\x00\x00\x00\x50\x68"), false, 0x1);

      // The return address of Buf_AddtextWithMarkerers in BaseClientState::ProcessStringCmd
      add_signature(HASH("baseclientstate_processstringcmd_retaddr1"), SIG("\x83\xC4\x0C\xB0\x01\x5D\xC2\x04\x00"), false);

      // Search for "StartSound" once in the function look for the string soundname sig the function the string is being passed into.
      add_signature(HASH("keyvalues_setstring"), SIG("\x55\x8B\xEC\x53\x56\x8B\x75\x00\x6A"), false);

      // Search for "Software\\Valve\\Steam" and when inside function it is there.
      add_signature(HASH("steam3client_addr"), SIG("\xE8\x00\x00\x00\x00\x8B\x78\x08"), true);

      // Found in cl_sendmove.
      //add_signature(HASH("clc_move_rtti"), SIG("\xC7\x45\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8D\x4D\xE4\xE8\x00\x00\x00\x00\x6A\xFF\x6A\x00\x68\x00\x00\x00\x00\x8D\x85\x00\x00\x00\x00\xC6\x45\xB0\x00"), false);

      //
      add_signature(HASH("net_stringcmd_rtti"), SIG("\xE8\x00\x00\x00\x00\xEB\x02\xDD\xD8\x8B\x43\x24"), true, 0x36);

      // was found by logging __builtin_return_address(1) in studio_render_context_drawmodel_hook when a crash occours on cp_mercenarypark
      // this bug causes random crashes on all maps as we have tested and what ever this function does is rendering props that arent entities
      // this function seems to draw grass and similar types of special props
      add_signature(HASH("bad_rendercontext_drawmodel_retaddr1"), SIG("\x8B\xCF\xFF\x50\x4C\x5F"), false, 0x5);

      // Search for the string "Current alias commands:"
      add_signature(HASH("cmd_alias_list"), SIG("\x8B\x35\x00\x00\x00\x00\x83\xC4\x04\x85\xF6"), false);

      // The return address of the call to studio_render forced material override. Which is called inside CModelRender forced material override.
      add_signature(HASH("cmodelrender_forced_material_override_retaddr1"), SIG("\x85\xDB\x74\x00\x8B\x43\x00\x6A\x00\x6A\x00\x6A\x00\x57\x56\x53\xFF\xD0\x5F\x5E\x5B\x8B\xE5\x5D\xC2\x00\x00\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x55"), false);

      // bad call to DrawModelStaticProp
      //add_signature(HASH("draw_model_static_prop_retaddr1"), SIG("\x8B\x45\x00\x83\xC7\x00\x83\xE8"), false);
    }
#if defined(DEV_MODE)
    set_module(HASH("server.dll"));
    {
      add_signature(HASH("get_server_animating_addr"), SIG("\x55\x8B\xEC\x8B\x55\x00\x85\xD2\x7E\x00\xA1"), false);
      //add_signature(HASH("draw_server_hitbox_addr"), SIG("\xE8\x00\x00\x00\x00\x8B\x7D\xD4\x80\x7D\xF3\x00"), true);
    }
#endif

    return true;
  }

  bool write_pointers() override{
    if(!handle_globaldata())
      return false;

    //if(!patch_lobby())
    //  return false;

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
      // Found in write usercmd delta buffer have to step into it with a debugger.
      add_hook(HASH("cinput_get_user_cmd_hook"), SIG("\x55\x8B\xEC\x56\x57\x8B\x7D\x08\xB8\x00\x00\x00\x00\xF7\xEF\x8B\xF1"));

      // look for "WARNING! User command buffer overflow(%i %i), last cmd was %i bits long\n" and xref 2 up in vtable list
      add_hook(HASH("create_move_hook"), SIG("\xE8\x00\x00\x00\x00\x8B\x7D\xFC\x8B\xCF\x56"), true);

      add_hook(HASH("override_view_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\xE8\x00\x00\x00\x00\x85\xC0\x0F\x84\x00\x00\x00\x00\x8B\x10"), false);

      // https://github.com/TheAlePower/TeamFortress2/blob/1b81dded673d49adebf4d0958e52236ecc28a956/tf2_src/game/client/tf/vgui/tf_clientscoreboard.cpp#L1378
      // if ( !AreEnemyTeams( g_PR->GetTeam( playerIndex ), localteam ) && !( localteam == TEAM_UNASSIGNED ) )
      add_hook(HASH("player_resource_get_team_hook"), SIG("\x55\x8B\xEC\x8B\x55\x08\x8D\x42\xFF"), false);

      // https://github.com/TheAlePower/TeamFortress2/blob/1b81dded673d49adebf4d0958e52236ecc28a956/tf2_src/game/client/tf/vgui/tf_clientscoreboard.cpp#L1378
      add_hook(HASH("player_panel_get_team_hook"), SIG("\x8B\x91\x00\x00\x00\x00\x83\xFA\xFF\x74\x12"), false);

      // v135 = (*(*a2 + 20))(a2, "showdisguisedcrit", 0);
      // result = CTFPlayerShared::InCond(3);
      // Also this is the function we do the return address check on.
      add_hook(HASH("ctfplayershared_incondition_hook"), SIG("\x55\x8B\xEC\x83\xEC\x08\x56\x57\x8B\x7D\x08\x8B\xF1\x83\xFF\x20"), false);

      // search for "taunt_attr_player_invis_percent" and sig the function it is in!
      add_hook(HASH("ctfplayer_geteffectiveinvislevel_hook"), SIG("\xE8\x00\x00\x00\x00\xEB\xDE"), true);

      // sub_107CDDC0("DamagedPlayer", 1);
      // above that string is an if statement if ( C_TFPlayer::IsPlayerClass(8) )
      // Check "tf_hud_account.cpp"
      add_hook(HASH("ctfplayer_isplayerclass_hook"), SIG("\x55\x8B\xEC\x8D\x81\x00\x00\x00\x00\x85\xC0"), false);

      // search for "%8.4f : %30s : %5.3f : %4.2f  +\n" and sig the function is it in.
      add_hook(HASH("maintain_sequence_transition_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x56\x57\x8B\x7D\x00\x8B\xF1\x8B\xCF\x89\x75\x00\xE8\x00\x00\x00\x00\x85\xC0"), false);

      // search for "Blocking HTML info panel '%s'; Using plaintext instead.\n" and sig the function is it in
      add_hook(HASH("show_url_hook"), SIG("\x55\x8B\xEC\x56\x8B\xF1\xE8\x00\x00\x00\x00\x80\x7D\x0C\x00"), false);
      add_hook(HASH("is_info_panel_allowed_hook"), SIG("\x80\xB9\x00\x00\x00\x00\x00\x74\x0C\x80\xB9\x00\x00\x00\x00\x00\x74\x03\x32\xC0"), false);
      
	    add_hook(HASH("medigun_itempostframe_hook"), SIG("\x55\x8B\xEC\x51\x56\x8B\xF1\x8B\x8E\x00\x00\x00\x00\x85\xC9\x0F\x84\x00\x00\x00\x00\xBA\x00\x00\x00\x00\x83\xF9\x00\x74\x00\x0F\xB7\xD1\x57\x8B\x3D\x00\x00\x00\x00\xC1\xE2\x00\x83\xC7\x00\x03\xFA\x0F\x84\x00\x00\x00\x00\xC1\xE9\x00\x39\x4F\x00\x0F\x85\x00\x00\x00\x00\x8B\x3F\x85\xFF\x0F\x84\x00\x00\x00\x00\x8B\x07\x8B\xCF\x8B\x80\x00\x00\x00\x00\xFF\xD0\x84\xC0\x0F\x84\x00\x00\x00\x00\x8B\x06"), false);

      // search for "cl_updaterate" it will be the only string in the function it'll have an "HushAssert" in the function also.
      add_hook(HASH("client_interp_amount_hook"), SIG("\xE8\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\xD9\x5D\xF4"), true);

      // https://github.com/TheAlePower/TeamFortress2/blob/1b81dded673d49adebf4d0958e52236ecc28a956/tf2_src/game/client/tf/c_tf_player.cpp#L6734
      add_hook(HASH("ctfplayer_avoidplayers_hook"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x57\x8B\xF9\x89\x7D\x00\x83\x78\x00\x00\x0F\x84\x00\x00\x00\x00\xA1"), false);

      // search for "ghost_wearable" and there will be 2 functions the correct one will be with in an if statement if ( CTFPlayerShared::InCond(77) )
      // DO NOT HOOK THE FUNCTION THAT HAS "(float)(*(float *)(v5 + 7252) + 2.0) > *((float *)off_10BD4990 + 3))" in it!!!
      add_hook(HASH("ctfwearable_shoulddraw_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x56\x57\x8B\xF9\x8B\x97"), false);

      // CUtlString::Format(&v9, "connect %s matchmaking");
      // if ( engine )
      // {
      //   v3 = GetMatchGroupDescription(this + 1396);
      //   if ( v3 && *(v3 + 71) )
      //     ForceCompetitiveConvars(a1);
      // THEY INLINED THE FUNCTION!
      // - add_hook(HASH("force_competitive_convars_hook"), SIG("\x8B\x0D\x00\x00\x00\x00\x53\x56\x32\xDB\x8B\x01"), false);

      add_hook(HASH("run_command_hook"), SIG("\x55\x8B\xEC\x51\x53\x56\x8B\xF1\x57\x89\x75\xFC\xE8\x00\x00\x00\x00\x8B\x7D\x08"), false);

      add_hook(HASH("post_think_hook"), SIG("\x56\x8B\x35\x00\x00\x00\x00\x57\x8B\xF9\x8B\xCE\x8B\x06\xFF\x50\x00\x8B\x07"), false);

      // found in this function https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/game/client/c_baseanimating.cpp#L1814
      add_hook(HASH("check_for_sequence_change_hook"), SIG("\x55\x8B\xEC\x53\x8B\x5D\x08\x57\x8B\xF9\x85\xDB\x0F\x84\x00\x00\x00\x00"), false);

      // search for "ParticleTracer" and sig the function it is in.
      add_hook(HASH("util_particle_tracer_hook"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x56\xFF\x75\x00\xE8\x00\x00\x00\x00\x83\xC4"), false);

      // search for "%s_crit" and sig the function it is in.
      add_hook(HASH("ctfplayer_firebullet_hook"), SIG("\x53\x8B\xDC\x83\xEC\x00\x83\xE4\x00\x83\xC4\x00\x55\x8B\x6B\x00\x89\x6C\x24\x00\x8B\xEC\x81\xEC\x00\x00\x00\x00\x56\x57\x8B\xF9\x8B\x4B"), false);

      // search for "CUserMessages::DispatchUserMessage" and sig the function it is in.
      add_hook(HASH("usermessages_dispatch_user_message_hook"), SIG("\x55\x8B\xEC\x8B\x55\x08\x83\xEC\x18\x56"), false);

      // search for "water" or "wade" or "ladder" and that is the function to hook
      add_hook(HASH("update_step_sound_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x0F\x57\xD2\x57"), false);
	  
      // stepped into this function by placing a break point in C_TFPlayer::FireBullets and stepping into GetTracerType()
      add_hook(HASH("get_tracer_type_hook"), SIG("\x56\x8B\xF1\x8B\x06\xFF\x90\x00\x00\x00\x00\x8B\xCE\x85\xC0\x74\x10"), false);

      // search for "sniper_fires_tracer" and sig the function it is in. (INLINED)
      // - add_hook(HASH("ctfplayer_maybedrawrailbeam_hook"), SIG("\x55\x8B\xEC\x56\x8B\x75\x00\x57\x6A\x00\x6A\x00\x56\x68"), false);

      // look at CTFPlayerShared::OnAddBurning can be found by searching for "Fire.Engulf"
      // Step into view->SetScreenOverlayMaterial( pMaterial ); using a debugger.
      add_hook(HASH("cviewrender_setscreenoverlaymaterial_hook"), SIG("\x55\x8B\xEC\x81\xC1\x00\x00\x00\x00\x5D\xE9\x00\x00\x00\x00\xCC\x55\x8B\xEC\x51\x56"), false);

      // search for "tf_playergib_maxspeed" the convar.
      add_hook(HASH("ctfplayer_createplayergibs_hook"), SIG("\x55\x8B\xEC\x83\xEC\x50\x57\x8B\xF9\x83\xBF\x00\x00\x00\x00\x00\x0F\x84\x00\x00\x00\x00\xD9\x05\x00\x00\x00\x00"), false);

      // search for C_BaseAnimating::SetupBones and sig the function it is in.
      add_hook(HASH("base_animating_setupbones_hook"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x53\x33\xD2\x8B\xD9\x8B\x0D\x00\x00\x00\x00\x89\x55\x00\x89\x55\x00\x57"), false);

      // Found by printing proxy address for m_bDormant and using cheat engine to find what writes to it
      add_hook(HASH("set_dormant_hook"), SIG("\x55\x8B\xEC\x53\x8B\x5D\x00\x56\x8B\xF1\x8B\x06\x88\x9E"));

      // Might be it. Pretty sure I fixed it, totally.
      add_hook(HASH("get_outer_abs_velocity_hook"), SIG("\xE8\x00\x00\x00\x00\xF3\x0F\x10\x9F\x00\x00\x00\x00\x0F\x57\xC0"), true);

      // Had to break point in ChatPrintf and step into the function.
      add_hook(HASH("hud_chatline_insert_and_colorize_text_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x8B\xD9\x56\x57\x89\x5D\x00\x8B\x83\x00\x00\x00\x00\x85\xC0"));

      // Use the game's code to find it.
      add_hook(HASH("input_cam_is_thirdperson_hook"), SIG("\x0F\xB6\x81\x00\x00\x00\x00\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x55\x8B\xEC\xA1"), false);

      // Search for "minigun_fire" and the function should be right below.
      add_hook(HASH("calc_is_attack_critical_hook"), SIG("\x53\x57\x6A\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x6A\x00\x8B\xF9\xE8\x00\x00\x00\x00\x50\xE8\x00\x00\x00\x00\x8B\xD8\x83\xC4\x00\x85\xDB\x0F\x84"), false);

      // search for "minigun_no_spin_sounds"
      add_hook(HASH("minigun_weapon_sound_update_hook"), SIG("\xE8\x00\x00\x00\x00\x83\xBF\x00\x00\x00\x00\x00\x8B\x87\x00\x00\x00\x00"), true);

      // Search for the string "WeaponMedigun_Vaccinator.Toggle" and sig the functions the string gets passed in.
      add_hook(HASH("emit_sound_hook"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x8B\x15\x00\x00\x00\x00\x53\x56\x57\x33\xFF\x33\xDB"));

      // Search for the string "models/props_halloween/bumpercar_cage.mdl" and goto the function you're in.
      // Once inside look for the FollowEntity function. Sig that function.
      add_hook(HASH("cbaseentity_followentity_hook"), SIG("\x55\x8B\xEC\x8B\x45\x08\x56\x8B\xF1\x85\xC0\x74\x50"));

      // Search for "Achievements disabled: cheats turned on in this app session." and sig the function it is in.
      add_hook(HASH("achievementmgr_checkachievementenabled_hook"), SIG("\x55\x8B\xEC\xA1\x00\x00\x00\x00\x83\xEC\x00\x56\x8B\xF1\x8B\x48"), false);

      // Search for "FX_FireBullets: weapon alias for ID %i not found\n" and sig the function it is in.
      add_hook(HASH("fx_firebullets_hook"), SIG("\x53\x8B\xDC\x83\xEC\x00\x83\xE4\x00\x83\xC4\x00\x55\x8B\x6B\x00\x89\x6C\x24\x00\x8B\xEC\x81\xEC\x00\x00\x00\x00\x56\x57\x8B\x7B\x00\x57"), false);

      // search for "Bad field type\n" and look for switch statement execution 5:
      //  v23 = sub_1027CD00(v12, v13, v11);
      //  sub_1027D5E0((_BYTE *)this, v23, (int)v12, (int)v13, v26); <- HOOK THIS FUNCTION
      // compare to CPredictionCopy::CopyFields
      // INLINED

      // \xE8\x00\x00\x00\x00\x83\xC4\x00\x80\x7F\x00\x00\x74\x00\x80\x7F
      // SOLUTION: COPYINT JUST DOES A MEM MOVE, LETS HOOK MEM MOVE, CHECK RETURN ADDRESS AND JUST STOP IT WRITING THERE INSTEAD
      //add_hook(HASH("prediction_copy_int_hook"), SIG("\x55\x8B\xEC\x80\x79\x2C\x00\x74\x1B\x83\x7D\x08\x01\x74\x15\x8B\x45\x14\xC1\xE0\x02"), false);

      add_hook(HASH("can_fire_random_critical_shot_hook"), SIG("\x55\x8B\xEC\xF3\x0F\x10\x4D\x00\xF3\x0F\x58\x0D\x00\x00\x00\x00"), false);

      add_hook(HASH("is_allowed_to_withdraw_from_crit_bucket_hook"), SIG("\x55\x8B\xEC\x56\x8B\xF1\x0F\xB7\x86"), false);

      add_hook(HASH("add_to_crit_bucket_hook"), SIG("\x55\x8B\xEC\xA1\x00\x00\x00\x00\xF3\x0F\x10\x81\x00\x00\x00\x00\xF3\x0F\x10\x48"), false);

      // Search for "Saxxy.TurnGold" and sig the function it is in.
      add_hook(HASH("ctfragdoll_create_ragdoll_hook"), SIG("\x55\x8B\xEC\xB8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x53\x56\x57\x8B\xF9\x89\x7D"), false);

       // Search for "C_BaseAnimating::UpdateClientSideAnimations" and step into C_BaseAnimating::UpdateClientSideAnimation using a debugger.
      add_hook(HASH("cbaseanimating_update_client_side_animation_hook"), SIG("\x56\x8B\xF1\x80\xBE\x00\x00\x00\x00\x00\x74\x27"), false);

      // Search for "tf_viewmodels_offset_override" and sig the function it is in.
      add_hook(HASH("ctfviewmodel_calcviewmodelview_hook"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x8B\x55\x00\x53\x8B\x5D\x00\x89\x4D"), false);

      // Search for "EntityGlowEffects" and sig the function it is in.
      add_hook(HASH("glowobjectmanager_rendergloweffects_hook"), SIG("\x55\x8B\xEC\x83\xEC\x14\x57\x8B\xF9\x8B\x0D\x00\x00\x00\x00"), false);

      // Search for "Chat.DisplayText" and sig the function it is in.
      add_hook(HASH("menu_lobbychat_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x56\x8B\x75\x00\x57\x8B\xF9\x8B\xCE\x8B\x06\xFF\x50\x00\x68"), false);

      // search for "Writing demo message %i bytes at file pos %i\n" in engine.dll, look for GetClientCmdInfo and then follow with debugger until you get to g_pClientSidePrediction->GetLocalViewAngles
      add_hook(HASH("get_local_view_angles_hook"), SIG("\x55\x8B\xEC\xE8\x00\x00\x00\x00\x8B\x4D"), false);

      // search for "mat_motion_blur_enabled" in client.dll and follow the address from call "(*(void (__thiscall **)(int, _DWORD *))(*(_DWORD *)v24 + 156))(v24, a2)"
      add_hook(HASH("do_post_screen_space_effects_hook"), SIG("\x55\x8B\xEC\xFF\x75\x08\xE8\x00\x00\x00\x00\x84\xC0"), false);

      // uhhhh
      add_hook(HASH("addviewmodelbobhelper_hook"), SIG("\x55\x8B\xEC\x83\xEC\x18\x57\x8B\x7D\x10"), false);

      // search for "CSecondaryTargetID"
      // xref that function to its vmt table and go down 3 and thats C_TFPlayer::IsPlayerOnSteamFriendsList
      add_hook(HASH("ctfplayer_playeronsteamfriends_hook"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x85\xC0\x74\x00\x8B\x4D"), false);

      // Can be found by using the cvar "tf_weapon_criticals_bucket_default" it's the cbasecombatweapon constructor.
      add_hook(HASH("cbase_combat_weapon_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x56\x8B\xF1\xE8\x00\x00\x00\x00\xC7\x06\x00\x00\x00\x00\x8D\x4D\x00\x33\xC0"), false);

      // CTFPlayerShared::OnAddedCondition
      add_hook(HASH("ctfplayershared_onconditionadded_hook"), SIG("\xE8\x00\x00\x00\x00\xEB\x0C\x85\xF2"), true);

      //  Called in CTFPlayerShared::RemoveCond you can find this function by looking for the string "damage_prevented"
      add_hook(HASH("ctfplayershared_onconditionremoved_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x56\x8B\x75\x00\x8B\xD9\x56\xE8\x00\x00\x00\x00\x84\xC0"), false);

      // Search for:
      // WARNING: Found item in a duplicate backpack position. Moving to the backpack end.
      // You'll be in CTFPlayerInventory::ValidateInventoryPositions
      // Search for GetMaxItemCount
      // At the top of the function: maxitems = (*(*v1 + 36))(v1); break point into the call.
      // Find the location of it in x86 debugger and place a breakpoint.
      // Close TF2 and reopen the game and quickly reattach the debugger.
      // Should get a breakpoint when the main menu starts to appear. Step into the function and boom!
      add_hook(HASH("ctfplayerinventory_getmaxitems_hook"), SIG("\x8B\x49\x68\x56\xBE\x00\x00\x00\x00\x85\xC9\x74\x2E\x6A\x07"), false);

      // Hooks CTFPlayerAnimState::ComputePoseParam_AimYaw
      // Can be found by using "host_timescale"
      add_hook(HASH("ctfplayeranimstate_aimyaw_hook"), SIG("\x55\x8B\xEC\x51\x56\x8B\xF1\x8B\x0D\x00\x00\x00\x00\x57\x8B\x01\x8B\x80"), false);

      // Found by searching for Can't create temporary entity with NULL model!\n
      add_hook(HASH("tempents_tempentalloc_hook"), SIG("\xE8\x00\x00\x00\x00\x8B\xF0\x85\xF6\x74\xE3"), true);

      // You can use the cl_crosshair_red convar which can get you in CHudCrosshair::Paint which contains this function
      // check hud_crosshair.cpp for ref if needed.
      add_hook(HASH("chudcrosshair_getdrawposition_hook"), SIG("\xE8\x00\x00\x00\x00\x83\xC4\x18\x80\x7D\xFF\x00\x0F\x85\x00\x00\x00\x00\x8B\x87\x5C\x01\x00\x00"), true);

      // Inside CNewParticleEffect::DrawModel
      add_hook(HASH("cnewparticle_effect_render_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x57\x8B\xF9\x8B\x4F\x00\x85\xC9\x0F\x84\x00\x00\x00\x00\xF6\x81"), false);

      // Search for "laser_sight_beam" the correct function has another string called "eye_1"
      add_hook(HASH("csniperdot_clientthink_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x56\x8B\xF1\x8B\x0D\x00\x00\x00\x00\x57\xE8"), false);

      // Search for "ParticleSystem_SetControlPointPosition" and sig the function it is in.
      add_hook(HASH("particlesystem_setcontrolpoint_hook"), SIG("\x55\x8B\xEC\x53\x8B\x5D\x00\x57\x8B\xF9\x83\xBF"), false);

      // Search for "mult_crit_when_health_is_below_percent" and sig the function it is in.
      //add_hook(HASH("is_crit_boosted_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x56\x8B\xF1\x6A\x00\x8D\x8E"), false);

      // Use the convar "fog_enable" to find the function.
      add_hook(HASH("get_fog_enable_hook"), SIG("\x55\x8B\xEC\x8B\x0D\x00\x00\x00\x00\x83\xEC\x00\x8B\x01\x56\xFF\x90\x00\x00\x00\x00\x8B\xF0\x85\xF6\x74\x00\x8B\x06\x8B\xCE\xFF\x50\x00\x57"), false);

      // Use the convar "r_farz" to find the function.
      add_hook(HASH("cviewrender_getzfar_hook"), SIG("\x55\x8B\xEC\x51\xA1\x00\x00\x00\x00\xF3\x0F\x10\x05\x00\x00\x00\x00\xF3\x0F\x10\x48"), false);

      // Search for "$underwateroverlay"
      /*
        overlay_name = (*(*v19 + 24))(v19);
        if ( *overlay_name != '0' )
        {
          overlay_material = (*(*dword_CF9CE8 + 292))(dword_CF9CE8, overlay_name, "Other textures", 1, 0);
          SetWaterOverlayMaterial((*(this + 212) + 860), overlay_material);
        }
      */
      add_hook(HASH("cviewrender_setwateroverlaymaterial_hook"), SIG("\x55\x8B\xEC\x56\x8B\x75\x00\x57\x8B\xF9\x8B\x0F\x3B\xCE\x74"), false);

      // Use the convar "mat_autoexposure_min / mat_autoexposure_max"
      add_hook(HASH("get_exposure_range_hook"), SIG("\x55\x8B\xEC\x80\x3D\x00\x00\x00\x00\x00\x0F\x57\xC0"), false);

      // use the convar mat_bloomscale
      //add_hook(HASH("get_bloom_amount_hook"), SIG("\x55\x8B\xEC\x51\x8B\x0D\x00\x00\x00\x00\x8B\x01\xFF\x90\x00\x00\x00\x00\x83\xF8"), false);

      // You can find this function inside DisplayDamageFeedback
      add_hook(HASH("medic_gethealtarget_hook"), SIG("\x56\x6A\x00\x8B\xF1\xE8\x00\x00\x00\x00\x84\xC0\x74\x00\x8B\x06"), false);

      //Use the "CClientEntityList::`vftable'" to find the functions
      //First function is OnAddEntity and the second function is OnRemoveEntit
      add_hook(HASH("client_entity_list_on_add_entity_hook"), SIG("\x55\x8B\xEC\x8B\x45\x00\x83\xEC\x00\x53\x56\x8B\xF1\x57"), false);
      add_hook(HASH("client_entity_list_on_remove_entity_hook"), SIG("\x55\x8B\xEC\x51\x8B\x45\x00\x53\x8B\xD9\x56\x57\x83\xF8"), false);

      // Search for [PartyClient] Sending request to cancel outgoing invite/incoming join request
      add_hook(HASH("ctfpartyclient_cancel_out_going_request_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x56\x8B\xF1\x8D\x4D\x00\x57\x89\x75\x00\xFF\x76"), false);

      // Search for "[PartyClientDbg] Sending PartyKickMember"
      add_hook(HASH("ctfpartyclient_kick_party_member_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x8B\xD9\x8B\x4B\x00\x85\xC9\x0F\x84\x00\x00\x00\x00\x80\x7B\x00\x00\x0F\x84\x00\x00\x00\x00\x8B\x81\x00\x00\x00\x00\x8D\x55\x00\x81\xC1\x00\x00\x00\x00\x52\xFF\x50\x00\x83\xF8\x00\x0F\x84\x00\x00\x00\x00\x68\x00\x00\x00\x00\x8D\x4D\x00\xE8\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x33\xD2\xC7\x45\x00\x00\x00\x00\x00\xC7\x45\x00\x00\x00\x00\x00\x89\x55\x00\x89\x55\x00\x8B\x41\x00\x89\x55\x00\x89\x55\x00\x85\xC0\x74\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x52\x52\x52\x52\x8D\x4D\x00\x51\x50\x8B\x40\x00\xFF\xD0\x8B\x45\x00\x83\xC4\x00\x8B\x0D\x00\x00\x00\x00\x89\x45\x00\x8B\x45\x00\x89\x45\x00\x56\x8B\x71\x00\x8B\x0D\x00\x00\x00\x00\x57\x89\x75\x00\x8B\xB9\x00\x00\x00\x00\x89\x7D\x00\x85\xFF\x74\x00\x6A\x00\x6A\x00\x68\x00\x00\x00\x00\x6A\x00\x68\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x80\x3D\x00\x00\x00\x00\x00\x0F\x85\x00\x00\x00\x00\xB9\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x80\x3D\x00\x00\x00\x00\x00\x75\x00\x6A\x00\xE8\x00\x00\x00\x00\x8B\xF8\x83\xC4\x00\x85\xFF\x74\x00\xE8\x00\x00\x00\x00\xFF\x35\x00\x00\x00\x00\xFF\x35\x00\x00\x00\x00\x8B\x48\x00\x83\xC1\x00\xE8\x00\x00\x00\x00\x8B\xF0\xE8\x00\x00\x00\x00\x56\xFF\x35\x00\x00\x00\x00\x8B\x48\x00\xFF\x35\x00\x00\x00\x00\x83\xC1\x00\xE8\x00\x00\x00\x00\x50\x8B\xCF\xE8\x00\x00\x00\x00\x8B\x75\x00\xC7\x07\x00\x00\x00\x00\xEB\x00\x33\xFF\x57\x89\x3D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\xC8\xE8\x00\x00\x00\x00\x8B\x7D\x00\xC6\x05\x00\x00\x00\x00\x00\x68\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x89\x45\x00\x85\xFF\x74\x00\x8B\x0D\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x85\xF6\x74\x00\x8B\x46\x00\x6A\x00\x6A\x00\x6A\x00\xFF\x75\x00\xFF\x75\x00\x56\xFF\xD0\x8B\x45\x00\x8B\x4B\x00\x8B\x53\x00\x5F\x83\x48\x00\x00\x89\x48\x00\x89\x50\x00\x8B\x45\x00\x8B\x4D\x00\x8B\x55\x00\x5E\x83\x48\x00\x00\x89\x48\x00\x89\x50\x00\xA1\x00\x00\x00\x00\x83\x78\x00\x00\x74\x00\x8D\x4D\x00\xE8\x00\x00\x00\x00\x50\x68\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x83\xC4\x00\x8D\x45\x00\x50\xE8\x00\x00\x00\x00\x8B\xC8\xE8\x00\x00\x00\x00\x8B\x45\x00\xC7\x45\x00\x00\x00\x00\x00\x85\xC0\x74\x00\x8B\x0D\x00\x00\x00\x00\x50\xE8\x00\x00\x00\x00\xC7\x45\x00\x00\x00\x00\x00\x8D\x4D\x00\xE8\x00\x00\x00\x00\xB0\x00\x5B\x8B\xE5\x5D\xC2\x00\x00\x32\xC0\x5B\x8B\xE5\x5D\xC2\x00\x00\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x55\x8B\xEC\x83\xEC\x00\x53\x8B\x5D"), false);

      add_hook(HASH("crenderview_renderview_hook"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x53\x8B\xD9\x56\x57\x89\x5D\x00\x8D\x8B"), false);

      // called inside of the load_itempreset_callback function
      add_hook(HASH("inventorymanager_loadpreset_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x56\x8B\xF1\xE8\x00\x00\x00\x00\x66\x8B\x5D"), false);

      // Use r_drawtracers or r_drawtracers_firstperson to find this function.
      add_hook(HASH("tracer_callback_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x57\xE8\x00\x00\x00\x00\x8B\xF8\x85\xFF\x0F\x84\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x83\x79"), false);

      // Search for "muzzle_sentry2" or "muzzle_sentry" in client.dll to find this function.
      add_hook(HASH("sentry_muzzle_flash_hook"), SIG("\x55\x8B\xEC\x8B\x4D\x00\x53\x56\x57\x8B\x59"), false);

      // Find using the convar 'cl_wpn_sway_scale'.
      add_hook(HASH("ctfviewmodel_calcviewmodellag_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x56\x8B\xF1\x8B\x0D\x00\x00\x00\x00\x8B\x01\x8B\x40\x00\xFF\xD0\x84\xC0\x0F\x85"), false);

      // Search for 'Weapon_CowMangler.ReloadFinal' and go into the function it's in.
      /*
        if ( a2 == 6 && (v5 = *(this + 769), v3 = (*(*this + 1740))(this) + v5, v6 = v3, sub_2B126D50(this), v6 == v3) )
          result = "Weapon_CowMangler.ReloadFinal";
        else
          result = CTFWeaponBase::GetShootSound(this, a2); <-- This is the function that needs to be sigged.
      */
      add_hook(HASH("ctfweaponbase_getshootsound_hook"), SIG("\x55\x8B\xEC\x51\x53\x8B\xD9\x80\xBB\x00\x00\x00\x00\x00\x0F\x84\x00\x00\x00\x00\x8B\x03"), false);

      // Find using "models/effects/resist_shield/resist_shield.mdl"
      add_hook(HASH("add_resist_shield_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\xE8\x00\x00\x00\x00\x8B\x5D"), false);

      // This stuff was aids to find.
      add_hook(HASH("generate_equip_region_conflict_mask_hook"), SIG("\x55\x8B\xEC\x53\x8B\x5D\x00\x56\x57\x33\xFF\x33\xF6"), false);

      // this has no docs, nothing and is a nightmare to get right now for such a shitty feature. so fuck it.
      //add_hook(HASH("equippable_items_for_slotgenerator_constructor_hook"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x57\x8B\xF9\xC7\x07"), false);

      //
      add_hook(HASH("inventory_manager_updateinv_equipped_state_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x8B\x5D\x00\x8B\xC3\x56\x57"), false);
    
      // No idea what this function is called. But look for this in client.dll
      /*
        if ( !IsSteamFriend3(this, a2, a3) )
        {
          v5 = sub_297530(&a2);
          Warning("[PartyClient] Attempting to invite user %s to party who is not in an invitable state\n", v5);
          return 0;
        }
      */

      add_hook(HASH("tflobby_is_steam_friend3_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x56\x8B\xF1\x57\x8B\x4E\x00\x85\xC9"), false);

      // No idea what this function is called. As usual look at the ref below.
      /*
        if ( IsSteamFriend(v4, v10, v12) )
        {
          v11 = v14;
          v9 = v13;
          sub_4EB2F0();
          if ( sub_4F6250(v9, v11, v15) )
          {
            v7 = sub_297530(&v13);
            ConMsg("Requested to join party of player %s\n", v7);
          }
          else
          {
            v6 = sub_297530(&v13);
            ConWarning("Failed to request to join party of player %s\n", v6);
          }
        }
      */
      add_hook(HASH("tflobby_is_steam_friend_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x8B\x55\x00\x53\x8B\xD9\x89\x55"), false);

      //add_hook(HASH("ctfweaponbasegun_firenail_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x56\x57\x8B\xF9\x8B\x07\xFF\x90\x00\x00\x00\x00\x8B"), false);
      
      // TOSS_JAR_THINK
      add_hook(HASH("set_context_think_hook"), SIG("\x55\x8B\xEC\x53\x56\x8B\x75\x00\x8B\xD9\x85\xF6\x75\x00\x8B\x45"), false);

      //add_hook(HASH("econ_texture_cache_poll_hook"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x56\x8B\xF1\x57\x8B\x7E"), false);
    }

    set_module(HASH("vgui2.dll"));
    {
      add_hook(HASH("paint_traverse_hook"), SIG("\x55\x8B\xEC\x8B\x01\xFF\x75\x08\xFF\x90\x00\x00\x00\x00\xFF\x75\x10\x8B\xC8\xFF\x75\x0C\x8B\x10\xFF\x52\x0C"), false);
    }

    set_module(HASH("engine.dll"));
    {
      // search for "CEngineVGui::Paint" and sig the function it is in
      add_hook(HASH("engine_paint_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x33\xD2\x8B\xD9\x8B\x0D\x00\x00\x00\x00\x89\x55\x00\x89\x55\x00\x56\x8B\x41\x00\x89\x55\x00\x89\x55\x00\x85\xC0\x74\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x52\x52\x52\x52\x8D\x4D\x00\x51\x50\x8B\x40\x00\xFF\xD0\x8B\x45\x00\x83\xC4\x00\x8B\x0D\x00\x00\x00\x00\x89\x45\x00\x8B\x45\x00\x89\x45\x00\x8B\x71\x00\x8B\x0D\x00\x00\x00\x00\x89\x75\x00\x8B\x81\x00\x00\x00\x00\x89\x45\x00\x85\xC0\x74\x00\x6A\x00\x6A\x00\x68\x00\x00\x00\x00\x6A\x00\x68\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x83\x7B"), false);

      // search for "PureServerWhitelist" and sig the function is is in!
      add_hook(HASH("check_for_pure_server_whitelist_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x83\x3D\x00\x00\x00\x00\x00\x0F\x8E"), false);

      // Use the string "DPT_" to find these functions
      // x-ref the string that has the DPT_ string in it.
      // .data:functions usually 2 above the DPT_ function.
      add_hook(HASH("int_decode_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x56\x57\x8B\x7D\x00\x8B\x77\x00\x89\x75\x00\x8B\x46"), false);
      add_hook(HASH("float_decode_hook"), SIG("\x55\x8B\xEC\x56\x8B\x75\x00\xFF\x76\x00\xFF\x76\x00\xE8\x00\x00\x00\x00\x8B\x06"), false);

      // not being used, but will be kept.
      //add_hook(HASH("vectorxy_decode_hook"), SIG("\x55\x8B\xEC\x56\x8B\x75\x08\xFF\x76\x28\xFF\x76\x24\xE8\x00\x00\x00\x00\xD9\x5E\x04\xFF\x76\x28\xFF\x76\x24\xE8\x00\x00\x00\x00\xD9\x5E\x08"), false);

      // search for "cl_interp_ratio" and you'll get into the function. You should also see "cl_interp" below.
      add_hook(HASH("cl_client_interp_amount_hook"), SIG("\x55\x8B\xEC\x8B\x0D\x00\x00\x00\x00\x83\xEC\x00\x85\xC9\x75\x00\x8B\x0D"), false);

      // Search for "FireEvent: event '%s' not registered.\n" or "Game event \"%s\", Tick %i:\n"
      // Xref the function and look for "return sub_170B90(this, a1, 0, 1);" and thats the function to sig (not the return function)
      add_hook(HASH("fire_event_hook"), SIG("\x55\x8B\xEC\x6A\x01\x6A\x00\xFF\x75\x08"), false);

      // search for "WARNING:  Connection Problem" and that is the function to hook
      add_hook(HASH("cl_move_hook"), SIG("\x55\x8B\xEC\xB8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\x3D\x00\x00\x00\x00\x00\x0F\x8C"), false);

      // search for "UDP <- %s: sz=%i seq=%i ack=%i rel=%i ch=%d, tm=%f rt=%f wire=%i\n" and thats the function to hook
      add_hook(HASH("process_packet_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x56\x8B\x75\x00\x83\x7E"), false);

      // search for "%s:send reliable stream overflow\n" or "CNetChan_TransmitBits->send" or "datagram" and thats the function to hook
      add_hook(HASH("send_datagram_hook"), SIG("\x55\x8B\xEC\xB8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x53\x56\x8B\xD9"), false);

      // search for the text "Cbuf_AddTextWithMarkers: execution marker overflow\n" and sig the function you're in.
      add_hook(HASH("buf_addtext_withmarkers_hook"), SIG("\x55\x8B\xEC\xA1\x00\x00\x00\x00\x81\xEC\x00\x00\x00\x00\x83\xC0\x02"), false);

      // finding this was kinda aids.
      add_hook(HASH("engine_get_player_info_hook"), SIG("\x55\x8B\xEC\x51\x56\x8B\x75\x08\x4E"), false);

      // Search for "NetMsg" and sig the function it is in.
      add_hook(HASH("cnetchan_sendnetmsg_hook"), SIG("\x55\x8B\xEC\x57\x8B\xF9\x8D\x8F\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x85\xC0\x75\x00\xB0\x00\x5F\x5D\xC2\x00\x00\x53"), false);

      // Found in clc_move.
      //add_hook(HASH("cl_sendmove_hook"), SIG("\xE8\x00\x00\x00\x00\x80\x7D\xFF\x00\x0F\x84\x00\x00\x00\x00"), true);

      // The string "(source/command) %s" is found inside the function.
      add_hook(HASH("cbuf_executecmd_hook"), SIG("\xE8\x00\x00\x00\x00\x83\xC4\x08\xB9\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x84\xC0"), true);

      // break pointed on a call to SetValue from the name concommand in and stepped through 2 functions.
      add_hook(HASH("cvar_set_value_hook"), SIG("\x55\x8B\xEC\x83\xEC\x28\x56\x8B\xF1\x68\x00\x00\x00\x00"), false);

      // "Voice - %d incoming samples added to channel %d\n"
      add_hook(HASH("voice_assign_channel_hook"), SIG("\x55\x8B\xEC\x80\x3D\x00\x00\x00\x00\x00\x74\x07"), false);

      // Search for "StartStaticSound" and sig the function it is in
      add_hook(HASH("s_startsound_hook"), SIG("\xE8\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x83\xC4\x04\x33\xFF"), true);

      add_hook(HASH("draw_model_execute_hook"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x56\x57\x8B\x7D\x00\x89\x4D\x00\x8B\x47"), false);

      // Search for CEngineTrace::TraceRay
      add_hook(HASH("engine_trace_ray_hook"), SIG("\x53\x8B\xDC\x83\xEC\x00\x83\xE4\x00\x83\xC4\x00\x55\x8B\x6B\x00\x89\x6C\x24\x00\x8B\xEC\xB8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x56"), false);

      // Found in "CAbuseReportManager::PopulateIncident".
      // Break point in m_pIncidentData->m_matWorldToClip = engine->WorldToScreenMatrix();
      // Step into engine->WorldToScreenMatrix()
      // that function calls crender_worldtoscren_matrix
      add_hook(HASH("crender_worldtoscreen_matrix_hook"), SIG("\x83\xB9\xE8\x00\x00\x00\x01\x7E\x16"), false);

      // search for "CViewRender::SetUpView->OnRenderEnd" and sig the function is it in. Or ClientDLL_FrameStageNotify
      add_hook(HASH("frame_stage_notify_hook"), SIG("\x55\x8B\xEC\x8B\x0D\x00\x00\x00\x00\x83\xEC\x00\x85\xC9\x74"), false);

      // Be in-game first
      // Set a break point on the tier0 function "ConMsg"
      // Go on a community server and type "sm plugins"
      // Go through the callstack until you find the function that called ConMsg
      // Should look like this. Also if offset changes from 20 to something else update the offset in the s_svc_print struct.
      /*
        ConMsg("%s", *(a1 + 20));
        return 1;
      */
      add_hook(HASH("cbaseclientstate_process_print_hook"), SIG("\x55\x8B\xEC\x8B\x45\x00\xFF\x70"), false);

      // Use the convar fog_enable_water_fog to find it.
      add_hook(HASH("r_setfogvolumestate_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x8A\x45\x00\x53\x56"), false);

      // Search for "SetConVar: %s = \"%s\"\n" and sig the function it is in.
      add_hook(HASH("cbaseclientstate_process_set_convar_hook"), SIG("\x55\x8B\xEC\x8B\x49\x00\x83\xEC"), false);

      // Search for CStaticPropMgr::DrawStaticProps
      add_hook(HASH("cstaticpropmgr_drawstaticprops_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x8B\x15\x00\x00\x00\x00\x53\x56\x57\x33\xF6"), false);

      // Called inside the function above.
      add_hook(HASH("is_using_static_prop_debug_mode_hook"), SIG("\xA1\x00\x00\x00\x00\x83\x78\x00\x00\x75\x00\xA1\x00\x00\x00\x00\x83\x78\x00\x00\x7D"), false);
    }

    set_module(HASH("studiorender.dll"));
    {
      // Search for "Missing LODs for %s, lod index is %d.\n" this will land you in CStudioRender::DrawModel step out of the function and that will land you in CStudioRenderContext::DrawModel BOOM!
      add_hook(HASH("studio_render_context_drawmodel_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x8B\x5D\x00\x8B\xD1\x89\x55"), false);

      // Go in engine.dll and search for the string "CModelRender::ForcedMaterialOverride" this function calls studio render one.
      // Use a debugger to break point in it.
      add_hook(HASH("studio_render_forced_material_override_hook"), SIG("\x55\x8B\xEC\x8B\x45\x00\x89\x81"), false);

      // Use the DrawDecal function to find this function.
      //add_hook(HASH("studio_render_context_draw_model_static_prop_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x8B\x5D\x00\x56\x57\x89\x4D\x00\x8B\x43"), false);
    }

    set_module(HASH("materialsystem.dll"));
    {
      //CMaterial::Uncache
      //You can find this function by searching for the string "materials/%s.vmt" the string will be at the bottom of the function.
      add_hook(HASH("cmaterial_uncache_hook"), SIG("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x53\x56\x57\x8B\xF9\x8B\x0D\x00\x00\x00\x00\x8B\x01"), false);

      // You can use cheat engine and check what accesses this address on the ref counter.
      add_hook(HASH("cmaterial_deleteifunreferenced_hook"), SIG("\x56\x8B\xF1\x83\x7E\x00\x00\x7F\x00\x57"), false);
    }

    DBG("[!] steamnetworkingsockets\n");
    set_module(HASH("steamclient.dll"));
    {
      // Inside CTFGCClientSystem::PingThink()
      // pUtils->GetDirectPingToPOP( id ) is called to measure ping to servers
      add_hook(HASH("get_direct_ping_to_pop_hook"), SIG("\x55\x8B\xEC\x8B\x49\x00\x8B\x01\x5D\xFF\x60\x00\xCC\xCC\xCC\xCC\x55\x8B\xEC\x56\x8B\xF1\x6A"), false);
    }

    set_module(HASH("crashhandler.dll"));
    {
      add_hook(HASH("steam_exception_handler_hook"), SIG("\x55\x8B\xEC\x53\x56\x8B\xF1\x57\x8B\x7D\x00\x8B\x06"), false);
    }


    set_module(HASH("vstdlib.dll"));
    {
      add_hook(HASH("ccvar_find_command_hook"), SIG("\x55\x8B\xEC\x8B\x01\x56\x57\xFF\x50\x00\x8B\xF0\x85\xF6\x74\x00\x8B\x7D\x00\x8B\x06\x8B\xCE\xFF\x50\x00\x50\x57\xE8\x00\x00\x00\x00\x83\xC4\x00\x85\xC0\x74\x00\x8B\xCE\xE8\x00\x00\x00\x00\x8B\xF0\x85\xF6\x75\x00\x5F\x33\xC0\x5E\x5D\xC2\x00\x00\x5F\x8B\xC6\x5E\x5D\xC2\x00\x00\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x55\x8B\xEC\x8B\x01"), false);
    }

    //set_module(HASH("serverbrowser.dll"));
    //{
    //  add_hook(HASH("is_blacklisted_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x56\x57\xFF\x75\x00\x8B\xF9\x8D\x4D"), false);
    //}
#if defined(DEV_MODE)
    //set_module(HASH("vphysics.dll"));
    //{
    //  add_hook(HASH("physics_object_getdragindir_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x56\x8B\xF1\x8D\x45\x00\x50\xFF\x75\x00\x8B\x46"), false);
    //  add_hook(HASH("physics_object_getangdir_hook"), SIG("\x55\x8B\xEC\x8B\x45\x00\xD9\x41\x00\xD8\x48"), false);
    //  add_hook(HASH("physics_clamp_velocity_hook"), SIG("\x55\x8B\xEC\x83\xEC\x00\x53\x56\x8B\xF1\x0F\x57\xC9"), false);
    //}
    //set_module(HASH("server.dll"));
    //{
    //  add_hook(HASH("server_adjust_player_time_base_hook"), SIG("\x55\x8B\xEC\x83\xEC\x0C\x56\x8B\x75\x08\x89\x4D\xFC"), false);
    //  add_hook(HASH("server_fire_bullet_hook"), SIG("\xE8\x00\x00\x00\x00\x8B\x45\x20\x47"), true);
    //}
#endif

    return true;
  }
};