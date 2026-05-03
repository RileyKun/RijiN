import hashlib

already_listed = []

# The source engine debug message functions.
wanted_imports = ["Warning", "Msg", "DevMsg", "ConMsg", "DevWarning", "DevMsgRT", "?DevWarning@@YAXPBDZZ", "?ConWarning@@YAXPBDZZ", "?ConColorMsg@@YAXABVColor@@PBDZZ", "ConDMsg"]

print("\n\n\n\n\n\nScanning all source engine DBG messages\n\n\n")
def import_callback(ea, name, i):

  if wanted_imports.count(name) != 0: # Is this the import we're looking for?
    for x in CodeRefsTo(ea, False):
      pos = wanted_imports.index(name) # Get the position of the import with in the list so we can get the offset of it.
      if pos == -1:
        continue

      x = idc.find_binary(x, SEARCH_UP, "68 ? ? ? ? ", 16)

      # Get the debug text
      warning_txt = str(ida_bytes.get_strlit_contents(idc.get_operand_value(x, 0), -1, ida_nalt.STRTYPE_C))
      if warning_txt == "None":
        continue
      
      # Clean the string a bit of non ida safe names for functions or whatever...  
      warning_txt = warning_txt.replace("b'", "").replace('b"','').replace('"','').replace(">","").replace("'","").replace("(","_").replace(")","_").replace("-","").strip()
      warning_txt = warning_txt.strip()
      
      # Create a hash of the debug text.
      hash_obj = hashlib.md5(warning_txt.encode())
      str_hash = hash_obj.hexdigest()
   
      #print("[" + name + "]" + warning_txt + "->" + str_hash)

      # Find debug strings and name the function to what it's supposed to be with the hashes.
      function_name = "None"
      if str_hash == "ed93706855a8fdf4c5213548445dd22d":
        function_name = "FmtStrVSNPrintf"
      elif str_hash == "b7e514eaa93c51d2dccc00551bfa934b":
        function_name = "CInput::WriteUsercmdDeltaToBuffer"
      elif str_hash == "75dd3736d5dfd98178c4bc9a42760a4f":
        function_name = "CC_DumpClientSoundscapeData"
      elif str_hash == "99004b384261c3dc7922bbbce833fe12":
        function_name = "C_BaseAnimating::DelayedInitModelEffects"
      elif str_hash == "cc6546f9fe98f575d106d07655c8e09d":
        function_name = "C_BaseAnimating::FindFollowedEntity"
      elif str_hash == "37b3b96e8b947be50f74ce203319dafe":
        function_name = "C_BaseAnimating::DoAnimationEvents"
      elif str_hash == "734cfde804fdb031aefc4b34995201cb":
        function_name = "C_BaseAnimating::BecomeRagdollOnClient"
      elif str_hash == "9645e1dcd5bbfca4e4005a398007e000":
        function_name = "C_BaseEntity::UnlinkFromHierarchy"
      elif str_hash == "345bea2e11d9835c882620115d142f2d":
        function_name = "C_BaseAnimating::FrameAdvance"
      elif str_hash == "2ae7af3b556a4c0d7797fa48ec9cb820":
        function_name = "CScreenSpaceEffectManager::GetScreenSpaceEffect"
      elif str_hash == "7085feb24d9ee57782ad2f75d97ca9fa":
        function_name = "__MsgFunc_VGUIMenu"
      elif str_hash == "9fef5b1477aa2cfbf927bd39b9ab9295":
        function_name = "ClientModeShared::FireGameEvent"
      elif str_hash == "91dddb155e58cbfc3f61f2052dd6d221":
        function_name = "CPrediction::ShutdownPredictables"
      elif str_hash == "5a5ae1628d3bed60a41a44cdd01f6fb3":
        function_name = "CPrediction::ReinitPredictables"
      elif str_hash == "2168aae246fba0c3d399d2e89163ae9f":
        function_name = "CPrediction::RemoveStalePredictedEntities"
      elif str_hash == "3783968182f4c7ac77a6644403fcf28d":
        function_name = "KeyDown"
      elif str_hash == "cdf41c8ceeedd3a77f3c096a226d37fb":
        function_name = "C_TFRagdoll::FrameAdvance"
      elif str_hash == "6947959d8959b49606cb288e20ff4421":
        function_name = "FX_FireBullets"
      elif str_hash == "b9f32a450fabb6f8a1d7546efb616d54":
        function_name = "SV_ActivateServer"
      elif str_hash == "925a0b551e68b5eb80901b07afd0f7df":
        function_name = "CLZMAStream::TryParseHeader"
      elif str_hash == "f1fbec2a84f794c12acf6db8ab99d87c":
        function_name = "CCommand::Tokenize"
      elif str_hash == "dd4340ad13662b8049fac42770f341b5":
        function_name = "ConVarRef::Init"
      elif str_hash == "934a7581afb948cd1d9d1055cbb829fa":
        function_name = "CGameMovement::CheckVelocity"
      elif str_hash == "39b21f74edcbf617d6d358fbdbcb10e1":
        function_name = "CGameMovement::Duck"
      elif str_hash == "e0528c7561f9302583b2b1fc2cc95112":
        function_name = "CGameMovement::PlayerMove"
      elif str_hash == "641dee1538c1a2acf3d6edc3b73f7b37":
        function_name = "CTFPartyClient::CancelOutgoingJoinRequestOrIncomingInvite"
      elif str_hash == "965273ff7c1637f09fb18a8156eee857":
        function_name = "GetClientConVarValue"
      elif str_hash == "babb9347a539b282eb170537fe3e38c0":
        function_name = "KeyValues::RecursiveSaveToFile"
      elif str_hash == "51d0e8348ea93eb211ae8cf2f1a92780":
        function_name = "KeyValues::SaveToFile"
      elif str_hash == "da58412be54748c32d8378f1ee02e793":
        function_name = "CL_ProcessPacketEntities"
      elif str_hash == "7f522ba54fe5b8386f7323ffdc44b672":
        function_name = "CBaseCombatWeapon::Precache"
      elif str_hash == "3da2b7c4bb6a49841209ab80f1ced474":
        function_name = "GetSequenceName"
      elif str_hash == "dc945b586323323a0486964b45f887cf":
        function_name = "GetSequenceActivityName"
      elif str_hash == "276575c85c6b2bd4adf4c1067fc7fc3d":
        function_name = "C_BaseEntity::PhysicsDispatchThink"
      elif str_hash == "4bdc8d3a094ddfca384b5d8d15ceeb2a":
        function_name = "CValueChangeTracker::SetupTracking"
      elif str_hash == "c76bfad064e6cf595af60380786ca89e":
        function_name = "cl_pred_track_callback"
      elif str_hash == "aa6d6ec7666bd557c2aa4146f81d6359":
        function_name = "CVoiceStatus::SetPlayerBlockedState"
      elif str_hash == "f4b726ed49493bf7f1aac99a9a160c03":
        function_name = "CVoiceStatus::UpdateServerState"
      elif str_hash == "35666894923ba9d0e98fb5bda3a2d08a":
        function_name = "CVoiceStatus::HandleReqStateMsg"
      elif str_hash == "2b20312f000afbb82edd7348df372e3c":
        function_name = "CVoiceStatus::HandleVoiceMaskMsg"
      elif str_hash == "2806c3d2de460831ac8c4ebbd6fe1438":
        function_name = "CVoiceStatus::UpdateSpeakerStatus"
      elif str_hash == "f92fb5f8cb2b7a90c3b354afba4d02b7":
        function_name = "CBaseEntity::ApplyAbsVelocityImpulse"
      elif str_hash == "85dc1d1bd1b62082fdb2d7af1df84237":
        function_name = "CAchievementMgr::AwardAchievement"
      elif str_hash == "3c4121d9b5f2f3a6e2aaa359d2cbce9f":
        function_name = "CAchievementMgr::UpdateAchievement"
      elif str_hash == "9b91f0dbb0ac55a5fd96fd8577b32399":
        function_name = "CAchievementMgr::CheckAchievementsEnabled"
      elif str_hash == "b4450efd86de4eda42c02ab4d80112e0":
        function_name = "CAchievementMgr::ResetAchievements"
      elif str_hash == "366387f7ab7edba71a5634740d4eb659":
        function_name = "CC_DumpClientSoundscapeData"
      elif str_hash == "242fc9ab3d83b511b26160e6b2122b5a":
        function_name = "CTFGCClientSystem::DumpPing"
      elif str_hash == "c9508dd10e701371751d692badd8e0db":
        function_name = "CGCClientSystem::ReceivedClientGoodbye"
      elif str_hash == "1e7abe1b20d63798a83d202c851fa9c2":
        function_name = "CGCClientSystem::InitGC"
      elif str_hash == "85db7015918b2d4164a2d9dbb0ccb11f":
        function_name = "CClientSteamContext::Activate"
      elif str_hash == "47f075d407a8782d79a02ab97b97d26c":
        function_name = "CClientSteamContext::OnSteamServerConnectFailure"
      elif str_hash == "edc250715713bd93ea5f6262f9732e77":
        function_name = "CClientSteamContext::OnSteamServersConnected"
      elif str_hash == "32106d90d07f4b2704ef455ac9f016c6":
        function_name = "CClientSteamContext::OnGameJoinRequested"
      elif str_hash == "11c20c2b58990d9c94bfb8fa413b049f":
        function_name = "CInventoryManager::LoadPreset"
      elif str_hash == "22b3c72d072ae1aa19da996db792e6dd":
        function_name = "IDelayedSchemaData::InitializeSchemaInternal"
      elif str_hash == "27e654a242114ced0c681cf4c075fc78":
        function_name = "CBaseObject::CreateBuildPoints"
      elif str_hash == "bc713b04f5b25acde3c440131244416f":
        function_name = "CRopeManager::AddToRenderCache"
      elif str_hash == "94bd3e02b1d99a870cae3f09975fe3b5":
        function_name = "CRopeManager::GetNextSegmentFromCache"
      elif str_hash == "97d1c705a57fb465eb3f3992789f6052":
        function_name = "CAmmoDef::AddAmmoType"
      elif str_hash == "2cc92acfff33b18bb072153d02b92c9f":
        function_name = "CC_CL_Find_Ent"
      elif str_hash == "c5fd6dec79bd096b17a8df06d2df505e":
        function_name = "CBaseEntity::PhysicsCheckVelocity"
      elif str_hash == "128da7a87dac2c9cb4e5dd6ccc00dd23":
        function_name = "SOTrackerSpew"
      elif str_hash == "30ab1e3f1b535327216fd492152f944a":
        function_name = "SOTrackerSpewTypeToggle"
      elif str_hash == "9b5ac4db3781e6ca6eb9af5b1fe0e370":
        function_name = "CIKContext::AutoIKRelease"
      elif str_hash == "8040920d81d9f6ca950339d0c97202d8":
        function_name = "CHud::FindElement"
      elif str_hash == "9a0236dcd22044ab9b12cc8caec6c07a":
        function_name = "CHud::Init"
      elif str_hash == "0187a9212d3d9f6bf2d4c299e5383dfb":
        function_name = "vr_toggle"
      elif str_hash == "e66008c4034a411e9f14ff2934688a78":
        function_name = "CHudBaseDeathNotice::FireGameEvent"
      elif str_hash == "cf0087221b4bccc36b2ddb4aa0dc88ba":
        function_name = "CViewRenderBeams::DrawBeam"
      elif str_hash == "205911772388c3cd23a63bbcbc0c1ce6":
        function_name = "C_BaseEntity::DrawModel"
      elif str_hash == "7a46544a7862cb6342df6d7612de6049":
        function_name = "C_BaseFlex::FlexControllerLocalToGlobal"
        
      # Do not continue if the function name is None
      if function_name == "None":
        continue

      # Do not continue if the function name has been seen before
      function_name_count = already_listed.count(function_name)
      if function_name_count != 0:
        continue

      for func in Functions(x, x - 5):
        idc.set_name(func, function_name)
        print("Found: " + function_name + " at", hex(func))

      already_listed.append(function_name)

  return True

while True:

  # Check through every import for the debugging messages SE uses
  import_amount = idaapi.get_import_module_qty()
  for i in range(0, import_amount):
    cur = idaapi.get_import_module_name(i)
    idaapi.enum_import_names(i, import_callback)
 

  break
print("Found " + str(len(already_listed)) + " functions")