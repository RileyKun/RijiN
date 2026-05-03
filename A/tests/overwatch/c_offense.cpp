#include "link.h"

c_offense offense_instance;
c_offense* g_offense = &offense_instance;

void c_offense::echo_fire_think() {
	pseudo_cmd* ucmd = (pseudo_cmd*)&(g_ow->_cmd);
	
	if (local->ability_limit != 0)
		return;
	
	if (ucmd->buttons & IN_ATTACK && g_state->is_fire_ready()) {
		if (g_state->_curtime >= _echo_next_attack + g_hero_data[HERO_ECHO].attack_recovery) 
			_echo_next_attack = g_state->_curtime;
	}
}

void c_offense::soldier_fire_think() {
	pseudo_cmd* ucmd = (pseudo_cmd*)&(g_ow->_cmd);
	
	if (ucmd->buttons & IN_ATTACK && g_state->is_fire_ready()) {
		if (g_state->_curtime >= _soldier_next_attack + g_hero_data[HERO_SOLDIER].attack_recovery) {
			_soldier_shot_count += 1;
			
			_soldier_next_attack = g_state->_curtime;
		}
	} else {
		_soldier_shot_count = 0;
	}
}

void c_offense::genji_fire_think() {
	pseudo_cmd* ucmd = (pseudo_cmd*)&(g_ow->_cmd);
	
	if (local->ability_limit != 0)
		return;
	
	if (local->projectile_speed == 0.0f)
		return;
	
	if (ucmd->buttons & IN_ATTACK && g_state->is_fire_ready()) {
		if (g_state->_curtime >= _genji_next_primary + g_hero_data[HERO_GENJI].attack_recovery &&
			g_state->_curtime >= _genji_next_secondary + g_hero_data[HERO_GENJI].attack2_recovery)
			_genji_next_primary = g_state->_curtime;
	}
	
	if (ucmd->buttons & IN_ATTACK2 && g_state->is_fire_ready()) {
		if (g_state->_curtime >= _genji_next_primary + g_hero_data[HERO_GENJI].attack_recovery &&
			g_state->_curtime >= _genji_next_secondary + g_hero_data[HERO_GENJI].attack2_recovery)
			_genji_next_secondary = g_state->_curtime;
	}
}

void c_offense::soldier_think() {
	if (g_aim->_aiming)
		return;
	
//	if (g_cvars_misc->soldier_recoil.value == 0)
	//	return;
	
	if (_soldier_shot_count > 0) {
		float recoil = _soldier_shot_count >= 3 ? 0.8f : 0.15f;
		
		Vector test = Vector(g_ow->_cmd.forward.pitch(), 0.0f, 0.0f);
		test.x += recoil / (g_hero_data[HERO_SOLDIER].attack_recovery / OW_TICK_INTERVAL);
		
		g_ow->_cmd.forward.y = test.forward().y;
		g_ow->_cmd.view = g_ow->_cmd.forward;
	}
	
	float* x = (float*)(g_ow->_singleton_input.component + 0x1560);
	float* y = (float*)(g_ow->_singleton_input.component + 0x1564);
	
	*x = 0.0f;
	*y = 0.0f;
}

void c_offense::genji_think() {
	pseudo_cmd* ucmd = (pseudo_cmd*)&(g_ow->_cmd);
	
	Vector fwd = local->viewforward;
	Vector base_angles = Vector(fwd.pitch(), fwd.yaw(), 0.0f);
	Vector proj_left = (base_angles + Vector(0.0f, -9.0f, 0.0f)).forward();
	Vector proj_right = (base_angles - Vector(0.0f, 9.0f, 0.0f)).forward();
	
	g_anim->_intersect_flags = ~0;
	g_anim->_intersect_mult = 1.0f;
	
	for (int i = 0; i < g_state->_object_count; ++i) {
		ow_object* obj = &(g_state->_objects[i]);
		if (obj->type == OBJECT_INVALID)
			continue;
		
		if (!g_state->is_alive(obj))
			continue;
		
		if (g_state->get_team(local->object.filter_bits) == g_state->get_team(obj->filter_bits))
			continue;
		
		Vector org = obj->model_center;
		Vector dt = (org - local->vieworigin);
		Vector dt_normal = dt.normal();
		
		float dist = dt.length_sqr();
		float diff = fwd.dot(dt_normal);
		
		if (diff < 0.707f)
			continue; // ignore any > 45 degrees behind
		
		if (dist > (2.5f * 2.5f))
			continue;
		
		int proj_count = 0;
		
		if (g_anim->find_capsule_intersection(obj, fwd, Vector(0.0f, 0.0f, 0.0f)) != -1)
			proj_count += 1;
		
		if (g_anim->find_capsule_intersection(obj, proj_left, Vector(0.0f, 0.0f, 0.0f)) != -1)
			proj_count += 1;
		
		if (g_anim->find_capsule_intersection(obj, proj_right, Vector(0.0f, 0.0f, 0.0f)) != -1)
			proj_count += 1;
		
		int damage = 30 + (29 * proj_count);
		
		if (damage >= obj->health) {
//			g_ow->_cmd.bits |= UCMD_BUTTONS;
			
			if (_genji_fire_ticks % 2 == 0)
				g_ow->_cmd.buttons |= IN_ATTACK2;
			else
				g_ow->_cmd.buttons |= IN_MELEE;
			
			_genji_fire_ticks += 1;
			
			return;
		}
	}
	
	_genji_fire_ticks = 0;
}