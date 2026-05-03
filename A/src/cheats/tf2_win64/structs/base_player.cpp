#include "../link.h"

ALWAYSINLINE c_player_data* c_base_player::data(){
  return utils::get_player_data(get_index());
}

bool c_base_player::is_sentry_buster(){
  if(max_health() == 2500 && is_player_class(TF_CLASS_DEMOMAN))
    return true;

  return false;
}

ALWAYSINLINE i32 c_base_player::get_ammo_count(i32 ammo_type){
  return utils::call_fastcall64<i32, i32>(global->get_ammo_count, this, ammo_type);
}

ALWAYSINLINE i32 c_base_player::get_loaded_ammo(){
  c_base_weapon* weapon = get_weapon();

  if(weapon == nullptr)
    return 0;

  i32 primary_ammo_count = get_ammo_count(weapon->primary_ammo_type());
  i32 clip1              = weapon->clip1();

  if(weapon->is_melee() || weapon->is_medigun() || !weapon->is_throwables() && weapon->is_harmless_item() || weapon->is_wrangler())
    return -1;

  return clip1 == -1 ? primary_ammo_count : clip1;
}

ALWAYSINLINE bool c_base_player::is_crit_boosted(){
  if(!is_player())
    return false;

  c_player_data* data = this->data();
  if(data == nullptr)
    return false;

  return data->tfdata.is_crit_boosted;
}

ALWAYSINLINE float c_base_player::get_max_speed(){
  if(!is_player())
    return 0.f;

  return utils::call_fastcall64<float, bool>(global->tf2_getmaxspeed, this, false);
}

ALWAYSINLINE float c_base_player::get_condition_prediction_time(u32 cond){
  assert(this != nullptr);
  assert(cond >= 0 && cond < TF_COND_COUNT);
  if(!is_player())
    return -1.f;

  c_player_data* data = this->data();
  if(data == nullptr)
    return -1.f;

  float time = math::time();
  if(data->tfdata.condition_active_time[cond] > time)
    return math::abs(data->tfdata.condition_active_time[cond] - time);
  
  return 0.f;
}

ALWAYSINLINE bool c_base_player::has_condition(u32 cond, bool predicted){
  assert(this != nullptr);
  assert(cond >= 0 && cond < TF_COND_COUNT);
  if(!is_player())
    return false;

  c_player_data* data = this->data();
  if(data == nullptr)
    return false;

  if(predicted){
    if(data->tfdata.condition_active_time[cond] > 0.f && data->tfdata.condition_active_time[cond] <= math::time())
      return false;
  }

  return data->tfdata.condition_active[cond];
}

ALWAYSINLINE void c_base_player::add_condition(u32 cond, float duration){
  if(!is_player())
    return;

  utils::call_fastcall64<void, u32, float, u32>(global->ctfplayershared_addcond, shared(), cond, duration, 0);
}

ALWAYSINLINE void c_base_player::remove_condition(u32 cond){
  if(!is_player())
    return;

  utils::call_fastcall64<void, u32, bool>(global->ctfplayershared_removecond, shared(), cond, true);
}

ALWAYSINLINE bool c_base_player::can_move_during_taunt(){
  if(!is_player())
    return false;

  return utils::call_fastcall64<bool>(global->ctfplayer_canmoveduringtaunt, this);
}

// TODO: find how the syringe gun reloads and check if its in a reload state then return true
ALWAYSINLINE bool c_base_player::can_fire(){
  if(!is_player())
    return false;

  if(double_tap->is_charging || is_bonked() || deadringer_deployed() || is_taunting())
    return false;

  if(double_tap->is_shift_tick())
    return true;

  c_base_weapon* wep = get_weapon();

  if(wep == nullptr)
    return false;

  // more logic here
  if(wep->is_minigun() && wep->weapon_state() <= 1)
    return false;

  if(wep->weapon_id() == WPN_Machina && !is_scoped())
    return false;

  if(wep->is_medigun())
    return should_medigun_fire();

  if(wep->is_wrangler())
    return true;

  // I've checked everywhere and looked at every timer smack_time, primary attack, etc etc...
  // When throwing the weapon the ammo count will be zero and during that we'll assume that we can still "Fire".
  // This is because this weapon uses an animation timer of at least 200ms so no working this thing is stupid.
  if(wep->is_throwables() || wep->is_ball_bat())
    return throwable_should_fire();
  
  if(!wep->is_beggars() && !wep->is_melee() && (get_ammo_count(wep->primary_ammo_type()) < 1 && wep->clip1() == -1 || wep->clip1() == 0))
    return false;

  if(wep->reloading())
    return true;

  return wep->is_flamethrower() || wep->next_primary_attack() <= globaldata->cur_time;
}

ALWAYSINLINE bool c_base_player::can_fire_secondary(bool all_weapons_required, bool no_button_check = false){
  if(!is_player())
    return false;

  c_base_weapon* wep = get_weapon();
  if(wep == nullptr)
    return false;

  // We only care about these weapons.
  if(!all_weapons_required)
    if(!wep->is_flamethrower() && !wep->is_medigun() && !wep->is_ball_bat() && wep->weapon_id() != WPN_ShortCircut)
      return false;

  if(wep->is_ball_bat())
    return throwable_should_fire(true);

  if(wep->next_secondary_attack() > globaldata->cur_time)
    return false;

  if(wep->is_medigun()){
    if(!wep->can_uber())
      return false;
    else{
      if(wep->weapon_id() == WPN_Vaccinator){
        if(wep->charge_resist_type() == 0 && is_bullet_charge())
          return false;
        else if(wep->charge_resist_type() == 1 && is_blast_charge())
          return false;
        else if(wep->charge_resist_type() == 2 && is_fire_charge())
          return false;
      }
    }
  }

  if(wep->is_flamethrower()){
    i32 clip = get_ammo_count(1);
    if(clip < 20 || clip < 40 && wep->weapon_id() == WPN_Backburner)
      return false;
  }

  return global->current_cmd->buttons & IN_ATTACK2 || no_button_check;
}

bool c_base_player::throwable_should_fire(bool holding_fire_button_check){
  c_base_weapon* wep = get_weapon();
  if(wep == nullptr)
    return false;

  i32 ammo = get_loaded_ammo();
  if(wep->is_ball_bat()){ 
    if(ammo){
      if(global->animation_fire_time <= 0.f)
        return false;

      if(global->animation_fire_time <= globaldata->cur_time){
        float delta = math::abs(global->animation_fire_time - globaldata->cur_time);
        if(delta <= 0.1f)
          return true;
      }
    }

    if(!holding_fire_button_check)
      return ammo > 0 && wep->next_secondary_attack() <= globaldata->cur_time;

    return ammo > 0 && wep->next_secondary_attack() <= globaldata->cur_time && global->current_cmd->buttons & IN_ATTACK2;
  }
  else if(wep->is_throwables()){

    if(ammo == 0){
      if(global->animation_fire_time <= 0.f)
        return false;

      if(global->animation_fire_time <= globaldata->cur_time){
        float delta = math::abs(global->animation_fire_time - globaldata->cur_time);
        if(delta <= 0.1f)
          return true;
      }

      return false;
    }

    if(!holding_fire_button_check)
      return ammo > 0 && wep->next_primary_attack() <= globaldata->cur_time;

    return wep->next_primary_attack() <= globaldata->cur_time && global->current_cmd->buttons & IN_ATTACK; 
  }

  return true;
}

ALWAYSINLINE bool c_base_player::should_medigun_fire(){
  c_base_weapon* wep = get_weapon();
  if(wep == nullptr)
    return false;

  if(!wep->is_medigun())
    return false;

  if(global->lookat_target_entindex == -1)
    return false;

  c_base_entity* entity = global->entity_list->get_entity(global->lookat_target_entindex);
  if(entity == nullptr)
    return false;

  // Can only heal players + revive markers.
  if(!entity->is_player() && !entity->is_revive_marker())
    return false;

  // Can only heal friendly / disguised enemy players.
  if(entity->is_player()){
    c_base_player* p = (c_base_player*)entity;
    if(entity->team() != team() && !p->is_disguised())
      return false;
  }
  else{
    if(entity->team() != team())
      return false;
  }

  c_base_player* heal_target = wep->get_heal_target();
  if(heal_target != nullptr){
    if(heal_target->get_index() != global->lookat_target_entindex)
      return true;
  }
  else
    return true;

  return false;
}

ALWAYSINLINE bool c_base_player::should_secondary_pda_fire(){
  c_base_weapon* wep = get_weapon();
  if(wep == nullptr)
    return false;

  if(!wep->is_wrench())
    return false;

  if(global->lookat_target_entindex == -1)
    return false;

  c_base_entity* entity = global->entity_list->get_entity(global->lookat_target_entindex);
  if(entity == nullptr)
    return false;

  if(!entity->is_sentry() && !entity->is_dispenser() && !entity->is_teleporter())
    return false;

  c_base_player* owner = entity->get_building_owner();
  if(owner == nullptr)
    return false;

  return (owner->get_index() == get_index());
}

ALWAYSINLINE bool c_base_player::will_fire_this_tick(){
  if(global->current_cmd == nullptr)
    return false;

  c_base_weapon* wep = get_weapon();
  if(wep == nullptr)
    return false;

  if(is_taunting())
    return false;
 
  // 
  if(wep->is_wrench() && global->current_cmd->buttons & IN_ATTACK2)
    return should_secondary_pda_fire();
  
  if(wep->get_slot() == 2 && !wep->is_knife())
    return wep->smack_time() > 0.f && globaldata->cur_time > wep->smack_time();

  if(wep->is_sticky_launcher()){
    float charge_time     = (globaldata->cur_time - wep->charge_begin_time());
    float max_charge_time = wep->get_sticky_launcher_max_charge();
    return (wep->charge_begin_time() > 0.f && charge_time >= max_charge_time || !(global->current_cmd->buttons & IN_ATTACK)) && wep->charge_begin_time() > 0.f && charge_time > 0.f;
  }
  else if(wep->weapon_id() == WPN_LoooseCannon){
    float charge_time = 1.f - (globaldata->cur_time - wep->get_loose_cannon_charge_begin_time());
    return charge_time > 0.f && !(global->current_cmd->buttons & IN_ATTACK);
  }
  else if(wep->is_huntsman())
    return wep->charge_begin_time() > 0.f && !(global->current_cmd->buttons & IN_ATTACK);
  else if(wep->is_classic_sniper_rifle())
    return wep->charged_damage() > 0.f && !(global->current_cmd->buttons & IN_ATTACK);
  else if(wep->is_cow_mangler()){
    if(wep->cow_mangler_charge_begin_time() > 0.f)
      return (globaldata->cur_time - wep->cow_mangler_charge_begin_time()) >= 2.0f;
  }
  else if(wep->is_beggars()){
    // It does work however. You release your IN_ATTACK instantly it'll still shoot where ever your anti-aim is at or where you're looking if using aimbot -rud
    return wep->clip1() > 0 && wep->reload_mode() == 0 && wep->next_primary_attack() <= globaldata->cur_time;
  }
  else if(wep->is_wrangler())
    return global->current_cmd->buttons & IN_ATTACK || global->current_cmd->buttons & IN_ATTACK2;
  else if(wep->is_builder_pda())
    return global->current_cmd->buttons & IN_ATTACK;
  else if(wep->is_throwables())
    return throwable_should_fire(true);
  
  if(!can_fire())
    return false;

  return global->current_cmd->buttons & IN_ATTACK;
}

ALWAYSINLINE bool c_base_player::is_player_blocked(){
  if(!is_player())
    return false;

  void* voice_mgr = utils::call_fastcall64<void*>(global->get_client_voice_manager, (void*)1);

  if(voice_mgr == nullptr)
    return false;

  return utils::call_fastcall64<bool, i32>(global->is_player_blocked, voice_mgr, get_index());
}

// When we set the blocked state
// We need to check if the player is blocked, depending on if they are or arent
// We call the function again to swap the block status and then check that its what we want
// We need to do this because "blocked" in set player block state does nothing
ALWAYSINLINE void c_base_player::set_blocked_state(bool blocked){
  if(!is_player())
    return;

  void* voice_mgr = utils::call_stdcall<void*>(global->get_client_voice_manager);

  if(voice_mgr == nullptr)
    return;

  if(is_player_blocked() && blocked || !is_player_blocked() && !blocked)
    return;

  utils::call_fastcall64<void, i32, bool>(global->set_player_blocked_state, voice_mgr, get_index(), blocked);
  DBG("c_base_player::set_blocked_state %i for %i\n", blocked ? 1 : 0, get_index());
}

ALWAYSINLINE bool c_base_player::can_headshot(){
  return has_condition(TF_COND_ZOOMED) && math::abs(globaldata->cur_time - fov_time()) >= 0.2f;
}

bool c_base_player::can_spy_headshot(){
  c_base_weapon* wep = get_weapon();
  if(wep == nullptr)
    return false;

  return wep->weapon_id() == WPN_Ambassador || wep->weapon_id() == WPN_FestiveAmbassador;
}

ALWAYSINLINE float c_base_player::get_sniper_charge_damage(c_base_player* target, bool headshot){
  if(!is_player())
    return 0.f;

  c_base_weapon* wep = get_weapon();

  if(wep == nullptr)
    return 0.f;

  bool crit_boosted = is_crit_boosted();

  if(!has_condition(TF_COND_ZOOMED) || !can_headshot())
    return 50.f * (crit_boosted ? 3.f : 1.f);

  float dmg_mod    = (headshot || crit_boosted) ? 3.f : 1.f;
  float dmg        = math::clamp(wep->charged_damage() * dmg_mod, 50.f * dmg_mod, 450.f);

  if(wep->charged_damage() >= 150.f)
    dmg = utils::attrib_hook_float(dmg, XOR("sniper_full_charge_damage_bonus"), wep, 0, 1);

  if(target != nullptr){
    if(!target->is_player())
      return dmg;

    if(!headshot)
      dmg = utils::attrib_hook_float(dmg, XOR("bodyshot_damage_modify"), wep, 0, 1);
    else{
      dmg = utils::attrib_hook_float(dmg, XOR("headshot_damage_modify"), wep, 0, 1);
    }

    dmg = utils::attrib_hook_float(dmg, XOR("mult_dmgtaken"), target, 0, 1);
    dmg = utils::attrib_hook_float(dmg, XOR("mult_dmgtaken_from_bullets"), target, 0, 1);
    if(!headshot){
      if(has_condition(TF_COND_OFFENSEBUFF) || target->has_condition(TF_COND_URINE) || target->has_condition(TF_COND_MARKEDFORDEATH) || target->has_condition(TF_COND_ENERGY_BUFF))
        dmg *= 1.35f; // mini-crits give us a 35% damage bonus.
    }
    else
      dmg = utils::attrib_hook_float(dmg, XOR("mult_dmgtaken_from_crit"), target, 0, 1);

    if(target->is_cloaked())
      dmg *= 0.80f; // Cloaked players have a 20% damage res.

    if(target->has_condition(TF_COND_MEDIGUN_SMALL_BULLET_RESIST))
      dmg *= 0.90f;
    else if(target->has_condition(TF_COND_MEDIGUN_UBER_BULLET_RESIST))
      dmg *= 0.25f;

    c_base_weapon* target_wep = target->get_weapon();
    if(target_wep != nullptr){

      if(target_wep->weapon_id() == WPN_FistsOfSteel)
        dmg *= 0.60f;

      if(target_wep->weapon_id() == WPN_BrassBeast || target_wep->weapon_id() == WPN_Natascha){
        if(target->get_health_percent() < 50.f && target->has_condition(TF_COND_AIMING))
          dmg *= 0.80f;
      }

      // This has no effect on headshot damage.
      if(!headshot && target->has_condition(TF_COND_DEFENSEBUFF)){
        dmg *= 0.65f; // 35% damage res.
      }
    }
  }

  return dmg;
}

bool c_base_player::can_huntsman_kill(c_base_player* target){
  if(target == nullptr)
    return false;

  c_base_weapon* wep = get_weapon();

  if(wep == nullptr)
    return false;

  if(!wep->is_huntsman())
    return false;

  if(wep->charge_begin_time() <= 0.f)
    return false;

  float delta = globaldata->cur_time - wep->charge_begin_time();

  if(delta <= 0.f)
    return false;

  float dmg_mod = math::clamp(delta, 0.f, 1.f);
  float dmg     = math::clamp(360.f * dmg_mod, 0.f, 360.f);

  float health = 360.f;
  if(target->is_player())
    health = (float)target->health();
  else if(target->is_sentry() || target->is_dispenser() || target->is_teleporter())
    health = (float)target->object_health();

  return dmg_mod >= 0.333f && (dmg >= health || dmg_mod >= 1.f);
}

i32 c_base_player::get_max_buffed_health(){
  assert(this != nullptr);

  c_player_data* data = this->data();
  if(data == nullptr)
    return 0;

  i32 max_hp = data->tfdata.max_buffed_health;

  c_base_weapon* wep = get_weapon();
  if(wep == nullptr)
    return max_hp;

  switch(wep->weapon_id()){
    default: break;
    case WPN_FistsOfSteel:
    {
      max_hp = 390;
      break;
    }
  }

  return max_hp;
}

float c_base_player::get_air_speed_cap(){
  if(grappling_hook_target_handle() > 0){
    DBG("[!] This player has a grappling_hook_target_handle above zero!\n");

    if(has_condition(TF_COND_RUNE_AGILITY)){
      switch(player_class()){
        default: return 950.f;
        case TF_CLASS_HEAVY:
        case TF_CLASS_SOLDIER:
          return 850.f;
      }
    }

    return global->tf_grapplinghook_move_speed->flt;
  }
  else if(has_condition(TF_COND_SHIELD_CHARGE))
    return global->tf_max_charge_speed->flt;

  float cap = 30.f;
  if(has_condition(TF_COND_PARACHUTE_ACTIVE))
    cap *= global->tf_parachute_aircontrol->flt;
  else if(has_condition(TF_COND_HALLOWEEN_KART)){
    if(has_condition(TF_COND_HALLOWEEN_KART_DASH))
      return global->tf_halloween_kart_dash_speed->flt;

    cap *= global->tf_halloween_kart_aircontrol->flt;
  }

  float increased_air_control = utils::attrib_hook_float(1.f, XOR("mod_air_control"), this, 0, 1);

  return (cap * increased_air_control);
}

bool c_base_player::is_carrying_objective(){
  assert(global->ctfplayer_hastheflag != nullptr);
  i32 types = 0;
  return utils::call_fastcall64<bool, i32*, i32>(global->ctfplayer_hastheflag, this, &types, 0);
}

ALWAYSINLINE bool c_base_player::is_localplayer(){
  return get_index() == global->engine->get_local_player();
}

ALWAYSINLINE bool c_base_player::has_razer_back(){
  return utils::attrib_hook_int(0, XOR("set_blockbackstab_once"), this, 0, 1) == 1;
}

ALWAYSINLINE bool c_base_player::is_dueling(bool cache){
  return false;
}

ALWAYSINLINE c_base_player* c_base_player::get_spectate_target(){
  if(observer_target() <= 0)
    return nullptr;

  c_base_player* spectate_target = global->entity_list->get_entity_handle(observer_target());
  if(spectate_target == nullptr)
    return nullptr;

  if(!spectate_target->is_player())
    return nullptr;

  return spectate_target;
}

ALWAYSINLINE bool c_base_player::is_saxton_hale(){
  if(player_class() != TF_CLASS_HEAVY)
    return false;

  void* model = get_model();
  if(model == nullptr)
    return false;

  i8* model_name = global->model_info->get_model_name(model);
  if(model_name == nullptr)
    return false;

  u32 hash = HASH_RT(model_name);
  return hash == HASH("models/player/saxton_hale.mdl");
}

i32 c_base_player::get_heal_arrow_heal_amount(){
  c_player_data* data = this->data();
  if(data == nullptr)
    return 0;

  return data->tfdata.heal_bolt_amount;
}

void* c_base_player::get_equip_wearable_for_loadout_slot(i32 slot){
  return utils::call_fastcall64<void*, i32>(global->get_equip_wearable_for_loadout_slot_addr, this, slot);
}

bool c_base_player::has_canteen_equipped(){
  c_base_entity* action_slot = get_equip_wearable_for_loadout_slot(9);
  if(action_slot == nullptr)
    return false;

  s_client_class* cc = action_slot->get_client_class();
  if(cc == nullptr)
    return false;

  return cc->id == CTFPowerupBottle;
}
