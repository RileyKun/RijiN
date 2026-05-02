#include "link.h"

c_trig trig_instance;
c_trig* g_trig = &trig_instance;

bool c_trig::is_trigger_target(ow_object* obj) {
	bool shotgun = local->shotgun;
	
	Vector forward = g_ow->_cmd.forward;
	Vector org = obj->model_center;
	
	if (obj->type != OBJECT_PLAYER)
		return false;
	
	bool team_mate = false;
	if (local->team != 3) {
		if (local->team == g_state->get_team(obj->filter_bits))
			team_mate = true;
	}
	
	if (team_mate) {
		if (local->object.hero == HERO_ZARYA)
			return false;
		
		if (obj->total_hp == obj->total_max_hp)
			return false;
	}
	
	Vector dt = (org - local->vieworigin);
	Vector dt_normal = dt.normal();
	
	float dist = dt.length_sqr();
	float diff = forward.dot(dt_normal);
	
	if (dist > 100.0f * 100.0f)
		return false;
	
	if (_data.range != 0.0f) {
		if (dist > _data.range * _data.range)
			return false;
	}
	
	if (diff < 0.0f)
		return false; // ignore any > 90 degrees behind
	
	if (_data.hitbox != -1 || (local->object.hero == HERO_ANA && local->projectile_speed == 0.0f)) {
		Vector delta = obj->predicted_delta;
    g_anim->_intersect_flags = 0;
		
		if (_data.hitboxes & (1<<0))
			g_anim->_intersect_flags |= (1<<HITBOX_HEAD);
		if (_data.hitboxes & (1<<1))
      g_anim->_intersect_flags |= (1<<HITBOX_CHEST);
    if (_data.hitboxes & (1<<2))
      g_anim->_intersect_flags |= (1<<HITBOX_BODY);
		
		g_anim->_intersect_mult = _data.hitbox_scale;
		
		g_anim->_history_lag = _data.history;
		g_anim->_history_pred = _data.history_pred;
		
		float dmg = 0.0f;
		
		pseudo_ray_t* ray = &(local->view_ray);
		
		Vector source = local->vieworigin;
		Vector dir = forward;
		
		if (aim_config.position_enabled && (_data.history || _data.history_pred))
			dmg = g_anim->find_history_intersection(obj, source, dir, delta, &(g_anim->_tick_force_ms));
		else if (shotgun)
			dmg = g_anim->shotgun_intersection(obj, dir, delta, nullptr);
		else
			dmg = g_anim->find_hitbox_intersection(obj, source, dir, delta, nullptr, nullptr) != -1 ? 1.0f : 0.0f;
		
		if (local->object.hero == HERO_HANZO && local->ability_limit == 0x5a1) {
			if (dmg == 0.0f) {
				source = ray->reflect_start;
				dir = (ray->reflect_end - ray->reflect_start).normal();
				
				dmg = g_anim->find_hitbox_intersection(obj, source, dir, delta, nullptr, nullptr) != -1 ? 1.0f : 0.0f;
				if (dmg != 0.0f) {
					g_anim->_tick_force_ms = (int)((OW_TICK_INTERVAL * (float)(obj->predicted_backward)) * 1000.0f);
					if (g_anim->_tick_force_ms + g_anim->_tick_limit_ms < 0)
						g_anim->_tick_force_ms = g_anim->_tick_limit_ms * -1;
				}
			}
		}
		
		if (shotgun) {
			if (_data.min_accuracy > 0.0f) {
				if (dmg >= _data.min_accuracy - 0.01f)
					return true;
			} else {
				if (dmg != 0.0f)
					return true;
			}
		} else {
			if (dmg != 0.0f)
				return true;
		}
	} else {
		Vector delta = obj->predicted_delta;
		g_anim->_intersect_mult = _data.hitbox_scale;
		
		Vector new_forward = g_ow->_cmd.forward;
		
		pseudo_ray_t* ray = &(local->view_ray);
	//	if (g_anim->find_capsule_intersection(obj, new_forward, obj->position_root + delta) != -1)
	//	return true;
		
		{
			Vector root = obj->position_root + delta;
			
			float sz = 0.4f;
			float rad = sz * 1.0f;
			
			Vector new_min = root;
			new_min.y += 0.4f;
			Vector new_max = root + Vector(0.0f, obj->model_max.y - obj->position_root.y, 0.0f);
			new_max.y -= 0.4f;
			
			Vector endpos;
			if (RayIntersectCapsule(local->vieworigin, new_forward, new_min, new_max, rad, &endpos)) {
				if ((endpos - local->vieworigin).length() < ray->test_length)
					return true;
			}
		}
		
		if (obj->on_ground) {
			Vector root = obj->position_root + delta;
			Vector center = obj->model_center + delta;
			Vector mmax = obj->model_max + delta;
			
			Vector source = ray->test_hitpos;
			float dist = 2.5f - (2.5f * _data.min_accuracy);
			
			if ((root - source).length() < dist)
				return true;
			
			if ((center - source).length() < dist)
				return true;
			
			if ((mmax - source).length() < dist)
				return true;
		}
	}
	
	return false;
}

bool c_trig::get_target() {
	for (int i = 0; i < g_state->_object_count; ++i) {
		ow_object* obj = &(g_state->_objects[i]);
		
		if (!g_aim->is_valid_target(obj))
			continue;
		
		if (this->is_trigger_target(obj))
			return true;
	}
	
	return false;
}

void c_trig::think() {
	pseudo_cmd* change_cmd = &(g_ow->_cmd);
	bool fire = false;
	
	if (_data.type != TRIG_TYPE_NONE) {
		if (this->update())
			fire = true;
	}
	
	if (_data.fire_delay != 0.0f) {
		if (_data.in_attack)
			change_cmd->buttons &= ~(IN_ATTACK);
		
		if (_press_time_diff >= _data.fire_delay) {
			fire = true;
			_press_time = 0.0f;
		}
	}
	
	if (fire)
		_first_fired = true;
	else if (_data.type == TRIG_TYPE_NONE)
		_first_fired = false;
	
	if (g_aim->_aim_obj != nullptr) {
		if (g_aim->_aim_obj->type != OBJECT_PLAYER)
			fire = false;
	}
	
	if (local->object.hero == HERO_WIDOW) {
		if (local->sniper_charge < 0.01f)
			fire = false;
		
		if (_data.min_accuracy > 0.0f) {
			if (local->sniper_charge < _data.min_accuracy)
				fire = false;
		}
		
		if (local->fire_ready == false)
			fire = false;
	}
	
	if (local->object.hero == HERO_ASHE) {
		if ((g_state->is_fire_ready() && local->fire_time >= g_hero_data[HERO_ASHE].attack2_recovery) == false)
			fire = false;
	}
	
	if (local->object.hero == HERO_HANZO && local->ability_limit != 0x5a1) {
		if (local->sniper_charge < 0.01f)
			fire = false;
		
		if (_data.min_accuracy > 0.0f) {
			if (local->sniper_charge < _data.min_accuracy)
				fire = false;
		}
	}
	
	if (_data.type == TRIG_TYPE_ZEN) {
		if (local->zen_orbs < 0.2f)
			fire = false;
		
		float wait = (float)(aim_config.zen_wait_orbs) * 0.2f;
		if (local->zen_orbs < wait)
			fire = false;
	}
	
	if (!g_state->is_fire_ready())
		fire = false;
	
	if (fire) {
		if (local->object.hero == HERO_HANZO && local->ability_limit != 0x5a1)
			change_cmd->buttons &= ~(IN_ATTACK);
		else if (_data.type == TRIG_TYPE_ZEN)
			change_cmd->buttons &= ~IN_ATTACK2;
		else if (_data.type == TRIG_TYPE_PRIMARY)
			change_cmd->buttons |= IN_ATTACK;
		else if (_data.type == TRIG_TYPE_SECONDARY)
			change_cmd->buttons |= IN_ATTACK2;
		else if (_data.type == TRIG_TYPE_ABILITY3)
			change_cmd->buttons |= IN_ABILITY3;
		
		if (_data.type == TRIG_TYPE_ZEN) {
			//if (g_cvars_misc->zen_melee.value != 0 && (g_ow->_last_buttons & IN_ATTACK2) == 0)
			//	change_cmd->buttons |= IN_MELEE;
		}
	} else if (_data.type != TRIG_TYPE_NONE) {
		if (local->object.hero == HERO_ECHO) {
		//	if (g_cvars_misc->echo_m2_trig.value != 0 && local->ability_limit != 0x1e) {
			//	if (change_cmd->buttons & IN_ATTACK2)
			//		change_cmd->buttons &= ~IN_ATTACK2;
		//	}
			
		//	if (g_cvars_misc->echo_e_trig.value != 0 && local->ability_limit != 0x6fc) {
		//		if (change_cmd->buttons & IN_ABILITY2)
		//			change_cmd->buttons &= ~IN_ABILITY2;
		//	}
		}
		
		if (local->object.hero == HERO_SOLDIER) {
		//	if (g_cvars_misc->soldier_trig.value != 0) {
			//	if (change_cmd->buttons & IN_ATTACK2)
			//		change_cmd->buttons &= ~IN_ATTACK2;
			//}
		}
		
		if (local->object.hero == HERO_TRACER) {
	//		if (g_cvars_misc->tracer_ult_trig.value != 0) {
		//		if (change_cmd->buttons & IN_ABILITY3)
			//		change_cmd->buttons &= ~IN_ABILITY3;
		//	}
		}
	}
}

bool c_trig::update() {
	bool fire = false;
	
	if (this->get_target()) // see update_trig_rays
		fire = true;
	
	pseudo_ray_t* ray = &(local->view_ray);
	if (ray->fraction > 0.0f && ray->fraction < 1.0f) {
		if (_data.objects)
			fire = true;
	}
	
	if (fire)
		return true;
	
	return false;
}

void c_trig::aim_trig_think() {
  _trig_aiming = false;
  
  float temp = _data.hitbox_scale;
  _data.hitbox_scale = _data.avoid_aim;
  
  if (_data.avoid_aim != 0.0f) {
    if (this->update())
      _trig_aiming = true;
  }
  
  _data.hitbox_scale = temp;
  
  _ghost_sens = 0.0f;
  
  if (_data.ghost_aim != 0.0f) {
    if (this->update())
      _ghost_sens = _data.ghost_aim;
  }
}