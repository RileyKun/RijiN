#pragma once

typedef void*(*init_interface_fn)();
struct s_interface_list{
  void*             init_interface;
  i8*               name;
  s_interface_list* next;
};

class s_net_channel{
public:
  PAD(0x8);
  i32 out_sequence_nr;
  i32 in_sequence_nr;
  i32 out_sequence_nr_ack;
  i32 out_reliable_state;
  i32 in_reliable_state;
  i32 choked_packets;
};

struct s_player_info{
  i8      name[ 32 ];
  i32     user_id;
  i8      guid[ 33 ];
  u32     friends_id;
  i8      friends_name[ 32 ];
  bool    fake_player;
  bool    is_hltv;
  u32     custofiles[ 4 ];
  u8      files_downloaded;
};

struct s_user_cmd{
  PAD(0x4);
  i32   command_number;
  i32   tick_count;
  vec3  view_angles;
  vec3  move;
  i32   buttons;
  u8    impulse;
  i32   weapon_select;
  i32   weapon_subtype;
  i32   random_seed;
  i16   mouse_x;
  i16   mouse_y;
  bool  predicted;
};

struct s_client_state{
  union{
    DATA(0x198, i32 server_tick);              // m_nServerTick
    DATA(0x19C, i32 client_tick);              // m_nClientTick
    DATA(0x1A0, i32 delta_tick);               // m_nDeltaTick
    DATA(0x1A4, i32 signon_state);             // m_nSignonState
    DATA(0x4B2C, i32 last_command_ack);       // last_command_ack
    DATA(0x4B24, i32 last_outgoing_command);  // lastoutgoingcommand
    DATA(0x4B28, i32 choked_commands);        // chokedcommands
  };
};

struct s_input{
  union{
    DATA(0x5, bool mouse_init);
    DATA(0x6, bool mouse_active);
    DATA(0xEC, s_user_cmd* commands);
    DATA(0xF0, s_user_cmd* verified_commands);
  };
};

struct s_global_data{
  float   real_time;
  i32     frame_count;
  float   absolute_frame_time;
  float   cur_time;
  float   frame_time;
  i32     max_clients;
  i32     tick_count;
  float   interval_per_tick;
  float   interpolation_amount;
  i32     sim_ticks_this_frame;
};

struct s_view_setup{
  i32       x;
  i32       unscaled_x;
  i32       y;
  i32       unscaled_y;
  i32       width;
  i32       unscaled_width;
  i32       height;
  i32       stereo_eye;
  i32       unscaled_height;
  bool      ortho;
  float     ortho_left;
  float     ortho_top;
  float     ortho_right;
  float     ortho_bottom;
  float     fov;
  float     viewmodel_fov;
  vec3      origin;
  vec3      angles;
  float     nearz;
  float     farz;
  float     viewmodel_z_near;
  float     viewmodel_z_far;
  bool      render_to_subrect_of_larger_screen;
  float     aspect_ratio;
  bool      off_center;
  float     off_center_top;
  float     off_center_bottom;
  float     off_center_left;
  float     off_center_right;
  bool      do_bloom_and_tone_mapping;
  bool      cache_full_scene_state;
  bool      view_to_projection_override;
  matrix4x4 view_to_projection;
};

struct s_client_class{
  union{
    DATA(0x8, i8* name);
    DATA(0x14, i32 id);
  };
};

enum FCVAR {
  FCVAR_NONE = 0,
  FCVAR_UNREGISTERED = 1 << 0,
  FCVAR_DEVELOPMENTONLY = 1 << 1,
  FCVAR_GAMEDLL = 1 << 2,
  FCVAR_CLIENTDLL = 1 << 3,
  FCVAR_HIDDEN = 1 << 4,
  FCVAR_PROTECTED = 1 << 5,
  FCVAR_SPONLY = 1 << 6,
  FCVAR_ARCHIVE = 1 << 7,
  FCVAR_NOTIFY = 1 << 8,
  FCVAR_USERINFO = 1 << 9,
  FCVAR_CHEAT = 1 << 14,
  FCVAR_PRINTABLEONLY = 1 << 10,
  FCVAR_UNLOGGED = 1 << 11,
  FCVAR_NEVER_AS_STRING = 1 << 12,
  FCVAR_REPLICATED = 1 << 13,
  FCVAR_DEMO = 1 << 16,
  FCVAR_DONTRECORD = 1 << 17,
  FCVAR_NOT_CONNECTED = 1 << 22,
  FCVAR_ARCHIVE_XBOX = 1 << 24,
  FCVAR_SERVER_CAN_EXECUTE = 1 << 28,
  FCVAR_SERVER_CANNOT_QUERY = 1 << 29,
  FCVAR_CLIENTCMD_CAN_EXECUTE = 1 << 30
};

enum frame_stage {
  FRAME_UNDEFINED = -1,
  FRAME_START,
  FRAME_NET_UPDATE_START,
  FRAME_NET_UPDATE_POSTDATAUPDATE_START,
  FRAME_NET_UPDATE_POSTDATAUPDATE_END,
  FRAME_NET_UPDATE_END,
  FRAME_RENDER_START,
  FRAME_RENDER_END
};

enum player_flags {
  FL_ONGROUND = 1 << 0,
  FL_DUCKING = 1 << 1,
  FL_WATERJUMP = 1 << 2,
  FL_ONTRAIN = 1 << 3,
  FL_INRAIN = 1 << 4,
  FL_FROZEN = 1 << 5,
  FL_ATCONTROLS = 1 << 6,
  FL_CLIENT = 1 << 7,
  FL_FAKECLIENT = 1 << 8,
  FL_INWATER = 1 << 9,
  FL_FLY = 1 << 10,
  FL_SWIM = 1 << 11,
  FL_CONVEYOR = 1 << 12,
  FL_NPC = 1 << 13,
  FL_GODMODE = 1 << 14,
  FL_NOTARGET = 1 << 15,
  FL_AIMTARGET = 1 << 16,
  FL_PARTIALGROUND = 1 << 17,
  FL_STATICPROP = 1 << 18,
  FL_GRAPHED = 1 << 19,
  FL_GRENADE = 1 << 20,
  FL_STEPMOVEMENT = 1 << 21,
  FL_DONTTOUCH = 1 << 22,
  FL_BASEVELOCITY = 1 << 23,
  FL_WORLDBRUSH = 1 << 24,
  FL_OBJECT = 1 << 25,
  FL_KILLME = 1 << 26,
  FL_ONFIRE = 1 << 27,
  FL_DISSOLVING = 1 << 28,
  FL_TANSRAGDOLL = 1 << 29,
  FL_UNBLOCKABLE_BY_PLAYER = 1 << 30,
  IN_ATTACK = 1 << 0,
  IN_JUMP = 1 << 1,
  IN_DUCK = 1 << 2,
  IN_FORWARD = 1 << 3,
  IN_BACK = 1 << 4,
  IN_USE = 1 << 5,
  IN_CANCEL = 1 << 6,
  IN_LEFT = 1 << 7,
  IN_RIGHT = 1 << 8,
  IN_MOVELEFT = 1 << 9,
  IN_MOVERIGHT = 1 << 10,
  IN_ATTACK2 = 1 << 11,
  IN_RUN = 1 << 12,
  IN_RELOAD = 1 << 13,
  IN_ALT1 = 1 << 14,
  IN_ALT2 = 1 << 15,
  IN_SCORE = 1 << 16,
  IN_SPEED = 1 << 17,
  IN_WALK = 1 << 18,
  IN_ZOOM = 1 << 19,
  IN_WEAPON1 = 1 << 20,
  IN_WEAPON2 = 1 << 21,
  IN_BULLRUSH = 1 << 22,
  IN_GRENADE1 = 1 << 23,
  IN_GRENADE2 = 1 << 24
};

enum class_ids{
  CTestTraceline = 211,
  CTEWorldDecal = 212,
  CTESpriteSpray = 209,
  CTESprite = 208,
  CTESparks = 207,
  CTESmoke = 206,
  CTEShowLine = 204,
  CTEProjectedDecal = 202,
  CTEPlayerDecal = 201,
  CTEPhysicsProp = 199,
  CTEParticleSystem = 198,
  CTEMuzzleFlash = 197,
  CTELargeFunnel = 195,
  CTEKillPlayerAttachments = 194,
  CTEImpact = 193,
  CTEGlowSprite = 191,
  CTEShatterSurface = 203,
  CTEFootprintDecal = 189,
  CTEFizz = 188,
  CTEExplosion = 187,
  CTEEnergySplash = 186,
  CTEEffectDispatch = 185,
  CTEDynamicLight = 184,
  CTEDecal = 182,
  CTEClientProjectile = 180,
  CTEBubbleTrail = 179,
  CTEBubbles = 178,
  CTEBSPDecal = 177,
  CTEBreakModel = 176,
  CTEBloodStream = 175,
  CTEBloodSprite = 174,
  CTEBeamSpline = 173,
  CTEBeamRingPoint = 172,
  CTEBeamRing = 171,
  CTEBeamPoints = 170,
  CTEBeamLaser = 169,
  CTEBeamFollow = 168,
  CTEBeamEnts = 167,
  CTEBeamEntPoint = 166,
  CTEBaseBeam = 165,
  CTEArmorRicochet = 164,
  CTEMetalSparks = 196,
  CSteamJet = 157,
  CSmokeStack = 152,
  DustTrail = 261,
  CFireTrail = 59,
  SporeTrail = 268,
  SporeExplosion = 267,
  RocketTrail = 265,
  SmokeTrail = 266,
  CPropVehicleDriveable = 135,
  ParticleSmokeGrenade = 264,
  CParticleFire = 115,
  MovieExplosion = 263,
  CTEGaussExplosion = 190,
  CEnvQuadraticBeam = 52,
  CEmbers = 44,
  CEnvWind = 57,
  CPrecipitation = 130,
  CBaseTempEntity = 23,
  CWeaponStunStick = 254,
  CWeaponSMG1 = 252,
  CWeapon_SLAM = 224,
  CWeaponShotgun = 250,
  CWeaponRPG = 248,
  CLaserDot = 99,
  CWeaponPistol = 247,
  CWeaponPhysCannon = 246,
  CBaseHL2MPCombatWeapon = 15,
  CBaseHL2MPBludgeonWeapon = 14,
  CHL2MPMachineGun = 90,
  CWeaponHL2MPBase = 244,
  CWeaponFrag = 243,
  CWeaponCrowbar = 235,
  CWeaponCrossbow = 234,
  CWeaponAR2 = 226,
  CWeapon357 = 223,
  CHL2MPGameRulesProxy = 89,
  CTEHL2MPFireBullets = 192,
  CTEPlayerAnimEvent = 200,
  CHL2MPRagdoll = 91,
  CHL2MP_Player = 88,
  CHalfLife2Proxy = 83,
  CCrossbowBolt = 39,
  CWeaponCubemap = 236,
  CWeaponAnnabelle = 225,
  CWeaponBugBait = 231,
  CWeaponCycler = 237,
  CVortigauntEffectDispel = 217,
  CVortigauntChargeToken = 216,
  CNPC_Vortigaunt = 114,
  CPropVehiclePrisonerPod = 136,
  CPropCrane = 134,
  CPropAirboat = 132,
  CFlare = 61,
  CTEConcussiveExplosion = 181,
  CNPC_Strider = 113,
  CScriptIntro = 149,
  CRotorWashEmitter = 143,
  CPropCombineBall = 133,
  CNPC_RollerMine = 112,
  CNPC_Manhack = 111,
  CNPC_CombineGunship = 110,
  CNPC_AntlionGuard = 107,
  CInfoTeleporterCountdown = 97,
  CMortarShell = 106,
  CEnvStarfield = 55,
  CEnvHeadcrabCanister = 49,
  CAlyxEmpEffect = 2,
  CCorpse = 38,
  CCitadelEnergyCore = 33,
  CHL2_Player = 87,
  CBaseHLBludgeonWeapon = 16,
  CHLSelectFireMachineGun = 93,
  CHLMachineGun = 92,
  CBaseHelicopter = 13,
  CNPC_Barney = 109,
  CNPC_Barnacle = 108,
  CTEAntlionDust = 163,
  CBaseHLCombatWeapon = 17,
  CWeaponYellowboy = 256,
  CWeaponXBow = 255,
  CBaseWhiskey = 26,
  CWhiskey2 = 258,
  CWhiskey1 = 257,
  CWalker2 = 220,
  CWalker1 = 219,
  CVolcanic2 = 215,
  CVolcanic1 = 214,
  CWeaponSpencer = 253,
  CWeaponShotgunFoF = 251,
  CWeaponSharps1874 = 249,
  CSchofield2 = 148,
  CSchofield1 = 147,
  CSawedShotgun2 = 145,
  CSawedShotgun1 = 144,
  CRemington_Army2 = 141,
  CRemington_Army = 140,
  CPeacemaker2 = 119,
  CPeacemaker1 = 118,
  CMauser2 = 105,
  CMauser1 = 104,
  CMaresLeg2 = 102,
  CMaresLeg1 = 101,
  CWeaponMachete = 245,
  CHammerless2 = 85,
  CHammerless1 = 84,
  CGhostGun2 = 82,
  CGhostGun1 = 81,
  CWeaponFistsGhost = 242,
  CWeaponFists = 241,
  CWeaponDynamiteBlack = 240,
  CWeaponDynamiteBelt = 239,
  CWeaponDynamite = 238,
  CDeringer2 = 41,
  CDeringer1 = 40,
  CColtNavy2 = 37,
  CColtNavy1 = 36,
  CWeaponCoachgun = 233,
  CWeaponCarbine = 232,
  CWeaponBowie = 230,
  CWeaponBowarrowBlack = 229,
  CWeaponBowarrow = 228,
  CWeaponAxe = 227,
  CFoFBaseRevolver = 64,
  CFoF_Horse = 62,
  CFoFPushCart = 66,
  CFoFCapEnt = 65,
  FoF_Crate = 262,
  CFuncRespawnRoomVisualizer = 76,
  CBBMulti = 27,
  CKnifeBolt = 98,
  CXArrow = 260,
  CBowarrowBolt = 30,
  CAxeBolt = 3,
  CFoF_Player = 63,
  CBaseGhost = 11,
  CHandleTest = 86,
  CTeamplayRoundBasedRulesProxy = 160,
  CTeamRoundTimer = 161,
  CSpriteTrail = 156,
  CSpriteOriented = 155,
  CSprite = 154,
  CRagdollPropAttached = 139,
  CRagdollProp = 138,
  CPredictedViewModel = 131,
  CPoseController = 129,
  CGameRulesProxy = 80,
  CInfoLadderDismount = 94,
  CFuncLadder = 72,
  CEnvDetailController = 48,
  CWorld = 259,
  CWaterLODControl = 222,
  CWaterBullet = 221,
  CVoteController = 218,
  CVGuiScreen = 213,
  CTest_ProxyToggle_Networkable = 210,
  CTesla = 205,
  CTeamTrainWatcher = 162,
  CBaseTeamObjectiveResource = 22,
  CTeam = 159,
  CSun = 158,
  CParticlePerformanceMonitor = 116,
  CSpotlightEnd = 153,
  CSlideshowDisplay = 151,
  CShadowControl = 150,
  CSceneEntity = 146,
  CRopeKeyframe = 142,
  CRagdollManager = 137,
  CPhysicsPropMultiplayer = 123,
  CPhysBoxMultiplayer = 121,
  CBasePropDoor = 21,
  CDynamicProp = 43,
  CPointCommentaryNode = 128,
  CPointCamera = 127,
  CPlayerResource = 126,
  CPlasma = 125,
  CPhysMagnet = 124,
  CPhysicsProp = 122,
  CPhysBox = 120,
  CParticleSystem = 117,
  CMaterialModifyControl = 103,
  CLightGlow = 100,
  CInfoOverlayAccessor = 96,
  CFuncTrackTrain = 79,
  CFuncSmokeVolume = 78,
  CFuncRotating = 77,
  CFuncReflectiveGlass = 75,
  CFuncOccluder = 74,
  CFuncMonitor = 73,
  CFunc_LOD = 69,
  CTEDust = 183,
  CFunc_Dust = 68,
  CFuncConveyor = 71,
  CBreakableSurface = 32,
  CFuncAreaPortalWindow = 70,
  CFish = 60,
  CEntityFlame = 46,
  CFireSmoke = 58,
  CEnvTonemapController = 56,
  CEnvScreenEffect = 53,
  CEnvScreenOverlay = 54,
  CEnvProjectedTexture = 51,
  CEnvParticleScript = 50,
  CFogController = 67,
  CEntityParticleTrail = 47,
  CEntityDissolve = 45,
  CDynamicLight = 42,
  CColorCorrectionVolume = 35,
  CColorCorrection = 34,
  CBreakableProp = 31,
  CBaseToggle = 24,
  CBasePlayer = 19,
  CBaseFlex = 10,
  CBaseEntity = 9,
  CBaseDoor = 8,
  CBaseCombatCharacter = 6,
  CBaseAnimatingOverlay = 5,
  CBoneFollower = 29,
  CBaseAnimating = 4,
  CInfoLightingRelative = 95,
  CAI_BaseNPC = 1,
  CBeam = 28,
  CBaseViewModel = 25,
  CBaseProjectile = 20,
  CBaseParticleEntity = 18,
  CBaseGrenade = 12,
  CBaseCombatWeapon = 7,
};

enum player_movetype {
  MOVETYPE_NONE = 0,
  MOVETYPE_ISOMETRIC,
  MOVETYPE_WALK,
  MOVETYPE_STEP,
  MOVETYPE_FLY,
  MOVETYPE_FLYGRAVITY,
  MOVETYPE_VPHYSICS,
  MOVETYPE_PUSH,
  MOVETYPE_NOCLIP,
  MOVETYPE_LADDER,
  MOVETYPE_OBSERVER,
  MOVETYPE_CUSTOM,
  MOVETYPE_LAST = MOVETYPE_CUSTOM,
  MOVETYPE_MAX_BITS = 4
};

class s_bf_write {
public:
  u32  data;
  i32  data_bytes;
  i32  data_bits;
  i32  cur_bit;
  bool overflow;
  bool assert_on_overflow;
  i8*  debug_name;

  void start_writing( void* _data, i32 bytes, i32 start_bit, i32 bits ) {
    bytes &= ~3;

    data       = ( u32 )_data;
    data_bytes = bytes;

    if ( bits == -1 )
      data_bits = bytes << 3;
    else
      data_bits = bits;

    cur_bit  = start_bit;
    overflow = false;
  }
};

class s_bf_read{
public:
  u32  data;
  i32  data_bytes;
  i32  data_bits;
  i32  cur_bit; //
  bool overflow;
  bool assert_on_overflow;
  i8*  debug_name;

  template < typename T = u8 > T read( i32 size = -1 ) {
    if ( size < 0 )
      size = sizeof( T );

    auto v = *(T*)( data );
    data += size;
    cur_bit += size;

    return v;
  }

  i8* read_str(bool skip_start = true) {
    // cuts off the start of the string.
    if(skip_start)
      data++;

    u32 str_len = strlen(data);

    i8* str_buf = data;

    data += str_len + 1;
    if(skip_start)
      cur_bit += str_len + 2;
    else
      cur_bit += str_len + 1;

    return str_buf;
  }

  void reset(){
    data -= cur_bit;
    cur_bit = 0;
  }

  i32 get_num_bytes_left(){
    return data_bytes - cur_bit;
  }
};

#pragma pack(push, 1)
class c_clc_move{
public:
  u32 rtti = 0;
  PAD( 0xC );
  i32 backup_commands = 0;
  i32 new_commands    = 0;
  PAD( 0x1C );
  s_bf_write data_out{};
  i32        unk0 = 1;
  i32        unk1 = 0;
  i32        unk2 = 0;
  PAD( 0x4 );
};
#pragma pack(pop)

struct s_recv_prop{
  i8*                 name;
  u32                 type;
  u32                 flags;
  i32                 string_buffer_len;
  bool                inside_aray;
  void*               extra_data;
  s_recv_prop*        array_prop;
  void*               array_length_proxy;
  void*               proxy;
};

struct s_send_prop{
  union{
    DATA(0x18, s_send_prop* array_prop);
    DATA(0x1C, void* array_length_proxy);
    DATA(0x30, i8* var_name);
    DATA(0x38, i32 flags);
  };
};

struct s_decode_info{
  // CRecvProxyData
  s_recv_prop*  recv_prop;
  PAD(16);
  i32           element_id;
  i32           object_id;

  // DecodeInfo
  void*         struct_base;
  void*         data;
  s_send_prop*  send_prop;
  s_bf_read*    in;
  i8            temp_str[19];
};

struct s_ray{
  vec3  start;
  float start_z;
  vec3  delta;
  float delta_z;
  vec3  start_offset;
  float start_offset_z;
  vec3  extents;
  float extens_z;
  bool  is_ray;
  bool  is_swept;
};

struct s_plane{
  vec3  normal;
  float dist;
  u8    type;
  u8    sign_bits;
  PAD(0x2);
};

enum trace_type {
  TRACE_EVERYTHING = 0,
  TRACE_WORLD_ONLY,
  TRACE_ENTITIES_ONLY,
  TRACE_EVERYTHING_FILTER_PROPS,
};

struct s_surface{
  i8* name;
  i16 props;
  u16 flags;
};

struct s_trace{
  vec3            start;
  vec3            end;
  s_plane         plane;
  float           fraction;
  i32             contents;
  u16             disp_flags;
  bool            solid;
  bool            started_solid;
  float           fraction_left_solid;
  s_surface       surface;
  i32             hit_group;
  i16             physics_bone;
  c_base_entity*  hit_entity;
  i32             hitbox;

  ALWAYSINLINE bool vis(void* ent = nullptr){
    return fraction >= 1.f || hit_entity == ent;
  }
};

enum trace_mask {
  mask_all                    = 0xFFFFFFFF,
  mask_solid                  = 0x200400B,
  mask_playersolid            = 0x201400B,
  mask_npcsolid               = 0x202400B,
  mask_npcfluid               = 0x2024003,
  mask_water                  = 0x4030,
  mask_opaque                 = 0x40A0,
  mask_opaque_npc             = 0x20040A0,
  mask_blocklos               = 0x4011,
  mask_blocklos_npc           = 0x2004011,
  mask_visible                = 0x60A0,
  mask_visible_npc            = 0x20060A0,
  mask_shot                   = 0x46004003,
  mask_shot_brushonly         = 0x4004003,
  mask_shot_hull              = 0x600400B,
  mask_shot_portal            = 0x2004003,
  mask_solid_brushonly        = 0x400B,
  mask_playersolid_brushonly  = 0x1400B,
  mask_npcsolid_brushonly     = 0x2400B,
  mask_npcworldstatic         = 0x2000B,
  mask_npcworldstatic_fluid   = 0x20003,
  mask_splitareaportal        = 0x30,
  mask_current                = 0xFC0000,
  mask_deadsolid              = 0x1000B,

  // ray trace bullet mask
  mask_bullet                 = 0x46004023,
  mask_explosion              = 0x6004003,
};

struct s_studio_bbox{
  i32 bone;
  i32 group;
  vec3 mins;
  vec3 maxs;
  i32  hitbox_name_index;
  PAD(0xC);
  float radius;
  PAD(0x10);
};

struct s_studio_hitbox_set{
  i32 name_index;
  i32 num_hitboxes;
  i32 hitbox_index;

  ALWAYSINLINE s_studio_bbox* get_hitbox(i32 i){
    return (s_studio_bbox*)((void*)this + hitbox_index) + i;
  }
};

struct s_studio_hdr{
  i32     id;
  i32     version;
  i32     checksum;
  i8      name[64];
  i32     len;
  vec3    eye_pos;
  vec3    illum_pos;
  vec3    hull_min;
  vec3    hull_max;
  vec3    view_bbmin;
  vec3    view_bbmax;
  i32     flags;
  i32     num_bones;
  i32     bone_index;
  i32     num_bone_controllers;
  i32     bone_controller_index;
  i32     num_hitbox_sets;
  i32     hitbox_set_index;

  ALWAYSINLINE s_studio_hitbox_set* get_hitbox_set(i32 i){
    return (s_studio_hitbox_set*)((void*)this + hitbox_set_index) + i;
  }
};

struct s_var_map_entry{
  u16     type;
  u16     needs_interpolation;
  void*   data;
  void*   watcher;
};

class css_weapon_file_info;
#include "utlvector.h"
#include "netvars.h"
#include "game_events.h"
#include "client_renderable.h"
#include "net_message.h"
#include "base_entity.h"
#include "base_player.h"
#include "base_weapon.h"

class c_game_rules{
public:
  union{
    DATA(DT_HL2MPRules_m_bTeamPlayEnabled, bool is_teamplay_enabled);
  };
};

struct s_fire_bullets_info{
  i32             shots;
  vec3            src;
  vec3            dir;
  vec3            spread;
  float           distance;
  i32             ammo_type;
  i32             tracer_freq;
  float           damage;
  i32             player_damage;
  i32             flags;
  float           damage_force_scale;
  c_base_entity*  attacker;
  c_base_entity*  ignore_ent;
  bool            primary_attack;
  bool            use_server_random_seed;
};

class css_weapon_file_info{
public:
  union{
    DATA(0x6C0, i32 ammo_type);
    DATA(0x888, i32 damage);
    DATA(0x88C, float range);
    DATA(0x890, float range_modifier);
    DATA(0x884, i32 penetration);
    DATA(0x894, i32 bullets);
  };
};

class c_interface_steamclient;
class c_interface_steamfriends;
class c_interface_steamuser;
class c_steamapicontext
{
public:
  c_interface_steamclient* steamclient; //0x0000
  c_interface_steamuser* steamuser; //0x0004
  c_interface_steamfriends* steamfriends; //0x0008
  void* steamutils; //0x000C
  void* steammatchmaking; //0x0010
  void* steamgamesearch; //0x0014
  void* steamuserstats; //0x0018
  void* steamapps; //0x001C
  void* steam_matching_servers; //0x0020
  void* steam_networking; //0x0024
  void* steam_remote_storage; //0x0028
  void* steam_screenshot; //0x002C
  void* steam_http; //0x0030
  void* steamcontroller; //0x0034
  void* steamugc; //0x0038
  void* steamapplist; //0x003C
  void* steammusic; //0x0040
  void* steam_music_remote; //0x0044
};

class c_steamid{
public:
  union u_steamid{
    struct s_steam{
      u32 account_id : 32;
      u32 account_instance : 20;
      u32 account_type : 4;
      i32 universe : 8;
    } comp;

    u64 bits;
  } m_steamid;
};

enum query_cvar_status{
  query_cvar_value_intact = 0,
  query_cvar_value_not_found = 1,
  query_cvar_value_not_a_cvar = 2,
  query_cvar_value_protected = 3,
};

struct s_respond_cvar_value{
  union{
    DATA(0x14, i8* name);
    DATA(0x18, i8* val);
    DATA(0x1C, i32 query_status);
  };
};

struct s_svc_print{
  union{
    DATA(0x10, i8* txt);
  };
};

struct s_clc_client_info_msg {
  i32  sendtable_crc;
  i32  servercount;
  bool is_replay;
  bool is_hltv;
  i8   pad[ 2 ];
  i32  friendsid;
  i8   friendsname[ 32 ];
  i32  custom_files[ 4 ];
};

class c_net_message_base {
public:
  i32   rtti;
  i32   reliable;
  void* net_channel;
  PAD( 4 )
};

enum material_flags {
  MATERIAL_VAR_DEBUG = ( 1 << 0 ),
  MATERIAL_VAR_NO_DEBUG_OVERRIDE = ( 1 << 1 ),
  MATERIAL_VAR_NO_DRAW = ( 1 << 2 ),
  MATERIAL_VAR_USE_IN_FILLRATE_MODE = ( 1 << 3 ),

  MATERIAL_VAR_VERTEXCOLOR = ( 1 << 4 ),
  MATERIAL_VAR_VERTEXALPHA = ( 1 << 5 ),
  MATERIAL_VAR_SELFILLUM = ( 1 << 6 ),
  MATERIAL_VAR_ADDITIVE = ( 1 << 7 ),
  MATERIAL_VAR_ALPHATEST = ( 1 << 8 ),
  MATERIAL_VAR_MULTIPASS = ( 1 << 9 ),
  MATERIAL_VAR_ZNEARER = ( 1 << 10 ),
  MATERIAL_VAR_MODEL = ( 1 << 11 ),
  MATERIAL_VAR_FLAT = ( 1 << 12 ),
  MATERIAL_VAR_NOCULL = ( 1 << 13 ),
  MATERIAL_VAR_NOFOG = ( 1 << 14 ),
  MATERIAL_VAR_IGNOREZ = ( 1 << 15 ),
  MATERIAL_VAR_DECAL = ( 1 << 16 ),
  MATERIAL_VAR_ENVMAPSPHERE = ( 1 << 17 ),
  MATERIAL_VAR_NOALPHAMOD = ( 1 << 18 ),
  MATERIAL_VAR_ENVMAPCAMERASPACE = ( 1 << 19 ),
  MATERIAL_VAR_BASEALPHAENVMAPMASK = ( 1 << 20 ),
  MATERIAL_VAR_TRANSLUCENT = ( 1 << 21 ),
  MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK = ( 1 << 22 ),
  MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING = ( 1 << 23 ),
  MATERIAL_VAR_OPAQUETEXTURE = ( 1 << 24 ),
  MATERIAL_VAR_ENVMAPMODE = ( 1 << 25 ),
  MATERIAL_VAR_SUPPRESS_DECALS = ( 1 << 26 ),
  MATERIAL_VAR_HALFLAMBERT = ( 1 << 27 ),
  MATERIAL_VAR_WIREFRAME = ( 1 << 28 ),
  MATERIAL_VAR_ALLOWALPHATOCOVERAGE = ( 1 << 29 ),
  MATERIAL_VAR_IGNORE_ALPHA_MODULATION = ( 1 << 30 ),
};

enum render_group_config
{
  // Number of buckets that are used to hold opaque entities
  // and opaque static props by size. The bucketing should be used to reduce overdraw.
  RENDER_GROUP_CFG_NUM_OPAQUE_ENT_BUCKETS = 4,
};

enum render_group{
  RENDER_GROUP_OPAQUE_STATIC_HUGE     = 0,    // Huge static prop
  RENDER_GROUP_OPAQUE_ENTITY_HUGE     = 1,    // Huge opaque entity
  RENDER_GROUP_OPAQUE_STATIC = RENDER_GROUP_OPAQUE_STATIC_HUGE + ( RENDER_GROUP_CFG_NUM_OPAQUE_ENT_BUCKETS - 1 ) * 2,
  RENDER_GROUP_OPAQUE_ENTITY,         // Opaque entity (smallest size, or default)

  RENDER_GROUP_TRANSLUCENT_ENTITY,
  RENDER_GROUP_TWOPASS,           // Implied opaque and translucent in two passes
  RENDER_GROUP_VIEW_MODEL_OPAQUE,       // Solid weapon view models
  RENDER_GROUP_VIEW_MODEL_TRANSLUCENT,    // Transparent overlays etc

  RENDER_GROUP_OPAQUE_BRUSH,          // Brushes

  RENDER_GROUP_OTHER,             // Unclassfied. Won't get drawn.

  // This one's always gotta be last
  RENDER_GROUP_COUNT
};

struct s_client_renderables_entry{
  void* renderable;
  u16   world_list_info_leaf;
  u16   two_pass;
  u16   render_handle;
  PAD(0x2);
};

struct s_client_renderables_list{
  PAD(0x8);
  s_client_renderables_entry render_groups[RENDER_GROUP_COUNT][4096];
  i32                        render_group_counts[RENDER_GROUP_COUNT];
};

struct s_setup_render_info{
  void*                       world_list_info;
  s_client_renderables_list*  render_list;
  vec3                        render_origin;
  vec3                        render_forward;
  i32                         render_frame;
  i32                         detail_build_frame;
  float                       render_dist_sq;
  bool                        draw_detail_objects;
  bool                        draw_translucent_objects;
};

struct s_model_render_data{
  vec3        origin;
  vec3        angles;
  void*       renderable;
  void*       model;
  matrix3x4*  model_to_world;
  matrix3x4*  lighting_offset;
  vec3*       lighting_origin;
  i32         flags;
  i32         entity_index;
  i32         skin;
  i32         body;
  i32         hitbox_set;
  u16         instance;
};

struct s_draw_model_info{
  union{
    DATA(0x18, c_client_renderable* client_entity); // renderable
  };
};

enum shader_stencil_op{
  STENCILOPERATION_KEEP = 1,
  STENCILOPERATION_ZERO = 2,
  STENCILOPERATION_REPLACE = 3,
  STENCILOPERATION_INCRSAT = 4,
  STENCILOPERATION_DECRSAT = 5,
  STENCILOPERATION_INVERT = 6,
  STENCILOPERATION_INCR = 7,
  STENCILOPERATION_DECR = 8,
};

enum stencil_comparison_function{
  STENCILCOMPARISONFUNCTION_NEVER = 1,
  STENCILCOMPARISONFUNCTION_LESS = 2,
  STENCILCOMPARISONFUNCTION_EQUAL = 3,
  STENCILCOMPARISONFUNCTION_LESSEQUAL = 4,
  STENCILCOMPARISONFUNCTION_GREATER = 5,
  STENCILCOMPARISONFUNCTION_NOTEQUAL = 6,
  STENCILCOMPARISONFUNCTION_GREATEREQUAL = 7,
  STENCILCOMPARISONFUNCTION_ALWAYS = 8,
  STENCILCOMPARISONFUNCTION_FORCE_DWORD = 0x7FFFFFFF
};

struct s_stencil_state{
  s_stencil_state()
  {
    enable = false;
    pass_op = fail_op = zfail_op = STENCILOPERATION_KEEP;
    compare_func = STENCILCOMPARISONFUNCTION_ALWAYS;
    reference_value = 0;
    test_mask = write_mask = 0xFFFFFFFF;
  }

  bool                        enable;
  shader_stencil_op           fail_op;
  shader_stencil_op           zfail_op;
  shader_stencil_op           pass_op;
  stencil_comparison_function compare_func;
  i32                         reference_value;
  u32                         test_mask;
  u32                         write_mask;
};

enum compiled_vtf_flags
{
  // flags from the *.txt config file
  TEXTUREFLAGS_POINTSAMPLE = 0x00000001,
  TEXTUREFLAGS_TRILINEAR = 0x00000002,
  TEXTUREFLAGS_CLAMPS = 0x00000004,
  TEXTUREFLAGS_CLAMPT = 0x00000008,
  TEXTUREFLAGS_ANISOTROPIC = 0x00000010,
  TEXTUREFLAGS_HINT_DXT5 = 0x00000020,
  TEXTUREFLAGS_SRGB = 0x00000040,
  TEXTUREFLAGS_NORMAL = 0x00000080,
  TEXTUREFLAGS_NOMIP = 0x00000100,
  TEXTUREFLAGS_NOLOD = 0x00000200,
  TEXTUREFLAGS_ALL_MIPS = 0x00000400,
  TEXTUREFLAGS_PROCEDURAL = 0x00000800,

  // These are automatically generated by vtex from the texture data.
  TEXTUREFLAGS_ONEBITALPHA = 0x00001000,
  TEXTUREFLAGS_EIGHTBITALPHA = 0x00002000,

  // newer flags from the *.txt config file
  TEXTUREFLAGS_ENVMAP = 0x00004000,
  TEXTUREFLAGS_RENDERTARGET = 0x00008000,
  TEXTUREFLAGS_DEPTHRENDERTARGET = 0x00010000,
  TEXTUREFLAGS_NODEBUGOVERRIDE = 0x00020000,
  TEXTUREFLAGS_SINGLECOPY = 0x00040000,

  TEXTUREFLAGS_STAGING_MEMORY = 0x00080000,
  TEXTUREFLAGS_IMMEDIATE_CLEANUP = 0x00100000,
  TEXTUREFLAGS_IGNORE_PICMIP = 0x00200000,

  TEXTUREFLAGS_UNUSED_00400000 = 0x00400000,

  TEXTUREFLAGS_NODEPTHBUFFER = 0x00800000,

  TEXTUREFLAGS_UNUSED_01000000 = 0x01000000,

  TEXTUREFLAGS_CLAMPU = 0x02000000,

  TEXTUREFLAGS_VERTEXTEXTURE = 0x04000000,          // Useable as a vertex texture

  TEXTUREFLAGS_SSBUMP = 0x08000000,

  TEXTUREFLAGS_UNUSED_10000000 = 0x10000000,

  // Clamp to border color on all texture coordinates
  TEXTUREFLAGS_BORDER = 0x20000000,

  TEXTUREFLAGS_STREAMABLE_COARSE = 0x40000000,
  TEXTUREFLAGS_STREAMABLE_FINE = 0x80000000,
  TEXTUREFLAGS_STREAMABLE = (TEXTUREFLAGS_STREAMABLE_COARSE | TEXTUREFLAGS_STREAMABLE_FINE)
};

enum render_target_size_mode
{
  RT_SIZE_NO_CHANGE = 0,      // Only allowed for render targets that don't want a depth buffer
  // (because if they have a depth buffer, the render target must be less than or equal to the size of the framebuffer).
  RT_SIZE_DEFAULT = 1,        // Don't play with the specified width and height other than making sure it fits in the framebuffer.
  RT_SIZE_PICMIP = 2,       // Apply picmip to the render target's width and height.
  RT_SIZE_HDR = 3,          // frame_buffer_width / 4
  RT_SIZE_FULL_FRAME_BUFFER = 4,  // Same size as frame buffer, or next lower power of 2 if we can't do that.
  RT_SIZE_OFFSCREEN = 5,      // Target of specified size, don't mess with dimensions
  RT_SIZE_FULL_FRAME_BUFFER_ROUNDED_UP = 6, // Same size as the frame buffer, rounded up if necessary for systems that can't do non-power of two textures.
  RT_SIZE_REPLAY_SCREENSHOT = 7,  // Rounded down to power of 2, essentially...
  RT_SIZE_LITERAL = 8,      // Use the size passed in. Don't clamp it to the frame buffer size. Really.
  RT_SIZE_LITERAL_PICMIP = 9    // Use the size passed in, don't clamp to the frame buffer size, but do apply picmip restrictions.
};

enum material_render_target_depth
{
  MATERIAL_RT_DEPTH_SHARED = 0x0,
  MATERIAL_RT_DEPTH_SEPARATE = 0x1,
  MATERIAL_RT_DEPTH_NONE = 0x2,
  MATERIAL_RT_DEPTH_ONLY = 0x3,
};

enum image_format
{
  IMAGE_FORMAT_UNKNOWN = -1,
  IMAGE_FORMAT_RGBA8888 = 0,
  IMAGE_FORMAT_ABGR8888,
  IMAGE_FORMAT_RGB888,
  IMAGE_FORMAT_BGR888,
  IMAGE_FORMAT_RGB565,
  IMAGE_FORMAT_I8,
  IMAGE_FORMAT_IA88,
  IMAGE_FORMAT_P8,
  IMAGE_FORMAT_A8,
  IMAGE_FORMAT_RGB888_BLUESCREEN,
  IMAGE_FORMAT_BGR888_BLUESCREEN,
  IMAGE_FORMAT_ARGB8888,
  IMAGE_FORMAT_BGRA8888,
  IMAGE_FORMAT_DXT1,
  IMAGE_FORMAT_DXT3,
  IMAGE_FORMAT_DXT5,
  IMAGE_FORMAT_BGRX8888,
  IMAGE_FORMAT_BGR565,
  IMAGE_FORMAT_BGRX5551,
  IMAGE_FORMAT_BGRA4444,
  IMAGE_FORMAT_DXT1_ONEBITALPHA,
  IMAGE_FORMAT_BGRA5551,
  IMAGE_FORMAT_UV88,
  IMAGE_FORMAT_UVWQ8888,
  IMAGE_FORMAT_RGBA16161616F,
  IMAGE_FORMAT_RGBA16161616,
  IMAGE_FORMAT_UVLX8888,
  IMAGE_FORMAT_R32F,      // Single-channel 32-bit floating point
  IMAGE_FORMAT_RGB323232F,
  IMAGE_FORMAT_RGBA32323232F,

  // Depth-stencil texture formats for shadow depth mapping
  IMAGE_FORMAT_NV_DST16,    //
  IMAGE_FORMAT_NV_DST24,    //
  IMAGE_FORMAT_NV_INTZ,   // Vendor-specific depth-stencil texture
  IMAGE_FORMAT_NV_RAWZ,   // formats for shadow depth mapping
  IMAGE_FORMAT_ATI_DST16,   //
  IMAGE_FORMAT_ATI_DST24,   //
  IMAGE_FORMAT_NV_NULL,   // Dummy format which takes no video memory

  // Compressed normal map formats
  IMAGE_FORMAT_ATI2N,     // One-surface ATI2N / DXN format
  IMAGE_FORMAT_ATI1N,     // Two-surface ATI1N format

  IMAGE_FORMAT_DXT1_RUNTIME,
  IMAGE_FORMAT_DXT5_RUNTIME,

  NUM_IMAGE_FORMATS
};
