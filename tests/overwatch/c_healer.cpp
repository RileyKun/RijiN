#include "link.h"

c_healer healer_instance;
c_healer* g_healer = &healer_instance;

void c_healer::mercy_think() {
  //if (g_cvars_misc->mercy_auto.value == 0)
   // return;
  
  pseudo_cmd* ucmd = (pseudo_cmd*)&(g_ow->_cmd);
  
  if (!(ucmd->buttons & IN_ATTACK) && !(ucmd->buttons & IN_ATTACK2))
    return;
  
//  if (local->healing_beam == 0.0f)
  //  return;
  
  Vector fwd = local->viewforward;
  
  ow_object* best_obj = nullptr;
  
  for (int i = 0; i < g_state->_object_count; ++i) {
    ow_object* obj = &(g_state->_objects[i]);
    if (obj->type == OBJECT_INVALID)
      continue;
    
    if (!g_state->is_alive(obj))
      continue;
    
    if (g_state->get_team(local->object.filter_bits) != g_state->get_team(obj->filter_bits))
      continue;
    
    if (obj->link_id != local->mercy_target)
      continue;
    
    best_obj = obj;
    break;
  }
  
  if (best_obj != nullptr) {
    ucmd->buttons |= IN_ATTACK;
    ucmd->buttons |= IN_ATTACK2;
    
    if (best_obj->total_hp < best_obj->total_max_hp) {
      ucmd->buttons &= ~IN_ATTACK2;
    }
  }
}

void c_healer::think() {
	this->mercy_think();
}

void c_healer::mercy_fire_think() {
	pseudo_cmd* ucmd = (pseudo_cmd*)&(g_ow->_cmd);
	
	if (local->ability_limit != 0)
		return;
	
	if (ucmd->buttons & IN_ATTACK && g_state->is_fire_ready()) {
		if (g_state->_curtime >= _mercy_next_attack + g_hero_data[HERO_MERCY].attack_recovery) 
			_mercy_next_attack = g_state->_curtime;
	}
}