enum {
	TRIG_TYPE_NONE = 0,
	TRIG_TYPE_PRIMARY = 1,
	TRIG_TYPE_SECONDARY = 2,
	TRIG_TYPE_ABILITY3 = 3,
	TRIG_TYPE_ZEN = 4,
};

struct trig_data {
	int type;
	
	int hitbox;
	float range;
	
	float fire_delay;
	float wait_time;
	
	int hitboxes;
	float hitbox_scale;
	float min_accuracy;
	
	bool pressing;
	
	bool history;
	bool history_pred;
	
	bool objects;
	float avoid_aim;
  
  float ghost_aim;
	
	bool in_attack;
};

class c_trig {
public:
	bool is_trigger_target(ow_object* obj);
	bool get_target();
	
	void think();
	
	bool update();
	
	void aim_trig_think();
	
	trig_data _data;
	
	float _press_time;
	float _press_time_diff;
	
	bool _first_fired;
	
	bool _trig_aiming;
  
  float _ghost_sens;
};

extern c_trig* g_trig;