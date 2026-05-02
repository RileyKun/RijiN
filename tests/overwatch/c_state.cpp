#include "link.h"

c_state state_instance;
c_state* g_state = &state_instance;

void c_state::update_objects() {
	if (_object_count == 0)
		return;
	
	for (int i = 0; i < _object_count; ++i) {
		ow_object* obj = &(_objects[i]);
		if (obj->entity_parent == 0)
			continue;
		
		if (obj->player_parent != 0) {
			if (!this->update_player(obj))
				obj->type = OBJECT_INVALID;
		} else {
			this->set_object_type(obj);
			if (!this->update_entity(obj))
				obj->type = OBJECT_INVALID;
		}
	}
}

bool c_state::update_entity(ow_object* obj) {
	cmp_health* hp = (cmp_health*)(g_ow->lookup_component(obj->entity_parent, CMPID_HEALTH));
	cmp_lerp* lerp = (cmp_lerp*)(g_ow->lookup_component(obj->entity_parent, CMPID_LERP));
	cmp_filter_bits* filter = (cmp_filter_bits*)(g_ow->lookup_component(obj->entity_parent, CMPID_FILTER_BITS));
	
	if (hp == 0 || filter == 0 || lerp == 0)
		return false;
	
	obj->health = hp->get_health();
	obj->max_health = hp->get_max_health();
	
	obj->armor = hp->get_armor();
	obj->max_armor = hp->get_max_armor();
	
	obj->shields = hp->get_shields();
	obj->max_shields = hp->get_max_shields();
	
	obj->total_hp = obj->health + obj->armor + obj->shields;
	obj->total_max_hp = obj->max_health + obj->max_armor + obj->max_shields;
	
	int index = lerp->get_index();
	
	int last_index = index - 1;
	if (last_index < 0)
		last_index += 20;
	
	Vector lerp_pos = *lerp->get_position_at_index(index);
	Vector lerp_pos_last = *lerp->get_position_at_index(last_index);
	
	Vector velocity = (lerp_pos - lerp_pos_last) / OW_TICK_INTERVAL;
	
	obj->position = lerp_pos;
	obj->velocity = velocity;
	
	obj->filter_bits = filter->get_bits();
	
	obj->is_visible = false;
	obj->on_ground = fabsf(obj->velocity.y) > 0.1f;
	
	return true;
}

void c_state::store_bones(ow_object* obj, cmp_velocity* vel) {
	if (obj->hero == HERO_UNDEFINED)
		return;
	
	if (obj->hero == HERO_TRAININGBOT1 || obj->hero == HERO_TRAININGBOT4)
		return;
	
	bool dva_mech = obj->pool_type == POOLID_DVAMECH;
	
	{
		switch (obj->hero) {
			case HERO_DVA: {
				if (dva_mech)
					obj->hitbox_map[HITBOX_HEAD] = 1337;//vel->get_skeleton_index(g_cvars_misc->test.value);
				else
					obj->hitbox_map[HITBOX_HEAD] = vel->get_skeleton_index(17);
				
				break;
			}
			case HERO_HAMMOND: {
				obj->hitbox_map[HITBOX_HEAD] = vel->get_skeleton_index(5);
				break;
			}
			default: {
				obj->hitbox_map[HITBOX_HEAD] = vel->get_skeleton_index(17);
				break;
			}
		}
		
		switch (obj->hero) {
			case HERO_DVA: {
				if (dva_mech)
					obj->hitbox_map[HITBOX_CHEST] = vel->get_skeleton_index(3);
				else
					obj->hitbox_map[HITBOX_CHEST] = vel->get_skeleton_index(5);
				
				break;
			}
			case HERO_REINHARDT:
			case HERO_BASTION: {
				obj->hitbox_map[HITBOX_CHEST] = vel->get_skeleton_index(4);
				break;
			}
			case HERO_HAMMOND: {
				obj->hitbox_map[HITBOX_CHEST] = vel->get_skeleton_index(4969);
				break;
			}
			default: {
				obj->hitbox_map[HITBOX_CHEST] = vel->get_skeleton_index(5);
				break;
			}
		}
	}
	
	hitbox_context_t* ctx = vel->get_hitbox_context();
	if (ctx) {
		if (ctx->sphere_data != 0) {
			for (int i = 0; i < ctx->sphere_count; ++i) {
				if (obj->hitbox_count >= ENGINE_HITBOX_MAX)
					break;
				
				sphere_t* dat = (sphere_t*)((uint64_t)(ctx->sphere_data) + (i * 0x40));
			//	if (dat->unknown != 0.0f)
			//	//	continue;
				
				if (dva_mech) {
					if (vel->get_skeleton_index(3) == (int)(dat->skel_index))
						continue;
				}
				
				bone_t* bone = vel->get_bone_by_index((int)(dat->skel_index));
				Vector offset = bone->position;
				
				int index = obj->hitbox_count++;
				
				obj->hitbox_id[index] = dat->skel_index;
				obj->hitbox_offset[index] = offset;
				obj->hitbox_radius[index] = dat->radius;
			}
		}
		
		if (ctx->capsule_data != 0) {
			for (int i = 0; i < ctx->capsule_count; ++i) {
				if (obj->hitbox_count >= ENGINE_HITBOX_MAX)
					break;
				
				capsule_t* dat = (capsule_t*)((uint64_t)(ctx->capsule_data) + (i * 0x50));
			//	if (dat->hitbox_type == 0xFFFF)
				//	continue;
				
				bone_t* bone = vel->get_bone_by_index((int)(dat->skel_index));
				Vector offset = bone->position;
				
				int index = obj->hitbox_count++;
				
				obj->hitbox_id[index] = dat->skel_index;
				obj->hitbox_offset[index] = offset;
				obj->hitbox_radius[index] = dat->radius;
				
				if (obj->hero == HERO_WIDOW) {
					if (obj->hitbox_id[i] == obj->hitbox_map[HITBOX_HEAD])
						obj->hitbox_offset[index] += Vector(-0.033f, 0.08f, 0.055f);
				}
			}
		}
		
		if (dva_mech && obj->hitbox_count < ENGINE_HITBOX_MAX) {
			int skel = vel->get_skeleton_index(11);
			bone_t* bone = vel->get_bone_by_index(skel);
			if (bone) {
				int index = obj->hitbox_count++;
				
				obj->hitbox_id[index] = 1337;
				obj->hitbox_offset[index] = bone->position;
				obj->hitbox_radius[index] = 0.26f;
			}
		}
		
	//	if (obj->hitbox_count == ENGINE_HITBOX_MAX)
//			g_console->print("hitbox max reached", 1.0f);
	}
	
	{
		bool head = false;
		bool chest = false;
		
		for (int i = 0; i < obj->hitbox_count; ++i) {
			if (obj->hitbox_id[i] == obj->hitbox_map[HITBOX_HEAD])
				head = true;
			
			if (obj->hitbox_id[i] == obj->hitbox_map[HITBOX_CHEST])
				chest = true;
		}
		
		if (head == false) {
		//	g_console->print("head failure", 1.0f);
		}
		
		if (chest == false) {
	///		g_console->print("chest failure", 1.0f);
		}
	}
}

bool c_state::update_player(ow_object* obj) {
	cmp_health* hp = (cmp_health*)(g_ow->lookup_component(obj->entity_parent, CMPID_HEALTH));
	cmp_velocity* vel = (cmp_velocity*)(g_ow->lookup_component(obj->entity_parent, CMPID_VELOCITY));
	cmp_filter_bits* filter = (cmp_filter_bits*)(g_ow->lookup_component(obj->entity_parent, CMPID_FILTER_BITS));
	
	cmp_rotation* rotation = (cmp_rotation*)(g_ow->lookup_component(obj->entity_parent, CMPID_ROTATION));
	cmp_hero_id* hero = (cmp_hero_id*)(g_ow->lookup_component(obj->player_parent, CMPID_HERO_ID));
	cmp_skill* skill = (cmp_skill*)(g_ow->lookup_component(obj->entity_parent, CMPID_SKILL));
	
	void* test = (void*)(g_ow->lookup_component(obj->entity_parent, 0x31));
	
	if (test == 0)
		return false;
	
	if (hp == 0 || vel == 0 || filter == 0)
		return false;
	
	if (rotation == 0 || hero == 0 || skill == 0)
		return false;
	// entity data
	
	obj->health = hp->get_health();
	obj->max_health = hp->get_max_health();
	
	obj->armor = hp->get_armor();
	obj->max_armor = hp->get_max_armor();
	
	obj->shields = hp->get_shields();
	obj->max_shields = hp->get_max_shields();
	
	obj->total_hp = obj->health + obj->armor + obj->shields;
	obj->total_max_hp = obj->max_health + obj->max_armor + obj->max_shields;
	
	Vector min = *vel->get_eyepos_min();
	Vector max = *vel->get_eyepos_max();
	obj->position = max;
	obj->velocity = *vel->get_velocity();
	
	obj->filter_bits = filter->get_bits();
	
	obj->is_visible = false;
	obj->on_ground = true;
	
	obj->is_bubbled = hp->bubble_state() != 0 ? true : false;
	
	// player data
	
	obj->type = OBJECT_PLAYER;
	
	obj->hero_id = hero->get_hero_id();
	
	int hero_index = g_state->get_hero(obj->hero_id);
	obj->hero = hero_index;
	
	float angle = rotation->get_rotation().y;
	if (angle != angle)
		angle = 0.0f;
	
	if (angle > PI*2.0f)
		angle = PI*2.0f;
	
	if (angle < PI*-2.0f)
		angle = PI*-2.0f;
	
	obj->rotation.y = angle;
	
	Vector forward = *(Vector*)((uint64_t)(test) + 0xd0);
	
	if (forward.x > -1.0f && forward.x < 1.0f
		&& forward.y > -1.0f && forward.y < 1.0f
		&& forward.z > -1.0f && forward.z < 1.0f)
		obj->forward = forward;
	
	//0x38
	//0x6B8
	//reinterpret_cast<void(__fastcall*)(uint64_t, uint64_t)>(g_ow->_base_address + 0xBDB7A0)((uint64_t)(rotation), NULL);
	//reinterpret_cast<void(__fastcall*)(uint64_t, uint64_t)>(g_ow->_base_address + 0xBDB7A0)((uint64_t)(rotation), vel->get_bone_struct());
	
	this->get_root_pos((uint64_t)vel, &(obj->position_root)); // equivalent to model min
	//0x2CC16C0
/*	uint64_t test = *(uint64_t*)(g_ow->_base_address + 0x2CAFFF0);
	test = *(uint64_t*)(test + 0x38);
	test = *(uint64_t*)(test + 0x8);
	test = *(uint64_t*)(test + 0x8);
	test = *(uint64_t*)(test + 0x8);
	//0x1564 = recoil
	//NtUserInjectMouseInput
	char err[128];
	I(wsprintfA)(err, "test %p ", test);
	g_console->error(err, 4.0f);*/
	
	/*if (obj->entity_parent == local->object.entity_parent) {
	char err[128];
	I(wsprintfA)(err, "test %p ", test);
	g_console->error(err, 4.0f);
}*/
	
	// lagged?
	obj->position_root.x = obj->position.x;
	obj->position_root.z = obj->position.z;
	
	obj->position_root_raw = obj->position_root;
	
	if (obj->entity_parent != local->object.entity_parent) {
		// fix timing to better represent fps (client vieworigin pred needs this too)
		// update: confirmed by spectate to not work
		int ticks = 1;
		Vector delta = {};
		
		g_anim->calculate_delta(obj, ticks, 0.0f, &delta);
		
		//obj->position += delta;
	//	obj->position_root += delta;
		
		if (vel->get_bone_struct() != 0)
			this->store_bones(obj, vel);
	} else {
		//char teststr[128];
	//I(wsprintfA)(teststr, "black %p %p", hp, obj->filter_bits);//*(uint32_t*)((uint64_t)(hp) + 0x30));
	//g_console->print(teststr, 1.0f);
	}
	
	obj->nameplate_y = *vel->get_nameplate_y();
	
	obj->model_min = obj->position_root;
	obj->model_max = obj->model_min + Vector(0.0f, obj->nameplate_y, 0.0f);
	obj->model_center = (obj->model_min + obj->model_max) * 0.5f;
	
	obj->position_address = (uint64_t)(vel) + 0x160;
	obj->skill = (uint64_t)skill;
	
	if (hero_index == HERO_ECHO) {
		int pool_hero = this->get_hero_from_pool(obj->pool_type);
		
		if (pool_hero != HERO_ECHO)
			obj->hero = pool_hero;
	}
	
	this->store_conditions(obj);
	
	float on_ground_float = 1.0f;
	skill->get_skill_float(0x8c, &(on_ground_float));
	
	if (on_ground_float == 0.0f)
		obj->on_ground = false;
	
	skill->get_ult_charge(&(obj->ult_charge));
	
	obj->pos = vel;
	
	/*if (hero_index == HERO_TRACER) {
	char teststr[128];
	I(wsprintfA)(teststr, "beast %p", obj->pos);
	g_console->print(teststr, 1.0f);
	*(uint8_t*)((uint64_t)(obj->pos) + 0x338) = 0xFF;
	*(uint32_t*)((uint64_t)(obj->pos) + 0x340) = 0xFFFFFFFF;
	}*/
	return true;
}

ow_condition* c_state::lookup_condition(ow_object* obj, uint32_t hash) {
	for (int i = 0; i < MAX_PLAYER_CONDS; ++i) {
		ow_condition* cond = &(obj->conditions[i]);
		if (cond->hash == hash)
			return cond;
	}
	
	return nullptr;
}

void c_state::add_condition(ow_object* obj, uint32_t hash, uint64_t data) {
	for (int i = 0; i < MAX_PLAYER_CONDS; ++i) {
		ow_condition* cond = &(obj->conditions[i]);
		if (cond->hash != 0x0)
			continue;
		
		cond->hash = hash;
		cond->data.u64 = data;
		break;
	}
}

#define SKILL_FIRE_TIME 0
#define SKILL_SNIPER_CHARGE 1

void c_state::store_conditions(ow_object* obj) {
	cmp_skill* skill = (cmp_skill*)(obj->skill);
	int hero = obj->hero;
	bool skill_valid[8] = {};
	uint32_t skill_value[8] = {};
	
	switch (hero) {
		case HERO_DOOM: {
			float move_speed = 0.0f;
			skill->get_skill_float(0x375, &move_speed);
			
			this->add_condition(obj, HASH("doom_invuln"), move_speed == 0.0f ? 1 : 0);
			
			break;
		}
		case HERO_ZEN: {	
			skill_valid[SKILL_FIRE_TIME] = skill->get_skill_u32(0x612, &(skill_value[SKILL_FIRE_TIME]));
			
			uint64_t ammo = this->get_skill_struct(obj->skill, 0x9, 0x35);
			
			if (ammo != 0)
				this->add_condition(obj, HASH("zen_ammo"), *(uint32_t*)(ammo + 0x50));
			
			uint32_t orb_u32 = 0;
			skill->get_skill_u32(0xc9, &orb_u32);
			
			this->add_condition(obj, HASH("zen_orb"), orb_u32);
			
			float move_speed = 0.0f;
			skill->get_skill_float(0x375, &move_speed);
			
			this->add_condition(obj, HASH("zen_invuln"), move_speed >= 2.0f ? 1 : 0);
			
			break;
		}
		case HERO_REAPER: {
			uint32_t shot1_u32 = 0;
			uint32_t shot2_u32 = 0;
			uint32_t value = 0;
			
			skill->get_skill_u32(0x48, &shot1_u32);
			skill->get_skill_u32(0x49, &shot2_u32);
			
			float shot1 = *(float*)(&shot1_u32);
			float shot2 = *(float*)(&shot2_u32);
			
			if (shot1 < shot2)
				value = shot1_u32;
			else
				value = shot2_u32;
			
			skill_valid[SKILL_FIRE_TIME] = true;
			skill_value[SKILL_FIRE_TIME] = value;
			
			break;
		}
		case HERO_BAPTISTE: {
			skill_valid[SKILL_FIRE_TIME] = skill->get_skill_u32(0xf7, &(skill_value[SKILL_FIRE_TIME]));
			
			break;
		}
		case HERO_PHARAH: {
			skill_valid[SKILL_FIRE_TIME] = skill->get_skill_u32(0x48, &(skill_value[SKILL_FIRE_TIME]));
			
			break;
		}
		case HERO_MCCREE: {
			skill_valid[SKILL_FIRE_TIME] = skill->get_skill_u32(0xf7, &(skill_value[SKILL_FIRE_TIME]));
			
			uint64_t ammo = this->get_skill_struct(obj->skill, 0x9, 0x35);
			
			if (ammo != 0)
				this->add_condition(obj, HASH("mccree_ammo"), *(uint32_t*)(ammo + 0x50));
			
			break;
		}
		case HERO_ANA:
		case HERO_ASHE: {
			skill_valid[SKILL_FIRE_TIME] = skill->get_skill_u32(0x48, &(skill_value[SKILL_FIRE_TIME]));
			break;
		}
		case HERO_WIDOW: {
			skill_valid[SKILL_FIRE_TIME] = skill->get_skill_u32(0x48, &(skill_value[SKILL_FIRE_TIME]));
			skill_valid[SKILL_SNIPER_CHARGE] = skill->get_skill_u32(0xa1, &(skill_value[SKILL_SNIPER_CHARGE]));
			
	//		if (local->sniper_charge != 0.0f)
			//	local->sniper_charge = std::min(1.0f, local->sniper_charge + (0.7625f * OW_TICK_INTERVAL));
			
			break;
		}
		/*case HERO_MERCY: { // doesn't work
			uint64_t cycle = this->get_skill_struct(obj->skill, 0xa, 0xad);
			
			if (cycle != 0) {
				float cycle_rate = *(float*)(cycle + 0x50);
				float cycle_remaining = *(float*)(cycle + 0x60);
				
				float value_float = cycle_rate - cycle_remaining;
				if (value_float == 0.0f)
					value_float = 1.0f;
				
				skill_valid[SKILL_FIRE_TIME] = true;
				skill_value[SKILL_FIRE_TIME] = *(uint32_t*)(&value_float);
			}
			
			break;
		}*/
		case HERO_GENJI: {
			uint64_t cycle = this->get_skill_struct(obj->skill, 0x9, 0xad);
			
			if (cycle != 0) {
				float cycle_rate = *(float*)(cycle + 0x50);
				float cycle_remaining = *(float*)(cycle + 0x60);
				
				float value_float = cycle_rate - cycle_remaining;
				if (value_float == 0.0f)
					value_float = 1.0f;
				
				skill_valid[SKILL_FIRE_TIME] = true;
				skill_value[SKILL_FIRE_TIME] = *(uint32_t*)(&value_float);
			}
			
			break;
		}
		case HERO_HANZO: {
			skill_valid[SKILL_FIRE_TIME] = skill->get_skill_u32(0x48, &(skill_value[SKILL_FIRE_TIME]));
			skill_valid[SKILL_SNIPER_CHARGE] = skill->get_skill_u32(0xc9, &(skill_value[SKILL_SNIPER_CHARGE]));
			
		//	if (local->sniper_charge != 0.0f)
			//	local->sniper_charge = std::min(1.0f, local->sniper_charge + (1.375f * OW_TICK_INTERVAL));
			
			break;
		}
		case HERO_ROADHOG: {
			skill_valid[SKILL_FIRE_TIME] = skill->get_skill_u32(0xf7, &(skill_value[SKILL_FIRE_TIME]));
			
			uint64_t ammo = this->get_skill_struct(obj->skill, 0x9, 0x35);
			
			if (ammo != 0)
				this->add_condition(obj, HASH("hog_ammo"), *(uint32_t*)(ammo + 0x50));
			
			uint64_t hook_time = this->get_skill_struct(obj->skill, 0x3, 0x1e0);
			
			if (hook_time != 0)
				this->add_condition(obj, HASH("hog_hook_time"), *(uint32_t*)(hook_time + 0x60));
			
			break;
		}
		case HERO_ZARYA: {
			skill_valid[SKILL_FIRE_TIME] = skill->get_skill_u32(0xf8, &(skill_value[SKILL_FIRE_TIME]));
			
			break;
		}
		case HERO_SOLDIER: {
			float helix = 0.0f;
			skill->get_skill_float(0xf8, &helix);
			
			this->add_condition(obj, HASH("usage_m2"), helix < 0.5f ? 1 : 0);
			
			break;
		}
		case HERO_SYM: {
			float m2 = 0.0f;
			skill->get_skill_float(0xf8, &m2);
			
			this->add_condition(obj, HASH("usage_m2"), m2 < 0.5f ? 1 : 0);
			
			break;
		}
		case HERO_ORISA: {
			float halt = 0.0f;
			skill->get_skill_float(0x615, &halt);
			
			this->add_condition(obj, HASH("usage_m2"), halt == 1.0f ? 1 : 0);
			
			break;
		}
		case HERO_DVA: {
			float matrix = 0.0f;
			skill->get_skill_float(0x148, &matrix);
			
			this->add_condition(obj, HASH("dva_matrix"), matrix == 1.0f ? 1 : 0);
			
			break;
		}
	}
	
	if (g_hero_data[hero].shift_id != 0) {
		uint32_t shift_id = g_hero_data[hero].shift_id;
		
		uint64_t shift_cd = this->get_skill_struct(obj->skill, shift_id, 0x3d);
		if (shift_cd == 0) {
			shift_cd = this->get_skill_struct(obj->skill, shift_id + (0x1b * 1), 0x3d);
			if (shift_cd == 0)
				shift_cd = this->get_skill_struct(obj->skill, shift_id + (0x1b * 2), 0x3d);
		}
		
		if (shift_cd != 0) {
			uint32_t time = *(uint32_t*)(shift_cd + 0x60);
			if (*(uint8_t*)(shift_cd + 0x48) == 0)
				time = 0;
			
			this->add_condition(obj, HASH("cooldown_shift"), time);
		}
		
		uint64_t shift_usage = 0;
		
		if (hero == HERO_JUNK) {
			shift_usage = this->get_skill_struct(obj->skill, shift_id, 0x3d7);
		} else {
			shift_usage = this->get_skill_struct(obj->skill, shift_id, 0x2bb);
			if (shift_usage == 0) {
				shift_usage = this->get_skill_struct(obj->skill, shift_id + (0x1b * 1), 0x2bb);
				if (shift_usage == 0)
					shift_usage = this->get_skill_struct(obj->skill, shift_id + (0x1b * 2), 0x2bb);
			}
		}
		
		if (shift_usage != 0)
			this->add_condition(obj, HASH("usage_shift"), *(uint8_t*)(shift_usage + 0x48));
	}
	
	if (g_hero_data[hero].e_id != 0) {
		uint32_t e_id = g_hero_data[hero].e_id;
		
		uint64_t e_cd = this->get_skill_struct(obj->skill, e_id, 0x3d);
		if (e_cd == 0) {
			e_cd = this->get_skill_struct(obj->skill, e_id + (0x1b * 1), 0x3d);
			if (e_cd == 0)
				e_cd = this->get_skill_struct(obj->skill, e_id + (0x1b * 2), 0x3d);
		}
		
		if (e_cd != 0) {
			uint32_t time = *(uint32_t*)(e_cd + 0x60);
			if (*(uint8_t*)(e_cd + 0x48) == 0)
				time = 0;
			
			this->add_condition(obj, HASH("cooldown_e"), time);
		}
		
		uint64_t e_usage = 0;
		
		if (hero == HERO_SIGMA) {
			e_usage = this->get_skill_struct(obj->skill, e_id, 0x286d);
		} else {
			e_usage = this->get_skill_struct(obj->skill, e_id, 0x2bb);
			if (e_usage == 0) {
				e_usage = this->get_skill_struct(obj->skill, e_id + (0x1b * 1), 0x2bb);
				if (e_usage == 0)
					e_usage = this->get_skill_struct(obj->skill, e_id + (0x1b * 2), 0x2bb);
			}
		}
		//this->get_skill_struct(obj->skill, e_id, 0x2bb); // find new, maybe 0x3b
		
		if (e_usage != 0)
			this->add_condition(obj, HASH("usage_e"), *(uint8_t*)(e_usage + 0x48));
	}
	
	if (g_hero_data[hero].m2_id != 0) {
		uint32_t m2_id = g_hero_data[hero].m2_id;
		
		uint64_t m2_cd = 0;
		
		if (hero == HERO_BRIG) {
			m2_cd = this->get_skill_struct(obj->skill, m2_id, 0x3306);
		} else {
			m2_cd = this->get_skill_struct(obj->skill, m2_id, 0x3d);
			
			if (m2_cd == 0) {
				m2_cd = this->get_skill_struct(obj->skill, m2_id + (0x1b * 1), 0x3d);
				if (m2_cd == 0)
					m2_cd = this->get_skill_struct(obj->skill, m2_id + (0x1b * 2), 0x3d);
			}
		}
		
		if (m2_cd != 0) {
			uint32_t time = *(uint32_t*)(m2_cd + 0x60);
			if (*(uint8_t*)(m2_cd + 0x48) == 0)
				time = 0;
			
			this->add_condition(obj, HASH("cooldown_m2"), time);
		}
		
		uint64_t m2_usage = 0;
		
		if (hero == HERO_BRIG) {
			m2_usage = this->get_skill_struct(obj->skill, m2_id, 0x254);
		} else {
			m2_usage = this->get_skill_struct(obj->skill, m2_id, 0x2bb);
			if (m2_usage == 0) {
				m2_usage = this->get_skill_struct(obj->skill, m2_id + (0x1b * 1), 0x2bb);
				if (m2_usage == 0)
					m2_usage = this->get_skill_struct(obj->skill, m2_id + (0x1b * 2), 0x2bb);
			}
		}
		
		if (hero != HERO_SOLDIER && m2_usage != 0)
			this->add_condition(obj, HASH("usage_m2"), *(uint8_t*)(m2_usage + 0x48));
	}
	
	if (skill_valid[SKILL_FIRE_TIME])
		this->add_condition(obj, HASH("fire_time"), skill_value[SKILL_FIRE_TIME]);
	
	if (skill_valid[SKILL_SNIPER_CHARGE])
		this->add_condition(obj, HASH("sniper_charge"), skill_value[SKILL_SNIPER_CHARGE]);
}

void c_state::update_aim_rays() {
	for (int i = 0; i < _object_count; ++i) {
		ow_object* obj = &(_objects[i]);
		if (obj->entity_parent == 0)
			continue;
		
		if (!g_aim->is_valid_target(obj))
			continue;
		
		Vector delta = {};
		
		{
			float ping = (float)(g_anim->_tick_limit_ms) / 1000.0f;
			ping = std::max(ping - 0.2f, 0.0f);
			
			int ticks = (int)(ping / 0.032f);
			float drop = 0.0f;
			
			Vector org = {};
			
			if (obj->type == OBJECT_PLAYER)
				org = obj->model_center;
			else
				org = obj->position;
			
			if (local->projectile_speed > 0.0f) {
				float pred_dist = (org - local->vieworigin).length();
				
				ticks += (int)(((pred_dist / local->projectile_speed) / OW_TICK_INTERVAL) + 0.5f);
				
				if (aim_config.position_enabled) {
					float time_limit = g_anim->_history_time_limit;
					
					float limit_ms = (float)(g_anim->_tick_limit_ms) / 1000.0f;
					float ms = OW_TICK_INTERVAL * (float)(ticks);
					if ((ms + limit_ms) > TIME_WINDOW_MS)
						ms = TIME_WINDOW_MS - limit_ms;
					
					ms = std::min(ms, time_limit);
					
					int backward = (int)((ms / OW_TICK_INTERVAL) + 0.5f);
					
				//	if (g_cvars->history_lowest.value != 0)
						ticks = std::max(0, ticks - backward);
					
					obj->predicted_backward = backward;
				}
				
				if (local->object.hero == HERO_HANZO || local->object.hero == HERO_ZARYA || local->object.hero == HERO_TORB) {
					float falloff = 3.0f;
					if (local->object.hero == HERO_HANZO)
						falloff = 10.0f;
					else if (local->object.hero == HERO_TORB)
						falloff = 7.0f;
					// zarya's right click projectile has different gravity
					
					float time = (pred_dist / local->projectile_speed);
					float gravity = 0.5f * falloff * time * time;
					
					drop = gravity;
				}
			}
			
			g_anim->calculate_delta(obj, ticks, 0.0f, &delta);
			
			delta.y += drop;
			
			if (fabsf(obj->velocity.y) > 0.1f) {
				Vector ray_start = {};
				Vector ray_end = {};
				
				if (obj->type == OBJECT_PLAYER)
					ray_start = obj->position_root;
				else
					ray_start = obj->position;
				
				ray_end = ray_start + delta;
				
				ray_setup_t ray_in = {};
				ray_result_t ray_out = {};
				bool result = this->engine_ray_aim(&ray_in, &ray_out, ray_start, ray_end);
				
				if (ray_out.fraction != 1.0f) {
					Vector hitpos = Vector(ray_out.hitpos_x, ray_out.hitpos_y, ray_out.hitpos_z);
					delta = hitpos - ray_start;
				}
			}
			
			obj->predicted_delta = delta;
		}
		
		bool bot = obj->hero == HERO_TRAININGBOT1 || obj->hero == HERO_TRAININGBOT4;
		
		if (bot) {
			Vector head = obj->position_root + Vector(0.0f, 1.96f, 0.0f);
			Vector chest = obj->position_root + Vector(0.0f, 1.3f, 0.0f);
			
			Vector ray_start = local->vieworigin;
			Vector ray_end = {};
			
			{
				ray_end = head;
				
				ray_setup_t ray_in = {};
				ray_result_t ray_out = {};
				bool result = this->engine_ray_aim(&ray_in, &ray_out, ray_start, ray_end);
				
				if (ray_out.fraction == 1.0f)
					obj->hitbox_visible[HITBOX_HEAD] = true;
			}
			
			{
				ray_end = chest;
				
				ray_setup_t ray_in = {};
				ray_result_t ray_out = {};
				bool result = this->engine_ray_aim(&ray_in, &ray_out, ray_start, ray_end);
				
				if (ray_out.fraction == 1.0f)
					obj->hitbox_visible[HITBOX_CHEST] = true;
			}
		} else if (obj->type == OBJECT_PLAYER) {
			matrix_t maty;
			OW_InitY(obj->rotation.y, maty);
			
			bool head = false;
			bool chest = false;
			
			bool head_done = false;
			bool chest_done = false;
			
			for (int n = 0; n < obj->hitbox_count; ++n) {
				bool head = obj->hitbox_id[n] == obj->hitbox_map[HITBOX_HEAD];
				bool chest = obj->hitbox_id[n] == obj->hitbox_map[HITBOX_CHEST];
				
				if (head == false && chest == false)
					continue;
				
				Vector ray_start = local->vieworigin;
				Vector ray_end = {};
				
				Vector offset = obj->hitbox_offset[n];	
				
				OW_Transform(offset, maty, &ray_end);
				ray_end += obj->position_root;
				
				ray_end += delta;
				
				ray_setup_t ray_in = {};
				ray_result_t ray_out = {};
				bool result = this->engine_ray_aim(&ray_in, &ray_out, ray_start, ray_end);
				
				if (ray_out.fraction == 1.0f) {
					if (head)
						obj->hitbox_visible[HITBOX_HEAD] = true;
					else
						obj->hitbox_visible[HITBOX_CHEST] = true;
				}
				
				if (head)
					head_done = true;
				
				if (chest)
					chest_done = true;
				
				if (head_done && chest_done)
					break;
			}
		}
		
		{
			Vector ray_start = local->vieworigin;
			Vector ray_end = obj->type == OBJECT_PLAYER ? obj->model_center: obj->position;
			ray_end += delta;
			
			ray_setup_t ray_in = {};
			ray_result_t ray_out = {};
			bool result = this->engine_ray_aim(&ray_in, &ray_out, ray_start, ray_end);
			
			if (ray_out.fraction == 1.0f)
				obj->is_visible = true;
		}
	}
}

char fuck[0x208];

// big note !!!
// rays seem to be queued by engine, probably to another thread
// that means they are x number of frames off, explaining why you always shoot 'after'
void c_state::update_trig_rays() {
	pseudo_ray_t* ray = &(local->view_ray);
	
	{
		Vector fwd = local->viewforward.normal();
		Vector ray_start = local->vieworigin;
		Vector ray_end = local->vieworigin + (fwd * 100.0f);
		
		ray_setup_t ray_in = {};
		ray_result_t ray_out = {};
		_rayray = true;
		
		bool result = this->engine_ray_trig(&ray_in, &ray_out, ray_start, ray_end);
		_rayray = false;
		
		memset(ray, 0, sizeof(pseudo_ray_t));
		
		if (ray_out.hit_flags == 0xFFFFFFFF && ray_out.hitbox == 0) {
			ray->fraction = ray_out.fraction;
			ray->hitbox = ray_out.hitbox;
			ray->head_hit = ray_out.head_var != 0;
		}
		
		memset(&ray_in, 0, sizeof(ray_in));
		memset(&ray_out, 0, sizeof(ray_out));
		
		result = this->engine_ray_aim(&ray_in, &ray_out, ray_start, ray_end);
		
		ray->test_fraction = ray_out.fraction;
		ray->test_length = (ray_end - ray_start).length() * ray->test_fraction;
		ray->test_hitpos = Vector(ray_out.hitpos_x, ray_out.hitpos_y, ray_out.hitpos_z);
		
		Vector hitpos = Vector(ray_out.hitpos_x, ray_out.hitpos_y, ray_out.hitpos_z);
		Vector pdir = (hitpos - ray_start).normal();
		Vector normal = Vector(ray_out.normal_x, ray_out.normal_y, ray_out.normal_z);
		
		float dot = pdir.dot(normal * -1.0f);
		
		Vector rdir = (normal * 2.0f * dot) + pdir;
		rdir = rdir.normal();
		
		memset(&ray_in, 0, sizeof(ray_in));
		memset(&ray_out, 0, sizeof(ray_out));
		
		ray_start = hitpos;
		ray_end = hitpos + (rdir * 100.0f);
		
		if (local->object.hero == HERO_HANZO) {
			result = this->engine_ray_aim(&ray_in, &ray_out, ray_start, ray_end);
			
			ray->reflect_start = ray_start;
			ray->reflect_end = Vector(ray_out.hitpos_x, ray_out.hitpos_y, ray_out.hitpos_z);
		}
	}
}

void c_state::set_object_type(ow_object* obj) {
	switch (obj->pool_type) {
		case POOLID_JUNK:
		case POOLID_TRANSLOCATOR:
		case POOLID_TPENTER:
		case POOLID_TPEXIT:
		case POOLID_WIDOWMINE:
		case POOLID_LAMP: {
			obj->type = OBJECT_EQUIPMENT;
			break;
		}
		case POOLID_DYNAMITE: {
			obj->type = OBJECT_DYNAMITE;
			break;
		}
	}
}

bool c_state::is_player(uint32_t filter_bits) {
	if ((filter_bits & FILTER_PLAYER) == 0)
		return false;
	
	return true;
}

bool c_state::is_alive(ow_object* obj) {
	if (obj->hero == HERO_DVA) {
		if (obj->health < 1.0f)
			return false;
	}
	
	if (obj->health > 0.0f)
		return true;
	
	return false;
}

bool c_state::is_object_destroyed(uint32_t filter_bits) {
	if ((filter_bits & FILTER_DESTROYED1) != 0)
		return true;
	
	if ((filter_bits & FILTER_DESTROYED2) != 0)
		return true;
	
	return false;
}

int c_state::get_team(uint32_t filter_bits) {
	if (filter_bits & FILTER_TEAM1)
		return 1;
	
	if (filter_bits & FILTER_TEAM2)
		return 2;
	
	if (filter_bits & FILTER_TEAMFFA)
		return 3;
	
	return 0;
}

static uint64_t s_hero_to_heroid[HERO_MAX] = {
	0,
	HEROID_DVA,
	HEROID_ORISA,
	HEROID_REINHARDT,
	HEROID_ROADHOG,
	HEROID_SIGMA,
	HEROID_WINSTON,
	HEROID_WRECKINGBALL,
	HEROID_ZARYA,
	HEROID_ASHE,
	HEROID_BASTION,
	HEROID_DOOMFIST,
	HEROID_ECHO,
	HEROID_GENJI,
	HEROID_HANZO,
	HEROID_JUNKRAT,
	HEROID_MCCREE,
	HEROID_MEI,
	HEROID_PHARAH,
	HEROID_REAPER,
	HEROID_SOLDIER76,
	HEROID_SOMBRA,
	HEROID_SYMMETRA,
	HEROID_TORBJORN,
	HEROID_TRACER,
	HEROID_WIDOWMAKER,
	HEROID_ANA,
	HEROID_BAPTISTE,
	HEROID_BRIGITTE,
	HEROID_LUCIO,
	HEROID_MERCY,
	HEROID_MOIRA,
	HEROID_ZENYATTA,
	HEROID_TRAININGBOT1,
	HEROID_TRAININGBOT2,
	HEROID_TRAININGBOT3,
	HEROID_TRAININGBOT4,
};

static uint64_t s_hero_to_poolid[HERO_MAX] = {
	0,
	POOLID_DVAMECH,
	POOLID_ORISA,
	POOLID_REINHARDT,
	POOLID_ROADHOG,
	POOLID_SIGMA,
	POOLID_MONKEY,
	POOLID_HAMSTER,
	POOLID_ZARYA,
	POOLID_ASHE,
	POOLID_BASTION,
	POOLID_DOOM,
	POOLID_ECHO,
	POOLID_GENJI,
	POOLID_HANZO,
	POOLID_JUNK,
	POOLID_MCCREE,
	POOLID_MEI,
	POOLID_PHARAH,
	POOLID_REAPER,
	POOLID_SOLDIER,
	POOLID_SOMBRA,
	POOLID_SYM,
	POOLID_TORB,
	POOLID_TRACER,
	POOLID_WIDOW,
	POOLID_ANA,
	POOLID_BAPTISTE,
	POOLID_BRIG,
	POOLID_LUCIO,
	POOLID_MERCY,
	POOLID_MOIRA,
	POOLID_ZEN,
	POOLID_TRAININGBOT1,
	POOLID_TRAININGBOT1,
	POOLID_TRAININGBOT1,
	POOLID_TRAININGBOT4,
};

int c_state::get_hero(uint64_t heroid) {
	for (int i = 0; i < HERO_MAX; ++i) {
		uint64_t id = s_hero_to_heroid[i];
		if (heroid == id)
			return i;
	}
	
	return 0;
}

int c_state::get_hero_from_pool(uint64_t poolid) {
	for (int i = 0; i < HERO_MAX; ++i) {
		uint64_t id = s_hero_to_poolid[i];
		if (poolid == id)
			return i;
	}
	
	return 0;
}

bool c_state::is_fire_ready() {
	if (local->object.hero == HERO_TRACER) {
		if (local->ability_limit == 0x16)
			return true;
	}
	
	if (local->object.hero == HERO_ECHO) {
		if (local->ability_limit == 0x6fc)
			return true;
		
		if (local->ability_limit == 0x1e)
			return true;
	}
	
	if (local->object.hero == HERO_ECHO) {
		if (local->ability_limit == 0xd5)
			return true;
		
		if (local->ability_limit == 0xd6)
			return true;
	}
	
	if (local->ability_limit == 0xD) // melee
		return false;
	
	pseudo_cmd* ucmd = (pseudo_cmd*)&(g_ow->_cmd);
	if (ucmd->buttons & IN_RELOAD)
		return false;
	
	if (local->reloading)
		return false;
	
	if (local->ability_locked)
		return false;
	
	if (local->ammo_percent == 0.0f)
		return false;
	
	if (local->object.hero == HERO_SOLDIER) {
		if (ucmd->buttons & IN_ABILITY1)
			return false;
		
	//	if (local->move_speed_mult >= 1.5f)
		//	return false;
	}
	
	if (local->object.hero == HERO_GENJI) {
		ow_condition* dash = g_state->lookup_condition(&(local->object), HASH("usage_shift"));
		if (dash) {
			if (dash->data.u32 & 1)
				return false;
		}
	}
	
	if (local->object.hero == HERO_MCCREE) {
		ow_condition* ammo = g_state->lookup_condition(&(local->object), HASH("mccree_ammo"));
		if (ammo) {
			if (ammo->data.u32 == 0)
				return false;
		}
		
		ow_condition* roll = g_state->lookup_condition(&(local->object), HASH("usage_shift"));
		if (roll) {
			if (roll->data.u32 & 1)
				return false;
		}
		
		if (local->move_speed_mult == 0.3f)
			return false; // ult in use
	}
	
	if (local->object.hero == HERO_ROADHOG) {
		ow_condition* ammo = g_state->lookup_condition(&(local->object), HASH("hog_ammo"));
		if (ammo) {
			if (ammo->data.u32 == 0)
				return false;
		}
	}
	
	return true;
}

bool c_state::is_invuln(ow_object* obj, bool melee) {
  if (obj->filter_bits & FILTER_INVULN)
    return true;
  
  bool team_mate = local->team == g_state->get_team(obj->filter_bits);
  
  bool looking = true;
  
  Vector dt = (local->vieworigin - obj->model_center).normal();
  if (dt.dot(obj->forward) < 0.0f)
    looking = false;
  
  switch (obj->hero) {
    case HERO_DOOM: {
      ow_condition* invuln = g_state->lookup_condition(obj, HASH("doom_invuln"));
      if (invuln) {
        if (invuln->data.u32 & 1)
          return true;
      }
      
      break;
    }
    case HERO_ZEN: {
      ow_condition* invuln = g_state->lookup_condition(obj, HASH("zen_invuln"));
      if (invuln) {
        if (invuln->data.u32 & 1)
          return true;
      }
      
      break;
    }
    case HERO_DVA: {
      ow_condition* matrix = g_state->lookup_condition(obj, HASH("dva_matrix"));
      if (matrix && team_mate == false && looking && melee == false) {
        if (matrix->data.u32 & 1)
          return true;
      }
      
      break;
    }
    case HERO_SIGMA: {
      ow_condition* shift = g_state->lookup_condition(obj, HASH("usage_shift"));
      if (shift && team_mate == false && looking && melee == false) {
        if (shift->data.u32 & 1)
          return true;
      }
      
      break;
    }
    case HERO_GENJI: {
      ow_condition* e = g_state->lookup_condition(obj, HASH("usage_e"));
      if (e && team_mate == false && (melee == false && looking == false) == false) {
        if (e->data.u32 & 1)
          return true;
      }
      
      break;
    }
    case HERO_TRACER: {
      ow_condition* e = g_state->lookup_condition(obj, HASH("usage_e"));
      if (e) {
        if (e->data.u32 & 1)
          return true;
      }
      
      break;
    }
    case HERO_REAPER: {
      ow_condition* shift = g_state->lookup_condition(obj, HASH("usage_shift"));
      if (shift && team_mate == false) {
        if (shift->data.u32 & 1)
          return true;
      }
      
      break;
    }
    case HERO_MOIRA:
    case HERO_MEI: {
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

/*
	mov     rdx, 591655AB3CA98E76h
	mov     rax, rcx
	mov     r11, 0ADF98327F7A1F73Bh
	and     eax, 7
	mov     r14, 8F6273A108D43032h
	mov     r13, 70C9F778D8DBB05Bh
	mov     r8, 81793807628196B0h
	mov     r9, 3F72D68D7B52C902h
	mov     r10, 0F9573C21148098E9h
	mov     r15, 94AA5F0412E32725h
*/

// 49 BE ? ? ? ? ? ? ? ? 49 BD ? ? ? ? ? ? ? ? 49 B8 ? ? ? ? ? ? ? ? 49 B9 ? ? ? ? ? ? ? ? 49 BA ? ? ? ? ? ? ? ? 49 BF ? ? ? ? ? ? ? ?
uint64_t vec_keys[8] = {
	0x94AA5F0412E32725, // r15
	0x0ADF98327F7A1F73B, // r11
	0x8F6273A108D43032, // r14
	0x70C9F778D8DBB05B, // r13
	0x591655AB3CA98E76, // rdx
	0x81793807628196B0, // r8
	0x3F72D68D7B52C902, // r9
	0x0F9573C21148098E9 // r10
};

uint64_t c_state::get_vec_key(uint64_t index) {
	uint64_t key_index = index & 7;
	if (key_index < 0 || key_index > 7)
		key_index = 0;
	
	return vec_keys[key_index];
}

// 0 & 1 rotation
// 4 & 5 rotation
void c_state::get_root_pos(uint64_t a1, Vector* out) {
	uint64_t v2 = a1 + 0x480;
	uint64_t v4 = *(uint64_t*)(a1 + 0x480 + 0x40);
	
	uint64_t buffer[8];
	
	uint64_t v5 = get_vec_key(v4 & 7);
	uint64_t v6 = v4 + v5;
	
	for (int i = 0; i < 8; ++i) {
		uint64_t decrypt = *(uint64_t*)(v2 + (i * 0x8)) ^ v6;
		buffer[i] = decrypt;
		
		v6 += get_vec_key(v6 & 7);
	}
	
	uint64_t p1 = buffer[6];
	uint64_t p2 = buffer[7];
	
	uint32_t x = p1 & 0xFFFFFFFF;
	uint32_t y = (p1 >> 32) & 0xFFFFFFFF;
	uint32_t z = p2 & 0xFFFFFFFF;
	
	*out = Vector(*(float*)(&x), *(float*)(&y), *(float*)(&z));
}

bool c_state::engine_ray_aim(ray_setup_t* setup, ray_result_t* result, Vector start, Vector end) {
	if (!this->setup_ray(setup, start, end, true))
		return false;
	
	this->ray_add_filter(setup, &(local->object.entity_parent));
	
	this->ray_trace(setup, result);
	
	return true;
}

bool c_state::engine_ray_trig(ray_setup_t* setup, ray_result_t* result, Vector start, Vector end) {
	if (!this->setup_ray(setup, start, end, false))
		return false;
	
	this->ray_add_filter(setup, &(local->object.entity_parent));
	
	for (int i = 0; i < g_state->_object_count; ++i) {
		if (local->team == 3)
			break;
		
		ow_object* obj = &(g_state->_objects[i]);
		
		if (local->team != g_state->get_team(obj->filter_bits))
			continue;
		
		uint64_t entity_parent = obj->entity_parent;
		this->ray_add_filter(setup, &entity_parent);
	}
	
	this->ray_trace(setup, result);
	
	return true;
}

bool c_state::setup_ray(ray_setup_t* setup, Vector start, Vector end, bool filter_players) {
	setup->start_x = start.x;
	setup->start_y = start.y;
	setup->start_z = start.z;
	
	setup->end_x = end.x;
	setup->end_y = end.y;
	setup->end_z = end.z;
	
	setup->type = filter_players ? 0x690000000000017 : 0x0690000000000002;
	
	setup->gm = (uint64_t)(g_ow->_gm);
	setup->ray_cmp = local->ray_cmp;
	
	uint64_t a1 = (uint64_t)setup;
	
	*(uint32_t*)(a1 + 0x20) = 0x3F800000;
	*(uint64_t*)(a1 + 0x60) = a1 + 0x70;
	*(uint32_t*)(a1 + 0x6C) = 0x80000004;
	*(uint64_t*)(a1 + 0x90) = a1 + 0xA0;
	*(uint32_t*)(a1 + 0x9C) = 0x80000004;
	*(uint64_t*)(a1 + 0xD0) = a1 + 224;
	*(uint32_t*)(a1 + 0xDC) = 0x80000008;
	*(uint32_t*)(a1 + 0x198) = 0x10000;
	
	return true;
}

bool c_state::ray_add_filter(ray_setup_t* setup, uint64_t* entity_parent) {
	uint64_t result = reinterpret_cast<uint64_t(*)(void*, void*)>(g_ow->_ray_add_filter)(&(setup->entity_list), entity_parent);
	return result == 0x0; // ret_result = entity count
}

bool c_state::ray_trace(ray_setup_t* setup, ray_result_t* trace_result) {
	uint64_t result = reinterpret_cast<uint64_t(*)(void*, void*, int)>(g_ow->_ray_trace)(setup, trace_result, 0);
	return result == 0x0;
}

uint64_t c_state::get_skill_struct(uint64_t skill, uint16_t id1, uint16_t id2) {
	unsigned __int64 m128_u64[2];
	m128_u64[1] = skill + 0xD0;
	
	uint16_t ids[20] = {id1, id2};
	uint64_t result = reinterpret_cast<uint64_t(__fastcall*)(__m128*, uint16_t*)>(g_ow->_skill_struct)((__m128*)m128_u64, ids);
	return result;
}