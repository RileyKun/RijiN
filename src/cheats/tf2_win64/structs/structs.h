#pragma once

struct s_input{
  union{
    DATA(0x108, c_user_cmd* commands); // sizeof c_user_cmd: 0x50
  };
};

struct s_animation_state{
  PAD(0x8);
  bool force_aim_yaw;
  PAD(0x8C);
  float yaw; // 0x8C
  float pitch; // 0x90
  float goal_feet_yaw; // 0x94
  float current_feet_yaw; // 0x98
};

enum classids{
  CTFWearableRazorback = 341,
  CTFWearableDemoShield = 338,
  CTFWearableLevelableItem = 340,
  CTFWearableCampaignItem = 337,
  CTFBaseRocket = 185,
  CTFWeaponBaseMerasmusGrenade = 325,
  CTFWeaponBaseMelee = 324,
  CTFWeaponBaseGun = 323,
  CTFWeaponBaseGrenadeProj = 322,
  CTFWeaponBase = 321,
  CTFWearableRobotArm = 342,
  CTFRobotArm = 287,
  CTFWrench = 344,
  CTFProjectile_ThrowableBreadMonster = 279,
  CTFProjectile_ThrowableBrick = 280,
  CTFProjectile_ThrowableRepel = 281,
  CTFProjectile_Throwable = 278,
  CTFThrowable = 319,
  CTFSyringeGun = 315,
  CTFKatana = 225,
  CTFSword = 314,
  CSniperDot = 118,
  CTFSniperRifleClassic = 308,
  CTFSniperRifleDecap = 309,
  CTFSniperRifle = 307,
  CTFChargedSMG = 197,
  CTFSMG = 306,
  CTFSlap = 305,
  CTFShovel = 304,
  CTFShotgunBuildingRescue = 303,
  CTFPEPBrawlerBlaster = 241,
  CTFSodaPopper = 310,
  CTFShotgun_Revenge = 301,
  CTFScatterGun = 297,
  CTFShotgun_Pyro = 300,
  CTFShotgun_HWG = 299,
  CTFShotgun_Soldier = 302,
  CTFShotgun = 298,
  CTFRocketPack = 296,
  CTFCrossbow = 201,
  CTFRocketLauncher_Mortar = 295,
  CTFRocketLauncher_AirStrike = 293,
  CTFRocketLauncher_DirectHit = 294,
  CTFRocketLauncher = 292,
  CTFRevolver = 286,
  CTFDRGPomson = 202,
  CTFRaygun = 284,
  CTFPistol_ScoutSecondary = 246,
  CTFPistol_ScoutPrimary = 245,
  CTFPistol_Scout = 244,
  CTFPistol = 243,
  CTFPipebombLauncher = 242,
  CTFWeaponPDA_Spy = 332,
  CTFWeaponPDA_Engineer_Destroy = 331,
  CTFWeaponPDA_Engineer_Build = 330,
  CTFWeaponPDAExpansion_Teleporter = 334,
  CTFWeaponPDAExpansion_Dispenser = 333,
  CTFWeaponPDA = 329,
  CTFParticleCannon = 239,
  CTFParachute_Secondary = 238,
  CTFParachute_Primary = 237,
  CTFParachute = 236,
  CTFMinigun = 234,
  CTFMedigunShield = 231,
  CWeaponMedigun = 352,
  CTFProjectile_MechanicalArmOrb = 263,
  CTFMechanicalArm = 230,
  CTFLunchBox_Drink = 229,
  CTFLunchBox = 228,
  CLaserDot = 78,
  CTFLaserPointer = 227,
  CTFKnife = 226,
  CTFGasManager = 212,
  CTFProjectile_JarGas = 261,
  CTFJarGas = 223,
  CTFProjectile_Cleaver = 254,
  CTFProjectile_JarMilk = 262,
  CTFProjectile_Jar = 260,
  CTFCleaver = 198,
  CTFJarMilk = 224,
  CTFJar = 222,
  CTFWeaponInvis = 328,
  CTFCannon = 196,
  CTFGrenadeLauncher = 216,
  CTFGrenadePipebombProjectile = 217,
  CTFGrapplingHook = 215,
  CTFFlareGun_Revenge = 210,
  CTFFlareGun = 209,
  CTFFlameRocket = 207,
  CTFFlameThrower = 208,
  CTFFists = 205,
  CTFFireAxe = 204,
  CTFWeaponFlameBall = 327,
  CTFCompoundBow = 200,
  CTFClub = 199,
  CTFBuffItem = 195,
  CTFStickBomb = 312,
  CTFBreakableSign = 194,
  CTFBottle = 192,
  CTFBreakableMelee = 193,
  CTFBonesaw = 190,
  CTFBall_Ornament = 182,
  CTFStunBall = 313,
  CTFBat_Giftwrap = 188,
  CTFBat_Wood = 189,
  CTFBat_Fish = 187,
  CTFBat = 186,
  CTFProjectile_EnergyRing = 256,
  CTFDroppedWeapon = 203,
  CTFWeaponSapper = 335,
  CTFWeaponBuilder = 326,
  CTFProjectile_Rocket = 264,
  CTFProjectile_Flare = 257,
  CTFProjectile_EnergyBall = 255,
  CTFProjectile_GrapplingHook = 258,
  CTFProjectile_HealingBolt = 259,
  CTFProjectile_Arrow = 252,
  CMannVsMachineStats = 80,
  CTFTankBoss = 316,
  CTFBaseBoss = 183,
  NextBotCombatCharacter = 357,
  CTFProjectile_SpellKartBats = 268,
  CTFProjectile_SpellKartOrb = 269,
  CTFHellZap = 220,
  CTFProjectile_SpellLightningOrb = 270,
  CTFProjectile_SpellTransposeTeleport = 277,
  CTFProjectile_SpellMeteorShower = 271,
  CTFProjectile_SpellSpawnBoss = 274,
  CTFProjectile_SpellMirv = 272,
  CTFProjectile_SpellPumpkin = 273,
  CTFProjectile_SpellSpawnHorde = 275,
  CTFProjectile_SpellSpawnZombie = 276,
  CTFProjectile_SpellBats = 266,
  CTFProjectile_SpellFireball = 267,
  CTFSpellBook = 311,
  CHightower_TeleportVortex = 74,
  CTeleportVortex = 160,
  CZombie = 354,
  CMerasmusDancer = 83,
  CMerasmus = 82,
  CHeadlessHatman = 73,
  CEyeballBoss = 48,
  CTFBotHintEngineerNest = 191,
  CPasstimeGun = 94,
  CTFViewModel = 320,
  CRobotDispenser = 112,
  CTFRobotDestruction_Robot = 288,
  CTFReviveMarker = 285,
  CTFPumpkinBomb = 282,
  CTFProjectile_BallOfFire = 253,
  CTFBaseProjectile = 184,
  CTFPointManager = 250,
  CBaseObjectUpgrade = 11,
  CTFRobotDestructionLogic = 291,
  CTFRobotDestruction_RobotGroup = 289,
  CTFRobotDestruction_RobotSpawn = 290,
  CTFPlayerDestructionLogic = 248,
  CPlayerDestructionDispenser = 101,
  CTFMinigameLogic = 233,
  CTFHalloweenMinigame_FallingPlatforms = 219,
  CTFHalloweenMinigame = 218,
  CTFMiniGame = 232,
  CTFPowerupBottle = 251,
  CTFItem = 221,
  CHalloweenSoulPack = 71,
  CTFGenericBomb = 213,
  CBonusRoundLogic = 23,
  CTFGameRulesProxy = 211,
  CTETFParticleEffect = 179,
  CTETFExplosion = 178,
  CTETFBlood = 177,
  CTFFlameManager = 206,
  CHalloweenGiftPickup = 69,
  CBonusDuckPickup = 21,
  CHalloweenPickup = 70,
  CCaptureFlagReturnIcon = 27,
  CCaptureFlag = 26,
  CBonusPack = 22,
  CTFTeam = 318,
  CTFTauntProp = 317,
  CTFPlayerResource = 249,
  CTFPlayer = 247,
  CTFRagdoll = 283,
  CTEPlayerAnimEvent = 165,
  CTFPasstimeLogic = 240,
  CPasstimeBall = 93,
  CTFObjectiveResource = 235,
  CTFGlow = 214,
  CTEFireBullets = 152,
  CTFAmmoPack = 181,
  CObjectTeleporter = 89,
  CObjectSentrygun = 88,
  CTFProjectile_SentryRocket = 265,
  CObjectSapper = 87,
  CObjectCartDispenser = 85,
  CObjectDispenser = 86,
  CMonsterResource = 84,
  CFuncRespawnRoomVisualizer = 64,
  CFuncRespawnRoom = 63,
  CFuncPasstimeGoal = 61,
  CFuncForceField = 57,
  CCaptureZone = 28,
  CCurrencyPack = 31,
  CBaseObject = 10,
  CTestTraceline = 176,
  CTEWorldDecal = 180,
  CTESpriteSpray = 174,
  CTESprite = 173,
  CTESparks = 172,
  CTESmoke = 171,
  CTEShowLine = 169,
  CTEProjectedDecal = 167,
  CTEPlayerDecal = 166,
  CTEPhysicsProp = 164,
  CTEParticleSystem = 163,
  CTEMuzzleFlash = 162,
  CTELargeFunnel = 159,
  CTEKillPlayerAttachments = 158,
  CTEImpact = 157,
  CTEGlowSprite = 156,
  CTEShatterSurface = 168,
  CTEFootprintDecal = 154,
  CTEFizz = 153,
  CTEExplosion = 151,
  CTEEnergySplash = 150,
  CTEEffectDispatch = 149,
  CTEDynamicLight = 148,
  CTEDecal = 146,
  CTEClientProjectile = 145,
  CTEBubbleTrail = 144,
  CTEBubbles = 143,
  CTEBSPDecal = 142,
  CTEBreakModel = 141,
  CTEBloodStream = 140,
  CTEBloodSprite = 139,
  CTEBeamSpline = 138,
  CTEBeamRingPoint = 137,
  CTEBeamRing = 136,
  CTEBeamPoints = 135,
  CTEBeamLaser = 134,
  CTEBeamFollow = 133,
  CTEBeamEnts = 132,
  CTEBeamEntPoint = 131,
  CTEBaseBeam = 130,
  CTEArmorRicochet = 129,
  CTEMetalSparks = 161,
  CSteamJet = 123,
  CSmokeStack = 117,
  DustTrail = 355,
  CFireTrail = 50,
  SporeTrail = 362,
  SporeExplosion = 361,
  RocketTrail = 359,
  SmokeTrail = 360,
  CPropVehicleDriveable = 108,
  ParticleSmokeGrenade = 358,
  CParticleFire = 90,
  MovieExplosion = 356,
  CTEGaussExplosion = 155,
  CEnvQuadraticBeam = 43,
  CEmbers = 36,
  CEnvWind = 47,
  CPrecipitation = 107,
  CBaseTempEntity = 17,
  CWeaponIFMSteadyCam = 351,
  CWeaponIFMBaseCamera = 350,
  CWeaponIFMBase = 349,
  CTFWearableVM = 343,
  CTFWearable = 336,
  CTFWearableItem = 339,
  CEconWearable = 35,
  CBaseAttributableItem = 3,
  CEconEntity = 34,
  CHandleTest = 72,
  CTeamplayRoundBasedRulesProxy = 126,
  CTeamRoundTimer = 127,
  CSpriteTrail = 122,
  CSpriteOriented = 121,
  CSprite = 120,
  CRagdollPropAttached = 111,
  CRagdollProp = 110,
  CPoseController = 106,
  CGameRulesProxy = 68,
  CInfoLadderDismount = 75,
  CFuncLadder = 58,
  CEnvDetailController = 40,
  CWorld = 353,
  CWaterLODControl = 348,
  CWaterBullet = 347,
  CVoteController = 346,
  CVGuiScreen = 345,
  CTest_ProxyToggle_Networkable = 175,
  CTesla = 170,
  CTeamTrainWatcher = 128,
  CBaseTeamObjectiveResource = 16,
  CTeam = 125,
  CSun = 124,
  CParticlePerformanceMonitor = 91,
  CSpotlightEnd = 119,
  CSlideshowDisplay = 116,
  CShadowControl = 115,
  CSceneEntity = 114,
  CRopeKeyframe = 113,
  CRagdollManager = 109,
  CPhysicsPropMultiplayer = 98,
  CPhysBoxMultiplayer = 96,
  CBasePropDoor = 15,
  CDynamicProp = 33,
  CPointWorldText = 105,
  CPointCommentaryNode = 104,
  CPointCamera = 103,
  CPlayerResource = 102,
  CPlasma = 100,
  CPhysMagnet = 99,
  CPhysicsProp = 97,
  CPhysBox = 95,
  CParticleSystem = 92,
  CMaterialModifyControl = 81,
  CLightGlow = 79,
  CInfoOverlayAccessor = 77,
  CFuncTrackTrain = 67,
  CFuncSmokeVolume = 66,
  CFuncRotating = 65,
  CFuncReflectiveGlass = 62,
  CFuncOccluder = 60,
  CFuncMonitor = 59,
  CFunc_LOD = 54,
  CTEDust = 147,
  CFunc_Dust = 53,
  CFuncConveyor = 56,
  CBreakableSurface = 25,
  CFuncAreaPortalWindow = 55,
  CFish = 51,
  CEntityFlame = 38,
  CFireSmoke = 49,
  CEnvTonemapController = 46,
  CEnvScreenEffect = 44,
  CEnvScreenOverlay = 45,
  CEnvProjectedTexture = 42,
  CEnvParticleScript = 41,
  CFogController = 52,
  CEntityParticleTrail = 39,
  CEntityDissolve = 37,
  CDynamicLight = 32,
  CColorCorrectionVolume = 30,
  CColorCorrection = 29,
  CBreakableProp = 24,
  CBasePlayer = 13,
  CBaseFlex = 8,
  CBaseEntity = 7,
  CBaseDoor = 6,
  CBaseCombatCharacter = 4,
  CBaseAnimatingOverlay = 2,
  CBoneFollower = 20,
  CBaseAnimating = 1,
  CInfoLightingRelative = 76,
  CBeam = 19,
  CBaseViewModel = 18,
  CBaseProjectile = 14,
  CBaseParticleEntity = 12,
  CBaseGrenade = 9,
  CBaseCombatWeapon = 5
};

struct s_shared{
  union{
    // Can be found by using the string "Player.Spy_Disguise" in client.dll
    DATA(0x1B8, void* outer);
  };
};

class ctf_weapon_info{
public:
  union{
    DATA(0x728, i32 damage);
    DATA(0x72C, i32 bullets_per_shot);
    DATA(0x730, float range);
    DATA(0x734, float spread);
    DATA(0x738, float punch_angle);
    DATA(0x73C, float time_fire_delay);
    DATA(0x760, float smack_delay);
    DATA(0x764, bool  use_rapid_fire_crits);
  };
};

class c_score_data;
class c_achievement_mgr;
class ctf_party;

class ctf_ragdoll{
public:

  bool& electrocuted(){
    gen_read(bool, "DT_TFRagdoll_m_bElectrocuted");
  }

  bool& become_ash(){
    gen_read(bool, "DT_TFRagdoll_m_bBecomeAsh");
  }

  bool& damage_custom(){
    gen_read(bool, "DT_TFRagdoll_m_iDamageCustom");
  }

  i32& team(){
    gen_read(i32, "DT_TFRagdoll_m_iTeam");
  }

  bool& gold_ragdoll(){
    gen_read(bool, "DT_TFRagdoll_m_bGoldRagdoll");
  }

  bool& ice_ragdoll(){
    gen_read(bool, "DT_TFRagdoll_m_bIceRagdoll");
  }

  bool& dissolving(){
    gen_read_offset(bool, "DT_TFRagdoll_m_bFeignDeath", -1);
  }

  bool& burning(){
    gen_read(bool, "DT_TFRagdoll_m_bBurning");
  }
};

class c_score_data{
public:
  i32& damage_done(){
    gen_read(i32, "DT_SPlayerScoringDataExclusive_m_iDamageDone");
  }
};

class ctf_party{
public:
  union{
    DATA(0x28, u32 leaderid);
    DATA(0x38, u32 member_count);
  };

  ALWAYSINLINE u32 get_member_id(u32 i){
    if(i >= member_count)
      return 0;

    void* list = *(void**)((uptr)this + 0x30) + (0x8 * i);
    return *(u32*)list;
  }
};

struct s_draw_model_info{
  union{
    DATA(0x10, void* decals);
    DATA(0x28, c_client_renderable* client_entity); // renderable
    DATA(0x30, i32 lod);
  };
};

class c_game_rules{
public:

  bool& truce_active(){
    gen_read(bool, "DT_TFGameRules_m_bTruceActive");
  }

  bool& playing_mvm(){
    gen_read(bool, "DT_TFGameRules_m_bPlayingMannVsMachine");
  }

  bool& playing_mannpower(){
    gen_read(bool, "DT_TFGameRules_m_bPowerupMode");
  }

  bool& in_setup(){
    gen_read(bool, "DT_TeamplayRoundBasedRules_m_bInSetup");
  }

  bool& show_match_summary(){
    gen_read(bool, "DT_TFGameRules_m_bShowMatchSummary");
  }

  bool& medieval(){
    gen_read(bool, "DT_TFGameRules_m_bPlayingMedieval");
  }

  bool* is_player_ready(){
    gen_read_pointer(bool*, "DT_TeamplayRoundBasedRules_m_bPlayerReady");
  }
};