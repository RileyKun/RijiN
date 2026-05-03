#include "link.h"

c_local local_instance;
c_local* g_local = &local_instance;

ow_local* local = &(local_instance._local_data);

bool c_local::update() {
	ow_object* obj = &(local->object);
	
	obj->index = -1;
	
	object_pool* pool = (object_pool*)obj->entity_parent;
	obj->pool_type = pool->get_type();
	
	if (!g_state->update_player(obj))
		return false;
	
	local->ray_cmp = this->lookup_component(0x2);
	
	cmp_input* c = (cmp_input*)(g_ow->_singleton_input.component);
	pseudo_cmd* ucmd = (pseudo_cmd*)&(g_ow->_cmd);
	
	input_cmd* in = g_ow->_input_cmd;
	Vector forward = Vector(in->forward_x, in->forward_y, in->forward_z);
	
	local->last_viewforward = local->viewforward;
	
	local->vieworigin = obj->position_root; // obj->position has jitter issue
	local->viewforward = forward;
	local->viewangles = Vector(forward.pitch(), forward.yaw(), 0.0f);
	
	local->team = g_state->get_team(obj->filter_bits);
	
	int hero = local->object.hero;
	
	local->reloading = c->get_reload_limit() == 0x4;
	local->ability_locked = false;
	
	uint16_t limit = c->get_ability_limit();
	local->ability_limit = limit;
	
	switch (hero) {
		case HERO_SIGMA: {
			if (limit != 0x0 && limit != 0xd5 && limit != 0xd6) // sig shoot
				local->ability_locked = true;
			
			break;
		}
		case HERO_MCCREE: {
			if (limit != 0x0 && limit != 0x172) // fan
				local->ability_locked = true;
			
			break;
		}
		case HERO_ASHE: {
			if (limit != 0x0 && limit != 0x479 && limit != 0x45d && limit != 0x45e) // dynamite & ads
				local->ability_locked = true;
			
			break;
		}
		case HERO_ROADHOG: {
			if (limit != 0x0 && limit != 0x2b) // hook
				local->ability_locked = true;
			
			break;
		}
		case HERO_HANZO: {
			if (limit != 0x0 && limit != 0x5a1) // storm
				local->ability_locked = true;
			
			break;
		}
		case HERO_ANA: {
			if (limit != 0x0 && limit != 0x478) // sleep
				local->ability_locked = true;
			
			break;
		}
		case HERO_ZEN: {
			break;
		}
		default: {
			if (limit != 0x0)
				local->ability_locked = true;
			
			break;
		}
	}
	
	float view_height = g_hero_data[hero].view_height;
	if (view_height != 0.0f) {
		if (hero != HERO_DVA && hero != HERO_HAMMOND) {
			if (ucmd->buttons & IN_DUCK)
				view_height *= 0.7f;
		}
		
		if (obj->pool_type == POOLID_DVAPILOT)
			view_height = 1.25f;
		
		local->vieworigin.y = obj->position_root.y + view_height;
	} else {
		local->vieworigin.y = obj->position.y;
	}
	
	Vector local_delta = Vector(0.0f, 0.0f, 0.0f);
	g_anim->calculate_delta(obj, 1, 0.0f, &local_delta);
	// 1 for usercmd function delay
	
	//local->vieworigin += local_delta;
	
	char teststr[128];
	
	cmp_skill* skill = (cmp_skill*)(obj->skill);
	
	{
		// 0x1f6 = shield active?
		
		// 0x254
		//0x3950
	/*uint64_t sk = g_state->get_skill_struct(obj->skill, 0xA, 0xAD); // 0x108 uint16_t
		if (sk) {
		char teststr[128];
	I(wsprintfA)(teststr, "local %p", sk);//(int)(*(float*)(sk + 0x60) * 1000.0f));
	g_console->print(teststr, 1.0f);
	}*/
	}
	
	if (hero != HERO_SOLDIER && hero != HERO_ECHO && hero != HERO_GENJI && hero != HERO_MERCY && hero != HERO_SIGMA) {
		local->fire_time = 0.0f;
		
		ow_condition* cond = g_state->lookup_condition(obj, HASH("fire_time"));
		if (cond)
			local->fire_time = std::max(0.0f, cond->data.fl);
		
		if (hero == HERO_BAPTISTE) {
  		if (local->fire_time > 0.0f && local->fire_time <= 0.23f) {
        local->bap_burst = true;
        
        float t = 0.2f;
        float t2 = t + OW_TICK_INTERVAL;
        
        if (local->fire_time > t2)
          local->fire_time -= t;
        
        if (local->fire_time > t2)
          local->fire_time -= t;
      }
		}
		
		// all data is 1 tick off, as proven by the fire rate
		// so we need to adjust all localplayer data to account for this delay
	//	if (local->fire_time != 0.0f)
		//	local->fire_time += OW_TICK_INTERVAL;
	} else {
		float curtime = (float)(g_ow->_game_tickcount + 1) * OW_TICK_INTERVAL;
		
		switch (hero) {
			case HERO_ECHO: {
				local->fire_time = curtime - g_offense->_echo_next_attack;
				break;
			}
			case HERO_SOLDIER: {
				local->fire_time = curtime - g_offense->_soldier_next_attack;
				break;
			}
			case HERO_GENJI: {
				local->fire_time = curtime - g_offense->_genji_next_secondary;
				break;
			}
			case HERO_MERCY: {
				local->fire_time = curtime - g_healer->_mercy_next_attack;
				break;
			}
			case HERO_SIGMA: {
				local->fire_time = curtime - g_hero->_sigma_next_attack;
				break;
			}
		}
		
		local->fire_time = std::max(0.0f, local->fire_time);
	}
	
	{
		local->sniper_charge = _last_sniper_charge;
		
		ow_condition* cond = g_state->lookup_condition(obj, HASH("sniper_charge"));
		if (cond)
			_last_sniper_charge = cond->data.fl;
		else
			_last_sniper_charge = 0.0f;
	}
	
	local->ammo_percent = 1.0f;
	
	if (hero == HERO_ASHE || hero == HERO_WIDOW || hero == HERO_ANA)
		skill->get_skill_float(0x6d, &(local->ads));
	
	switch (hero) {
		case HERO_MCCREE: {
			skill->get_skill_float(0x375, &(local->move_speed_mult));
			break;
		}
		case HERO_DVA: {
			skill->get_skill_float(0x375, &(local->move_speed_mult));
			skill->get_skill_float(0x48, &(local->fire_time));
			break;
		}
		case HERO_ECHO: {
			skill->get_skill_float(0x375, &(local->move_speed_mult));
			break;
		}
		case HERO_MEI: {
			skill->get_skill_float(0xf8, &(local->fire_time));
			break;
		}
		case HERO_DOOM: {
			skill->get_skill_float(0x49, &(local->fire_time));
			skill->get_skill_float(0x614, &(local->rocket_punch));
			skill->get_skill_float(0x375, &(local->move_speed_mult));
			break;
		}
		case HERO_TORB: {
			skill->get_skill_float(0xf7, &(local->fire_time));
			break;
		}
		case HERO_ZEN: {
			skill->get_skill_float(0xc9, &(local->zen_orbs));
			skill->get_skill_float(0x375, &(local->move_speed_mult));
			break;
		}
		case HERO_HAMMOND: {
			skill->get_skill_float(0x285, &(local->move_speed_mult));
			
			break;
		}
		case HERO_REINHARDT: {
			skill->get_skill_float(0x375, &(local->move_speed_mult));
			
			break;
		}
		case HERO_MERCY: {
			skill->get_skill_float(0xc7, &(local->healing_beam));
			skill->get_skill_float(0x5e2, &(local->cur_weapon));
			
			uint32_t target = 0;
			
			uint64_t sk1 = *(uint64_t*)(obj->skill + 0x2C0);
			if (sk1 != 0) {
				uint64_t sk2 = *(uint64_t*)(sk1 + 0x100);
				if (sk2 != 0) {
					for (int i = 0; i < 0x7; ++i) {
						if (*(uint16_t*)(sk2 + i * 0x10) == 0x217) {
				     	  		target = *(uint32_t*)(*(uint64_t*)((sk2 + i * 0x10) + 0x8) + 0x60); // Target linkid
								break;
						}
					}
				}
			}
			
			local->mercy_target = target;
			
			break;
		}
		case HERO_ROADHOG: {
			skill->get_skill_float(0xf7, &(local->fire_time));
			skill->get_skill_float(0x375, &(local->move_speed_mult));
			skill->get_skill_float(0x285, &(local->hog_hook));
			
			if (local->hog_hook == 2.0f)
				g_local->_hook_attach_time = g_state->_curtime;
			
			uint64_t hook = g_state->get_skill_struct(obj->skill, 0x3, 0x1b0);
			if (hook != 0) {
				local->hog_hook_pos.x = *(float*)(hook + 0x60);
				local->hog_hook_pos.y = *(float*)(hook + 0x64);
				local->hog_hook_pos.z = *(float*)(hook + 0x68);
			}
			
			break;
		}
		case HERO_PHARAH:
		case HERO_ANA: {
			skill->get_skill_float(0x375, &(local->move_speed_mult));
			skill->get_skill_float(0x4a, &(local->ammo_percent));
			
			break;
		}
		case HERO_GENJI: {
			skill->get_skill_float(0x4a, &(local->ammo_percent));
			
			float fire_time = 0.0f;
			skill->get_skill_float(0xf7, &fire_time);
			
			local->fire_time = std::min(fire_time, local->fire_time);
			local->genji_primary = fire_time;
			
			break;
		}
		case HERO_ASHE:
		case HERO_TRACER:
		case HERO_SOLDIER: {
			skill->get_skill_float(0x375, &(local->move_speed_mult));
			skill->get_skill_float(0x4a, &(local->ammo_percent));
			local->ammo_percent = 1.0f - local->ammo_percent;
			
			break;
		}
		case HERO_LUCIO: {
			skill->get_skill_float(0x3d9, &(local->cross_fade));
			
			break;
		}
	}
	
	g_esp->_fov = 103.0f;
	
	if (hero == HERO_ASHE) {
		if (ucmd->buttons & IN_ATTACK2)
			g_esp->_fov = 66.0f;
	} else if (hero == HERO_ANA || hero == HERO_WIDOW) {
		if (local->ads == 1.0f)
			g_esp->_fov = 51.0f;
	} else if (hero == HERO_HAMMOND) {
	}
	
	local->firing = false;
	local->fire_ready = false;
	local->projectile_speed = 0.0f;
	local->melee = false;
	local->melee_range = 2.5f;
	local->shotgun = local->object.hero == HERO_REAPER || local->object.hero == HERO_ROADHOG || local->object.hero == HERO_DVA || local->object.hero == HERO_DOOM;
	local->no_headshot = false;
	
	switch (hero) {
		case HERO_REINHARDT: {
			if (ucmd->buttons & IN_ATTACK)
				local->melee = true;
			
			local->melee_range = 5.0f;
			
			break;
		}
		case HERO_WINSTON: {
			if (ucmd->buttons & IN_ATTACK)
				local->melee = true;
			
			local->melee_range = 8.0f;
			
			break;
		}
		case HERO_BRIG: {
			if (ucmd->buttons & IN_ATTACK || ucmd->buttons & IN_MELEE)
				local->melee = true;
			
			local->melee_range = 6.0f;
			
			break;
		}
		default: {
			if (ucmd->buttons & IN_MELEE)
				local->melee = true;
			
			break;
		}
	}
	
	{
		int hero = local->object.hero;
		
		float attack_recovery = g_hero_data[hero].attack_recovery;
		if (hero == HERO_HANZO && local->ability_limit == 0x5a1)
			attack_recovery = 0.25f;
		else if (hero == HERO_ASHE && local->ads != 1.0f)
			attack_recovery = g_hero_data[hero].attack2_recovery;
		
		if (hero == HERO_SOLDIER)
			attack_recovery = 0.0f;
		else if (hero == HERO_MERCY)
			attack_recovery = 0.06f;
		
		if (hero == HERO_SIGMA) {
			if (g_hero->_sigma_attack_interval == 0)
				attack_recovery = 1.3f;
		}
    
		if (hero == HERO_BAPTISTE) {
	      if (local->bap_burst)
	        attack_recovery = 0.1f;
	    }
    
		if (g_state->is_fire_ready()) {
			if (local->fire_time != 0.0f) {
				if (local->fire_time >= attack_recovery - OW_TICK_INTERVAL)
					local->fire_ready = true;
			} else {
				local->fire_ready = true;
			}
		}
		
		if (g_hero_data[hero].attack_speed > 0.0f)
			local->projectile_speed = g_hero_data[hero].attack_speed;
		
		switch (hero) {
			case HERO_DVA: {
				bool dva_mech = obj->pool_type == POOLID_DVAMECH;
				
				if (dva_mech == false)
					local->projectile_speed = 50.0f;
				
				break;
			}
			case HERO_MEI: {
				if ((ucmd->buttons & IN_ATTACK) == 0)
					local->projectile_speed = g_hero_data[hero].attack2_speed;
				
				break;
			}
			case HERO_PHARAH:
			case HERO_ZARYA: {
				local->no_headshot = true;
				break;
			}
			case HERO_SIGMA: {
				bool throwing = false;
				
				ow_condition* rock = g_state->lookup_condition(&(local->object), HASH("usage_e"));
				if (rock) {
					if (rock->data.u32 & 1)
						throwing = true;
				}
				
				if (throwing)//ucmd->buttons & IN_ABILITY2)
					local->projectile_speed = g_hero_data[hero].ability2_speed;
				
				local->no_headshot = true;
				
				break;
			}
			case HERO_DOOM: {
				ow_condition* shift = g_state->lookup_condition(&(local->object), HASH("usage_shift"));
				if (shift) {
					if (shift->data.u32 & 1) {
						local->melee = true;
						local->melee_range = 5.0f;
					}
				}
				
				if (ucmd->buttons & IN_ABILITY2) {
					local->melee = true;
					local->melee_range = 8.0f;
				}
				
				if (ucmd->buttons & IN_ATTACK2) {
					local->melee = true;
					local->melee_range = 8.0f;
				}
				
				break;
			}
			case HERO_SOLDIER: {
				if (ucmd->buttons & IN_ATTACK2) {
					local->projectile_speed = g_hero_data[hero].attack2_speed;
					local->no_headshot = true;
				}
				
				break;
			}
			case HERO_MCCREE: {
				if (local->ability_limit == 0x14C)
					local->projectile_speed = g_hero_data[hero].ability2_speed;
				
				break;
			}
			case HERO_ANA: {
				if ((ucmd->buttons & IN_ATTACK2) == 0)
					local->projectile_speed = g_hero_data[hero].attack2_speed;
				
				if (local->ability_limit == 0x478)
					local->projectile_speed = g_hero_data[hero].ability1_speed;
				
				break;
			}
			case HERO_TORB: {
				if ((ucmd->buttons & IN_ATTACK2) != 0)
					local->projectile_speed = g_hero_data[hero].attack2_speed;
				
				break;
			}
			case HERO_SYM: {
				if ((ucmd->buttons & IN_ATTACK2) != 0)
					local->projectile_speed = g_hero_data[hero].attack2_speed;
				
				break;
			}
			case HERO_ECHO: {
				if (local->ability_limit == 0x6fc)
					local->projectile_speed = 0.0f;
				else if (local->ability_limit == 0x1e)
					local->projectile_speed = 50.0f;
				
				if (local->ability_limit == 0x6fc || local->ability_limit == 0x1e)
					local->no_headshot = true;
				
				break;
			}
			case HERO_GENJI: {
				ow_condition* shift = g_state->lookup_condition(&(local->object), HASH("usage_shift"));
				if (shift) {
					if (shift->data.u32 & 1) {
						local->melee = true;
						local->melee_range = 15.0f;
					}
				}
				
				bool deflecting = false;
				
				ow_condition* deflect = g_state->lookup_condition(&(local->object), HASH("usage_e"));
				if (deflect) {
					if (deflect->data.u32 & 1)
						deflecting = true;
				}
				
				if (deflecting)
					local->projectile_speed = 0.0f;
				
				break;
			}
			case HERO_TRACER: {
				bool ult = false;
				if (ucmd->buttons & IN_ABILITY3) {
					if (local->ability_limit == 0x16)
						ult = true;
					
					if (ucmd->buttons & IN_USE)
						ult = true;
				}
				
				if (ult) {
					local->projectile_speed = 15.0f;
					local->no_headshot = true;
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
				
				bool throw_hook = false;
				
				if ((ucmd->buttons & IN_ABILITY1) && (cooldown == 0.0f || cooldown > 7.5f))
					throw_hook = true;
				
				if (local->hog_hook == 1.0f && hook_time < 0.3f)
					throw_hook = true;
				
				local->throwing_hook = throw_hook;
        
				if (throw_hook)
					local->projectile_speed = 40.0f;
					
				break;
			}
		}
	}
	
	return true;
}

uint64_t c_local::lookup_component(uint64_t id) {
	uint64_t localplayer_pool = g_ow->_gm->localplayer_pool;
	if (localplayer_pool == 0)
		return 0;
	
	uint64_t pool_entry = *(uint64_t*)(localplayer_pool + ((id & 0x7F) * 0x10));
	if (pool_entry == 0)
		return 0;
	
	return *(uint64_t*)(pool_entry);
}

void c_local::post_think() {
	pseudo_cmd* ucmd = (pseudo_cmd*)&(g_ow->_cmd);
	
	if (local->fire_ready) {
		if (local->object.hero == HERO_HANZO && local->ability_limit != 0x5a1) {
			if (!(ucmd->buttons & IN_ATTACK) && g_ow->_last_buttons & IN_ATTACK)
				local->firing = true;
		} else {
			if (ucmd->buttons & IN_ATTACK)
				local->firing = true;
		}
	}
	
	if (local->firing == false) {
  		if (g_aim->_data.type == AIM_TYPE_RAGE)
			ucmd->forward = local->viewforward;
	}
}