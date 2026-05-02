#include "link.h"

c_hero hero_instance;
c_hero* g_hero = &hero_instance;

void c_hero::test() {
	float best_len = 1.e+9f;
	ow_object* best_obj = nullptr;
	
	_beasd = false;
	
	for (int i = 0; i < g_state->_object_count; ++i) {
		ow_object* obj = &(g_state->_objects[i]);
		if (obj->type != OBJECT_PLAYER)
			continue;
		
		if (!g_state->is_alive(obj))
			continue;
		
		if (g_state->get_team(local->object.filter_bits) != g_state->get_team(obj->filter_bits))
			continue;
		
		//if (obj->hero == HERO_ANA || obj->hero == HERO_LUCIO)
	//		continue;
		
		float len = (obj->position - local->vieworigin).length();
		if (obj->total_max_hp > 300.0f)
			len *= 0.5f;
		
		if (len < best_len) {
			best_len = len;
			best_obj = obj;
		}
	}
	
	if (best_obj != nullptr) {
		_beasd = true;
		_beasd_dir = (best_obj->position - local->vieworigin).normal();
		_beasd_dist = (best_obj->position - local->vieworigin).length();
	}
}

void c_hero::think() {
	this->test();
	
	aim_data* aim = &(g_aim->_data);
	memset(aim, 0, sizeof(aim_data));
	
	trig_data* trig = &(g_trig->_data);
	memset(trig, 0, sizeof(trig_data));
	
	pseudo_cmd* ucmd = &(g_ow->_cmd);
	
	int hero = local->object.hero;
	switch (hero) {
		case HERO_SOLDIER: {
			g_offense->soldier_think();
			break;
		}
		case HERO_MERCY: {
			g_healer->think();
			break;
		}
	}
	
	if (config->misc.bhop != 0) {
		bool on_ground = true;
		
		switch (hero) {
			case HERO_MERCY:
			case HERO_PHARAH:
			case HERO_GENJI:
			case HERO_HANZO: {
				break;
			}
			default: {
				on_ground = local->object.on_ground;
				break;
			}
		}
		
		if (on_ground == false)
			ucmd->buttons &= ~IN_JUMP;
	}
	
	if (false) {//if (g_cvars_misc->misc_auto_duck.value != 0) {
		if (ucmd->buttons & IN_DUCK) {
			if (g_ow->_game_tickcount % 6 < 3)
				ucmd->buttons &= ~IN_DUCK;
		}
	}
	
	if (hero == HERO_MERCY) {
	//	if (this->get_aim_key(g_cvars_misc->mercy_super_jump.value) != -1) {
		//	if ((ucmd->buttons & IN_JUMP) == 0)
		//		ucmd->buttons |= IN_ABILITY1|IN_DUCK;
		//}
	}
}

int c_hero::get_aim_key(int value) {
  if (value < 0 || value >= 0xFF)
    return -1;
  
  if (input_system->held((u8)value))
    return 0;
  
	return -1;
}

void c_hero::aim_setup() {
	aim_data* aim = &(g_aim->_data);
	
	{
		bool pressing = false;
		
		if (this->get_aim_key(aim_config.aim_hotkey) != -1)
			pressing = true;
		
		if (!aim_config.enabled)
			pressing = false;
		
		if (pressing)
			aim->type = AIM_TYPE_PRIMARY;
	}
	
	{
		aim->hitbox = 0;
    if (aim_config.aim_hitbox_head)
      aim->hitbox |= 1<<0;
    if (aim_config.aim_hitbox_body)
      aim->hitbox |= 1<<1;
    if (aim_config.aim_hitbox_limbs)
      aim->hitbox |= 1<<2;
    
		aim->fov = aim_config.aim_fov;

		aim->bounds = aim_config.aim_dynamic_fov * 0.01f;
		aim->random = aim_config.aim_random_point * 0.01f;
    
    aim->smooth = aim_config.aim_smooth;
    aim->smooth_const = aim_config.aim_smooth_const; 
    
    if (aim_config.aim_use_const)
      aim->smooth_type = 1;
    
    aim->smooth_correction = aim_config.aim_smooth_correction * 0.01f;
    aim->smooth_accel = aim_config.aim_smooth_accel * 0.01f;
    aim->smooth_sine = aim_config.aim_smooth_sine * 0.01f;
	}
	
	{
		//aim->auto_fire = g_cvars->aim_autofire.value;
	//	aim->movement = g_cvars->aim_movement.value != 0 ? true : false;
	//	aim->single_target = g_cvars->aim_single_target.value != 0 ? true : false;
    //aim->single_target = local->object.hero == HERO_HANZO ? false : true;
    aim->single_target = aim_config.sticky_aim;
	//	aim->ignore_objects = g_cvars->aim_ignore_objects.value != 0 ? true : false;
		aim->wait_time = aim_config.aim_wait_time / 100.0f;
    aim->auto_fire = aim_config.aim_autofire_time > 0.0f ? 2 : 0;
		aim->autofire_time = aim_config.aim_autofire_time / 1000.0f;
	//	aim->range = (float)(g_cvars->aim_range.value) / 100.0f;
		
    aim->shoot_bubbled = true;
		//aim->shoot_bubbled = g_cvars->aim_ignore_bubbled.value != 0 ? false : true;
		//aim->avoid_aids = g_cvars->aim_avoid_aids.value != 0 ? true : false;
	}
}

void c_hero::post_aim_setup() {
	aim_data* aim = &(g_aim->_data);
	pseudo_cmd* ucmd = (pseudo_cmd*)&(g_ow->_cmd);
	
	int hero = local->object.hero;
	
	float attack_recovery = g_hero_data[hero].attack_recovery;
	float attack_range = g_hero_data[hero].attack_range;
	float attack_falloff_end = g_hero_data[hero].attack_falloff_end;
	
	float attack2_recovery = g_hero_data[hero].attack2_recovery;
	
	if (hero == HERO_GENJI) {
		float delta = g_state->_curtime - g_offense->_genji_next_secondary;
		if (delta < local->genji_primary)
			attack_recovery = g_hero_data[hero].attack2_recovery;
	}
	
	if (hero == HERO_MEI) {
		attack_recovery = g_hero_data[hero].attack2_recovery;
		attack_range = g_hero_data[hero].attack2_range;
	}
	
	if (hero == HERO_ZARYA)
		attack_recovery = g_hero_data[hero].attack2_recovery;
	
	if (hero == HERO_WIDOW)
		attack_recovery = 1.0f;
	else if (hero == HERO_ASHE && local->ads == 0.0f)
		attack_recovery = attack2_recovery;
	else if (hero == HERO_HANZO && local->ability_limit == 0x5a1)
		attack_recovery = 0.25f;
	
	if (hero == HERO_SIGMA) {
		if (this->_sigma_attack_interval == 0)
			attack_recovery = 1.3f;
	}
  
  if (hero == HERO_BAPTISTE) {
    if (local->bap_burst)
      attack_recovery = 0.1f;
  }
	
//	if (local->object.pool_type == POOLID_DVAMECH)
//		attack_recovery = 1.0f / 7.0f;
	
	aim->wait_time = attack_recovery * aim->wait_time;
	
	if (local->no_headshot)
		aim->hitbox = -1;
}

void c_hero::aim_think() {
	aim_data* aim = &(g_aim->_data);
	trig_data* trig = &(g_trig->_data);
	
	pseudo_cmd* ucmd = (pseudo_cmd*)&(g_ow->_cmd);
	int hero = local->object.hero;
	
	this->aim_setup();
	
	switch (hero) {
		case HERO_ASHE: {
			if (local->ads == 0.0f) {
			/*	if (g_cvars_misc->ashe_smooth_min.value != 0) {
					aim->smooth = (float)(g_cvars_misc->ashe_smooth_min.value);
					aim->smooth_max = (float)(g_cvars_misc->ashe_smooth_max.value);
					aim->scale_min = (float)(g_cvars_misc->ashe_smooth_scale_min.value);
					aim->scale_max = (float)(g_cvars_misc->ashe_smooth_scale_max.value);
					aim->smooth_fix = (float)(g_cvars_misc->ashe_smooth_fix.value) * 0.01f;
				}*/
			}
			break;
		}
		case HERO_HANZO: {
			if (local->ability_limit == 0x5a1) {
				/*if (g_cvars_misc->hanzo_smooth_min.value != 0) {
					aim->smooth = (float)(g_cvars_misc->hanzo_smooth_min.value);
					aim->smooth_max = (float)(g_cvars_misc->hanzo_smooth_max.value);
					aim->scale_min = (float)(g_cvars_misc->hanzo_smooth_scale_min.value);
					aim->scale_max = (float)(g_cvars_misc->hanzo_smooth_scale_max.value);
					aim->smooth_fix = (float)(g_cvars_misc->hanzo_smooth_fix.value) * 0.01f;
				}*/
			}
			break;
		}
		case HERO_MEI: {
			if (aim->type == AIM_TYPE_PRIMARY)
				aim->type = AIM_TYPE_SECONDARY;
			
			break;
		}
		case HERO_ZEN: {
			bool pressing = false;
			
			i32 type = 1;

			if (type == 1) {
				if (input_system->held(aim_config.zen_wait_orbs_hotkey))
					pressing = true;

				if (pressing)
					aim->auto_fire = 1;
			} else if (type == 2) {
				if (ucmd->buttons & IN_ATTACK2) {
					pressing = true;
					aim->auto_fire = 0;
				}
				
			} else
				pressing = false;
			
			if (pressing)
				aim->type = AIM_TYPE_ZEN;
			
			float wait = (float)(aim_config.zen_wait_orbs) * 0.2f;
			if (g_aim->_last_type == AIM_TYPE_ZEN && aim->type == AIM_TYPE_NONE && local->zen_orbs >= wait) {
				aim->type = AIM_TYPE_ZEN;
				aim->auto_fire = 0;
			}
			
			break;
		}
		case HERO_ANA: {
			bool pressing = false;
			
		/*	if (this->get_aim_key(g_cvars_misc->heal_key.value) != -1)
				pressing = true;
			
			if (g_cvars_misc->heal_key.value == -1)
				pressing = true;
			
			if (g_cvars_misc->heal_type.value == 0)
				pressing = false;
			
			if (pressing) {
				memset(aim, 0, sizeof(aim_data));
				
				aim->type = AIM_TYPE_HEAL;
				aim->silent =  g_cvars_misc->heal_type.value == 2;
					
				aim->hitbox = -1;
				
				aim->fov = (float)(g_cvars_misc->heal_fov.value);
				
				aim->smooth = (float)(g_cvars_misc->heal_smooth.value);
				
				aim->auto_fire = true;
				aim->wait_time = 0.5f;
			}*/
			
			if (local->ability_limit == 0x478) {
				//memset(aim, 0, sizeof(aim_data));
				
				aim->type = AIM_TYPE_ABILITY1;
				
				aim->bounds = 7.5f;
				aim->hitbox = -1;
				
				aim->range = 50.0f;
				
				aim->single_target = true;
			}
		
			break;
		}
		case HERO_DOOM: {
			/*if (ucmd->buttons & IN_ATTACK2 && local->rocket_punch > (float)(g_cvars_misc->doom_punch.value) * 0.01f) {
				//memset(aim, 0, sizeof(aim_data));
				
				aim->type = AIM_TYPE_ABILITY1;
				
				aim->bounds = 5.0f;
				aim->hitbox = -1;
				
				aim->range = 50.0f;
			//	aim->smooth = 5.0f;
				//aim->smooth_fix = 1.0f;
				
				aim->single_target = true;
			}*/
			
			break;
		}
		case HERO_WIDOW: {
			if (ucmd->buttons & IN_ATTACK && local->sniper_charge < 1.e-5f && false) {
				memset(aim, 0, sizeof(aim_data));
				
				aim->type = AIM_TYPE_SECONDARY;
				
				aim->range = 20.0f;
				aim->smooth = 5.0f;
				
				aim->single_target = true;
			}
			
			break;
		}
		case HERO_ECHO: {
			if (local->ability_limit == 0x6fc && false) {
				memset(aim, 0, sizeof(aim_data));
				
				aim->type = AIM_TYPE_ABILITY2;
				
				aim->range = 16.0f;
				aim->smooth = 5.0f;
				
				aim->hitbox = -1;
				
				aim->single_target = true;
			} else if (local->ability_limit == 0x1e && false) {
				memset(aim, 0, sizeof(aim_data));
				
				aim->type = AIM_TYPE_SECONDARY;
				
				aim->range = 10.0f;
				aim->smooth = 5.0f;
				
				aim->hitbox = -1;
				
				aim->single_target = true;
			}
			
			break;
		}
		case HERO_GENJI: {		
			bool deflecting = false;
			
			ow_condition* deflect = g_state->lookup_condition(&(local->object), HASH("usage_e"));
			if (deflect) {
				if (deflect->data.u32 & 1)
					deflecting = true;
			}
			
			if (deflecting && false) {
				memset(aim, 0, sizeof(aim_data));
				
				aim->type = AIM_TYPE_ABILITY2;
				
				aim->range = 50.0f;
				aim->smooth = 5.0f;
				
				aim->single_target = true;
			}
			
			break;
		}
		case HERO_TRACER: {
			bool ult = false;
			if (local->ability_limit == 0x16)
				ult = true;
			
			if (local->object.ult_charge == 100.0f)
				ult = true;
			
			bool pressing = false;
		//	if (this->get_aim_key(g_cvars_misc->tracer_ult_key.value) != -1)
			//	pressing = true;
			
			if (pressing && ult) {
				aim->type = AIM_TYPE_ABILITY3;
				
				aim->range = 5.0f; // max range about 10
				aim->hitbox = -1;
				
				trig->type = TRIG_TYPE_ABILITY3;
				trig->range = 5.0f;
				trig->hitbox = -1;
			}
			
			break;
		}
		case HERO_MCCREE: {
			if (ucmd->buttons & IN_ATTACK2 && false) {
				memset(aim, 0, sizeof(aim_data));
				
				aim->type = AIM_TYPE_SECONDARY;
				
				aim->hitbox = HITBOX_CHEST;
				
				aim->range = 7.5f;
				aim->smooth = 5.0f;
				
				aim->single_target = true;
			}
			
			break;
		}
		case HERO_ROADHOG: {
			float hook_time = 0.0f;
			
			ow_condition* hook = g_state->lookup_condition(&(local->object), HASH("hog_hook_time"));
			if (hook)
				hook_time = hook->data.fl;
			
			float cooldown = 0.0f;
			ow_condition* shift = g_state->lookup_condition(&(local->object), HASH("cooldown_shift"));
			if (shift)
				cooldown = shift->data.fl;
			
			bool crosshair = false;
			float dt = g_state->_curtime - g_local->_hook_attach_time;
			
			if (dt < 0.5f)
				crosshair = true;
			
			if (local->move_speed_mult == 0.75f && aim->type != AIM_TYPE_NONE) {
				
			} else if (crosshair) { // hook attached
				memset(aim, 0, sizeof(aim_data));
				
				aim->type = AIM_TYPE_ABILITY1;
        
        aim->hitbox = 1<<0;
				
				aim->bounds = 5.0f;
				
				aim->smooth = 3.0f;
				
				aim->range = 3.0f;
				
				aim->single_target = true;
				aim->distance_sorting = false;
			} else if (local->throwing_hook) { // throwing hook
        memset(aim, 0, sizeof(aim_data));
        aim->type = AIM_TYPE_HOOK;
        
        aim->bounds = 5.0f;
        aim->hitbox = -1;
        
        aim->smooth_type = 1;
        aim->smooth_const = 3.5f;
        aim->smooth_correction = 1.0f;
        aim->range = 25.0f * 25.0f;
        
        aim->single_target = true;
			} 
			
			break;
		}
		/*case HERO_ZARYA: {
			aim->hitbox = -1;
			aim->hitscan = 0;
			aim->pointscan = 0;
			
			bool right_click = false;
			
			bool key = this->get_ability_mode() != -1;
			if (key)
				right_click = true;
			
			if (right_click && g_cvars->ability_smooth.value > 0) {
				aim->type = AIM_TYPE_SECONDARY;
				//aim->auto_fire = true;
				
				//aim->fov_mult = 5.0f;
				//aim->fov_time = 0.0f;
				//aim->wait_time = 0.0f;
				aim->range = 100.0f;
				aim->smooth = (float)(g_cvars->ability_smooth.value);
				aim->smooth_fix = 0.0f;
				
				aim->projectile_speed = 25.0f;
				
				aim->single_target = false;
				//aim->force_every_frame = true;
				
				trig->type = TRIG_TYPE_SECONDARY;
				trig->hitbox = -1;
				trig->projectile_speed = 25.0f;
			}
			
			break;
		}*/
		case HERO_MERCY: {
			if (local->cur_weapon == 1.0f)
				aim->type = 0;
			
			break;
		}
		default: {
			
			
			break;
		}
	}
	
	this->post_aim_setup();
	
  if (config->aimbot[0].rage_enabled && input_system->held((u8)(config->aimbot[0].rage_hotkey))) {
		memset(aim, 0, sizeof(aim_data));
		
		aim->type = AIM_TYPE_RAGE;
		aim->hitbox = 1<<0;
    
		aim->auto_fire = 2;
		aim->range = 200.0f;
		
		aim->distance_sorting = false;
	}
	
	if (!g_state->is_fire_ready())
		aim->type = AIM_TYPE_NONE;
	
	g_aim->_last_type = aim->type;
}

void c_hero::trig_think() {
	trig_data* trig = &(g_trig->_data);
	
	if (g_aim->_data.type == AIM_TYPE_RAGE)
		return;
	
	int hero = local->object.hero;
	
	{
		//trig->fire_delay = (float)(g_cvars->trig_delay.value) / 1000.0f;
		trig->hitboxes = 0;
    if (aim_config.trig_hitbox_head)
      trig->hitboxes |= 1<<0;
    if (aim_config.trig_hitbox_body)
      trig->hitboxes |= 1<<1;
    if (aim_config.trig_hitbox_limbs)
      trig->hitboxes |= 1<<2;
    
		trig->hitbox_scale = 0.9f;//(float)(g_cvars->history_hitbox_scale.value) / 100.0f;
		trig->min_accuracy = aim_config.trig_min_charge * 0.01f;//(float)(g_cvars->trig_min_accuracy.value) / 100.0f;
		trig->history = aim_config.trig_backtrack;
		trig->history_pred = aim_config.trig_forwardtrack;
		trig->objects = aim_config.trig_shoot_objects;
    
    if (aim_config.ghost_aim)
      trig->ghost_aim = aim_config.ghost_aim_amount * 0.01f;
		//trig->avoid_aim = (float)(g_cvars->trig_avoid_aim.value) / 100.0f;
		
		//s_vkey_names
	//	if (g_cvars->trig_key.value == 0)
		//	trig->in_attack = true;
	}
	
	if (aim_config.trig_enabled == true) {
    if (this->get_aim_key(aim_config.trig_hotkey) != -1)
      trig->pressing = true;
		
		bool secondary = hero == HERO_GENJI || hero == HERO_MEI;
		
		if (trig->pressing)
			trig->type = secondary ? TRIG_TYPE_SECONDARY : TRIG_TYPE_PRIMARY;
		
		if (g_aim->_data.type == AIM_TYPE_ZEN)
			trig->type = TRIG_TYPE_ZEN;
		
		if (local->no_headshot)
			trig->hitbox = -1;
	}
	
	if (trig->pressing) {
		if (g_trig->_press_time == 0.0f)
			g_trig->_press_time = g_state->_curtime;
		
		g_trig->_press_time_diff = g_state->_curtime - g_trig->_press_time;
	} else {
		g_trig->_press_time = 0.0f;
		g_trig->_press_time_diff = 0.0f;
	}
}

// YASRJ (workshop code) for info on melee hitboxes
// fix for invulnerable targets
void c_hero::auto_melee() { // auto melee when friendly hog gets a hook?
//	if (!g_cvars_misc->misc_auto_melee.value)
//		return;
	
	if (local->object.hero == HERO_REINHARDT || local->object.hero == HERO_BRIG)
		return;
	
	pseudo_cmd* ucmd = (pseudo_cmd*)&(g_ow->_cmd);
	Vector fwd = ucmd->forward.normal();
	
	for (int i = 0; i < g_state->_object_count; ++i) {
		ow_object* obj = &(g_state->_objects[i]);
		if (obj->type != OBJECT_PLAYER)
			continue;
		
		if (!g_state->is_alive(obj))
			continue;
		
		if (g_state->get_team(local->object.filter_bits) == g_state->get_team(obj->filter_bits))
			continue;
    
    if (g_state->is_invuln(obj, true))
      continue;
		
		Vector org = obj->position_root;
		// 1.5 distance, 1.0 radius
		if (g_anim->_melee_frame != 0) {
			int f = g_anim->_frame_count - g_anim->_melee_frame;
			if (f < 0)
				f = 0;
			
			c_anim_frame* frame = &(g_anim->_frames[f % MAX_ANIM_FRAMES]);
			for (int n = 0; n < frame->_object_count; ++n) {
				c_anim_object* anim_obj = &(frame->_objects[n]);
				if (anim_obj->_entity_parent == obj->entity_parent) {
					org = anim_obj->_position_root;
					break;
				}
			}
		}
		
		float height = obj->model_max.y - obj->position_root.y;
		
		Vector melee = local->vieworigin + (fwd * 1.5f);
		
		Vector dt = (org - melee);
		Vector dt_normal = dt.normal();
		
		float intersect = false;
		
		for (float f = 0.0f; f <= 0.8f; f += 0.2f) {
			if (((org + Vector(0.0f, height * f, 0.0f)) - melee).length() < 1.0f + 0.4f) {
				intersect = true;
				break;
			}
		}
		
		if (intersect == false)
			continue;
		
		float damage = 30.0f;
		if (obj->armor > 0.0f)
			damage -= 5.0f;
		
		float hp = obj->total_hp;
		
		switch (obj->hero) {
			case HERO_ORISA: {
				ow_condition* shift = g_state->lookup_condition(obj, HASH("usage_shift"));
				if (shift) {
					if (shift->data.u32 & 1)
						damage *= 0.5f;
				}
				
				break;
			}
			case HERO_ROADHOG: {
				ow_condition* e = g_state->lookup_condition(obj, HASH("usage_e"));
				if (e) {
					if (e->data.u32 & 1)
						damage *= 0.5f;
				}
				
				break;
			}
		}
		
		if (hp <= damage) {
			if (local->ability_limit != 0xD) {
				if ((g_ow->_last_buttons & IN_MELEE) == 0)
					g_ow->_cmd.buttons |= IN_MELEE;
			}
			
			break;
		}
	}
}

void c_hero::zarya_super_jump() {
	if (local->object.hero != HERO_ZARYA)
		return;
	
	pseudo_cmd* ucmd = (pseudo_cmd*)&(g_ow->_cmd);
	
	if (ucmd->buttons & IN_USE) {
		Vector angles = Vector(ucmd->forward.pitch(), ucmd->forward.yaw(), 0.0f);
		
		int n = _jump_time % (62*6);
		_jump_time = std::min(_jump_time + 1, (62*5) + 5 + 1);
		
		if (n == 0) {
			angles.x = -89.0f;
			ucmd->buttons |= IN_ATTACK2;
		} else if (n == (62*5) + 5) {
			angles.x = 89.0f;
			ucmd->buttons |= IN_ATTACK2|IN_JUMP;
		}
		
		ucmd->forward = angles.forward().normal();
	} else {
		_jump_time = 0;
	}
}

void c_hero::fire_time_think() {
	int hero = local->object.hero;
	switch (hero) {
		case HERO_ECHO: {
			g_offense->echo_fire_think();
			break;
		}
		case HERO_SOLDIER: {
			g_offense->soldier_fire_think();
			break;
		}
		case HERO_GENJI: {
			g_offense->genji_fire_think();
			break;
		}
		case HERO_MERCY: {
			g_healer->mercy_fire_think();
			break;
		}
		case HERO_SIGMA: {
			this->sigma_fire_think();
			break;
		}
	}
}

void c_hero::sigma_fire_think() {
  pseudo_cmd* ucmd = (pseudo_cmd*)&(g_ow->_cmd);
  
  if (ucmd->buttons & IN_ATTACK && g_state->is_fire_ready()) {
    if (g_state->_curtime >= _sigma_next_fire + 1.5f) {
      _sigma_next_attack = g_state->_curtime;
      _sigma_next_fire = g_state->_curtime;
      
      _sigma_attack_interval = 1;
    }
  }
  
  if (_sigma_attack_interval == 1 && g_state->_curtime >= _sigma_next_fire + 0.2f) {
    _sigma_next_attack = g_state->_curtime;
    _sigma_attack_interval = 0;
  }
  
  /*if (ucmd->buttons & IN_ATTACK && g_state->is_fire_ready()) {
    if (_sigma_attack_interval == 0 && g_state->_curtime >= _sigma_next_attack + 1.3f) {
      _sigma_next_fire = g_state->_curtime;
      _sigma_attack_interval = 1;
    }
  }
  
  if (_sigma_attack_interval == 1 && g_state->_curtime >= _sigma_next_fire + 0.3f) {
    //_sigma_next_attack = g_state->_curtime;
    _sigma_attack_interval = 0;
  }*/
}