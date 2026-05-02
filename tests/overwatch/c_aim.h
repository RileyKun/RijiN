#pragma once

enum {
	AIM_TYPE_NONE = 0,
	AIM_TYPE_PRIMARY = 1,
	AIM_TYPE_SECONDARY = 2,
	AIM_TYPE_ABILITY1 = 3,
	AIM_TYPE_ABILITY2 = 4,
	AIM_TYPE_ABILITY3 = 5,
	AIM_TYPE_HOOK = 6,
	AIM_TYPE_HEAL = 7,
	AIM_TYPE_ZEN = 9,
	AIM_TYPE_RAGE = 128,
};

struct aim_data {
	int type;
	
	float fov;
	float bounds;
	int hitbox;
	float random;
	
	int smooth_type;
	
	float smooth;
	float smooth_const;
	float smooth_correction;
	float smooth_accel;
	float smooth_sine;
	
	int auto_fire;
	bool movement;
	bool single_target;
	float wait_time;
	float autofire_time;
	float range;
	
	int hitbox_level;
	
	bool distance_sorting;
	
	bool shoot_bubbled;
	bool avoid_aids;
};

class c_aim {
public:
  bool is_valid_hitbox(int hitbox);
	bool is_aids_movement(ow_object* obj);
	bool is_valid_aim_ent(ow_object* obj);
	bool is_valid_target(ow_object* obj);
	int get_hitbox_level(ow_object* obj);
	bool get_heal_pos(ow_object* obj, Vector* pos_out, float* diff_out);
	bool get_aim_pos(ow_object* obj, Vector* pos_out, float* diff_out);
	bool get_target();
	bool get_healing_target();
	
	void think();
	
	bool update();
	
	bool pre_aim();
	void apply_smooth(Vector forward, bool silent);
	bool set_aim();
	bool set_aim_silent();
	void autofire_think();
	
	void force_tick();
	
	aim_data _data;
	int _last_type;
	
	ow_object* _aim_obj;
	Vector _aim_pos;
	
	ow_object* _last_obj;
	
	bool _aiming;
	bool _arrow_drawn;
	bool _arrow_drawn_silent;
	bool _on_heal_target;
	bool _zen_charging;
	
	float _aim_time;
	
	float _key_time;
	float _autofire_time;
	
	int _backward_tick;
	float _last_aimed;
	
	Vector _aim_dir;
  Vector _new_dir;
	
	uint32_t _xseed;
	uint32_t _yseed;
	uint32_t _zseed;
    
  float _cur_speed;
};

extern c_aim* g_aim;