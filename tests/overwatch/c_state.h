#pragma once

struct pseudo_ray_t {
	float fraction;
	uint16_t hitbox;
	bool head_hit;
	float length;
	
	float test_fraction;
	float test_length;
	Vector test_hitpos;
	
	Vector reflect_start;
	Vector reflect_end;
};

#define MAX_OBJECTS 64

enum {
	OBJECT_INVALID = 0,
	OBJECT_PLAYER,
	OBJECT_DYNAMITE,
	OBJECT_EQUIPMENT,
};

#define MAX_PLAYER_CONDS 10

struct ow_condition {
	uint32_t hash;
	union {
		float fl;
		uint32_t u32;
		uint64_t u64;
	} data;
};

struct ow_object {
	int index;
	uint8_t type;
	
	uint64_t entity_parent;
	uint64_t player_parent;
	
	uint32_t link_id;
	uint64_t pool_type;
	
	Vector position;
	Vector model_min;
	Vector model_max;
	Vector model_center;
	
	float nameplate_y;
	
	Vector velocity;
	Vector rotation;
	
	Vector forward;
	
	Vector position_root;
	Vector position_root_raw;
	
	Vector predicted_delta;
	int predicted_backward;
	
	uint32_t hitbox_map[HITBOX_MAX];
	bool hitbox_visible[HITBOX_MAX];
	
	int hitbox_count;
	
	uint32_t hitbox_id[ENGINE_HITBOX_MAX]; 
	Vector hitbox_offset[ENGINE_HITBOX_MAX];
	float hitbox_radius[ENGINE_HITBOX_MAX];
	
	float health;
	float max_health;
	
	float armor;
	float max_armor;
	
	float shields;
	float max_shields;
	
	float total_hp;
	float total_max_hp;
	
	uint32_t filter_bits;
	
	uint64_t hero_id;
	int hero;
	
	ow_condition conditions[MAX_PLAYER_CONDS];
	
	uint64_t position_address;
	uint64_t skill; // unsafe plz fix
	
	float ult_charge;
	
	bool is_visible;
	bool on_ground;
	
	bool is_bubbled;
	
	uint64_t pos;
	uint64_t failed_bone;
	
	Vector box_test[16];
	int box_num;
	
	float damage_taken;
};

class c_state {
public:
	void update_objects();
	
	void store_bones(ow_object* obj, cmp_velocity* vel);
	
	ow_condition* lookup_condition(ow_object* obj, uint32_t hash);
	void add_condition(ow_object* obj, uint32_t hash, uint64_t data);
	void store_conditions(ow_object* obj);
	
	bool update_entity(ow_object* obj);
	bool update_player(ow_object* obj);
	
	void update_aim_rays();
	void update_trig_rays();
	
	void set_object_type(ow_object* obj);
	bool is_player(uint32_t filter_bits);
	bool is_alive(ow_object* obj);
	bool is_object_destroyed(uint32_t filter_bits);
	int get_team(uint32_t filter_bits);
	int get_hero(uint64_t heroid);
	int get_hero_from_pool(uint64_t poolid);
	bool is_fire_ready();
	bool is_invuln(ow_object* obj, bool melee);
	
	uint64_t get_vec_key(uint64_t index);
	void get_root_pos(uint64_t a1, Vector* out);
	
	bool engine_ray_aim(ray_setup_t* setup, ray_result_t* result, Vector start, Vector end);
	bool engine_ray_trig(ray_setup_t* setup, ray_result_t* result, Vector start, Vector end);
	
	bool setup_ray(ray_setup_t* setup, Vector start, Vector end, bool filter_players);
	bool ray_add_filter(ray_setup_t* setup, uint64_t* entity_parent);
	bool ray_trace(ray_setup_t* setup, ray_result_t* trace_result);
	
	uint64_t get_skill_struct(uint64_t skill, uint16_t id1, uint16_t id2);
	
	ow_object _objects[MAX_OBJECTS];
	int _object_count;
	
	float _curtime;
	
	bool _rayray;
	Vector _black;
};

extern c_state* g_state;