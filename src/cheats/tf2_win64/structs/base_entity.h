#pragma once
class c_entity_data;

class c_base_player;
class c_base_weapon;
class c_base_entity : public c_internal_base_player{
public:
  ALWAYSINLINE i32 object_health(){
    gen_read(i32, "DT_BaseObject_m_iHealth");
  }

  ALWAYSINLINE i32 object_max_health(){
    gen_read(i32, "DT_BaseObject_m_iMaxHealth");
  }

  ALWAYSINLINE bool sapped(){
    gen_read(bool, "DT_BaseObject_m_bHasSapper");
  }

  ALWAYSINLINE bool mini_building(){
    gen_read(bool, "DT_BaseObject_m_bMiniBuilding");
  }

  ALWAYSINLINE bool disposable_building(){
    gen_read(bool, "DT_BaseObject_m_bDisposableBuilding");
  }

  ALWAYSINLINE i32 upgrade_level(){
    gen_read(i32, "DT_BaseObject_m_iUpgradeLevel");
  }

  ALWAYSINLINE bool disabled(){
    gen_read(bool, "DT_BaseObject_m_bDisabled");
  }

  ALWAYSINLINE bool plasma_disabled(){
    gen_read(bool, "DT_BaseObject_m_bPlasmaDisable");
  }

  ALWAYSINLINE bool building(){
    gen_read(bool, "DT_BaseObject_m_bBuilding");
  }

  ALWAYSINLINE bool placing(){
    gen_read(bool, "DT_BaseObject_m_bPlacing");
  }

  ALWAYSINLINE i32 auto_aim_target_handle(){
    gen_read(i32, "DT_ObjectSentrygun_m_hAutoAimTarget");
  }

  ALWAYSINLINE i32 building_owner_handle(){
    gen_read(i32, "DT_BaseObject_m_hBuilder");
  }

  // Sentry gun
  ALWAYSINLINE bool player_controlled(){
    gen_read(bool, "DT_ObjectSentrygun_m_bPlayerControlled");
  }

  ALWAYSINLINE i32 shells(){
    gen_read(i32, "DT_ObjectSentrygun_m_iAmmoShells");
  }

  ALWAYSINLINE i32 max_shells(){
    gen_read_offset(i32, "DT_ObjectSentrygun_m_iAmmoShells", 0x4);
  }

  ALWAYSINLINE i32 rockets(){
    gen_read(i32, "DT_ObjectSentrygun_m_iAmmoRockets");
  }

  // teleporter
  ALWAYSINLINE i32 object_mode(){
    gen_read(i32, "DT_BaseObject_m_iObjectMode");
  }

  // capture flag

  ALWAYSINLINE i32 flag_status(){
    gen_read(i32, "DT_CaptureFlag_m_nFlagStatus");
  }

  ALWAYSINLINE i32 flag_type(){
    gen_read(i32, "DT_CaptureFlag_m_nType");
  }

  ALWAYSINLINE float flag_reset_time(){
    gen_read(float, "DT_CaptureFlag_m_flResetTime");
  }

  ALWAYSINLINE i32 pipebomb_type(){
    gen_read(i32, "DT_TFProjectile_Pipebomb_m_iType");
  }

  ALWAYSINLINE bool pipebomb_touched(){
    gen_read(bool, "DT_TFProjectile_Pipebomb_m_bTouched");
  }

  ALWAYSINLINE float damage_radius(){
    gen_read(float, "DT_BaseGrenade_m_DmgRadius");
  }

  ALWAYSINLINE bool rocket_critical(){
    gen_read(bool, "DT_TFProjectile_Rocket_m_bCritical");
  }

  ALWAYSINLINE bool flare_critical(){
    gen_read(bool, "DT_TFProjectile_Flare_m_bCritical");
  }

  ALWAYSINLINE bool arrow_critical(){
    gen_read(bool, "DT_TFProjectile_Arrow_m_bCritical");
  }

  ALWAYSINLINE bool demo_critical(){
    gen_read(bool, "DT_TFWeaponBaseGrenadeProj_m_bCritical");
  }

  ALWAYSINLINE bool energy_ball_charged(){
    gen_read(bool, "DT_TFProjectile_EnergyBall_m_bChargedShot");
  }

  // Arrow
  ALWAYSINLINE i32 arrow_type(){
    gen_read(i32, "DT_TFProjectile_Arrow_m_iProjectileType");
  }

  // Deflected.
  ALWAYSINLINE i32 rocket_deflected(){
    gen_read(i32, "DT_TFBaseRocket_m_iDeflected");
  }

  ALWAYSINLINE i32 grenade_deflected(){
    gen_read(i32, "DT_TFWeaponBaseGrenadeProj_m_iDeflected");
  }

  ALWAYSINLINE i32 passtime_ball_carrier(){
    gen_read(i32, "DT_PasstimeBall_m_hCarrier");
  }

  // owners for weapons.
  ALWAYSINLINE i32 pipebomb_launcher_handle(){
    gen_read(i32, "DT_TFProjectile_Pipebomb_m_hLauncher");
  }

  ALWAYSINLINE i32 rocket_launcher_handle(){
    gen_read(i32, "DT_TFBaseRocket_m_hLauncher");
  }

  ALWAYSINLINE float sticky_creation_time(){
    gen_read_offset(float, "DT_TFWeaponBaseGrenadeProj_m_bCritical", -0x4);
  }

  ALWAYSINLINE bool currency_distributed(){
    gen_read(bool, "DT_CurrencyPack_m_bDistributed");
  }

  ALWAYSINLINE bool currency_touched(){
    gen_read_offset(bool, "DT_CurrencyPack_m_bDistributed", -0x8);
  }

  // sniper laser dot.
  ALWAYSINLINE float dot_charge_start_time(){
    gen_read(float, "DT_SniperDot_m_flChargeStartTime");
  }

  ALWAYSINLINE i32 revive_marker_health(){
    gen_read(i32, "DT_TFReviveMarker_m_iHealth");
  }

  ALWAYSINLINE i32 revive_marker_max_health(){
    gen_read(i32, "DT_TFReviveMarker_m_iMaxHealth");
  }

  VIRTUAL_TABLE_RW_IMPLEMENTS()

  c_entity_data* entity_data();

  ALWAYSINLINE i32 get_health(){
    if(is_player()){
      return health();
    }
    else if(is_sentry() || is_dispenser() || is_teleporter())
      return this->object_health();

    return 0;
  }

  ALWAYSINLINE i32 max_health(){
    return utils::virtual_call64<107, i32>(this);
  }

  // RenderableToWorldTransform
  ALWAYSINLINE matrix3x4 obb_transform(){
    //entity_flags2 |= (1 << 11);
    //matrix3x4 ret = utils::virtual_call64<34, matrix3x4&>(renderable());
    return rgfl_coordinate_frame();
  }

  c_base_entity* first_move_child();
  c_base_entity* next_move_peer();

  NEVERINLINE bool is_projectile(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    switch(cc->id){
      case CTFProjectile_JarGas:
      case CTFProjectile_Cleaver:
      case CTFProjectile_JarMilk:
      case CTFProjectile_Jar:
      case CTFProjectile_MechanicalArmOrb:
      case CTFGrenadePipebombProjectile:
      case CTFProjectile_Rocket:
      case CTFProjectile_SentryRocket:
      case CTFProjectile_Flare:
      case CTFProjectile_EnergyBall:
      case CTFProjectile_SpellKartBats:
      case CTFProjectile_SpellKartOrb:
      case CTFProjectile_SpellLightningOrb:
      case CTFProjectile_SpellMeteorShower:
      case CTFProjectile_SpellMirv:
      case CTFProjectile_SpellPumpkin:
      case CTFProjectile_SpellSpawnHorde:
      case CTFProjectile_SpellSpawnZombie:
      case CTFProjectile_SpellSpawnBoss:
      case CTFProjectile_SpellBats:
      case CTFProjectile_SpellFireball:
      case CTFBall_Ornament:
      case CTFStunBall:
      case CTFProjectile_BallOfFire:
        return true;
      case CTFProjectile_Arrow:
      case CTFProjectile_HealingBolt:
        return is_arrow();
    }
    return false;
  }

  ALWAYSINLINE bool is_fire_spell(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    switch(cc->id){
      case CTFProjectile_SpellFireball:
      case CTFProjectile_SpellMeteorShower:
        return true;
    }

    return false;
  }

  ALWAYSINLINE std::wstring get_projectile_name(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return L"";
    }

    switch(cc->id){
      case CTFProjectile_JarGas:
        return WXOR(L"Gas");
      case CTFProjectile_Cleaver:
        return WXOR(L"Cleaver");
      case CTFProjectile_JarMilk:
        return WXOR(L"Mad Milk");
      case CTFProjectile_Jar:
        return WXOR(L"Jarate");
      case CTFProjectile_MechanicalArmOrb:
        return WXOR(L"Mechanical orb");
      case CTFGrenadePipebombProjectile:{
        switch(this->pipebomb_type()){
          default: return WXOR(L"Pipebomb");
          case 1:  return WXOR(L"Sticky");
          case 3:  return WXOR(L"Cannonball");
        }
        break;
      }
      case CTFProjectile_Rocket:
      case CTFProjectile_SentryRocket:
        return WXOR(L"Rocket");
      case CTFProjectile_Flare:
        return WXOR(L"Flare");
      case CTFProjectile_EnergyBall:
        return WXOR(L"Energy ball");
      case CTFProjectile_HealingBolt:
        return WXOR(L"Healing bolt");
      case CTFProjectile_Arrow:
        return this->arrow_type() == 18 ? WXOR(L"Rescue bolt") : WXOR(L"Arrow");
      case CTFProjectile_SpellKartBats:
        return WXOR(L"Kart bats");
      case CTFProjectile_SpellKartOrb:
        return WXOR(L"Kart orb");
      case CTFProjectile_SpellLightningOrb:
        return WXOR(L"Lightning orb");
      case CTFProjectile_SpellMeteorShower:
        return WXOR(L"Meteor shower");
      case CTFProjectile_SpellMirv:
        return WXOR(L"Mirv spell");
      case CTFProjectile_SpellPumpkin:
        return WXOR(L"Pumpkin spell");
      case CTFProjectile_SpellSpawnHorde:
      case CTFProjectile_SpellSpawnZombie:
      case CTFProjectile_SpellSpawnBoss:
        return WXOR(L"Boss Spawner spell");
      case CTFProjectile_SpellBats:
        return WXOR(L"Bat spell");
      case CTFProjectile_SpellFireball:
        return WXOR(L"Fireball spell");
      case CTFBall_Ornament:
      case CTFStunBall:
        return WXOR(L"Stun ball");
      case CTFProjectile_BallOfFire:
        return WXOR(L"Fire ball");
    }

    return L"";
  }

  ALWAYSINLINE bool is_healing_bolt(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFProjectile_HealingBolt;
  }

  ALWAYSINLINE bool is_throwables(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    switch(cc->id){
      case CTFProjectile_Jar:
      case CTFProjectile_JarMilk:
      case CTFProjectile_JarGas:
      case CTFProjectile_Cleaver:
      case CTFBall_Ornament:
      case CTFStunBall:
        return true;
    }

    return false;
  }

  // You need to check if the projectile is moving!
  ALWAYSINLINE bool is_projectile_deflectable(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    switch(cc->id){
      case CTFProjectile_Rocket:
      case CTFProjectile_SentryRocket:
      case CTFProjectile_HealingBolt:
      case CTFProjectile_EnergyBall:
      case CTFProjectile_MechanicalArmOrb:
      case CTFGrenadePipebombProjectile:
      case CTFProjectile_Jar:
      case CTFProjectile_Flare:
      case CTFProjectile_JarMilk:
      case CTFProjectile_JarGas:
      case CTFProjectile_Cleaver:
      case CTFBall_Ornament:
      case CTFStunBall:
      case CTFProjectile_BallOfFire:
        return true;
    }

    if(is_arrow())
      return true;

    return false;
  }

  ALWAYSINLINE bool can_destroy_projectiles(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFProjectile_MechanicalArmOrb;
  }

  ALWAYSINLINE bool is_projectile_critical(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    switch(cc->id){
      case CTFProjectile_Rocket:
        return this->rocket_critical();
      case CTFProjectile_Arrow:
      case CTFProjectile_HealingBolt:
        return this->arrow_critical();
      case CTFGrenadePipebombProjectile:
        return this->demo_critical();
      case CTFProjectile_Flare:
        return this->flare_critical();
      case CTFProjectile_EnergyBall:
        return this->energy_ball_charged();
    }

    return false;
  }

  ALWAYSINLINE bool is_arrow(bool bypass_velocity = false){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    switch(cc->id){
      case CTFProjectile_Arrow:
      case CTFProjectile_HealingBolt:
        return !bypass_velocity && est_velocity().length() > 1.f || bypass_velocity;
    }

    return false;
  }

  ALWAYSINLINE bool is_huntsman_arrow(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    switch(cc->id){
      case CTFProjectile_Arrow:
      return this->arrow_type() == 8;
    }
  }

  ALWAYSINLINE bool is_flare(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFProjectile_Flare;
  }

  ALWAYSINLINE bool is_rocket(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    switch(cc->id){
      case CTFProjectile_Rocket:
      case CTFProjectile_SentryRocket:
      case CTFProjectile_EnergyBall:
        return true;
    }

    return false;
  }

  ALWAYSINLINE bool is_demo_projectile(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFGrenadePipebombProjectile;
  }

  ALWAYSINLINE bool is_flame_ball(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFProjectile_BallOfFire;
  }

  ALWAYSINLINE bool is_stickybomb(){
    if(!is_demo_projectile())
      return false;

    return this->pipebomb_type() == 1;
  }

  ALWAYSINLINE bool is_sticky_onsurface(){
    if(!is_stickybomb())
      return false;

    return this->pipebomb_touched();
  }

  bool is_krampus();
  bool is_boss();

  ALWAYSINLINE bool is_tank_boss(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CTFTankBoss;
  }

  ALWAYSINLINE bool is_money(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CCurrencyPack;
  }

  ALWAYSINLINE bool is_player(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr)
      return false;

    return cc->id == CTFPlayer;
  }

  ALWAYSINLINE bool is_sentry(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CObjectSentrygun;
  }

  ALWAYSINLINE bool is_dispenser(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CObjectDispenser;
  }

  ALWAYSINLINE bool is_payload_dispenser(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CObjectCartDispenser;
  }

  ALWAYSINLINE bool is_teleporter(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CObjectTeleporter;
  }

  ALWAYSINLINE bool is_revive_marker(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr)
      return false;
    }

    return cc->id == CTFReviveMarker;
  }

  ALWAYSINLINE bool is_objective(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr)
      return false;
    }

    return cc->id == CPasstimeBall || cc->id == CCaptureFlag || cc->id == CBonusPack || cc->id == CBonusDuckPickup;
  }

  ALWAYSINLINE bool is_halloween_gift_pickup(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    return cc->id == CHalloweenGiftPickup;
  }

  ALWAYSINLINE bool is_halloween_pickup(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    return cc->id == CHalloweenPickup;
  }

  ALWAYSINLINE bool is_on_func_track(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr)
      return false;
    }

    return cc->id == CFuncTrackTrain;
  }

  ALWAYSINLINE bool is_deflected(){
    // It does work. But, they aren't mini-critted or dangerous.
    if(is_stickybomb())
      return false;

    if(is_rocket() || is_healing_bolt() || is_arrow() || is_flare())
      return rocket_deflected() > 0;
    else if(is_demo_projectile())
      return grenade_deflected() > 0;

    return false;
  }

  ALWAYSINLINE vec3 get_sentry_shoot_pos(){
    if(!is_sentry())
      return vec3();

    vec3 pos = origin();
    switch(this->upgrade_level()){
      default: break;
      case 1: return pos + vec3(0.f, 0.f, 32.f);
      case 2: return pos + vec3(0.f, 0.f, 40.f);
      case 3: return pos + vec3(0.f, 0.f, 46.f);
    }

    return pos;
  }

  ALWAYSINLINE bool is_door(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    return cc->id == CBasePropDoor || cc->id == CBaseDoor;
  }

  i8* get_class_name();
  c_base_player*  get_player();
  i32             get_spellbook_type();
  bool            is_bomb();
  bool            is_health_pack();
  bool            is_ammo_pack();
  bool            is_powerup();
  std::wstring    get_powerup_name();
  bool            is_lunchable();
  c_base_player*  get_sentry_target();
  c_base_player*  get_building_owner();
  i32             update_delta();
  vec3            est_velocity();
  c_base_weapon*  get_launcher();
  c_base_player*  get_passtime_carrier();
  bool            is_fake_projectile();
  bool            is_considered_enemy(c_base_entity* target);
  void            calc_nearest_point(vec3& src, vec3* vec_pos);
  bool            get_hitbox_pos(i32 hitbox, vec3* output_pos);
};