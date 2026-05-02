struct game_manager_t {
	union {
		char pad[0x100];
		OFIELD(0x20, uint64_t entity_admin);
		OFIELD(0xE0, uint64_t localplayer_pool);
	};
};

class object_pool {
public:
	uint64_t get_type() {
		uint64_t v1 = *(uint64_t*)((uint64_t)(this) + 0x30);
		if (v1 == -1)
			return 0;
		
		return *(uint64_t*)((v1 & 0xFFFFFFFFFFFFFFC0) + 0x10);
	}
	uint32_t get_link_id() {
		return *(uint32_t*)((uint64_t)(this) + 0xb0);
	}
};

struct map_element {
	uint64_t hash;
	void* pool;
	
	char pad3[0xC0];
};

#define ANGLE2SHORT(x) ((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))

#define MOVE2BYTE(x) ((int)((x)*127/100))
#define	BYTE2MOVE(x)	((x)*(100.0/127))

/*
	in usercmd hook:
	0x6 is yaw (quake)
	0x8 is pitch (quake)
	0x70 is view forward
	0x30 is buttons
	0x2b is sidemove
	0x2c is forwardmove
	
	input singleton + 0x1020 is start of usercmd
*/

enum {
	UCMD_FORWARDX = (1<<0),
	UCMD_FORWARDY = (1<<1),
	UCMD_FORWARDZ = (1<<2),
	UCMD_PITCH = UCMD_FORWARDY,
	UCMD_YAW = UCMD_FORWARDX|UCMD_FORWARDZ,
	UCMD_FORWARD = UCMD_FORWARDX|UCMD_FORWARDY|UCMD_FORWARDZ,
};

enum {
	IN_ATTACK = (1<<0),
	IN_ATTACK2 = (1<<1),
	IN_USE = (1<<2),
	IN_ABILITY1 = (1<<3),
	IN_ABILITY2 = (1<<4),
	IN_ABILITY3 = (1<<5),
	IN_JUMP = (1<<6),
	IN_DUCK = (1<<7),
	IN_RELOAD = (1<<10),
	IN_MELEE = (1<<11),
};

struct pseudo_cmd {
	Vector forward;
	Vector view;
	uint32_t buttons;
	uint8_t forwardmove; // forward = 7f, back = 81
	uint8_t sidemove; // left = 81, right = 7f
};

struct input_cmd {
	union {
		char pad[0x100];
		OFIELD(0x6, uint16_t yaw_hsort);
		OFIELD(0x8, uint16_t pitch_short);
		OFIELD(0x14, uint32_t buttons_final);
		OFIELD(0x4, uint8_t test);
		OFIELD(0x70, float forward_x);
		OFIELD(0x74, float forward_y);
		OFIELD(0x78, float forward_z);
		OFIELD(0x30, uint16_t buttons);
		OFIELD(0x2b, uint8_t sidemove);
		OFIELD(0x2c, uint8_t forwardmove);
	};
};

struct server_cmd {
	union {
		char pad[0x100];
		OFIELD(0x0, uint16_t time1);
		OFIELD(0x4, uint16_t time2); // 0x12
		OFIELD(0xA, uint16_t yaw_quake);
		OFIELD(0xC, uint16_t pitch_quake);
		OFIELD(0x18, uint16_t buttons);
		OFIELD(0x13, uint8_t sidemove);
		OFIELD(0x14, uint8_t forwardmove);
	};
}; // sizeof 0x24

/*struct input_cmd {
	union {
		char pad[0x1000];
		OFIELD(0x0, uint8_t air);
		OFIELD(0x20, uint8_t sidemove);
		OFIELD(0x21, uint8_t forwardmove);
		OFIELD(0x22, uint8_t last_sidemove);
		OFIELD(0x23, uint8_t last_forwardmove);
		OFIELD(0x42, uint16_t pitch);
		OFIELD(0x60, uint16_t yaw);
	};
};*/

enum {
	FILTER_PLAYER = (1<<0),
	FILTER_BOT = (1<<1),
	FILTER_INVULN = (1<<2), // when players enter spawn room, and applies to dynamite
	FILTER_DEAD = (1<<4),
	FILTER_DESTROYED1 = (1<<8),
	FILTER_DESTROYED2 = (1<<9),
	FILTER_HAS_OWNER = (1<<15),
	FILTER_DAMAGEABLE = (1<<16), // guessing
	FILTER_UNK1 = (1<<18), // 0 on friendlies
	FILTER_UNK2 = (1<<19),
	FILTER_MEIWALL = (1<<20),
	FILTER_DYNAMITE = (1<<21),
	FILTER_TEAM1 = (1<<23),
	FILTER_TEAM2 = (1<<24),
	FILTER_TEAMFFA = (1<<27),
};

/*
	velocity + 0x6f0 -> bone struct
	bone struct + 0x0 -> bone base
	bone base + 0x90 + (sizeof(bone_t) * i) -> bone_t
*/

struct sphere_t {
	union {
		char pad[64];
		OFIELD(0x8, float unknown);
		OFIELD(0x10, float radius);
		OFIELD(0x30, uint16_t skel_index);
		OFIELD(0x32, uint16_t hitbox_type); // speculation
	};
};

struct capsule_t {
	union {
		char pad[128];
		OFIELD(0x0, float start[4]);
		OFIELD(0x10, float end[4]);
		OFIELD(0x20, float radius);
		OFIELD(0x2C, uint32_t unknown);
		OFIELD(0x40, uint16_t skel_index);
		OFIELD(0x42, uint16_t hitbox_type); // speculation
	};
};

struct hitbox_context_t {
	union {
		char pad[0x100];
		OFIELD(0x8, void* sphere_data);
		OFIELD(0x10, void* capsule_data);
		OFIELD(0x28, uint32_t sphere_count);
		OFIELD(0x2C, uint32_t capsule_count);
		OFIELD(0x38, uint32_t hitbox_count);
	};
};

struct bone_t {
	Quat test;
	char pad2[0x10];
	Vector position;
};

struct skeleton_t {
	union {
		char pad[0x100];
		OFIELD(0x30, uint32_t* ids);
		OFIELD(0x56, uint16_t id_count);
	};
};

struct skill_t {
	union {
		char pad[0x100];
		OFIELD(0x0, uint16_t id);
		OFIELD(0x30, float var_fl);
		OFIELD(0x30, uint32_t var_u32);
	};
};

struct attribute_t {
	union {
		char pad[0x100];
		OFIELD(0x0, uint16_t id);
		OFIELD(0x30, float var_fl);
		OFIELD(0x48, uint32_t var_u32);
	};
};

struct ray_setup_t {
	union {
		char pad[0x208];
		OFIELD(0x0, float start_x);
		OFIELD(0x4, float start_y);
		OFIELD(0x8, float start_z);
		OFIELD(0x10, float end_x);
		OFIELD(0x14, float end_y);
		OFIELD(0x18, float end_z);
		OFIELD(0x58, uint64_t type);
		OFIELD(0x60, uint64_t entity_list);
		OFIELD(0x68, uint64_t entity_count);
		OFIELD(0x1C0, uint64_t gm);
		OFIELD(0x1B8, uint64_t ray_cmp);
	};
};

struct ray_result_t {
	union {
		char pad[0x208];
		OFIELD(0x0, float hitpos_x);
		OFIELD(0x4, float hitpos_y);
		OFIELD(0x8, float hitpos_z);
		OFIELD(0x10, float normal_x);
		OFIELD(0x14, float normal_y);
		OFIELD(0x18, float normal_z);
		OFIELD(0x28, uint32_t hit_flags);
		OFIELD(0x30, float fraction);
		OFIELD(0x38, uint16_t hitbox);
		OFIELD(0x3C, uint16_t entity_id); // speculation
		OFIELD(0x40, uint64_t head_var);
	};
};