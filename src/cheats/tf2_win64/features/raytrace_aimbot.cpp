#include "../link.h"

bool c_raytrace_aimbot::should_target_entity(c_base_entity* entity){
  if(global->aimbot_settings == nullptr)
    return false;

  if(entity == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return false;

  i32 flags = get_target_flags(entity);

  bool truce_active = utils::is_truce_active();
  if(entity->is_player()){
    if(truce_active) // We can't damage players during this.
      return false;

    c_base_player* player = (c_base_player*)entity;
    if(!player->is_valid())
      return false;

    if(!localplayer->is_considered_enemy(player))
      return false;

    if(player->is_ubercharged() || player->is_bonked())
      return false;

    if(flags & RT_AIMBOT_IGNORE_MEDIC_VACCINATOR || flags & RT_AIMBOT_PLAYERLIST_IGNORE_AIMBOT)
      return false;

    c_player_data* data = player->data();
    if(data == nullptr)
      return false;

    // Aimbot only mode enabled.
    if(global->aimbot_settings->aimbot_only_enabled){
      if(!global->aimbot_settings->aimbot_only_use_key || input_system->held(global->aimbot_settings->aimbot_only_key)){

        bool fail = true;
        if(global->aimbot_settings->aimbot_only_prioritized && (data->playerlist.priority_mode && data->playerlist.priority > 0))
          fail = false;
        else if(global->aimbot_settings->aimbot_only_cheaters && cheat_detection->is_cheating(player->get_index()))
          fail = false;
        else if(global->aimbot_settings->aimbot_only_bots && cheat_detection->is_cheating_bot(player->get_index()))
          fail = false;

        if(fail)
          return false;
      }
    }

    return true;
  }
  else{
    if(entity->is_dormant())
      return false;

    if(entity->team() == localplayer->team())
      return false;

    if(entity->is_boss() && global->aimbot_settings->target_boss)
      return true;

    // This happens in the special halloween maps. We can no longer damage any enemy buildings or stickies.
    if(truce_active)
      return false;

    if(entity->is_sentry() && global->aimbot_settings->target_sentries || entity->is_dispenser() && global->aimbot_settings->target_dispenser || entity->is_teleporter() && global->aimbot_settings->target_teleporter)
      return entity->object_health() > 0;
    else if(entity->is_stickybomb() && global->aimbot_settings->target_sticky){
      if(!entity->is_sticky_onsurface()) // Stickies are immune to damage in air.
        return false;

      // Don't target stickies with the sniper rifle.
      if(wep->is_sniper_rifle())
        return false;

      return true;
    }
  }

  return false;
}

i32 c_raytrace_aimbot::get_priority_hitbox(){
  if(global->aimbot_settings == nullptr)
    return -1;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return -1;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return -1;

  i32 priority_hitbox = -1;
  for(i32 i = 0; i < sizeof(global->aimbot_settings->priority_hitbox) / sizeof(bool); i++){
    if(global->aimbot_settings->priority_hitbox[i]){
      priority_hitbox = i;
      break;
    }
  }

  // Error correct settings.
  if(!global->aimbot_settings->double_tap_aim_point_pelvis && global->aimbot_settings->double_tap_aim_point_stomach)
    global->aimbot_settings->double_tap_aim_point_pelvis = true;

  bool force_dt_safe_point = global->aimbot_settings->double_tap_aim_point;
  bool allow_custom_points = true;

  // Allow users to prefer a hitbox during dt.
  if(force_dt_safe_point){
    if(double_tap->is_dt_active()){
      priority_hitbox = 1;
      if(allow_custom_points){
        if(global->aimbot_settings->double_tap_aim_point_stomach)
          priority_hitbox = 2;
      }
    }
  }

  // Never aim at the head with something like an SMG
  if(priority_hitbox == 0 && localplayer->is_player_class(TF_CLASS_SNIPER) && !wep->is_sniper_rifle())
    return 1;

  return priority_hitbox;
}

// Somethings in here are called in is_considered_enemy and wouldn't be replaced from there to here. 
// 'Some' logic will be added in here solely for 'usefulness'
// Call is not thread safe.
i32 c_raytrace_aimbot::get_target_flags(c_base_entity* entity){
  if(global->aimbot_settings == nullptr)
    return 0;
  
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return 0;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return 0;

  if(entity == nullptr)
    return 0;

  bool has_sniper_rifle = localplayer->is_player_class(TF_CLASS_SNIPER) && wep->is_sniper_rifle();
  bool has_ambassador   = localplayer->is_player_class(TF_CLASS_SPY) && localplayer->can_spy_headshot();

  i32 flags = 0;
  if(entity->is_player()){
    c_base_player* player = entity->get_player();
    if(player == nullptr)
      return flags;

    if(!player->is_valid())
      return flags;

    c_base_weapon* player_weapon = player->get_weapon();

    c_player_data* data = player->data();
    if(data == nullptr)
      return flags;

    // This player is ignored!
    if(data->playerlist.ignore_player)
      flags |= RT_AIMBOT_PLAYERLIST_IGNORE_AIMBOT;

    // Ignore vaccinator.
    if(global->aimbot_settings->sniper_ignore_vaccinator && has_sniper_rifle && player->is_bullet_charge())
      flags |= RT_AIMBOT_IGNORE_MEDIC_VACCINATOR;

    if(player_weapon != nullptr){
      // Try and detect medic DT baiting or shot baiting in general.
      /* I've noticed people will try and kill the medic and then change their minds and start shooting the heavy again. The game's code
          makes the medic gain alot of passive heal rate when their heal target is being damaged. 
    
          So due to the 'user' swapping between targets usually results in a lost fight. While I think heavy 2v2 are decided on how the good the medic is rather than the cheats at play.
          Optimizing the aimbot behind the scenes to remove bad gameplay tactics is simply just a good idea.
          
          // Make sure they aren't healing another medic to avoid a case of ignoring both medics.
      */
      if(player_weapon->is_vaccinator()){
        c_base_entity* heal_target = player_weapon->get_heal_target();
        if(heal_target != nullptr){

          c_base_player* heal_player = heal_target->get_player();
          if(heal_player != nullptr){

            if(!heal_player->is_player_class(TF_CLASS_MEDIC)){

              if(player->health()> 20){ // If the medic has more than 20 HP then lets avoid targeting them.
                if(wep->is_minigun() && player->is_bullet_charge())
                  flags |= RT_AIMBOT_IGNORE_MEDIC_VACCINATOR;
              }
            }
          }
        }
      }
    }

    // Force body aim on key.
    if(global->aimbot_settings->force_body_aim_on_key){
      if(input_system->held(global->aimbot_settings->force_body_aim_key))
        flags |= (RT_AIMBOT_FORCE_BODY_AIM | RT_AIMBOT_FORCE_BODY_AIM_MANUAL);
    }

     // If the target is crit immune. This will mean we're unable to headshot them.
    if(global->aimbot_settings->sniper_body_aim_if_crit_immune && has_sniper_rifle){
      if(player->has_condition(TF_COND_DEFENSEBUFF_HIGH) || player->has_condition(TF_COND_DEFENSEBUFF) || player->is_bullet_charge())
        flags |= RT_AIMBOT_FORCE_BODY_AIM;
    }

    // Force the priority hitbox to body aim when not scoped. (Make this work on non players once we start using hitboxes for non players)
    if(has_sniper_rifle && !localplayer->is_scoped() && wep->charged_damage() <= 0.f && !(wep->is_classic_sniper_rifle() && wep->charged_damage() >= 150.f))
      flags |= (RT_AIMBOT_FORCE_BODY_AIM | RT_AIMBOT_FORCE_BODY_AIM_SILENT);
    
    // If we are a spy and we can body shot with the ambassador. Then set the priority hitbox to the pelvis.
    if(global->aimbot_settings->spy_ambassador_body_aim_if_lethal && has_ambassador){
      if(localplayer->is_crit_boosted() || (i32)utils::get_damage(localplayer, localplayer->shoot_pos(), player->obb_center()) >= player->health())
        flags |= (RT_AIMBOT_FORCE_BODY_AIM | RT_AIMBOT_FORCE_BODY_AIM_AMBY);
    }
  }

  if(global->aimbot_settings->sniper_body_aim_if_lethal && has_sniper_rifle || !entity->is_player() && has_sniper_rifle){
    float charge_damage = localplayer->get_sniper_charge_damage(entity, false);
    if(charge_damage >= entity->get_health())
      flags |= RT_AIMBOT_FORCE_BODY_AIM;
  }

  return flags;
}

bool c_raytrace_aimbot::should_use_hitbox(i32 hitbox){
  if(global->aimbot_settings == nullptr)
    return false;

  if(hitbox > sizeof(global->aimbot_settings->hitbox) / sizeof(bool))
    return false;

  return global->aimbot_settings->hitbox[hitbox];
}

// We need a function get hitboxes for sentries. So, for now no point scale for non players.
bool c_raytrace_aimbot::get_other_aim_points(i32 hitbox, vec3 shoot_pos, c_base_entity* target, s_lc_record* lc_record, vec3& aim_point){
  // Only run on priority hitbox.
  if(hitbox != get_priority_hitbox())
    return false;

  // Do not run when attempting to DT
  if(double_tap->is_dt_active())
    return false;

  if(global->aimbot_settings == nullptr)
    return false;

  if(!global->aimbot_settings->point_scale_enabled)
    return false;

  global->aimbot_settings->point_scale = math::clamp(global->aimbot_settings->point_scale, 1.f, 100.f);
  if(target == nullptr || lc_record == nullptr)
    return false;

  vec3 p[8];
  if(!lc_record->get_hitbox_points(target, hitbox, global->aimbot_settings->point_scale / 100.f, p))
    return false;

  for(i32 i = 0; i < sizeof(p) / sizeof(vec3); i++){
    s_trace tr = global->trace->ray(shoot_pos, p[i], mask_bullet);

    // Point on entity not visible.
    if(!tr.vis(target))
      continue;

    aim_point = p[i];
    return true;
  }

  return false;
}
  
bool c_raytrace_aimbot::get_best_hitbox_point(i32 hitbox, vec3 shoot_pos, c_base_entity* target, s_lc_record* lc_record, vec3& aim_point){
  if(target == nullptr)
    return false;

  if(target->is_player()){
    if(lc_record == nullptr)
      return false;

    if(!lc_record->get_hitbox_pos((c_base_player*)target, hitbox, aim_point))
      return false;
  }
  else if(target->is_sentry() || target->is_dispenser() || target->is_teleporter()){
    // This has its own little handling done here.

    for(i32 i = target->is_sentry() ? 1 : 0; i < target->get_max_hitbox_count(); i++){
      if(!target->get_hitbox_pos(i, &aim_point))
        continue;

      if(global->aimbot_settings->max_fov > 0 && global->aimbot_settings->max_fov < 180)
        if(math::get_pixel_fov_delta(aim_point) > (float)global->aimbot_settings->max_fov)
          continue;
    
      s_trace tr = global->trace->ray(shoot_pos, aim_point, mask_bullet);
      if(tr.vis(target))
        return true;

      if(i == 1 && target->is_sentry() && double_tap->is_dt_active() || !i && !target->is_sentry() && double_tap->is_dt_active())
        return false;
    }

    return false;
  }
  else // World objects for now aim at OBB center.
    aim_point = target->obb_center();

  // Are they in our fov?
  if(global->aimbot_settings->max_fov > 0 && global->aimbot_settings->max_fov < 180)
    if(math::get_pixel_fov_delta(aim_point) > (float)global->aimbot_settings->max_fov)
      return false;

  s_trace tr = global->trace->ray(shoot_pos, aim_point, mask_bullet);
  if(!tr.vis(target)){
    if(get_other_aim_points(hitbox, shoot_pos, target, lc_record, aim_point))
      return true;

    return false;
  }

  vec3 org_aim_point = aim_point;

  // Height adjustment for head hitbox.
  if(hitbox == 0){
    if(target->is_player() && tr.hitbox != 0 && tr.hit_group != hitgroup_generic){
      while(hitbox == 0 && tr.hitbox != 0 && tr.hit_entity != nullptr && tr.hit_entity == target){
        aim_point.z += 1.f;
        tr = global->trace->ray(shoot_pos, aim_point, mask_bullet);

        // We are no longer over a valid hitgroup or lost sight of the position we're going to.
        if(tr.hit_group <= hitgroup_generic || !tr.vis(target)){
          aim_point = org_aim_point; // Restore original position.
          break;
        }

        // Ensure we got the hitbox we wanted and we have a valid hit group.
        if(tr.hitbox == 0 && tr.hit_group > hitgroup_generic)
          break;
      }
    }
  }

  return true;
}

bool c_raytrace_aimbot::get_best_aim_point(c_base_entity* target, s_lc_record* lc_record, vec3& aim_point){
  if(target == nullptr || global->aimbot_settings == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return false;

  vec3 shoot_pos      = localplayer->shoot_pos();
  i32 priority_hitbox = get_priority_hitbox();

  if(priority_hitbox == -1)
    return false;

  i32 flags = get_target_flags(target);
  if(flags & RT_AIMBOT_FORCE_BODY_AIM_MANUAL)
    global->info_panel->add_entry(INFO_PANEL_AIMBOT, WXOR(L"ACTIVE"), colour(255, 255, 0));

  bool has_sniper_rifle = localplayer->is_player_class(TF_CLASS_SNIPER) && wep->is_sniper_rifle();
  bool has_ambassador   = localplayer->is_player_class(TF_CLASS_SPY) && localplayer->can_spy_headshot();

  bool ignore_priority_charge_test = false;
  bool ignore_wait_for_spread_test = (flags & RT_AIMBOT_FORCE_BODY_AIM_AMBY);
  if(flags & RT_AIMBOT_FORCE_BODY_AIM){
    priority_hitbox             = 1;
    ignore_priority_charge_test = true;
  }

  // Conditional checks that are player only.
  if(target->is_player()){
    c_base_player* target_player = target->get_player();

    if(priority_hitbox == 0){ // Only run when attempting to headshot.
      if(global->aimbot_settings->spy_ambassador_wait_for_spread && !ignore_wait_for_spread_test){
        if(has_ambassador){
          if(wep->get_spread() > 0.f)
            return false;
        }
      }
    }

    for(i32 i = -1; i < 4; i++){
      if(priority_hitbox < 0 || i == priority_hitbox || i >= 0 && !should_use_hitbox(i))
        continue;

      i32 target_hitbox = i == -1 ? priority_hitbox : i;
      if(target_hitbox != priority_hitbox && double_tap->is_dt_active())
        return false;

      if(has_sniper_rifle && (!ignore_priority_charge_test || target_hitbox != priority_hitbox) && wep->charged_damage() < 150.f){
        float charge_damage = localplayer->get_sniper_charge_damage(target, target_hitbox == 0);

        if(global->aimbot_settings->sniper_wait_for_charge && charge_damage < target->health())
          continue;

        // 150 < 50 && !headshot
        if(global->aimbot_settings->sniper_wait_for_headshot){
          if(target_hitbox == 0){
            if(charge_damage < target->health() && !(localplayer->can_headshot()))
              continue;
          }
          else if(charge_damage < target->health())
            continue;
        }
      }

      if(get_best_hitbox_point(target_hitbox, shoot_pos, target, lc_record, aim_point)){
        aim_target_hitbox = target_hitbox;
        return true;
      }
    }
  }
  else{
    // Currently we bounding boxes for world objects so make the target hitbox -1 and lag record a nullptr
    if(get_best_hitbox_point(-1, shoot_pos, target, nullptr, aim_point)){
      aim_target_hitbox = -1;
      return true;
    }
  }

  return false;
}

bool c_raytrace_aimbot::get_target(c_base_entity*& target, s_lc_record*& lc_record, vec3& aim_point){
  if(global->aimbot_settings == nullptr)
    return false;

  c_base_player* localplayer = utils::localplayer();

  if(localplayer == nullptr)
    return false;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return false;

  set_aimbot_filter();

  if(filter == nullptr)
    return false;

  bool playing_mvm = utils::is_playing_mvm();
  {
    for(u32 i = target_list->get_loop_start_index(); i <= target_list->get_loop_end_index(); i++){
      i32 index = target_list->get_sorted_target_index(i);
      if(index == global->engine->get_local_player())
        continue;

      c_base_entity* entity = global->entity_list->get_entity(index);
      if(entity == nullptr)
        continue;

      if(!should_target_entity(entity))
        continue;

      // If we have a valid lock target then only process this entity.
      if(aim_lock_target != nullptr && aim_lock_target != entity)
        continue;

      if(entity->is_player()){
        c_base_player* player = (c_base_player*)entity;

        c_player_data* data = player->data();
        if(data == nullptr)
          continue;

        if(playing_mvm && player->info().fake_player){
          if(global->aimbot_settings->mvm_ignore_sentrybuster && player->is_sentry_buster())
            continue;

          if(global->aimbot_settings->mvm_ignore_healed && player->healers() > 0)
            continue;
        }

        s_lc_data* lc = lag_compensation->get_data(player);
        if(lc == nullptr)
          continue;


        float closest_dist = -1.f;
        vec3  shoot_pos    = localplayer->shoot_pos();

        for(u32 ii = 0; ii < lc->max_records; ii++){
          s_lc_record* record = lc->get_record(ii);

          if(record == nullptr)
            continue;

          if(!record->valid())
            continue;

          // If we dont want to use history, then only use the first valid tick
          if(!acm->can_back_track() || !global->aimbot_settings->position_adjust_history)
            ii = lc->max_records;

          if(!filter->sanity(record, entity))
            continue;

          if(!filter->handle(record, entity))
            continue;

          if(!wep->is_sniper_rifle()){
            float dist = shoot_pos.distance(record->obb_center);

            if(closest_dist > 0.f && dist >= closest_dist)
              continue;

            if(!filter->store(record, entity))
              continue;

            closest_dist = dist;
          }
          else if(wep->is_sniper_rifle()){
            float fov = math::get_fov_delta(data->networked_ang, player->shoot_pos(), localplayer->shoot_pos());

            // If the resolver is active, then we'll check to see if records past the first one are good to use or not.
            // That way we can stop ourselves from using old unresolved angles or old resolver angles.
            if(resolver->is_resolver_active(player)){
              if(!resolver->is_good_record(player, record) && ii > 0)
                continue;
            }
            else{
              if(closest_dist > 0.f && fov >= closest_dist)
                continue;
            }
                  
            if(!filter->store(record, entity))
              continue;

            closest_dist = fov;
          }
          else{
            if(filter->store(record, entity))
              break;
          }
        }
      }
      else{
        if(!filter->sanity(nullptr, entity))
          continue;

        if(!filter->handle(nullptr, entity))
          continue;

        if(!filter->store(nullptr, entity))
          continue;
      }
    }

    aim_point = filter->aim_point;
    target    = filter->target_ent;
    lc_record = filter->lc_record;
  }

  return target != nullptr;
}

bool c_raytrace_aimbot::crithack_force_wrapper(bool b){
  return crit_hack->force(b);
}

bool c_raytrace_aimbot::run(){
  //if(double_tap->charged_ticks != 0 && !double_tap->can_weapon_dt())
  //  return false;

  aim_target        = nullptr;
  aim_target_hitbox = -1;

  if(global->aimbot_settings == nullptr)
    return false;

  if(!global->aimbot_settings->enabled)
    return false;

  if(target_list->is_being_spectated() && global->aimbot_settings->disable_on_spectated)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(localplayer->is_bonked() || localplayer->is_taunting())
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  // Don't run on non hitscan / harmless weapons.
  if(!weapon->is_hitscan_weapon() || weapon->is_harmless_item())
    return false;

  if(!global->aimbot_settings->silent_aim_none && !localplayer->can_fire() && !weapon->is_minigun())
    return false;

  if(global->aimbot_settings->aim_mode_on_key && !input_system->held(global->aimbot_settings->aim_hotkey) || global->aimbot_settings->aim_mode_on_attack && !(global->current_cmd->buttons & IN_ATTACK))
    return false;


  bool scoped_only = global->aimbot_settings->sniper_scoped_only;
  if(config->automation.auto_rezoom && !scoped_only){
    scoped_only = config->automation.auto_rezoom_when_moving ?
    !(global->current_cmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT | IN_JUMP | IN_DUCK)) : true;
  }

  // If we're using a sniper rifle and we're not scoped in with scoped only mode enabled then return.
  if(weapon->is_sniper_rifle() && !localplayer->is_scoped()){
    if(scoped_only){
      global->info_panel->add_entry(INFO_PANEL_AIMBOT, WXOR(L"SCOPED ONLY MODE"), INFO_PANEL_WARNING_CLR);
      return false;
    }
  }

  if(weapon->is_minigun() && global->aimbot_settings->minigun_auto_rev)
    global->current_cmd->buttons |= IN_ATTACK2;

  if(weapon->is_classic_sniper_rifle()){
    if(weapon->charged_damage() > 0.f)
      global->current_cmd->buttons |= IN_ATTACK;
  }

  if(!double_tap->is_shift_tick())
    aim_lock_target = nullptr;

  vec3            aim_point;
  c_base_entity*  target = nullptr;
  s_lc_record*    record = nullptr;

  if(!get_target(target, record, aim_point)){
    aim_lock_target = nullptr;
    return false;
  }

  global->on_aimbot_target(target->get_index(), aim_point, weapon);

  // Wait!
  if(utils::wait_lag_fix(target)){
    global->info_panel->add_entry(INFO_PANEL_AIMBOT,  WXOR(L"ACCURATE UNLAG ACTIVE"), INFO_PANEL_WARNING_CLR);
    return false;
  }

  vec3 aim_angle = math::calc_view_angle(localplayer->shoot_pos(), aim_point);

  if(global->aimbot_settings->spread_manipulation && global->aimbot_settings->spread_manipulation_hit_chance > 0.f && (!global->aimbot_settings->no_spread || utils::is_match_making_server())){
    vec3 test_angle = math::calc_view_angle(localplayer->shoot_pos(), target->obb_center());
    if(utils::get_weapon_hit_chance(target, test_angle) < (global->aimbot_settings->spread_manipulation_hit_chance / 100.f))
      return false;
    
  }

  aim_target = target;

  bool will_fire = false;

  if(global->aimbot_settings->auto_shoot || !global->aimbot_settings->aim_mode_on_key)
    global->current_cmd->buttons |= IN_ATTACK;
  else{
    if(!global->aimbot_settings->smoothing || !global->aimbot_settings->silent_aim_none){
      if(global->aimbot_settings->aim_mode_on_key && !(global->current_cmd->buttons & IN_ATTACK))
        return false;
    }
  }

  // Support for the classic sniper rifle.
  if(weapon->is_classic_sniper_rifle()){
    if(weapon->charged_damage() >= 150.f || (weapon->charged_damage() * 0.90f) >= (float)target->get_health()) // Require a full charge todo headshot damage.
      global->current_cmd->buttons &= ~IN_ATTACK;
    else{
      // Allow people to trigger it manually by pressing IN_ATTACK if aim mode is not on attack. Otherwise trigger it if they stop attacking.
      if(weapon->charged_damage() > 0.f && global->original_cmd.buttons & IN_ATTACK && !global->aimbot_settings->aim_mode_on_attack || weapon->charged_damage() > 0.f && !(global->original_cmd.buttons & IN_ATTACK) && global->aimbot_settings->aim_mode_on_attack){
        global->current_cmd->buttons &= ~IN_ATTACK;
        // Let the code run.
      }
      else{
        // Prevent scoping in. Makes you shoot.
        global->current_cmd->buttons &= ~IN_ATTACK2;
        return false;
      }
    }
  }

  // set our viewangle
  {
    if(global->aimbot_settings->silent_aim_none && global->aimbot_settings->smoothing > 0)
      aim_angle = math::lerp_angle(global->engine->get_view_angles(), aim_angle, math::clamp(1.f - (global->aimbot_settings->smoothing / 100.f), 0.f, 1.f) );

    if(!weapon->is_minigun() || localplayer->will_fire_this_tick()){
      global->current_cmd->view_angles = aim_angle;

      // mark this tick as a fire
      will_fire = true;
    }

    if(global->aimbot_settings->silent_aim_none)
      global->engine->set_view_angles(aim_angle);
  }

  // Maybe we could avoid doing crit hack on crit immune targets? Or no?
  bool disallow_crits = false;

  if(target->is_player()){
    interp_manager->set_tick_count_with_interp(record != nullptr ? record->simulation_time : target->simulation_time(), target->get_index(), record != nullptr);
  }
  else{
    if(!target->is_boss()) // We can deal crit damage to bosses.
      disallow_crits = true;
  }

  // don't allow double tap to function if we aren't aiming at a player, sentry or boss. Or we point scaled them.
  if(!target->is_player() && !target->is_sentry() && !target->is_boss())
    global->allow_double_tap = false;

  if(disallow_crits)
    crithack_force_wrapper(false);

  if(double_tap->is_shift_tick())
    aim_lock_target = target;

  return will_fire;
}

CLASS_ALLOC(c_raytrace_aimbot, raytrace_aimbot);