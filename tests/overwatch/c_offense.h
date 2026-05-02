#pragma once

class c_offense {
public:
	void echo_fire_think();
	void soldier_fire_think();
	void genji_fire_think();
	
	void soldier_think();
	void genji_think();
	
	float _echo_next_attack;
	
	float _soldier_next_attack;
	int _soldier_shot_count;
	
	float _genji_next_primary;
	float _genji_next_secondary;
	int _genji_fire_ticks;
};

extern c_offense* g_offense;