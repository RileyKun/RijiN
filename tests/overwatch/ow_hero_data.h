#pragma once

enum {
	HERO_UNDEFINED = 0,
	HERO_DVA,
	HERO_ORISA,
	HERO_REINHARDT,
	HERO_ROADHOG,
	HERO_SIGMA,
	HERO_WINSTON,
	HERO_HAMMOND,
	HERO_ZARYA,
	HERO_ASHE,
	HERO_BASTION,
	HERO_DOOM,
	HERO_ECHO,
	HERO_GENJI,
	HERO_HANZO,
	HERO_JUNK,
	HERO_MCCREE,
	HERO_MEI,
	HERO_PHARAH,
	HERO_REAPER,
	HERO_SOLDIER,
	HERO_SOMBRA,
	HERO_SYM,
	HERO_TORB,
	HERO_TRACER,
	HERO_WIDOW,
	HERO_ANA,
	HERO_BAPTISTE,
	HERO_BRIG,
	HERO_LUCIO,
	HERO_MERCY,
	HERO_MOIRA,
	HERO_ZEN,
	HERO_TRAININGBOT1,
	HERO_TRAININGBOT2,
	HERO_TRAININGBOT3,
	HERO_TRAININGBOT4,
	HERO_MAX,
};

struct hero_data {
	char* name;
	
	int value;
	
	uint32_t color;
	
	float view_height;
	
	uint16_t shift_id;
	uint16_t e_id;
	uint16_t m2_id;
	//uint16_t ctrl_id;
	
	int max_health;
	int max_armor;
	int max_shields;
	
	int attack_numshots;
	float attack_damage;
	float attack_splash_min;
	float attack_splash_max;
	float attack_splash_radius;
	float attack_range;
	float attack_speed;
	float attack_spread;
	float attack_recovery;
	float attack_falloff_start;
	float attack_falloff_end;
	float attack_headshot;
	float attack_move_mult;
	
	float attack2_cooldown;
	float attack2_value;
	int attack2_numshots;
	float attack2_damage;
	float attack2_splash_min;
	float attack2_splash_max;
	float attack2_splash_radius;
	float attack2_range;
	float attack2_speed;
	float attack2_spread;
	float attack2_recovery;
	float attack2_falloff_start;
	float attack2_falloff_end;
	float attack2_headshot;
	float attack2_move_mult;
	
	float ability1_cooldown;
	float ability1_duration;
	float ability1_value;
	int ability1_numshots;
	float ability1_damage;
	float ability1_splash_min;
	float ability1_splash_max;
	float ability1_splash_radius;
	float ability1_range;
	float ability1_speed;
	float ability1_headshot;
	float ability1_move_mult;
	float ability1_cast_time;
	
	float ability2_cooldown;
	float ability2_duration;
	float ability2_value;
	int ability2_numshots;
	float ability2_damage;
	float ability2_splash_min;
	float ability2_splash_max;
	float ability2_splash_radius;
	float ability2_range;
	float ability2_speed;
	float ability2_headshot;
	float ability2_move_mult;
	float ability2_cast_time;
	
	float ult_value;
	float ult_duration;
	float ult_health;
	int ult_numshots;
	float ult_damage;
	float ult_splash_min;
	float ult_splash_max;
	float ult_splash_radius;
	float ult_range;
	float ult_speed;
	float ult_headshot;
	float ult_move_mult;
	float ult_cast_time;
};

enum {
	HEROID_REAPER = 0x02E0000000000002,
	HEROID_TRACER = 0x02E0000000000003,
	HEROID_MERCY = 0x02E0000000000004,
	HEROID_HANZO = 0x02E0000000000005,
	HEROID_TORBJORN = 0x02E0000000000006,
	HEROID_REINHARDT = 0x02E0000000000007,
	HEROID_PHARAH = 0x02E0000000000008,
	HEROID_WINSTON = 0x02E0000000000009,
	HEROID_WIDOWMAKER = 0x02E000000000000A,
	HEROID_BASTION = 0x02E0000000000015,
	HEROID_SYMMETRA = 0x02E0000000000016,
	HEROID_ZENYATTA = 0x02E0000000000020,
	HEROID_GENJI = 0x02E0000000000029,
	HEROID_ROADHOG = 0x02E0000000000040,
	HEROID_MCCREE = 0x02E0000000000042,
	HEROID_JUNKRAT = 0x02E0000000000065,
	HEROID_ZARYA = 0x02E0000000000068,
	HEROID_SOLDIER76 = 0x02E000000000006E,
	HEROID_LUCIO = 0x02E0000000000079,
	HEROID_DVA = 0x02E000000000007A,
	HEROID_MEI = 0x02E00000000000DD,
	HEROID_ANA = 0x02E000000000013B,
	HEROID_SOMBRA = 0x02E000000000012E,
	HEROID_ORISA = 0x02E000000000013E,
	HEROID_DOOMFIST = 0x02E000000000012F,
	HEROID_MOIRA = 0x02E00000000001A2,
	HEROID_BRIGITTE = 0x02E0000000000195,
	HEROID_WRECKINGBALL = 0x02E00000000001CA,
	HEROID_ASHE = 0x02E0000000000200,
	HEROID_BAPTISTE = 0x02E0000000000221,
	HEROID_SIGMA = 0x02E000000000023B,
	HEROID_ECHO = 0x02E0000000000206,
	HEROID_TRAININGBOT1 = 0x02E000000000016B,
	HEROID_TRAININGBOT2 = 0x02E000000000016C,
	HEROID_TRAININGBOT3 = 0x02E000000000016D,
	HEROID_TRAININGBOT4 = 0x02E000000000016E,
};

enum {
	POOLID_LOCALPLAYER = 0x040000000000000E,
	POOLID_PLAYER = 0x04000000000033DD,
	POOLID_TRAININGBOT = 0x0400000000001404,
	POOLID_TRAININGBOT1 = 0x0400000000001403,
	POOLID_TRAININGBOT4 = 0x04000000000014ED,
	POOLID_MCCREE_BOT = 0x04000000000015F0,
	POOLID_REAPER_BOT = 0x040000000000118E,
	POOLID_TORB_BOT = 0x0400000000001667,
	POOLID_BASTION_BOT = 0x0400000000000D57,
	POOLID_ZEN_BOT = 0x0400000000001468,
	POOLID_ROADHOG_BOT = 0x040000000000147E,
	POOLID_ZARYA_BOT = 0x04000000000014BA,
	POOLID_SOLDIER_BOT = 0x04000000000011F4,
	POOLID_LUCIO_BOT = 0x0400000000001635,
	POOLID_MEI_BOT = 0x0400000000001868,
	POOLID_SOMBRA_BOT = 0x040000000000178D,
	POOLID_ANA_BOT = 0x040000000000160D,
	
	POOLID_DVAMECH = 0x0400000000000407,
	POOLID_DVAPILOT = 0x040000000000040a,
	POOLID_ORISA = 0x0400000000001498,
	POOLID_REINHARDT = 0x0400000000000024,
	POOLID_ROADHOG = 0x040000000000028f,
	POOLID_SIGMA = 0x0400000000002ab1,
	POOLID_MONKEY = 0x04000000000001c0,
	POOLID_HAMSTER = 0x040000000000200b,
	POOLID_ZARYA = 0x04000000000003bf,
	POOLID_ASHE = 0x040000000000251c,
	POOLID_BASTION = 0x04000000000001fd,
	POOLID_MCCREE = 0x04000000000002a3,
	POOLID_DOOM = 0x0400000000001235,
	POOLID_ECHO = 0x0400000000002593,
	POOLID_GENJI = 0x0400000000000225,
	POOLID_HANZO = 0x0400000000000141,
	POOLID_JUNK = 0x0400000000000380,
	POOLID_MEI = 0x04000000000008ba,
	POOLID_PHARAH = 0x0400000000000032,
	POOLID_REAPER = 0x0400000000000022,
	POOLID_SOLDIER = 0x04000000000003cf,
	POOLID_SOMBRA = 0x040000000000121c,
	POOLID_SYM = 0x04000000000026f0,
	POOLID_TORB = 0x04000000000028b6,
	POOLID_TRACER = 0x0400000000000001,
	POOLID_WIDOW = 0x040000000000001c,
	POOLID_ANA = 0x0400000000001424,
	POOLID_BAPTISTE = 0x04000000000029a6,
	POOLID_BRIG = 0x0400000000001b1b,
	POOLID_LUCIO = 0x04000000000003fc,
	POOLID_MERCY = 0x04000000000000e4,
	POOLID_MOIRA = 0x0400000000001c46,
	POOLID_ZEN = 0x0400000000000216,
	
	POOLID_DVABOMB = 0x0400000000000407,
	POOLID_ORISASHIELD = 0x04000000000014A1,
	POOLID_REINSHIELD = 0x040000000000011A,
	POOLID_SIGSHIELD = 0x0400000000002D25,
	POOLID_MONKEYBUBBLE = 0x04000000000001C3,
	POOLID_SELFBUBBLE = 0x04000000000013F4,
	POOLID_ALLYBUBBLE = 0x04000000000003DA,
	
	POOLID_DYNAMITE = 0x0400000000002533,
	POOLID_TRANSLOCATOR = 0x0400000000001575,
	POOLID_TPENTER = 0x04000000000026F2,
	POOLID_TPEXIT = 0x040000000000272D,
	POOLID_WIDOWMINE = 0x04000000000000AE,
	
	POOLID_LAMP = 0x04000000000029A9,
};

enum {
	HITBOX_HEAD,
	HITBOX_CHEST,
	HITBOX_BODY,
/*	HITBOX_PELVIS,
	
	HITBOX_LSHOULDER,
	HITBOX_RSHOULDER,
	HITBOX_LARM,
	HITBOX_RARM,
	HITBOX_LHAND,
	HITBOX_RHAND,
	
	HITBOX_LTHIGH,
	HITBOX_RTHIGH,
	HITBOX_LCALF,
	HITBOX_RCALF,*/
	
	HITBOX_MAX
};

#define ENGINE_HITBOX_MAX 48

extern hero_data g_hero_data[HERO_MAX];
extern uint64_t g_pool_ids[128];

/*
	dva:
	*baby dva and mech have seperate health components, and parents*
	0x48 = last mech fire time
	0x49 = last mech fire time
	0x8c = on ground float
	0x6c = shooting float
	0x375 = move speed mult, == 2.180f boosters, == 0.6f shooting
	0x9d = time in air
	0x148 = matrix
	
	orisa:
	0xf7 = last fire time
	0x612 = last halt press
	0x615 = halt in air
	0x6c = firing float
	0xfe = start firing time
	0x61c = ammo consumed percentage
	0x375 = move speed mult
	0x618 = using gold float
	
	rein: 
	0x375 = move speed mult
	0x8c = onground float
	0x9d = time in air
	
	hog:
	0xf7 = last fire time
	0xf8 = last fire2 time
	0x222 = hog hook float, == 0.0f animation, == 1.0f no animation
	0x375 = move speed mult
	0x285 = hog hook float, == 1.0f throwing, == 2.0f hit enemy, == 3.0f hit wall, == 4.0f hit air
	0x286 = same as 0x285
	0x125 = same as 0x285
	0x25e = eye pitch ?
	0x82 = last hook start time
	0x83 = last hook end time
	
	sig:
	0x4 = fire float ?
	0x7b5 = move shield float
	0x680 = shield up float
	
	monkey:
	0x4a = ammo consumed percentage
	
	ham:
	0x48 = last fire time
	0x49 = last fire time
	0x6c = firing float
	0x6c4 = spread percentage
	0x61c = ammo consumed percentage
	0x37 = spread percent ?
	0x375 = move speed mult
	0x68 = using grapple
	0x285 = move mult ?
	
	zarya:
	0x6c = firing float
	0xf7 = last fire release time
	0xf8 = last fire2 time
	0xfe = time since beam started
	0x4a = ammo consumed percentage
	
	ashe:
	0x6d = ads float
	0x4a = ammo consumed percentage
	
	3 3d = e cooldown
	6 3d = shift cooldown
	
	bastion:
	0x149 = sentry cfg
	0x148 = healing float (0.0f/1.0f)
	
	doom:
	0x49 = last fire time
	0x5b3 = ammo reserve
	0x4a1 = holding attack float
	0x614 = rocket punch charge
	0x640 = uppercut float (0.0f/1.0f)
	0x63f = slam float (0.0f/1.0f)
	0x375 = move speed mult, == 3.0f in ult ? may be affected by lucio/brig speed
	
	0 0x21 = ult
	0 0x34a = doom invulnerability
	
	echo:
	0x4a = ammo consumed percentage
	
	0 0x21 = ult info
	5 0xd7 = echo duplicate info, check 0x21 for ult mode, then compare to when duplicate starts
	
	genji:
	0xf7 = last first shot fired
	0xf8 = last first shot fired
	0x4a = ammo reserve percentage
	*secondary not available in this struct*
	
	junk:
	0x0004 = fire ?
	0x4a = ammo reserve percentage
	
	cree:
	0x37 = fan, == 6.0f
	0xf7 = primary last shot
	0xf8 = secondary last shot
	0xe8 = fan float, (0.0f/1.0f)
	
	6 3d = shift cooldown
	7 3d = e cooldown
	
	mei:
	0x4a1 = firing float
	0xf7 = time since fire released
	0xf8 = time since fire2 released
	0xfe = time since started firing
	0x4a2 = is building icicle to fire
	0x56e = time since started firing, additive
	0x577 = time since started firing2, additive (max 1.0f)
	
	pharah:
	0x48 = last fire time
	0x4a = ammo reserve percentage
	0x5b3 = ammo count
	0x375 = move speed mult, == 1.2f boosting
	
	reaper:
	0x48 = right shotgun last fire time
	0x49 = left shotgun last fire time
	0x375 = move speed mult, == 1.5f reaper wraith
	
	soldier:
	0x6c = fire float
	0xf7 = time since last mouse1 held
	0xf8 = time since last helix
	0x4a = ammo consumed percentage
	0x34d = is reloading float (== 0.0f true, == 1.0f false)
	0x375 = move speed mult, == 1.5f sprinting
	0x393 = is helix on cooldown float (== 0.0f true, == 1.0f false)
	
	6 3d = e cooldown
	0xa 3d = helix cooldown
	
	sombra:
	0x37 = spread angle float
	0x6c = firing float
	0x4a = ammo consumed percentage
	
	sym:
	0x651 = right click charge amount
	0xf7 = time since last mouse1 released
	0x4a = ammo consumed percentage
	0x125 = beam charge mult (0.0f/1.0f/2.0f)
	0x6c = fire float
	0xfe = time spent beaming
	0xf8 = time since last right click
	0xe9 = beam last hit an enemy
	
	torb:
	0xf7 = time since fired shot
	0xf8 = time since fired 2nd
	0x375 = move speed mult, == 1.3f overload
	
	1 3d = e cooldown
	2 3d = blink cooldown
	
	tracer:
	0x36 = spread angle mult
	0x37 = spread angle float
	0x4a = ammo consumed percentage
	0x6c = shooting float
	1 0x2bb = in recall
	
	widow:
	0x48 = time since last scoped shot
	0x37 = spread angle float
	0x6c = shooting float
	0xa1 = widow charge percentage
	0x4a = ammo consumed percentage
	0x6d = ads time percentage
	0x375 = move speed mult, == 0.350f scoped
	0x85 = hook float ?
	
	ana:
	0x48 = last fired time
	0x4a = ammo reserve percentage
	0x6d = scope percentage
	0x375 = move speed mult, == 0.350f scoped
	
	bap:
	0x614 = basketball jump percentage
	0x4a = ammo consumed percentage
	0x61c = number of healing shots
	0xf7 = time since last fired burst
	0xf8 = time since last fired healing
	0x5b3 = ammo count
	
	brig:
	0x1e6 = shield float, == 0.0f shield up, == 1.0f shield down
	0x666 = swing angle ? > 0.0f swinging
	0x375 = move speed mult, == 0.7f shield up
	0x286 = boop logic float
	
	lucio:
	0x3d9 = healing aura == 1.0f, speed aura == 0.0f
	0x6c = bullets being fired float (0.0f/1.0f)
	0x4a = ammo consumed percentage
	0xf7 = time since burst finished
	0x375 = move speed ult, == 1.25f
	
	mercy:
	0x5e2 = == 2.0f blaster, == 1.0f staff
	0x6c = healing float
	0xe8 = damage boost float
	0xc7 = beam connected float
	0xc9 = beam floats (0.5f healing not connected, 1.0f healing connected, -0.5f damage not connected, -1.0f damage connected)
	0xe9 = beam connected float
	0xea = pressing beam button(s)
	
	moira:
	0x4a2 = right click float
	0xf7 = time since last started holding left click (+ 0.5 if right click pressed)
	0xf8 = same as 0xf7 but right click
	0x649 = healing resource
	0x4a1 = left click float
	0x659 = time spent healing
	0x375 = move speed mult, == 3.5f fade, == 1.5f ult
	
	zen:
	0x48 = time since fired right hand
	0x49 = time since fired left hand
	0xc9 = right click charge (increments of 0.2, uncapped to 1.4) (charge max at 1.0)
	0x123 = right click float (is charging right click)
	0x612 = time since fired left click
	
	0 0x21
	0 0x1d4f = zen invulnerability
	
	hanzo:
	0x48 = last arrow shot time
	0xc9 = arrow charge percentage
	0x69c = storm arrow
	0x8c = on ground float
	0x3001 = move speed mult
	0x149 = arrow type float, == 1.0f sonic, == 3.0f ultimate (dragon)
	
	ult charge
	0x1e32
*/