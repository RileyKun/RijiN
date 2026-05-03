#include "link.h"

c_aim aim_instance;
c_aim* g_aim = &aim_instance;

bool c_aim::is_valid_hitbox(int hitbox) {
 // if (_data.hitbox == 0)
  //  return true;
  
  if (hitbox == HITBOX_HEAD && _data.hitbox & (1<<0))
    return true;
  
  if (hitbox == HITBOX_CHEST && _data.hitbox & (1<<1))
    return true;
  
  if (hitbox == HITBOX_BODY && _data.hitbox & (1<<2))
    return true;
  
  return false;
}

bool c_aim::is_aids_movement(ow_object* obj) {
	if (obj->velocity.length_2d() > 22.0f) // 20 units about ball fire move speed
		return true;
	
	if (sqrtf(obj->velocity.y*obj->velocity.y) > 30.0f)
		return true;
	
	switch (obj->hero) {
		case HERO_DOOM: {
			ow_condition* shift = g_state->lookup_condition(obj, HASH("usage_shift"));
			if (shift) {
				if (shift->data.u32 & 1)
					return true;
			}
			
			break;
		}
	}
	
	return false;
}

bool c_aim::is_valid_aim_ent(ow_object* obj) {
	if (obj->type == OBJECT_PLAYER && g_state->is_player(obj->filter_bits)) {
		bool team_mate = local->team != 3 && local->team == g_state->get_team(obj->filter_bits);
		
		if ((local->object.hero != HERO_ANA) && (local->object.hero != HERO_ZARYA)) {
			if (team_mate)
				return false;
		}
		
		if (_data.shoot_bubbled == false && team_mate == false) {
			if (obj->is_bubbled)
				return false;
		}
		
		if (g_state->is_invuln(obj, false))
			return false;
		
	//	if (obj->hero == HERO_DVA)
		//	return false;
		
		return true;
	}
	
	if (obj->type == OBJECT_DYNAMITE) {
		if (local->object.hero != HERO_ASHE)
			return false;
		
		if (local->team == 3)
			return false;
		
		if (g_state->is_object_destroyed(obj->filter_bits))
			return false;
		
		if (local->team != g_state->get_team(obj->filter_bits))
			return false;
		
		return true;
	}
	
	return false;
}

bool c_aim::is_valid_target(ow_object* obj) {
	if (!g_state->is_alive(obj))
		return false;
	
	if (!this->is_valid_aim_ent(obj))
		return false;
	
	return true;
}

int c_aim::get_hitbox_level(ow_object* obj) {
	if (local->object.hero == HERO_ROADHOG) {
		switch (obj->hero) {
			case HERO_BRIG:
			//case HERO_MERCY:
			//case HERO_ZEN:
			case HERO_REINHARDT:
			case HERO_ZARYA: {
				return 0;
			}
			case HERO_REAPER:
			case HERO_MERCY:
			case HERO_ZEN:
			case HERO_SIGMA: {
				return 2;
			}
			case HERO_ROADHOG:
			case HERO_ASHE:
			case HERO_GENJI:
			case HERO_SOLDIER:
			case HERO_DOOM: {
				return 1;
			}
		}
		
		return 1;
	}
	
	return _data.hitbox_level;
}

bool c_aim::get_heal_pos(ow_object* obj, Vector* pos_out, float* diff_out) {
	int value = g_hero_data[obj->hero].value;
	
	Vector vieworigin = local->vieworigin;
	Vector viewforward = local->viewforward;
	
	Vector org = obj->type == OBJECT_PLAYER ? obj->model_center: obj->position;
	
	Vector dt = (org - local->vieworigin);
	Vector dt_normal = dt.normal();
	
	float dist = dt.length_sqr();
	float diff = viewforward.dot(dt_normal);
	
	if (local->team != g_state->get_team(obj->filter_bits))
		return false;
	
	if (obj->total_hp == obj->total_max_hp)
		return false;
	
	if (value < 3000) {
		if (local->ammo_percent < 0.5f) {
			//if ((obj->total_max_hp - obj->total_hp) < 50)
			//	return false;
		}
	}
	
	if (obj->is_visible == false)
		return false;
	
	if (_data.fov > 0.0f) {
		if (diff < cosf(DEG2RAD(_data.fov)))
			return false;
	}
	
	int points = 0;
	
	if (value < 3000) {
		float frac = std::min(obj->total_hp / std::max(obj->total_max_hp, 1.0f), 1.0f);
		
		points = (int)(1000.0f * frac);
		if (obj->total_hp < 125) // one shot range
			points = value;
	} else {
		points = 1.e+9;
	}
	
	float calc = (float)(points) / 1000.0f;
	
	{
		if (pos_out)
			*pos_out = org;
		
		if (diff_out)
			*diff_out = calc;
		
		return true;
	}
	
	return false;
}

bool c_aim::get_aim_pos(ow_object* obj, Vector* pos_out, float* diff_out) {
	bool team_mate = false;
	if (local->team != 3) {
		if (local->team == g_state->get_team(obj->filter_bits))
			team_mate = true;
	}
	
	if (_data.type == AIM_TYPE_RAGE && team_mate)
		return false;
	
	if (team_mate) {
		if (local->object.hero == HERO_ZARYA)
			return false;
		
		if (obj->total_hp == obj->total_max_hp)
			return false;
		
		if (local->object.hero == HERO_ANA) {
			if (_data.type != AIM_TYPE_HEAL)
				return false;
		}
	}
	
  if (_data.type == AIM_TYPE_RAGE) {
    if (local->object.hero == HERO_WIDOW && local->sniper_charge < 0.99f) {
      float damage = local->sniper_charge * (300.0f);
      if (obj->armor > 0.0f)
        damage -= 5.0f;
      
      if (damage < obj->total_hp)
        return false;
    }
  }
  
	Vector vieworigin = local->vieworigin;
	Vector viewforward = local->viewforward;
	
	Vector org = obj->type == OBJECT_PLAYER ? obj->model_center : obj->position;
	
	if (_data.type == AIM_TYPE_SECONDARY && local->object.hero == HERO_ZARYA) {
		org = obj->model_min;
		org.y += (obj->model_max.y - obj->model_min.y) * 0.1f;
	}
	
	Vector delta = obj->predicted_delta;
	org += delta;
	
	if (_data.bounds > 0.0f && obj->type == OBJECT_PLAYER) {
		//g_anim->_intersect_flags = (1<<_data.hitbox);
		g_anim->_intersect_mult = _data.bounds;
		
		if (g_anim->is_in_fov(obj, viewforward, delta, 0) == false)
			return false;
	}
	
	Vector dt = (org - vieworigin);
	Vector dt_normal = dt.normal();
	
	float dist = dt.length_sqr();
	float diff = viewforward.dot(dt_normal);
	
	if (_data.type != AIM_TYPE_RAGE) {
		if (diff < 0.0f)
			return false; // ignore any > 90 degrees behind
	}
	
	if (obj->type == OBJECT_PLAYER) {
		if (_data.range > 0.0f && dist > _data.range * _data.range)
			return false;
		
		if (_data.avoid_aids == true && this->is_aids_movement(obj))
			return false;
	} else {
		if (diff < cosf(DEG2RAD(5.0f)))
			return false;
	}
	
	if (_data.type == AIM_TYPE_ABILITY1 && local->object.hero == HERO_ROADHOG) {
		if (diff < cosf(DEG2RAD(45.0f)))
			return false;
	}
	
	if (obj->type != OBJECT_PLAYER || _data.hitbox == -1) {
		if (obj->is_visible == false)
			return false;
		
		if (_data.fov > 0.0f) {
			if (diff < cosf(DEG2RAD(_data.fov)))
				return false;
		}
		
		if (pos_out) {
			if ((local->object.hero == HERO_PHARAH || local->object.hero == HERO_SOLDIER) && obj->on_ground)
				*pos_out = obj->position_root + delta;
			else
				*pos_out = org;
		}
		
		if (diff_out)
			*diff_out = diff;
		
		return true;
	}
	
	matrix_t maty;
	OW_InitY(obj->rotation.y, maty);
	
	int best_points = -1;
	
	Vector position_root = obj->position_root;
	
	bool bot = obj->hero == HERO_TRAININGBOT1 || obj->hero == HERO_TRAININGBOT4;
	
	bool head_visible = obj->hitbox_visible[HITBOX_HEAD];
	bool chest_visible = obj->hitbox_visible[HITBOX_CHEST];
	
	if (bot) {
		Vector head = obj->position_root + Vector(0.0f, 1.96f, 0.0f);
		Vector body = obj->position_root + Vector(0.0f, 1.3f, 0.0f);
		
		for (int i = 0; i < 2; ++i) {
			Vector base = {};
      
			if (this->is_valid_hitbox(i) == false)
        		continue;
			
			if (i == 0 && head_visible == false)
				continue;
			
			if (i == 1 && chest_visible == false)
				continue;
			
			if (i == 0)
				base = head;
			else
				base = body;
			
			Vector pos = base + delta;
			Vector dir = (pos - vieworigin).normal();
			
			int points = 0;
			float diff = dir.dot(viewforward);
			
			if (_data.fov > 0.0f) {
				if (diff < cosf(DEG2RAD(_data.fov)))
					continue;
			}
			
			if (true) {
				float sorting = (1.0f + diff) * 1.e+4f;
				points += (int)(sorting);
				
				if (team_mate)
					points += 1.e+8f;
			}
			
			if (points > best_points) {
				best_points = points;
				
				if (pos_out)
					*pos_out = pos;
				
				if (diff_out)
					*diff_out = diff;
			}
		}
		
		if (best_points == -1)
			return false;
		
		return true;
	}
	
	for (int i = 0; i < obj->hitbox_count; ++i) {
    int hitbox = HITBOX_BODY;
      if (obj->hitbox_id[i] == obj->hitbox_map[HITBOX_HEAD])
        hitbox = HITBOX_HEAD;
      else if (obj->hitbox_id[i] == obj->hitbox_map[HITBOX_CHEST])
        hitbox = HITBOX_CHEST;

    if (this->is_valid_hitbox(hitbox) == false)
          continue;

    if (head_visible == false || chest_visible == false) {
      if (hitbox == HITBOX_HEAD) {
        if (head_visible == false)
          continue;
      } else if (hitbox == HITBOX_CHEST) {
        if (chest_visible == false)
          continue;
      } else {
        continue;
      }
    }
		
		Vector base = {};
		Vector point_offset = {};
		
		float radius = obj->hitbox_radius[i];
		
		if (local->object.hero == HERO_WIDOW) {
			point_offset += Vector(0.0f, radius * 0.5f, 0.0f);
			radius *= 0.5f;
		}
		
		if (_data.random > 0.0f) {
			uint32_t xseed = _xseed;
			uint32_t yseed = _yseed;
			uint32_t zseed = _zseed;
			
			int test = (int)(xseed % 100) * ((xseed & 1) ? -1 : 1);
			int test2 = (int)(yseed % 100) * ((yseed & 1) ? -1 : 1);
			int test3 = (int)(zseed % 100) * ((zseed & 1) ? -1 : 1);
			
			float x = radius * (0.01f * (float)(test));
			float y = radius * (0.01f * (float)(test2));
			float z = radius * (0.01f * (float)(test3));
			
			point_offset += Vector(x, y, z) * _data.random;
		}
		
		if (local->object.hero == HERO_ROADHOG) {
			switch (obj->hero) {
				case HERO_ROADHOG:
				case HERO_DOOM:
				case HERO_SOLDIER:
				case HERO_GENJI:
				case HERO_ASHE:
				case HERO_HANZO: {
					point_offset += Vector(0.0f, -0.05f, 0.0f);
					break;
				}
			}
			
			if (obj->hero == HERO_ANA)
				point_offset += Vector(0.0f, -0.06f, 0.0f);
			
			if (obj->hero == HERO_MEI)
				point_offset += Vector(0.0f, -0.09f, 0.0f);
			
			if (obj->hero == HERO_SOMBRA)
				point_offset += Vector(0.0f, 0.05f, 0.0f);
			
			if (obj->hero == HERO_ORISA)
				point_offset += Vector(0.0f, 0.06f, 0.0f);
			
			if (obj->hero == HERO_MERCY)
				point_offset += Vector(0.0f, 0.08f, 0.0f);
			
			if (obj->hero == HERO_TORB)
				point_offset += Vector(-0.05f, -0.05f, 0.0f);
		}
		
		int hitbox_level = this->get_hitbox_level(obj);
		
		{
			Vector offset = obj->hitbox_offset[i];
			/*Vector min = offset + Vector(box->min[0], box->min[1], box->min[2]);
			Vector max = offset + Vector(box->max[0], box->max[1], box->max[2]);
			
			if (i == HITBOX_HEAD) {
				if (hitbox_level == 1)
					offset = min;
				else if (hitbox_level == 2)
					offset = max;
			}*/
			
			offset += point_offset;
			
			OW_Transform(offset, maty, &base);
			base += position_root;
		}
		
		Vector pos = base + delta;
		Vector dir = (pos - vieworigin).normal();
		
		int points = 0;
		float diff = dir.dot(viewforward);
		
		if (_data.fov > 0.0f) {
			if (diff < cosf(DEG2RAD(_data.fov)))
				continue;
		}
		
		if (_data.distance_sorting) {
			if (_data.type == AIM_TYPE_ABILITY1 && local->object.hero == HERO_ROADHOG) {
				Vector hook_dt = (local->hog_hook_pos - vieworigin);
				dist = (hook_dt.x * hook_dt.x) + (hook_dt.z * hook_dt.z);
			}
			
			if (_data.type == AIM_TYPE_RAGE && obj->total_max_hp > 300.0f)
				dist /= 100.0f;
			
			if (dist > 1.e-9f)
				diff = 1.0f / dist;
		}
		
		if (true) {
			float sorting = (1.0f + diff) * 1.e+4f;
			points += (int)(sorting);
		}
		
		if (points > best_points) {
			best_points = points;
			
			if (pos_out)
				*pos_out = pos;
			
			if (diff_out)
				*diff_out = diff;
		}
	}
	
	if (best_points == -1)
		return false;
	
	return true;
}

bool c_aim::get_target() {
	float best_diff = -1.0f;
	
	_aim_obj = nullptr;
	
	for (int i = 0; i < g_state->_object_count; ++i) {
		ow_object* obj = &(g_state->_objects[i]);
		
		if (_data.single_target) {
			if (_last_obj != nullptr) {
				if (obj != _last_obj)
					continue;
			}
		}
		
		if (!this->is_valid_target(obj))
			continue;
		
		Vector pos = Vector(0.0f, 0.0f, 0.0f);
		float diff = 0.0f;
		
		if (_data.type == AIM_TYPE_HEAL) {
			if (!this->get_heal_pos(obj, &pos, &diff))
				continue;
		} else {
			if (!this->get_aim_pos(obj, &pos, &diff))
				continue;
		}
		
		if (_data.type == AIM_TYPE_RAGE) {
			if (obj == _last_obj)
				diff = 1.e+9f;
		}
		
		if (diff > best_diff) {
			best_diff = diff;
			
			_aim_obj = obj;
			_aim_pos = pos;
		}
	}
	
	if (best_diff != -1.0f) {
		if (_data.type == AIM_TYPE_RAGE) {
			if (_aim_obj != _last_obj)
				_last_obj = nullptr;
		}
		
		if (_last_obj == nullptr)
			_last_obj = _aim_obj;
		
		return true;
	}
	
	return false;
}

void c_aim::think() {
	_aiming = false;
	_on_heal_target = false;
	
	if (!this->update()) {
		_aim_time = 0.0f;
		_cur_speed = 0.0f;
		_xseed = FNV1A_RT((char*)&(g_ow->_game_tickcount), 2);
		_yseed = FNV1A_RT((char*)&(g_ow->_game_tickcount), 3);
		_zseed = FNV1A_RT((char*)&(g_ow->_game_tickcount), 4);
	}
	
	this->autofire_think();
}

bool c_aim::update() {
	if (_data.type == AIM_TYPE_NONE) {
		_key_time = 0.0f;
		_autofire_time = 0.0f;
		_last_obj = nullptr;
		
		pseudo_cmd* change_cmd = &(g_ow->_cmd);
		
		if (_arrow_drawn) {
			change_cmd->buttons |= IN_ATTACK|IN_ATTACK2;
			
			_arrow_drawn = false;
		}
		
		if (_zen_charging) {
			float wait = (float)(aim_config.zen_wait_orbs) * 0.2f;
			if (local->zen_orbs < wait)
				change_cmd->buttons |= IN_ATTACK2;
			
			float ammo = 0.0f;
			
			ow_condition* cond = g_state->lookup_condition(&(local->object), HASH("zen_ammo"));
			if (cond)
				ammo = cond->data.fl;
			
			if (ammo > 0.0f) {
				//if (g_cvars_misc->zen_melee.value != 0)
					//change_cmd->buttons |= IN_MELEE;
			}
			
			_zen_charging = false;
		}
		
		return false;
	}
	
	if (local->object.hero == HERO_HANZO && local->ability_limit != 0x5a1 && _data.type != AIM_TYPE_RAGE) {
		if (_data.auto_fire != 0) {
			pseudo_cmd* change_cmd = &(g_ow->_cmd);
			
			change_cmd->buttons |= IN_ATTACK;
			
			if (local->sniper_charge < _data.wait_time)
				_arrow_drawn = true;
			else
				_arrow_drawn = false;
		}
	}
	
	if (local->object.hero == HERO_ZEN) {
		if (_data.auto_fire != 0) {
			pseudo_cmd* change_cmd = &(g_ow->_cmd);
			
			bool cycle = true;
			
			if (local->zen_orbs == 0.0f) {
				if (g_ow->_game_tickcount % 2)
					cycle = false;
			}
			
			if (cycle)
				change_cmd->buttons |= IN_ATTACK2;
			
			_zen_charging = true;
		}
	}
	
	_key_time += OW_TICK_INTERVAL;
	_autofire_time += OW_TICK_INTERVAL;
	
	if (!this->get_target())
		return false;
	
	if (!this->set_aim())
		return false;
	
	return true;
}

bool c_aim::pre_aim() {
	bool aim = true;
	
	if (local->fire_time != 0.0f && _data.wait_time != 0.0f) {
		if (local->object.hero == HERO_WIDOW || (local->object.hero == HERO_HANZO && local->ability_limit != 0x5a1)) {
			if (local->sniper_charge < _data.wait_time)
				aim = false;
		} else {
			if (local->fire_time < _data.wait_time)
				aim = false;
		}
	}
	
	if (_data.type == AIM_TYPE_ZEN) {
		float wait = (float)(aim_config.zen_wait_orbs) * 0.2f;
		if (local->zen_orbs < wait)
			aim = false;
	}
	
	if (_aim_obj->type != OBJECT_PLAYER) {
		aim = true;
		// object aim stuff
	}
	
	Vector v1 = _aim_obj->velocity;
	Vector v2 = local->object.velocity;
	
	v1.y = 0.0f;
	v2.y = 0.0f;
	
	Vector n1 = v1.normal();
	Vector n2 = v2.normal();
	
	float move_dir = n1.dot(n2);
	
	Vector dt = (_aim_pos - local->vieworigin).normal();
	_aim_dir = dt;
	
	if (_data.movement) {
		if ((v1.length() > 0.1f || v2.length() > 0.1f) && move_dir < 0.707f)
			aim = false;
	}
	
	if (g_trig->_trig_aiming)
		aim = false;
	
	return aim;
}

void c_aim::apply_smooth(Vector forward, bool silent) {
  Vector dt = _aim_dir;
  
  float frame_mult = 1.0f;
    
  if (false && g_ow->_call_count > 0)
    frame_mult = (1.0f / (float)(g_ow->_call_count));
  
  float speed = 1.0f;
  if (_data.smooth_accel > 0.0f) {
    _cur_speed += _data.smooth_accel;
    _cur_speed = std::min(_cur_speed, 1.0f);
    speed = _cur_speed;
  }
  
  float sine = 1.0f;
  if (_data.smooth_sine > 0.0f) {
    float sine_length = _data.smooth_sine;
    
    // 5.0f = 200 MS
    float angle = fmodf(5.0f * (g_state->_curtime / OW_TICK_INTERVAL), 360.0f);
    sine = (1.5f - (0.5f * (1.0f - sine_length))) - (fabsf(sinf(DEG2RAD(angle))) * sine_length);
  }
  
  int type = _data.smooth_type;
  
  if (type == 1) {
    Vector pred_dt = dt;
    
    {
      int ticks = 1;
      
      Vector delta = Vector(0.0f, 0.0f, 0.0f);
      g_anim->calculate_delta(_aim_obj, ticks, 22.0f, &delta);
      
      Vector newpos = _aim_pos + (delta * frame_mult);
      
      Vector local_delta = Vector(0.0f, 0.0f, 0.0f);
      g_anim->calculate_delta(&(local->object), ticks, 22.0f, &local_delta);
      
      Vector neworg = local->vieworigin + (local_delta * frame_mult);
      
      Vector normal = (newpos - neworg).normal();
      
      pred_dt = normal;
    }
    
    Vector newfwd = forward;
    
    float mod = 1.0f;
    float dot = forward.dot(dt);
    
    float speed_deg = RAD2DEG(acosf(dt.dot(pred_dt)));
    speed_deg *= _data.smooth_correction;
    
    float result = std::min(RAD2DEG(acosf(dot)), 40.0f);
    mod = std::max((float)((int)(result)), 1.0f);
    
    float precision = std::max(0.066f * _data.smooth_const, 1.e-7f);
    
    float degree = precision * mod;
    degree *= frame_mult;
    
    degree += speed_deg;
    
    degree *= sine;
    degree *= speed;
    
    Vector aimangles = Vector(dt.pitch(), dt.yaw(), 0.0f);
    Vector viewangles = Vector(forward.pitch(), forward.yaw(), 0.0f);
    Vector angle_diff = aimangles - viewangles;
    
    NormalizeAngle(&(angle_diff.x));
    NormalizeAngle(&(angle_diff.y));
    
    Vector new_angle_diff = angle_diff;
    new_angle_diff.x = precision * (float)((int)(new_angle_diff.x / precision));
    new_angle_diff.y = precision * (float)((int)(new_angle_diff.y / precision));
    
    if (new_angle_diff.x > degree)
      new_angle_diff.x = degree;
    else if (new_angle_diff.x < -degree)
      new_angle_diff.x = -degree;
    
    if (new_angle_diff.y > degree)
      new_angle_diff.y = degree;
    else if (new_angle_diff.y < -degree)
      new_angle_diff.y = -degree;
    
    Vector new_ang = viewangles + new_angle_diff;
    
    Vector new_dt = new_ang.forward().normal();
    new_dt = new_dt.normal();
    
    _new_dir = new_dt;
  } else {
    float smooth = _data.smooth;
    float smooth_mul = 1.0f;
    if (smooth > 0.0f)
      smooth_mul = (1.0f / smooth);
    
    smooth_mul *= sine;
    smooth_mul *= speed;
    
    smooth_mul *= frame_mult;
    
    if (_data.smooth_correction > 0.0f && smooth > 0.0f) {
      float temp = smooth_mul;
      if (temp < 1.e-5f)
        temp = 1.e-5f;
      
      float t = 1.0f / temp;
      t *= _data.smooth_correction * speed;
      
      int ticks = (int)(t + 0.5f) - 1;
      
      Vector delta = Vector(0.0f, 0.0f, 0.0f);
      g_anim->calculate_delta(_aim_obj, ticks, 0.0f, &delta);
      
      Vector newpos = _aim_pos + (delta * frame_mult);
      
      Vector local_delta = Vector(0.0f, 0.0f, 0.0f);
      g_anim->calculate_delta(&(local->object), ticks, 0.0f, &local_delta);
      
      Vector neworg = local->vieworigin + (local_delta * frame_mult);
      
      Vector normal = (newpos - neworg).normal();
      
      dt = normal;
    }
    
    Vector smooth_dt = (dt - forward) * smooth_mul;
    _new_dir = forward + smooth_dt;
    _new_dir = _new_dir.normal();
  }
}

bool c_aim::set_aim() {
	pseudo_cmd* ucmd = &(g_ow->_cmd);
	Vector viewforward = g_ow->_cmd.forward;
	
	if (_aim_obj->type != OBJECT_PLAYER)
		_data.smooth_correction = 1.0f;
	
	if (this->pre_aim() == false) {
		if (_data.auto_fire == 2)
			_autofire_time = 0.0f;
		
		return false;
	}
	
	if (_data.type != AIM_TYPE_RAGE) {
		{
			Vector test = Vector(90.0f, 0.0f, 0.0f);
			Vector test_forward = test.forward().normal();
			
			if (_aim_dir.dot(test_forward) > 0.93969f)
				return false;
		}
		
		{
			Vector test = Vector(-90.0f, 0.0f, 0.0f);
			Vector test_forward = test.forward().normal();
			
			if (_aim_dir.dot(test_forward) > 0.93969f)
				return false;
		}
	}
	
	{
    	if (false == false)
	 		this->apply_smooth(viewforward, false);
		
		bool rotate = true;
		
		if (rotate) {
			Vector new_dir = _new_dir.normal();
			
      if (false == false) {
  			ucmd->forward = new_dir;
  			
  			if (_data.type != AIM_TYPE_RAGE)
  				ucmd->view = new_dir;
      }
      
      if (_data.type == AIM_TYPE_RAGE)
        ucmd->forward = _aim_dir.normal();
			
			float* x = (float*)(g_ow->_singleton_input.component + 0x1560);
			float* y = (float*)(g_ow->_singleton_input.component + 0x1564);
			
			if (local->object.hero == HERO_MCCREE && _data.type != AIM_TYPE_RAGE) {
				//if (g_cvars_misc->cree_test.value != 0) {
					*x = 0.0f;
					*y = 0.0f;
				//}
			}
		}
	}
	
	_aiming = true;
	
	_aim_time += OW_TICK_INTERVAL;
	
	_backward_tick = _aim_obj->predicted_backward;
	_last_aimed = g_state->_curtime;
	
	return true;
}

void c_aim::autofire_think() {
	pseudo_cmd* ucmd = &(g_ow->_cmd);
	bool auto_fire = false;
	
	if (_data.auto_fire != 0) {
		if (_data.autofire_time > 0.0f && _autofire_time >= _data.autofire_time)
			g_trig->_first_fired = true;
		
		if (_data.auto_fire == 2 && _aiming)
			auto_fire = true;
		else if (_data.auto_fire == 1)
			auto_fire = true;
	}
	
	if (_data.autofire_time > 0.0f && _autofire_time < _data.autofire_time)
		auto_fire = false;
	
	if (local->object.hero == HERO_HANZO && local->ability_limit != 0x5a1 && _data.type != AIM_TYPE_RAGE)
		auto_fire = false;
	
	if (local->object.hero == HERO_HANZO && local->ability_limit == 0x5a1 && _data.autofire_time == 0.0f)
		auto_fire = false;
	
	if (g_trig->_data.type != TRIG_TYPE_NONE && g_trig->_first_fired == false)
		auto_fire = false;
	
	if (_data.type == AIM_TYPE_ABILITY2 || _data.type == AIM_TYPE_HEAL)
		auto_fire = false;
  
  
	
	if (_data.type == AIM_TYPE_RAGE) {
		if (_aiming)
			auto_fire = true;
		else
			auto_fire = false;
		
		if (local->object.hero == HERO_HANZO && local->ability_limit != 0x5a1) {
			if (_aiming && local->sniper_charge > 0.99f)
				auto_fire = false;
			else
				auto_fire = true;
		}
    
    if (local->object.hero == HERO_BRIG && g_ow->_game_tickcount % 2)
      auto_fire = false;
	}
  
  
	if (_data.type == AIM_TYPE_HEAL && _aiming) {
		Vector delta = _aim_obj->predicted_delta;
		g_anim->_intersect_mult = 0.9f;
		
		Vector new_forward = g_ow->_cmd.forward;
		
		if (g_anim->find_capsule_intersection(_aim_obj, new_forward, delta) != -1)
			auto_fire = true;
	}
	
	if (auto_fire) {
		if (_data.type == AIM_TYPE_HOOK)
			ucmd->buttons |= IN_ABILITY1;
		else if (_data.type == AIM_TYPE_PRIMARY || _data.type == AIM_TYPE_RAGE || _data.type == AIM_TYPE_HEAL)
			ucmd->buttons |= IN_ATTACK;
		else if (_data.type == AIM_TYPE_SECONDARY )//|| _data.type == AIM_TYPE_ZEN)
			ucmd->buttons |= IN_ATTACK2;
	}
	
	if (true && local->fire_ready == false)
		_autofire_time = 0.0f;
}

void c_aim::force_tick() {
	//if (g_cvars->history_lowest.value == 0)
	//	return;
	
	if ((g_state->_curtime - _last_aimed) > 1.0f)
		return;
	
	g_anim->_tick_force_ms = (int)((OW_TICK_INTERVAL * (float)(_backward_tick)) * 1000.0f);
	if (g_anim->_tick_force_ms + g_anim->_tick_limit_ms < 0)
		g_anim->_tick_force_ms = g_anim->_tick_limit_ms * -1;
}