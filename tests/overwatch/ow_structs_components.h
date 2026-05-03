enum {
	CMPID_INPUT_SINGLETON = 0x3b,
	CMPID_HEALTH = 0x33,
	CMPID_LINK = 0x2c,
	CMPID_VISIBLE = 0x2D,
	CMPID_PLAYERID = 0x2b,
	CMPID_VELOCITY = 0x4,
	CMPID_LERP = 0x18, // IDK That & i dont use 
	CMPID_FILTER_BITS = 0x19, // its just team check 
	CMPID_HERO_ID = 0x4b,
	CMPID_SKILL = 0x2f,
	CMPID_Border = 0x53,
	CMPID_Animate = 0x01,
	CMPID_ROTATION = 0x27,
};

class cmp_parent {
public:
	uint32_t get_link_id() {
		return *(uint32_t*)((uint64_t)(this) + 0xb0);
	}
};

class cmp_base {
public:
	uint64_t _vtable;
	uint64_t _parent;
	uint64_t _id;
};

class cmp_input : public cmp_base {
public:
	/*input_cmd* get_cmd() {
		return (input_cmd*)((uint64_t)(this) + 0x70);
	}*/
	// 0x1fe = ability preventing fire
	uint8_t get_reload_limit() {
		return *(uint8_t*)((uint64_t)(this) + 0x205); // reloading == 0x4
	}// + 8xf0 previous state (useless?)
	uint16_t get_ability_limit() {
		return *(uint16_t*)((uint64_t)(this) + 0x1fe);
	}
	// soldier ult & genji ult = 0x33
	// mccree ult = 0xb6
	// other character skkill = offsets 
	
};

class cmp_health : public cmp_base {
public:
	float get_health() {
		return *(float*)((uint64_t)(this) + 0xe0);
	}
	float get_max_health() {
		return *(float*)((uint64_t)(this) + 0xe0 - 0x4);
	}
	float get_armor() {
		return *(float*)((uint64_t)(this) + 0x220);
	}
	float get_max_armor() {
		return *(float*)((uint64_t)(this) + 0x220 - 0x4);
	}
	float get_shields() {
		return *(float*)((uint64_t)(this) + 0x360);
	}
	float get_max_shields() {
		return *(float*)((uint64_t)(this) + 0x360 - 0x4);
	}
	uint8_t bubble_state() {
		return *(uint8_t*)((uint64_t)(this) + 0x4a4);
	}
	uint8_t immortality_state() {
		return *(uint8_t*)((uint64_t)(this) + 0x4a5);
	}
	/*
	0x30 = heal state?
	OFF_HEROID = 0xD0,
	OFF_HEALTH = 0xE0,
	OFF_ARMOR = 0x220,
	OFF_BARRIER = 0x360,
	OFF_BARTIST_IMMORTAL = 0x4A5,
	OFF_ZARYA_BUBBLE = 0x4A4,
	OFF_IS_DEAD = 0x4D0,
	OFF_BATTLEPTR = 0xE0
	*/
};

class cmp_link : public cmp_base {
public:
	uint32_t get_link_id() {
		return *(uint32_t*)((uint64_t)(this) + 0xd0);
	}
};

class cmp_playerid : public cmp_base {
public:
};

class cmp_lerp : public cmp_base {
public:
	int get_index() {
		// idk 
		int best_tick = -1;
		int best_i = 0;
		
		for (int i = 0; i < 20; ++i) {
			int tick_count = *(int*)((uint64_t)(this) + 0x1d0 + (0x4 * (uint64_t)(i)));
			if (tick_count > best_tick) {
				best_i = i;
				best_tick = tick_count;
			}
		}
		
		return best_i;
	}
	Vector* get_position_at_index(int index) {
		return (Vector*)((uint64_t)(this) + 0x70 + (0x10 * (uint64_t)(index)));
	}
};

class cmp_velocity : public cmp_base {
public:
	Vector* get_eyepos_min() {
		return (Vector*)((uint64_t)(this) + 0x160);
	}
	Vector* get_eyepos_max() {
		return (Vector*)((uint64_t)(this) + 0x170);
	}
	float* get_nameplate_y() {
		return (float*)((uint64_t)(this) + 0x1d4);
	}
	Vector* get_velocity() {
		return (Vector*)((uint64_t)(this) + 0x50);
	}
	hitbox_context_t* get_hitbox_context() {
		return *(hitbox_context_t**)((uint64_t)(this) + 0xc0);
	}
	uint64_t get_bone_struct() {
		return *(uint64_t*)((uint64_t)(this) + 0x6f0);
	}
	bone_t* get_bone_by_index(int index) {
		uint64_t bone_struct = this->get_bone_struct();
		if (bone_struct == 0)
			return 0;
		
		uint64_t bone_raw = *(uint64_t*)(bone_struct + 0x28);
		bone_t* bone = (bone_t*)(bone_raw + ((uint64_t)(index) * 0x30)); 
		
		return bone;
	}
	int get_skeleton_index(uint32_t id) {
		uint64_t bone_struct = this->get_bone_struct();
		if (bone_struct == 0)
			return -1;
		
		skeleton_t* skel = *(skeleton_t**)(bone_struct + 0x0);
		for (int i = 0; i < (int)(skel->id_count); ++i) {
			if (id == skel->ids[i])
				return i;
		}
		
		return -1;
	}
	uint32_t get_skeleton_id(int index) {
		uint64_t bone_struct = this->get_bone_struct();
		if (bone_struct == 0)
			return 0;
		
		skeleton_t* skel = *(skeleton_t**)(bone_struct + 0x0);
		return skel->ids[index];
	}
	uint32_t* get_boder_struct() {
		return (uint32_t*)((uint64_t)(this) + 0x33c); 
	}
	uint32_t* get_bodercolor_struct() {
		return (uint32_t*)((uint64_t)(this) + 0x340); 
	}
};

class cmp_rotation : public cmp_base {
public:
	Vector get_rotation() {
		uint64_t rot = *(uint64_t*)((uint64_t)(this) + 0x748);
		if (rot == 0)
			return Vector(0.0f, 0.0f, 0.0f);
		
		return *(Vector*)(rot + 0xa78);
	}
};

class cmp_filter_bits : public cmp_base {
public:
	uint32_t get_bits() {
		return (*(uint32_t*)((uint64_t)(this) + 0x58)); //& 0x0F800000); // bts 17 = red, 18 blue
	}
};

class cmp_hero_id : public cmp_base {
public:
	uint64_t get_hero_id() {
		return *(uint64_t*)((uint64_t)(this) + 0xd0); // 0xe8 as well
	}
	uint64_t get_skin_id() {
		return *(uint64_t*)((uint64_t)(this) + 0xd8); // 0xf0 as well
	}
};

class cmp_skill : public cmp_base {
public:
	uint64_t get_skill_base() {
		uint64_t v1 = *(uint64_t*)((uint64_t)(this) + 0x2c0);
		if (v1 == 0)
			return;
		
		return *(uint64_t*)(v1 + 0x60);
	}
	uint64_t get_skill_struct() {
		return *(uint64_t*)((uint64_t)(this) + 0x1218);
	}
	skill_t* lookup_skill(uint16_t id) {
		uint64_t skill_str = this->get_skill_struct();
		if (skill_str == 0)
			return 0;
		
		uint64_t skill_base = *(uint64_t*)(skill_str + 0x10);
		uint16_t skill_max = *(uint16_t*)(skill_str + 0x18);
		
		for (int i = 0; i < (int)skill_max; ++i) {
			skill_t* sk = (skill_t*)(skill_base + (0x80 * (uint64_t)(i)));
			if (sk->id == id)
				return sk;
		}
		
		return 0;
	}
	bool get_skill_u32(uint16_t id, uint32_t* var) {
		skill_t* sk = this->lookup_skill(id);
		if (sk == 0)
			return false;
		
		if (var)
			*var = sk->var_u32;
		
		return true;
	}
	bool get_skill_float(uint16_t id, float* var) {
		skill_t* sk = this->lookup_skill(id);
		if (sk == 0)
			return false;
		
		if (var)
			*var = sk->var_fl;
		
		return true;
	}
	bool get_ult_charge(float* var) {
		uint64_t skill_base = this->get_skill_base();
		if (skill_base == 0)
			return false;
		
		for (int i = 0x4; i <= 0x6; ++i) {
			uint16_t id = *(uint16_t*)(skill_base + (i * 0x10));
			if (id == 0x1E32) {
				uint64_t ult_base = *(uint64_t*)(skill_base + (i * 0x10) + 0x8);
				if (ult_base != 0) {
					float ult_charge = *(float*)(ult_base + 0x60);
					*var = ult_charge;
					
					return true;
				}
			}
		}
		
		return true;
	}
};