#pragma once

#define MAX_ANIM_FRAMES 63

class c_anim_object {
public:
	uint64_t _entity_parent;
	Vector _position_root;
	Vector _velocity;
	Vector _rotation;
	Vector _hitbox_offset[ENGINE_HITBOX_MAX];
	float _total_hp;
};

class c_anim_frame {
public:
	c_anim_object _objects[MAX_OBJECTS];
	int _object_count;
};

class c_anim {
public:
	bool is_in_fov(ow_object* obj, Vector forward, Vector pred_delta, int fov_frames);
	int find_capsule_intersection(ow_object* obj, Vector forward, Vector delta);
	int find_melee_intersection(ow_object* obj, Vector forward, Vector root);
	int find_hitbox_intersection(ow_object* obj, Vector source, Vector forward, Vector delta, c_anim_object* anim_obj, Vector* hitpos);
	float shotgun_intersection(ow_object* obj, Vector forward, Vector delta, c_anim_object* anim_obj);
	float find_history_intersection(ow_object* obj, Vector source, Vector forward, Vector delta, int* tick_force_ms);
	bool find_closest_frame(Vector forward, int* tick_force_ms);
	bool is_valid_anim_object(ow_object* obj);
	void think();
	void history_aim();
	void force_tick();
	void calculate_delta(ow_object* obj, int ticks, float limit_velocity, Vector* delta_out);
	
	c_anim_frame _frames[MAX_ANIM_FRAMES];
	int _frame_count;
	
	uint32_t _intersect_flags;
	float _intersect_mult;
	
	int _tick_limit_ms;
	int _tick_force_ms;
	
	bool _history_lag;
	bool _history_pred;
	float _history_time_limit;
	float _history_pred_limit;
	
	int _melee_frame;
};

extern c_anim* g_anim;

#define TIME_WINDOW_MS 0.26f