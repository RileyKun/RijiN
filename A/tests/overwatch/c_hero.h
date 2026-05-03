#pragma once

class c_hero {
public:
	void test();
	
	void think();
	
	int get_aim_key(int value);
	void aim_setup();
	void post_aim_setup();
	
	void aim_think();
	
	void trig_think();
	
	void auto_melee();
	
	void zarya_super_jump();
	
	void fire_time_think();
  
  void sigma_fire_think();
	
	bool _beasd;
	Vector _beasd_dir;
	float _beasd_dist;
	
	int _jump_time;
  
  float _sigma_next_attack;
  float _sigma_next_fire;
  int _sigma_attack_interval;
};

extern c_hero* g_hero;