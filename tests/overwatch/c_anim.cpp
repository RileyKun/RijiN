#include "link.h"

c_anim anim_instance;
c_anim* g_anim = &anim_instance;

bool c_anim::is_in_fov(ow_object* obj, Vector forward, Vector pred_delta, int fov_frames) {
	if (this->find_capsule_intersection(obj, forward, pred_delta) != -1)
		return true;
	
	for (int i = 1; i < fov_frames; ++i) {
		Vector delta = Vector(0.0f, 0.0f, 0.0f);
		this->calculate_delta(obj, i, 0.0f, &delta);
		
		if (this->find_capsule_intersection(obj, forward, pred_delta + delta) != -1) {
			
			return true;
		}
	}
	
	for (int i = 1; i < fov_frames; ++i) {
		Vector delta = Vector(0.0f, 0.0f, 0.0f);
		this->calculate_delta(obj, i, 0.0f, &delta);
		
		if (this->find_capsule_intersection(obj, forward, pred_delta - delta) != -1) {
			
			return true;
		}
	}
	
	return false;
}

#define BASE_BOUNDS_RADIUS 0.4f

int c_anim::find_capsule_intersection(ow_object* obj, Vector forward, Vector delta) {
	float rad = BASE_BOUNDS_RADIUS * _intersect_mult;
	
	Vector new_min = obj->model_min + delta;
	new_min.y += BASE_BOUNDS_RADIUS;
	Vector new_max = obj->model_max + delta;
	new_max.y -= BASE_BOUNDS_RADIUS;
	
	if (RayIntersectCapsule(local->vieworigin, forward, new_min, new_max, rad, nullptr))
		return 0;
	
	return -1;
}

int c_anim::find_melee_intersection(ow_object* obj, Vector forward, Vector root) {
	float rad = BASE_BOUNDS_RADIUS + 1.0f;
	
	Vector new_min = root;
	new_min.y += BASE_BOUNDS_RADIUS;
	Vector new_max = root + Vector(0.0f, obj->model_max.y - obj->position_root.y, 0.0f);
	new_max.y -= BASE_BOUNDS_RADIUS;
	
	if (RayIntersectCapsule(local->vieworigin, forward, new_min, new_max, rad, nullptr))
		return 0;
	
	return -1;
}

int c_anim::find_hitbox_intersection(ow_object* obj, Vector source, Vector forward, Vector delta, c_anim_object* anim_obj, Vector* hitpos) {
	Vector position_root = obj->position_root + delta;
	if (anim_obj)
		position_root = anim_obj->_position_root;
	
	pseudo_ray_t* ray = &(local->view_ray);
	
	matrix_t maty;
	
	if (anim_obj)
		OW_InitY(anim_obj->_rotation.y, maty);
	else
		OW_InitY(obj->rotation.y, maty);
	
	bool bot = obj->hero == HERO_TRAININGBOT1 || obj->hero == HERO_TRAININGBOT4;
	
	if (bot) {
		Vector head = position_root + Vector(0.0f, 1.96f, 0.0f);
		Vector body = position_root + Vector(0.0f, 1.3f, 0.0f);
		float head_size = 0.440f * _intersect_mult;
		float body_size = 0.5f * _intersect_mult;
		float test_len = 0.0f;
		
		if (ray->test_length >= (head - source).length() && (_intersect_flags & (1<<HITBOX_HEAD))) {
			if (RayIntersectSphere(source, head, forward, head_size * head_size, &test_len)) {
				if (hitpos)
					*hitpos = source + (forward * test_len);
				
				return 0;
			}
		}
		
		if (ray->test_length >= (body - source).length() && (_intersect_flags & (1<<HITBOX_CHEST))) {
			if (RayIntersectSphere(source, body, forward, body_size * body_size, &test_len)) {
				if (hitpos)
					*hitpos = source + (forward * test_len);
				
				return 1;
			}
		}
		
		return -1;
	}
	
	int best_n = -1;
	float best_len = 1.e+7f;
	
	for (int i = 0; i < obj->hitbox_count; ++i) {
		Vector offset = obj->hitbox_offset[i];
		if (anim_obj)
			offset = anim_obj->_hitbox_offset[i];
		
		Vector center = {};
		OW_Transform(offset, maty, &center);
		center += position_root;
		
		if (source == local->vieworigin) {
			if (ray->test_length < (center - source).length())
				continue;
		}
		
		float radius = obj->hitbox_radius[i];
		
		float size = radius * _intersect_mult;
	/*	if (box->min[1] != 0.0f && box->max[1] != 0.0f) {
			Vector box_min = offset + Vector(box->min[0], box->min[1], box->min[2]);
			Vector box_max = offset + Vector(box->max[0], box->max[1], box->max[2]);
			
			Vector min = {};
			Vector max = {};
			
			OW_Transform(box_min, maty, &min);
			OW_Transform(box_max, maty, &max);
			
			min += position_root;
			max += position_root;
			
			if (RayIntersectCapsule(vieworigin, forward, min, max, size, hitpos))
				return i;
		} else */{
			float test_len = 0.0f;
			if (RayIntersectSphere(source, center, forward, size * size, &test_len)) {
				if (hitpos)
					*hitpos = source + (forward * test_len);
				
				int n = HITBOX_BODY;
				if (obj->hitbox_id[i] == obj->hitbox_map[HITBOX_HEAD])
					n = HITBOX_HEAD;
				else if (obj->hitbox_id[i] == obj->hitbox_map[HITBOX_CHEST])
					n = HITBOX_CHEST;
				
				if (test_len < best_len) {
					best_len = test_len;
					best_n = n;	
				}
				//if (n > best_n)
					//best_n = n;
			}
		}
	}
	
	if (_intersect_flags == ~0)
		return best_n;
	
	if (_intersect_flags & (1<<HITBOX_HEAD)) {
		if (best_n == HITBOX_HEAD)
			return best_n;
	}

	if (_intersect_flags & (1<<HITBOX_CHEST)) {
		if (best_n == HITBOX_CHEST)
			return best_n;
	}
  
  if (_intersect_flags & (1<<HITBOX_BODY)) {
    if (best_n == HITBOX_BODY)
      return best_n;
  }
	
	return -1;
}

static float s_shotgun_spread[12][3] = {
	{0.333f, 0.333f, 0.0f},
	{-0.333f, 0.333f, 0.0f},
	{0.333f, -0.333f, 0.0f},
	{-0.333f, -0.333f, 0.0f},
	
	{0.666f, 0.666f, 0.0f},
	{-0.666f, 0.666f, 0.0f},
	{0.666f, -0.666f, 0.0f},
	{-0.666f, -0.666f, 0.0f},
	
	{1.0f, 1.0f, 0.0f},
	{-1.0f, 1.0f, 0.0f},
	{1.0f, -1.0f, 0.0f},
	{-1.0f, -1.0f, 0.0f},
};

float c_anim::shotgun_intersection(ow_object* obj, Vector forward, Vector delta, c_anim_object* anim_obj) {
	const float frac = 1.0f / (12.0f);
	float degree = g_hero_data[local->object.hero].attack_spread;
	float damage = 0.0f;
	
	for (int i = 0; i < 12; ++i) {
		float pitch = degree * s_shotgun_spread[i][0];
		float yaw = degree * s_shotgun_spread[i][1];
		
		ray_setup_t ray_in = {};
		ray_result_t ray_out = {};
		
		Vector angles = Vector(forward.pitch(), forward.yaw(), 0.0f);
		Vector new_forward = (angles + Vector(pitch, yaw, 0.0f)).forward();
		new_forward = new_forward.normal();
		
		int test = this->find_hitbox_intersection(obj, local->vieworigin, new_forward, delta, anim_obj, nullptr);
		if (test != -1)
			damage += frac * (test == HITBOX_HEAD ? 2.0f : 1.0f);
	}
	
	return damage;
}

float c_anim::find_history_intersection(ow_object* obj, Vector source, Vector forward, Vector delta, int* tick_force_ms) {
	bool stop = false;
	int offset = 0;
	float best_dmg = 0.0f;
	float best_dist = 1.e+9f;
	
	float limit_ms = (float)(_tick_limit_ms) / 1000.0f;
	bool shotgun = local->shotgun;
	
	if (shotgun) {
		float dmg = this->shotgun_intersection(obj, forward, delta, nullptr);
		best_dmg = dmg;
	} else {
		int test = this->find_hitbox_intersection(obj, source, forward, delta, nullptr, nullptr);
		if (test != -1)
			return 1.0f;
	}
	
	Vector hitpos = Vector(0.0f, 0.0f, 0.0f);
	
	for (int i = 1; i < 20; ++i) {
		if (!_history_lag)
			break;
		
		int f = _frame_count - i;
		if (f < 0)
			break;
		
		if (local->projectile_speed > 0.0f)
			break;
		
		float ms = OW_TICK_INTERVAL * (float)(i);
		if ((ms + limit_ms) > TIME_WINDOW_MS)
			break;
		
		c_anim_frame* frame = &(_frames[f % MAX_ANIM_FRAMES]);
		for (int n = 0; n < frame->_object_count; ++n) {
			c_anim_object* anim_obj = &(frame->_objects[n]);
			if (anim_obj->_entity_parent == obj->entity_parent) {
				Vector past_delta = anim_obj->_position_root - obj->position_root;
				
				if (shotgun) {
					float dmg = this->shotgun_intersection(obj, forward, delta + past_delta, anim_obj);
					if (dmg > 0.0f) {
						if (dmg > best_dmg) {
							best_dmg = dmg;
							
							offset = i;
						}
						
						break;
					}
				} else {
					int test = this->find_hitbox_intersection(obj, source, forward, delta + past_delta, anim_obj, &hitpos);
					if (test != -1) {
						if (ms > _history_time_limit)
							break;
						
						stop = true;
						
						float dist = (hitpos - local->vieworigin).length();
						dist += (float)(test) * 1.e+4f;
						
						if (dist < best_dist) {
							best_dist = dist;
							
							best_dmg = 1.0f;
							offset = i;
						}
						
						break;
					}
				}
			}
		}
	}
	
	int max_forward = (int)((limit_ms / OW_TICK_INTERVAL) + 0.5f);
	
	for (int i = 1; i < max_forward; ++i) {
		if (!_history_pred || stop == true)
			break;
		
		float ms = OW_TICK_INTERVAL * (float)(i);
		
		if (obj->hero >= HERO_DVA && obj->hero <= HERO_ZARYA) {
			if (ms > _history_pred_limit)//_history_tank_limit)
				continue;
		}
		
		Vector ping_delta = Vector(0.0f, 0.0f, 0.0f);
		this->calculate_delta(obj, i, 0.0f, &ping_delta);
		
		if (shotgun) {
			float dmg = this->shotgun_intersection(obj, forward, delta + ping_delta, nullptr);
			if (dmg > 0.0f) {
				if (dmg > best_dmg) {
					best_dmg = dmg;
					
					offset = -i;
				}
			}
		} else {
			int test = this->find_hitbox_intersection(obj, source, forward, delta + ping_delta, nullptr, &hitpos);
			if (test != -1) {
				if (ms > _history_pred_limit)
					continue;
				
				float dist = (hitpos - local->vieworigin).length();
				dist += (float)(test) * 1.e+4f;
				
				if (dist < best_dist) {
					best_dist = dist;
					
					best_dmg = 1.0f;
					offset = -i;
				}
			}
		}
	}
	
	if (best_dmg != 0.0f) {
		if (offset != 0 && tick_force_ms) {
			*tick_force_ms = (int)((OW_TICK_INTERVAL * (float)(offset)) * 1000.0f);
			if (*tick_force_ms + _tick_limit_ms < 0)
				*tick_force_ms = _tick_limit_ms * -1;
		}
		
		return best_dmg;
	}
	
	return 0.0f;
}

bool c_anim::find_closest_frame(Vector forward, int* tick_force_ms) {
	int offset = 0;
	
	float limit_ms = (float)(_tick_limit_ms) / 1000.0f;
	
	float best_total_dist = 1.e+9f;
	
	for (int i = 0; i < 20; ++i) {
		int f = _frame_count - i;
		if (f < 0)
			break;
		
		float ms = OW_TICK_INTERVAL * (float)(i);
		if ((ms + limit_ms) > TIME_WINDOW_MS)
			break;
		
		float total_dist = 0.0f;
		
		c_anim_frame* frame = &(_frames[f % MAX_ANIM_FRAMES]);
		for (int n = 0; n < frame->_object_count; ++n) {
			c_anim_object* anim_obj = &(frame->_objects[n]);
			if (anim_obj->_entity_parent == 0)
				continue;
			
			Vector new_pos = anim_obj->_position_root;
			
			float dist = (new_pos - local->object.position_root).length();
			if (dist > local->melee_range)
				continue;
			
			total_dist += dist;
		}
		
		if (total_dist < best_total_dist) {
			best_total_dist = total_dist;
			
			offset = i;
		}
	}
	
	/*int max_forward = (int)((limit_ms / OW_TICK_INTERVAL) + 0.5f);
	
	for (int i = 1; i < max_forward; ++i) {
		
		break;
		if (!_history_pred || stop == true)
			break;
		
		float ms = OW_TICK_INTERVAL * (float)(i);
		if (ms > _history_time_limit)
			break;
		
		Vector ping_delta = Vector(0.0f, 0.0f, 0.0f);
		this->calculate_delta(obj, i, 0.0f, &ping_delta);
		
		Vector new_pos = obj->position_root + ping_delta;
		
		float dist = (new_pos - local->vieworigin).length();
		if (dist < best_dist) {
			best_dist = dist;
			
			offset = -i;
		}
	}*/
	
	if (offset != 0) {
		_melee_frame = offset;
		
		if (tick_force_ms) {
			*tick_force_ms = (int)((OW_TICK_INTERVAL * (float)(offset)) * 1000.0f);
			if (*tick_force_ms + _tick_limit_ms < 0)
				*tick_force_ms = _tick_limit_ms * -1;
		}
		
		return true;
	}
	
	return false;
}

bool c_anim::is_valid_anim_object(ow_object* obj) {
	if (obj->health < 1.0f)
		return false;
	
	bool team_mate = local->team == g_state->get_team(obj->filter_bits);
	
	//if (local->team != 3) {
		//if (team_mate)
		//	return false;
//	}
	
	return true;
}

void c_anim::think() {
	_frame_count += 1;
	
	c_anim_frame* frame = &(_frames[_frame_count % MAX_ANIM_FRAMES]);
	
	frame->_object_count = g_state->_object_count;
	
	for (int i = 0; i < g_state->_object_count; ++i) {
		ow_object* obj = &(g_state->_objects[i]);
		c_anim_object* anim_obj = &(frame->_objects[i]);
		memset(anim_obj, 0, sizeof(anim_obj));
		
		if (!this->is_valid_anim_object(obj))
			continue;
		
		anim_obj->_entity_parent = obj->entity_parent;
		anim_obj->_position_root = obj->position_root;
		anim_obj->_velocity = obj->velocity;
		anim_obj->_rotation = obj->rotation;
		anim_obj->_total_hp = obj->total_hp;
		
		for (int n = 0; n < ENGINE_HITBOX_MAX; ++n)
			anim_obj->_hitbox_offset[n] = obj->hitbox_offset[n];
	}
	
	for (int i = 0; i < g_state->_object_count; ++i) {
		ow_object* obj = &(g_state->_objects[i]);
		
		if (!this->is_valid_anim_object(obj))
			continue;
		
		bool team_mate = local->team == g_state->get_team(obj->filter_bits);
		if (team_mate == false)
			continue;
		
		float diff = 0.0f;
		float last_hp = obj->total_hp;
		int index = 1;
		
		for (int k = 0; k < MAX_ANIM_FRAMES; ++k) {
			c_anim_frame* last_frame = &(_frames[(_frame_count - k) % MAX_ANIM_FRAMES]);
			
			if (_frame_count - k < 0)
				break;
			
			for (int n = 0; n < last_frame->_object_count; ++n) {
				c_anim_object* anim_obj = &(last_frame->_objects[n]);
				if (anim_obj->_entity_parent == obj->entity_parent) {
					float temp = last_hp - anim_obj->_total_hp;
					
					if (temp <= 0.0f) {
						diff += temp * -1.0f;
						index += 1;
					}
					
					last_hp = anim_obj->_total_hp;
				}
			}
		}
		
		float result = diff / (float)(index);
		
		obj->damage_taken = result;
	}
	
	
	// correct for 1 frame tick prediction, previous frames won't need it
	/*for (int i = 0; i < g_state->_object_count; ++i) {
		ow_object* obj = &(g_state->_objects[i]);
		
		if (!this->is_valid_anim_object(obj))
			continue;
		
		for (int n = 0; n < last_frame->_object_count; ++n) {
			c_anim_object* anim_obj = &(last_frame->_objects[n]);
			if (anim_obj->_entity_parent == obj->entity_parent)
				anim_obj->_position_root = obj->position_root_raw;
		}
	}*/
}

void c_anim::history_aim() {
	_melee_frame = 0;
	
	if (aim_config.position_enabled == false)
		return;
	
	_history_lag = true;
	_history_pred = aim_config.position_forwardtrack != 0;
	_history_time_limit = aim_config.position_backtrack * OW_TICK_INTERVAL;
	_history_pred_limit = aim_config.position_forwardtrack * OW_TICK_INTERVAL;
	
	Vector forward = g_ow->_cmd.forward;
	
	_intersect_flags = ~0;
	
	if (local->melee) {
		this->find_closest_frame(forward, &_tick_force_ms);
		return;
	}
	
	for (int i = 0; i < g_state->_object_count; ++i) {
		ow_object* obj = &(g_state->_objects[i]);
		
		if (!g_aim->is_valid_target(obj))
			continue;
		
		if (local->projectile_speed > 0.0f) {
			_intersect_mult = 1.0f;
			
			if (this->find_capsule_intersection(obj, forward, Vector(0.0f, 0.0f, 0.0f)) != -1) {
				_tick_force_ms = (int)((OW_TICK_INTERVAL * (float)(obj->predicted_backward)) * 1000.0f);
				if (_tick_force_ms + _tick_limit_ms < 0)
					_tick_force_ms = _tick_limit_ms * -1;
				
				break;
			}
		} else {
			_intersect_mult = 0.9f;//(float)(g_cvars->history_hitbox_scale.value) / 100.0f;
			
			if (this->find_history_intersection(obj, local->vieworigin, forward, obj->predicted_delta, &_tick_force_ms) != 0.0f)
				break;
		}
	}
}

void c_anim::force_tick() {
	if (local->object.hero == HERO_ROADHOG) {
		float dt = g_state->_curtime - g_local->_hook_attach_time;
		if (dt < 0.5f)
			_tick_force_ms = 0;
	}
}

void c_anim::calculate_delta(ow_object* obj, int ticks, float limit_velocity, Vector* delta_out) {
	Vector delta = Vector(0.0f, 0.0f, 0.0f);
	Vector velocity = obj->velocity;
	float gravity = 20.0f;
	
	for (int i = 0; i < ticks; ++i) {
		if (obj->on_ground == false)
			velocity.y -= gravity * OW_TICK_INTERVAL * 0.5f;
		
		delta += velocity * OW_TICK_INTERVAL;
		
		if (obj->on_ground == false)
			velocity.y -= gravity * OW_TICK_INTERVAL * 0.5f;
		
		if (limit_velocity > 0.0f) {
			float len = delta.length() / OW_TICK_INTERVAL;
			if (len > limit_velocity) {
				delta = Vector(0.0f, 0.0f, 0.0f);
				break;
			}
		}
	}
	
	*delta_out = delta;
}