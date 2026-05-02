#include "../link.h"

ALWAYSINLINE c_entity_data* c_base_entity::entity_data(){
  return utils::get_entity_data(get_index());
}

ALWAYSINLINE c_base_player* c_base_entity::get_player(){
  if(!is_player())
    return nullptr;

  return (c_base_player*)this;
}

ALWAYSINLINE c_base_player* c_base_entity::get_building_owner(){
  return (c_base_entity*)global->entity_list->get_entity_handle(building_owner_handle());
}

// Returns 0 if not a spellbook, 1 = normal spellbook, 2 = rare spellbook
i32 c_base_entity::get_spellbook_type(){
  s_entity_info* info = entity_info->get_data(get_index());
  if(info == nullptr)
    return 0;

  switch(info->model_hash){
    default: break;
    case HASH("models/props_halloween/hwn_spellbook_upright.mdl"):
    case HASH("models/items/crystal_ball_pickup.mdl"):
    case HASH("models/props_monster_mash/flask_vial_green.mdl"):
      return 1;
    case HASH("models/props_halloween/hwn_spellbook_upright_major.mdl"):
    case HASH("models/items/crystal_ball_pickup_major.mdl"):
    case HASH("models/props_monster_mash/flask_vial_purple.mdl"):
      return 2;
  }

  return 0;
}

bool c_base_entity::is_bomb(){
  s_client_class* cc = get_client_class();
  if(cc == nullptr){
    assert(cc != nullptr);
    return false;
  }

  switch(cc->id){
    default: break;
    case CTFPumpkinBomb:
    case CTFGenericBomb:
      return true;
  }

  return false;
}

bool c_base_entity::is_health_pack(){
  s_entity_info* info = entity_info->get_data(get_index());
  if(info == nullptr)
    return false;

  switch(info->model_hash){
    default: break;
    case HASH("models/items/medkit_small.mdl"):
    case HASH("models/items/medkit_medium.mdl"):
    case HASH("models/items/medkit_large.mdl"):
    case HASH("models/items/medkit_small_bday.mdl"):
    case HASH("models/items/medkit_medium_bday.mdl"):
    case HASH("models/items/medkit_large_bday.mdl"):
    case HASH("models/props_halloween/halloween_medkit_small.mdl"):
    case HASH("models/props_halloween/halloween_medkit_medium.mdl"):
    case HASH("models/props_halloween/halloween_medkit_large.mdl"):
    case HASH("models/props_medieval/medieval_meat.mdl"):
      return true;
  }

  return false;
}

bool c_base_entity::is_ammo_pack(){
  s_entity_info* info = entity_info->get_data(get_index());
  if(info == nullptr)
    return false;

  switch(info->model_hash){
    case HASH("models/items/ammopack_small.mdl"):
    case HASH("models/items/ammopack_medium.mdl"):
    case HASH("models/items/ammopack_large.mdl"):
    case HASH("models/items/ammopack_small_bday.mdl"):
    case HASH("models/items/ammopack_medium_bday.mdl"):
    case HASH("models/items/ammopack_large_bday.mdl"):
      return true;
  }

  return info->class_id == CTFAmmoPack;
}

bool c_base_entity::is_powerup(){
  s_entity_info* info = entity_info->get_data(get_index());
  if(info == nullptr)
    return false;

  switch(info->model_hash){
    default: break;
    case HASH("models/pickups/pickup_powerup_haste.mdl"):
    case HASH("models/pickups/pickup_powerup_vampire.mdl"):
    case HASH("models/pickups/pickup_powerup_precision.mdl"):
    case HASH("models/pickups/pickup_powerup_regen.mdl"):
    case HASH("models/pickups/pickup_powerup_supernova.mdl"):
    case HASH("models/pickups/pickup_powerup_strength.mdl"):
    case HASH("models/pickups/pickup_powerup_knockout.mdl"):
    case HASH("models/pickups/pickup_powerup_defense.mdl"):
    case HASH("models/pickups/pickup_powerup_crit.mdl"):
    case HASH("models/pickups/pickup_powerup_agility.mdl"):
    case HASH("models/pickups/pickup_powerup_king.mdl"):
    case HASH("models/pickups/pickup_powerup_plague.mdl"):
    case HASH("models/pickups/pickup_powerup_reflect.mdl"):
      return true;
  }

  return false;
}

std::wstring c_base_entity::get_powerup_name(){
  s_entity_info* info = entity_info->get_data(get_index());
  if(info == nullptr)
    return L"";

  switch(info->model_hash){
    default: break;
    case HASH("models/pickups/pickup_powerup_haste.mdl"):
      return WXOR(L"Haste");
    case HASH("models/pickups/pickup_powerup_vampire.mdl"):
      return WXOR(L"Vampire");
    case HASH("models/pickups/pickup_powerup_precision.mdl"):
      return WXOR(L"Precision");
    case HASH("models/pickups/pickup_powerup_regen.mdl"):
      return WXOR(L"Regen");
    case HASH("models/pickups/pickup_powerup_supernova.mdl"):
      return WXOR(L"Supernova");
    case HASH("models/pickups/pickup_powerup_strength.mdl"):
      return WXOR(L"Strength");
    case HASH("models/pickups/pickup_powerup_knockout.mdl"):
      return WXOR(L"Knockout");
    case HASH("models/pickups/pickup_powerup_defense.mdl"):
      return WXOR(L"Defense");
    case HASH("models/pickups/pickup_powerup_crit.mdl"):
      return WXOR(L"Crits");
    case HASH("models/pickups/pickup_powerup_agility.mdl"):
      return WXOR(L"Agility");
    case HASH("models/pickups/pickup_powerup_king.mdl"):
      return WXOR(L"King");
    case HASH("models/pickups/pickup_powerup_plague.mdl"):
      return WXOR(L"Plague");
    case HASH("models/pickups/pickup_powerup_reflect.mdl"):
      return WXOR(L"Reflect");
  }

  return L"";
}

bool c_base_entity::is_lunchable(){
  s_entity_info* info = entity_info->get_data(get_index());
  if(info == nullptr)
    return false;

  switch(info->model_hash){
    default: break;
    case HASH("models/items/plate.mdl"):
    case HASH("models/items/plate_sandwich_xmas.mdl"):
    case HASH("models/items/plate_robo_sandwich.mdl"):
    case HASH("models/workshop/weapons/c_models/c_fishcake/plate_fishcake.mdl"):
    case HASH("models/workshop/weapons/c_models/c_buffalo_steak/plate_buffalo_steak.md"):
    case HASH("models/workshop/weapons/c_models/c_chocolate/plate_chocolate.mdl"):
    case HASH("models/items/banana/plate_banana.mdl"):
      return true;
  }

  return false;
}

ALWAYSINLINE c_base_player* c_base_entity::get_sentry_target(){
  if(!is_sentry())
    return nullptr;

  if(auto_aim_target_handle() <= 0)
    return nullptr;

  return (c_base_player*)global->entity_list->get_entity_handle(auto_aim_target_handle());
}

i32 c_base_entity::update_delta(){
  // the localplayer is always simulated every tick
  if(this == global->localplayer)
    return 1;

  s_lc_data* lc = lag_compensation->get_data(this);

  if(lc == nullptr)
    return math::biggest(math::time_to_ticks(simulation_time() - old_simulation_time()), 1);

  s_lc_record* record1 = lc->get_record(0);
  s_lc_record* record2 = lc->get_record(1);

  if(record1 == nullptr || record2 == nullptr)
    return math::biggest(math::time_to_ticks(simulation_time() - old_simulation_time()), 1);

  return math::biggest(math::time_to_ticks(record1->simulation_time - record2->simulation_time), 1);
}

vec3 c_base_entity::est_velocity(){
  auto safe_delta = [](vec3 x, vec3 y, float time){
    vec3 delta = x - y;

    for(u8 i = 0; i < 3; i++)
      if(delta[i] != 0.f)
        delta[i] /= time;

    return delta;
  };

  s_lc_data* lc = lag_compensation->get_data(this);
  if(lc == nullptr)
    return safe_delta(origin(), old_origin(), math::ticks_to_time(update_delta()));

  s_lc_record* record1 = lc->get_record(0);
  s_lc_record* record2 = lc->get_record(1);

  if(record1 == nullptr || record2 == nullptr)
    return safe_delta(origin(), old_origin(), math::ticks_to_time(update_delta()));

  return safe_delta(record1->origin, record2->origin, math::ticks_to_time(update_delta()));
}

ALWAYSINLINE c_base_weapon* c_base_entity::get_launcher(){
  s_client_class* cc = get_client_class();
  if(cc == nullptr){
    assert(cc != nullptr);
    return nullptr;
  }

  switch(cc->id){
    case CTFProjectile_Rocket:
    case CTFProjectile_Flare:
    {
      if(rocket_launcher_handle() <= 0)
        return nullptr;

      return (c_base_weapon*)global->entity_list->get_entity_handle(rocket_launcher_handle());
    }
    case CTFGrenadePipebombProjectile:
    {
      if(pipebomb_launcher_handle() <= 0)
        return nullptr;

      return (c_base_weapon*)global->entity_list->get_entity_handle(pipebomb_launcher_handle());
    }
  }

  return nullptr;
}

ALWAYSINLINE c_base_player* c_base_entity::get_passtime_carrier(){
  s_client_class* cc = get_client_class();
  if(cc == nullptr){
    assert(cc != nullptr);
    return nullptr;
  }

  if(cc->id != CPasstimeBall)
    return nullptr;

  if(passtime_ball_carrier() <= 0)
    return nullptr;

  return (c_base_player*)global->entity_list->get_entity_handle(passtime_ball_carrier());
}

// Sticky Jumper and Rocket jumper do no damage.
bool c_base_entity::is_fake_projectile(){
  c_base_weapon* launcher = get_launcher();
  if(launcher == nullptr)
    return false;

  return launcher->is_blast_jumper_weapon();
}

bool c_base_entity::is_considered_enemy(c_base_entity* target){
  if(target == nullptr || global->aimbot_settings == nullptr || target == this)
    return false;

  // A little trick to make us start targetting teammates with the whip
  if(config->automation.auto_soldier_whip){
    c_base_player* player = (c_base_player*)this;

    // Only run logic on fellow team-mates.
    if(player->is_player_class(TF_CLASS_SOLDIER) && player->team() == target->team()){
      c_base_weapon* weapon = player->get_weapon();
      if(weapon != nullptr && weapon->is_melee() && weapon->is_soldier_whip())
        return true;
    }
  }

  if(target->is_player()){
    c_base_player* p = target->get_player();
    c_player_data* data = p->data();

    //if(data->is_dev_ignored)
    //  return false;
    
    if(data != nullptr){
      if(data->playerlist.ignore_player)
        return false;

      if(global->aimbot_settings->ignore_cheaters){
        //if(data->is_cheater || data->playerlist.mark_as_cheater)
        //  return false;
      }
    }
  }

  if(!global->aimbot_settings->target_cloaked_spies && target->is_player())
    if(((c_base_player*)target)->is_player_class(TF_CLASS_SPY) && ((c_base_player*)target)->is_cloaked())
      return false;

  if(!global->aimbot_settings->target_friends && is_player() && target->is_player())
    if(target->is_steam_friend())
      return false;

  if(global->aimbot_settings->target_friendlies && team() == target->team())
    return true;

  if(global->aimbot_settings->target_enemies && team() != target->team())
    return true;

  return false;
}

ALWAYSINLINE void c_base_entity::calc_nearest_point(vec3& src, vec3* vec_pos){
  assert(global->collision_property_calcnearestpoint != nullptr);
  utils::call_fastcall64<void, vec3&, vec3*>(global->collision_property_calcnearestpoint, collision(), src, vec_pos);
}

bool c_base_entity::get_hitbox_pos(i32 hitbox, vec3* output_pos){
  if(is_player()){
    assert(false && "This function is used for entities! Use lag compensation class for players!");
    return false;
  }
  
  assert(output_pos != nullptr);
  assert(hitbox >= 0);

  c_entity_data* data = entity_data();
  if(data == nullptr)
    return false;

  if(!data->has_bone_matrix)
    return false;

  s_studio_bbox* bbox = get_studio_bbox(hitbox);
  if(bbox == nullptr)
    return false;

  *output_pos = math::bbox_pos(bbox, data->bone_matrix, model_scale());
  return true;
}

ALWAYSINLINE i8* c_base_entity::get_class_name(){
  assert(global->get_class_name != nullptr);
  return utils::call_fastcall64<i8*>(global->get_class_name, this);
}

// C_BaseEntity::FirstMoveChild
ALWAYSINLINE c_base_entity* c_base_entity::first_move_child(){
  // search for "C_BaseEntity::UnlinkFromHierarchy(): Entity has a child with the wrong parent!\n" in client.dll
  // look up the top of the function, should give you something like "v1 = *(_DWORD *)(a1 + 0x22C);" where 0x22C = m_pMoveParent
  // 0x22C + 0x4 = m_pMoveChild
  return global->entity_list->get_entity(read<u32>(0x230/*m_pMoveChild*/) & 0xFFF);
}

// C_BaseEntity::NextMovePeer
ALWAYSINLINE c_base_entity* c_base_entity::next_move_peer(){
  // m_pMovePeer = m_pMoveChild + 0x4
  return global->entity_list->get_entity(read<u32>(0x234/*m_pMovePeer*/) & 0xFFF);
}


ALWAYSINLINE bool c_base_entity::is_krampus(){
  s_entity_info* info = entity_info->get_data(get_index());
  return info != nullptr && info->model_hash == HASH("models/bots/krampus.mdl");
} 

ALWAYSINLINE bool c_base_entity::is_boss(){
  s_client_class* cc = get_client_class();

  if(cc == nullptr){
    assert(cc != nullptr);
    return false;
  }

  if(is_krampus())
    return true;

  switch(cc->id){
    case CEyeballBoss:
      return this->team() > 3;
    case CHeadlessHatman:
    case CMerasmus:
    case CZombie:
    case CTFTankBoss:
    case CTFRobotDestruction_Robot:
      return true;
  }

  return false;
}