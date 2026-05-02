#pragma once

class c_healer {
public:
  void mercy_think();
	void think();
	void mercy_fire_think();
	
	float _mercy_next_attack;
};

extern c_healer* g_healer;