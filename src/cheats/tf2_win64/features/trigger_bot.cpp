#include "../link.h"

CLASS_ALLOC(c_trigger_bot, trigger_bot);

bool c_trigger_bot::has_valid_weapon(c_base_player* localplayer, c_base_weapon* weapon){
  if(localplayer == nullptr || weapon == nullptr)
    return false;

  if(weapon->is_harmless_item() || weapon->is_projectile_weapon() || weapon->is_medigun() || weapon->is_melee() || weapon->is_wrangler())
    return false;

  if(weapon->weapon_id() == WPN_Machina && !localplayer->is_scoped())
    return false;

  // Do not do triggerbot unless the weapon is being deployed.
  if(weapon->is_minigun() && !(global->current_cmd->buttons & IN_ATTACK2))
    return false;
  

  ctf_weapon_info* weapon_info = weapon->get_weapon_info();
  if(weapon_info == nullptr)
    return false;

  if(weapon_info->range <= 0)
    return false;

  return true;
}

void c_trigger_bot::run(){
  if(!config->triggerbot.enabled)
    return;

  if(utils::is_truce_active())
    return;

  // TEMP.
  config->automation.trigger_bot_delay         = 0;
  config->automation.trigger_bot_fire_for_time = 304;

  global->info_panel->add_entry(INFO_PANEL_TRIGGERBOT, WXOR(L"INACTIVE"));
  if(config->triggerbot.use_key){
    static c_key_control key(&config->triggerbot.key, &config->triggerbot.key_held, &config->triggerbot.key_toggle, &config->triggerbot.key_double_click);
    if(!key.is_toggled())
      return;
  }

  float time = math::time();

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(!localplayer->is_valid())
    return;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return;

  ctf_weapon_info* weapon_info = weapon->get_weapon_info();
  if(weapon_info == nullptr)
    return;

  if(!has_valid_weapon(localplayer, weapon))
    return;


  global->info_panel->add_entry(INFO_PANEL_TRIGGERBOT, WXOR(L"ACTIVE"));

  if(fire_for_time >= time){
    global->info_panel->add_entry(INFO_PANEL_TRIGGERBOT, WXOR(L"FIRING FOR TIME"), colour(255, 0, 0));
    global->current_cmd->buttons |= IN_ATTACK;
  }

  vec3 fwd;
  math::angle_2_vector(global->untouched_cmd.view_angles, &fwd, nullptr, nullptr);

  vec3 start_pos = localplayer->shoot_pos();
  vec3 end_pos   = (fwd * weapon_info->range) + start_pos;

  s_trace trace  = global->trace->ray(start_pos, end_pos, mask_bullet);

  // Sanity check our hit entity.
  if(!is_valid_entity(localplayer, trace.hit_entity)){
    if(config->triggerbot.show_trigger_bot_traces)
      render_debug->draw_3dbox(trace.end, colour(255, 255, 255, 32), 1.f);

    if(found_target){
      found_target = false;
      if(config->automation.trigger_bot_fire_for_time > 0 && trigger_delay <= time){ // Only set the fire for time if the trigger_delay is less than our current time.
        fire_for_time = time + (float)(config->automation.trigger_bot_fire_for_time / 1000);
        trigger_delay = -1.f;
      }
    }

    return;
  }

  if(config->triggerbot.show_trigger_bot_traces)
    render_debug->draw_3dbox(trace.end, colour(255, 0, 0, 32), 1.f);
  
  if(!process_hitbox(localplayer, trace.hit_entity, trace.hitbox))
    return;

  // Reset fire_for_time.
  fire_for_time = -1.f;

  // Setup trigger bot delay.
  if(!found_target){
    found_target = true;
    if(config->automation.trigger_bot_delay > 0){
      trigger_delay = time + (float)(config->automation.trigger_bot_delay / 1000);
      return;
    }
  }

  // Don't run bro we have a delay!
  if(trigger_delay >= time){
    global->info_panel->add_entry(INFO_PANEL_TRIGGERBOT, WXOR(L"FIRE DELAY"), colour(255, 255, 0));
    return;
  }

  c_base_entity* tr_entity = (c_base_entity*)trace.hit_entity;

  global->info_panel->add_entry(INFO_PANEL_TRIGGERBOT, WXOR(L"FIRING"), colour(255, 0, 0));
  if(tr_entity->is_player())
    interp_manager->set_tick_count_with_interp(trace.hit_entity->simulation_time(), trace.hit_entity->get_index(), false);

  global->current_cmd->buttons |= IN_ATTACK;
}

bool c_trigger_bot::is_valid_entity(c_base_player* localplayer, c_base_entity* entity){
  if(localplayer == nullptr || entity == nullptr)
    return false;

  if(localplayer->team() == entity->team())
    return false;

  if(entity->is_dormant())
    return false;

  if(entity->is_player() && !utils::is_truce_active()){

    c_base_player* p = (c_base_player*)entity;
    if(!p->is_valid())
      return false;

    if(p->is_steam_friend() && !config->triggerbot.shoot_friends)
      return false;

    if(p->is_cloaked() && !config->triggerbot.shoot_cloaked)
      return false;

    if(p->is_disguised() && !config->triggerbot.shoot_disguised)
      return false;

    if(p->is_ubercharged() && !config->triggerbot.shoot_uber || p->is_bonked() && !config->triggerbot.shoot_bonked)
      return false;

    u32 player_class = p->player_class();
    if(player_class >= 1 && player_class <= 9){
      if(config->triggerbot.ignore_player_class[player_class])
        return false;
    }

    return true;
  }
  else{
    if(entity->is_sentry() && config->triggerbot.shoot_sentry && !utils::is_truce_active() || entity->is_dispenser() && config->triggerbot.shoot_dispenser && !utils::is_truce_active() || entity->is_teleporter() && config->triggerbot.shoot_teleporter && !utils::is_truce_active())
      return true;
    else if(entity->is_boss() && config->triggerbot.shoot_boss){
      if(entity->team() > 0 && entity->team() <= 3 && !entity->is_tank_boss())
        return false;

      return true;
    }
    else if(entity->is_stickybomb() && config->triggerbot.shoot_sticky && !utils::is_truce_active()){
      if(!entity->is_sticky_onsurface())
        return false;

      if(localplayer->is_scoped())
        return false;

      return true;
    }
  }

  return false;
}

bool c_trigger_bot::process_hitbox(c_base_player* localplayer, c_base_entity* entity, i32 hitbox){
  if(localplayer == nullptr || entity == nullptr || hitbox == -1)
    return false;

  if(!entity->is_player())
    return true;

  // Stop people from breaking settings.
  if(config->triggerbot.hitbox_ignore_body && config->triggerbot.hitbox_ignore_limbs && config->triggerbot.hitbox_ignore_head)
    return true;

  bool is_head = (hitbox == 0);
  bool is_body = (hitbox >= 1 && hitbox <= 4);
  if(config->triggerbot.hitbox_ignore_limbs && !is_head && !is_body)
    return false;

  if(config->triggerbot.hitbox_ignore_body && is_body)
    return false;

  if(config->triggerbot.hitbox_ignore_head && is_head)
    return false;


  return true;
}