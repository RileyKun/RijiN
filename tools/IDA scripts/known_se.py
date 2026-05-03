
base = idaapi.get_imagebase() + 1024

print("Finding known source engine functions.")
def find_func(sig, name):
  cur_base = base;

  function_name = "None"
  known_names = []
  while True:

    cur = idc.find_binary(cur_base, SEARCH_DOWN | SEARCH_UP | SEARCH_NEXT, sig, 16)
    if cur == idc.BADADDR or cur == 0xffffffff:
      break

    cur_base = cur + 1
    
    name_count = known_names.count(name)
    if name_count == 0:
      function_name = name
    else:
      function_name = name + str(name_count)

    known_names.append(name)
    idc.set_name(cur, function_name)
    print("Found: " + function_name + " at", hex(cur))

while True:
    
    print("Starting")
    find_func('55 8B EC 83 EC ? 53 56 57 8B D9 E8 ? ? ? ? 8D 7B', "C_TFPlayer::ClientThink");
    find_func('55 8B EC 83 EC ? 57 8B F9 E8 ? ? ? ? 8B C8 85 C9', "C_TFPlayer::UpdateIDTarget");
    find_func('55 8B EC 83 EC ? 56 57 8D 45 ? 8B F1 50 89 75', "C_TFPlayer::UpdateLookAt");
    find_func('55 8B EC 83 EC ? 53 56 57 8B F9 E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 0F B7 9F', "C_TFPlayer::UpdateOverheadEffects");
    find_func('55 8B EC 56 8B 75 ? 57 8B F9 83 FE ? 7D', "CTFPlayerShared::InCondition");
    find_func('57 8B F9 8D 87 ? ? ? ? 85 C0', "C_TFPlayer::CanLightCigarette");
    find_func('53 56 8B F1 57 8D 8E ? ? ? ? E8 ? ? ? ? 8B CE', "C_TFPlayer::UpdateRecentlyTeleportedEffect");
    find_func('55 8B EC 83 EC ? 53 56 8B 35 ? ? ? ? 8B CE 57 8B 06 FF 50 ? 8B 0D', "OnRenderStart");
    find_func('55 8B EC 83 EC ? 8D 45 ? 53 56 57 6A ? 68 ? ? ? ? 68 ? ? ? ? 68', "C_BaseAnimating::PushAllowBoneAccess");
    find_func('55 8B EC FF 15 ? ? ? ? 0F B6 15', "C_BaseEntity::EnableAbsRecomputations");
    find_func('55 8B EC FF 15 ? ? ? ? 84 C0 74 ? 80 7D', "C_BaseEntity::SetAbsQueriesValid");
    find_func('FF 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC 80 79', "C_BaseAnimating::InvalidateBoneCaches");
    find_func('8B 15 ? ? ? ? 33 C0 85 D2 7E ? 8D 64 24', "C_BaseEntity::MarkAimEntsDirty");
    find_func('0F B7 05 ? ? ? ? 3D ? ? ? ? 74 ? 8B 15 ? ? ? ? 56', "ProcessOnDataChangedEvents");
    find_func('8B 0D ? ? ? ? 85 C9 74 ? 8B 01 8B 40 ? FF D0 84 C0 74 ? B0 ? C3 32 C0 C3 CC CC CC CC CC 55', "ToolsEnabled");
    find_func('C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ? C3 CC CC CC CC CC CC CC CC CC 55', "Rope_ResetCounters");
    find_func('C7 05 ? ? ? ? ? ? ? ? E8 ? ? ? ? 8B F0 85 F6',"UpdateLocalPlayerVisionFlags");

    find_func("55 8B EC 51 56 8B F1 8B 4D ? 85 C9", "C_TFPlayer::ClearSceneEvents");
    find_func('55 8B EC 53 57 8B 7D ? 8B D9 85 FF 74 ? 8B 07 83 B8', "C_TFPlayer::ResetFlexWeights");
    find_func('57 8B F9 80 BF ? ? ? ? ? 75 ? 83 BF ? ? ? ? ? 75 ? E8', "C_TFPlayer::InitializePoseParams");
    find_func('55 8B EC 83 EC ? 53 56 8B F1 8D 9E ? ? ? ? 89 75 ? C7 43 ? ? ? ? ? 83 7B ? ? 57', "C_TFPlayer::InitPlayerGibs");
    find_func('55 8B EC 56 8B F1 80 BE ? ? ? ? ? 74 ? 83 C8', "FindBodygroupByName");

    find_func('56 57 8B F1 E8 ? ? ? ? 8B CE 8B F8 E8 ? ? ? ? 8B CE E8 ? ? ? ? 6A', "C_TFPlayer::OnNewModel");
    find_func('55 8B EC 53 8A 5D ? 56 8B F1 84 DB 74 ? 8B 06 FF 90 ? ? ? ? 83 E8 ? 74 ? 83 E8 ? 75 ? B8 ? ? ? ? 8B CE 50 E8 ? ? ? ? 88 9E ? ? ? ? 5E 5B 5D C2 ? ? B8 ? ? ? ? 8B CE 50 E8 ? ? ? ? 88 9E ? ? ? ? 5E 5B 5D C2 ? ? 6A ? 68 ? ? ? ? E8 ? ? ? ? 6A ? 68 ? ? ? ? 8B CE E8 ? ? ? ? 88 9E ? ? ? ? 5E 5B 5D C2 ? ? CC CC CC CC CC CC CC CC 55', "C_TFPlayer::ShowNemesisIcon");
    find_func('55 8B EC 51 56 8B F1 6A ? 8D 8E ? ? ? ? E8 ? ? ? ? 84 C0 75 ? 8B 86 ? ? ? ? C1 E8 ? 24 ? 75 ? F6 86', "CTFPlayerShared::IsStealthed");
    find_func('55 8B EC 57 8B F9 E8 ? ? ? ? 84 C0 0F 85', "C_TFPlayer::UpdatedMarkedForDeathEffect");
    find_func('55 8B EC 53 56 8B F1 8D 45', "C_TFPlayer::RemoveOverheadEffect");
    find_func('55 8B EC 83 EC ? 8D 45 ? 56 8B F1 57 50 8D 8E', "C_TFPlayer::AddOverheadEffect");
    find_func('55 8B EC A1 ? ? ? ? 83 EC ? 56 8B 70', "EnableSteamScreenshots");
    find_func('55 8B EC 83 EC ? 56 8B 75 ? 85 F6 0F 84 ? ? ? ? 81 FE', "ScreenshotTaggingKeyInput");
    find_func('55 8B EC 51 53 56 57 E8 ? ? ? ? 8B F0', "TauntHandlesKeyInput");
    find_func('55 8B EC 56 E8 ? ? ? ? 8B F0 85 F6 0F 84 ? ? ? ? 8B 16 8B CE 8B 92 ? ? ? ? FF D2 84 C0 0F 84 ? ? ? ? 6A', "HalloweenHandlesKeyInput");
    find_func('83 78 ? ? 0F 95 C0 A2', "C_BaseAnimating::ThreadedBoneSetup");
    find_func('55 8B EC 51 53 8B 5D ? 56 8B 75 ? 57 8B 7D', "TrainingHandlesKeyInput");
    find_func('55 8B EC 8B 45 ? 8B 4D ? 3B C1 74 ? 51', "FStrEq");
    find_func('55 8B EC 56 8B 75 ? 57 8B 7D ? 3B F7 75', "FStrEq2");
    find_func('53 8B DC 83 EC ? 83 E4 ? 83 C4 ? 55 8B 6B ? 89 6C 24 ? 8B EC 81 EC ? ? ? ? A1 ? ? ? ? 56 8B F1', "CTFWeaponBaseMelee::DoSwingTraceInternal");
    find_func('55 8B EC 6A ? 6A ? FF 75 ? E8 ? ? ? ? 5D C2 ? ? CC CC CC CC CC CC CC CC CC CC CC CC CC 53', "CTFWeaponBaseMelee::DoSwingTrace");
    find_func('55 8B EC 8B 55 08 8B 4D 10 8B 45 0C 89 4A 2C 8B 4D 14 89 4A 08', "RecvPropDataTable");
    find_func('55 8B EC 8B 45 08 8B 55 1C C7 40 ? ? ? ? ? C7 00 ? ? ? ? C7 40 ? ? ? ? ? C7 40 ? ? ? ? ? C7 40 ? ? ? ? ? C7 40 ? ? ? ? ? C7 40 ? ? ? ? ?', "RecvPropInt")
    find_func('55 8B EC 6A 00 6A 00 FF 75 14 FF 75 10 FF 75 0C FF 75 08 E8 ? ? ? ?', "RecvPropBool")
    find_func('55 8B EC FF 75 18 6A 00 FF 75 14 FF 75 10 FF 75 0C', "RecvPropEHandle")
    find_func('55 8B EC 8B 45 08 8B 4D 0C 89 08 8B 4D 10 89 48 2C 8B 4D 18 89 48 08 8B 4D 1C', "RecvPropVector");
    find_func('55 8B EC D9 EE 8B 55 0C', "ConVar::RegisterConvar");
    find_func('55 8B EC D9 45 24 8B 55 14', "IConVar::RegisterConvar");
    find_func('55 8B EC 83 EC 18 8B 55 08 F3 0F 10 05 ? ? ? ? 56 8B F1 F3 0F 11 45 ? 8B 02 89 45 F4 8B 42 04 89 45 F8 8B 42 08 89 45', "ApplyAbsVelocityImpulse")
    find_func('55 8B EC 8B 55 08 8B 4D 10 8B 45 0C D9 02 D9 1C 81 D9 42 04 D9 5C 81 10 D9 42 08 D9 5C 81 20 5D C3', "MatrixSetColumn")
    find_func('55 8B EC 8B 4D 0C 8B 45 08 F3 0F 10 49 ? F3 0F 10 20 F3 0F 10 50 ? F3 0F 5C 61 ? F3 0F 5C 51 ? F3 0F 10 01 F3 0F 10 58 ? F3 0F 5C 59 ? 8B 45 10 F3 0F 59 C4 F3 0F 59 CA F3 0F 58 C8 F3 0F 10 41 ? F3 0F 59 C3 F3 0F 58 C8 F3 0F 11 08 F3 0F 10 49 ? F3 0F 10 41 ? F3 0F 59 C4 F3 0F 59 CA F3 0F 58 C8 F3 0F 10 41 ? F3 0F 59 C3 F3 0F 58 C8 F3 0F 11 48 ? F3 0F 10 41 ? F3 0F 10 49 ? F3 0F 59 C4 F3 0F 59 CA F3 0F 58 C8 F3 0F 10 41 ? F3 0F 59 C3 F3 0F 58 C8 F3 0F 11 48 ? 5D C3', "VectorITransform")
    find_func('55 8B EC 83 EC 20 8B 4D 08 8D 45 EC F3 0F 10 0D ? ? ? ? 89 45 F4 8D 45 E8 89 45 F0 F3 0F 10 41 ? F3 0F 59 C1 F3 0F 11 45 ? D9 45 F8 D9 FB 8B 55 F4 8B 45 F0 D9 1A D9 18 F3 0F 10 01 8D 45 08 F3 0F 59 C1 89 45 F4 8D 45 E4 89 45 F8 F3 0F 11 45 ? D9 45 F0 D9 FB 8B 55 F4 8B 45 F8 D9 1A D9 18 F3 0F 10 41 ? 8D 45 FC F3 0F 59 C1 89 45 F4 8D 45 E0 89 45 F8 F3 0F 11 45 ? D9 45 F0 D9 FB 8B 55 F4 8B 45 F8 D9 1A D9 18 8B 45 0C F3 0F 10 4D ? F3 0F 10 45 ? F3 0F 10 5D ? F3 0F 10 7D ? F3 0F 10 55 ? F3 0F 59 C1 0F 28 E2 C7 40 ? ? ? ? ? F3 0F 10 75 ? F3 0F 10 6D ? F3 0F 11 00 F3 0F 10 45 ? F3 0F 59 C3 C7 40 ? ? ? ? ? F3 0F 59 E1 C7 40 ? ? ? ? ? F3 0F 11 40 ? 0F 28 C7 0F 57 05 ? ? ? ? F3 0F 11 40 ? 0F 28 C4 F3 0F 59 C7 F3 0F 59 F3 F3 0F 59 E9 0F 28 CA F3 0F 59 55 ? F3 0F 5C C6 F3 0F 59 CB F3 0F 59 F7 F3 0F 11 40 ? 0F 28 C1 F3 0F 59 C7 F3 0F 5C F4 F3 0F 11 50 ? F3 0F 58 C5 F3 0F 59 EF F3 0F 11 70 ? F3 0F 58 E9 F3 0F 11 40 ? F3 0F 10 45 ? F3 0F 59 45 ? F3 0F 11 68 ? F3 0F 11 40 ? 8B E5 5D C3', "AngleMatrix")
    find_func('55 8B EC 8B 4D 08 8B 45 0C 3B C8 75 32 F3 0F 10 40 ? D9 40 10 D9 58 04 F3 0F 11 40 ? F3 0F 10 40 ? D9 40 20 D9 58 08 F3 0F 11 40 ? D9 40 24 F3 0F 10 40 ? D9 58 18 F3 0F 11 40 ? EB 34 D9 01 D9 18 D9 41 10 D9 58 04 D9 41 20 D9 58 08 D9 41 04 D9 58 10 D9 41 14 D9 58 14 D9 41 24 D9 58 18 D9 41 08 D9 58 20 D9 41 18 D9 58 24 D9 41 28 D9 58 28 F3 0F 10 69 ? F3 0F 10 59 ? F3 0F 10 48 ? F3 0F 10 00 F3 0F 10 61 ? F3 0F 10 15 ? ? ? ? F3 0F 59 CB F3 0F 59 C5 F3 0F 58 C8 0F 28 C4 F3 0F 59 40 ? F3 0F 58 C8 0F 57 CA F3 0F 11 48 ? F3 0F 10 48 ? F3 0F 10 40 ? F3 0F 59 CB F3 0F 59 C5 F3 0F 58 C8 F3 0F 10 40 ? F3 0F 59 C4 F3 0F 58 C8 0F 57 CA F3 0F 11 48 ? F3 0F 10 40 ? F3 0F 10 48 ? F3 0F 59 CB F3 0F 59 C5 F3 0F 58 C8 F3 0F 10 40 ? F3 0F 59 C4 F3 0F 58 C8 0F 57 CA F3 0F 11 48 ? 5D C3', "MatrixInvert")
    find_func('53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 10 8B 43 08 0F 28 05 ? ? ? ? 0F 29 45 F0 0F 10 10 0F 10 60 10 0F 10 78 20 8B 43 0C 0F 28 CA 0F 28 C2 0F C6 CA AA 0F C6 C2 55 0F 10 68 10 0F 10 58 20 0F 10 30 8B 43 10 0F 59 C5 0F 59 CB 0F 58 C8 0F 28 C2 0F C6 C2 00 0F 54 55 F0 0F 59 C6 0F 58 C8 0F 28 C4 0F C6 C4 55 0F 59 C5 0F 58 CA 0F 11 08 0F 28 CC 0F C6 CC AA 0F 59 CB 0F 58 C8 0F 28 C4 0F C6 C4 00 0F 54 65 F0 0F 59 C6 0F 58 C8 0F 28 C7 0F C6 C7 55 0F 59 C5 0F 58 CC 0F 11 48 10 0F 28 CF 0F C6 CF AA 0F 59 CB 0F 58 C8 0F 28 C7 0F C6 C7 00 0F 54 7D F0 0F 59 C6 0F 58 C8 0F 58 CF 0F 11 48 20 8B E5 5D 8B E3 5B C3', "ConcatTransforms")
    find_func('55 8B EC 83 EC 24 8B 45 08 F3 0F 10 40 ? F3 0F 10 10 F3 0F 10 48 ? F3 0F 11 45 ? F3 0F 10 40 ? F3 0F 11 4D ? F3 0F 11 45 ? F3 0F 10 40 ? F3 0F 11 55 ? F3 0F 59 C9 F3 0F 59 D2 F3 0F 11 45 ? F3 0F 10 40 ? F3 0F 58 CA F3 0F 11 45 ? F3 0F 10 40 ? F3 0F 11 45 ? 8B 45 0C F3 0F 51 C1 0F 2F 05 ? ? ? ? F3 0F 11 45 ? 76 34 D9 45 F8 D9 45 F4 D9 F3 D9 05 ? ? ? ? DC C9 D9 C9 D9 58 04 D9 45 FC D9 E0 D9 45 08 D9 F3 D8 C9 D9 18 D9 45 F0 D9 45 E4 D9 F3 DE C9 D9 58 08 8B E5 5D C3', "MatrixAngles")    
    find_func('55 8B EC 8B 55 08 8B 4D 0C 8B 45 10 D9 04 8A D9 18 D9 44 8A 10 D9 58 04 D9 44 8A 20 D9 58 08 5D C3', "MatrixGetColumn")
    find_func('55 8B EC A1 ? ? ? ? F3 0F 10 40 ? 0F 2E 05 ? ? ? ? 9F F6 C4 44 7B 2B 8D 45 0C 50 FF 75 08 68 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? 6A 01 FF 15 ? ? ? ? 83 C4 1C 5D C3', "DevMsgRT")
    find_func('55 8B EC 56 8B 75 0C 57 FF 75 14 8B 7D 08 6A 00 FF 75 10 56 57 E8 ? ? ? ? 8B 08 FF 70 04 83 C9 01 51 E8 ? ? ? ? 83 C9 FF 83 C4 1C 85 C0 0F 48 C1 85 C0 78 04 3B C6 7C 07 C6 44 37 ? ? 8B C6 5F 5E 5D C3', "Q_vsnprintf")
    find_func('55 8B EC 83 EC 68 6A 58 8D 45 98 6A 00 50 E8 ? ? ? ?', "MD5_PseudoRandom")
    find_func('55 8B EC 83 EC 50 53 8B 5D 08', "CInput::CreateMove")
    find_func('55 8B EC 83 EC 48 33 C0 C7 45 ? ? ? ? ?', "CInput::WriteUsercmdDeltaToBuffer")
    find_func('55 8B EC 83 EC 08 53 8B 5D 0C 56 8B 75 10 57 8B 7D 08 8B 46 04 40', "WriteUsercmd")
    find_func('55 8B EC A1 ? ? ? ? 83 EC 0C F3 0F 10 4D ?', "CInput::AdjustAngles")
    find_func('55 8B EC 8B 45 ? 56 8B F1 85 C0 75 ? 8B 86', "C_BaseEntity::GetNextThinkTick")
    find_func('55 8B EC 8B 81 ? ? ? ? 8B D0', "C_BaseEntity::UpdateButtonState")
    find_func('55 8B EC F3 0F 10 45 ? 0F 2E 05 ? ? ? ? 53', "C_BaseEntity::SetNextThink")
    find_func('55 8B EC A1 ? ? ? ? 83 78 30 00 74 04 32 C0 5D C3', "TF_IsHolidayActive")
    find_func('55 8B EC 83 EC 0C 8B 0D ? ? ? ? 53 56 57 33 F6 33 FF 89 75 F4 89 7D F8 8B 41 08 85 C0 74 38', "CALL_ATTRIB_HOOK_FLOAT")
    find_func('55 8B EC 83 EC 10 8B 0D ? ? ? ? 53 56 57 33 FF 33 DB 89 7D F0 89 5D F4 8B 41 08 85 C0', "CALL_ATTRIB_HOOK_INT")
    find_func('55 8B EC 83 EC 18 56 57 6A 00 68 ? ? ? ? 68 ? ? ? ? 6A 00 8B F9 E8 ? ? ? ? 50 E8 ? ? ? ? 8B F0 83 C4 14 89 75 EC', "CTFWeaponBase::CalcIsAttackCriticalHelper")
    find_func('53 57 6A 00 68 ? ? ? ? 68 ? ? ? ? 6A 00 8B F9 E8 ? ? ? ? 50 E8 ? ? ? ? 8B D8 83 C4 14 85 DB 0F 84 ? ? ? ?', "CTFWeaponBase::CalcIsAttackCritical")
    find_func('55 8B EC 81 EC ? ? ? ? 53 56 8B D9 57 89 5D E8', "CHudTFCrosshair::Paint")
    find_func('55 8B EC 8B 45 08 89 41 04 8B 45 0C 89 41 08 8B 45 10 89 41 0C 8B C1', "CTraceFilterSimple::Constructor")
    find_func('55 8B EC 81 EC ? ? ? ? 53 8B 5D 18 56 53 E8 ? ? ? ? 8B F0 83 C4 04 85 F6 75 17 53', "FX_FireBullets")
    find_func('55 8B EC 8B 45 08 83 F8 6E 73 0D 85 C0 78 09', "WeaponIDToAlias")
    find_func('55 8B EC 8B 45 08 83 EC 08 85 C0 74 18 89 45 F8 B9 ? ? ? ? 8D 45 F8 50 E8 ? ? ? ? 0F B7 C0 8B E5 5D C3', "LookupWeaponInfoSlot")
    find_func('55 8B EC B8 ? ? ? ? E8 ? ? ? ? 8D 45 10 50 FF 75 0C 8D 85 ? ? ? ? 68 ? ? ? ? 50 E8 ? ? ? ? 8B 0D ? ? ? ? 8D 85 ? ? ? ? 83 C4 10 50 FF 75 08 E8 ? ? ? ? 8B E5 5D C3', "Q_vsprintf")
    find_func('55 8B EC 83 EC 18 8D 45 F4 56 50 FF 75 08 8B F1 E8 ? ? ? ? 8D 45 E8 50 8D 45 F4 50 8D 46 2C 50 8D 46 20 50 E8 ? ? ? ? 83 C4 10 8D 45 E8', "CCollisionProperty::CalcNearestPoint");
    find_func('55 8B EC 56 8B 75 ? 8D 45 ? 57 8B 7D ? 50 6A ? FF 75 ? 56 57 E8 ? ? ? ? 8B 08 FF 70 ? 83 C9 ? 51 E8 ? ? ? ? 83 C9 ? 83 C4 ? 85 C0 0F 48 C1 85 C0 78 ? 3B C6 7C ? C6 44 37', "Q_snprintf");
    find_func('55 8B EC 8B 4D ? 56 8B F1', "Q_StripExtension");
    find_func('55 8B EC 8D 81 ? ? ? ? F7 D9', "FindSceneFile");
    find_func('55 8B EC 51 53 56 8B F1 57 68 ? ? ? ? 6A ? 8D 9E ? ? ? ? 89 75 ? 53 E8 ? ? ? ? 8B 7D ? 81 C6 ? ? ? ? 57 68 ? ? ? ? 6A ? 56 E8 ? ? ? ? 57 68 ? ? ? ? 6A ? 53 C6 46 ? ? E8 ? ? ? ? 8B 45 ? 57 68 ? ? ? ? 05 ? ? ? ? 6A ? 50 E8 ? ? ? ? 83 C4 ? 5F 5E 5B 8B E5 5D C2 ? ? CC CC CC CC CC CC 55 8B EC 80 B9', "SetupMappings");
    find_func('55 8B EC 81 EC ? ? ? ? 56 8B F1 80 BE ? ? ? ? ? 0F 85 ? ? ? ? 83 BE ? ? ? ? ? 75 ? E8 ? ? ? ? 8B 96', "C_TFPlayer::InitPhonemeMappings");
    find_func('55 8B EC 83 EC ? 53 8B D9 56 57 8D 7B ? 8B CF 89 7D ? E8 ? ? ? ? 50', "C_TFPlayer::OnDataChanged");
    find_func('55 8B EC 8B 45 ? D9 00 D9 99 ? ? ? ? D9 40 ? D9 99 ? ? ? ? D9 40 ? D9 99 ? ? ? ? 5D C2 ? ? CC CC CC CC CC CC CC CC CC CC CC CC 55 8B EC 8B 45 ? D9 00', "C_BaseEntity::SetNetworkAngles");
    find_func('53 57 8B F9 E8 ? ? ? ? 8B D8 85 DB 0F 84 ? ? ? ? 8B 13 8B CB 8B 92 ? ? ? ? FF D2 84 C0 74', "C_TFPlayer::InitInvulnerableMaterial");
    find_func('55 8B EC 83 EC ? 53 56 57 8B F9 E8 ? ? ? ? 6A', "C_TFPlayer::UpdateKartSounds");
    find_func('55 8B EC 83 EC ? 53 57 8B F9 E8 ? ? ? ? 83 BF', "C_TFPlayer::StartBurningSound");
    find_func('55 8B EC 83 EC ? 53 56 57 8B F9 8B 0D ? ? ? ? 89 7D ? 85 C9', "CNewParticleEffect::Create");
    find_func('55 8B EC 51 8B 45 ? 53 8B D9 85 C0', "CNewParticleEffect::StopParticlesNamed");
    find_func('55 8B EC 83 EC ? 56 8B 75 ? 83 7E', "S_StartSound");
    find_func('55 8B EC 83 EC ? 56 8B 75 ? 8B CE FF 75', "CL_LoadWhitelist");
    #find_func('55 8B EC 83 EC ? 53 56 57 8B F9 8B 8F ? ? ? ? 83 F9 ? 0F 84 ? ? ? ? A1', "C_TFPlayer::ShouldDraw");
    find_func('55 8B EC 83 EC ? 53 56 57 8B F9 8B 8F ? ? ? ? 83 F9 ? 0F 84 ? ? ? ? 8B 35', "C_TFPlayer::ShouldDraw");
    find_func('53 56 57 8B F1 E8 ? ? ? ? 8B D8', "C_TFPlayer::HandleTaunting");
    find_func('55 8B EC 83 EC ? 53 56 57 8B F9 8D 8F', "C_TFPlayer::GetEffectiveInvisibilityLevel");
    find_func('56 8B F1 8D 4E ? E8 ? ? ? ? 84 C0 74 ? 80 BE', "CTFPlayerShared::IsAiming");
    find_func('55 8B EC 56 8B F1 80 7E ? ? 74 ? B0', "SetupPoseParameters");
    find_func('55 8B EC 81 EC ? ? ? ? 53 57 8B F9 8B 9F', "CTFPlayerAnimState::Update");
    find_func('8B 0D ? ? ? ? 53 56 32 DB 8B 01', "ForceCompetitiveConvars");
    find_func('55 8B EC A1 ? ? ? ? 83 EC ? 56 A8', "GetClientInterp");
    find_func("55 8B EC 56 8B F1 F6 46 ? ? 75 ? 8A 46 ? 3C ? 74 ? 84 C0 74 ? 8B 06 FF 50 ? F3 0F 10 05 ? ? ? ? 8B C8 0F 2E 01 9F F6 C4 ? 7A ? F3 0F 10 05 ? ? ? ? 0F 2E 41 ? 9F F6 C4 ? 7A ? F3 0F 10 05 ? ? ? ? 0F 2E 41 ? 9F F6 C4 ? 7B ? 8B 06 8B CE FF 50 ? 8B 75 ? 56 50 FF 75 ? E8 ? ? ? ? 83 C4 ? 8B C6 5E 5D C2 ? ? 8B 06 8B CE FF 50 ? 8B 4D ? 8B D0 8B 45 ? 5E F3 0F 10 01", "WorldToCollisionSpace")
    find_func("55 8B EC 8B 55 ? 0F 57 C9", "CalcClosestPointOnAABB")
    find_func("55 8B EC 56 8B F1 F6 46 ? ? 75 ? 8A 46 ? 3C ? 74 ? 84 C0 74 ? 8B 06 FF 50 ? F3 0F 10 05 ? ? ? ? 8B C8 0F 2E 01 9F F6 C4 ? 7A ? F3 0F 10 05 ? ? ? ? 0F 2E 41 ? 9F F6 C4 ? 7A ? F3 0F 10 05 ? ? ? ? 0F 2E 41 ? 9F F6 C4 ? 7B ? 8B 06 8B CE FF 50 ? 8B 75 ? 56 50 FF 75 ? E8 ? ? ? ? 83 C4 ? 8B C6 5E 5D C2 ? ? 8B 06 8B CE FF 50 ? 8B 4D ? 8B D0 8B 45 ? 5E F3 0F 10 02", "CollisionToWorldSpace")
    find_func("55 8B EC 83 EC ? E8 ? ? ? ? 85 C0 0F 84 ? ? ? ? 8B 10", "ClientMode::OverrideView")
    find_func("55 8B EC 8B 45 ? 85 C0 74 ? 89 45 ? 81 C1 ? ? ? ? 5D E9 ? ? ? ? 5D C2 ? ? CC CC CC 55 8B EC 8A 45", "HLTV::CreateMove")

    print("Finished")
    print("You can click on the names in the console to jump the function")
    break