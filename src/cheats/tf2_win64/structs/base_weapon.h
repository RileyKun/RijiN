#pragma once

class c_base_weapon : public c_internal_base_weapon, public c_base_entity {
public:
  ALWAYSINLINE bool reload_button_pressed(){
    gen_read_offset(bool, "DT_WeaponMedigun_m_nChargeResistType", 0xB);
  }

  ALWAYSINLINE bool in_reload(){
    gen_read_offset(bool, "DT_LocalActiveWeaponData_m_flNextPrimaryAttack", 0xC);
  }

  ALWAYSINLINE void* get_attribute_manager(){
    gen_read_pointer(void*, "DT_EconEntity_m_AttributeManager");
  }

  ALWAYSINLINE void* get_attribute_item(){
    gen_read_pointer_other(get_attribute_manager(), void*, "DT_AttributeContainer_m_Item");
  }

  ALWAYSINLINE u32 weapon_id(){
    gen_read_other(get_attribute_item(), u32, "DT_ScriptCreatedItem_m_iItemDefinitionIndex");
  }

  ALWAYSINLINE float& smack_time(){
    gen_read(float, "smack_time");
  }

  ALWAYSINLINE bool& current_attack_is_crit(){
    gen_read(bool, "current_attack_is_crit");
  }

  ALWAYSINLINE bool& current_crit_is_random(){
    gen_read(bool, "current_crit_is_random");
  }

  ALWAYSINLINE float& crit_token_bucket(){
    gen_read(float, "crit_token_bucket");
  }

  ALWAYSINLINE i32& crit_checks(){
    gen_read(i32, "crit_checks");
  }

  ALWAYSINLINE i32& crit_seed_requests(){
    gen_read(i32, "crit_seed_requests");
  }

  ALWAYSINLINE float& crit_time(){
    gen_read(float, "crit_time");
  }

  ALWAYSINLINE i32& last_crit_check_frame(){
    gen_read(i32, "last_crit_check_frame");
  }

  ALWAYSINLINE i32& current_seed(){
    gen_read(i32, "current_seed");
  }

  ALWAYSINLINE float& last_rapid_fire_crit_check_time(){
    gen_read(float, "last_rapid_fire_crit_check_time");
  }

  ALWAYSINLINE i32& weapon_mode(){
    gen_read(i32, "weapon_mode");
  }

  ALWAYSINLINE i32 heal_target_handle(){
    gen_read(i32, "DT_WeaponMedigun_m_hHealingTarget");
  }

  ALWAYSINLINE i32& charge_resist_type(){
    gen_read(i32, "DT_WeaponMedigun_m_nChargeResistType");
  }

  ALWAYSINLINE bool& medigun_attacking(){
    gen_read(bool, "DT_WeaponMedigun_m_bAttacking");
  }

  ALWAYSINLINE bool& charge_released(){
    gen_read(bool, "DT_WeaponMedigun_m_bChargeRelease");
  }

  ALWAYSINLINE float& uber_charge_level(){
    gen_read(float, "DT_TFWeaponMedigunDataNonLocal_m_flChargeLevel");
  }

  ALWAYSINLINE i32& weapon_state(){
    gen_read(i32, "DT_WeaponMinigun_m_iWeaponState");
  }

  ALWAYSINLINE bool& minigun_crit_shot(){
    gen_read(bool, "DT_WeaponMinigun_m_bCritShot");
  }

  ALWAYSINLINE float& charge_begin_time(){
    gen_read(float, "DT_PipebombLauncherLocalData_m_flChargeBeginTime");
  }

  ALWAYSINLINE float& detonate_time(){
    gen_read(float, "DT_WeaponGrenadeLauncher_m_flDetonateTime");
  }

  ALWAYSINLINE i32 reload_mode(){
    gen_read(i32, "DT_TFWeaponBase_m_iReloadMode");
  }

  ALWAYSINLINE float charged_damage(){
    gen_read(float, "DT_SniperRifleLocalData_m_flChargedDamage");
  }

  ALWAYSINLINE float& observed_crit_chance(){
    gen_read(float, "DT_LocalTFWeaponData_m_flObservedCritChance");
  }

  ALWAYSINLINE float& last_crit_check_time(){
    gen_read(float, "DT_LocalTFWeaponData_m_flLastCritCheckTime");
  }

  ALWAYSINLINE float charge_begin_time_pipebomb(){
    gen_read(float, "DT_PipebombLauncherLocalData_m_flChargeBeginTime");
  }

  ALWAYSINLINE i32 pipebomb_count(){
    gen_read(i32, "DT_PipebombLauncherLocalData_m_iPipebombCount");
  }

  ALWAYSINLINE float last_fire_time(){
    gen_read(float, "DT_LocalTFWeaponData_m_flLastFireTime");
  }

  ALWAYSINLINE float cow_mangler_charge_begin_time(){
    gen_read(float, "DT_ParticleCannon_m_flChargeBeginTime");
  }
  
  NEVERINLINE bool reloading(){
    return !is_melee() && (in_reload() || reload_mode() != 0);
  }

  ALWAYSINLINE bool is_melee(){
    return get_slot() == 2;
  }

  // Find inside of CTFWeaponBaseGun::FireBullet x-ref FX_FireBullets
  NEVERINLINE float get_spread(){
    if(get_slot() >= 2 || is_medigun() || is_sapper() || is_harmless_item())
      return 0.f;

    create_offset_hash("get_spread");
    return utils::internal_virtual_call<float>(offset, this);
  }

  ALWAYSINLINE bool swing_trace(s_trace* trace){
    create_offset_hash("swing_trace");
    return utils::internal_virtual_call<bool, s_trace*>(offset, this, trace);
  }

  ALWAYSINLINE bool is_medigun(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CWeaponMedigun;
  }

  ALWAYSINLINE bool is_kritzkrieg(){
    return weapon_id() == WPN_Kritzkrieg;
  }

  ALWAYSINLINE bool is_quickfix(){
    return weapon_id() == WPN_QuickFix;
  }

  ALWAYSINLINE bool is_knife(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFKnife;
  }

  ALWAYSINLINE bool is_diamond_back(){
    return this->weapon_id() == WPN_Diamondback;
  }

  ALWAYSINLINE bool is_crossbow(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFCrossbow;
  }

  ALWAYSINLINE bool is_huntsman(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFCompoundBow;
  }

  ALWAYSINLINE bool is_sticky_launcher(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFPipebombLauncher;
  }

  NEVERINLINE bool is_shotgun(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    switch(cc->id){
      default: break;
      case CTFShotgun_Revenge:
      case CTFShotgun_Pyro:
      case CTFShotgun_HWG:
      case CTFShotgun_Soldier:
      case CTFShotgun:
        return true;
    }

    return false;
  }

  ALWAYSINLINE bool is_scattergun(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    switch(cc->id){
      default: break;
      case CTFSodaPopper:
      case CTFPEPBrawlerBlaster:
      case CTFScatterGun:
        return true;
    }

    return false;
  }

  ALWAYSINLINE bool is_minigun(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFMinigun;
  }

  ALWAYSINLINE bool is_wrench(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFWrench || cc->id == CTFRobotArm;
  }

  ALWAYSINLINE bool is_flamethrower(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFFlameThrower;
  }

  ALWAYSINLINE bool is_sapper(){
    return get_id() == TF_WEAPON_BUILDER;
  }

  NEVERINLINE bool is_sniper_rifle(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    //if(weapon_id == WPN_SydneySleeper)
      //return false;

    switch(cc->id){
      default: break;
      case CTFSniperRifleClassic:
      case CTFSniperRifleDecap:
      case CTFSniperRifle:
        return true;
    }

    return false;
  }

  ALWAYSINLINE bool is_hitman_heatmaker(){
    return weapon_id() == WPN_HitmanHeatmaker;
  }

  ALWAYSINLINE bool is_classic_sniper_rifle(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    return cc->id == CTFSniperRifleClassic;
  }
  // These weapons do no damage.
  NEVERINLINE bool is_harmless_item(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    switch(cc->id){
      default: break;
      case CTFGrapplingHook:
      case CTFWeaponBuilder:
      case CTFLunchBox:
      case CTFLunchBox_Drink:
      case CTFWeaponSapper:
      case CTFWeaponPDA:
      case CTFWeaponPDA_Spy:
      case CTFWeaponPDA_Engineer_Build:
      case CTFWeaponPDA_Engineer_Destroy:
      case CTFWeaponPDAExpansion_Dispenser:
      case CTFWeaponPDAExpansion_Teleporter:
      case CTFRocketPack:
      case CTFBuffItem:
      case CTFJar:
      case CTFJarGas:
      case CTFJarMilk:
        return true;
    }
    return false;
  }

  ALWAYSINLINE bool is_soldier_whip(){
    return weapon_id() == WPN_SoldierWhip;
  }

  ALWAYSINLINE bool is_holiday_punch(){
    return weapon_id() == WPN_HolidayPunch;
  }

  ALWAYSINLINE bool is_ubersaw(){
    return weapon_id() == WPN_Ubersaw || weapon_id() == WPN_FestiveUbersaw;
  }

  ALWAYSINLINE bool is_sandman(){
    return weapon_id() == WPN_Sandman;
  }

  ALWAYSINLINE bool is_wrap_assassin(){
    return weapon_id() == WPN_WrapAssassin;
  }

  ALWAYSINLINE bool is_force_a_nature(){
    return weapon_id() == WPN_FAN || weapon_id() == WPN_FestiveFAN;
  }

  ALWAYSINLINE bool is_ball_bat(){
    return is_sandman() || is_wrap_assassin();
  }

  NEVERINLINE bool deals_bullet_dmg(){
    return !is_harmless_item() && !is_medigun() && !is_wrangler() && !deals_blast_dmg() && !deals_fire_dmg() && !is_melee();
  }

  NEVERINLINE bool deals_blast_dmg(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    switch(cc->id){
      default: return false;
      case CTFRocketLauncher:
      case CTFRocketLauncher_Mortar:
      case CTFRocketLauncher_AirStrike:
      case CTFRocketLauncher_DirectHit:
      case CTFPipebombLauncher:
      case CTFGrenadeLauncher:
      case CTFParticleCannon:
      case CTFCannon:
        return true;
    }
    return false;
  }

  NEVERINLINE bool deals_fire_dmg(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    switch(cc->id){
      default: return false;
      case CTFFlareGun_Revenge:
      case CTFFlareGun:
      case CTFWeaponFlameBall:
      case CTFFlameThrower:
      case CTFJarGas:
        return true;
    }
    return false;
  }

  NEVERINLINE bool is_projectile_weapon(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    switch(cc->id){
      default: return false;
      case CTFRocketLauncher:
      case CTFRocketLauncher_Mortar:
      case CTFRocketLauncher_AirStrike:
      case CTFRocketLauncher_DirectHit:
      case CTFPipebombLauncher:
      case CTFGrenadeLauncher:
      case CTFParticleCannon:
      case CTFCannon:
      case CTFFlareGun_Revenge:
      case CTFFlareGun:
      case CTFWeaponFlameBall:
      case CTFCompoundBow:
      case CTFCrossbow:
      case CTFDRGPomson:
      case CTFRaygun:
      case CTFShotgunBuildingRescue:
      case CTFJarGas:
      case CTFJarMilk:
      case CTFJar:
      case CTFCleaver:
      case CTFSyringeGun:
      case CTFFlameThrower:
        return true;
    }

    return false;
  }

  ALWAYSINLINE bool is_pomson(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    return cc->id == CTFDRGPomson;
  }

  NEVERINLINE bool is_hitscan_weapon(){
    if(is_projectile_weapon() || is_flamethrower() || is_harmless_item() || is_medigun() || is_melee() || is_wrangler())
      return false;

    return true;
  }

  ALWAYSINLINE bool is_rocket_launcher(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    switch(cc->id){
      default: return false;
      case CTFRocketLauncher:
      case CTFRocketLauncher_Mortar:
      case CTFRocketLauncher_AirStrike:
      case CTFRocketLauncher_DirectHit:
      case CTFParticleCannon:
        return true;
    }

    return false;
  }

  ALWAYSINLINE bool is_direct_hit(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    return cc->id == CTFRocketLauncher_DirectHit;
  }

  NEVERINLINE bool is_beggars(){
    if(!is_rocket_launcher())
      return false;

    return utils::attrib_hook_int(0, XOR("can_overload"), this, 0, 1) != 0;
  }

  ALWAYSINLINE bool is_blast_jumper_weapon(){
    switch(weapon_id()) {
      default: return false;
      case WPN_RocketJump:
      case WPN_StickyJumper:
        return true;
    }

    return false;
  }

  NEVERINLINE bool can_randomly_crit(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    if(is_blast_jumper_weapon() || is_cleaver() || is_knife() || is_wrangler() || is_harmless_item() || is_huntsman() || is_medigun() || is_sniper_rifle() || is_huntsman())
      return false;

    switch(weapon_id()) {
      default: break;
      case WPN_FrontierJustice:
      case WPN_Ambassador:
      case WPN_Phlogistinator:
      case WPN_BackScatter:
      case WPN_CowMangler:
      case WPN_MarketGardener:
      case WPN_DragonFury:
      case WPN_ManMelter:
      case WPN_Sword:
      case WPN_FestiveEyeLander:
      case WPN_Claidheamhmor:
      case WPN_PersainPersuader:
      case WPN_Golfclub:
      case WPN_Headless:
      case WPN_Katana:
      case WPN_Ullapool:
      case WPN_ShortCircut:
      case WPN_Gunslinger:
      case WPN_Enforcer:
      case WPN_Diamondback:
      case WPN_Bushwacka:
      case WPN_CleanersCarbine:
      case WPN_Axtingusher:
      case WPN_FestiveAxtwingisher:
      case WPN_NeonAnnihilator1:
      case WPN_NeonAnnihilator2:
      case WPN_Mailbox:
      case WPN_Backburner:
        return false;
    }

    return true;
  }

  NEVERINLINE bool can_weapon_repair_objects(bool sapped = false){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr)
      return false;
    }

    if(cc->id == CTFWrench || cc->id == CTFRobotArm || weapon_id() == WPN_RescueRanger)
      return true;
    else{
      switch(weapon_id()) {
        default: break;
        case WPN_HomeWrecker:
        case WPN_Maul:
        case WPN_NeonAnnihilator1:
        case WPN_NeonAnnihilator2:
          return sapped;
      }
    }

    return false;
  }

  NEVERINLINE bool is_able_to_dt(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    if(is_phlog() || is_cleaver() || is_medigun() || is_sticky_launcher() || is_wrangler() || is_huntsman() || is_sniper_rifle() || is_classic_sniper_rifle() || is_harmless_item())
      return false;

    switch(cc->id){
      default: break;
      case CTFFlareGun:
      case CTFFlareGun_Revenge:
      case CTFCrossbow:
        return false;
    }

    return weapon_id() != WPN_LoooseCannon;
  }

  ALWAYSINLINE bool is_flaregun(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFFlareGun || cc->id == CTFFlareGun_Revenge;
  }

  ALWAYSINLINE bool is_scorch_shot(){
    return this->weapon_id() == WPN_ScorchShot;
  }

  ALWAYSINLINE bool is_detonator(){
    return this->weapon_id() == WPN_Detonator;
  }

  ALWAYSINLINE bool is_market_gardener(){
    return this->weapon_id() == WPN_MarketGardener;
  }

  ALWAYSINLINE bool is_grenade_launcher(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFGrenadeLauncher;
  }

  ALWAYSINLINE bool is_loch_and_load(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFGrenadeLauncher && weapon_id() == WPN_LochNLoad;
  }

  ALWAYSINLINE bool is_iron_bomber(){
    return this->weapon_id() == WPN_IronBomber;
  }

  ALWAYSINLINE bool is_quickie_bomb_launcher(){
    return this->weapon_id() == WPN_QuickieBombLauncher;
  }

  ALWAYSINLINE bool is_loose_cannon(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFCannon && weapon_id() == WPN_LoooseCannon;
  }

  ALWAYSINLINE bool is_syringe_gun(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFSyringeGun;
  }

  ALWAYSINLINE bool is_rescue_ranger(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFShotgunBuildingRescue;
  }

  ALWAYSINLINE bool is_jarate_jar(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFJar;
  }

  ALWAYSINLINE bool is_mad_milk_jar(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFJarMilk;
  }

  ALWAYSINLINE bool is_gas_jar(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFJarGas;
  }

  ALWAYSINLINE bool is_cleaver(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFCleaver;
  }

  NEVERINLINE bool is_throwables(){
    return is_jarate_jar() || is_mad_milk_jar() || is_gas_jar() || is_cleaver();
  }

  ALWAYSINLINE bool is_dragons_fury(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFWeaponFlameBall;
  }

  ALWAYSINLINE bool is_base_jumper(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFParachute_Primary || cc->id == CTFParachute_Secondary;
  }

  ALWAYSINLINE bool is_sniper_smg(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFChargedSMG || cc->id == CTFSMG;
  }

  ALWAYSINLINE bool is_cow_mangler(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFParticleCannon;
  }

  ALWAYSINLINE bool is_builder_pda(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFWeaponBuilder;
  }

  ALWAYSINLINE bool is_third_degree(){
    return weapon_id() == WPN_ThirdDegree;
  }

  ALWAYSINLINE bool is_vaccinator(){
    return weapon_id() == WPN_Vaccinator;
  }

  ALWAYSINLINE bool is_man_melter(){
    return weapon_id() == WPN_ManMelter;
  }

  ALWAYSINLINE bool is_bison(){
    return weapon_id() == WPN_RighteousBison;
  }

  ALWAYSINLINE bool is_phlog(){
    return weapon_id() == WPN_Phlogistinator;
  }

  ALWAYSINLINE bool is_ambassador(){
    return weapon_id() == WPN_Ambassador || weapon_id() == WPN_FestiveAmbassador;
  }

  ALWAYSINLINE bool is_sydney_sleeper(){
    return weapon_id() == WPN_SydneySleeper;
  }

  ALWAYSINLINE bool is_short_circuit(){
    return weapon_id() == WPN_ShortCircut;
  }

  ALWAYSINLINE bool is_short_stop(){
    return weapon_id() == WPN_Shortstop;
  }

  ALWAYSINLINE bool is_fists(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    return cc->id == CTFFists;
  }

  ALWAYSINLINE bool is_wrangler(){
      switch(weapon_id()){
        default: break;
        case WPN_Wrangler:
        case WPN_FestiveWrangler:
        case WPN_GigarCounter:
          return true;
    }
    return false;
  }

  ALWAYSINLINE bool is_enforcer(){
    return weapon_id() == WPN_Enforcer;
  }

  NEVERINLINE bool is_splash_weapon(){
    if(get_blast_radius() <= 0.0f)
      return false;

    return !(is_grenade_launcher() || is_loose_cannon() || is_direct_hit() || is_loch_and_load() || is_detonator() || is_flaregun() && !is_scorch_shot());
  }

  ALWAYSINLINE bool can_headshot(){
    return is_ambassador() || is_sniper_rifle() && !is_sydney_sleeper();
  }

  ALWAYSINLINE bool is_release_weapon(){
    return is_huntsman() || is_sticky_launcher() || is_loose_cannon();
  }

  ALWAYSINLINE float get_sticky_launcher_max_charge(){
    return utils::attrib_hook_float(4.0f, XOR("stickybomb_charge_rate"), this, 0, 1);
  }

  ALWAYSINLINE float get_overheal_expert_mod(){
    float overheal_expert = (float)utils::attrib_hook_int(0, XOR("overheal_expert"), this, 0, 1);
    return overheal_expert > 0.f ? overheal_expert / 4.f : 0.f;
  }

  ALWAYSINLINE bool is_huo_long_heater(){
    if(!is_minigun())
      return false;

    return utils::attrib_hook_int(0, XOR("ring_of_fire_while_aiming"), this, 0, 1) != 0;
  }

  ALWAYSINLINE c_base_entity* get_entity(){
    return (c_base_entity*)this;
  }

  // Things like the sniper rifle technically reload. But not in the sense of what we're looking for.
  // Weapons like the SMG, Shotgun, Rocket launchers, and more. Weapons that have a clip and reserve ammo that you have to press the reload key.
  // To you know... Reload your weapon... This function will return true if it is one of those weapons.
  NEVERINLINE bool does_weapon_reload(){
    if(is_sniper_rifle() || is_medigun() || is_flaregun() || is_dragons_fury() || is_flamethrower() || is_minigun() || is_wrangler() || is_melee() || is_huntsman())
      return false;

    if(is_cleaver() || is_short_circuit() || is_harmless_item())
      return false;

    return true;
  }

  // Found inside "CTFWeaponBaseMelee::DoSwingTraceInternal" use the string "melee_bounds_multiplier" in client.dll
  float get_swing_range(bool apply_attrib = false);

  void  setup_random_seed(i32 seed = -1);
  float get_sticky_launcher_arm_time();
  bool  get_vphysics_velocity(vec3* vel, vec3 angles = vec3(), bool disable_random_impulse = false);
  float get_projectile_speed(vec3 angles = vec3());
  float get_projectile_gravity();
  bool  get_projectile_size(vec3& min, vec3& max);
  void  get_projectile_fire_setup(c_base_player* player, vec3 offset, vec3* shoot_pos, vec3* eye_angles);
  float get_loose_cannon_charge_begin_time();
  float get_loose_cannon_charge();
  
  bool get_weapon_offset(vec3& offset);
  float get_blast_radius(float creation_time = -1.f, bool touched = false);

  std::wstring get_weapon_name();
  c_base_player* get_heal_target();
  bool can_uber();
  ctf_weapon_info* get_weapon_info();
  float time_between_shots();
  bool is_perfect_shot();
  bool deals_minicritdmg_inair();
};