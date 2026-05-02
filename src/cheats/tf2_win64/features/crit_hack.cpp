#include "../link.h"

CLASS_ALLOC(c_crit_hack, crit_hack);

void c_crit_hack::reset_bucket_backup(){
  memset(&bucket_backup, 0, sizeof(bucket_backup));
}

void c_crit_hack::resync_bucket(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!needs_bucket_resync)
    return;

  for(i32 i = 0; i < 7; i++){
    c_base_weapon* wep = localplayer->get_weapon_from_belt(i, false);
    if(wep == nullptr)
      continue;

    if(bucket_backup[i].player_class != localplayer->player_class() || bucket_backup[i].item_def_index != wep->weapon_id()){
      reset_bucket_backup();
      break;
    } 

    wep->crit_checks()        = bucket_backup[i].crit_checks;
    wep->crit_seed_requests() = bucket_backup[i].crit_seed_requests;
    wep->crit_token_bucket()  = bucket_backup[i].crit_token_bucket;
  }

  needs_bucket_resync = false;
}

void c_crit_hack::backup_crit_bucket(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  for(i32 i = 0; i < 7; i++){
    c_base_weapon* wep = localplayer->get_weapon_from_belt(i, false);
    if(wep == nullptr)
      continue;

    bucket_backup[i].player_class       = localplayer->player_class();
    bucket_backup[i].item_def_index     = wep->weapon_id();
    bucket_backup[i].crit_checks        = wep->crit_checks();
    bucket_backup[i].crit_seed_requests = wep->crit_seed_requests();
    bucket_backup[i].crit_token_bucket  = wep->crit_token_bucket();
  }
}

void c_crit_hack::reset_damages(){
  global->total_round_damage                      = 0;
  global->total_round_crit_random_damage          = 0;
  global->total_round_crit_boosted_damage         = 0;
  global->total_round_melee_damage                = 0;

  tfstat_damage_ranged_crit_random.reset();
  DBG("[!] c_crit_hack::reset_damages\n");
}

void c_crit_hack::update_damages(){
  c_base_player* localplayer = utils::localplayer();

  if(localplayer == nullptr)
    return;

  c_score_data* score_data = localplayer->get_round_score_data();

  if(score_data == nullptr){
    DBG("[-] c_crit_hack::update_damages - score_data nullptr\n");
    return;
  }

  i32 new_total_round_damage = score_data->damage_done();
  if(global->total_round_damage > 0 && new_total_round_damage == 0){
    reset_damages();
    DBG("[!] c_crit_hack::update_damages - reset round damages\n");
  }

  global->total_round_damage = new_total_round_damage;
}

// The server does not network us our "TFSTAT_DAMAGE_RANGED_CRIT_RANDOM" stat
// However, when we attempt to force a crit, CanFireRandomCriticalShot is called on the server
// Which will set a float called m_flObservedCritChance that is also networked to our client
// Because we know our "TFSTAT_DAMAGE_RANGED" stat, we can extract the "TFSTAT_DAMAGE_RANGED_CRIT_RANDOM" stat
// From the crit chance by manually calculating our own observed crit chance and comparing it to the servers
// Then from that we can accurately calculate our "TFSTAT_DAMAGE_RANGED_CRIT_RANDOM" stat
void c_crit_hack::find_tfstat_damage_ranged_crit_random(float observed_crit_chance){
  float closest_observed      = 0.f;
  i32   tfstat_damage_ranged  = global->total_round_damage;
  i32   real_damage           = 0;

  // Iterate from last synced damage to TFSTAT_DAMAGE_RANGED
  for(i32 i = tfstat_damage_ranged_crit_random.last_synced_damage; i <= tfstat_damage_ranged; i++){
    float normalized_damage = (float)i / 3.f;
    float observed_sim      = normalized_damage / (normalized_damage + ((float)tfstat_damage_ranged - (float)i));
    float delta             = math::abs(observed_crit_chance - observed_sim);

    // Store damage that contains lowest delta from our observed crit chance to servers networked
    // m_flObservedCritChance, We have to do it like this because of network compression
    // The m_flObservedCritChance will be compressed and FPP (float precision) will be lost
    // This method will still find the damage we need
    if(delta <= math::abs(observed_crit_chance - closest_observed)){
      real_damage       = i;
      closest_observed  = observed_sim;
    }
    else if(real_damage > 0){
      DBG("[!] TFSTAT_DAMAGE_RANGED_CRIT_RANDOM: closest_observed: %f, observed_crit_chance: %f, delta: %f\n", closest_observed, observed_crit_chance, delta);
      break;
    }
  }

  // If we have found the real damage
  if(real_damage > 0 && global->total_round_crit_random_damage != real_damage){
    DBG("[+] TFSTAT_DAMAGE_RANGED_CRIT_RANDOM: real_damage: %i, desynced_damage: %i, closest_observed: %f\n", real_damage, global->total_round_crit_random_damage, closest_observed);

    // Set our data for the sync struct
    tfstat_damage_ranged_crit_random.last_sync_time     = math::time();
    tfstat_damage_ranged_crit_random.last_synced_damage = real_damage;
    tfstat_damage_ranged_crit_random.was_desynced       = real_damage != global->total_round_crit_random_damage;

    // Now update client with true TFSTAT_DAMAGE_RANGED_CRIT_RANDOM
    global->total_round_crit_random_damage = real_damage;
  }
}

i32 c_crit_hack::get_weapon_mask_xor(){
  c_base_player* localplayer = utils::localplayer();

  if(localplayer == nullptr)
    return 0;

  c_base_weapon* weapon = localplayer->get_weapon();

  if(weapon == nullptr)
    return 0;

  i32 mask_xor = (weapon->get_index() << 8) | (global->localplayer_index);

  if(weapon->is_melee())
    mask_xor <<= 8;

  return mask_xor;
}

void c_crit_hack::calculate_can_crit(){
  can_crit = false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  s_cmd_list* force_list = get_cmd_list(true);
  assert(force_list != nullptr);

  if(force_list == nullptr || !force_list->filled)
    return;

  u32 backup_start           = CRIT_HACK_BACKUP_START;
  u32 backup_end             = CRIT_HACK_BACKUP_END;
  u32 backup_size            = backup_end - backup_start;
  static void*  backup_data  = malloc(backup_size);

  // backup weapon data
  memcpy(backup_data, (uptr)weapon + (uptr)backup_start, backup_size);

  // make sure randomseed respects our prediction seed
  weapon->current_seed() = -1;

  // make calcisattackcriticalhelper run every tick instead of every second for stream weapons
  weapon->last_crit_check_time()            = -1.f;
  weapon->last_rapid_fire_crit_check_time() = -1.f;

  // bypass calcisattackcritical framecount check
  weapon->last_crit_check_frame() = globaldata->frame_count - 1;

  // set weapon mode
  i32 old_weapon_mode = weapon->weapon_mode();
  weapon->weapon_mode() = 0;

  // BUG: force_list->current_cmd returns an invalid crit command in the event nospread takes awhile to calculate no idea why.
  u32 old_prediction_seed         = *global->prediction_random_seed;
  *global->prediction_random_seed = math::md5_pseudo_random(force_list->current_cmd()) & 0x7FFFFFFF;

  // invoke the games crit calculation function
  utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->calc_is_attack_critical_hook_trp), weapon);

  float simulated_observed_crit_chance = weapon->observed_crit_chance();

  *global->prediction_random_seed = old_prediction_seed;

  // store if we can crit
  can_crit = weapon->current_attack_is_crit();

  // restore weapon mode
  weapon->weapon_mode() = old_weapon_mode;

  // restore weapon data
  memcpy((uptr)weapon + (uptr)backup_start, backup_data, backup_size);

  // set our new simulated observed crit chance
  if(!weapon->is_melee())
    weapon->observed_crit_chance() = simulated_observed_crit_chance;
  else // melee weapons cant be crit banned
    crit_banned = false;
}

// Fixes a bug causing weapons such as the minigun to operate in the incorrect weapon mode
// This causes the client when holding attack2 aswell as attack to call calcisattackcritical every tick
// By releasing attack2 when attack is pressed, we can fix this issue entirely and have synced crits while revving
void c_crit_hack::fix_shared_attack_bug(){
  if(global->current_cmd == nullptr)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  if(!weapon->is_minigun() && !weapon->is_crossbow())
    return;

  if((global->current_cmd->buttons & IN_ATTACK) && (global->current_cmd->buttons & IN_ATTACK2))
    global->current_cmd->buttons &= ~IN_ATTACK2;
}

// When we hold down in attack the server will process our commands with in_attack down
// The issue is that if the server gives us ammo or ammo is applied to us before we get that update (because of ping)
// We will fire a bullet on the server before we do on the client, this shot will not be registered on the client
// And things like crit bucket will become desynced.
// The solution is simple; when we have no ammo on the client, strip the in attack flag to ensure the server can never fire and therefor solving the ping issue
void c_crit_hack::fix_primary_attack_bug(bool pre_prediction){
  if(global->current_cmd == nullptr)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  if(weapon->weapon_id() == WPN_BeggersBazooka)
    return;

  static i32 pre_predicted_ammo_count = 0;

  if(pre_prediction)
    pre_predicted_ammo_count = localplayer->get_loaded_ammo();

  if(pre_predicted_ammo_count == 0 && (global->current_cmd->buttons & IN_ATTACK))
    global->current_cmd->buttons &= ~IN_ATTACK;

  // 'Apparently' this was messing with auto vaccinator.
  // It would actually make sense.
  if(weapon->is_vaccinator())
    return;

  if(weapon->is_beggars()){
    if(weapon->clip1() > 0 && weapon->reload_mode() == 0)
      global->current_cmd->buttons &= ~IN_ATTACK;

    return;
  }

  // Also, when we press IN_ATTACK with aimbot the server will fight with IN_RELOAD
  // Recalculate this behaviour and strip the in_reload when in_attack is pressed
  if(weapon->does_weapon_reload() && global->cl_autoreload->val == 1 && !pre_prediction){
    if(global->current_cmd->buttons & IN_ATTACK){
      global->current_cmd->buttons &= ~IN_RELOAD;
      localplayer->buttons() &= ~IN_RELOAD;
    }
    else{
      global->current_cmd->buttons |= IN_RELOAD;
      localplayer->buttons() |= IN_RELOAD;
    }
  }
}

float c_crit_hack::calculate_observed_crit_chance(bool is_melee, i32 crit_dmg_simulate, i32 normal_damage_simulate){
  if(is_melee)
    return 0.f;

  i32 ranged_crit_random = global->total_round_crit_random_damage + crit_dmg_simulate;
  i32 ranged_damage      = (global->total_round_damage - global->total_round_melee_damage) + normal_damage_simulate;

  if(ranged_damage <= 0 || ranged_crit_random <= 0)
    return 0.f;

  float normalized_damage = (float)ranged_crit_random / 3.f;
  float ratio             = normalized_damage + ((float)ranged_damage - (float)ranged_crit_random);

  if(normalized_damage <= 0.f || ratio <= 0.f)
    return 0.f;

  return normalized_damage / ratio;
}

void c_crit_hack::can_fire_random_critical_shot_run(c_base_weapon* ecx, float crit_chance){
  if(ecx == nullptr){
    DBG("[-] c_crit_hack::can_fire_random_critical_shot_run - ecx is nullptr\n");
    return;
  }

  ecx->observed_crit_chance() = 0.f;
  crit_chance_correct       = 0.f;
  crit_banned               = false;

  float rebuilt_crit_chance = calculate_observed_crit_chance(ecx->is_melee());
  ecx->observed_crit_chance() = rebuilt_crit_chance;

  crit_chance_correct       = crit_chance + 0.1f;
  crit_banned               = ecx->observed_crit_chance() >= crit_chance_correct;
}

s_cmd_list* c_crit_hack::get_cmd_list(bool crit){
  c_base_player* localplayer = utils::localplayer();

  if(localplayer == nullptr)
    return nullptr;

  c_base_weapon* weapon = localplayer->get_weapon();

  if(weapon == nullptr)
    return nullptr;

  i32 idx = weapon->get_index() - 1;
  if(idx < 0 || idx >= 4096){
    DBG("[-] c_crit_hack::get_cmd_list - weapon index is invalid (%i)\n", idx);
    return nullptr;
  }

  return crit ? &force_cmds[idx] : &skip_cmds[idx];
}

// calculates the crit cost based on the requests and checks made
float c_crit_hack::get_crit_cost(i32 requests, i32 checks, bool melee, bool use_rapid_fire_crits){
  float mult = 0.5f;

  if(!melee){
    float ratio = (float)(requests + 1) / (float)(checks);

    if(ratio != 0.1f)
      mult = (ratio - 0.1f) / 0.9f;
    else
      mult = 0.f;

    mult = 1.f + (2.f) * math::clamp(mult, 0.f, 1.f);
  }

  float cost = (damage_correct * 3.f) * mult;

  // weapons add this damage to thee critbucket every fire/melee
  // so basically the math goes that the game does something like this in the end
  // (cost = (cost - damage + ((damage * 3.f) * cost)))
  if(!use_rapid_fire_crits)
    cost -= damage_correct;

  return math::floor(cost);
}

bool c_crit_hack::get_crit_counts(i32* available_crits, i32* potential_crits, float* remaining_bucket = nullptr, float* remaining_cost = nullptr){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  ctf_weapon_info* weapon_info = weapon->get_weapon_info();
  if(weapon_info == nullptr)
    return false;

  bool is_melee             = weapon->get_slot() == 2;
  bool use_rapid_fire_crits = weapon_info->use_rapid_fire_crits;

  // calculate how many crits we have based on our current crit bucket
  if(available_crits != nullptr){
    i32 seed_requests = weapon->crit_seed_requests();
    float cost        = get_crit_cost(seed_requests, weapon->crit_checks(), is_melee, use_rapid_fire_crits);
    float bucket      = weapon->crit_token_bucket();

    *available_crits = 0;
    while(cost > 0.f && bucket >= cost){
      bucket -= cost;

      if(can_crit)
        *available_crits = *available_crits + 1;

      seed_requests++;
      cost = get_crit_cost(seed_requests, weapon->crit_checks(), is_melee, use_rapid_fire_crits);
    }

    if(remaining_bucket != nullptr)
      *remaining_bucket = bucket;

    if(remaining_cost != nullptr)
      *remaining_cost = cost;
  }

  // calculate how many crits we could potentially have based on the maximum bucket value
  if(potential_crits != nullptr){
    float cost        = get_crit_cost(0, 100000, is_melee, use_rapid_fire_crits);
    float bucket      = global->tf_weapon_criticals_bucket_cap->flt;

    *potential_crits = 0;
    while(cost > 0.f && bucket >= cost){
      bucket -= cost;
      *potential_crits = *potential_crits + 1;
    }
  }

  if(potential_crits == nullptr && available_crits == nullptr){
    DBG("[-] c_crit_hack::get_crit_counts - strange call both available_crits and potential_crits are nullptr. (nothing was done)\n");
    return false;
  }

  return true;
}

void c_crit_hack::update_damage_for_unban(){
  if(!crit_banned){
    global->damage_to_unban         = 0;
    global->display_damage_to_unban = 0;
    return;
  }

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  static s_crit_damage_thread_data* thread_data = malloc(XOR32(sizeof(s_crit_damage_thread_data)));
  memset(thread_data, 0, XOR32(sizeof(s_crit_damage_thread_data)));

  // Setup thread.
  {
    thread_data->crit_chance = crit_chance_correct;
    thread_data->is_melee    = weapon->is_melee();
    thread_data->run_thread  = true;
  }

  static bool thread_created = false;
  if(thread_created)
    return;

  thread_created = true;

  DBG("[+] Creating update_damage_for_unban thread.\n");
  if(!utils::create_worker([](s_crit_damage_thread_data* thread_data){
    while(true){
      if(global->unloading)
        break;

      if(!thread_data->run_thread){
        Sleep(1);
        continue;
      }

      thread_data->run_thread = false;

      global->damage_to_unban = 0;

      i32 max_damage = XOR32(100000);
      for(i32 i = 0; i <= max_damage; i++){
        if(crit_hack->calculate_observed_crit_chance(thread_data->is_melee, 0, i) < thread_data->crit_chance){
          global->damage_to_unban = i;
          break;
        }
      }

      // The required damage is so high.
      if(global->damage_to_unban <= 0)
        global->damage_to_unban = max_damage;

      global->display_damage_to_unban = global->damage_to_unban;
    }
    return 0;
  }, thread_data));

}

void c_crit_hack::fire_event_run(c_game_event* event){
  if(event == nullptr){
    assert(event != nullptr);
    return;
  }

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  i32  attacked  = global->engine->get_player_for_user_id(event->get_int(XOR("userid")));
  i32  attacker  = global->engine->get_player_for_user_id(event->get_int(XOR("attacker")));
  bool crit      = event->get_bool(XOR("crit"));
  i32  damage    = event->get_int(XOR("damageamount"));
  i32  health    = event->get_int(XOR("health"));
  i32  weapon_id = event->get_int(XOR("weaponid"));

  if(attacker != global->localplayer_index || attacked == attacker)
    return;

  c_base_player* attacked_entity = global->entity_list->get_entity(attacked);
  if(attacked_entity == nullptr)
    return;

  if(!attacked_entity->is_player())
    return;

  c_player_data* data = attacked_entity->data();

  if(data == nullptr)
    return;

  //DBG("health: %i %i\n", health, data->health_before_death);

  if(data->health_before_death <= 1)
    data->health_before_death = attacked_entity->health();

  if(health == 0 && damage > data->health_before_death)
    damage = data->health_before_death;

#if defined(DEV_MODE) && defined(CRIT_HACK_DEBUG)
  DBG("damage: %i\n", damage);
#endif

  c_base_weapon* weapon_used = localplayer->get_weapon_from_belt(weapon_id);

  if(weapon_used != nullptr){
    if(weapon_used->is_melee() && global->last_fired_weapon_id != weapon_used->get_id()){
      c_base_weapon* new_weapon_used = localplayer->get_weapon_from_belt(global->last_fired_weapon_id);

      if(new_weapon_used != nullptr)
        weapon_used = new_weapon_used;
    }

    if(weapon_used->is_melee())
      global->total_round_melee_damage += damage;
    else if(crit){
      if(localplayer->is_crit_boosted())
        global->total_round_crit_boosted_damage += damage;
      else
        global->total_round_crit_random_damage += damage;
    }

#if defined(DEV_MODE) && defined(CRIT_HACK_DEBUG)
      DBG("[!] total_round_crit_random_damage %i\n", global->total_round_crit_random_damage);
      DBG("[!] total_round_crit_boosted_damage %i\n", global->total_round_crit_boosted_damage);
      DBG("[!] total_round_melee_damage %i\n", global->total_round_melee_damage);
#endif
  }
}

void c_crit_hack::fill_crit_seed_list(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  if(!weapon->can_randomly_crit())
    return;

  c_aimbot_settings* settings = utils::get_aimbot_settings();
  if(settings == nullptr)
    return;

  s_cmd_list* force_list = get_cmd_list(true);
  s_cmd_list* skip_list  = get_cmd_list(false);
  assert(force_list != nullptr);
  assert(skip_list  != nullptr);

  // wait for threads to stop first if they are running
  if(force_list->thread_running || skip_list->thread_running)
    return;

  i32 mask_xor = get_weapon_mask_xor();
  if(force_list->mask_xor != mask_xor || skip_list->mask_xor != mask_xor){
    DBG("[!] c_crit_hack: mask xor changed, force recalculation (%i, %i) (%i, %i)\n", force_list->mask_xor, mask_xor, skip_list->mask_xor, mask_xor);

    force_list->filled    = skip_list->filled   = false;
  }

  if(force_list->nospread != settings->no_spread || skip_list->nospread != settings->no_spread){
    DBG("[!] c_crit_hack: user toggled nospread, force recalculation\n");

    force_list->filled    = skip_list->filled   = false;
  }

  if(force_list->filled && skip_list->filled)
    return;

  static bool is_crits_being_calculated = false;
  if(is_crits_being_calculated){
    DBG("[!] c_crit_hack::fill_crit_seed_list (Waiting for threads to finish)\n");
    return;
  }


  force_list->reset();
  skip_list->reset();

  force_list->nospread = settings->no_spread;
  skip_list->nospread  = settings->no_spread;

  force_list->thread_running = true;
  skip_list->thread_running  = true;

  c_good_seed_data* seed_data = malloc(sizeof(c_good_seed_data));
  accuracy->setup_good_seed_data(seed_data, weapon);

  s_crit_thread_data* thread_data = malloc(sizeof(s_crit_thread_data));
  {
    thread_data->seed_data  = seed_data;
    thread_data->mask_xor   = mask_xor;
    thread_data->force_list = force_list;
    thread_data->skip_list  = skip_list;
    thread_data->nospread   = settings->no_spread;
    thread_data->calc_taken = &is_crits_being_calculated;
  }

  is_crits_being_calculated = true;

  if(!utils::create_worker([](s_crit_thread_data* thread_data){
    if(thread_data->force_list != nullptr && thread_data->skip_list != nullptr){

      float begin_crit_gather_time = math::time();
      for(u32 i = 0; i < INT_MAX; i++){
        // Senator:
        // NOTE: we must do this or we will cause 2 threads to run and fuck up each others force, skip lists
        // Caused a huge bug causing crithack not to work because seeds where completely incorrect
        if(!thread_data->force_list->thread_running || !thread_data->skip_list->thread_running){
          DBG("[-] c_crit_hack::fill_crit_seed_list (force_list->thread_running or skip_list) condition\n");
          *thread_data->calc_taken = false;
          break;
        }

        float time_taken = math::abs(begin_crit_gather_time - math::time());
        if(i > 0 && time_taken > 20.f){
          DBG("[-] c_crit_hack::fill_crit_seed_list: Failing thread... Took 20s!! (%2.2fs)!! Command numbers checked: %i\n", time_taken, i);
          *thread_data->calc_taken = false;
          break;
        }
      
        // Rud:
        // This is done so when we have nospread we'll reduce the 'quality' of the seeds we create.
        // Even though, I should note that these values here are excessive as fuck and the ones used for nospread have had no problems when using them in Lithium.
        i32 best_crit_command     = 0;
        i32 best_non_crit_command = 9999;

        // Senator:
        // Instead of using is_command_number_crit or is_command_number_not_crit
        // Doing this is much much faster for iterations, around 2-3x faster
        u32 seed = math::md5_pseudo_random(i) & 0x7FFFFFFF;
        if(thread_data->nospread && thread_data->seed_data->valid){
          // Rud:
          // It took 2 minutes on my computer to do the demoman seeds.
          // With this it takes a second. If you're ADHD freaking out cause you got a fake crit.
          // This isn't the fucking cause u spaz. That shit has been happening for a while prior to messing with this.
          best_crit_command     = 100;
          best_non_crit_command = 5000;
          if(!accuracy->is_good_seed(seed, thread_data->seed_data))
            continue;
        }

        math::set_random_seed(seed ^ thread_data->mask_xor);
        i32 dice = math::random_int(0, 9999);

        if(!thread_data->force_list->filled && dice <= best_crit_command){
          thread_data->force_list->current_cmd() = i;
          thread_data->force_list->filled        = thread_data->force_list->set_next_current_cmd();
        }
        else if(!thread_data->skip_list->filled && dice >= best_non_crit_command){
          thread_data->skip_list->current_cmd() = i;
          thread_data->skip_list->filled        = thread_data->skip_list->set_next_current_cmd();
        }

        if(thread_data->force_list->filled && thread_data->skip_list->filled){
          DBG("[!] crit_hack (thread): found %i skip/force seeds for weapon\n", CRIT_HACK_MAX_CMDS);
          *thread_data->calc_taken = false;
          break;
        }
      }

      *thread_data->calc_taken                = false;
      thread_data->force_list->thread_running = false;
      thread_data->skip_list->thread_running  = false;
      thread_data->force_list->mask_xor       = thread_data->mask_xor;
      thread_data->skip_list->mask_xor        = thread_data->mask_xor;
    }
    else
      assert(false);

    free(thread_data->seed_data);
    free(thread_data);
    return 0;
  }, thread_data)){
    free(thread_data->seed_data);
    free(thread_data);
  }

  DBG("[!] crit_hack: dispatching thread for weapon %i...\n", weapon->get_index());
}

bool c_crit_hack::force(bool should_crit){
  if(!config->misc.crithack_enabled)
    return false;

  if(!should_crit && !config->misc.crithack_avoid_natural)
    return false;

  if(should_crit && !can_crit)
    should_crit = false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(!weapon->can_randomly_crit())
    return false;

  // This gamemode disables random crits.
  if(utils::is_playing_mannpower())
    return false;

  if(should_crit && crit_banned)
    should_crit = false;

  s_cmd_list* list = get_cmd_list(should_crit);
  assert(list != nullptr);

  if(list == nullptr || !list->filled)
    return false;

  i32 seed_entry = list->get_entry(weapon->current_seed() ^ get_weapon_mask_xor());

  list->set_next_current_cmd();

  if(seed_entry == list->cur_cmd)
    list->set_next_current_cmd();

  command_manager->set_command(list->current_cmd());

  // make the engine use this seed when we fire
  force_prediction_seed = math::md5_pseudo_random(global->current_cmd->command_number) & 0x7FFFFFFF;

  return true;
}

void c_crit_hack::createmove_run(){
  // reset the force prediction seed
  force_prediction_seed = 0;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(utils::is_safe_mode_enabled())
    return;

  resync_bucket();
  if(!localplayer->is_valid())
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  //if(acp->is_active())
  //  return;

  update_damages();
  fill_crit_seed_list();
  calculate_can_crit();

  if(can_crit && !crit_banned && input_system->held(config->misc.crithack_hotkey))
    force(true);
  else
    force(false);
}

// -- hooks --

EXPORT void __fastcall calc_is_attack_critical_hook(c_base_weapon* rcx){
  assert(global->prediction != nullptr);
  if(rcx == nullptr || !global->prediction->first_time_predicted)
    return;

  global->last_fired_weapon_id = rcx->get_id();

  // set prediction seed
  u32 old_prediction_seed = 0;
  if(crit_hack->force_prediction_seed != 0){
    old_prediction_seed             = *global->prediction_random_seed;
    *global->prediction_random_seed = crit_hack->force_prediction_seed;

#if defined(DEV_MODE) && defined(CRIT_HACK_DEBUG)
    DBG("[+] set prediction seed to %i\n", crit_hack->force_prediction_seed);
#endif

    crit_hack->force_prediction_seed = 0;
  }

  utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->calc_is_attack_critical_hook_trp), rcx);
  crit_hack->backup_crit_bucket();

  // restore prediction seeds
  if(old_prediction_seed != 0)
    *global->prediction_random_seed = old_prediction_seed;
}

// Fix minigun crit sound spam
EXPORT void __fastcall minigun_weapon_sound_update_hook(c_base_weapon* rcx){
  if(rcx == nullptr)
    return;

  c_base_player* localplayer = utils::localplayer();

  if(localplayer != nullptr){
    c_base_weapon* localplayer_wep = localplayer->get_weapon();

    if(localplayer_wep != nullptr && localplayer_wep == rcx)
      localplayer_wep->minigun_crit_shot() = localplayer_wep->crit_time() > math::ticks_to_time(localplayer->tick_base());
  }

  utils::call_fastcall64<void>(gen_internal->decrypt_asset(global->minigun_weapon_sound_update_hook_trp), rcx);
}

// The engine considers these values to be "predicted", predictioncopy will restore these to there original values
// For some reason only affects these values, to fix this problem we just dont let it copy the broken memory to these values
// Feb 13, 2025: Function has not been hooked due to being inlined in 64-bit, and I noticed no issues with crit-hack or anything, 
//EXPORT void __fastcall prediction_copy_int_hook(void* ecx, void* edx, u32 dt, i32* out, i32* in, i32 count){
//  if(out != nullptr && in != nullptr){
//    c_base_player* localplayer = utils::localplayer();
//
//    if(localplayer != nullptr){
//      c_base_weapon* weapon = localplayer->get_weapon();
//
//      // PROBLEM!!!
//      /* 
//        I assume that crit_checks, crit_seed_requests and current_seed are shared across every weapon.
//        But, current_attack_is_crit and minigun_crit_shot aren't. 
//        
//        Unlucky for us current_attack_is_crit happens to have the same offset as m_flChargedDamage. 
//        This results in our sniper charge being really inaccurate.
//
//        The solution to this was to simply only apply these checks if our weapon can randomly crit. But
//        also only perform a check if we're holding a weapon specific netvar. Such as the minigun_crit_shot.. Check if we're holding the
//        minigun.
//
//        - Rud
//      */
//      if(weapon != nullptr){
//        if(weapon->can_randomly_crit()){
//          if(out == &weapon->crit_checks() ||
//            out == &weapon->crit_seed_requests() ||
//            out == &weapon->current_seed() ||
//            out == &weapon->current_attack_is_crit() ||
//            weapon->is_minigun() && out == &weapon->minigun_crit_shot()){
//            return;
//          }
//        }
//      }
//
//      // Fix gravity getting fucked
//      if(out == &localplayer->gravity())
//        return;
//    }
//  }
//
//  utils::call_fastcall<void, u32, i32*, i32*, i32>(gen_internal->decrypt_asset(global->prediction_copy_int_hook_trp), ecx, edx, dt, out, in, count);
//}

EXPORT void* __fastcall cbase_combat_weapon_hook(c_base_weapon* rcx){
  assert(rcx != nullptr);

  void* ret = utils::call_fastcall64<void*>(gen_internal->decrypt_asset(global->cbase_combat_weapon_hook_trp), rcx);
  crit_hack->needs_bucket_resync = true;
  return ret;
}

EXPORT bool __fastcall can_fire_random_critical_shot_hook(c_base_weapon* rcx, float crit_chance){
  assert(rcx != nullptr);

  crit_hack->can_fire_random_critical_shot_run(rcx, crit_chance);
  crit_hack->update_damage_for_unban();

  return (crit_chance + 0.1f) >= rcx->observed_crit_chance();
}

EXPORT bool __fastcall is_allowed_to_withdraw_from_crit_bucket_hook(c_base_weapon* rcx, float damage){
  crit_hack->damage_correct = damage;
  return utils::call_fastcall64<bool, float>(gen_internal->decrypt_asset(global->is_allowed_to_withdraw_from_crit_bucket_hook_trp), rcx, damage);
}

EXPORT void __fastcall add_to_crit_bucket_hook(c_base_weapon* rcx, float amount){
  crit_hack->bucket_add_per_shot = amount;

  assert(global->tf_weapon_criticals_bucket_cap  != nullptr);
  rcx->crit_token_bucket() = math::smallest(global->tf_weapon_criticals_bucket_cap->flt, rcx->crit_token_bucket() + amount);
}

// test hook
#if defined(DEV_MODE)
EXPORT bool __fastcall server_calc_is_attack_critical_hook(void* ecx, void* edx){
 // bool result = utils::call_fastcall<bool>(gen_internal->decrypt_asset(global->server_calc_is_attack_critical_hook_trp), ecx, edx);

#if defined(DEV_MODE) && defined(CRIT_HACK_DEBUG)
  //DBG("\n[!] SERVER CALCISATTACKCRITICAL\n");
  //DBG("-------------------------\n");
  //DBG("Crit bucket: %f\n", *(float*)((uptr)ecx + 0x594));
  //DBG("Crit checks: %i\n", *(i32*)((uptr)ecx + 0x598));
  //DBG("Crit requests: %i\n", *(i32*)((uptr)ecx + 0x59C));
  //DBG("Observed Crit Chance: %f\n", *(float*)((uptr)ecx + 0x7BC));
#endif

 // return result;
}
#endif