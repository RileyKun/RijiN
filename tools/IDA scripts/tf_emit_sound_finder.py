import hashlib

already_listed = []

print("\n\n\nFinding TF functions...\n\n\n");

# This can be used for anything that passes strings.
# Its just you may have to change some of the offets and code sadly.
base = idaapi.get_imagebase() + 1024
attrib_managers_sigs = ["55 8B EC 83 EC ? 8B 15 ? ? ? ? 53 56 57 33 FF 33 DB 89 7D ? 8B F1 89 5D ? 8B 42 ? 89 75", "55 8B EC 83 EC ? 83 7D ? ? 0F 84 ? ? ? ? 8B 0D"]

for sig in attrib_managers_sigs:
  count = 0
  cur_base = base
  cur = idc.find_binary(cur_base, SEARCH_DOWN | SEARCH_UP | SEARCH_NEXT, sig, 16)
  if cur != idc.BADADDR and cur != 0xffffffff:
    #print("Attrib:" + hex(cur))
    for x in CodeRefsTo(cur, False):
      str_offset = idc.find_binary(x, SEARCH_UP, "68", 16)
      #print("String ASM offset: " + hex(str_offset));
      val = idc.get_operand_value(str_offset - 3, 0);
      if val <= 1024:
        val = idc.get_operand_value(str_offset, 0);

      #print("String: " + hex(val))
      sound_name = str(ida_bytes.get_strlit_contents(val, -1, ida_nalt.STRTYPE_C))
      if sound_name == "None" or sound_name.find(".") == -1:
        val = idc.get_operand_value(str_offset - 1, 0);
        sound_name = str(ida_bytes.get_strlit_contents(val, -1, ida_nalt.STRTYPE_C))
        if sound_name == "None" or sound_name.find(".") == -1:
          val = idc.get_operand_value(str_offset, 0);
          sound_name = str(ida_bytes.get_strlit_contents(val, -1, ida_nalt.STRTYPE_C))
          if sound_name == "None":
            #print("FAIL ASM Offset: " + hex(str_offset))
            #print("FAIL val: " + hex(val))
            continue

      # Remove IDA debug text.
      sound_name = sound_name.replace("b'","").replace('"','').replace("'","").replace(",","")
      sound_name = sound_name.strip()

      #print(sound_name + ": " + hex(str_offset))
      count = count + 1
      if count > 100:
        break

      function_name = "None"
      if sound_name == "Player.AmbientUnderWater":
        function_name = "CBasePlayer::SetPlayerUnderwater"
      elif sound_name == "Player.PickupWeapon":
        function_name = "C_BaseCombatWeapon::OnDataChanged"
      elif sound_name == "Vote.Cast.No":
        function_name = "C_VoteController::FireGameEvent"
      elif sound_name == "Hud.Warning":
        function_name = "CHudBuildingStatusContainer::OnTick"
      elif sound_name == "Game.Overtime":
        function_name = "CTFGameRules::HandleOvertimeBegin"
      elif sound_name == "MVM.PlayerUsedPowerup":
        function_name = "ClientModeTFNormal::FireGameEvent"
      elif sound_name == "BumperCar.Jump":
        function_name = "CTFGameMovement::CheckJumpButton"
      elif sound_name == "Announcer.MVM_Spy_Alert":
        function_name = "CTFHudMannVsMachineStatus::FireGameEvent"
      elif sound_name == "Vote.Created":
        function_name = "CHudVote::OnThink"
      elif sound_name == "Medigun.DrainCharge":
        function_name = "CTFDroppedWeapon::OnDataChanged"
      elif sound_name == "ArrowLight":
        function_name = "CTFCompoundBow::OnDataChanged"
      elif sound_name == "Weapon_SniperRailgun.NonScoped":
        function_name = "CTFFlareGun::ClientEffectsThink"
      elif sound_name == "WeaponMedigun_Vaccinator.Toggle":
        function_name = "CWeaponMedigun::OnDataChanged"
      elif sound_name == "music.mvm_class_select":
        function_name = "CTFClassMenu::Go"
      elif sound_name == "MVM_Tank.BulletImpact":
        function_name = "ImpactCallback"
      elif sound_name == "WaterExplosionEffect.Sound":
        function_name = "C_WaterExplosionEffect::PlaySound"
      elif sound_name == "Building_Speedpad.BoostStop":
        function_name = "CTFPlayerShared::OnRemoveSpeedBoost"
      elif sound_name == "Halloween.dance_loop":
        function_name = "CTFPlayerShared::OnAddHalloweenThriller"
      elif sound_name == "Bombinomicon.Explode":
        function_name = "C_TFRagdoll::CreateTFGibs"
      elif sound_name == "TFPlayer.MedicChargedDeath":
        function_name = "C_TFRagdoll::CreateTFRagdoll"
      elif sound_name == "TFPlayer.Decapitated":
        function_name = "C_TFRagdoll::OnDataChanged"
      elif sound_name == "Parachute_close":
        function_name = "CTFPlayerShared::OnRemoveCondParachute"
      elif sound_name == "Parachute_open":
        function_name = "CTFPlayerShared::OnAddCondParachute"
      elif sound_name == "DemoCharge.ChargeCritOff":
        function_name = "CTFPlayerShared::OnRemoveDemoCharge"

      #Do not continue if the function name is None
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


print("Finished")