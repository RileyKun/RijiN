#include "../link.h"


std::wstring c_base_weapon::get_weapon_name(){
  switch(weapon_id()){
    case 1132:
    case 1069:
    case 1070:
    case 5604:
    {
      return WXOR(L"SPELL  BOOK");
    }
    case WPN_Scattergun:
    case WPN_FestiveScattergun:
    case WPN_NewScattergun:
    case WPN_BotScattergunS:
    case WPN_BotScattergunG:
    case WPN_BotScattergunR:
    case WPN_BotScattergunB:
    case WPN_BotScattergunC:
    case WPN_BotScattergunD:
    case WPN_BotScattergunES:
    case WPN_BotScattergunEG:
    case 15002:
    case 15015:
    case 15021:
    case 15029:
    case 15036:
    case 15065:
    case 15053:
    case 15069:
    case 15106:
    case 15107:
    case 15131:
    case 15108:
    case 15151:
    case 15157:
      return WXOR(L"SCATTERGUN");
    case WPN_FAN:
    case WPN_FestiveFAN:
      return WXOR(L"FORCE  A  NATURE");
    case WPN_Shortstop:
      return WXOR(L"SHORT  STOP");
    case WPN_SodaPopper:
      return WXOR(L"SODA  POPPER");
    case WPN_BabyFaceBlaster:
      return WXOR(L"BABY  FACE BLASTER");
    case WPN_BackScatter:
      return WXOR(L"BACK  SCATTER");
    case WPN_ScoutPistol:
    case WPN_NewPistol:
    case WPN_Lugermorph:
    case WPN_Lugermorph2:
    case WPN_EngineerPistol:
    case WPN_CAPPER:
    case 15013:
    case 15018:
    case 15035:
    case 15041:
    case 15046:
    case 15056:
    case 15060:
    case 15061:
    case 15100:
    case 15101:
    case 15102:
    case 15126:
    case 15148:
      return WXOR(L"PISTOL");
    case WPN_SunStick:
      return WXOR(L"SUN  STICK");
    case WPN_Bonk:
    case WPN_FestiveBonk:
      return WXOR(L"BONK");
    case WPN_CritCola:
      return WXOR(L"CRIT  COLA");
    case WPN_Milk:
    case 1121:
      return WXOR(L"MAD  MILK");
    case WPN_Winger:
      return WXOR(L"WINGER");
    case WPN_PocketPistol:
      return WXOR(L"POCKET  PISTOL");
    case WPN_Bat:
    case WPN_NewBat:
    case WPN_FestiveBat:
    case WPN_BatSaber:
      return WXOR(L"BAT");
    case WPN_Sandman:
      return WXOR(L"SANDMAN");
    case WPN_Fish:
    case WPN_FestiveFish:
    case 572:
      return WXOR(L"FISH");
    case WPN_Cane:
      return WXOR(L"CANDY  CANE");
    case WPN_BostonBasher:
    case WPN_RuneBlade:
      return WXOR(L"BOSTON  BASHER");
    case WPN_FanOWar:
      return WXOR(L"FAN  O  WAR");
    case WPN_Atomizer:
      return WXOR(L"ATOMIZER");
    case WPN_ConscientiousObjector:
    case WPN_NecroSmasher:
    case WPN_BatOuttaHell:
    case WPN_FreedomStaff:
    case 1013:
      return WXOR(L"MELEE");
    case WPN_WrapAssassin:
      return WXOR(L"WRAP  ASSASSIN");
    case WPN_FlyingGuillotine1:
    case WPN_FlyingGuillotine2:
      return WXOR(L"FLYING  GUILLOTINE");
    case WPN_RocketLauncher:
    case WPN_NewRocketLauncher:
    case WPN_FestiveRocketLauncher:
    case WPN_BotRocketlauncherS:
    case WPN_BotRocketlauncherG:
    case WPN_BotRocketlauncherR:
    case WPN_BotRocketlauncherB:
    case WPN_BotRocketlauncherC:
    case WPN_BotRocketlauncherD:
    case WPN_BotRocketlauncherES:
    case WPN_BotRocketlauncherEG:
    case 15006:
    case 15014:
    case 15028:
    case 15043:
    case 15052:
    case 15057:
    case 15081:
    case 15104:
    case 15105:
    case 15129:
    case 15130:
    case 15150:
      return WXOR(L"ROCKET  LAUNCHER");
    case WPN_DirectHit:
      return WXOR(L"DIRECT  HIT");
    case WPN_BlackBox:
    case WPN_FestiveBlackbox:
      return WXOR(L"BLACK  BOX");
    case WPN_AirStrike:
      return WXOR(L"AIR  STRIKE");
    case WPN_RocketJump:
      return WXOR(L"ROCKET  JUMPER");
    case WPN_LibertyLauncher:
      return WXOR(L"LIBERTY  LAUNCHER");
    case WPN_CowMangler:
      return WXOR(L"COW  MANGLER");
    case WPN_Original:
      return WXOR(L"ORIGINAL");
    case WPN_BeggersBazooka:
      return WXOR(L"BEGGERS  BAZOOKA");
    case WPN_SoldierShotgun:
    case WPN_NewShotgun:
    case WPN_PyroShotgun:
    case WPN_FestiveShotgun:
    case WPN_HeavyShotgun:
    case WPN_EngineerShotgun:
    case 15003:
    case 15016:
    case 15044:
    case 15047:
    case 15085:
    case 15109:
    case 15132:
    case 15133:
    case 15152:
      return WXOR(L"SHOTGUN");
    case WPN_BuffBanner:
    case WPN_FestiveBuffBanner:
      return WXOR(L"BUFF  BANNER");
    case WPN_BattalionBackup:
      return WXOR(L"BATTALION  BACKUP");
    case WPN_Concheror:
      return WXOR(L"CONCHEROR");
    case WPN_ReserveShooter:
      return WXOR(L"RESERVE  SHOOTER");
    case WPN_RighteousBison:
      return WXOR(L"RIGHTEOUS  BISON");
    case WPN_Shovel:
    case WPN_NewShovel:
      return WXOR(L"SHOVEL");
    case WPN_Equalizer:
      return WXOR(L"EQUALIZER");
    case WPN_PainTrain:
      return WXOR(L"PAIN  TRAIN");
    case WPN_Katana:
      return WXOR(L"KATANA");
    case WPN_MarketGardener:
      return WXOR(L"MARKET  GARDENER");
    case WPN_DisciplinaryAction:
      return WXOR(L"WHIP");
    case WPN_EscapePlan:
      return WXOR(L"ESCAPE  PLAN");
    case WPN_Flamethrower:
    case WPN_NewFlamethrower:
    case WPN_FestiveFlamethrower:
    case WPN_Rainblower:
    case WPN_BotFlamethrowerS:
    case WPN_BotFlamethrowerG:
    case WPN_BotFlamethrowerR:
    case WPN_BotFlamethrowerB:
    case WPN_BotFlamethrowerC:
    case WPN_BotFlamethrowerD:
    case WPN_BotFlamethrowerES:
    case WPN_BotFlamethrowerEG:
    case 15005:
    case 15017:
    case 15030:
    case 15034:
    case 15049:
    case 15054:
    case 15066:
    case 15067:
    case 15068:
    case 15089:
    case 15090:
    case 15115:
    case 15141:
    case 30474:
      return WXOR(L"FLAME  THROWER");
    case WPN_DragonFury:
      return WXOR(L"DRAGON'S FURY");
    case WPN_GasPasser:
      return WXOR(L"GAS PASSER");
    case WPN_ThermalThruster:
      return WXOR(L"THERMAL  THRUSTER");
    case WPN_Backburner:
    case WPN_FestiveBackBurner:
      return WXOR(L"BACK  BURNER");
    case WPN_Degreaser:
      return WXOR(L"DEGREASER");
    case WPN_Phlogistinator:
      return WXOR(L"PHLOGISTINATOR");
    case WPN_Flaregun:
    case 1081:
      return WXOR(L"FLAREGUN");
    case WPN_Detonator:
      return WXOR(L"DETONATOR");
    case WPN_ManMelter:
      return WXOR(L"MAN  MELTER");
    case WPN_ScorchShot:
      return WXOR(L"SCORCH  SHOT");
    case WPN_Fireaxe:
    case WPN_NewAxe:
    case WPN_Lollychop:
      return WXOR(L"FIRE  AXE");
    case WPN_Axtingusher:
    case WPN_FestiveAxtwingisher:
    case WPN_Mailbox:
      return WXOR(L"AXTINGUSHER");
    case WPN_HomeWrecker:
    case WPN_Maul:
      return WXOR(L"HOME  WRECKER");
    case WPN_PowerJack:
      return WXOR(L"POWER  JACK");
    case WPN_Backscratcher:
      return WXOR(L"BACK  SCRATCHER");
    case WPN_VolcanoFragment:
      return WXOR(L"VOLCANO FRAGMENT");
    case WPN_ThirdDegree:
      return WXOR(L"THIRD  DEGREE");
    case WPN_NeonAnnihilator1:
    case WPN_NeonAnnihilator2:
      return WXOR(L"NEON  ANNIHILATOR");
    case WPN_GrenadeLauncher:
    case WPN_NewGrenadeLauncher:
    case WPN_FestiveGrenadeLauncher:
    case 15077:
    case 15079:
    case 15091:
    case 15092:
    case 15116:
    case 15117:
    case 15142:
    case 15158:
      return WXOR(L"GRENADE  LAUNCHER");
    case WPN_LochNLoad:
      return WXOR(L"LOCH  N  LOAD");
    case WPN_LoooseCannon:
      return WXOR(L"LOOSE  CANNON");
    case WPN_IronBomber:
      return WXOR(L"IRON  BOMBER");
    case WPN_StickyLauncher:
    case WPN_NewStickyLauncher:
    case WPN_FestiveStickyLauncher:
    case WPN_BotStickyS:
    case WPN_BotStickyG:
    case WPN_BotStickyR:
    case WPN_BotStickyB:
    case WPN_BotStickyC:
    case WPN_BotStickyD:
    case WPN_BotStickyES:
    case WPN_BotStickyEG:
    case 15009:
    case 15012:
    case 15024:
    case 15038:
    case 15045:
    case 15048:
    case 15082:
    case 15083:
    case 15084:
    case 15113:
    case 15137:
    case 15138:
    case 15155:
      return WXOR(L"STICKY  LAUNCHER");
    case WPN_StickyJumper:
      return WXOR(L"STICKY  JUMPER");
    case WPN_ScottishResistance:
      return WXOR(L"SCOTTISH  RESISTANCE");
    case WPN_QuickieBombLauncher:
      return WXOR(L"QUICKIE  BOMB  LAUNCHER");
    case WPN_Bottle:
    case WPN_NewBottle:
    case WPN_ScottishHandshake:
      return WXOR(L"BOTTLE");
    case WPN_Sword:
    case WPN_FestiveEyeLander:
    case WPN_Headless:
    case WPN_Golfclub:
      return WXOR(L"EYE  LANDER");
    case WPN_ScottsSkullctter:
      return WXOR(L"SKULL  CUTTER");
    case WPN_Fryingpan:
    case 1071:
      return WXOR(L"FRYING  PAN");
    case WPN_Claidheamhmor:
      return WXOR(L"CLAIDHEAMHMOR");
    case WPN_PersainPersuader:
      return WXOR(L"PERSAIN  PERSUADER");
    case WPN_Ullapool:
      return WXOR(L"ULLAPOOL");
    case WPN_Revolver:
    case WPN_FestiveRevolver:
    case WPN_NewRevolver:
    case WPN_BigKill:
    case WPN_BigEarner:
    case 15011:
    case 15027:
    case 15042:
    case 15051:
    case 15062:
    case 15063:
    case 15064:
    case 15103:
    case 15128:
    case 15149:
    case 15127:
      return WXOR(L"REVOLVER");
    case WPN_FestiveBonesaw:
      return WXOR(L"BONESAW");
    case WPN_SMG:
    case WPN_FestiveSMG:
    case 15001:
    case 15022:
    case 15032:
    case 15037:
    case 15058:
    case 15076:
    case 15110:
    case 15134:
    case 15153:
    case 203:
      return WXOR(L"SMG");
    case WPN_Minigun:
    case WPN_NewMinigun:
    case WPN_FestiveMinigun:
    case WPN_BotMinigunS:
    case WPN_BotMinigunG:
    case WPN_BotMinigunR:
    case WPN_BotMinigunB:
    case WPN_BotMinigunC:
    case WPN_BotMinigunD:
    case WPN_BotMinigunES:
    case WPN_BotMinigunEG:
    case WPN_IronCurtain:
    case 15020:
    case 15004:
    case 15026:
    case 15031:
    case 15040:
    case 15055:
    case 15086:
    case 15087:
    case 15088:
    case 15098:
    case 15099:
    case 15123:
    case 15124:
    case 15125:
    case 15147:
      return WXOR(L"MINIGUN");
    case WPN_Deflector:
      return WXOR(L"DEFLECTOR");
    case WPN_Natascha:
      return WXOR(L"NATASCHA");
    case WPN_BrassBeast:
      return WXOR(L"BRASS  BEAST");
    case WPN_Tomislav:
      return WXOR(L"TOMISLAV");
    case WPN_HuoLongHeatmaker1:
    case WPN_HuoLongHeatmaker2:
      return WXOR(L"HUO-LONG  HEATER");
    case WPN_Sandvich:
    case WPN_RobotSandvich:
    case WPN_FestiveSandvich:
      return WXOR(L"SANDVICH");
    case WPN_Banana:
      return WXOR(L"BANANA");
    case WPN_CandyBar:
    case WPN_Fishcake:
      return WXOR(L"CANDYBAR");
    case WPN_Steak:
      return WXOR(L"STEAK");
    case WPN_FamilyBuisness:
      return WXOR(L"FAMILY  BUSINESS");
    case WPN_Fists:
    case WPN_NewFists:
    case WPN_ApocoFists:
    case 1100:
      return WXOR(L"FISTS");
    case WPN_KGB:
      return WXOR(L"FISTS-KGB");
    case WPN_GRU:
    case WPN_FestiveRunningGloves:
    case 1184:
      return WXOR(L"FISTS-GRU");
    case WPN_WarriorSpirit:
      return WXOR(L"WARRIOR  SPIRIT");
    case WPN_FistsOfSteel:
      return WXOR(L"FISTS  OF  STEEL");
    case WPN_EvictionNotice:
      return WXOR(L"EVICTION  NOTICE");
    case WPN_HolidayPunch:
      return WXOR(L"HOLIDAY  PUNCH");

    case WPN_PanicAttack:
      return WXOR(L"PANIC  ATTACK");
    case WPN_FrontierJustice:
    case WPN_FestiveFrontierJustice:
      return WXOR(L"FRONTIER  JUSTICE");
    case WPN_Widowmaker:
      return WXOR(L"WIDOW  MAKER");
    case WPN_Pomson:
      return WXOR(L"POMSON");
    case WPN_RescueRanger:
      return WXOR(L"RESCUE  RANGER");
    case WPN_Wrangler:
    case WPN_FestiveWrangler:
    case WPN_GigarCounter:
      return WXOR(L"WRANGLER");
    case WPN_ShortCircut:
      return WXOR(L"SHORT  CIRCUIT");
    case WPN_Wrench:
    case WPN_NewWrench:
    case WPN_FestiveWrench:
    case WPN_BotWrenchS:
    case WPN_BotWrenchG:
    case WPN_BotWrenchR:
    case WPN_BotWrenchB:
    case WPN_BotWrenchC:
    case WPN_BotWrenchD:
    case WPN_BotWrenchES:
    case WPN_BotWrenchEG:
    case 15114:
    case 15140:
    case 15074:
    case 15156:
    case 15075:
    case 15073:
    case 15139:
      return WXOR(L"WRENCH");
    case WPN_Goldenwrench:
      return WXOR(L"GOLDEN  WRENCH");
    case WPN_SouthernHospitality:
      return WXOR(L"SOUTHERN  HOSPITALITY");
    case WPN_Gunslinger:
      return WXOR(L"GUNSLINGER");
    case WPN_Jag:
      return WXOR(L"JAG");
    case WPN_EurekaEffect:
      return WXOR(L"EUREKA  EFFECT");
    case WPN_Builder:
    case WPN_Builder2:
      return WXOR(L"BUILDER");
    case WPN_Destructor:
      return WXOR(L"DESTRUCTOR");
    case WPN_Toolbox:
      return WXOR(L"TOOLBOX");
    case WPN_SyringeGun:
    case WPN_NewSyringeGun:
      return WXOR(L"SYRINGE  GUN");
    case WPN_Blutsauger:
      return WXOR(L"BLUTSAUGER");
    case WPN_Crossbow:
    case 1079:
      return WXOR(L"CROSSBOW");
    case WPN_Overdose:
      return WXOR(L"OVERDOSE");
    case WPN_Medigun:
    case WPN_NewMedigun:
    case WPN_FestiveMedigun:
    case WPN_MedigunS:
    case WPN_MedigunG:
    case WPN_MedigunR:
    case WPN_MedigunB:
    case WPN_MedigunC:
    case WPN_MedigunD:
    case WPN_MedigunES:
    case WPN_MedigunEG:
    case 15008:
    case 15120:
    case 15010:
    case 15025:
    case 15039:
    case 15050:
    case 15078:
    case 15097:
    case 15121:
    case 15122:
    case 15145:
    case 15146:
      return WXOR(L"MEDIGUN");
    case WPN_Kritzkrieg:
      return WXOR(L"KRITZKRIEG");
    case WPN_QuickFix:
      return WXOR(L"QUICKFIX");
    case WPN_Vaccinator:
      return WXOR(L"VACCINATOR");
    case WPN_Bonesaw:
    case WPN_NewBonesaw:
      return WXOR(L"BONESAW");
    case WPN_Ubersaw:
    case WPN_FestiveUbersaw:
      return WXOR(L"UBERSAW");
    case WPN_Vitasaw:
      return WXOR(L"VITASAW");
    case WPN_Amputator:
      return WXOR(L"AMPUTATOR");
    case WPN_Solemnvow:
      return WXOR(L"SOLEMNVOW");

    case WPN_SniperRifle:
    case WPN_NewSniperRifle:
    case WPN_FestiveSniperRifle:
    case WPN_BotSniperRifleS:
    case WPN_BotSniperRifleG:
    case WPN_BotSniperRifleR:
    case WPN_BotSniperRifleB:
    case WPN_BotSniperRifleC:
    case WPN_BotSniperRifleD:
    case WPN_BotSniperRifleES:
    case WPN_BotSniperRifleEG:
    case 15000:
    case 15007:
    case 15019:
    case 15023:
    case 15033:
    case 15059:
    case 15070:
    case 15071:
    case 15072:
    case 15111:
    case 15112:
    case 15135:
    case 15136:
    case 15154:
    case WPN_AWP:
    case WPN_ShootingStar:
      return WXOR(L"SNIPER  RIFLE");
    case WPN_Machina:
      return WXOR(L"MACHINA");
    case WPN_Huntsman:
    case WPN_FestiveHuntsman:
    case WPN_FortifiedCompound:
      return WXOR(L"HUNTSMAN");
    case WPN_SydneySleeper:
      return WXOR(L"SYDNEY  SLEEPER");
    case WPN_Bazaarbargain:
      return WXOR(L"BAZAAR  BARGAIN");
    case WPN_HitmanHeatmaker:
      return WXOR(L"HITMAN  HEATMAKER");
    case WPN_Classic:
      return WXOR(L"CLASSIC");
    case WPN_Jarate:
    case WPN_FestiveJarate:
    case 1105:
      return WXOR(L"JARATE");
    case WPN_CleanersCarbine:
      return WXOR(L"CLEANERS  CARBINE");
    case WPN_Kukri:
    case WPN_NewKukri:
      return WXOR(L"KUKRI");
    case WPN_TribalmansShiv:
      return WXOR(L"TRIBALMANS SHIV");
    case WPN_Bushwacka:
      return WXOR(L"BUSHWACKA");
    case WPN_Shahanshah:
      return WXOR(L"SHAHANSHAH");
    case WPN_Ambassador:
    case WPN_FestiveAmbassador:
      return WXOR(L"AMBASSADOR");
    case WPN_Letranger:
      return WXOR(L"LETRANGER");
    case WPN_Enforcer:
      return WXOR(L"ENFORCER");
    case WPN_Diamondback:
      return WXOR(L"DIAMONDBACK");
    case WPN_Knife:
    case WPN_NewKnife:
    case WPN_BlackRose:
    case WPN_SharpDresser:
    case WPN_FestiveKnife:
    case WPN_BotKnifeS:
    case WPN_BotKnifeG:
    case WPN_BotKnifeR:
    case WPN_BotKnifeB:
    case WPN_BotKnifeC:
    case WPN_BotKnifeD:
    case WPN_BotKnifeES:
    case WPN_BotKnifeEG:
    case 15094:
    case 15095:
    case 15096:
    case 15118:
    case 15119:
    case 15143:
    case 15144:
      return WXOR(L"KNIFE");
    case 30758:
      return WXOR(L"PRINNY  MACHETE");
    case 1127:
      return WXOR(L"SAXXY");
    case WPN_Spycicle:
      return WXOR(L"SPY  CICLE");
    case WPN_EternalReward:
    case WPN_WangaPrick:
      return WXOR(L"ETERNAL  REWARD");
    case WPN_DisguiseKit:
      return WXOR(L"DISGUISE  KIT");
    case WPN_Kunai:
      return WXOR(L"KUNAI");
    case WPN_Sapper:
    case WPN_Sapper2:
    case WPN_FestiveSapper:
    case 933:
    case 1102:
      return WXOR(L"SAPPER");
    case 1181:
      return WXOR(L"HOT  HAND");
    case WPN_RedTape1:
    case WPN_RedTape2:
      return WXOR(L"RED  TAPE  RECORDER");
    case 40000:
      return WXOR(L"ITEM TEST WEAPON TESTER");
    case 5622:
    case 5623:
    case 5624:
    case 5625:
    case 5621:
    case 5620:
    case 5619:
    case 5618:
    case 5617:
      return WXOR(L"VOODOO  CURSED  SOUL");
    default:{
      DBG("[!] unknown weapon def index: %i\n", weapon_id());
      return WXOR(L"UNK");
    }
  }
}

ALWAYSINLINE c_base_player* c_base_weapon::get_heal_target(){
  if(heal_target_handle() <= 0)
    return nullptr;
  
  return (c_base_player*)global->entity_list->get_entity_handle(heal_target_handle());
}

ALWAYSINLINE bool c_base_weapon::can_uber(){
  if( !is_medigun() )
    return false;

  if(this->weapon_id() == WPN_Vaccinator)
    return this->uber_charge_level() >= 0.25f;

  return this->uber_charge_level() >= 1.0f;
}


// This function is not thread safe. Do not call outside of a game thread.
ALWAYSINLINE ctf_weapon_info* c_base_weapon::get_weapon_info(){
  assert(global->look_weapon_info_slot != nullptr);
  assert(global->file_weapon_info_from_handle != nullptr);

  u8* class_name = get_class_name();
  if(class_name == nullptr)
    return nullptr;

  i32 slot     = utils::call_fastcall64_raw<i32, u8*>(global->look_weapon_info_slot, get_class_name());
  if(slot == -1)
    return nullptr;

  return utils::call_fastcall64_raw<ctf_weapon_info*, i32>(global->file_weapon_info_from_handle, slot);
}

NEVERINLINE float c_base_weapon::time_between_shots(){
  ctf_weapon_info* info = get_weapon_info();

  if(info == nullptr){
    assert(info != nullptr);
    return 0.f;
  }

  // search for "Weapon_StickyBombLauncher.ChargeUp"
  // v5 = ((double (__thiscall *)(float *, _DWORD))*(_DWORD *)(*(_DWORD *)this + 0x654))(this, *(float*)((*((_DWORD*)this + 713) << 6) + *((_DWORD*)this + 716) + 1804))
  // 0x654 <- offset
  create_offset_hash("time_between_shots");
  return utils::internal_virtual_call<float, float>(offset, this, info->time_fire_delay);
}

ALWAYSINLINE bool c_base_weapon::is_perfect_shot(){
  ctf_weapon_info* info = get_weapon_info();

  if(info == nullptr){
    assert(info != nullptr);
    return false;
  }

  i32 bullets = (i32)utils::attrib_hook_float( (float)info->bullets_per_shot, XOR("mult_bullets_per_shot"), this, nullptr, true);
  float time_since_lastshot = math::abs(globaldata->cur_time - last_fire_time());

  bullets = math::biggest(bullets, 1);

  if( bullets > 1 && time_since_lastshot > 0.25f ||
      bullets == 1 && time_since_lastshot > 1.25f)
    return true;

  return false;
}

ALWAYSINLINE bool c_base_weapon::deals_minicritdmg_inair(){
  assert(this != nullptr);
  switch(weapon_id()){
    case WPN_ReserveShooter:
    case WPN_DirectHit:
      return true;
  }

  return false;
}

NEVERINLINE float c_base_weapon::get_swing_range(bool apply_attrib = false){
  if(!is_melee())
    return 0.f;

  create_offset_hash("get_swing_range");
  float range = (float)utils::internal_virtual_call<i32>(offset, this);

  if(apply_attrib){
    c_base_player* owner = get_owner();
    if(owner != nullptr){

      if(owner->model_scale() > 1.0f)
        range *= owner->model_scale();

      range = utils::attrib_hook_float(range, XOR("melee_range_multiplier"), this, 0, 1);
    }
  }

  return range;
}

// Pass the random_seed value that's meant to be sent to the server.
// Unless you're using this to test something.
ALWAYSINLINE void c_base_weapon::setup_random_seed(i32 seed){
  i32 itr = 6;
  //if(is_syringe_gun()) (I think our cheat causes the game todo an extra 3 random float calls) - 1.26.2024 (no its the game that does it)
  //  itr = 9;

  math::setup_random_seed(itr, seed, false);
}

NEVERINLINE float c_base_weapon::get_sticky_launcher_arm_time(){
  float live_time = utils::attrib_hook_float(0.8f, XOR("sticky_arm_time"), this, 0, 1);

  if(utils::is_playing_mannpower()){
    c_base_player* owner = get_owner();
    if(owner != nullptr){
      if(owner->has_condition(TF_COND_RUNE_HASTE))
        live_time *= 0.5f;
      else if(owner->has_condition(TF_COND_KING_BUFFED))
        live_time *= 0.75f;
    }
  }

  return live_time;
}

NEVERINLINE bool c_base_weapon::get_vphysics_velocity(vec3* vel, vec3 angles, bool disable_random_impulse){
  assert(vel != nullptr);

  c_base_player* owner = get_owner();
  if(owner == nullptr)
    return false;

  vec3 fwd, right, up;
  math::angle_2_vector(angles, &fwd, &right, &up);

  setup_random_seed(command_manager->get_seed());
  if(is_grenade_launcher() || is_loose_cannon()){
    float speed = utils::attrib_hook_float(1200.f, XOR("mult_projectile_range"), this, 0, 1);
    speed = utils::attrib_hook_float(speed, XOR("mult_projectile_speed"), this, 0, 1);
    if(owner->has_condition(TF_COND_RUNE_PRECISION))
      speed = 3000.f;

    if(!disable_random_impulse)
      *vel = (fwd * speed) + (up * 200.f) + (right * math::random_float(-10.f, 10.f)) + (up * math::random_float(-10.f, 10.f));
    else
      *vel = (fwd * speed) + (up * 200.f);

    return true;
  }
  else if(is_sticky_launcher()){
    float charge = (globaldata->cur_time - charge_begin_time());
    float speed  = math::remap_val_clamped(charge, 0.f, get_sticky_launcher_max_charge(), 900.f, 2400.f);
    if(charge <= 0.f || charge_begin_time() <= 0.f)
      speed = 900.f;

    speed = utils::attrib_hook_float(speed, XOR("mult_projectile_range"), this, 0, 1);
    speed = utils::attrib_hook_float(speed, XOR("mult_projectile_speed"), this, 0, 1);

    if(!disable_random_impulse)
      *vel = (fwd * speed) + (up * 200.f) + (right * math::random_float(-10.f, 10.f)) + (up * math::random_float(-10.f, 10.f));
    else
      *vel = (fwd * speed) + (up * 200.f);

    return true;
  }
  else if(is_jarate_jar() || is_mad_milk_jar() || is_gas_jar()){
    float speed = is_gas_jar() ? 2000.f : 1000.f;
    if(!disable_random_impulse)
      *vel = (fwd * speed) + (up * 200.f) + (right * math::random_float(-10.f, 10.f)) + (up * math::random_float(-10.f, 10.f));
    else
      *vel = (fwd * speed) + (up * 200.f);

    return true;
  }
  else if(is_cleaver()){
    vec3 calc = fwd * 10.f;
    calc += up * 1.f;
    calc.normalize();
    calc *= 3000.f;
    *vel = calc;
    return true;
  }
  else if(is_ball_bat()){
    #if !defined(DEV_MODE)
    //(THROWABLE SUPPORT)
    return false;
    #endif
    vec3 calc = fwd * 10.f;
    calc += up * 1.f;
    calc.normalize();
    calc *= 3000.f;
    *vel = calc;
    return true;
  }

  return false;
}

NEVERINLINE float c_base_weapon::get_projectile_speed(vec3 angles){
  if(is_wrangler())
    return 65355.f;

  if(!is_projectile_weapon())
    return 0.f;

  create_offset_hash("get_projectile_speed");
  float speed = utils::internal_virtual_call<float>(offset, this); // get_projectile_speed
  if(speed <= 0.0f)
    speed = 1100.f;

  c_base_player* owner = get_owner();
  if(owner == nullptr)
    return speed;

  if(is_rocket_launcher()){
    speed = utils::attrib_hook_float(speed, XOR("mult_projectile_speed"), this, 0, 1);

    // Rocket specialist adjusts the projectile speed.
    i32 rocket_specialist = utils::attrib_hook_int(0, XOR("rocket_specialist"), owner, 0, 1);
    if(rocket_specialist > 0){
      speed *= math::remap_val_clamped((float)rocket_specialist, 1.f, 4.f, 1.15f, 1.6f);
      speed = math::smallest(speed, 3000.f);
    }

    if(owner->has_condition(TF_COND_RUNE_PRECISION))
      return 3000.f;

    return speed;
  }
  else if(is_flaregun())
    speed = 2000.f;
  else if(is_grenade_launcher() || is_loose_cannon()){
    vec3 vel;
    get_vphysics_velocity(&vel, angles);
    return math::round(vel.length());
  }
  else if(is_syringe_gun())
    speed = 1000.f;
  else if(is_rescue_ranger() || is_crossbow())
    speed = math::remap_val_clamped(0.75f, 0.0f, 1.f, 1800.f, 2600.f);
  else if(is_cleaver()){
    vec3 vel;
    get_vphysics_velocity(&vel, angles);
    return math::round(vel.length());
  }
  else if(is_mad_milk_jar() || is_jarate_jar() || is_gas_jar()){
    vec3 vel;
    get_vphysics_velocity(&vel, angles);
    return math::round(vel.length());
  }
  else if(is_dragons_fury())
    speed = global->tf_fireball_speed->flt;
  else if(is_flamethrower())
    speed = global->tf_flamethrower_velocity->flt;
  else if(is_huntsman()){
    float charge = charge_begin_time() > 0.f ? math::smallest(globaldata->cur_time - charge_begin_time(), 1.f) : 0.f;
    speed = math::remap_val_clamped(charge, 0.0f, 1.f, 1800.f, 2600.f);
  }
  else if(is_sticky_launcher()){
    vec3 vel;
    get_vphysics_velocity(&vel, angles);
    return math::round(vel.length());
  }

  speed = utils::attrib_hook_float(speed, XOR("mult_projectile_speed"), this, 0, 1);
  return speed;
}

NEVERINLINE float c_base_weapon::get_projectile_gravity(){
  if(!is_projectile_weapon())
    return 0.f;

  create_offset_hash("get_projectile_gravity");
  float gravity = utils::internal_virtual_call<float>(offset, this); // get_projectile_gravity

  if(is_flaregun())
    gravity = utils::attrib_hook_float(0.3f, XOR("mult_projectile_speed"), this, 0, 1);
  else if(is_syringe_gun() || is_jarate_jar() || is_gas_jar() || is_mad_milk_jar() || is_cleaver())
    gravity = 0.3f; // This is the syringe's guns but every thing else no idea.
  else if(is_rescue_ranger() || is_crossbow())
    gravity = math::remap_val_clamped(0.75f, 0.0f, 1.f, 0.5f, 0.1f);
  else if(is_huntsman()){
    float charge = charge_begin_time() > 0.f ? math::smallest(globaldata->cur_time - charge_begin_time(), 1.f) : 0.f;
    gravity = math::remap_val_clamped(charge, 0.0f, 1.f, 0.5f, 0.1f);
  }
  else if(is_sticky_launcher() || is_grenade_launcher() || is_loose_cannon()){
    return 0.5f;
  }

  return gravity;
}

NEVERINLINE bool c_base_weapon::get_projectile_size(vec3& min, vec3& max){
  if(!is_projectile_weapon() && !is_wrangler() && !is_ball_bat())
    return false;

  if(is_flamethrower()){
    float box_size = global->tf_flamethrower_boxsize->flt;
    min = vec3(-box_size, -box_size, -box_size);
    max = vec3(box_size, box_size, box_size);
  }
  else if(is_wrangler()){
    min = vec3();
    max = vec3();
  }
  else if(is_dragons_fury()){
    min = vec3(-6.f, -6.f, -6.f);
    max = vec3(6.f, 6.f, 6.f);
  }
  else if(is_grenade_launcher() || is_loose_cannon() || is_sticky_launcher()){
    min = vec3(-6.f, -6.f, -6.f);
    max = vec3(6.f, 6.f, 6.f);
  }
  else if(is_throwables()){
    min = vec3(-8.f, -8.f, -8.f);
    max = vec3(8.f, 8.f, 8.f);
  }
  else if(is_crossbow()){
    min = vec3(-4.f, -4.f, -4.f);
    max = vec3(4.f, 4.f, 4.f);
  }
  else if(is_rescue_ranger() || is_huntsman()){
    min = vec3(-2.f, -2.f, -2.f);
    max = vec3(2.f, 2.f, 2.f);
  }
  else if(is_ball_bat()){
    min = vec3(-4.25f, -4.25f, -4.25f);
    max = vec3(4.25f, 4.25f, 4.25f);
  }
  else if(is_rocket_launcher() || is_beggars()){
    min = vec3(-1.f, -1.f, -1.f);
    max = vec3(1.f, 1.f, 1.f);
  }
  else{
    // was the size of 6.
    min = vec3(-2.f, -2.f, -2.f);
    max = vec3(2.f, 2.f, 2.f);
  }

  return true;
}

ALWAYSINLINE void c_base_weapon::get_projectile_fire_setup(c_base_player* player, vec3 offset, vec3* shoot_pos, vec3* eye_angles){
  assert(global->get_projectile_fire_setup != nullptr);
  if(player == nullptr || shoot_pos == nullptr || eye_angles == nullptr)
    return;

  utils::call_fastcall64<void, c_base_player*, vec3, vec3*, vec3*, bool, float>(global->get_projectile_fire_setup, this, player, offset, shoot_pos, eye_angles, is_syringe_gun(), 2000.f);
  if(weapon_id() == WPN_Pomson)
    shoot_pos->z -= 13.f;
}

ALWAYSINLINE float c_base_weapon::get_loose_cannon_charge_begin_time(){
  float detonate_time_length      = utils::attrib_hook_float(0.f, XOR("grenade_launcher_mortar_mode"), this, 0,1);
  float detonate_time_length_mod  = detonate_time_length;

  if(detonate_time() > 0.f)
    detonate_time_length_mod = math::clamp(detonate_time() - globaldata->cur_time, 0.f, detonate_time_length);

  return globaldata->cur_time - detonate_time_length_mod;
}

ALWAYSINLINE float c_base_weapon::get_loose_cannon_charge(){
  return 1.f - math::clamp(globaldata->cur_time - get_loose_cannon_charge_begin_time(), 0.f, 1.f);
}

NEVERINLINE bool c_base_weapon::get_weapon_offset(vec3& offset){
  if(!is_projectile_weapon()){
    offset = vec3();
    return false;
  }

  c_base_player* owner = get_owner();
  if(owner == nullptr){
    offset = vec3();
    return false;
  }

  if(is_huntsman() || is_rescue_ranger() || is_crossbow() || weapon_id() == WPN_RighteousBison || weapon_id() == WPN_Pomson){
    offset = vec3(23.5f, -8.0f, -3.0f);
    return true;
  }
  else if(is_syringe_gun()){
    offset = vec3(16.f, 6.f, -8.f);
    return true;
  }
  else if(is_cow_mangler()){
    offset = vec3(23.5f, -8.0f, owner->entity_flags() & FL_DUCKING ? 8.0f : -3.0f);
    return true;
  }
  else if(is_rocket_launcher() || is_flaregun()){
    offset = vec3(23.5f, 12.0f, owner->entity_flags() & FL_DUCKING ? 8.0f : -3.0f);
    return true;
  }
  else if(is_grenade_launcher() || is_sticky_launcher() || is_loose_cannon() || is_throwables() || is_cleaver()){
    offset = vec3(16.0f, 8.f, -6.f);
    return true;
  }
  else if(is_flamethrower() || is_dragons_fury()){
    offset = vec3();
    return true;
  }

  offset = vec3();
  DBG("[-] Projectile weapon %s has no size programmed! Fix it!\n", get_client_class()->name);
  return false;
}

NEVERINLINE float c_base_weapon::get_blast_radius(float creation_time, bool touched){
  float radius = 0.f;

  if(is_detonator() || is_scorch_shot())
    radius = 110.f;
  else if(is_sticky_launcher()){
    radius = utils::attrib_hook_float(146.f, XOR("mult_explosion_radius"), this, 0, 1);
    float radius_mod = 1.0f;
    if(creation_time > -1.f){
      float tf_grenadelauncher_livetime = 0.8f;
      float tf_sticky_radius_ramp_time  = 2.0f;
      float tf_sticky_airdet_radius     = 0.85f;
      radius_mod = math::remap_val_clamped(globaldata->cur_time - creation_time, tf_grenadelauncher_livetime, tf_grenadelauncher_livetime + tf_sticky_radius_ramp_time, tf_sticky_airdet_radius, 1.0f);
    }

    return radius * radius_mod;
  }
  else if(is_rocket_launcher() || is_beggars()){
    radius = 146.f;
    if(is_beggars()) // mult_explosion_radius check below handles this just fine, it's just that for some reason unknown for now it appears to be too big still.
      radius *= 0.95f;
  }
  else if(is_huo_long_heater()){
    return 135.f;
  }
  else if(is_jarate_jar() || is_mad_milk_jar() || is_gas_jar()){
    return 200.f;
  }

  if(radius <= 0.f)
    return 0.f;

  return utils::attrib_hook_float(radius, XOR("mult_explosion_radius"), this, 0, 1);
}