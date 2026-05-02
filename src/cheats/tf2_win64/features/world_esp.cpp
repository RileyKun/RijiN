#include "../link.h"

CLASS_ALLOC(c_world_esp, world_esp);

void c_world_esp::precache(){
  static bool init = false;
  if(init)
    return;

  init = true;

  DBG("[!] c_world_esp::precache\n");
  memset(_str, 0, sizeof(_str));

  #define ASSIGN_BUFFER(i, x) formatW(_str[i], WXOR(x));

  // Since this function is only called once, might as well XOR everything.
  for(u32 i = 0; i <= XOR32(PLAYER_ESP_MAX_DIST_CACHE); i++)
    formatW(_dist[i], WXOR(L"[ %im ]"), i); // Create a string cache of all possible distances.

  for(u32 i = 0; i < XOR32(300); i++)
    formatW(_intel_returns_time[i], WXOR(L"RETURNS IN %i SEC"), i);
  
  for(u32 i = 0; i <= XOR32(3); i++)
    formatW(_building_level[i], WXOR(L"LEVEL %i"), i);

  ASSIGN_BUFFER(DISPENSER_TEXT, L"Dispenser");
  ASSIGN_BUFFER(SENTRY_TEXT, L"Sentry");
  ASSIGN_BUFFER(MINI_SENTRY_TEXT, L"Mini-Sentry");
  ASSIGN_BUFFER(DISPENSER_TEXT, L"Dispenser");
  ASSIGN_BUFFER(TELEPORTER_ENTRANCE_TEXT, L"Teleporter Entrance");
  ASSIGN_BUFFER(TELEPORTER_EXIT_TEXT, L"Teleporter Exit");
  ASSIGN_BUFFER(OBJECTIVE_TEXT, L"Objective");
  ASSIGN_BUFFER(AMMOPACK_TEXT, L"Ammopack");
  ASSIGN_BUFFER(HEALTHPACK_TEXT, L"Healthpack");
  ASSIGN_BUFFER(LUNCHABLE_TEXT, L"Lunchable");
  ASSIGN_BUFFER(BOSS_TEXT, L"Boss");
  ASSIGN_BUFFER(MONEY_TEXT, L"Money");
  ASSIGN_BUFFER(BOMB_TEXT, L"Bomb");
  ASSIGN_BUFFER(REVIVE_MARKER_TEXT, L"Revive marker");
  ASSIGN_BUFFER(HALLOWEEN_GIFT_TEXT, L"Halloween gift");
  ASSIGN_BUFFER(HALLOWEEN_PICKUP_TEXT, L"Halloween pickup");
  ASSIGN_BUFFER(NORMAL_SPELLBOOK_TEXT, L"Normal spellbook");
  ASSIGN_BUFFER(RARE_SPELLBOOK_TEXT, L"Rare spellbook");
  ASSIGN_BUFFER(DORMANT_TEXT, L"DORMANT");

  ASSIGN_BUFFER(CONTROLLED_TEXT, L"CONTROLLED");
  ASSIGN_BUFFER(SAPPED_TEXT, L"SAPPED");
  ASSIGN_BUFFER(DISABLED_TEXT, L"DISABLED");
  ASSIGN_BUFFER(CONSTRUCTING_TEXT, L"CONSTRUCTING");
  ASSIGN_BUFFER(SENTRY_NO_AMMO_TEXT, L"NO AMMO");
  ASSIGN_BUFFER(INTEL_HOME_TEXT, L"HOME");
  ASSIGN_BUFFER(INTEL_STOLEN_TEXT, L"STOLEN");
  ASSIGN_BUFFER(INTEL_DROPPED_TEXT, L"DROPPED");
  ASSIGN_BUFFER(CRITICAL_TEXT, L"CRITICAL");
  ASSIGN_BUFFER(REFLECTED_TEXT, L"REFLECTED");
}

u32 c_world_esp::get_object_index(c_base_entity* ent){
  if(ent == nullptr)
    return OBJECT_TYPE_INVALID;

  s_client_class* cc = ent->get_client_class();
  if(cc == nullptr)
    return OBJECT_TYPE_INVALID;

  if(ent->is_fake_projectile())
    return OBJECT_TYPE_INVALID;

  if(ent->is_health_pack())
    return OBJECT_TYPE_HEALTHPACK;
  else if(ent->is_ammo_pack())
    return OBJECT_TYPE_AMMOPACK;
  else if(ent->is_revive_marker() || ent->get_spellbook_type() != 0 || ent->is_lunchable() || ent->is_powerup() || ent->is_boss() || ent->is_bomb())
    return OBJECT_TYPE_WORLD_ENTITY;
  else if(ent->is_money()){
    if(!ent->currency_distributed() && !ent->currency_touched())
      return OBJECT_TYPE_WORLD_ENTITY;
  }
  else if(ent->is_arrow()) // Has special code to check if it's moving.
    return OBJECT_TYPE_PROJECTILES;
  else if(ent->is_projectile())
    return OBJECT_TYPE_PROJECTILES;
  else if(ent->is_sentry())
    return OBJECT_TYPE_SENTRY;
  else if(ent->is_dispenser())
    return OBJECT_TYPE_DISPENSER;
  else if(ent->is_teleporter())
    return OBJECT_TYPE_TELEPORTER;
  else if(ent->is_objective())
    return OBJECT_TYPE_OBJECTIVE;
  else if(cc->id == CHalloweenPickup || cc->id == CHalloweenGiftPickup)
    return OBJECT_TYPE_WORLD_ENTITY;

  return OBJECT_TYPE_INVALID;
}

bool c_world_esp::should_render_extra_entity(c_base_entity* ent, u32 type){
  if(ent == nullptr || type == OBJECT_TYPE_INVALID)
    return false;

  c_world_esp_settings* settings = utils::get_world_esp_settings(ent);
  if(settings == nullptr)
    return false;

  if(!settings->object_enabled[type])
    return false;

  if(ent->get_spellbook_type() != 0)
    return settings->ent_spellbooks;
  else if(ent->is_lunchable())
    return settings->ent_lunchables;
  else if(ent->is_powerup())
    return settings->ent_powerup;
  else if(ent->is_boss())
    return settings->ent_boss;
  else if(ent->is_bomb())
    return settings->ent_bombs;
  else if(ent->is_money())
    return settings->ent_money;
  else if(ent->is_revive_marker())
    return settings->ent_revive_marker;
  else if(ent->is_halloween_gift_pickup()  || ent->is_halloween_pickup())
    return settings->ent_halloween_pickup;
  else{
    s_client_class* cc = ent->get_client_class();
    if(cc == nullptr)
      return false;

    switch(cc->id){
      default:{
        if(ent->is_arrow() && settings->projectile_arrow)
          return true;

        break;
      }
      case CTFProjectile_JarGas:
      case CTFProjectile_Cleaver:
      case CTFProjectile_JarMilk:
      case CTFProjectile_Jar:
      case CTFProjectile_MechanicalArmOrb:
        return settings->projectile_throwables;
      case CTFGrenadePipebombProjectile:
        return settings->projectile_sticky && ent->pipebomb_type() == 1 || settings->projectile_pipebomb && ent->pipebomb_type() != 1;
      case CTFProjectile_Rocket:
      case CTFProjectile_SentryRocket:
      case CTFProjectile_EnergyBall:
        return settings->projectile_rocket;
      case CTFProjectile_Flare:
        return settings->projectile_flare;
      case CTFProjectile_SpellKartBats:
      case CTFProjectile_SpellKartOrb:
      case CTFProjectile_SpellLightningOrb:
      case CTFProjectile_SpellMeteorShower:
      case CTFProjectile_SpellMirv:
      case CTFProjectile_SpellPumpkin:
      case CTFProjectile_SpellSpawnHorde:
      case CTFProjectile_SpellSpawnZombie:
      case CTFProjectile_SpellSpawnBoss:
      case CTFProjectile_SpellBats:
      case CTFProjectile_SpellFireball:
        return settings->projectile_spells;
    }
  }

  return settings->object_enabled[type];
}

std::wstring c_world_esp::get_object_name(c_base_entity* ent, u32 type){
  if( ent == nullptr )
    return L"";

  switch(type){
    default: return L"";
    case OBJECT_TYPE_DISPENSER:
      return get_str(DISPENSER_TEXT);
    case OBJECT_TYPE_TELEPORTER:
      return ent->object_mode() == 0 ? get_str(TELEPORTER_ENTRANCE_TEXT) : get_str(TELEPORTER_EXIT_TEXT);
    case OBJECT_TYPE_OBJECTIVE:
      return get_str(OBJECTIVE_TEXT);
    case OBJECT_TYPE_AMMOPACK:
      return get_str(AMMOPACK_TEXT);
    case OBJECT_TYPE_HEALTHPACK:
      return get_str(HEALTHPACK_TEXT);
    case OBJECT_TYPE_PROJECTILES:
      return ent->get_projectile_name();
    case OBJECT_TYPE_WORLD_ENTITY:{
      if(ent->is_lunchable())
        return get_str(LUNCHABLE_TEXT);
      else if(ent->is_boss())
        return get_str(BOSS_TEXT);
      else if(ent->is_money())
        return get_str(MONEY_TEXT);
      else if(ent->is_bomb())
        return get_str(BOMB_TEXT);
      else if(ent->is_revive_marker())
        return get_str(REVIVE_MARKER_TEXT);
      else if(ent->is_halloween_gift_pickup())
        return get_str(HALLOWEEN_GIFT_TEXT);
      else if(ent->is_halloween_pickup())
        return get_str(HALLOWEEN_PICKUP_TEXT);
      else{
        switch(ent->get_spellbook_type()){
          default: break;
          case 1: return get_str(NORMAL_SPELLBOOK_TEXT);
          case 2: return get_str(RARE_SPELLBOOK_TEXT);
        }

        if(ent->is_powerup())
          return ent->get_powerup_name();
      }
      break;
    }
  }
}

bool c_world_esp::should_draw(c_base_entity* ent){
  if(ent == nullptr)
    return false;

  c_world_esp_settings* settings = utils::get_world_esp_settings(ent);
  if(settings == nullptr)
    return false;

  if(config->visual.render_engineer_mode){
    bool hurt = false;
    if(utils::engineer_mode(ent, hurt))
      return hurt;
  }

  if(config->visual.render_medic_mode){
    bool hurt = false;
    if(utils::medic_mode(ent, hurt))
      return hurt;
  }

  return settings->team_enabled;
}

bool c_world_esp::should_draw_extra(c_base_entity* ent, u32 type, u32 render_type){
  if( ent == nullptr )
    return false;

  s_client_class* cc = ent->get_client_class();
  if(cc == nullptr)
    return false;

  // It looks really weird.
  if(type == OBJECT_TYPE_OBJECTIVE && render_type != WORLD_ESP_RENDER_TYPE_GLOW){
    if(cc->id == classids::CCaptureFlag){
      if(ent->flag_status() == 1)
        return false;
    }
  }

  if(cc->id == classids::CPasstimeBall){
    if(ent->get_passtime_carrier() != nullptr)
      return false;
  }

  return true;
}

colour c_world_esp::get_draw_colour(c_base_entity* ent, i32 type, i32 colour_type){
  if(ent == nullptr)
    return rgb(255, 255, 255);

  colour draw_col = rgb(255, 255, 255);
  if(colour_type == WORLD_ESP_RENDER_TYPE_TEXT){

    // Team colour.
    if(config->visual.world_esp_use_team_colours){
      if(ent->team() == TEAM_BLU)
        draw_col = flt_array2clr(config->visual.world_esp_blu_team_colour);
      else if(ent->team() == TEAM_RED)
        draw_col = flt_array2clr(config->visual.world_esp_red_team_colour);
      else
        draw_col = flt_array2clr(config->visual.world_esp_unassigned_team_colour);
    }
    else if(config->visual.world_esp_use_enemyteam_colours){ // Enemy and friendly.
      if(global->localplayer_team != ent->team() && ent->team() >= 2 && ent->team() <= 3)
        draw_col = flt_array2clr(config->visual.world_esp_enemy_colour);
      else if(global->localplayer_team == ent->team())
        draw_col = flt_array2clr(config->visual.world_esp_team_colour);
      else
        draw_col = flt_array2clr(config->visual.world_esp_unassigned_team_colour);
    }

    if(type == OBJECT_TYPE_AMMOPACK)
      draw_col = flt_array2clr(config->visual.world_esp_ammo_pack_colour);
    else if(type == OBJECT_TYPE_HEALTHPACK)
      draw_col = flt_array2clr(config->visual.world_esp_health_pack_colour);
    else if(type == OBJECT_TYPE_WORLD_ENTITY){
      if(ent->is_boss())
        draw_col = flt_array2clr(config->visual.world_esp_boss_colour);
      else if(ent->is_bomb())
        draw_col = flt_array2clr(config->visual.world_esp_bomb_colour);
      else if(ent->is_money())
        draw_col = flt_array2clr(config->visual.world_esp_money_colour);
      else if(ent->is_lunchable())
        draw_col = flt_array2clr(config->visual.world_esp_lunchables_colour);
      else if(ent->is_powerup())
        draw_col = flt_array2clr(config->visual.world_esp_powerup_colour);
      else{
        switch(ent->get_spellbook_type()){
          default: break;
          case 1: // normal.
          {
            draw_col = flt_array2clr(config->visual.world_esp_spellbook_normal_colour);
            break;
          }
          case 2: // rare.
          {
            draw_col = flt_array2clr(config->visual.world_esp_spellbook_rare_colour);
            break;
          }
        }
      }
    }

    if(config->visual.world_esp_aimbot_target){
      if(ent->get_index() == global->aimbot_target_index)
        draw_col = flt_array2clr(config->visual.world_esp_aimbot_target_colour);
    }

    draw_col.w = 255;
  }
  else if(colour_type == WORLD_ESP_RENDER_TYPE_CHAMS){

    // Team colour.
    if(config->visual.world_chams_use_team_colours){
      if(ent->team() == TEAM_BLU)
        draw_col = flt_array2clr(config->visual.world_chams_blu_team_colour);
      else if(ent->team() == TEAM_RED)
        draw_col = flt_array2clr(config->visual.world_chams_red_team_colour);
      else
        draw_col = flt_array2clr(config->visual.world_chams_unassigned_team_colour);
    }
    else if(config->visual.world_chams_use_enemyteam_colours){ // Enemy and friendly.
      if(global->localplayer_team != ent->team() && ent->team() >= 2 && ent->team() <= 3)
        draw_col = flt_array2clr(config->visual.world_chams_enemy_colour);
      else if(global->localplayer_team == ent->team())
        draw_col = flt_array2clr(config->visual.world_chams_team_colour);
      else
        draw_col = flt_array2clr(config->visual.world_chams_unassigned_team_colour);
    }

    if(type == OBJECT_TYPE_AMMOPACK)
      draw_col = flt_array2clr(config->visual.world_chams_ammo_pack_colour);
    else if(type == OBJECT_TYPE_HEALTHPACK)
      draw_col = flt_array2clr(config->visual.world_chams_health_pack_colour);
    else if(type == OBJECT_TYPE_WORLD_ENTITY){
      if(ent->is_boss())
        draw_col = flt_array2clr(config->visual.world_chams_boss_colour);
      else if(ent->is_bomb())
        draw_col = flt_array2clr(config->visual.world_chams_bomb_colour);
      else if(ent->is_money())
        draw_col = flt_array2clr(config->visual.world_chams_money_colour);
      else if(ent->is_lunchable())
        draw_col = flt_array2clr(config->visual.world_chams_lunchables_colour);
      else if(ent->is_powerup())
        draw_col = flt_array2clr(config->visual.world_chams_powerup_colour);
      else{
        switch(ent->get_spellbook_type()){
          default: break;
          case 1: // normal.
          {
            draw_col = flt_array2clr(config->visual.world_chams_spellbook_normal_colour);
            break;
          }
          case 2: // rare.
          {
            draw_col = flt_array2clr(config->visual.world_chams_spellbook_rare_colour);
            break;
          }
        }
      }
    }

    if(config->visual.world_chams_aimbot_target){
      if(ent->get_index() == global->aimbot_target_index)
        draw_col = flt_array2clr(config->visual.world_chams_aimbot_target_colour);
    }
  }
  else if(colour_type == WORLD_ESP_RENDER_TYPE_CHAMS_OVERLAY){

    // Team colour.
    if(config->visual.world_chams_use_team_colours){
      if(ent->team() == TEAM_BLU)
        draw_col = flt_array2clr(config->visual.world_chams_overlay_blu_team_colour);
      else if(ent->team() == TEAM_RED)
        draw_col = flt_array2clr(config->visual.world_chams_overlay_red_team_colour);
      else
        draw_col = flt_array2clr(config->visual.world_chams_overlay_unassigned_team_colour);
    }
    else if(config->visual.world_chams_use_enemyteam_colours){ // Enemy and friendly.
      if(global->localplayer_team != ent->team() && ent->team() >= 2 && ent->team() <= 3)
        draw_col = flt_array2clr(config->visual.world_chams_overlay_enemy_colour);
      else if(global->localplayer_team == ent->team())
        draw_col = flt_array2clr(config->visual.world_chams_overlay_team_colour);
      else
        draw_col = flt_array2clr(config->visual.world_chams_overlay_unassigned_team_colour);
    }

    if(type == OBJECT_TYPE_AMMOPACK)
      draw_col = flt_array2clr(config->visual.world_chams_overlay_ammo_pack_colour);
    else if(type == OBJECT_TYPE_HEALTHPACK)
      draw_col = flt_array2clr(config->visual.world_chams_overlay_health_pack_colour);
    else if(type == OBJECT_TYPE_WORLD_ENTITY){
      if(ent->is_boss())
        draw_col = flt_array2clr(config->visual.world_chams_overlay_boss_colour);
      else if(ent->is_bomb())
        draw_col = flt_array2clr(config->visual.world_chams_overlay_bomb_colour);
      else if(ent->is_money())
        draw_col = flt_array2clr(config->visual.world_chams_overlay_money_colour);
      else if(ent->is_lunchable())
        draw_col = flt_array2clr(config->visual.world_chams_overlay_lunchables_colour);
      else if(ent->is_powerup())
        draw_col = flt_array2clr(config->visual.world_chams_overlay_powerup_colour);
      else{
        switch(ent->get_spellbook_type()){
          default: break;
          case 1: // normal.
          {
            draw_col = flt_array2clr(config->visual.world_chams_overlay_spellbook_normal_colour);
            break;
          }
          case 2: // rare.
          {
            draw_col = flt_array2clr(config->visual.world_chams_overlay_spellbook_rare_colour);
            break;
          }
        }
      }
    }

    if(config->visual.world_chams_overlay_aimbot_target){
      if(ent->get_index() == global->aimbot_target_index)
        draw_col = flt_array2clr(config->visual.world_chams_overlay_aimbot_target_colour);
    }
  }
  else if(colour_type == WORLD_ESP_RENDER_TYPE_GLOW){
    // Team colour.
    if(config->visual.world_glow_use_team_colours){
        if(ent->team() == TEAM_BLU)
          draw_col = flt_array2clr(config->visual.world_glow_blu_team_colour);
        else if(ent->team() == TEAM_RED)
          draw_col = flt_array2clr(config->visual.world_glow_red_team_colour);
        else
          draw_col = flt_array2clr(config->visual.world_glow_unassigned_team_colour);
    }
    else if(config->visual.world_glow_use_enemyteam_colours){ // Enemy and friendly.
        if(global->localplayer_team != ent->team() && ent->team() >= 2 && ent->team() <= 3)
          draw_col = flt_array2clr(config->visual.world_glow_enemy_colour);
        else if(global->localplayer_team == ent->team())
          draw_col = flt_array2clr(config->visual.world_glow_team_colour);
        else
          draw_col = flt_array2clr(config->visual.world_glow_unassigned_team_colour);
    }
    if(type == OBJECT_TYPE_AMMOPACK)
      draw_col = flt_array2clr(config->visual.world_glow_ammo_pack_colour);
    else if(type == OBJECT_TYPE_HEALTHPACK)
      draw_col = flt_array2clr(config->visual.world_glow_health_pack_colour);
    else if(type == OBJECT_TYPE_WORLD_ENTITY){

        if(ent->is_boss())
          draw_col = flt_array2clr(config->visual.world_glow_boss_colour);
        else if(ent->is_bomb())
          draw_col = flt_array2clr(config->visual.world_glow_bomb_colour);
        else if(ent->is_money())
          draw_col = flt_array2clr(config->visual.world_glow_money_colour);
        else if(ent->is_lunchable())
          draw_col = flt_array2clr(config->visual.world_glow_lunchables_colour);
        else if(ent->is_powerup())
          draw_col = flt_array2clr(config->visual.world_glow_powerup_colour);
        else{
          switch(ent->get_spellbook_type()){
            default: break;
            case 1: // normal.
            {
              draw_col = flt_array2clr(config->visual.world_glow_spellbook_normal_colour);
              break;
            }
            case 2: // rare.
            {
              draw_col = flt_array2clr(config->visual.world_glow_spellbook_rare_colour);
              break;
            }
          }
        }
    }

    if(config->visual.health_glow_enabled){   
      if(type == OBJECT_TYPE_SENTRY && config->visual.health_glow_sentries || type == OBJECT_TYPE_DISPENSER && config->visual.health_glow_dispenser || type == OBJECT_TYPE_TELEPORTER && config->visual.health_glow_teleporter){
        colour hp_col;
        if(utils::get_health_colour(ent->object_health(), ent->object_max_health(), &hp_col)){ 
          if(global->localplayer_team == ent->team() && config->visual.health_glow_show_friendly || global->localplayer_team != ent->team() && config->visual.health_glow_show_enemy)
            draw_col = hp_col;
        }
      }
    }

    if(config->visual.world_glow_aimbot_target){
      if(ent->get_index() == global->aimbot_target_index)
        draw_col = flt_array2clr(config->visual.world_glow_aimbot_target_colour);
    }
  }

  return draw_col;
}

colour c_world_esp::get_bland_colour(c_base_entity* entity){
  if(entity == nullptr)
    return rgb(255, 255, 255);

  return entity->is_dormant()? rgb(181, 181, 181) : rgb(255, 255, 255);
}

template<auto c>
void c_world_esp::add_flags(c_base_entity* ent, u32 type, c_esp_data<c>* esp_data){
  if(ent == nullptr || esp_data == nullptr)
    return;

  c_world_esp_settings* settings = utils::get_world_esp_settings(ent);
  if(settings == nullptr)
    return;

  colour team_col = get_draw_colour(ent, type);


  if(type == OBJECT_TYPE_SENTRY){
    if(settings->sentry_level){
      if(!(ent->mini_building() || ent->disposable_building()))
        esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 255, 255), global->esp_font_small, 8, _building_level[math::clamp(ent->upgrade_level(), 0, 3)]);
    }

    if(settings->sentry_controlled){
      if(ent->player_controlled())
        esp_data->add(ESP_TEXT_TYPE_RIGHT, team_col, global->esp_font_small, 8, get_str(CONTROLLED_TEXT));
    }

    if(settings->sentry_activestate){
      if(ent->sapped())
        esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(167, 221, 242), global->esp_font_small, 8, get_str(SAPPED_TEXT));
      else{
        if((ent->disabled() || ent->plasma_disabled()))
          esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(188, 188, 188), global->esp_font_small, 8, get_str(DISABLED_TEXT));
      }
    }

    if(settings->sentry_buildingstate){
      if(ent->building() || ent->placing())
        esp_data->add(ESP_TEXT_TYPE_RIGHT, team_col, global->esp_font_small, 8, get_str(CONSTRUCTING_TEXT));
    }

    if(settings->sentry_ammostatus){
      if(ent->shells() == 0 && !(ent->building() || ent->placing()))
        esp_data->add(ESP_TEXT_TYPE_RIGHT, team_col, global->esp_font_small, 8, get_str(SENTRY_NO_AMMO_TEXT));
    }
  }
  else if(type == OBJECT_TYPE_DISPENSER){
    if(settings->dispenser_level)
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 255, 255), global->esp_font_small, 8, _building_level[math::clamp(ent->upgrade_level(), 0, 3)]);
    

    if(settings->dispenser_buildingstate){
      if(ent->building() || ent->placing())
        esp_data->add(ESP_TEXT_TYPE_RIGHT, team_col, global->esp_font_small, 8, get_str(CONSTRUCTING_TEXT));
    }

    if(settings->dispenser_activestate){
      if(ent->sapped())
        esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(167, 221, 242), global->esp_font_small, 8, get_str(SAPPED_TEXT));
      else{
        if((ent->disabled() || ent->plasma_disabled()))
          esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(188, 188, 188), global->esp_font_small, 8, get_str(DISABLED_TEXT));
      }
    }
  }
  else if(type == OBJECT_TYPE_TELEPORTER){
    if(settings->teleporter_level)
      esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 255, 255), global->esp_font_small, 8, _building_level[math::clamp(ent->upgrade_level(), 0, 3)]);
    
    if(settings->teleporter_buildingstate){
      if(ent->building() || ent->placing())
        esp_data->add(ESP_TEXT_TYPE_RIGHT, team_col, global->esp_font_small, 8, get_str(CONSTRUCTING_TEXT));
    }

    if(settings->teleporter_activestate){
      if(ent->sapped())
        esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(167, 221, 242), global->esp_font_small, 8, get_str(SAPPED_TEXT));

      if((ent->disabled() || ent->plasma_disabled()))
        esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(188, 188, 188), global->esp_font_small, 8, get_str(DISABLED_TEXT));
    }
  }
  else if(type == OBJECT_TYPE_OBJECTIVE){

    // Probably some day we're gonna add some new objective and have problems with this since it's only for the CCaptureFlag class.
    if(ent->get_client_class()->id == classids::CCaptureFlag){
      if(settings->objective_status){
        if(ent->flag_status() == 0)
          esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(188, 255, 188), global->esp_font_small, 8, get_str(INTEL_HOME_TEXT));
        else if( ent->flag_status() == 1 )
          esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 188, 188), global->esp_font_small, 8, get_str(INTEL_STOLEN_TEXT));
        else if( ent->flag_status() == 2 )
          esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 255, 188), global->esp_font_small, 8, get_str(INTEL_DROPPED_TEXT));
      }

      if(settings->objective_returntime){
        if(ent->flag_status() == 2 /*&& ent->flag_type != 1*/){
          i32 return_in_time = (i32)math::abs(ent->flag_reset_time() - globaldata->cur_time);
          if(return_in_time <= 300){
            esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 255, 255), global->esp_font_small, 8, _intel_returns_time[return_in_time]);
          }
        }
      }
    }
  }
  else if(type == OBJECT_TYPE_PROJECTILES){
    if(settings->projectile_flag_criticals){
      if(ent->is_projectile_critical())
        esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 175, 255), global->esp_font_small, 8, get_str(CRITICAL_TEXT));
      else if(ent->is_deflected())
        esp_data->add(ESP_TEXT_TYPE_RIGHT, rgb(255, 195, 255), global->esp_font_small, 8, get_str(REFLECTED_TEXT));
    }
  }
}

u32 c_world_esp::fetch_render_flags(u32 type, c_world_esp_settings* settings){
  u32 flags = 0;

  if(!config->visual.cheap_text_rendering)
    flags |= ESP_FLAG_TEXT_OUTLINES;

  if(settings->box[type])
    flags |= ESP_FLAG_BOX;

  if(settings->box_outlines[type])
    flags |= ESP_FLAG_OUTLINES;

  return flags;
}

void c_world_esp::run(){
  precache();
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  float time = math::time();
  for(u32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* ent = global->entity_list->get_entity(i);

    if(ent == nullptr)
      continue;

    if(!ent->should_draw())
      continue;

    uptr* model = ent->get_model();
    if(model == nullptr)
      continue;

    u32 type = get_object_index(ent);
    if(type == OBJECT_TYPE_INVALID)
      continue;

    assert(type >= 0);
    if(!should_render_extra_entity(ent, type))
      continue;

    if(!should_draw(ent))
      continue;

    if(!should_draw_extra(ent, type, WORLD_ESP_RENDER_TYPE_TEXT))
      continue;

    s_client_class* cc = ent->get_client_class();
    if(cc == nullptr)
      continue;

    c_entity_data* data = ent->entity_data();
    if(data == nullptr){
      DBG("[!] No entity data for %s (CALLING utils::create_entity_data)\n", ent->get_client_class()->name);
      utils::create_entity_data(ent);
      continue;
    }

    c_world_esp_settings* settings = utils::get_world_esp_settings(ent);
    if(settings == nullptr)
      continue;

    if(ent->is_dormant()){
      if(data->dormant_timeout < time || !settings->dormant_enabled[type])
        continue;
    }

    if(!data->has_transform)
      continue;

    s_box_data box_data = math::calculate_box_region(ent->obb_mins(), ent->obb_maxs(), data->transform);
    if(!box_data.valid)
      continue;

    u32 render_esp_flags = fetch_render_flags(type, settings);

    // box thinning
    {
      box_data.size.x = math::biggest(box_data.size.x, 8);
      box_data.size.y = math::biggest(box_data.size.y, 8);
    }

    c_esp_data<32> ex_data;

    // We won't render the health bar if we have received no health!
    if(type >= OBJECT_TYPE_SENTRY && type <= OBJECT_TYPE_TELEPORTER && settings->healthbar[type]){
      colour hp_col;
      if(utils::get_health_colour(ent->object_health(), ent->object_max_health(), &hp_col))
        ex_data.add(ESP_BAR_TYPE_LEFT, math::clamp((float)ent->object_health() / (float)ent->object_max_health(), 0.f, 1.f), settings->health_bar_fractions, hp_col);   
    }

    colour bland_colour = get_bland_colour(ent);
    // draw name
    if(settings->name[type]){
      if( type == OBJECT_TYPE_SENTRY )
        ex_data.add(ESP_TEXT_TYPE_TOP, bland_colour, global->esp_font_large, 12, (ent->mini_building() || ent->disposable_building()) ? get_str(MINI_SENTRY_TEXT) : get_str(SENTRY_TEXT));
      else
        ex_data.add(ESP_TEXT_TYPE_TOP, bland_colour, global->esp_font_large, 12, get_object_name(ent, type));
    }

    // Show the dormant text.
    if(settings->dormant_render_text[type] && ent->is_dormant()){
      ex_data.add(ESP_TEXT_TYPE_TOP, rgb(181, 181, 181), global->esp_font_large, 12, get_str(DORMANT_TEXT));
    }

    if(config->visual.show_estimated_aim_target){
      if(i == target_list->best_target_index){
        i32    size = math::clamp(config->visual.estimated_aim_target_size, 10, 14);
        colour clr  = rgba(config->visual.estimated_aim_target_colour[0], config->visual.estimated_aim_target_colour[1], config->visual.estimated_aim_target_colour[2], 255);

        if(config->visual.estimated_aim_target_circle){
          ex_data.add(ESP_TEXT_TYPE_TOP, clr, global->esp_font_large, size, get_str(AIM_TARGET_CIRCLE_TEXT));
        }
        else if(config->visual.estimated_aim_target_triangle)
          ex_data.add(ESP_TEXT_TYPE_TOP, clr, global->esp_font_large, size, get_str(RESOLVER_MANUAL_ARROW_TEXT));
        else
          ex_data.add(ESP_TEXT_TYPE_TOP, clr, global->esp_font_large, size, get_str(AIM_TARGET_LINE_ARROW_TEXT));
      }
    }

    // draw flags
    if(settings->flags[type])
      add_flags(ent, type, &ex_data);

    // draw distance
    if(settings->distance[type]){
      i32 dist = math::smallest(1024, math::biggest(0, localplayer->origin().distance_meter(ent->origin())));
      ex_data.add(ESP_TEXT_TYPE_BOTTOM, bland_colour, global->esp_font_small, 10, _dist[dist]);
    }

    render->draw_esp_box(box_data.pos, box_data.size, get_draw_colour(ent, type), render_esp_flags, &ex_data);
  }
}