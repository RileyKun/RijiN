#pragma once

#include "settings.h"

class gcs_menu : public gcs_base_menu{
public:
  void create_aimbot_main_layer(gcs_component_group* group, i32 current_class, i32 i){
    if(group == nullptr)
      return;

    assert(i >= 0 && i <= 3);

    c_aimbot_settings* aimbot_settings = &config->aimbot[current_class - 1][i];
    if(aimbot_settings == nullptr)
      return;

    // Aimbot
    {
      gcs::toggle(group, WXOR(L"Enabled"), &aimbot_settings->enabled)
        ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&aimbot_settings->aim_hotkey)
            ->set_render_on(&aimbot_settings->aim_mode_on_key)
            ->set_render_on(&aimbot_settings->enabled);

      if(i != AIMBOT_WEAPON_SETTING_MEDIGUN){
        gcs::toggle(group, WXOR(L"Auto shoot"), &aimbot_settings->auto_shoot)
          ->set_render_on(&aimbot_settings->aim_mode_on_key)
          ->set_render_on(&aimbot_settings->enabled);

        if(i == AIMBOT_WEAPON_SETTING_PROJ){
          if(current_class == TF_CLASS_DEMOMAN || current_class == TF_CLASS_SNIPER){
            gcs::toggle(group, WXOR(L"Auto release"), &aimbot_settings->auto_release)
              ->set_render_on(&aimbot_settings->aim_mode_on_attack)
              ->set_render_on(&aimbot_settings->enabled)
              ->set_help_text(WXOR(L"Auto-release the sticky launcher, loose cannon, or huntsman when charged."));
          }
          else if(current_class == TF_CLASS_SOLDIER){
            gcs::toggle(group, WXOR(L"Auto load beggars"), &aimbot_settings->auto_reload_beggars)
              ->set_render_on(&aimbot_settings->aim_mode_on_key)
              ->set_render_on(&aimbot_settings->enabled);
          }
        }
      }

      gcs::dropdown(group, WXOR(L"Aim mode"), false)
        ->add_option(WXOR(L"Automatic"), &aimbot_settings->aim_mode_automatic)
        ->add_option(WXOR(L"On attack"), &aimbot_settings->aim_mode_on_attack)
        ->add_option(WXOR(L"On key"), &aimbot_settings->aim_mode_on_key)
          ->set_render_on(&aimbot_settings->enabled);

      gcs::slider(group, WXOR(L"Maximum FOV"), WXOR(L"°"), 1, 180, GCS_SLIDER_TYPE_INT, &aimbot_settings->max_fov)
        ->set_render_on(&aimbot_settings->enabled);

      if(i == AIMBOT_WEAPON_SETTING_HITSCAN){
        gcs::slider(group, WXOR(L"Smoothing"), WXOR(L"%"), 0, 99, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->smoothing)
          ->set_render_on(&aimbot_settings->enabled)
          ->set_render_on(&aimbot_settings->silent_aim_none);
      }

      gcs::dropdown(group, WXOR(L"Silent aim"), false)
        ->add_option(WXOR(L"Disabled"), &aimbot_settings->silent_aim_none)
        ->add_option(WXOR(L"Clientside"), &aimbot_settings->silent_aim_clientside)
        ->add_option(WXOR(L"Serverside"), &aimbot_settings->silent_aim_serverside)
          ->set_render_on(&aimbot_settings->enabled)
          ->set_help_text(WXOR(L"Disabled: Plain aimbot. Recommended on community servers with an anti-cheat installed.\nClientside: Your view won't snap to targets. But other players will still see you snapping.\nServerside: Only works on non hitscan weapons, prevents people from being able to see your angle snaps."));

      if(i == AIMBOT_WEAPON_SETTING_HITSCAN){
        gcs::dropdown(group, WXOR(L"Position adjustment"), true)
          ->add_option(WXOR(L"History"), &aimbot_settings->position_adjust_history)
          ->add_option(WXOR(L"Accurate unlag"), &aimbot_settings->position_adjust_accurate_unlag)
          ->set_render_on(&aimbot_settings->enabled)
          ->set_help_text(WXOR(L"History: This backtracking, not recommended for closet gameplay.\nAccurate unlag: Fix player's breaking lag compensation."));
      }
      else if(i == AIMBOT_WEAPON_SETTING_MELEE){
        gcs::dropdown(group, WXOR(L"Position adjustment"), true)
          ->add_option(WXOR(L"History"), &aimbot_settings->position_adjust_history)
          ->set_render_on(&aimbot_settings->enabled)
          ->set_help_text(WXOR(L"History: This backtracking, not recommended for closet gameplay."));
      }
    }
  }

  void create_aimbot_target_layer(gcs_component_group* group, i32 current_class, i32 i){
    if(group == nullptr)
      return;

    assert(i >= 0 && i <= 3);

    c_aimbot_settings* aimbot_settings = &config->aimbot[current_class - 1][i];
    if(aimbot_settings == nullptr)
      return;

    if(i == AIMBOT_WEAPON_SETTING_MEDIGUN){
      gcs::toggle(group, WXOR(L"Disable when spectated"), &aimbot_settings->disable_on_spectated);
      gcs::add_space(group);

      gcs::dropdown(group, WXOR(L"Healing selection"), false)
        ->add_option(WXOR(L"All"), &aimbot_settings->auto_medigun_preferall)
        ->add_option(WXOR(L"Prefer friends"), &aimbot_settings->auto_medigun_preferfriends)
        ->add_option(WXOR(L"Prefer friends except key"), &aimbot_settings->auto_medigun_preferfriends_except_on_key)
        ->add_option(WXOR(L"Friends only"), &aimbot_settings->auto_medigun_friendsonly);

      gcs::label(group, WXOR(L"Ignore prefer friends key"))
        ->set_render_on(&aimbot_settings->auto_medigun_preferfriends_except_on_key)
          ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&aimbot_settings->auto_medigun_preferfriends_except_on_key_hotkey);

      gcs::slider(group, WXOR(L"Next heal target delay"), WXOR(L"s"), 0, 5, GCS_SLIDER_TYPE_FLOAT_PRECISE, &aimbot_settings->auto_heal_next_target_delay);
      gcs::toggle(group, WXOR(L"Share uber"), &aimbot_settings->enable_uber_sharing);
      gcs::slider(group, WXOR(L"Next swap delay"), WXOR(L"s"), 0, 10, GCS_SLIDER_TYPE_FLOAT_PRECISE, &aimbot_settings->uber_sharing_time)
        ->set_render_on(&aimbot_settings->enable_uber_sharing)
        ->set_help_text(WXOR(L"Note: TF2 will reduce your ubercharge faster when you swap heal targets."));
      return;
    }
    else if(i == AIMBOT_WEAPON_SETTING_HITSCAN){
        gcs::toggle(group, WXOR(L"Force body aim on key"), &aimbot_settings->force_body_aim_on_key)
          ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&aimbot_settings->force_body_aim_key)
          ->set_render_on(&aimbot_settings->force_body_aim_on_key);

        gcs::toggle(group, WXOR(L"Disable when spectated"), &aimbot_settings->disable_on_spectated);
        gcs::add_space(group);

        gcs::dropdown(group, WXOR(L"Aim at"), true)
          ->add_option(WXOR(L"Team"), &aimbot_settings->target_friendlies)
          ->add_option(WXOR(L"Enemies"), &aimbot_settings->target_enemies)
          ->add_option(WXOR(L"Cloaked Spies"), &aimbot_settings->target_cloaked_spies)
          ->add_option(WXOR(L"Friends"), &aimbot_settings->target_friends)
          ->add_option(WXOR(L"Sentry"), &aimbot_settings->target_sentries)
          ->add_option(WXOR(L"Dispenser"), &aimbot_settings->target_dispenser)
          ->add_option(WXOR(L"Teleporter"), &aimbot_settings->target_teleporter)
          ->add_option(WXOR(L"Sticky"), &aimbot_settings->target_sticky)
          ->add_option(WXOR(L"Boss/Tank"), &aimbot_settings->target_boss);
      }
    else if(i == AIMBOT_WEAPON_SETTING_PROJ){
        gcs::toggle(group, WXOR(L"Disable when spectated"), &aimbot_settings->disable_on_spectated);
        gcs::add_space(group);

        gcs::dropdown(group, WXOR(L"Aim at"), true)
          ->add_option(WXOR(L"Team"), &aimbot_settings->target_friendlies)
          ->add_option(WXOR(L"Enemies"), &aimbot_settings->target_enemies)
          ->add_option(WXOR(L"Cloaked Spies"), &aimbot_settings->target_cloaked_spies)
          ->add_option(WXOR(L"Friends"), &aimbot_settings->target_friends)
          ->add_option(WXOR(L"Sentry"), &aimbot_settings->target_sentries)
          ->add_option(WXOR(L"Dispenser"), &aimbot_settings->target_dispenser)
          ->add_option(WXOR(L"Teleporter"), &aimbot_settings->target_teleporter);

          if(current_class == TF_CLASS_DEMOMAN){
            gcs::dropdown(group, WXOR(L"Loose cannon"), true)
              ->add_option(WXOR(L"Target ubercharged"), &aimbot_settings->loose_cannon_target_ubercharged)
              ->add_option(WXOR(L"Target bonked"), &aimbot_settings->loose_cannon_target_bonked);
          }
      }
    else if(i == AIMBOT_WEAPON_SETTING_MELEE){
      if(current_class != TF_CLASS_SPY)
        #if defined(DEV_MODE)
        gcs::toggle(group, WXOR(L"Swing prediction"), &aimbot_settings->swing_prediction)
          ->set_help_text(WXOR(L"Swing in advance if it predicts it'll hit the target."));
        #endif

      gcs::toggle(group, WXOR(L"Disable when spectated"), &aimbot_settings->disable_on_spectated);
      gcs::add_space(group);

      gcs::dropdown(group, WXOR(L"Aim at"), true)
        ->add_option(WXOR(L"Team"), &aimbot_settings->target_friendlies)
        ->add_option(WXOR(L"Enemies"), &aimbot_settings->target_enemies)
        ->add_option(WXOR(L"Cloaked Spies"), &aimbot_settings->target_cloaked_spies)
        ->add_option(WXOR(L"Friends"), &aimbot_settings->target_friends)
        ->add_option(WXOR(L"Sentry"), &aimbot_settings->target_sentries)
        ->add_option(WXOR(L"Dispenser"), &aimbot_settings->target_dispenser)
        ->add_option(WXOR(L"Teleporter"), &aimbot_settings->target_teleporter)
        ->add_option(WXOR(L"Boss/Tank"), &aimbot_settings->target_boss);
    }

    if(i == AIMBOT_WEAPON_SETTING_MELEE){
      if(current_class == TF_CLASS_SPY){
        gcs::dropdown(group, WXOR(L"Ignore flags"), true)
          ->add_option(WXOR(L"Ignore healed robots"), &aimbot_settings->mvm_ignore_healed)
          ->add_option(WXOR(L"Ignore sentry busters"), &aimbot_settings->mvm_ignore_sentrybuster)
          ->add_option(WXOR(L"Ignore razerback"), &aimbot_settings->spy_ignore_razer_back)
          ->add_option(WXOR(L"Ignore cheaters"), &aimbot_settings->ignore_cheaters)
            ->set_render_on(&aimbot_settings->enabled);
      }
      else{
        gcs::dropdown(group, WXOR(L"Ignore flags"), true)
          ->add_option(WXOR(L"Ignore healed robots"), &aimbot_settings->mvm_ignore_healed)
          ->add_option(WXOR(L"Ignore sentry busters"), &aimbot_settings->mvm_ignore_sentrybuster)
          ->add_option(WXOR(L"Ignore cheaters"), &aimbot_settings->ignore_cheaters);
      }
    }
    else{
      gcs::dropdown(group, WXOR(L"Algorithm"), false)
        ->add_option(WXOR(L"Closest to FOV"), &aimbot_settings->target_method_closest_to_fov)
        ->add_option(WXOR(L"Distance"), &aimbot_settings->target_method_distance);

      if(current_class == TF_CLASS_SNIPER){
        gcs::dropdown(group, WXOR(L"Ignore flags"), true)
          ->add_option(WXOR(L"Ignore healed robots"), &aimbot_settings->mvm_ignore_healed)
          ->add_option(WXOR(L"Ignore sentry busters"), &aimbot_settings->mvm_ignore_sentrybuster)
          ->add_option(WXOR(L"Ignore vaccinator"), &aimbot_settings->sniper_ignore_vaccinator)
          ->add_option(WXOR(L"Ignore cheaters"), &aimbot_settings->ignore_cheaters);
      }
      else{
        gcs::dropdown(group, WXOR(L"Ignore flags"), true)
          ->add_option(WXOR(L"Ignore healed robots"), &aimbot_settings->mvm_ignore_healed)
          ->add_option(WXOR(L"Ignore sentry busters"), &aimbot_settings->mvm_ignore_sentrybuster)
          ->add_option(WXOR(L"Ignore cheaters"), &aimbot_settings->ignore_cheaters);
      }
    }

    if(i == AIMBOT_WEAPON_SETTING_HITSCAN){
      switch(current_class){
      default:
      {
        gcs::dropdown(group, WXOR(L"Priority hitbox"), false)
          ->add_option(WXOR(L"Pelvis"), &aimbot_settings->priority_hitbox[1])
          ->add_option(WXOR(L"Stomach"), &aimbot_settings->priority_hitbox[2])
          ->add_option(WXOR(L"Chest"), &aimbot_settings->priority_hitbox[3])
          ->add_option(WXOR(L"Upper chest"), &aimbot_settings->priority_hitbox[4]);

        gcs::dropdown(group, WXOR(L"Hitscan"), true)
          ->add_option(WXOR(L"Head"), &aimbot_settings->hitbox[0])
          ->add_option(WXOR(L"Pelvis"), &aimbot_settings->hitbox[1])
          ->add_option(WXOR(L"Stomach"), &aimbot_settings->hitbox[2])
          ->add_option(WXOR(L"Chest"), &aimbot_settings->hitbox[3])
          ->add_option(WXOR(L"Upper chest"), &aimbot_settings->hitbox[4])
          ->add_option(WXOR(L"Neck"), &aimbot_settings->hitbox[5]);

        gcs::toggle(group, WXOR(L"Use Double tap hitbox"), &aimbot_settings->double_tap_aim_point);
        gcs::dropdown(group, WXOR(L"Double tap hitbox"), false)
          ->add_option(WXOR(L"Pelvis"), &aimbot_settings->double_tap_aim_point_pelvis)
          ->add_option(WXOR(L"Stomach"), &aimbot_settings->double_tap_aim_point_stomach)
          ->set_render_on(&aimbot_settings->double_tap_aim_point);

        break;
      }
      case TF_CLASS_SNIPER:
      case TF_CLASS_SPY:
      {
        gcs::dropdown(group, WXOR(L"Priority hitbox"), false)
          ->add_option(WXOR(L"Head"), &aimbot_settings->priority_hitbox[0])
          ->add_option(WXOR(L"Pelvis"), &aimbot_settings->priority_hitbox[1])
          ->add_option(WXOR(L"Stomach"), &aimbot_settings->priority_hitbox[2])
          ->add_option(WXOR(L"Chest"), &aimbot_settings->priority_hitbox[3])
          ->add_option(WXOR(L"Upper chest"), &aimbot_settings->priority_hitbox[4]);

        gcs::dropdown(group, WXOR(L"Hitscan"), true)
          ->add_option(WXOR(L"Head"), &aimbot_settings->hitbox[0])
          ->add_option(WXOR(L"Pelvis"), &aimbot_settings->hitbox[1])
          ->add_option(WXOR(L"Stomach"), &aimbot_settings->hitbox[2])
          ->add_option(WXOR(L"Chest"), &aimbot_settings->hitbox[3])
          ->add_option(WXOR(L"Upper chest"), &aimbot_settings->hitbox[4])
          ->add_option(WXOR(L"Neck"), &aimbot_settings->hitbox[5]);

        gcs::toggle(group, WXOR(L"Use Double tap hitbox"), &aimbot_settings->double_tap_aim_point);
        gcs::dropdown(group, WXOR(L"Double tap hitbox"), false)
          ->add_option(WXOR(L"Pelvis"), &aimbot_settings->double_tap_aim_point_pelvis)
          ->add_option(WXOR(L"Stomach"), &aimbot_settings->double_tap_aim_point_stomach)
          ->set_render_on(&aimbot_settings->double_tap_aim_point);

        if(current_class == TF_CLASS_SPY){
          gcs::dropdown(group, WXOR(L"Bodyaim if"), true)
            ->add_option(WXOR(L"Lethal"), &aimbot_settings->spy_ambassador_body_aim_if_lethal);
        }
        else{
          gcs::dropdown(group, WXOR(L"Bodyaim if"), true)
            ->add_option(WXOR(L"Lethal"), &aimbot_settings->sniper_body_aim_if_lethal)
            ->add_option(WXOR(L"Crit immune"), &aimbot_settings->sniper_body_aim_if_crit_immune);
        }
        break;
      }
    }

      gcs::toggle(group, WXOR(L"Point scale"), &aimbot_settings->point_scale_enabled)
        ->set_render_on(&aimbot_settings->enabled)
        ->set_help_text(WXOR(L"Adds more aimpoints on the selected priority hitbox."));

      gcs::slider(group, WXOR(L"Scale"), WXOR(L""), 1, 100, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->point_scale)
        ->set_render_on(&aimbot_settings->enabled)
        ->set_render_on(&aimbot_settings->point_scale_enabled);
    }
    else if(i == AIMBOT_WEAPON_SETTING_PROJ || i == AIMBOT_WEAPON_SETTING_MEDIGUN){
      gcs::dropdown(group, WXOR(L"Priority point"), false)
        ->add_option(WXOR(L"Auto"), &aimbot_settings->obb_auto)
        ->add_option(WXOR(L"Top"), &aimbot_settings->obb_head)
        ->add_option(WXOR(L"Center"), &aimbot_settings->obb_body)
        ->add_option(WXOR(L"Bottom"), &aimbot_settings->obb_feet);
    }
  }

  void create_aimbot_other_layer(gcs_component_group* group, i32 current_class, i32 i){
    if(group == nullptr)
      return;

    assert(i >= 0 && i <= 3);

    c_aimbot_settings* aimbot_settings = &config->aimbot[current_class - 1][i];
    if(aimbot_settings == nullptr)
      return;

    if(i != AIMBOT_WEAPON_SETTING_MELEE){
      gcs::toggle(group, WXOR(L"No recoil"), &config->misc.no_recoil)
        ->set_help_text(WXOR(L"Removes recoil silently."));

      gcs::toggle(group, WXOR(L"No spread"), &aimbot_settings->no_spread)
        ->set_help_text(WXOR(L"Only works with projectile weapons, hitscan support is kept for certain HvH servers."));

      gcs::add_space(group);
      if(i == AIMBOT_WEAPON_SETTING_HITSCAN){
        gcs::dropdown(group, WXOR(L"Manipulation"), false)
          ->add_option(WXOR(L"Disabled"), &aimbot_settings->manipulation_disabled)
          ->add_option(WXOR(L"Seed prediction"), &aimbot_settings->predict_server_random_seed)
          ->add_option(WXOR(L"Tapfire"), &aimbot_settings->spread_manipulation)
          ->set_help_text(WXOR(L"Seed prediction: Will try and predict the server's random seed.\nIf the random seed is predicted successfully it automatically nospread supported weapons.\nTapfire: Abuses game logic where the first bullet is always nospreaded."));

        gcs::slider(group, WXOR(L"Minimum hit chance"), WXOR(L"%"), 1, 100, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->spread_manipulation_hit_chance)
          ->set_render_on(&aimbot_settings->spread_manipulation);

        gcs::add_space(group);

        
      }
    }

    if(i != AIMBOT_WEAPON_SETTING_MELEE){
      if(current_class == TF_CLASS_SNIPER){
        if(i == AIMBOT_WEAPON_SETTING_HITSCAN)
          gcs::toggle(group, WXOR(L"Scoped only"), &aimbot_settings->sniper_scoped_only);

        gcs::dropdown(group, i == AIMBOT_WEAPON_SETTING_HITSCAN ? WXOR(L"Sniper Rifle: Wait for") : WXOR(L"Huntsman: Wait for"), false)
          ->add_option(WXOR(L"Kill"), &aimbot_settings->sniper_wait_for_charge)
          ->add_option(WXOR(L"Headshot"), &aimbot_settings->sniper_wait_for_headshot);
      }
    }

    if(i != AIMBOT_WEAPON_SETTING_PROJ && i != AIMBOT_WEAPON_SETTING_MEDIGUN)
      add_max_targets_options(group, current_class, i);

    {
      gcs::toggle(group, WXOR(L"Only aimbot"), &aimbot_settings->aimbot_only_enabled)
       ->add_module<gcs_component_hotkey>()
         ->set_hotkey_ptr(&aimbot_settings->aimbot_only_key)
         ->set_render_on(&aimbot_settings->aimbot_only_use_key);
  
      gcs::toggle(group, WXOR(L"Use key"), &aimbot_settings->aimbot_only_use_key)
        ->set_render_on(&aimbot_settings->aimbot_only_enabled);
  
      gcs::dropdown(group, WXOR(L"Targets"), true)
          ->add_option(WXOR(L"Prioritized"), &aimbot_settings->aimbot_only_prioritized)
          ->add_option(WXOR(L"Cheaters"), &aimbot_settings->aimbot_only_cheaters)
          ->add_option(WXOR(L"Bots"), &aimbot_settings->aimbot_only_bots)
            ->set_render_on(&aimbot_settings->aimbot_only_enabled);
    }

    gcs::add_space(group);


    if(current_class == TF_CLASS_MEDIC && i == AIMBOT_WEAPON_SETTING_PROJ){
      gcs::toggle(group, WXOR(L"Heal arrow"), &aimbot_settings->heal_arrow_enabled)
        ->add_module<gcs_component_hotkey>()
        ->set_hotkey_ptr(&aimbot_settings->heal_arrow_key)
        ->set_render_on(&aimbot_settings->heal_arrow_enabled)
        ->set_render_on(&aimbot_settings->heal_arrow_use_key);

      gcs::toggle(group, WXOR(L"Use key"), &aimbot_settings->heal_arrow_use_key)
        ->set_render_on(&aimbot_settings->heal_arrow_enabled);

      gcs::slider(group, WXOR(L"Maximum FOV"), WXOR(L"°"), 1, 180, GCS_SLIDER_TYPE_INT, &aimbot_settings->heal_arrow_max_fov)
        ->set_render_on(&aimbot_settings->heal_arrow_enabled);

      gcs::dropdown(group, WXOR(L"Heal arrow target"), false)
        ->add_option(WXOR(L"Any"), &aimbot_settings->heal_arrow_any)
        ->add_option(WXOR(L"Friends only"), &aimbot_settings->heal_arrow_friendsonly)
          ->set_render_on(&aimbot_settings->heal_arrow_enabled);

      gcs::toggle(group, WXOR(L"Team priority"), &aimbot_settings->heal_arrow_priority_team)
        ->set_render_on(&aimbot_settings->heal_arrow_enabled)
        ->set_help_text(WXOR(L"Focus team-mates over enemies depending flag settings."));

      gcs::dropdown(group, WXOR(L"Flags"), false)
        ->add_option(WXOR(L"In view"), &aimbot_settings->heal_arrow_priority_team_inview)
        ->add_option(WXOR(L"Any"), &aimbot_settings->heal_arrow_priority_team_any)
          ->set_render_on(&aimbot_settings->heal_arrow_enabled)
          ->set_render_on(&aimbot_settings->heal_arrow_priority_team)
          ->set_help_text(WXOR(L"In view: You'll only prioritze team-mates if in view.\nAny: Will always prioritize injured team-mates if valid."));

      gcs::toggle(group, WXOR(L"Automatic heal arrow"), &config->automation.automatic_heal_arrow)
        ->set_render_on(&aimbot_settings->heal_arrow_enabled)
        ->set_help_text(WXOR(L"Will automatically peform a heal arrow 'clutch'."));
      {
        gcs::toggle(group, WXOR(L"Friends only"), &config->automation.automatic_heal_arrow_friends_only)
          ->set_render_on(&aimbot_settings->heal_arrow_enabled)
          ->set_render_on(&config->automation.automatic_heal_arrow);
      }
    }
    else if(current_class == TF_CLASS_ENGINEER && i == AIMBOT_WEAPON_SETTING_MELEE){
      gcs::toggle(group, WXOR(L"Auto repair"), &config->automation.auto_repair)
        ->add_module<gcs_component_hotkey>()
        ->set_hotkey_ptr(&config->automation.auto_repair_key)
        ->set_render_on(&config->automation.auto_repair)
        ->set_render_on(&config->automation.auto_repair_use_key);

      gcs::toggle(group, WXOR(L"Use key"), &config->automation.auto_repair_use_key)
        ->set_render_on(&config->automation.auto_repair);

      gcs::toggle(group, WXOR(L"Allow rescue ranger"), &config->automation.auto_repair_rescue_ranger_enabled)
        ->set_render_on(&config->automation.auto_repair);

      gcs::slider(group, WXOR(L"Maximum FOV"), WXOR(L"°"), 1, 180, GCS_SLIDER_TYPE_INT, &config->automation.auto_repair_max_fov)
        ->set_render_on(&config->automation.auto_repair);
    }
    else if(current_class == TF_CLASS_SPY && i == AIMBOT_WEAPON_SETTING_HITSCAN){
      gcs::toggle(group, WXOR(L"Wait for spread"), &aimbot_settings->spy_ambassador_wait_for_spread);
    }
    else if(current_class == TF_CLASS_HEAVY && i == AIMBOT_WEAPON_SETTING_HITSCAN){
      gcs::toggle(group, WXOR(L"Spinup minigun when active"), &aimbot_settings->minigun_auto_rev);
    }
  }

  void add_max_targets_options(gcs_component_group* group, i32 current_class, i32 i){
    assert(group != nullptr);

    c_aimbot_settings* aimbot_settings = &config->aimbot[current_class - 1][i];
    if(aimbot_settings == nullptr)
      return;

    gcs::toggle(group, WXOR(L"Max possible targets"), &aimbot_settings->max_targets_enabled)
      ->set_help_text(WXOR(L"Limits how much aimbot targets the aimbot can go through before stopping.\nThis is recommended when using weapons that utilize arc prediction."));

    gcs::dropdown(group, WXOR(L"Mode"), false)
      ->add_option(WXOR(L"Automatic"), &aimbot_settings->max_targets_auto)
      ->add_option(WXOR(L"Manual"), &aimbot_settings->max_targets_manual)
      ->set_render_on(&aimbot_settings->max_targets_enabled)
      ->set_help_text(WXOR(L"Automatic: Will automatically decide targets based on CPU core count."));

    gcs::slider(group, WXOR(L"Count"), L"", 1, 6, GCS_SLIDER_TYPE_INT, &aimbot_settings->max_targets)
      ->set_render_on(&aimbot_settings->max_targets_enabled)
      ->set_render_on(&aimbot_settings->max_targets_manual);

    gcs::add_space(group);
  }

  CFLAG_O0 void handle_aimbot_class(gcs_component_group_host* group_host, i32 current_class, vec3i pos, vec3i size){
    assert(group_host != nullptr);
    assert(current_class > 0 && current_class <= TF_CLASS_MAX);

    std::wstring name = WXOR(L"Aimbot - ");
    name += utils::get_player_class_name(current_class);

    // Config array starts at zero. Just applying an offset.
    const bool proj_class     = current_class != TF_CLASS_SPY && current_class != TF_CLASS_HEAVY;//&& current_class != TF_CLASS_SCOUT;
    const bool splash_bot     = current_class != TF_CLASS_SCOUT && current_class != TF_CLASS_SPY && current_class != TF_CLASS_HEAVY && current_class != TF_CLASS_ENGINEER && current_class != TF_CLASS_MEDIC;
    const bool hitscan_class  = current_class != TF_CLASS_DEMOMAN && current_class != TF_CLASS_MEDIC;

    gcs_component_group* aimbot_group         = group_host->new_group(name);
    gcs_component_group* aimbot_group_hitscan = hitscan_class ? aimbot_group->new_category(WXOR(L"Hitscan")) : nullptr;
    gcs_component_group* aimbot_group_proj    = proj_class ? aimbot_group->new_category(WXOR(L"Projectile")) : nullptr;
    gcs_component_group* aimbot_group_medigun = current_class == TF_CLASS_MEDIC ? aimbot_group->new_category(WXOR(L"Medigun")) : nullptr;

    gcs_component_group* target_group         = group_host->new_group(WXOR(L"Targets"));
    gcs_component_group* target_group_hitscan = hitscan_class ? target_group->new_category(WXOR(L"Hitscan")) : nullptr;
    gcs_component_group* target_group_proj    = proj_class ? target_group->new_category(WXOR(L"Projectile")) : nullptr;
    gcs_component_group* target_group_medigun = current_class == TF_CLASS_MEDIC ? target_group->new_category(WXOR(L"Medigun")) : nullptr;
 
    gcs_component_group* prediction_group        = proj_class ? group_host->new_group(WXOR(L"Prediction")) : nullptr;
    gcs_component_group* prediction_main_group   = proj_class && prediction_group != nullptr ? prediction_group->new_category(WXOR(L"Main")) : nullptr;
    gcs_component_group* prediction_splash_group = (proj_class && splash_bot && prediction_group != nullptr) ? prediction_group->new_category(WXOR(L"Splashbot")) : nullptr;

    gcs_component_group* other_group         = group_host->new_group(WXOR(L"Other"));
    gcs_component_group* other_group_hitscan = hitscan_class ? other_group->new_category(WXOR(L"Hitscan")) : nullptr;
    gcs_component_group* other_group_proj    = proj_class ? other_group->new_category(WXOR(L"Projectile")) : nullptr;
    group_host->update();

    // Aimbot
    {
      if(hitscan_class)
        create_aimbot_main_layer(aimbot_group_hitscan, current_class, AIMBOT_WEAPON_SETTING_HITSCAN);

      if(proj_class)
        create_aimbot_main_layer(aimbot_group_proj, current_class, AIMBOT_WEAPON_SETTING_PROJ);

      if(current_class == TF_CLASS_MEDIC)
        create_aimbot_main_layer(aimbot_group_medigun, current_class, AIMBOT_WEAPON_SETTING_MEDIGUN);
    }

    // Target
    {
      if(hitscan_class)
        create_aimbot_target_layer(target_group_hitscan, current_class, AIMBOT_WEAPON_SETTING_HITSCAN);

      if(proj_class)
        create_aimbot_target_layer(target_group_proj, current_class, AIMBOT_WEAPON_SETTING_PROJ);

      if(current_class == TF_CLASS_MEDIC)
        create_aimbot_target_layer(target_group_medigun, current_class, AIMBOT_WEAPON_SETTING_MEDIGUN);
    }

    // Prediction
    {
      if(prediction_group != nullptr){
        c_aimbot_settings* aimbot_settings = &config->aimbot[current_class - 1][AIMBOT_WEAPON_SETTING_PROJ];

        gcs::slider(prediction_main_group, WXOR(L"Max travel time"), WXOR(L"ms"), 250, 2500, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->projectile_prediction_max_travel_time);
        gcs::slider(prediction_main_group, WXOR(L"Hit chance"), WXOR(L"%"), 1, 100, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->projectile_prediction_min_hit_chance);

        gcs::add_space(prediction_main_group);
        gcs::toggle(prediction_main_group, WXOR(L"Draw path"), &config->visual.projectile_prediction_show_path)
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.projectile_prediction_show_path_col);
        gcs::dropdown(prediction_main_group, WXOR(L"Path flags"), true)
          ->add_option(WXOR(L"Outline"), &config->visual.projectile_prediction_show_path_outline)
          ->add_option(WXOR(L"Show tick positions"), &config->visual.projectile_prediction_show_path_ticks)
            ->set_render_on(&config->visual.projectile_prediction_show_path);

        gcs::add_space(prediction_main_group);
        add_max_targets_options(prediction_main_group, current_class, AIMBOT_WEAPON_SETTING_PROJ);

        if(current_class == TF_CLASS_DEMOMAN){
          gcs::toggle(prediction_main_group, WXOR(L"Ignore if lethal"), &aimbot_settings->loose_cannon_lethal);
          gcs::slider(prediction_main_group, WXOR(L"Double donk above"), WXOR(L"ms"), 0, 1000, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->double_donk_above_ms);
        }

        gcs::toggle(prediction_main_group, WXOR(L"Predict counterstrafe"), &aimbot_settings->predict_evasion)
          ->set_help_text(WXOR(L"Detects WASD or AD movement and then averages it out."));

        // Splash bot
        if(prediction_splash_group != nullptr){
          gcs::add_key_bind_control(prediction_splash_group, WXOR(L"Splash bot"),
            &aimbot_settings->splash_bot,
            &aimbot_settings->splash_bot_use_key,
            &aimbot_settings->splash_bot_key_held,
            &aimbot_settings->splash_bot_key_toggle,
            &aimbot_settings->splash_bot_key_double_click,
            &aimbot_settings->splash_bot_key);

          gcs::dropdown(prediction_splash_group, WXOR(L"Mode"), false)
            ->add_option(WXOR(L"Use prediction"), &aimbot_settings->splash_bot_fallback)
            ->add_option(WXOR(L"No prediction position"), &aimbot_settings->splash_bot_prefer)
            ->set_render_on(&aimbot_settings->splash_bot)
            ->set_help_text(WXOR(L"Use prediction: Will use the player's predicted position based on the travel time of your projectile.\nNo prediction position: Use an averaged/current position depending on player movement."));

          if(current_class == TF_CLASS_SOLDIER)
            gcs::toggle(prediction_splash_group, WXOR(L"In air: Prefer ground"), &aimbot_settings->splash_assist);
        }
      }
    }

    // Other
    {
      if(hitscan_class)
        create_aimbot_other_layer(other_group_hitscan, current_class, AIMBOT_WEAPON_SETTING_HITSCAN);

      if(proj_class)
        create_aimbot_other_layer(other_group_proj, current_class, AIMBOT_WEAPON_SETTING_PROJ);
    }
  }

  void handle_aimbot_class_melee(gcs_component_group_host* group_host, i32 current_class, vec3i pos, vec3i size){
    assert(group_host != nullptr);
    assert(current_class > 0 && current_class <= TF_CLASS_MAX);

    std::wstring name = WXOR(L"Melee Aimbot - ");
    name += utils::get_player_class_name(current_class);

    // Config array starts at zero. Just applying an offset.
    bool proj_class     = current_class != TF_CLASS_SPY && current_class != TF_CLASS_HEAVY;//&& current_class != TF_CLASS_SCOUT;
    bool hitscan_class  = current_class != TF_CLASS_DEMOMAN && current_class != TF_CLASS_MEDIC;

    gcs_component_group* aimbot_group         = group_host->new_group(name);
    gcs_component_group* aimbot_group_melee   = aimbot_group->new_category(WXOR(L"Melee"));

    gcs_component_group* other_group         = group_host->new_group(WXOR(L"Other"));
    gcs_component_group* other_group_melee   = other_group->new_category(WXOR(L"Melee"));
    group_host->update();

    // Aimbot
    {
      create_aimbot_main_layer(aimbot_group_melee, current_class, AIMBOT_WEAPON_SETTING_MELEE);
    }

    // Other
    {

      create_aimbot_other_layer(other_group_melee, current_class, AIMBOT_WEAPON_SETTING_MELEE);
      create_aimbot_target_layer(other_group_melee, current_class, AIMBOT_WEAPON_SETTING_MELEE);
    }
  }

  gcs_component_base* create_aimbot_main(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_canvas* root_panel = gcs::canvas(comp, WXOR(L"Main"), pos, size, false);
    {
      assert(root_panel != nullptr);

      root_panel->add_hook(GCS_HOOK_POST_PAINT, [](gcs_component_base* c, void* p){
        if(global->aimbot_settings_filled)
          return true;

        vec3i text_size = c->font()->draw(c->pos() + (c->size() / 2), 20, WXOR(L"Please select a class in-game."), c->scheme()->text, TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_CENTER);
        c->emoji()->draw(c->pos() + (c->size() / 2) - vec3i(0, text_size.y), 45, WXOR(L"⚠️"), c->scheme()->warning, TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_TOP);

        return true;
      });

      for(u32 i = 0; i < TF_CLASS_MAX; i++){
        i32   current_class = i + 1;
        void* buffer        = malloc(0x4);
        memset(buffer, 0, 0x4);

        *(i32*)((uptr)buffer) = current_class;

        assert(current_class > 0 && current_class <= TF_CLASS_MAX);
        gcs_component_group_host* group_host = gcs::group_host(root_panel, WXOR(L"Main"), vec3i(), size);
        {
          assert(group_host != nullptr);
          group_host->info()->param_buffer = buffer;
          group_host->add_hook(GCS_HOOK_SHOULD_ENABLE, [](gcs_component_base* c, void* p){
            return global->aimbot_settings_filled && *(i32*)c->info()->param_buffer == global->current_player_class;
          });

          handle_aimbot_class(group_host, current_class, pos, size);
        }
      }
    }

    return root_panel;
  }

  gcs_component_base* create_aimbot_melee(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_canvas* root_panel = gcs::canvas(comp, WXOR(L"Melee"), pos, size, false);
    {
      assert(root_panel != nullptr);

      root_panel->add_hook(GCS_HOOK_POST_PAINT, [](gcs_component_base* c, void* p){
        if(global->aimbot_settings_filled)
          return true;

        vec3i text_size = c->font()->draw(c->pos() + (c->size() / 2), 20, WXOR(L"Please select a class in-game."), c->scheme()->text, TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_CENTER);
        c->emoji()->draw(c->pos() + (c->size() / 2) - vec3i(0, text_size.y), 45, WXOR(L"⚠️"), c->scheme()->warning, TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_TOP);

        return true;
      });

      for(u32 i = 0; i < TF_CLASS_MAX; i++){
        i32   current_class = i + 1;

        void* buffer        = malloc(0x4);
        memset(buffer, 0, 0x4);

        *(i32*)((uptr)buffer) = current_class;

        assert(current_class > 0 && current_class <= TF_CLASS_MAX);
        gcs_component_group_host* group_host = gcs::group_host(root_panel, WXOR(L"Melee"), vec3i(), size);
        {
          assert(group_host != nullptr);
          group_host->info()->param_buffer = buffer;
          group_host->add_hook(GCS_HOOK_SHOULD_ENABLE, [](gcs_component_base* c, void* p){
            return global->aimbot_settings_filled && *(i32*)c->info()->param_buffer == global->current_player_class;
          });

          handle_aimbot_class_melee(group_host, current_class, pos, size);
        }
      }
    }

    return root_panel;
  }

  gcs_component_base* create_triggerbot(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);
    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Triggerbot"), pos, size);
    {
      gcs_component_group* g1         = group_host->new_group(WXOR(L"General"));
      gcs_component_group* g2         = group_host->new_group(WXOR(L"Targets"));

      gcs_component_group* main_group = g1->new_category(WXOR(L"General"));
      gcs_component_group* target_group = g2->new_category(WXOR(L"Targets"));

      // General.
      {
        gcs::toggle(main_group, WXOR(L"Enabled"), &config->triggerbot.enabled)
          ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&config->triggerbot.key)
          ->set_render_on(&config->triggerbot.enabled)
          ->set_render_on(&config->triggerbot.use_key);

        gcs::toggle(main_group, WXOR(L"Use key"), &config->triggerbot.use_key)
          ->set_render_on(&config->triggerbot.enabled);

        gcs::dropdown(main_group, WXOR(L"Key control"), false)
          ->add_option(WXOR(L"Held"), &config->triggerbot.key_held)
          ->add_option(WXOR(L"Toggle"), &config->triggerbot.key_toggle)
          ->add_option(WXOR(L"Double click"), &config->triggerbot.key_double_click)
          ->set_render_on(&config->triggerbot.enabled)
          ->set_render_on(&config->triggerbot.use_key);

        gcs::add_space(main_group);

        gcs::toggle(main_group, WXOR(L"Show traces"), &config->triggerbot.show_trigger_bot_traces)
          ->set_render_on(&config->triggerbot.enabled)
          ->set_help_text(WXOR(L"Will show 3D boxes of how the triggerbot does traces per tick.\nHelp explain why you might have a triggerbot failure due to you skipping over an entity."));
      }

      // Targets
      {

        gcs::dropdown(target_group, WXOR(L"Ignore hitbox"), true)
          ->add_option(WXOR(L"Limbs"), &config->triggerbot.hitbox_ignore_limbs)
          ->add_option(WXOR(L"Body"), &config->triggerbot.hitbox_ignore_body)
          ->add_option(WXOR(L"Head"), &config->triggerbot.hitbox_ignore_head);

        gcs::dropdown(target_group, WXOR(L"Ignore class"), true)
          ->add_option(WXOR(L"Scout"), &config->triggerbot.ignore_player_class[TF_CLASS_SCOUT])
          ->add_option(WXOR(L"Soldier"), &config->triggerbot.ignore_player_class[TF_CLASS_SOLDIER])
          ->add_option(WXOR(L"Pyro"), &config->triggerbot.ignore_player_class[TF_CLASS_PYRO])
          ->add_option(WXOR(L"Demoman"), &config->triggerbot.ignore_player_class[TF_CLASS_DEMOMAN])
          ->add_option(WXOR(L"Heavy"), &config->triggerbot.ignore_player_class[TF_CLASS_HEAVY])
          ->add_option(WXOR(L"Engineer"), &config->triggerbot.ignore_player_class[TF_CLASS_ENGINEER])
          ->add_option(WXOR(L"Medic"), &config->triggerbot.ignore_player_class[TF_CLASS_MEDIC])
          ->add_option(WXOR(L"Sniper"), &config->triggerbot.ignore_player_class[TF_CLASS_SNIPER])
          ->add_option(WXOR(L"Spy"), &config->triggerbot.ignore_player_class[TF_CLASS_SPY]);

        gcs::add_space(target_group);

        gcs::dropdown(target_group, WXOR(L"Shoot at"), true)
          ->add_option(WXOR(L"Friends"), &config->triggerbot.shoot_friends)
          ->add_option(WXOR(L"Sentry"), &config->triggerbot.shoot_sentry)
          ->add_option(WXOR(L"Dispenser"), &config->triggerbot.shoot_dispenser)
          ->add_option(WXOR(L"Teleporter"), &config->triggerbot.shoot_teleporter)
          ->add_option(WXOR(L"Boss/Tank"), &config->triggerbot.shoot_boss)
          ->add_option(WXOR(L"Sticky"), &config->triggerbot.shoot_sticky);

        gcs::dropdown(target_group, WXOR(L"Shoot at conditions"), true)
          ->add_option(WXOR(L"Cloaked"), &config->triggerbot.shoot_cloaked)
          ->add_option(WXOR(L"Disguised"), &config->triggerbot.shoot_disguised)
          ->add_option(WXOR(L"Bonked"), &config->triggerbot.shoot_bonked)
          ->add_option(WXOR(L"Ubercharged"), &config->triggerbot.shoot_uber);

      }


      group_host->update();
    }

    return group_host;
  }

  gcs_component_group_host* create_medic_automation(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Medic"), pos, size);
    {
      gcs_component_group* g1                     = group_host->new_group(WXOR(L"Medigun"));
      gcs_component_group* auto_vaccinator_group  = g1->new_category(WXOR(L"Vaccinator"));
      gcs_component_group* auto_uber_group        = g1->new_category(WXOR(L"Uber"));
      gcs_component_group* auto_kritz_group       = g1->new_category(WXOR(L"Kritzkrieg"));


      gcs_component_group* g2                     = group_host->new_group(WXOR(L"Automations"));
      gcs_component_group* medic_automation_group = g2->new_category(WXOR(L"General"));
      gcs_component_group* mvm_medic_shield_group = g2->new_category(WXOR(L"MvM: Medic Shield"));

      group_host->update();
      
      // HINT-HEADER: Auto vaccinator.
      {
        gcs::toggle(auto_vaccinator_group, WXOR(L"Enabled"), &config->automation.auto_vaccinator)
          ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&config->automation.auto_vaccinator_key)
          ->set_render_on(&config->automation.auto_vaccinator)
          ->set_render_on(&config->automation.auto_vaccinator_use_key);

        gcs::toggle(auto_vaccinator_group, WXOR(L"Use key"), &config->automation.auto_vaccinator_use_key)
          ->set_render_on(&config->automation.auto_vaccinator);

        gcs::dropdown(auto_vaccinator_group, WXOR(L"Key control"), false)
          ->add_option(WXOR(L"Held"), &config->automation.auto_vaccinator_key_held)
          ->add_option(WXOR(L"Toggle"), &config->automation.auto_vaccinator_key_toggle)
          ->add_option(WXOR(L"Double click"), &config->automation.auto_vaccinator_key_double_click)
          ->set_render_on(&config->automation.auto_vaccinator)
          ->set_render_on(&config->automation.auto_vaccinator_use_key);

        gcs::toggle(auto_vaccinator_group, WXOR(L"Friends only"), &config->automation.auto_vaccinator_friendsonly)
          ->set_render_on(&config->automation.auto_vaccinator);

        gcs::dropdown(auto_vaccinator_group, WXOR(L"Control"), false)
          ->add_option(WXOR(L"Full"), &config->automation.auto_vaccinator_fullcontrol)
          ->add_option(WXOR(L"Passive"), &config->automation.auto_vaccinator_passive)
          ->set_render_on(&config->automation.auto_vaccinator)
          ->set_help_text(WXOR(L"Full: This will take full control of the vaccinator.\nPassive: Will let you have control and only react to lethal threats."));

        gcs::dropdown(auto_vaccinator_group, WXOR(L"React"), true)
          ->add_option(WXOR(L"Friends"), &config->automation.auto_vaccinator_react_friends)
          ->add_option(WXOR(L"Bonked"), &config->automation.auto_vaccinator_react_bonked)
          ->add_option(WXOR(L"Cloaked"), &config->automation.auto_vaccinator_react_cloaked)
          ->set_render_on(&config->automation.auto_vaccinator);

        // Sensitivity
        {
          gcs::slider(auto_vaccinator_group, WXOR(L"Bullet sensitivity"), WXOR(L"%"), -100, 100, GCS_SLIDER_TYPE_INT, &config->automation.auto_vaccinator_sensitivity_bullet)
            ->set_render_on(&config->automation.auto_vaccinator);

          gcs::slider(auto_vaccinator_group, WXOR(L"Blast sensitivity"), WXOR(L"%"), -100, 100, GCS_SLIDER_TYPE_INT, &config->automation.auto_vaccinator_sensitivity_blast)
            ->set_render_on(&config->automation.auto_vaccinator);

          gcs::slider(auto_vaccinator_group, WXOR(L"Fire sensitivity"), WXOR(L"%"), -100, 100, GCS_SLIDER_TYPE_INT, &config->automation.auto_vaccinator_sensitivity_fire)
            ->set_render_on(&config->automation.auto_vaccinator);
        }

        gcs::dropdown(auto_vaccinator_group, WXOR(L"Healing resistance"), false)
          ->add_option(WXOR(L"Auto"), &config->automation.auto_vaccinator_passive_resist_none)
          ->add_option(WXOR(L"Bullet"), &config->automation.auto_vaccinator_passive_resist_bullet)
          ->add_option(WXOR(L"Blast"), &config->automation.auto_vaccinator_passive_resist_blast)
          ->add_option(WXOR(L"Fire"), &config->automation.auto_vaccinator_passive_resist_fire)
          ->set_render_on(&config->automation.auto_vaccinator)
          ->set_render_on(&config->automation.auto_vaccinator_fullcontrol)
          ->set_help_text(WXOR(L"If not set to Auto, and no threats are detected, It will default to this resistance."));

        gcs::dropdown(auto_vaccinator_group, WXOR(L"Manual uber resistance"), false)
          ->add_option(WXOR(L"Disallow"), &config->automation.auto_vaccinator_manualcharge_resist_none)
          ->add_option(WXOR(L"Bullet"), &config->automation.auto_vaccinator_manualcharge_resist_bullet)
          ->add_option(WXOR(L"Blast"), &config->automation.auto_vaccinator_manualcharge_resist_blast)
          ->add_option(WXOR(L"Fire"), &config->automation.auto_vaccinator_manualcharge_resist_fire)
          ->set_render_on(&config->automation.auto_vaccinator)
          ->set_render_on(&config->automation.auto_vaccinator_fullcontrol)
          ->set_help_text(WXOR(L"When not set to Disallow, it'll automatically swap to the selected resistance above and pop."));

        gcs::toggle(auto_vaccinator_group, WXOR(L"Resist cycle swaps type"), &config->automation.auto_vaccinator_manualcharge_resist_cycle_option)
          ->set_render_on(&config->automation.auto_vaccinator)
          ->set_render_on(&config->automation.auto_vaccinator_fullcontrol)
          ->set_help_text(WXOR(L"When enabled and you press your reload key it'll cycle the 'Manual uber resistance' option."));

        gcs::dropdown(auto_vaccinator_group, WXOR(L"Disallow resistance"), true)
          ->add_option(WXOR(L"Bullet"), &config->automation.auto_vaccinator_disallow_resist_bullet)
          ->add_option(WXOR(L"Blast"), &config->automation.auto_vaccinator_disallow_resist_blast)
          ->add_option(WXOR(L"Fire"), &config->automation.auto_vaccinator_disallow_resist_fire)
          ->set_render_on(&config->automation.auto_vaccinator);
      }

      // HINT-HEADER: Auto uber
      {
        gcs::toggle(auto_uber_group, WXOR(L"Enabled"), &config->automation.medic_auto_uber);
        {
          gcs::toggle(auto_uber_group, WXOR(L"Friends only"), &config->automation.auto_uber_friendsonly)
            ->set_render_on(&config->automation.medic_auto_uber);

          gcs::dropdown(auto_uber_group, WXOR(L"React"), true)
            ->add_option(WXOR(L"Friends"), &config->automation.auto_uber_react_friends)
            ->add_option(WXOR(L"Bonked"), &config->automation.auto_uber_react_bonked)
            ->add_option(WXOR(L"Cloaked"), &config->automation.auto_uber_react_cloaked)
            ->set_render_on(&config->automation.medic_auto_uber);

          gcs::slider(auto_uber_group, WXOR(L"Visible threat multiplier"), 25, 100, GCS_SLIDER_TYPE_FLOAT, &config->automation.auto_uber_visible_enemy_threat_multiplier)
            ->set_render_on(&config->automation.medic_auto_uber);

          gcs::slider(auto_uber_group, WXOR(L"Nearby threat multiplier"), 25, 100, GCS_SLIDER_TYPE_FLOAT, &config->automation.auto_uber_nearby_enemy_threat_multiplier)
            ->set_render_on(&config->automation.medic_auto_uber);
        }
      }

      // HINT-HEADER: Auto kritzkrieg
      {
        gcs::toggle(auto_kritz_group, WXOR(L"Enabled"), &config->automation.medic_auto_kritz);
        {
          gcs::toggle(auto_kritz_group, WXOR(L"Friends only"), &config->automation.auto_kritz_friendsonly)
            ->set_render_on(&config->automation.medic_auto_kritz);

          gcs::dropdown(auto_kritz_group, WXOR(L"React"), true)
            ->add_option(WXOR(L"Friends"), &config->automation.auto_kritz_react_friends)
            ->add_option(WXOR(L"Bonked"), &config->automation.auto_kritz_react_bonked)
            ->add_option(WXOR(L"Cloaked"), &config->automation.auto_kritz_react_cloaked)
            ->set_render_on(&config->automation.medic_auto_kritz);

          gcs::toggle(auto_kritz_group, WXOR(L"Use charge near tank"), &config->automation.auto_kritz_deploy_near_tank)
            ->set_render_on(&config->automation.medic_auto_kritz);
        }
      }

      // General.
      {
        gcs::toggle(medic_automation_group, WXOR(L"Quickfix follows demo-charge"), &config->automation.medic_follow_demo_charge);
        gcs::add_space(medic_automation_group);
        gcs::toggle(medic_automation_group, WXOR(L"Uber on 'Activate Charge!'"), &config->automation.medic_uber_active_charge);
        gcs::toggle(medic_automation_group, WXOR(L"Friends only"), &config->automation.medic_uber_active_charge_friendsonly)
          ->set_render_on(&config->automation.medic_uber_active_charge);

        gcs::dropdown(medic_automation_group, WXOR(L"'Activate charge' resistance"), false)
          ->add_option(WXOR(L"Bullet"), &config->automation.medic_uber_active_charge_resist_bullet)
          ->add_option(WXOR(L"Blast"), &config->automation.medic_uber_active_charge_resist_blast)
          ->add_option(WXOR(L"Fire"), &config->automation.medic_uber_active_charge_resist_fire)
          ->set_render_on(&config->automation.medic_uber_active_charge);
      }

      // Auto medic shield.
      {
        gcs::toggle(mvm_medic_shield_group, WXOR(L"Enabled"), &config->automation.auto_shield);
        {
          gcs::toggle(mvm_medic_shield_group, WXOR(L"Auto deploy"), &config->automation.auto_shield_auto_deploy)
            ->set_render_on(&config->automation.auto_shield);

          gcs::slider(mvm_medic_shield_group, WXOR(L"Entities required"), 0, 35, GCS_SLIDER_TYPE_INT, &config->automation.auto_shield_deploy_at_count)
            ->set_render_on(&config->automation.auto_shield)
            ->set_render_on(&config->automation.auto_shield_auto_deploy);
        }
      }

    }

    return group_host;
  }

  gcs_component_group_host* create_pyro_automation(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Pyro"), pos, size);
    {
      gcs_component_group* g1                          = group_host->new_group(WXOR(L"Auto reflect"));
      gcs_component_group* auto_reflect_group          = g1->new_category(WXOR(L"General"));
      gcs_component_group* auto_reflect_target_group   = g1->new_category(WXOR(L"Targeting"));
      gcs_component_group* auto_reflect_redirect_group = g1->new_category(WXOR(L"Redirection"));

      gcs_component_group* g2                          = group_host->new_group(WXOR(L"Automations"));
      gcs_component_group* automation_auto_det_group   = g2->new_category(WXOR(L"Auto detonator"));

      group_host->update();
     
      // General
      {
        gcs::toggle(auto_reflect_group, WXOR(L"Enabled"), &config->automation.auto_reflect)
          ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&config->automation.auto_reflect_key)
          ->set_render_on(&config->automation.auto_reflect)
          ->set_render_on(&config->automation.auto_reflect_use_key);

        gcs::toggle(auto_reflect_group, WXOR(L"Use key"), &config->automation.auto_reflect_use_key)
          ->set_render_on(&config->automation.auto_reflect);

        gcs::toggle(auto_reflect_group, WXOR(L"Aimbot"), &config->automation.auto_reflect_aimbot)
          ->set_help_text(WXOR(L"Enabled: Will aimbot reflect the projectile\nDisabled: Will only reflect if it's with in the air-blast FOV."));

        gcs::slider(auto_reflect_group, WXOR(L"Maximum FOV"), WXOR(L"°"), 1, 180, GCS_SLIDER_TYPE_INT, &config->automation.auto_reflect_fov)
          ->set_render_on(&config->automation.auto_reflect)
          ->set_render_on(&config->automation.auto_reflect_aimbot);

        gcs::slider(auto_reflect_group, WXOR(L"Reflect if ammo above"), WXOR(L""), 0, 180, GCS_SLIDER_TYPE_INT, &config->automation.auto_reflect_ammo_required)
          ->set_render_on(&config->automation.auto_reflect);

        gcs::add_space(auto_reflect_group, &config->automation.auto_reflect);

        gcs::toggle(auto_reflect_group, WXOR(L"Disable when ubercharged"), &config->automation.auto_reflect_disable_ubered)
          ->set_render_on(&config->automation.auto_reflect);

        gcs::toggle(auto_reflect_group, WXOR(L"Disable when forcing a crit"), &config->automation.auto_reflect_disable_forcing_crits)
          ->set_render_on(&config->automation.auto_reflect);

        gcs::toggle(auto_reflect_group, WXOR(L"Disable with dragon's fury"), &config->automation.auto_reflect_disable_dragonsfury)
          ->set_render_on(&config->automation.auto_reflect);
      }

      // Targeting
      {
        gcs::dropdown(auto_reflect_target_group, WXOR(L"Projectile targets"), true)
          ->add_option(WXOR(L"Rockets"), &config->automation.auto_reflect_target_rockets)
          ->add_option(WXOR(L"Flares"), &config->automation.auto_reflect_target_flares)
          ->add_option(WXOR(L"Pills"), &config->automation.auto_reflect_target_pills)
          ->add_option(WXOR(L"Stickies"), &config->automation.auto_reflect_target_stickies)
          ->add_option(WXOR(L"Arrows"), &config->automation.auto_reflect_target_arrows)
          ->add_option(WXOR(L"Rescue bolts"), &config->automation.auto_reflect_target_mechbolts)
          ->add_option(WXOR(L"Healing bolts"), &config->automation.auto_reflect_heal_bolts)
          ->add_option(WXOR(L"Throwables"), &config->automation.auto_reflect_target_throwables)
          ->add_option(WXOR(L"Flameball"), &config->automation.auto_reflect_target_flameball)
          ->add_option(WXOR(L"Mechorb"), &config->automation.auto_reflect_target_mechorb);

        gcs::add_space(auto_reflect_target_group);
        gcs::dropdown(auto_reflect_target_group, WXOR(L"Enemy players"), false)
          ->add_option(WXOR(L"None"), &config->automation.auto_reflect_enemy_players_none)
          ->add_option(WXOR(L"Any player"), &config->automation.auto_reflect_enemy_players)
          ->add_option(WXOR(L"Players with Melee"), &config->automation.auto_reflect_enemy_players_meleeonly);

        gcs::add_space(auto_reflect_target_group);
        gcs::dropdown(auto_reflect_target_group, WXOR(L"Burning players"), false)
          ->add_option(WXOR(L"None"), &config->automation.auto_reflect_burning_players_none)
          ->add_option(WXOR(L"Any player"), &config->automation.auto_reflect_burning_players)
          ->add_option(WXOR(L"(Im/Them) are injured"), &config->automation.auto_reflect_burning_players_ifhurt);
      }

      // Redirection
      {
        gcs::toggle(auto_reflect_redirect_group, WXOR(L"Enabled"), &config->automation.auto_reflect_redirect_towards)
          ->set_render_on(&config->automation.auto_reflect);

        gcs::dropdown(auto_reflect_redirect_group, WXOR(L"Towards"), true)
          ->add_option(WXOR(L"Players"), &config->automation.auto_reflect_redirect_towards_players)
          ->add_option(WXOR(L"Sentry"), &config->automation.auto_reflect_redirect_towards_sentry)
          ->add_option(WXOR(L"Dispenser"), &config->automation.auto_reflect_redirect_towards_dispenser)
          ->add_option(WXOR(L"Teleporter"), &config->automation.auto_reflect_redirect_towards_teleporter)
          ->add_option(WXOR(L"Projectiles"), &config->automation.auto_reflect_redirect_towards_projectiles)
          ->add_option(WXOR(L"Boss/Tank"), &config->automation.auto_reflect_redirect_towards_boss)
          ->set_render_on(&config->automation.auto_reflect)
          ->set_render_on(&config->automation.auto_reflect_redirect_towards);

        gcs::dropdown(auto_reflect_redirect_group, WXOR(L"Ignore"), true)
          ->add_option(WXOR(L"Friends"), &config->automation.auto_reflect_redirect_ignore_friends)
          ->add_option(WXOR(L"Cloaked"), &config->automation.auto_reflect_redirect_ignore_cloaked)
          ->set_render_on(&config->automation.auto_reflect)
          ->set_render_on(&config->automation.auto_reflect_redirect_towards);

        gcs::toggle(auto_reflect_redirect_group, WXOR(L"Redirect heal arrows to team"), &config->automation.auto_reflect_redirect_healbolt_teammates)
          ->set_render_on(&config->automation.auto_reflect)
          ->set_render_on(&config->automation.auto_reflect_redirect_towards);
      }

      // Auto detonator.
      {
        gcs::toggle(automation_auto_det_group, WXOR(L"Auto detonator"), &config->automation.auto_detonator);
        gcs::dropdown(automation_auto_det_group, WXOR(L"Target flags"), true)
          ->add_option(WXOR(L"Friend"), &config->automation.auto_detonator_friends)
          ->add_option(WXOR(L"Cloaked"), &config->automation.auto_detonator_cloaked)
          ->add_option(WXOR(L"Sentry"), &config->automation.auto_detonator_sentry)
          ->add_option(WXOR(L"Dispenser"), &config->automation.auto_detonator_dispenser)
          ->add_option(WXOR(L"Teleporter"), &config->automation.auto_detonator_teleporter)
          ->add_option(WXOR(L"Sticky"), &config->automation.auto_detonator_sticky)
          ->add_option(WXOR(L"Boss/Tank"), &config->automation.auto_detonator_boss)
          ->set_render_on(&config->automation.auto_detonator);
      }
    }

    return group_host;
  }

  gcs_component_group_host* create_demo_automation(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);
    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Demoman"),pos, size);
    {
      gcs_component_group* g1                   = group_host->new_group(WXOR(L"Sticky Launcher"));
      gcs_component_group* auto_sticky_launcher = g1->new_category(WXOR(L"Sticky Launcher"));

      gcs_component_group* g2         = group_host->new_group(WXOR(L"Charge bot"));
      gcs_component_group* charge_bot = g2->new_category(WXOR(L"Charge bot"));

      group_host->update();

      // Sticky launcher.
      {
        gcs::add_key_bind_control(auto_sticky_launcher,
          WXOR(L"Auto sticky detonation"),
          &config->automation.auto_sticky_detonate,
          &config->automation.auto_sticky_detonate_use_key,
          &config->automation.auto_sticky_detonate_key_held,
          &config->automation.auto_sticky_detonate_key_toggle,
          &config->automation.auto_sticky_detonate_key_double_click,
          &config->automation.auto_sticky_detonate_key);

        gcs::slider(auto_sticky_launcher, WXOR(L"Blast radius size"), WXOR(L"%"), 10, 100, GCS_SLIDER_TYPE_FLOAT, &config->automation.auto_sticky_blast_radius)
          ->set_render_on(&config->automation.auto_sticky_detonate);
        gcs::toggle(auto_sticky_launcher, WXOR(L"Prevent self damage"), &config->automation.auto_sticky_prevent_self_dmg)
          ->set_render_on(&config->automation.auto_sticky_detonate);
        gcs::slider(auto_sticky_launcher, WXOR(L"Minimum stickies"), L"", 1, 4, GCS_SLIDER_TYPE_INT, &config->automation.auto_sticky_min_bomb_count)
          ->set_render_on(&config->automation.auto_sticky_detonate);

        gcs::dropdown(auto_sticky_launcher, WXOR(L"Target flags"), true)
          ->add_option(WXOR(L"Friend"), &config->automation.auto_sticky_target_friend)
          ->add_option(WXOR(L"Cloaked"), &config->automation.auto_sticky_target_cloaked)
          ->add_option(WXOR(L"Sentry"), &config->automation.auto_sticky_target_sentry)
          ->add_option(WXOR(L"Dispenser"), &config->automation.auto_sticky_target_dispenser)
          ->add_option(WXOR(L"Teleporter"), &config->automation.auto_sticky_target_teleporter)
          ->add_option(WXOR(L"Sticky"), &config->automation.auto_sticky_target_sticky)
          ->add_option(WXOR(L"Boss"), &config->automation.auto_sticky_target_boss)
          ->set_render_on(&config->automation.auto_sticky_detonate);

        gcs::add_space(auto_sticky_launcher);
        gcs::toggle(auto_sticky_launcher, WXOR(L"Auto sticky release"), &config->automation.auto_sticky_spam)
          ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&config->automation.auto_sticky_spam_key)
          ->set_render_on(&config->automation.auto_sticky_spam)
          ->set_render_on(&config->automation.auto_sticky_spam_use_key);

        gcs::toggle(auto_sticky_launcher, WXOR(L"Use key"), &config->automation.auto_sticky_spam_use_key)
          ->set_render_on(&config->automation.auto_sticky_spam);

        gcs::slider(auto_sticky_launcher, WXOR(L"Release charge at"), WXOR(L"%"), 1, 100, GCS_SLIDER_TYPE_INT, &config->automation.auto_sticky_spam_at_charge)
          ->set_render_on(&config->automation.auto_sticky_spam);

        gcs::toggle(auto_sticky_launcher, WXOR(L"Auto release near entity"), &config->automation.auto_sticky_spam_auto_release_near_entity)
          ->set_render_on(&config->automation.auto_sticky_spam);

        gcs::toggle(auto_sticky_launcher, WXOR(L"Set release charge at"), &config->automation.auto_sticky_spam_set_charge_release_at)
          ->set_render_on(&config->automation.auto_sticky_spam)
          ->set_render_on(&config->automation.auto_sticky_spam_auto_release_near_entity)
            ->set_help_text(WXOR(L"If it finds an entity it'll set the release charge at value in the menu settings.")); 
      }

      // Charge bot.
      {
         gcs::toggle(charge_bot, WXOR(L"Charge bot"), &config->automation.demo_charge_bot)
          ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&config->automation.demo_charge_bot_key)
          ->set_render_on(&config->automation.demo_charge_bot_use_key)
          ->set_render_on(&config->automation.demo_charge_bot);

        gcs::toggle(charge_bot, WXOR(L"Use key"), &config->automation.demo_charge_bot_use_key)
          ->set_render_on(&config->automation.demo_charge_bot);
      }

    }
    return group_host;
  }

  gcs_component_group_host* create_misc_automation(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Others"), pos, size);
    {
      gcs_component_group* g1                    = group_host->new_group(WXOR(L"General"));
      gcs_component_group* misc_automation_group = g1->new_category(WXOR(L"General"));


      gcs_component_group* g2                            = group_host->new_group(WXOR(L"Class related"));
      gcs_component_group* misc_automation_spy_group     = g2->new_category(WXOR(L"Spy"));
      gcs_component_group* misc_automation_heavy_group   = g2->new_category(WXOR(L"Heavy"));
      gcs_component_group* misc_automation_soldier_group = g2->new_category(WXOR(L"Soldier"));
      gcs_component_group* misc_automation_sniper_group  = g2->new_category(WXOR(L"Sniper"));

      group_host->update();

      // General.
      {
        gcs::toggle(misc_automation_group, WXOR(L"Auto vote"), &config->misc.auto_vote_enabled);
        {
          gcs::dropdown(misc_automation_group, WXOR(L"Voting flags"), true)
            ->add_option(WXOR(L"Auto vote yes"), &config->misc.auto_vote_castyes)
            ->add_option(WXOR(L"Auto vote no"), &config->misc.auto_vote_castno)
            ->add_option(WXOR(L"Defensive"), &config->misc.auto_vote_autocast_defensive)
            ->set_render_on(&config->misc.auto_vote_enabled);

          gcs::dropdown(misc_automation_group, WXOR(L"Auto cast"), false)
           ->add_option(WXOR(L"Disabled"), &config->misc.auto_vote_autocast_disabled)
           ->add_option(WXOR(L"Random player"), &config->misc.auto_vote_autocast_random)
           ->add_option(WXOR(L"Bots only"), &config->misc.auto_vote_autocast_botsonly)
           ->add_option(WXOR(L"Any cheaters"), &config->misc.auto_vote_autocast_cheatersonly)
           ->set_render_on(&config->misc.auto_vote_enabled);
        }

        gcs::add_space(misc_automation_group);
        gcs::toggle(misc_automation_group, WXOR(L"Backtrack to crosshair"), &config->automation.backtrack_to_crosshair);
        gcs::toggle(misc_automation_group, WXOR(L"Auto call medic"), &config->automation.auto_call_medic);
        gcs::toggle(misc_automation_group, WXOR(L"Auto mute bots"), &config->misc.auto_mute_bots);
        gcs::toggle(misc_automation_group, WXOR(L"Auto avoid team balance"), &config->misc.auto_retry_teambalance);
        gcs::add_space(misc_automation_group);

        gcs::toggle(misc_automation_group, WXOR(L"Auto format ds_dir"), &config->misc.demosystem_format_ds_dir);
        gcs::toggle(misc_automation_group, WXOR(L"Auto record hvh matches"), &config->misc.auto_record_hvh_matches);
      }

      // Spy
      {
        gcs::toggle(misc_automation_spy_group, WXOR(L"Auto disguise"), &config->automation.auto_disguise);
        gcs::slider(misc_automation_spy_group, WXOR(L"Delay"), WXOR(L"ms"), 200, 5000, GCS_SLIDER_TYPE_FLOAT, &config->automation.auto_disguise_delay)
          ->set_render_on(&config->automation.auto_disguise);

        gcs::add_space(misc_automation_spy_group);
        gcs::toggle(misc_automation_spy_group, WXOR(L"Auto sapper"), &config->automation.auto_sapper);
        gcs::dropdown(misc_automation_spy_group, WXOR(L"Target flags"), true)
          ->add_option(WXOR(L"Robots"), &config->automation.auto_sapper_target_robots)
          ->add_option(WXOR(L"Sentry"), &config->automation.auto_sapper_target_sentry)
          ->add_option(WXOR(L"Dispenser"), &config->automation.auto_sapper_target_dispenser)
          ->add_option(WXOR(L"Teleporter"), &config->automation.auto_sapper_target_teleporter)
          ->set_render_on(&config->automation.auto_sapper);

        gcs::slider(misc_automation_spy_group, WXOR(L"Minimum robots"), L"", 1, 8, GCS_SLIDER_TYPE_INT, &config->automation.auto_sapper_min_robots)
          ->set_render_on(&config->automation.auto_sapper)
          ->set_render_on(&config->automation.auto_sapper_target_robots);
      }

      // Heavy
      {
        gcs::toggle(misc_automation_heavy_group, WXOR(L"Conserve huo-long heater ammo"), &config->automation.auto_conserve_hlh_ammo);
        gcs::slider(misc_automation_heavy_group, WXOR(L"Range multiplier"), WXOR(L"x"), 1, 16, GCS_SLIDER_TYPE_INT, &config->automation.auto_conserve_hlh_range_mult)
          ->set_render_on(&config->automation.auto_conserve_hlh_ammo);

        gcs::add_space(misc_automation_heavy_group);
        gcs::toggle(misc_automation_heavy_group, WXOR(L"Auto rev jump"), &config->misc.auto_rev_jump);
      }

      // Soldier
      {
        gcs::toggle(misc_automation_soldier_group, WXOR(L"Auto soldier whip"), &config->automation.auto_soldier_whip);

        gcs::add_space(misc_automation_soldier_group);
        gcs::toggle(misc_automation_soldier_group, WXOR(L"Auto rocket jumper"), &config->automation.auto_rocket_jumper)
          ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&config->automation.auto_rocket_jumper_key)
              ->set_render_on(&config->automation.auto_rocket_jumper);
      }

      {
        gcs::toggle(misc_automation_sniper_group, WXOR(L"Rezoom"), &config->automation.auto_rezoom);
        gcs::toggle(misc_automation_sniper_group, WXOR(L"Don't rezoom when moving"), &config->automation.auto_rezoom_when_moving);
        gcs::toggle(misc_automation_sniper_group, WXOR(L"On aimkey"), &config->automation.auto_rezoom_on_aimbot);
      }
    } 

    return group_host;
  }

  gcs_component_canvas* create_player_list(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    auto objects = gcs::fetch_playerlist_settings_comp(comp, WXOR(L"Player list"), WXOR(L"Player list settings"), pos, size);
    gcs_component_canvas*     main_canvas = objects.second;
    gcs_component_group_host* group_host  = objects.first;
    // All of these positions are set and done. Don't mess with them, unless some magical UI code is changed and somehow breaks them.
    {
      if(!global->playerlist_data.valid)
        global->playerlist_data.init();

      gcs_component_group*      group      = group_host->new_group(L"Settings");
      gcs_component_group*      settings   = group->new_category(WXOR(L"Settings"));
      {
        gcs::toggle(settings, WXOR(L"Friend"), &global->playerlist_data.mark_as_friend);
        gcs::toggle(settings, WXOR(L"Ignore on aimbot"), &global->playerlist_data.ignore_player);
        gcs::toggle(settings, WXOR(L"Ignore on auto-heal"), &global->playerlist_data.ignore_auto_heal);
        gcs::dropdown(settings, WXOR(L"Flags"), false)
          ->add_option(WXOR(L"Auto"), &global->playerlist_data.mark_as_auto)
          ->add_option(WXOR(L"Legit"), &global->playerlist_data.mark_as_legit)
          ->add_option(WXOR(L"Suspicious"), &global->playerlist_data.mark_as_suspicious)
          ->add_option(WXOR(L"Cheater"), &global->playerlist_data.mark_as_cheater)
          ->add_option(WXOR(L"Bot"), &global->playerlist_data.mark_as_bot);

        gcs::toggle(settings, WXOR(L"Priority"), &global->playerlist_data.priority_mode);
          gcs::slider(settings, WXOR(L"Value"), L"", 0, 15, GCS_SLIDER_TYPE_INT, &global->playerlist_data.priority)
            ->set_render_on(&global->playerlist_data.priority_mode);
      }
      gcs_component_group* resolver_settings = group->new_category(WXOR(L"Resolver"));
      {
        gcs::dropdown(resolver_settings, WXOR(L"Resolver state"), false)
          ->add_option(WXOR(L"Auto"), &global->playerlist_data.resolver_auto)
          ->add_option(WXOR(L"Disallow"), &global->playerlist_data.resolver_disallow)
          ->add_option(WXOR(L"Force enabled"), &global->playerlist_data.resolver_force);

        gcs::dropdown(resolver_settings, WXOR(L"Pitch"), false)
          ->add_option(WXOR(L"Auto"), &global->playerlist_data.resolver_pitch_auto)
          ->add_option(WXOR(L"Up"), &global->playerlist_data.resolver_pitch_up)
          ->add_option(WXOR(L"Down"), &global->playerlist_data.resolver_pitch_down)
          ->add_option(WXOR(L"Zero"), &global->playerlist_data.resolver_pitch_zero);

        gcs::dropdown(resolver_settings, WXOR(L"Yaw"), false)
          ->add_option(WXOR(L"Auto"), &global->playerlist_data.resolver_yaw_auto)
          ->add_option(WXOR(L"← (-90)"), &global->playerlist_data.resolver_yaw_neg90)
          ->add_option(WXOR(L"(90)  →"), &global->playerlist_data.resolver_yaw_pos90)
          ->add_option(WXOR(L"↖ (-45)"), &global->playerlist_data.resolver_yaw_neg45)
          ->add_option(WXOR(L"(45) ↗"), &global->playerlist_data.resolver_yaw_pos45)
          ->add_option(WXOR(L"↓ (towards) ↓"), &global->playerlist_data.resolver_yaw_lookat)
          ->add_option(WXOR(L"↑ (away) ↑"), &global->playerlist_data.resolver_yaw_180away);
      }
      gcs_component_group* helpers = group->new_category(WXOR(L"Helpers"));
      {
        gcs::button(helpers, WXOR(L"Call votekick"))
          ->add_hook(GCS_HOOK_CLICK, [](gcs_component_base* c, void* p){
            if(!global->playerlist_has_valid_player)
              return true;

            s_player_info info;
            if(!global->engine->get_player_info(global->playerlist_current_player_index, &info))
              return true;

            i8 buf[IDEAL_MIN_BUF_SIZE];
            formatA(buf, XOR("callvote kick \"%i cheating\""), info.user_id);
            global->engine->client_cmd(buf);

            return true;
          });
      }
      gcs_component_group* colour_settings = group->new_category(WXOR(L"Colours"));
      {
        gcs::toggle(colour_settings, WXOR(L"ESP"), &global->playerlist_data.custom_esp_colour)
          ->add_module<gcs_component_colour_picker>()
          ->set_output_colour_ptr(global->playerlist_data.esp_colour)
            ->set_render_on(&global->playerlist_data.custom_esp_colour);

        gcs::toggle(colour_settings, WXOR(L"Base Chams"), &global->playerlist_data.custom_chams_base_colour)
          ->add_module<gcs_component_colour_picker>()
          ->enable_alpha_bar()
          ->set_output_colour_ptr(global->playerlist_data.chams_colour)
            ->set_render_on(&global->playerlist_data.custom_chams_base_colour);

        gcs::toggle(colour_settings, WXOR(L"Overlay Chams"), &global->playerlist_data.custom_chams_overlay_colour)
          ->add_module<gcs_component_colour_picker>()
          ->enable_alpha_bar()
          ->set_output_colour_ptr(global->playerlist_data.chams_overlay_colour)
            ->set_render_on(&global->playerlist_data.custom_chams_overlay_colour);

        gcs::toggle(colour_settings, WXOR(L"Glow"), &global->playerlist_data.custom_glow_colour)
          ->add_module<gcs_component_colour_picker>()
          ->set_output_colour_ptr(global->playerlist_data.glow_colour)
            ->set_render_on(&global->playerlist_data.custom_glow_colour);   
      } 
  
      group_host->update();
    }
    
    gcs::fetch_playerlist_window_comp<gcs_component_player_list>(main_canvas, WXOR(L"Player List Window"), size);

    return main_canvas;
  }

  gcs_component_canvas* create_notifications_list(gcs_component_base* comp, vec3i pos, vec3i size){
    return gcs::create_notify_history<gcs_notify_component_history>(comp, WXOR(L"Notifications"), pos, size);
  }

  gcs_component_canvas* create_cheater_detection_list(gcs_component_base* comp, vec3i pos, vec3i size){
    return gcs::create_log_window(comp, logs->infractions, WXOR(L"Infractions"), pos, size);
  }

  gcs_component_canvas* create_connections_list(gcs_component_base* comp, vec3i pos, vec3i size){
    return gcs::create_log_window(comp, logs->connections, WXOR(L"Connections"), pos, size);
  }

  gcs_component_canvas* create_votes_list(gcs_component_base* comp, vec3i pos, vec3i size){
    return gcs::create_log_window(comp, logs->votes, WXOR(L"Votes"), pos, size);
  }

  void create_player_esp_layer(gcs_component_group* group, i32 i){
    c_player_esp_settings* esp_settings = utils::get_player_esp_settings(i);
    assert(esp_settings != nullptr); // If this is ever null then there was a mistake!

    gcs::toggle(group, WXOR(L"Enabled"), &esp_settings->enabled);
      gcs::toggle(group, WXOR(L"Show when cloaked"), &esp_settings->render_cloaked_spies)
        ->set_render_on(&esp_settings->enabled);

    if(i == PLAYER_ESP_SETTING_TYPE_FRIEND){
      gcs::toggle(group, WXOR(L"Show localplayer"), &config->visual.player_esp_include_localplayer)
        ->set_render_on(&esp_settings->enabled);

      gcs::toggle(group, WXOR(L"Render friend text"), &esp_settings->friend_tag)
        ->set_render_on(&esp_settings->enabled);

      gcs::toggle(group, WXOR(L"Use steam nickname"), &config->visual.use_steam_nicknames)
        ->set_render_on(&esp_settings->enabled);
    }

    gcs::dropdown(group, WXOR(L"Components"), true)
      ->add_option(WXOR(L"Player name"), &esp_settings->name)
      ->add_option(WXOR(L"Weapon name"), &esp_settings->weapon_name)
      ->add_option(WXOR(L"Distance"), &esp_settings->distance)
          ->set_render_on(&esp_settings->enabled);

    gcs::dropdown(group, WXOR(L"Box"), false)
      ->add_option(WXOR(L"Disabled"), &esp_settings->no_box)
      ->add_option(WXOR(L"Box"), &esp_settings->box)
      ->add_option(WXOR(L"Box + Outlines"), &esp_settings->box_outlines)
         ->set_render_on(&esp_settings->enabled);

    gcs::toggle(group, WXOR(L"Flags"), &esp_settings->flags)
      ->set_render_on(&esp_settings->enabled);
      {
        gcs::dropdown(group, WXOR(L"Flags list"), true)
          ->add_option(WXOR(L"Aimbot flags"), &esp_settings->flag_aimbot_target)
          ->add_option(WXOR(L"Slow down"), &esp_settings->flag_slow_down)
          ->add_option(WXOR(L"Cloaked"), &esp_settings->flag_cloaked)
          ->add_option(WXOR(L"Bonked"), &esp_settings->flag_bonked)
          ->add_option(WXOR(L"Disguised"), &esp_settings->flag_disguised)
          ->add_option(WXOR(L"Zoomed"), &esp_settings->flag_zoomed)
          ->add_option(WXOR(L"Ubercharged"), &esp_settings->flag_ubercharged)
          ->add_option(WXOR(L"Ubercharged hidden"), &esp_settings->flag_ubercharged_hidden)
          ->add_option(WXOR(L"Taunting"), &esp_settings->flag_taunting)
          ->add_option(WXOR(L"Crit boosted"), &esp_settings->flag_crit_boosted)
          ->add_option(WXOR(L"Damage buff"), &esp_settings->flag_damage_buff)
          ->add_option(WXOR(L"Demo charge"), &esp_settings->flag_demo_charge)
          ->add_option(WXOR(L"Crit cola"), &esp_settings->flag_crit_cola)
          ->add_option(WXOR(L"On fire"), &esp_settings->flag_on_fire)
          ->add_option(WXOR(L"Jarated"), &esp_settings->flag_jarated)
          ->add_option(WXOR(L"Bleeding"), &esp_settings->flag_bleeding)
          ->add_option(WXOR(L"Defense buff"), &esp_settings->flag_defense_buff)
          ->add_option(WXOR(L"Speed/heal buff"), &esp_settings->flag_speed_heal_buff)
          ->add_option(WXOR(L"Mad milked"), &esp_settings->flag_mad_milked)
          ->add_option(WXOR(L"Quick fix uber"), &esp_settings->flag_quick_fix_uber)
          ->add_option(WXOR(L"Hitmans buff"), &esp_settings->flag_hitmans_buff)
          ->add_option(WXOR(L"Bullet resist charge"), &esp_settings->flag_bullet_resist_charge)
          ->add_option(WXOR(L"Bullet resist"), &esp_settings->flag_bullet_resist)
          ->add_option(WXOR(L"Blast resist charge"), &esp_settings->flag_blast_resist_charge)
          ->add_option(WXOR(L"Blast resist"), &esp_settings->flag_blast_resist)
          ->add_option(WXOR(L"Fire resist charge"), &esp_settings->flag_fire_resist_charge)
          ->add_option(WXOR(L"Fire resist"), &esp_settings->flag_fire_resist)
          ->add_option(WXOR(L"Medigun debuff"), &esp_settings->flag_medigun_debuff)
          ->add_option(WXOR(L"Healing debuff"), &esp_settings->flag_healing_debuff)
          ->add_option(WXOR(L"Healing hp"), &esp_settings->flag_show_healing)
          ->add_option(WXOR(L"Has objective"), &esp_settings->flag_has_objective)
          ->add_option(WXOR(L"Sniper charge"), &esp_settings->flag_sniper_charge)
            ->set_render_on(&esp_settings->flags)
            ->set_render_on(&esp_settings->enabled);
      }

    gcs::toggle(group, WXOR(L"Health text"), &esp_settings->health_text)
      ->set_render_on(&esp_settings->enabled);

    gcs::toggle(group, WXOR(L"Health bar"), &esp_settings->healthbar)
      ->set_render_on(&esp_settings->enabled);

    gcs::slider(group, WXOR(L"Health fractions"), 0, 10, GCS_SLIDER_TYPE_INT, &esp_settings->health_bar_fractions)
      ->set_render_on(&esp_settings->healthbar)
      ->set_render_on(&esp_settings->enabled);

    gcs::toggle(group, WXOR(L"Ubercharge bar"), &esp_settings->ubercharge_bar)
      ->set_render_on(&esp_settings->enabled);

    gcs::toggle(group, WXOR(L"Cheater/bot alerts"), &esp_settings->cheater_bot_alert)
      ->set_render_on(&esp_settings->enabled);

    if(i != PLAYER_ESP_SETTING_TYPE_BLU_TEAM){
      gcs::toggle(group, WXOR(L"Show duels"), &esp_settings->flag_show_duels)
        ->set_render_on(&esp_settings->enabled);
    }

    gcs::dropdown(group, WXOR(L"Dormancy"), true)
      ->add_option(WXOR(L"Show"), &esp_settings->dormant_enabled)
      ->add_option(WXOR(L"Render dormant text"), &esp_settings->dormant_render_text)
      ->set_render_on(&esp_settings->enabled);

    gcs::dropdown(group, WXOR(L"Dormancy top/bottom colour"), false)
      ->add_option(WXOR(L"None"), &esp_settings->dormant_color_none)
      ->add_option(WXOR(L"Gray out"), &esp_settings->dormant_color_gray_out)
      ->add_option(WXOR(L"Team"), &esp_settings->dormant_color_team)
      ->set_render_on(&esp_settings->enabled);
  }

  void create_world_component_layer(gcs_component_group* group, c_world_esp_settings* esp_settings, i32 type, bool healthbar = false, bool flags_only = false){

    if(!healthbar){
      gcs::dropdown(group, WXOR(L"Components"), true)
        ->add_option(WXOR(L"Box"), &esp_settings->box[type])
        ->add_option(WXOR(L"Box outline"), &esp_settings->box_outlines[type])
        ->add_option(WXOR(L"Name"), &esp_settings->name[type])
        ->add_option(WXOR(L"Distance"), &esp_settings->distance[type])
        ->set_render_on(&esp_settings->object_enabled[type])
        ->set_render_on(&esp_settings->team_enabled);
    }
    else{
      if(!flags_only){
        gcs::dropdown(group, WXOR(L"Components"), true)
          ->add_option(WXOR(L"Box"), &esp_settings->box[type])
          ->add_option(WXOR(L"Box outline"), &esp_settings->box_outlines[type])
          ->add_option(WXOR(L"Name"), &esp_settings->name[type])
          ->add_option(WXOR(L"Health bar"), &esp_settings->healthbar[type])
          ->add_option(WXOR(L"Flags"), &esp_settings->flags[type])
          ->add_option(WXOR(L"Distance"), &esp_settings->distance[type])
          ->set_render_on(&esp_settings->object_enabled[type])
          ->set_render_on(&esp_settings->team_enabled);
      }
      else{
        gcs::dropdown(group, WXOR(L"Components"), true)
          ->add_option(WXOR(L"Box"), &esp_settings->box[type])
          ->add_option(WXOR(L"Box outline"), &esp_settings->box_outlines[type])
          ->add_option(WXOR(L"Name"), &esp_settings->name[type])
          ->add_option(WXOR(L"Flags"), &esp_settings->flags[type])
          ->add_option(WXOR(L"Distance"), &esp_settings->distance[type])
          ->set_render_on(&esp_settings->object_enabled[type])
          ->set_render_on(&esp_settings->team_enabled);
      }
    }


    gcs::dropdown(group, WXOR(L"Dormancy"), true)
      ->add_option(WXOR(L"Show"), &esp_settings->dormant_enabled[type])
      ->add_option(WXOR(L"Render dormant text"), &esp_settings->dormant_render_text[type])
      ->set_render_on(&esp_settings->object_enabled[type])
      ->set_render_on(&esp_settings->team_enabled);
  }

  void create_world_esp_layer(gcs_component_group* group, i32 i){
    c_world_esp_settings* esp_settings = utils::get_world_esp_settings(i);
    assert(esp_settings != nullptr); // If this is ever null then there was a mistake!

    gcs::toggle(group, WXOR(L"Enabled"), &esp_settings->team_enabled);

     if(i == WORLD_ESP_SETTING_TYPE_UNASSIGNED_TEAM){
       gcs::toggle(group, WXOR(L"Ammopack"), &esp_settings->object_enabled[4])
         ->set_render_on(&esp_settings->team_enabled);
       {
         create_world_component_layer(group, esp_settings, 4);
       }

       // Healthpack
       gcs::toggle(group, WXOR(L"Healthpack"), &esp_settings->object_enabled[5])
         ->set_render_on(&esp_settings->team_enabled);
       {
         create_world_component_layer(group, esp_settings, 5);
       }

       // Other entities
       gcs::toggle(group, WXOR(L"Other entities"), &esp_settings->object_enabled[7])
          ->set_render_on(&esp_settings->team_enabled);
       {
          gcs::dropdown(group, WXOR(L"List"), true)
            ->add_option(WXOR(L"Bombs"), &esp_settings->ent_bombs)
            ->add_option(WXOR(L"Lunchables"), &esp_settings->ent_lunchables)
            ->add_option(WXOR(L"Boss"), &esp_settings->ent_boss)
            ->add_option(WXOR(L"Powerup"), &esp_settings->ent_powerup)
            ->add_option(WXOR(L"Spellbooks"), &esp_settings->ent_spellbooks)
            ->add_option(WXOR(L"Money"), &esp_settings->ent_money)
            ->add_option(WXOR(L"Halloween pickups"), &esp_settings->ent_halloween_pickup)
              ->set_render_on(&esp_settings->object_enabled[7])
              ->set_render_on(&esp_settings->team_enabled);

          create_world_component_layer(group, esp_settings, 7);
      }


      return;
    }

    gcs::slider(group, WXOR(L"Health fractions"), 0, 10, GCS_SLIDER_TYPE_INT, &esp_settings->health_bar_fractions)
      ->set_render_on(&esp_settings->team_enabled);

    // Sentry.
    gcs::toggle(group, WXOR(L"Sentry"), &esp_settings->object_enabled[0])
      ->set_render_on(&esp_settings->team_enabled);
    {
      create_world_component_layer(group, esp_settings, 0, true);

      gcs::dropdown(group, WXOR(L"Flags"), true)
        ->add_option(WXOR(L"Level"), &esp_settings->sentry_level)
        ->add_option(WXOR(L"Active state"), &esp_settings->sentry_activestate)
        ->add_option(WXOR(L"Building state"), &esp_settings->sentry_buildingstate)
        ->add_option(WXOR(L"Controlled"), &esp_settings->sentry_controlled)
        ->add_option(WXOR(L"Ammo status"), &esp_settings->sentry_ammostatus)
        ->set_render_on(&esp_settings->object_enabled[0])
        ->set_render_on(&esp_settings->team_enabled);
    }

    // Dispenser
    gcs::toggle(group, WXOR(L"Dispenser"), &esp_settings->object_enabled[1])
      ->set_render_on(&esp_settings->team_enabled);
    {
      create_world_component_layer(group, esp_settings, 1, true);

      gcs::dropdown(group, WXOR(L"Flags"), true)
        ->add_option(WXOR(L"Level"), &esp_settings->dispenser_level)
        ->add_option(WXOR(L"Active state"), &esp_settings->dispenser_activestate)
        ->add_option(WXOR(L"Building state"), &esp_settings->dispenser_buildingstate)
        ->set_render_on(&esp_settings->object_enabled[1])
        ->set_render_on(&esp_settings->team_enabled);
    }

    // Teleporter
    gcs::toggle(group, WXOR(L"Teleporter"), &esp_settings->object_enabled[2])
      ->set_render_on(&esp_settings->team_enabled);
    {
      create_world_component_layer(group, esp_settings, 2, true);
      gcs::dropdown(group, WXOR(L"Flags"), true)
        ->add_option(WXOR(L"Level"), &esp_settings->teleporter_level)
        ->add_option(WXOR(L"Active state"), &esp_settings->teleporter_activestate)
        ->add_option(WXOR(L"Building state"), &esp_settings->teleporter_buildingstate)
        ->set_render_on(&esp_settings->object_enabled[2])
        ->set_render_on(&esp_settings->team_enabled);
    }

    // Objective
    gcs::toggle(group, WXOR(L"Objective"), &esp_settings->object_enabled[3])
      ->set_render_on(&esp_settings->team_enabled);
    {
      create_world_component_layer(group, esp_settings, 3, true, true);
      gcs::dropdown(group, WXOR(L"Flags"), true)
        ->add_option(WXOR(L"Return Time"), &esp_settings->objective_returntime)
        ->add_option(WXOR(L"Active state"), &esp_settings->objective_status)
        ->set_render_on(&esp_settings->object_enabled[3])
        ->set_render_on(&esp_settings->team_enabled);
    }

    // Projectiles
    gcs::toggle(group, WXOR(L"Projectiles"), &esp_settings->object_enabled[6])
      ->set_render_on(&esp_settings->team_enabled);
    {
      create_world_component_layer(group, esp_settings, 6, true, true);
      gcs::dropdown(group, WXOR(L"List"), true)
        ->add_option(WXOR(L"Rocket"), &esp_settings->projectile_rocket)
        ->add_option(WXOR(L"Sticky"), &esp_settings->projectile_sticky)
        ->add_option(WXOR(L"Pipebomb"), &esp_settings->projectile_pipebomb)
        ->add_option(WXOR(L"Arrow"), &esp_settings->projectile_arrow)
        ->add_option(WXOR(L"Flare"), &esp_settings->projectile_flare)
        ->add_option(WXOR(L"Throwables"), &esp_settings->projectile_throwables)
        ->add_option(WXOR(L"Spells"), &esp_settings->projectile_spells)
          ->set_render_on(&esp_settings->object_enabled[6])
          ->set_render_on(&esp_settings->team_enabled);

      gcs::dropdown(group, WXOR(L"Flags"), true)
        ->add_option(WXOR(L"Critical"), &esp_settings->projectile_flag_criticals)
        ->set_render_on(&esp_settings->object_enabled[6])
        ->set_render_on(&esp_settings->team_enabled);
    }

    gcs::toggle(group, WXOR(L"Other entities"), &esp_settings->object_enabled[7])
      ->set_render_on(&esp_settings->team_enabled);
    {
      create_world_component_layer(group, esp_settings, 7);
      gcs::dropdown(group, WXOR(L"List"), true)
        ->add_option(WXOR(L"Bombs"), &esp_settings->ent_bombs)
        ->add_option(WXOR(L"Boss"), &esp_settings->ent_boss)
        ->add_option(WXOR(L"Revive marker"), &esp_settings->ent_revive_marker)
        ->set_render_on(&esp_settings->object_enabled[7])
        ->set_render_on(&esp_settings->team_enabled);
    }
  }

  gcs_component_group_host* create_visuals_esp(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"ESP"), pos, size);
    {
      gcs_component_group* player_esp_group     = group_host->new_group(WXOR(L"Player esp"));
      gcs_component_group* player_enemy_group   = player_esp_group->new_category(WXOR(L"Enemy"));
      gcs_component_group* player_team_group    = player_esp_group->new_category(WXOR(L"Team"));
      gcs_component_group* player_friend_group  = player_esp_group->new_category(WXOR(L"Friends"));
      gcs_component_group* player_colours_group = player_esp_group->new_category(WXOR(L"Colours"));

      gcs_component_group* world_esp_group      = group_host->new_group(WXOR(L"World esp"));
      gcs_component_group* world_enemy_group    = world_esp_group->new_category(WXOR(L"Enemy"));
      gcs_component_group* world_team_group     = world_esp_group->new_category(WXOR(L"Team"));
      gcs_component_group* world_unassign_group = world_esp_group->new_category(WXOR(L"Other"));
      gcs_component_group* world_colours_group  = world_esp_group->new_category(WXOR(L"Colours"));

      gcs_component_group* esp_main_group       = group_host->new_group(WXOR(L"Global filters"));
      gcs_component_group* esp_filter_group     = esp_main_group->new_category(WXOR(L"Global filters"));

      group_host->update();

      // Player esp
      {
        create_player_esp_layer(player_enemy_group, PLAYER_ESP_SETTING_TYPE_RED_TEAM);
        create_player_esp_layer(player_team_group, PLAYER_ESP_SETTING_TYPE_BLU_TEAM);
        create_player_esp_layer(player_friend_group, PLAYER_ESP_SETTING_TYPE_FRIEND);

        // HINT-HEADER: Player esp colours
        {
           gcs::dropdown(player_colours_group, WXOR(L"Player scheme"), false)
             ->add_option(WXOR(L"Team based"), &config->visual.player_esp_use_team_colours)
             ->add_option(WXOR(L"Enemy / friendly"), &config->visual.player_esp_use_enemyteam_colours);

           // Team based.
           {
              gcs::label(player_colours_group, WXOR(L"RED team"))
                ->set_render_on(&config->visual.player_esp_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_esp_red_team_colour);

              gcs::label(player_colours_group, WXOR(L"BLU team"))
                ->set_render_on(&config->visual.player_esp_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_esp_blu_team_colour);
           }

           // Enemy / friendly based.
           {
              gcs::label(player_colours_group, WXOR(L"Enemy"))
                ->set_render_on(&config->visual.player_esp_use_enemyteam_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_esp_enemy_colour);

              gcs::label(player_colours_group, WXOR(L"Team"))
                ->set_render_on(&config->visual.player_esp_use_enemyteam_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_esp_team_colour);
           }

           gcs::add_space(player_colours_group);
           gcs::toggle(player_colours_group, WXOR(L"Aimbot target"), &config->visual.player_esp_aimbot_target)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.player_esp_aimbot_target_colour);

           gcs::add_space(player_colours_group);
           gcs::dropdown(player_colours_group, WXOR(L"Friend scheme"), false)
             ->add_option(WXOR(L"Single colour"), &config->visual.player_esp_friends_use_single_colour)
             ->add_option(WXOR(L"Use player scheme"), &config->visual.player_esp_friends_use_mult_colour);

           gcs::label(player_colours_group, WXOR(L"Friends"))
             ->set_render_on(&config->visual.player_esp_friends_use_single_colour)
             ->add_module<gcs_component_colour_picker>()
             ->set_output_colour_ptr(config->visual.player_esp_friend_colour);

            // Team based.
           {
              gcs::label(player_colours_group, WXOR(L"RED team"))
                ->set_render_on(&config->visual.player_esp_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_esp_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_esp_friend_red_team_colour);

              gcs::label(player_colours_group, WXOR(L"BLU team"))
                ->set_render_on(&config->visual.player_esp_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_esp_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_esp_friend_blu_team_colour);
           }

           // Enemy / friendly based.
           {
              gcs::label(player_colours_group, WXOR(L"Enemy"))
                ->set_render_on(&config->visual.player_esp_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_esp_use_enemyteam_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_esp_friend_enemy_colour);

              gcs::label(player_colours_group, WXOR(L"Team"))
                ->set_render_on(&config->visual.player_esp_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_esp_use_enemyteam_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_esp_friend_team_colour);
           }
        }
      }

      // World esp
      {
        create_world_esp_layer(world_enemy_group, WORLD_ESP_SETTING_TYPE_RED_TEAM);
        create_world_esp_layer(world_team_group, WORLD_ESP_SETTING_TYPE_BLU_TEAM);
        create_world_esp_layer(world_unassign_group, WORLD_ESP_SETTING_TYPE_UNASSIGNED_TEAM);

        // HINT-HEADER: World esp colours
        {
          gcs::dropdown(world_colours_group, WXOR(L"Scheme"), false)
            ->add_option(WXOR(L"Team based"), &config->visual.world_esp_use_team_colours)
            ->add_option(WXOR(L"Enemy / friendly"), &config->visual.world_esp_use_enemyteam_colours);

          // Team based.
          {
             gcs::label(world_colours_group, WXOR(L"RED team"))
               ->set_render_on(&config->visual.world_esp_use_team_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.world_esp_red_team_colour);

             gcs::label(world_colours_group, WXOR(L"BLU team"))
               ->set_render_on(&config->visual.world_esp_use_team_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.world_esp_blu_team_colour);
          }

          // Enemy / friendly based.
          {
             gcs::label(world_colours_group, WXOR(L"Enemy"))
               ->set_render_on(&config->visual.world_esp_use_enemyteam_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.world_esp_enemy_colour);

             gcs::label(world_colours_group, WXOR(L"Team"))
               ->set_render_on(&config->visual.world_esp_use_enemyteam_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.world_esp_team_colour);
          }

          gcs::add_space(world_colours_group);
          gcs::toggle(world_colours_group, WXOR(L"Aimbot target"), &config->visual.world_esp_aimbot_target)
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.world_esp_aimbot_target_colour);

          gcs::add_space(world_colours_group);
          gcs::label(world_colours_group, WXOR(L"Unassigned team"))
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.world_esp_unassigned_team_colour);

          gcs::label(world_colours_group, WXOR(L"Healthpack"))
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.world_esp_health_pack_colour);

          gcs::label(world_colours_group, WXOR(L"Ammopack"))
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.world_esp_ammo_pack_colour);

          gcs::label(world_colours_group, WXOR(L"Boss"))
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.world_esp_boss_colour);

          gcs::label(world_colours_group, WXOR(L"Bombs"))
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.world_esp_bomb_colour);

          gcs::label(world_colours_group, WXOR(L"Lunchables"))
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.world_esp_lunchables_colour);

          gcs::label(world_colours_group, WXOR(L"Money"))
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.world_esp_money_colour);

          gcs::label(world_colours_group, WXOR(L"Powerup"))
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.world_esp_powerup_colour);

          gcs::label(world_colours_group, WXOR(L"Normal spellbooks"))
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.world_esp_spellbook_normal_colour);

          gcs::label(world_colours_group, WXOR(L"Rare spellbooks"))
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.world_esp_spellbook_rare_colour);
        }
      }

      // Global filters
      {
        gcs::toggle(esp_filter_group, WXOR(L"Medic: Show injured team-mates"), &config->visual.render_medic_mode)
          ->set_help_text(WXOR(L"If holding a healing weapon.\nThis will only show injured team-mates."));

        gcs::toggle(esp_filter_group, WXOR(L"Engineer: Show damaged objects"), &config->visual.render_engineer_mode)
          ->set_help_text(WXOR(L"If holding the wrench or rescue ranger.\nThis will only show injured buildings."));
      }
    }

    return group_host;
  }

  gcs_component_group_host* create_visuals_main(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);
    gcs_component_group_host* group_host   = gcs::group_host(comp, WXOR(L"Main"), pos, size);

    gcs_component_group* ms_group      = group_host->new_group(L"");

    gcs_component_group* menu_settings    = ms_group->new_category(WXOR(L"Menu"));
    gcs_component_group* effects_group    = ms_group->new_category(WXOR(L"Effects"));
    gcs_component_group* aimbot_group     = ms_group->new_category(WXOR(L"Aimbot"));
    gcs_component_group* projectile_group = ms_group->new_category(WXOR(L"Projectile"));

    gcs_component_group* vm_group = group_host->new_group(WXOR(L"Viewmodel"));
    gcs_component_group* vm_main_group = vm_group->new_category(WXOR(L"Viewmodel"));

    gcs_component_group* misc_main_group  = group_host->new_group(L"");
    gcs_component_group* other_group      = misc_main_group->new_category(WXOR(L"Text/Particle"));
    gcs_component_group* view_hud_group   = misc_main_group->new_category(WXOR(L"View/HUD"));
    gcs_component_group* misc_group       = misc_main_group->new_category(WXOR(L"Other"));

    group_host->update();

    // Menu settings.
    {
      gcs::label(menu_settings, WXOR(L"Menu Foreground"))
        ->add_module<gcs_component_colour_picker>()
          ->set_output_colour_ptr(config->visual.menu_foreground_colour);

      gcs::label(menu_settings, WXOR(L"Menu Background"))
        ->add_module<gcs_component_colour_picker>()
          ->set_output_colour_ptr(config->visual.menu_background_colour);

      gcs::toggle(menu_settings, WXOR(L"Cheap text rendering"), &config->visual.cheap_text_rendering);
      gcs::add_space(menu_settings);
      gcs::toggle(menu_settings, WXOR(L"FOV circle"), &config->visual.aimbot_fov_circle_enabled);
      gcs::toggle(menu_settings, WXOR(L"FOV circle outlines"), &config->visual.aimbot_fov_circle_outlines)
        ->set_render_on(&config->visual.aimbot_fov_circle_enabled);

      gcs::label(menu_settings, WXOR(L"FOV circle colour"))
        ->set_render_on(&config->visual.aimbot_fov_circle_enabled)
          ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
            ->set_output_colour_ptr(config->visual.aimbot_fov_circle_colour);

      gcs::add_space(menu_settings);
      gcs::toggle(menu_settings, WXOR(L"Show spectators panel"), &config->visual.show_spectators);
      gcs::toggle(menu_settings, WXOR(L"Localplayer only"), &config->visual.show_spectators_localplayer)
        ->set_render_on(&config->visual.show_spectators);

      gcs::add_space(menu_settings);
      gcs::toggle(menu_settings, WXOR(L"Show information panel"), &config->visual.show_information);
    }

    // Effects
    {
      gcs::toggle(effects_group, WXOR(L"Bullet tracers"), &config->visual.bullet_tracers_enabled);
      {
        gcs::toggle(effects_group, WXOR(L"Include sentry"), &config->visual.bullet_tracers_include_sentry)
          ->set_render_on(&config->visual.bullet_tracers_enabled);

        gcs::dropdown(effects_group, WXOR(L"Effect"), false)
          ->add_option(WXOR(L"Beam"), &config->visual.tracer_beam_effect)
          ->add_option(WXOR(L"Lines"), &config->visual.tracer_lines)
          ->add_option(WXOR(L"Raygun"), &config->visual.tracer_raygun_effect)
          ->add_option(WXOR(L"Machina"), &config->visual.tracer_machina_effect)
          ->add_option(WXOR(L"Lightning ball"), &config->visual.tracer_lightning_effect)
          ->add_option(WXOR(L"Merasmus"), &config->visual.tracer_meramus_effect)
          ->add_option(WXOR(L"Distortion trail"), &config->visual.tracer_distortion_effect)
          ->add_option(WXOR(L"Big nasty"), &config->visual.tracer_bignasty_effect)
          ->set_render_on(&config->visual.bullet_tracers_enabled);

          gcs::label(effects_group, WXOR(L"Beam colour"))
            ->set_render_on(&config->visual.bullet_tracers_enabled)
            ->set_render_on(&config->visual.tracer_beam_effect)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.bullet_beam_tracer_colour)
              ->set_render_on(&config->visual.tracer_beam_effect);

          gcs::label(effects_group, WXOR(L"Line colour"))
            ->set_render_on(&config->visual.bullet_tracers_enabled)
            ->set_render_on(&config->visual.tracer_lines)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.tracer_line_colour)
              ->enable_alpha_bar()
              ->set_render_on(&config->visual.tracer_lines);

          gcs::toggle(effects_group, WXOR(L"Outline"), &config->visual.tracer_line_outline)
            ->set_render_on(&config->visual.tracer_lines)
            ->set_render_on(&config->visual.bullet_tracers_enabled);

          gcs::slider(effects_group, WXOR(L"Beam lifetime"), WXOR(L"ms"), 15, 2500, GCS_SLIDER_TYPE_INT, &config->visual.tracer_beam_effect_life_time)
            ->set_render_on(&config->visual.bullet_tracers_enabled)
            ->set_render_on(&config->visual.tracer_beam_effect);

          gcs::slider(effects_group, WXOR(L"Line lifetime"), WXOR(L"ms"), 15, 2500, GCS_SLIDER_TYPE_INT, &config->visual.tracer_line_life_time)
            ->set_render_on(&config->visual.bullet_tracers_enabled)
            ->set_render_on(&config->visual.tracer_lines);
      }

      gcs::add_space(effects_group);
      gcs::toggle(effects_group, WXOR(L"Disable overlay effects"), &config->visual.disable_overlay_effects_enabled);
      {
        gcs::dropdown(effects_group, WXOR(L"Effects"), true)
         ->add_option(WXOR(L"Invuln"), &config->visual.disable_overlay_invuln)
         ->add_option(WXOR(L"Milk"), &config->visual.disable_overlay_milk)
         ->add_option(WXOR(L"Jarate"), &config->visual.disable_overlay_jarate)
         ->add_option(WXOR(L"Bleed"), &config->visual.disable_overlay_bleed)
         ->add_option(WXOR(L"Stealth"), &config->visual.disable_overlay_stealth)
         ->add_option(WXOR(L"Bonked"), &config->visual.disable_overlay_bonked)
         ->add_option(WXOR(L"Gas"), &config->visual.disable_overlay_gas)
         ->add_option(WXOR(L"Fire"), &config->visual.disable_overlay_onfire)
         ->add_option(WXOR(L"Water"), &config->visual.disable_overlay_water)
         ->set_render_on(&config->visual.disable_overlay_effects_enabled);
      }

      gcs::add_space(effects_group);
      gcs::toggle(effects_group, WXOR(L"Ragdoll effects"), &config->visual.ragdoll_effects);
      {
          gcs::toggle(effects_group, WXOR(L"Disable death animations"), &config->visual.ragdoll_effects_disable_death_animations)
            ->set_render_on(&config->visual.ragdoll_effects);
  
          gcs::toggle(effects_group, WXOR(L"Disable game created effects"), &config->visual.ragdoll_effects_disable_game_spawned_effects)
            ->set_render_on(&config->visual.ragdoll_effects);
  
          gcs::dropdown(effects_group, WXOR(L"Model effects"), false)
            ->add_option(WXOR(L"None"), &config->visual.ragdoll_effect_model_none)
            ->add_option(WXOR(L"Golden"), &config->visual.ragdoll_effect_weak_golden)
            ->add_option(WXOR(L"Frozen"), &config->visual.ragdoll_effect_weak_frozen)
            ->set_render_on(&config->visual.ragdoll_effects);
  
          gcs::dropdown(effects_group, WXOR(L"Main particle effect"), false)
            ->add_option(WXOR(L"None"), &config->visual.ragdoll_effect_particle_none)
            ->add_option(WXOR(L"Burning"), &config->visual.ragdoll_effect_strong_burning)
            ->add_option(WXOR(L"Electrocuted"), &config->visual.ragdoll_effect_strong_electrocuted)
            ->set_render_on(&config->visual.ragdoll_effects);
  
          gcs::dropdown(effects_group, WXOR(L"Other particle effects"), true)
            ->add_option(WXOR(L"Ash"), &config->visual.ragdoll_effect_weak_ash)
            ->add_option(WXOR(L"Dissolve"), &config->visual.ragdoll_effect_weak_dissolve)
            ->set_render_on(&config->visual.ragdoll_effects);
  
          gcs::dropdown(effects_group, WXOR(L"Team"), false)
              ->add_option(WXOR(L"Friendly only"), &config->visual.ragdoll_effects_show_friendly)
              ->add_option(WXOR(L"Enemy only"), &config->visual.ragdoll_effects_show_enemy)
              ->add_option(WXOR(L"All"), &config->visual.ragdoll_effects_show_all)
              ->set_render_on(&config->visual.ragdoll_effects);
      }
    }

    // Aimbot
    {
      gcs::toggle(aimbot_group, WXOR(L"Estimated aimbot target"), &config->visual.show_estimated_aim_target)
       ->add_module<gcs_component_colour_picker>()
          ->set_output_colour_ptr(config->visual.estimated_aim_target_colour)
          ->set_render_on(&config->visual.show_estimated_aim_target);
      {
        gcs::dropdown(aimbot_group, WXOR(L"Shape"), false)
          ->add_option(WXOR(L"Circle"), &config->visual.estimated_aim_target_circle)
          ->add_option(WXOR(L"Triangle"), &config->visual.estimated_aim_target_triangle)
          ->add_option(WXOR(L"Arrow"), &config->visual.estimated_aim_target_arrow)
          ->set_render_on(&config->visual.show_estimated_aim_target);

        gcs::slider(aimbot_group, WXOR(L"Size"), L"", 10, 14, GCS_SLIDER_TYPE_INT, &config->visual.estimated_aim_target_size)
         ->set_render_on(&config->visual.show_estimated_aim_target);
      }
    }

    // Projectile.
    {
      gcs::toggle(projectile_group, WXOR(L"Trajectory visualizer"), &config->visual.projectile_trajectory_visualizer);
      {
        gcs::toggle(projectile_group, WXOR(L"Enable impact camera"), &config->visual.projectile_trajectory_impact_camera)
          ->set_render_on(&config->visual.projectile_trajectory_visualizer);

        gcs::dropdown(projectile_group, WXOR(L"VPhysics Flags"), true)
          ->add_option(WXOR(L"Show random impulse"), &config->visual.projectile_trajectory_show_random_impulse)
          ->add_option(WXOR(L"Show random velocity"), &config->visual.projectile_trajectory_show_random_velocity)
          ->set_render_on(&config->visual.projectile_trajectory_visualizer);

        gcs::add_space(projectile_group);
        gcs::dropdown(projectile_group, WXOR(L"Show trajectory path"), true)
          ->add_option(WXOR(L"Enabled"), &config->visual.projectile_trajectory_constant_line)
          ->add_option(WXOR(L"Outlined"), &config->visual.projectile_trajectory_constant_outline)
          ->add_option(WXOR(L"Splash radius circle"), &config->visual.projectile_trajectory_constant_splash_circle)
          ->add_option(WXOR(L"Impact box"), &config->visual.projectile_trajectory_constant_box)
            ->set_render_on(&config->visual.projectile_trajectory_visualizer)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.projectile_trajectory_constant_line_colour)
              ->enable_alpha_bar()
              ->set_render_on(&config->visual.projectile_trajectory_constant_line);

        gcs::dropdown(projectile_group, WXOR(L"Show trajectory flight"), true)
          ->add_option(WXOR(L"Enabled"), &config->visual.projectile_trajectory_dynamic_line)
          ->add_option(WXOR(L"Outlined"), &config->visual.projectile_trajectory_dynamic_outline)
          ->add_option(WXOR(L"Splash radius circle"), &config->visual.projectile_trajectory_dynamic_splash_circle)
          ->add_option(WXOR(L"Impact box"), &config->visual.projectile_trajectory_dynamic_box)
            ->set_render_on(&config->visual.projectile_trajectory_visualizer)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.projectile_trajectory_dynamic_line_colour)
              ->enable_alpha_bar()
             ->set_render_on(&config->visual.projectile_trajectory_dynamic_line);
      }
    }

    viewmodel->create_menu_settings(vm_main_group, &config->vm);

    // Text/Particles
    {
      gcs::toggle(other_group, WXOR(L"Show icons through walls"), &config->visual.show_icons_thru_walls);
      {
        gcs::dropdown(other_group, WXOR(L"Icons"), true)
          ->add_option(WXOR(L"Vaccinator"), &config->visual.show_vaccinator_icons_thru_walls)
          ->add_option(WXOR(L"Mannpower"), &config->visual.show_mannpower_icons_thru_walls)
            ->set_render_on(&config->visual.show_icons_thru_walls);

        gcs::dropdown(other_group, WXOR(L"Associated icon team"), false)
          ->add_option(WXOR(L"Friendly only"), &config->visual.show_friendly_team_icons)
          ->add_option(WXOR(L"Enemy only"), &config->visual.show_enemy_team_icons)
          ->add_option(WXOR(L"All"), &config->visual.show_both_team_icons)
            ->set_render_on(&config->visual.show_icons_thru_walls);
      }

      gcs::add_space(other_group);

      gcs::toggle(other_group, WXOR(L"Combat text"), &config->visual.combat_text_enabled);
      gcs::dropdown(other_group, WXOR(L"Options"), true)
        ->add_option(WXOR(L"Show through walls"), &config->visual.combat_text_show_thru_walls)
        ->add_option(WXOR(L"Show on cloaked/disguised"), &config->visual.combat_text_force_show_spies)
        ->add_option(WXOR(L"Show team damage"), &config->visual.combat_text_show_team_mate_dmg)
          ->set_render_on(&config->visual.combat_text_enabled);

      gcs::toggle(other_group, WXOR(L"Friends only"), &config->visual.combat_text_show_dmg_friends_only)
        ->set_render_on(&config->visual.combat_text_enabled)
        ->set_render_on(&config->visual.combat_text_show_team_mate_dmg);

      gcs::add_space(other_group);

      gcs::toggle(other_group, WXOR(L"Sniper LOS Laser"), &config->visual.sniper_los_enabled);
      gcs::toggle(other_group, WXOR(L"Ignore Z"), &config->visual.sniper_los_ignore_z)
        ->set_render_on(&config->visual.sniper_los_enabled);

      gcs::dropdown(other_group, WXOR(L"Team"), false)
          ->add_option(WXOR(L"Friendly only"), &config->visual.sniper_los_friendly_only)
          ->add_option(WXOR(L"Enemy only"), &config->visual.sniper_los_enemy_only)
          ->add_option(WXOR(L"All"), &config->visual.sniper_los_both)
            ->set_render_on(&config->visual.sniper_los_enabled);
    }

    // View / HUD
    {
      gcs::slider(view_hud_group, WXOR(L"Custom FOV"), WXOR(L"°"), 90, 140, GCS_SLIDER_TYPE_INT, &config->visual.custom_fov_slider);
      gcs::add_space(view_hud_group);

      gcs::toggle(view_hud_group, WXOR(L"Remove sniper scope overlay"), &config->visual.no_zoom_enabled);
      gcs::toggle(view_hud_group, WXOR(L"Remove sniper scope FOV"), &config->visual.no_zoom_fov);

      gcs::add_space(view_hud_group);
      gcs::toggle(view_hud_group, WXOR(L"No visual recoil"), &config->visual.no_visual_recoil);
      dispatch_user_msg->create_menu_settings(view_hud_group, &config->dum);

      gcs::toggle(view_hud_group, WXOR(L"No fog"), &config->visual.no_fog);
      gcs::toggle(view_hud_group, WXOR(L"No viewmodel bobbing"), &config->visual.no_viewmodel_bob);

      gcs::add_space(view_hud_group);
      thirdperson->create_menu_settings(view_hud_group, &config->tp);
      gcs::add_space(view_hud_group);

      freecam->create_menu_settings(view_hud_group, &config->freecam);
    }

    // Other
    {
      gcs::toggle(misc_group, WXOR(L"Show hitboxes on hit"), &config->visual.show_hitboxes_on_hit)
        ->add_module<gcs_component_colour_picker>()
          ->set_output_colour_ptr(config->visual.show_hitboxes_on_hit_colour)
          ->enable_alpha_bar()
          ->set_render_on(&config->visual.show_hitboxes_on_hit);

      gcs::add_space(misc_group);
      gcs::toggle(misc_group, WXOR(L"Spawn door transparency"), &config->visual.door_transparency_enabled);
      gcs::slider(misc_group, WXOR(L"Alpha"), L"", 0, 100, GCS_SLIDER_TYPE_FLOAT, &config->visual.door_transparency_value)
        ->set_render_on(&config->visual.door_transparency_enabled);

      gcs::add_space(misc_group);

      gcs::toggle(misc_group, WXOR(L"Draw crosshair at aimpoint"), &config->visual.draw_crosshair_at_aim_point);
      gcs::toggle(misc_group, WXOR(L"Force vaccinator beta shields"), &config->visual.use_alt_vaccinator_shields);

      gcs::add_space(misc_group);

      gcs::toggle(misc_group, WXOR(L"Hide sentry with wrangler"), &config->visual.hide_sentry_with_wrangler_out)
        ->set_help_text(WXOR(L"Will hide your sentry's model when you hold out the wrangler."));

      gcs::toggle(misc_group, WXOR(L"Override view with sentry"), &config->visual.override_view_with_sentry)
        ->set_help_text(WXOR(L"Will hide your sentry's model when you hold out the wrangler"))
        ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&config->visual.override_view_with_sentry_key)
          ->set_render_on(&config->visual.override_view_with_sentry)
          ->set_render_on(&config->visual.override_view_with_sentry_use_key);

      gcs::toggle(misc_group, WXOR(L"Use key"), &config->visual.override_view_with_sentry_use_key);
      gcs::dropdown(misc_group, WXOR(L"Key control"), false)
        ->add_option(WXOR(L"Held"), &config->visual.override_view_with_sentry_key_held)
        ->add_option(WXOR(L"Toggle"), &config->visual.override_view_with_sentry_key_toggle)
        ->add_option(WXOR(L"Double click"), &config->visual.override_view_with_sentry_key_double_click)
        ->set_render_on(&config->visual.override_view_with_sentry_use_key);
    }

    return group_host;
  }

  void create_player_chams_layer(gcs_component_group* group, i32 i){
    c_player_chams_settings* chams_settings = utils::get_player_chams_settings(i);
    assert(chams_settings != nullptr);

    gcs::toggle(group, WXOR(L"Enabled"), &chams_settings->enabled);

    if(i != WORLD_ESP_SETTING_TYPE_FAKEANGLE){
      gcs::toggle(group, WXOR(L"Show when cloaked"), &chams_settings->render_cloaked_spies)
        ->set_render_on(&chams_settings->enabled);
    }

    if(i == PLAYER_ESP_SETTING_TYPE_FRIEND){
      gcs::toggle(group, WXOR(L"Show localplayer"), &config->visual.player_chams_include_localplayer)
        ->set_render_on(&chams_settings->enabled);
    }

    gcs::toggle(group, WXOR(L"Ignore z"), &chams_settings->ignore_z)
      ->set_render_on(&chams_settings->enabled);

    if(i == WORLD_ESP_SETTING_TYPE_BACKTRACK){
      gcs::dropdown(group, WXOR(L"Base material"), false)
        ->add_option(WXOR(L"None"), &chams_settings->base_material_none)
        ->add_option(WXOR(L"Shaded"), &chams_settings->base_material_shaded)
        ->add_option(WXOR(L"Flat"), &chams_settings->base_material_flat)
        ->add_option(WXOR(L"Fresnel"), &chams_settings->base_material_frensel)
        ->add_option(WXOR(L"Tint"), &chams_settings->base_material_selfillum)
        ->add_option(WXOR(L"Wireframe"), &chams_settings->base_material_wireframe)
        ->set_render_on(&chams_settings->enabled)
        ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.backtrack_chams_colour)
            ->enable_alpha_bar()
            ->set_render_on(&chams_settings->enabled);
  
      gcs::dropdown(group, WXOR(L"Overlay material"), false)
        ->add_option(WXOR(L"None"), &chams_settings->overlay_material_none)
        ->add_option(WXOR(L"Shaded"), &chams_settings->overlay_material_shaded)
        ->add_option(WXOR(L"Flat"), &chams_settings->overlay_material_flat)
        ->add_option(WXOR(L"Fresnel"), &chams_settings->overlay_material_fresnel)
        ->add_option(WXOR(L"Tint"), &chams_settings->overlay_material_selfillum)
        ->add_option(WXOR(L"Wireframe"), &chams_settings->overlay_material_wireframe)
        ->set_render_on(&chams_settings->enabled)
        ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.backtrack_chams_overlay_colour)
            ->enable_alpha_bar()
            ->set_render_on(&chams_settings->enabled);
    }
    else if(i == WORLD_ESP_SETTING_TYPE_FAKEANGLE){
      gcs::dropdown(group, WXOR(L"Base material"), false)
        ->add_option(WXOR(L"None"), &chams_settings->base_material_none)
        ->add_option(WXOR(L"Shaded"), &chams_settings->base_material_shaded)
        ->add_option(WXOR(L"Flat"), &chams_settings->base_material_flat)
        ->add_option(WXOR(L"Fresnel"), &chams_settings->base_material_frensel)
        ->add_option(WXOR(L"Tint"), &chams_settings->base_material_selfillum)
        ->add_option(WXOR(L"Wireframe"), &chams_settings->base_material_wireframe)
        ->set_render_on(&chams_settings->enabled)
        ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.fake_angle_chams_colour)
            ->enable_alpha_bar()
            ->set_render_on(&chams_settings->enabled);
  
      gcs::dropdown(group, WXOR(L"Overlay material"), false)
        ->add_option(WXOR(L"None"), &chams_settings->overlay_material_none)
        ->add_option(WXOR(L"Shaded"), &chams_settings->overlay_material_shaded)
        ->add_option(WXOR(L"Flat"), &chams_settings->overlay_material_flat)
        ->add_option(WXOR(L"Fresnel"), &chams_settings->overlay_material_fresnel)
        ->add_option(WXOR(L"Tint"), &chams_settings->overlay_material_selfillum)
        ->add_option(WXOR(L"Wireframe"), &chams_settings->overlay_material_wireframe)
        ->set_render_on(&chams_settings->enabled)
        ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.fake_angle_chams_overlay_colour)
            ->enable_alpha_bar()
            ->set_render_on(&chams_settings->enabled);
    }
    else{
      gcs::dropdown(group, WXOR(L"Base material"), false)
        ->add_option(WXOR(L"None"), &chams_settings->base_material_none)
        ->add_option(WXOR(L"Shaded"), &chams_settings->base_material_shaded)
        ->add_option(WXOR(L"Flat"), &chams_settings->base_material_flat)
        ->add_option(WXOR(L"Fresnel"), &chams_settings->base_material_frensel)
        ->add_option(WXOR(L"Tint"), &chams_settings->base_material_selfillum)
        ->add_option(WXOR(L"Wireframe"), &chams_settings->base_material_wireframe)
          ->set_render_on(&chams_settings->enabled);
  
      gcs::dropdown(group, WXOR(L"Overlay material"), false)
        ->add_option(WXOR(L"None"), &chams_settings->overlay_material_none)
        ->add_option(WXOR(L"Shaded"), &chams_settings->overlay_material_shaded)
        ->add_option(WXOR(L"Flat"), &chams_settings->overlay_material_flat)
        ->add_option(WXOR(L"Fresnel"), &chams_settings->overlay_material_fresnel)
        ->add_option(WXOR(L"Tint"), &chams_settings->overlay_material_selfillum)
        ->add_option(WXOR(L"Wireframe"), &chams_settings->overlay_material_wireframe)
          ->set_render_on(&chams_settings->enabled);
    }
  }

  void create_world_chams_layer(gcs_component_group* group, i32 i){
    c_world_chams_settings* chams_settings = utils::get_world_chams_settings(i);
    assert(chams_settings != nullptr);

    gcs::toggle(group, WXOR(L"Enabled"), &chams_settings->team_enabled);
    gcs::toggle(group, WXOR(L"Ignore z"), &chams_settings->ignore_z)
      ->set_render_on(&chams_settings->team_enabled);

    // Material settings.
    {
      gcs::dropdown(group, WXOR(L"Base material"), false)
        ->add_option(WXOR(L"None"), &chams_settings->base_material_none)
        ->add_option(WXOR(L"Shaded"), &chams_settings->base_material_shaded)
        ->add_option(WXOR(L"Flat"), &chams_settings->base_material_flat)
        ->add_option(WXOR(L"Fresnel"), &chams_settings->base_material_frensel)
        ->add_option(WXOR(L"Tint"), &chams_settings->base_material_selfillum)
        ->add_option(WXOR(L"Wireframe"), &chams_settings->base_material_wireframe)
        ->set_render_on(&chams_settings->team_enabled);

      gcs::dropdown(group, WXOR(L"Overlay material"), false)
        ->add_option(WXOR(L"None"), &chams_settings->overlay_material_none)
        ->add_option(WXOR(L"Shaded"), &chams_settings->overlay_material_shaded)
        ->add_option(WXOR(L"Flat"), &chams_settings->overlay_material_flat)
        ->add_option(WXOR(L"Fresnel"), &chams_settings->overlay_material_fresnel)
        ->add_option(WXOR(L"Tint"), &chams_settings->overlay_material_selfillum)
        ->add_option(WXOR(L"Wireframe"), &chams_settings->overlay_material_wireframe)
        ->set_render_on(&chams_settings->team_enabled);

    }

    if(i == WORLD_ESP_SETTING_TYPE_UNASSIGNED_TEAM){
      gcs::toggle(group, WXOR(L"Ammopack"), &chams_settings->object_enabled[4])
        ->set_render_on(&chams_settings->team_enabled);

      gcs::toggle(group, WXOR(L"Healthpack"), &chams_settings->object_enabled[5])
        ->set_render_on(&chams_settings->team_enabled);

      gcs::toggle(group, WXOR(L"Other entities"), &chams_settings->object_enabled[7])
        ->set_render_on(&chams_settings->team_enabled);

      gcs::dropdown(group, WXOR(L"List"), true)
        ->add_option(WXOR(L"Bombs"), &chams_settings->ent_bombs)
        ->add_option(WXOR(L"Lunchables"), &chams_settings->ent_lunchables)
        ->add_option(WXOR(L"Boss"), &chams_settings->ent_boss)
        ->add_option(WXOR(L"Powerup"), &chams_settings->ent_powerup)
        ->add_option(WXOR(L"Spellbooks"), &chams_settings->ent_spellbooks)
        ->add_option(WXOR(L"Money"), &chams_settings->ent_money)
        ->add_option(WXOR(L"Halloween pickups"), &chams_settings->ent_halloween_pickup)
          ->set_render_on(&chams_settings->object_enabled[7])
          ->set_render_on(&chams_settings->team_enabled);

      return;
    }

    gcs::dropdown(group, WXOR(L"Buildables"), true)
      ->add_option(WXOR(L"Sentry"), &chams_settings->object_enabled[0])
      ->add_option(WXOR(L"Dispenser"), &chams_settings->object_enabled[1])
      ->add_option(WXOR(L"Teleporter"), &chams_settings->object_enabled[2])
      ->set_render_on(&chams_settings->team_enabled);

    gcs::toggle(group, WXOR(L"Objective"), &chams_settings->object_enabled[3])
      ->set_render_on(&chams_settings->team_enabled);

    gcs::toggle(group, WXOR(L"Projectiles"), &chams_settings->object_enabled[6])
      ->set_render_on(&chams_settings->team_enabled);

    gcs::dropdown(group, WXOR(L"List"), true)
      ->add_option(WXOR(L"Rocket"), &chams_settings->ent_projectile_rocket)
      ->add_option(WXOR(L"Sticky"), &chams_settings->ent_projectile_sticky)
      ->add_option(WXOR(L"Pipebomb"), &chams_settings->ent_projectile_pipebomb)
      ->add_option(WXOR(L"Arrow"), &chams_settings->ent_projectile_arrow)
      ->add_option(WXOR(L"Flare"), &chams_settings->ent_projectile_flare)
      ->add_option(WXOR(L"Throwables"), &chams_settings->ent_projectile_throwables)
      ->add_option(WXOR(L"Spells"), &chams_settings->ent_projectile_spells)
        ->set_render_on(&chams_settings->object_enabled[6])
        ->set_render_on(&chams_settings->team_enabled);

    gcs::toggle(group, WXOR(L"Other entities"), &chams_settings->object_enabled[7])
      ->set_render_on(&chams_settings->team_enabled);

    gcs::dropdown(group, WXOR(L"List"), true)
      ->add_option(WXOR(L"Bombs"), &chams_settings->ent_bombs)
      ->add_option(WXOR(L"Boss"), &chams_settings->ent_boss)
      ->add_option(WXOR(L"Revive marker"), &chams_settings->ent_revive_marker)
        ->set_render_on(&chams_settings->object_enabled[7])
        ->set_render_on(&chams_settings->team_enabled);
  }

  gcs_component_group_host* create_visuals_chams(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Chams"), pos, size);
    {
      gcs_component_group* player_chams_group         = group_host->new_group(WXOR(L"Player chams"));
      gcs_component_group* player_chams_enemy_group   = player_chams_group->new_category(WXOR(L"Enemy"));
      gcs_component_group* player_chams_team_group    = player_chams_group->new_category(WXOR(L"Team"));
      gcs_component_group* player_chams_friend_group  = player_chams_group->new_category(WXOR(L"Friends"));
      gcs_component_group* player_chams_colours_group = player_chams_group->new_category(WXOR(L"Colours"));


      gcs_component_group* world_chams_group          = group_host->new_group(WXOR(L"World chams"));
      gcs_component_group* world_chams_enemy_group    = world_chams_group->new_category(WXOR(L"Enemy"));
      gcs_component_group* world_chams_team_group     = world_chams_group->new_category(WXOR(L"Team"));
      gcs_component_group* world_chams_unassign_group = world_chams_group->new_category(WXOR(L"Other"));
      gcs_component_group* world_chams_colours_group  = world_chams_group->new_category(WXOR(L"Colours"));


      gcs_component_group* special_chams_group        = group_host->new_group(WXOR(L""));
      gcs_component_group* backtrack_chams_group      = special_chams_group->new_category(WXOR(L"History"));
      gcs_component_group* fake_angle_chams_group     = special_chams_group->new_category(WXOR(L"Fakeang"));
      gcs_component_group* hand_chams_group           = special_chams_group->new_category(WXOR(L"Hand"));
      gcs_component_group* weapon_chams_group         = special_chams_group->new_category(WXOR(L"Weapon"));


      group_host->update();

      // Player chams
      {
        create_player_chams_layer(player_chams_enemy_group, PLAYER_ESP_SETTING_TYPE_RED_TEAM);
        create_player_chams_layer(player_chams_team_group, PLAYER_ESP_SETTING_TYPE_BLU_TEAM);
        create_player_chams_layer(player_chams_friend_group, PLAYER_ESP_SETTING_TYPE_FRIEND);

        // HINT-HEADER: Player chams colour
        {
          gcs::dropdown(player_chams_colours_group, WXOR(L"Scheme"), false)
             ->add_option(WXOR(L"Team based"), &config->visual.player_chams_use_team_colours)
             ->add_option(WXOR(L"Enemy / friendly"), &config->visual.player_chams_use_enemyteam_colours);

           // Team based.
           {
                gcs::label(player_chams_colours_group, WXOR(L"RED team"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->enable_alpha_bar()
                  ->set_output_colour_ptr(config->visual.player_chams_red_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"BLU team"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->enable_alpha_bar()
                  ->set_output_colour_ptr(config->visual.player_chams_blu_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay RED team"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->enable_alpha_bar()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_red_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay BLU team"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->enable_alpha_bar()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_blu_team_colour);
           }

           // Enemy / friendly based.
           {
                gcs::label(player_chams_colours_group, WXOR(L"Enemy"))
                  ->set_render_on(&config->visual.player_chams_use_enemyteam_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->enable_alpha_bar()
                  ->set_output_colour_ptr(config->visual.player_chams_enemy_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Team"))
                  ->set_render_on(&config->visual.player_chams_use_enemyteam_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->enable_alpha_bar()
                  ->set_output_colour_ptr(config->visual.player_chams_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay Enemy"))
                  ->set_render_on(&config->visual.player_chams_use_enemyteam_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->enable_alpha_bar()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_enemy_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay Team"))
                  ->set_render_on(&config->visual.player_chams_use_enemyteam_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->enable_alpha_bar()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_team_colour);
           }

           gcs::add_space(player_chams_colours_group);
           gcs::toggle(player_chams_colours_group, WXOR(L"Aimbot target"), &config->visual.player_chams_aimbot_target)
               ->add_module<gcs_component_colour_picker>()
               ->enable_alpha_bar()
               ->set_output_colour_ptr(config->visual.player_chams_aimbot_target_colour);

           gcs::toggle(player_chams_colours_group, WXOR(L"Overlay Aimbot target"), &config->visual.player_chams_overlay_aimbot_target)
               ->add_module<gcs_component_colour_picker>()
               ->enable_alpha_bar()
               ->set_output_colour_ptr(config->visual.player_chams_overlay_aimbot_target_colour);

           gcs::add_space(player_chams_colours_group);
           gcs::dropdown(player_chams_colours_group, WXOR(L"Friend scheme"), false)
               ->add_option(WXOR(L"Single colour"), &config->visual.player_chams_friends_use_single_colour)
               ->add_option(WXOR(L"Use player scheme"), &config->visual.player_chams_friends_use_mult_colour);

           gcs::label(player_chams_colours_group, WXOR(L"Friends"))
               ->set_render_on(&config->visual.player_chams_friends_use_single_colour)
               ->add_module<gcs_component_colour_picker>()
               ->enable_alpha_bar()
               ->set_output_colour_ptr(config->visual.player_chams_friend_colour);

           gcs::label(player_chams_colours_group, WXOR(L"Overlay Friends"))
               ->set_render_on(&config->visual.player_chams_friends_use_single_colour)
               ->add_module<gcs_component_colour_picker>()
               ->enable_alpha_bar()
               ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_colour);

           // Team based.
           {
                gcs::label(player_chams_colours_group, WXOR(L"RED team"))
                  ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->enable_alpha_bar()
                  ->set_output_colour_ptr(config->visual.player_chams_friend_red_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"BLU team"))
                  ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->enable_alpha_bar()
                  ->set_output_colour_ptr(config->visual.player_chams_friend_blu_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay RED team"))
                  ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->enable_alpha_bar()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_red_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay BLU team"))
                  ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->enable_alpha_bar()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_blu_team_colour);
           }

           // Enemy / friendly based.
           {
                gcs::label(player_chams_colours_group, WXOR(L"Enemy"))
                  ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                  ->set_render_on(&config->visual.player_chams_use_enemyteam_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->enable_alpha_bar()
                  ->set_output_colour_ptr(config->visual.player_chams_friend_enemy_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Team"))
                  ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                  ->set_render_on(&config->visual.player_chams_use_enemyteam_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->enable_alpha_bar()
                  ->set_output_colour_ptr(config->visual.player_chams_friend_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay Enemy"))
                  ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                  ->set_render_on(&config->visual.player_chams_use_enemyteam_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->enable_alpha_bar()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_enemy_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay Team"))
                  ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                  ->set_render_on(&config->visual.player_chams_use_enemyteam_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->enable_alpha_bar()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_team_colour);
           }
        }
      }

      // World chams
      {
        create_world_chams_layer(world_chams_enemy_group, WORLD_ESP_SETTING_TYPE_RED_TEAM);
        create_world_chams_layer(world_chams_team_group, WORLD_ESP_SETTING_TYPE_BLU_TEAM);
        create_world_chams_layer(world_chams_unassign_group, WORLD_ESP_SETTING_TYPE_UNASSIGNED_TEAM);

        // HINT-HEADER: World chams colour
        {
          gcs::dropdown(world_chams_colours_group, WXOR(L"Scheme"), false)
            ->add_option(WXOR(L"Team based"), &config->visual.world_chams_use_team_colours)
            ->add_option(WXOR(L"Enemy / friendly"), &config->visual.world_chams_use_enemyteam_colours);

          // Team based.
          {
            // Base
            {
               gcs::label(world_chams_colours_group, WXOR(L"RED team"))
                 ->set_render_on(&config->visual.world_chams_use_team_colours)
                 ->add_module<gcs_component_colour_picker>()
                 ->enable_alpha_bar()
                 ->set_output_colour_ptr(config->visual.world_chams_red_team_colour);

               gcs::label(world_chams_colours_group, WXOR(L"BLU team"))
                 ->set_render_on(&config->visual.world_chams_use_team_colours)
                 ->add_module<gcs_component_colour_picker>()
                 ->enable_alpha_bar()
                 ->set_output_colour_ptr(config->visual.world_chams_blu_team_colour);
            }

            // Overlay
            {
               gcs::label(world_chams_colours_group, WXOR(L"Overlay RED team"))
                 ->set_render_on(&config->visual.world_chams_use_team_colours)
                 ->add_module<gcs_component_colour_picker>()
                 ->enable_alpha_bar()
                 ->set_output_colour_ptr(config->visual.world_chams_overlay_red_team_colour);

               gcs::label(world_chams_colours_group, WXOR(L"Overlay BLU team"))
                 ->set_render_on(&config->visual.world_chams_use_team_colours)
                 ->add_module<gcs_component_colour_picker>()
                 ->enable_alpha_bar()
                 ->set_output_colour_ptr(config->visual.world_chams_overlay_blu_team_colour);
            }
          }

          // Enemy / friendly based.
          {
            // Base
            {
              gcs::label(world_chams_colours_group, WXOR(L"Enemy"))
                ->set_render_on(&config->visual.world_chams_use_enemyteam_colours)
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_enemy_colour);
              gcs::label(world_chams_colours_group, WXOR(L"Team"))
                ->set_render_on(&config->visual.world_chams_use_enemyteam_colours)
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_team_colour);
            }

            // Overlay
            {
              gcs::label(world_chams_colours_group, WXOR(L"Overlay Enemy"))
                ->set_render_on(&config->visual.world_chams_use_enemyteam_colours)
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_enemy_colour);
              gcs::label(world_chams_colours_group, WXOR(L"Overlay Team"))
                ->set_render_on(&config->visual.world_chams_use_enemyteam_colours)
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_team_colour);
            }
          }

          gcs::add_space(world_chams_colours_group);
          gcs::toggle(world_chams_colours_group, WXOR(L"Aimbot target"), &config->visual.world_chams_aimbot_target)
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
            ->set_output_colour_ptr(config->visual.world_chams_aimbot_target_colour);

          gcs::toggle(world_chams_colours_group, WXOR(L"Overlay Aimbot target"), &config->visual.world_chams_overlay_aimbot_target)
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
            ->set_output_colour_ptr(config->visual.world_chams_overlay_aimbot_target_colour);

          gcs::add_space(world_chams_colours_group);

          // Others
          {
            // Base
            {
              gcs::label(world_chams_colours_group, WXOR(L"Unassigned team"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_unassigned_team_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Healthpack"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_health_pack_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Ammopack"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_ammo_pack_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Boss"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_boss_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Bombs"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_bomb_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Lunchables"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_lunchables_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Money"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_money_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Powerup"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_powerup_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Normal spellbooks"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_spellbook_normal_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Rare spellbooks"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_spellbook_rare_colour);
            }

            // Overlay
            {
              gcs::label(world_chams_colours_group, WXOR(L"Overlay Unassigned team"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_unassigned_team_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Overlay Healthpack"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_health_pack_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Overlay Ammopack"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_ammo_pack_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Overlay Boss"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_boss_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Overlay Bombs"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_bomb_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Overlay Lunchables"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_lunchables_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Overlay Money"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_money_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Overlay Powerup"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_powerup_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Overlay Normal spellbooks"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_spellbook_normal_colour);

              gcs::label(world_chams_colours_group, WXOR(L"Overlay Rare spellbooks"))
                ->add_module<gcs_component_colour_picker>()
                ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_spellbook_rare_colour);
            }
          }
        }
      }

      // Special chams.
      {
        create_player_chams_layer(backtrack_chams_group, WORLD_ESP_SETTING_TYPE_BACKTRACK);                 
        create_player_chams_layer(fake_angle_chams_group, WORLD_ESP_SETTING_TYPE_FAKEANGLE);
        // HINT-HEADER: Hand chams
        {         
          gcs::toggle(hand_chams_group, WXOR(L"Enabled"), &config->visual.hand_chams_enabled);
          {
            gcs::toggle(hand_chams_group, WXOR(L"Remove"), &config->visual.hand_chams_removal)
              ->set_render_on(&config->visual.hand_chams_enabled);
  
            // Base hands material
            {
              gcs::dropdown(hand_chams_group, WXOR(L"Base material"), false)
                ->add_option(WXOR(L"None"), &config->visual.hand_chams_base_material_none)
                ->add_option(WXOR(L"Shaded"), &config->visual.hand_chams_base_material_shaded)
                ->add_option(WXOR(L"Flat"), &config->visual.hand_chams_base_material_flat)
                ->add_option(WXOR(L"Fresnel"), &config->visual.hand_chams_base_material_fresnel)
                ->add_option(WXOR(L"Tint"), &config->visual.hand_chams_base_material_selfillum)
                ->add_option(WXOR(L"Wireframe"), &config->visual.hand_chams_base_material_wireframe)
                  ->set_render_on(&config->visual.hand_chams_enabled)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.hand_chams_base_material_colour)
                  ->enable_alpha_bar()
                    ->set_render_on(&config->visual.hand_chams_enabled);
            }
  
            // Overlay hands material
            {
              gcs::dropdown(hand_chams_group, WXOR(L"Overlay material"), false)
                ->add_option(WXOR(L"None"), &config->visual.hand_chams_overlay_material_none)
                ->add_option(WXOR(L"Shaded"), &config->visual.hand_chams_overlay_material_shaded)
                ->add_option(WXOR(L"Flat"), &config->visual.hand_chams_overlay_material_flat)
                ->add_option(WXOR(L"Fresnel"), &config->visual.hand_chams_overlay_material_fresnel)
                ->add_option(WXOR(L"Tint"), &config->visual.hand_chams_overlay_material_selfillum)
                ->add_option(WXOR(L"Wireframe"), &config->visual.hand_chams_overlay_material_wireframe)
                  ->set_render_on(&config->visual.hand_chams_enabled)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.hand_chams_overlay_material_colour)
                  ->enable_alpha_bar()
                    ->set_render_on(&config->visual.hand_chams_enabled);
            }
          }
        }

        // HINT-HEADER: Weapon chams
        gcs::toggle(weapon_chams_group, WXOR(L"Enabled"), &config->visual.weapon_chams_enabled);
        {
          // Base hands material
          {
            gcs::dropdown(weapon_chams_group, WXOR(L"Base material"), false)
              ->add_option(WXOR(L"None"), &config->visual.weapon_chams_base_material_none)
              ->add_option(WXOR(L"Shaded"), &config->visual.weapon_chams_base_material_shaded)
              ->add_option(WXOR(L"Flat"), &config->visual.weapon_chams_base_material_flat)
              ->add_option(WXOR(L"Fresnel"), &config->visual.weapon_chams_base_material_fresnel)
              ->add_option(WXOR(L"Tint"), &config->visual.weapon_chams_base_material_selfillum)
              ->add_option(WXOR(L"Wireframe"), &config->visual.weapon_chams_base_material_wireframe)
                ->set_render_on(&config->visual.weapon_chams_enabled)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.weapon_chams_base_material_colour)
                ->enable_alpha_bar()
                  ->set_render_on(&config->visual.weapon_chams_enabled);
          }

          // Overlay hands material
          {
            gcs::dropdown(weapon_chams_group, WXOR(L"Overlay material"), false)
              ->add_option(WXOR(L"None"), &config->visual.weapon_chams_overlay_material_none)
              ->add_option(WXOR(L"Shaded"), &config->visual.weapon_chams_overlay_material_shaded)
              ->add_option(WXOR(L"Flat"), &config->visual.weapon_chams_overlay_material_flat)
              ->add_option(WXOR(L"Fresnel"), &config->visual.weapon_chams_overlay_material_fresnel)
              ->add_option(WXOR(L"Tint"), &config->visual.weapon_chams_overlay_material_selfillum)
              ->add_option(WXOR(L"Wireframe"), &config->visual.weapon_chams_overlay_material_wireframe)
                ->set_render_on(&config->visual.weapon_chams_enabled)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.weapon_chams_overlay_material_colour)
                ->enable_alpha_bar()
                  ->set_render_on(&config->visual.weapon_chams_enabled);
          }
        }
      }
      
    }

    return group_host;
  }

  gcs_component_group_host* create_visuals_glow(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Glow"), pos, size);
    {

      gcs_component_group* glow_group          = group_host->new_group(WXOR(L"Entities"));
      gcs_component_group* glow_player_group   = glow_group->new_category(WXOR(L"Players"));
      gcs_component_group* glow_world_group    = glow_group->new_category(WXOR(L"World"));
      gcs_component_group* glow_health_group    = glow_group->new_category(WXOR(L"Health"));
      gcs_component_group* colours_glow_group  = glow_group->new_category(WXOR(L"Colours"));
    
      gcs_component_group* glow_misc_group        = group_host->new_group(WXOR(L"Misc"));
      gcs_component_group* glow_misc_colour_group = glow_misc_group->new_category(WXOR(L"Features"));
      gcs_component_group* glow_misc_mat_group    = glow_misc_group->new_category(WXOR(L"Glow Settings"));

      group_host->update();

      // HINT-HEADER: Glow.
      {

        // Players
        {
          gcs::toggle(glow_player_group, WXOR(L"Enabled"), &config->visual.player_glow_enabled);
          {
            gcs::dropdown(glow_player_group, WXOR(L"Show"), true)
              ->add_option(WXOR(L"Cloaked Spies"), &config->visual.player_glow_render_cloaked_spies)
              ->add_option(WXOR(L"Localplayer"), &config->visual.player_glow_include_localplayer)
              ->add_option(WXOR(L"Enemies"), &config->visual.player_glow_render_enemies)
              ->add_option(WXOR(L"Team"), &config->visual.player_glow_render_friendlies)
              ->add_option(WXOR(L"Friends"), &config->visual.player_glow_render_steamfriends)
              ->set_render_on(&config->visual.player_glow_enabled);
          }
        }

        // World
        {
          gcs::toggle(glow_world_group, WXOR(L"Enabled"), &config->visual.world_glow_enabled);
          {
            gcs::dropdown(glow_world_group, WXOR(L"Show"), true)
              ->add_option(WXOR(L"Enemies"), &config->visual.world_glow_render_enemies)
              ->add_option(WXOR(L"Team"), &config->visual.world_glow_render_friendlies)
              ->set_render_on(&config->visual.world_glow_enabled);

            gcs::dropdown(glow_world_group, WXOR(L"Buildables"), true)
              ->add_option(WXOR(L"Sentry"), &config->visual.glow_object_enabled[0])
              ->add_option(WXOR(L"Dispenser"), &config->visual.glow_object_enabled[1])
              ->add_option(WXOR(L"Teleporter"), &config->visual.glow_object_enabled[2])
              ->set_render_on(&config->visual.world_glow_enabled);

            gcs::toggle(glow_world_group, WXOR(L"Objective"), &config->visual.glow_object_enabled[3])
              ->set_render_on(&config->visual.world_glow_enabled);

            gcs::toggle(glow_world_group, WXOR(L"Ammopack"), &config->visual.glow_object_enabled[4])
              ->set_render_on(&config->visual.world_glow_enabled);

            gcs::toggle(glow_world_group, WXOR(L"Healthpack"), &config->visual.glow_object_enabled[5])
              ->set_render_on(&config->visual.world_glow_enabled);

            gcs::toggle(glow_world_group, WXOR(L"Projectiles"), &config->visual.glow_object_enabled[6])
              ->set_render_on(&config->visual.world_glow_enabled);

            gcs::dropdown(glow_world_group, WXOR(L"List"), true)
              ->add_option(WXOR(L"Rocket"), &config->visual.world_glow_ent_projectile_rocket)
              ->add_option(WXOR(L"Sticky"), &config->visual.world_glow_ent_projectile_sticky)
              ->add_option(WXOR(L"Pipebomb"), &config->visual.world_glow_ent_projectile_pipebomb)
              ->add_option(WXOR(L"Arrow"), &config->visual.world_glow_ent_projectile_arrow)
              ->add_option(WXOR(L"Flare"), &config->visual.world_glow_ent_projectile_flare)
              ->add_option(WXOR(L"Throwables"), &config->visual.world_glow_ent_projectile_throwables)
              ->add_option(WXOR(L"Spells"), &config->visual.world_glow_ent_projectile_spells)
                ->set_render_on(&config->visual.glow_object_enabled[6])
                ->set_render_on(&config->visual.world_glow_enabled);

            gcs::toggle(glow_world_group, WXOR(L"Other entities"), &config->visual.glow_object_enabled[7])
              ->set_render_on(&config->visual.world_glow_enabled);

            gcs::dropdown(glow_world_group, WXOR(L"List"), true)
              ->add_option(WXOR(L"Bombs"), &config->visual.world_glow_ent_bombs)
              ->add_option(WXOR(L"Lunchables"), &config->visual.world_glow_ent_lunchables)
              ->add_option(WXOR(L"Boss"), &config->visual.world_glow_ent_boss)
              ->add_option(WXOR(L"Powerup"), &config->visual.world_glow_ent_powerup)
              ->add_option(WXOR(L"Spellbooks"), &config->visual.world_glow_ent_spellbooks)
              ->add_option(WXOR(L"Money"), &config->visual.world_glow_ent_money)
              ->add_option(WXOR(L"Halloween pickups"), &config->visual.world_glow_ent_halloween_pickup)
              ->add_option(WXOR(L"Revive marker"), &config->visual.world_glow_ent_revive_marker)
                ->set_render_on(&config->visual.glow_object_enabled[7])
                ->set_render_on(&config->visual.world_glow_enabled);
          }
        }

        // Health
        {
          gcs::toggle(glow_health_group, WXOR(L"Health Colour"), &config->visual.health_glow_enabled);
          {
            gcs::dropdown(glow_health_group, WXOR(L"Apply to"), true)
              ->add_option(WXOR(L"Players"), &config->visual.health_glow_players)
              ->add_option(WXOR(L"Sentry"), &config->visual.health_glow_sentries)
              ->add_option(WXOR(L"Dispenser"), &config->visual.health_glow_dispenser)
              ->add_option(WXOR(L"Teleporter"), &config->visual.health_glow_teleporter)
                ->set_render_on(&config->visual.health_glow_enabled); 
  
            gcs::dropdown(glow_health_group, WXOR(L"Show"), true)
              ->add_option(WXOR(L"Localplayer"), &config->visual.health_glow_show_localplayer)
              ->add_option(WXOR(L"Team"), &config->visual.health_glow_show_friendly)
              ->add_option(WXOR(L"Enemy"), &config->visual.health_glow_show_enemy)
              ->add_option(WXOR(L"Friends"), &config->visual.health_glow_show_friends)
                ->set_render_on(&config->visual.health_glow_enabled);
          }
        }

    
        // Glow colours
        {      
          // HINT-HEADER: Player glow colours
          {
            gcs::dropdown(colours_glow_group, WXOR(L"Player scheme"), false)
              ->add_option(WXOR(L"Team based"), &config->visual.player_glow_use_team_colours)
              ->add_option(WXOR(L"Enemy / friendly"), &config->visual.player_glow_use_enemyteam_colours);

            // Team based.
            {
             gcs::label(colours_glow_group, WXOR(L"RED team"))
               ->set_render_on(&config->visual.player_glow_use_team_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.player_glow_red_team_colour);
             gcs::label(colours_glow_group, WXOR(L"BLU team"))
               ->set_render_on(&config->visual.player_glow_use_team_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.player_glow_blu_team_colour);
            }

            // Enemy / friendly based.
            {
              gcs::label(colours_glow_group, WXOR(L"Enemy"))
                ->set_render_on(&config->visual.player_glow_use_enemyteam_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_glow_enemy_colour);
              gcs::label(colours_glow_group, WXOR(L"Team"))
                ->set_render_on(&config->visual.player_glow_use_enemyteam_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_glow_team_colour);
            }

            gcs::add_space(colours_glow_group);
            gcs::toggle(colours_glow_group, WXOR(L"Aimbot target"), &config->visual.player_glow_aimbot_target)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.player_glow_aimbot_target_colour);
            gcs::add_space(colours_glow_group);

            gcs::dropdown(colours_glow_group, WXOR(L"Friend scheme"), false)
              ->add_option(WXOR(L"Single colour"), &config->visual.player_glow_friends_use_single_colour)
              ->add_option(WXOR(L"Use player scheme"), &config->visual.player_glow_friends_use_mult_colour);
            gcs::label(colours_glow_group, WXOR(L"Friends"))
              ->set_render_on(&config->visual.player_glow_friends_use_single_colour)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.player_glow_friend_colour);

            // Team based.
            {
              gcs::label(colours_glow_group, WXOR(L"RED team"))
                ->set_render_on(&config->visual.player_glow_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_glow_friend_red_team_colour);
              gcs::label(colours_glow_group, WXOR(L"BLU team"))
                ->set_render_on(&config->visual.player_glow_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_glow_friend_blu_team_colour);
            }

            // Enemy / friendly based.
            {
             gcs::label(colours_glow_group, WXOR(L"Enemy"))
               ->set_render_on(&config->visual.player_glow_friends_use_mult_colour)
               ->set_render_on(&config->visual.player_glow_use_enemyteam_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.player_glow_friend_enemy_colour);
             gcs::label(colours_glow_group, WXOR(L"Team"))
               ->set_render_on(&config->visual.player_glow_friends_use_mult_colour)
               ->set_render_on(&config->visual.player_glow_use_enemyteam_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.player_glow_friend_team_colour);
            }
          }

          gcs::add_space(colours_glow_group);

          // HINT-HEADER: Player glow colours
          {
            gcs::dropdown(colours_glow_group, WXOR(L"World scheme"), false)
              ->add_option(WXOR(L"Team based"), &config->visual.world_glow_use_team_colours)
              ->add_option(WXOR(L"Enemy / friendly"), &config->visual.world_glow_use_enemyteam_colours);

            // Team based.
            {
              gcs::label(colours_glow_group, WXOR(L"RED team"))
                ->set_render_on(&config->visual.world_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_red_team_colour);
              gcs::label(colours_glow_group, WXOR(L"BLU team"))
                ->set_render_on(&config->visual.world_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_blu_team_colour);
            }

            // Enemy / friendly based.
            {
              gcs::label(colours_glow_group, WXOR(L"Enemy"))
                ->set_render_on(&config->visual.world_glow_use_enemyteam_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_enemy_colour);
              gcs::label(colours_glow_group, WXOR(L"Team"))
                ->set_render_on(&config->visual.world_glow_use_enemyteam_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_team_colour);
            }

            gcs::add_space(colours_glow_group);
            gcs::toggle(colours_glow_group, WXOR(L"Aimbot target"), &config->visual.world_glow_aimbot_target)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_glow_aimbot_target_colour);

            gcs::add_space(colours_glow_group);
            {
              gcs::label(colours_glow_group, WXOR(L"Unassigned team"))
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_unassigned_team_colour);

              gcs::label(colours_glow_group, WXOR(L"Healthpack"))
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_health_pack_colour);

              gcs::label(colours_glow_group, WXOR(L"Ammopack"))
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_ammo_pack_colour);

              gcs::label(colours_glow_group, WXOR(L"Boss"))
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_boss_colour);

              gcs::label(colours_glow_group, WXOR(L"Bombs"))
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_bomb_colour);

              gcs::label(colours_glow_group, WXOR(L"Lunchables"))
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_lunchables_colour);

              gcs::label(colours_glow_group, WXOR(L"Money"))
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_money_colour);

              gcs::label(colours_glow_group, WXOR(L"Powerup"))
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_powerup_colour);

              gcs::label(colours_glow_group, WXOR(L"Normal spellbooks"))
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_spellbook_normal_colour);

              gcs::label(colours_glow_group, WXOR(L"Rare spellbooks"))
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_spellbook_rare_colour);
            }
          }
        }
      }

      // Misc Colours
      {
        gcs::toggle(glow_misc_colour_group, WXOR(L"Backtrack glow"), &config->visual.backtrack_glow_enabled)
          ->add_module<gcs_component_colour_picker>()
          ->enable_alpha_bar()
          ->set_output_colour_ptr(config->visual.backtrack_glow_colour)
          ->set_render_on(&config->visual.backtrack_glow_enabled);

        gcs::toggle(glow_misc_colour_group, WXOR(L"Show when cloaked"), &config->visual.backtrack_glow_render_cloaked_spies)
          ->set_render_on(&config->visual.backtrack_glow_enabled);

        gcs::add_space(glow_misc_colour_group, &config->visual.backtrack_glow_enabled);

        gcs::toggle(glow_misc_colour_group, WXOR(L"Fake-angle glow"), &config->visual.fake_angle_glow_enabled)
          ->add_module<gcs_component_colour_picker>()
          ->enable_alpha_bar()
          ->set_output_colour_ptr(config->visual.fake_angle_glow_colour)
          ->set_render_on(&config->visual.fake_angle_glow_enabled);

      }

      // Material settings for glow.
      {
          gcs::dropdown(glow_misc_mat_group, WXOR(L"Styles"), true)
            ->add_option(WXOR(L"Stencil"), &config->visual.glow_stencil)
            ->add_option(WXOR(L"Blur"), &config->visual.glow_blur);

          gcs::slider(glow_misc_mat_group, WXOR(L"Stencil thickness"), L"", 1, 4, GCS_SLIDER_TYPE_INT, &config->visual.glow_thickness)
            ->set_render_on(&config->visual.glow_stencil);

          gcs::slider(glow_misc_mat_group, WXOR(L"Bloom amount"), L"", 0.25, 5.0, GCS_SLIDER_TYPE_FLOAT_PRECISE, &config->visual.glow_blur_bloom_value)
            ->set_render_on(&config->visual.glow_blur);     
      }
    }

    return group_host;
  }

  gcs_component_group_host* create_hvh_antiaim(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Anti-aim"), pos, size);
    {
      gcs_component_group* g1                  = group_host->new_group(L"");

      gcs_component_group* antiaim_group_main = g1->new_category(WXOR(L"General"));
      {
        gcs::dropdown(antiaim_group_main, WXOR(L"Style"), false)
          ->add_option(WXOR(L"Legit"), &config->hvh.antiaim_legit)
          ->add_option(WXOR(L"Rage"), &config->hvh.antiaim_rage);

        gcs::add_space(antiaim_group_main);

        gcs::toggle(antiaim_group_main, WXOR(L"Enabled"), &config->hvh.antiaim)
          ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&config->hvh.antiaim_key)
          ->set_render_on(&config->hvh.antiaim_usekey);

        gcs::toggle(antiaim_group_main, WXOR(L"Use key"), &config->hvh.antiaim_usekey);
        gcs::dropdown(antiaim_group_main, WXOR(L"Key control"), false)
          ->add_option(WXOR(L"Held"), &config->hvh.antiaim_key_held)
          ->add_option(WXOR(L"Toggle"), &config->hvh.antiaim_key_toggle)
          ->add_option(WXOR(L"Double click"), &config->hvh.antiaim_key_double_click)
          ->set_render_on(&config->hvh.antiaim_usekey);

      }

      gcs_component_group* antiaim_group_conds = g1->new_category(WXOR(L"On Conditions"));
      {
        gcs::toggle(antiaim_group_conds, WXOR(L"Active when headshot threat"), &config->hvh.antiaim_headshotdanger);

        gcs::dropdown(antiaim_group_conds, WXOR(L"Active on"), false)
          ->add_option(WXOR(L"Any player"), &config->hvh.antiaim_headshotdanger_any)
          ->add_option(WXOR(L"Cheaters only"), &config->hvh.antiaim_headshotdanger_cheatersonly)
          ->set_render_on(&config->hvh.antiaim_headshotdanger);

        gcs::toggle(antiaim_group_conds, WXOR(L"Vischeck"), &config->hvh.antiaim_headshotdanger_vischeck)
          ->set_render_on(&config->hvh.antiaim_headshotdanger);
      }

      gcs_component_group* g2 = group_host->new_group(WXOR(L"Pitch"));
      {
        gcs_component_group* antiaim_group_pitch = g2->new_category(WXOR(L"Pitch"));
        {
          gcs::label(antiaim_group_pitch, WXOR(L"Legit anti-aim is enabled."))
            ->set_render_on(&config->hvh.antiaim_legit);

          gcs::dropdown(antiaim_group_pitch, WXOR(L"Enabled"), false)
            ->add_option(WXOR(L"Disabled"), &config->hvh.antiaim_pitch_disabled)
            ->add_option(WXOR(L"Automatic"), &config->hvh.antiaim_pitch_auto)
            ->add_option(WXOR(L"Manual"), &config->hvh.antiaim_pitch)
            ->set_render_on(&config->hvh.antiaim_rage);

          gcs::dropdown(antiaim_group_pitch, WXOR(L"Method"), false)
            ->add_option(WXOR(L"Static"), &config->hvh.antiaim_pitch_static)
            ->add_option(WXOR(L"Dynamic"), &config->hvh.antiaim_pitch_dynamic)
            ->set_render_on(&config->hvh.antiaim_rage);

          // static  
          gcs::dropdown(antiaim_group_pitch, WXOR(L"Fake"), false)
            ->add_option(WXOR(L"None"), &config->hvh.pitch_fake_none)
            ->add_option(WXOR(L"Up"), &config->hvh.pitch_fake_up)
            ->add_option(WXOR(L"Down"), &config->hvh.pitch_fake_down)
            ->set_render_on(&config->hvh.antiaim_pitch)
            ->set_render_on(&config->hvh.antiaim_rage)
            ->set_render_on(&config->hvh.antiaim_pitch_static);

          gcs::dropdown(antiaim_group_pitch, WXOR(L"Real"), false)
            ->add_option(WXOR(L"Up"), &config->hvh.pitch_real_up)
            ->add_option(WXOR(L"Zero"), &config->hvh.pitch_real_zero)
            ->add_option(WXOR(L"Down"), &config->hvh.pitch_real_down)
            ->set_render_on(&config->hvh.antiaim_pitch)
            ->set_render_on(&config->hvh.antiaim_pitch_static);
           
          gcs::dropdown(antiaim_group_pitch, WXOR(L"Real"), false)
            ->add_option(WXOR(L"Down"), &config->hvh.antiaim_pitch_dynamic_real_down)
            ->add_option(WXOR(L"Up"), &config->hvh.antiaim_pitch_dynamic_real_up)
            ->set_render_on(&config->hvh.antiaim_pitch)
            ->set_render_on(&config->hvh.antiaim_rage)
            ->set_render_on(&config->hvh.antiaim_pitch_dynamic);
        }
      }


      gcs_component_group* g3 = group_host->new_group(WXOR(L"Yaw"));
      {

        gcs_component_group* antiaim_group_yaw_general = g3->new_category(WXOR(L"Yaw"));
        {
          gcs::toggle(antiaim_group_yaw_general, WXOR(L"Enabled"), &config->hvh.antiaim_yaw);
          gcs::add_space(antiaim_group_yaw_general, &config->hvh.antiaim_yaw);

          gcs::toggle(antiaim_group_yaw_general, WXOR(L"Cycle key"), &config->hvh.antiaim_cycle_enabled)
            ->set_render_on(&config->hvh.antiaim_yaw)
            ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&config->hvh.antiaim_cycle_key)
            ->set_render_on(&config->hvh.antiaim_yaw)
            ->set_render_on(&config->hvh.antiaim_cycle_enabled);

          gcs::dropdown(antiaim_group_yaw_general, WXOR(L"Cycle method"), false)
            ->add_option(WXOR(L"Swap real and fake"), &config->hvh.antiaim_cycle_swap_settings)
            ->add_option(WXOR(L"Invert fake angle"), &config->hvh.antiaim_cycle_invert_fake_angle)
            ->add_option(WXOR(L"Invert real angle"), &config->hvh.antiaim_cycle_invert_real_angle)
            ->add_option(WXOR(L"Invert both angles"), &config->hvh.antiaim_cycle_invert_both_angles)
            ->set_render_on(&config->hvh.antiaim_yaw)
            ->set_render_on(&config->hvh.antiaim_cycle_enabled);
        }

        // Fake
        gcs_component_group* antiaim_group_yaw_fake = g3->new_category(WXOR(L"Fake"));
        {
          gcs::label(antiaim_group_yaw_fake, WXOR(L"Legit anti-aim is enabled."))
            ->set_render_on(&config->hvh.antiaim_legit);

          gcs::dropdown(antiaim_group_yaw_fake, WXOR(L"Fake method"), false)
            ->add_option(WXOR(L"Rotate"), &config->hvh.yaw_method_fake_rotate)
            ->add_option(WXOR(L"Rotate dynamic"), &config->hvh.yaw_method_fake_rotate_dynamic)
            ->add_option(WXOR(L"Spin"), &config->hvh.yaw_method_fake_spin)
            ->set_render_on(&config->hvh.antiaim_yaw)
            ->set_render_on(&config->hvh.antiaim_rage);

          // Spin options
          {
            gcs::toggle(antiaim_group_yaw_fake, WXOR(L"Invert direction"), &config->hvh.spin_fake_invert)
              ->set_render_on(&config->hvh.yaw_method_fake_spin)
              ->set_render_on(&config->hvh.antiaim_yaw)
              ->set_render_on(&config->hvh.antiaim_rage);

            gcs::slider(antiaim_group_yaw_fake, WXOR(L"Speed"), L"", 1, 16, GCS_SLIDER_TYPE_INT, &config->hvh.spin_fake_speed)
              ->set_render_on(&config->hvh.yaw_method_fake_spin)
              ->set_render_on(&config->hvh.antiaim_yaw)
              ->set_render_on(&config->hvh.antiaim_rage);
          }

          // Rotation options
          {
            gcs::dropdown(antiaim_group_yaw_fake, WXOR(L"Rotate target"), false)
              ->add_option(WXOR(L"Local angles"), &config->hvh.yaw_rotate_fake_target_method_view)
              ->add_option(WXOR(L"Closest player"), &config->hvh.yaw_rotate_fake_target_method_closetargets)
              ->add_option(WXOR(L"Closest scoped player"), &config->hvh.yaw_rotate_fake_target_method_snipers)
              ->set_render_on(&config->hvh.yaw_method_fake_rotate)
              ->set_render_on(&config->hvh.antiaim_yaw)
              ->set_render_on(&config->hvh.antiaim_rage);

            gcs::slider(antiaim_group_yaw_fake, WXOR(L"Fake angle"), WXOR(L"°"), -180, 180, GCS_SLIDER_TYPE_FLOAT, &config->hvh.rotate_fake_ang)
              ->set_render_on(&config->hvh.yaw_method_fake_rotate)
              ->set_render_on(&config->hvh.antiaim_yaw)
              ->set_render_on(&config->hvh.antiaim_rage);
          }

          // Rotate dynamic
          {
            gcs::dropdown(antiaim_group_yaw_fake, WXOR(L"Rotate target"), false)
              ->add_option(WXOR(L"Local angles"), &config->hvh.yaw_rotate_fake_target_method_view)
              ->add_option(WXOR(L"Closest player"), &config->hvh.yaw_rotate_fake_target_method_closetargets)
              ->add_option(WXOR(L"Closest scoped player"), &config->hvh.yaw_rotate_fake_target_method_snipers)
              ->set_render_on(&config->hvh.yaw_method_fake_rotate_dynamic)
              ->set_render_on(&config->hvh.antiaim_yaw)
              ->set_render_on(&config->hvh.antiaim_rage);

            gcs::slider(antiaim_group_yaw_fake, WXOR(L"Updaterate"), WXOR(L"ms"), 250, 5000, GCS_SLIDER_TYPE_FLOAT, &config->hvh.yaw_rotate_dynamic_update_rate)
              ->set_render_on(&config->hvh.yaw_method_fake_rotate_dynamic)
              ->set_render_on(&config->hvh.antiaim_yaw)
              ->set_render_on(&config->hvh.antiaim_rage);

            gcs::slider(antiaim_group_yaw_fake, WXOR(L"Fake angle #1"), WXOR(L"°"), -180, 180, GCS_SLIDER_TYPE_FLOAT, &config->hvh.rotate_fake_ang)
              ->set_render_on(&config->hvh.yaw_method_fake_rotate_dynamic)
              ->set_render_on(&config->hvh.antiaim_yaw)
              ->set_render_on(&config->hvh.antiaim_rage);

            gcs::slider(antiaim_group_yaw_fake, WXOR(L"Fake angle #2"), WXOR(L"°"), -180, 180, GCS_SLIDER_TYPE_FLOAT, &config->hvh.rotate_fake_ang2)
              ->set_render_on(&config->hvh.yaw_method_fake_rotate_dynamic)
              ->set_render_on(&config->hvh.antiaim_yaw)
              ->set_render_on(&config->hvh.antiaim_rage);
          }
        }

        // Real
        gcs_component_group* antiaim_group_yaw_real = g3->new_category(WXOR(L"Real"));
        {
          gcs::dropdown(antiaim_group_yaw_real, WXOR(L"Real method"), false)
            ->add_option(WXOR(L"Rotate"), &config->hvh.yaw_method_real_rotate)
            ->add_option(WXOR(L"Rotate dynamic"), &config->hvh.yaw_method_real_rotate_dynamic)
            ->add_option(WXOR(L"Spin"), &config->hvh.yaw_method_real_spin)
            ->set_render_on(&config->hvh.antiaim_yaw);

          // Spin options
          {
            gcs::toggle(antiaim_group_yaw_real, WXOR(L"Invert direction"), &config->hvh.spin_real_invert)
              ->set_render_on(&config->hvh.yaw_method_real_spin)
              ->set_render_on(&config->hvh.antiaim_yaw);

            gcs::slider(antiaim_group_yaw_real, WXOR(L"Speed"), L"", 1, 16, GCS_SLIDER_TYPE_INT, &config->hvh.spin_real_speed)
              ->set_render_on(&config->hvh.yaw_method_real_spin)
              ->set_render_on(&config->hvh.antiaim_yaw);
          }

          // Rotation options
          {
            gcs::dropdown(antiaim_group_yaw_real, WXOR(L"Rotate target"), false)
              ->add_option(WXOR(L"Local angles"), &config->hvh.yaw_rotate_real_target_method_view)
              ->add_option(WXOR(L"Closest player"), &config->hvh.yaw_rotate_real_target_method_closetargets)
              ->add_option(WXOR(L"Closest scoped player"), &config->hvh.yaw_rotate_real_target_method_snipers)
              ->set_render_on(&config->hvh.yaw_method_real_rotate)
              ->set_render_on(&config->hvh.antiaim_yaw);

            gcs::slider(antiaim_group_yaw_real, WXOR(L"Real angle"), WXOR(L"°"), -180, 180, GCS_SLIDER_TYPE_FLOAT, &config->hvh.rotate_real_ang)
              ->set_render_on(&config->hvh.yaw_method_real_rotate)
              ->set_render_on(&config->hvh.antiaim_yaw);
          }

          // Rotate dynamic
          {
            gcs::dropdown(antiaim_group_yaw_real, WXOR(L"Rotate target"), false)
              ->add_option(WXOR(L"Local angles"), &config->hvh.yaw_rotate_real_target_method_view)
              ->add_option(WXOR(L"Closest player"), &config->hvh.yaw_rotate_real_target_method_closetargets)
              ->add_option(WXOR(L"Closest scoped player"), &config->hvh.yaw_rotate_real_target_method_snipers)
              ->set_render_on(&config->hvh.yaw_method_real_rotate_dynamic)
              ->set_render_on(&config->hvh.antiaim_yaw);

            gcs::slider(antiaim_group_yaw_real, WXOR(L"Updaterate"), WXOR(L"ms"), 250, 5000, GCS_SLIDER_TYPE_FLOAT, &config->hvh.yaw_rotate_dynamic_update_rate)
              ->set_render_on(&config->hvh.yaw_method_real_rotate_dynamic)
              ->set_render_on(&config->hvh.antiaim_yaw);

            gcs::slider(antiaim_group_yaw_real, WXOR(L"Real angle #1"), WXOR(L"°"), -180, 180, GCS_SLIDER_TYPE_FLOAT, &config->hvh.rotate_real_ang)
              ->set_render_on(&config->hvh.yaw_method_real_rotate_dynamic)
              ->set_render_on(&config->hvh.antiaim_yaw);

            gcs::slider(antiaim_group_yaw_real, WXOR(L"Real angle #2"), WXOR(L"°"), -180, 180, GCS_SLIDER_TYPE_FLOAT, &config->hvh.rotate_real_ang2)
              ->set_render_on(&config->hvh.yaw_method_real_rotate_dynamic)
              ->set_render_on(&config->hvh.antiaim_yaw);
          }
        }

        gcs_component_group* antiaim_group_yaw_edge = g3->new_category(WXOR(L"Other"));
        {
          gcs::toggle(antiaim_group_yaw_edge, WXOR(L"Auto-edge"), &config->hvh.yaw_auto_edge_enabled)
            ->set_render_on(&config->hvh.antiaim_yaw);

          gcs::dropdown(antiaim_group_yaw_edge, WXOR(L"Type"), false)
            ->add_option(WXOR(L"Fake"), &config->hvh.yaw_auto_edge_fake)
            ->add_option(WXOR(L"Real"), &config->hvh.yaw_auto_edge_real)
            ->set_render_on(&config->hvh.antiaim_yaw)
            ->set_render_on(&config->hvh.yaw_auto_edge_enabled);

          gcs::dropdown(antiaim_group_yaw_edge, WXOR(L"Based on angle"), false)
            ->add_option(WXOR(L"Local angles"), &config->hvh.yaw_auto_edge_target_method_view)
            ->add_option(WXOR(L"Closest player"), &config->hvh.yaw_auto_edge_target_method_closetargets)
            ->add_option(WXOR(L"Closest scoped player"), &config->hvh.yaw_auto_edge_target_method_snipers)
            ->set_render_on(&config->hvh.yaw_auto_edge_enabled)
            ->set_render_on(&config->hvh.antiaim_yaw);

          gcs::add_space(antiaim_group_yaw_edge);
          gcs::toggle(antiaim_group_yaw_edge, WXOR(L"Anti backstab"), &config->misc.anti_backstab);
            gcs::toggle(antiaim_group_yaw_edge, WXOR(L"Ignore friends"), &config->misc.anti_backstab_ignore_friends)
              ->set_render_on(&config->misc.anti_backstab);
        }
      }

      group_host->update();
    }
    return group_host;
  }

  gcs_component_group_host* create_hvh_fakelag(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Fakelag"), pos, size);
    {

      gcs_component_group* g1 = group_host->new_group(WXOR(L""));

      gcs_component_group* fakelag_general_group = g1->new_category(WXOR(L"General"));
      {
          gcs::toggle(fakelag_general_group, WXOR(L"Fakelag"), &config->hvh.fakelag)
            ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&config->hvh.fakelag_key)
            ->set_render_on(&config->hvh.fakelag)
            ->set_render_on(&config->hvh.fakelag_usekey);

          gcs::toggle(fakelag_general_group, WXOR(L"Use key"), &config->hvh.fakelag_usekey);

          gcs::add_space(fakelag_general_group, &config->hvh.fakelag);

          gcs::dropdown(fakelag_general_group, WXOR(L"Key control"), false)
            ->add_option(WXOR(L"Held"), &config->hvh.fakelag_key_held)
            ->add_option(WXOR(L"Toggle"), &config->hvh.fakelag_key_toggle)
            ->add_option(WXOR(L"Double click"), &config->hvh.fakelag_key_double_click)
            ->set_render_on(&config->hvh.fakelag)
            ->set_render_on(&config->hvh.fakelag_usekey);

          gcs::dropdown(fakelag_general_group, WXOR(L"Method"), false)
            ->add_option(WXOR(L"Interval"), &config->hvh.fakelag_method_interval)
            ->add_option(WXOR(L"Movement"), &config->hvh.fakelag_method_movement)
            ->set_render_on(&config->hvh.fakelag);

          gcs::toggle(fakelag_general_group, WXOR(L"Adaptive"), &config->hvh.fakelag_adaptive)
            ->set_render_on(&config->hvh.fakelag);

          gcs::slider(fakelag_general_group, WXOR(L"Max choke"), WXOR(L"ticks"), 2, 22, GCS_SLIDER_TYPE_INT, &config->hvh.fakelag_maxchoke)
            ->set_render_on(&config->hvh.fakelag);
      }

      gcs_component_group* fakelag_cond_group = g1->new_category(WXOR(L"On Conditions"));
      {
        gcs::toggle(fakelag_cond_group, WXOR(L"Active when headshot threat"), &config->hvh.fakelag_headshotdanger);   
        gcs::dropdown(fakelag_cond_group, WXOR(L"Active on"), false)
          ->add_option(WXOR(L"Any player"), &config->hvh.fakelag_headshotdanger_any)
          ->add_option(WXOR(L"Cheaters only"), &config->hvh.fakelag_headshotdanger_cheatersonly)
          ->set_render_on(&config->hvh.fakelag_headshotdanger);

        gcs::toggle(fakelag_cond_group, WXOR(L"Vischeck"), &config->hvh.fakelag_headshotdanger_vischeck)
          ->set_render_on(&config->hvh.fakelag_headshotdanger);

      }

      gcs_component_group* g2 = group_host->new_group(WXOR(L"Misc"));
      {
        gcs_component_group* fakelag_misc = g2->new_category(WXOR(L"Misc"));
        {
          gcs::toggle(fakelag_misc, WXOR(L"Anti-backtrack"), &config->hvh.fakelag_anti_backtrack);
          gcs::toggle(fakelag_misc, WXOR(L"Peek fakelag"), &config->hvh.fakelag_peek);

          gcs::add_space(fakelag_misc);
          gcs::toggle(fakelag_misc, WXOR(L"Desync animations"), &config->hvh.fakelag_on_sequence_changed);

          gcs::add_space(fakelag_misc);

          gcs::dropdown(fakelag_misc, WXOR(L"Break jump"), false)
            ->add_option(WXOR(L"Disabled"), &config->hvh.break_animations_disabled)
            ->add_option(WXOR(L"Constant"), &config->hvh.break_animations_constant)
            ->add_option(WXOR(L"First jump"), &config->hvh.break_animations_on_jump);

          gcs::dropdown(fakelag_misc, WXOR(L"Fake crouch"), false)
            ->add_option(WXOR(L"Disabled"), &config->hvh.fakeduck_disabled)
            ->add_option(WXOR(L"On crouch"), &config->hvh.fakeduck_onduck)
            ->add_option(WXOR(L"On key"), &config->hvh.fakeduck_on_key)
            ->add_option(WXOR(L"Always"), &config->hvh.fakeduck_always)
               ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&config->hvh.fakeduck_key)
            ->set_render_on(&config->hvh.fakeduck_on_key);

        }
      }

      group_host->update();
    }
    return group_host;
  }

  gcs_component_group_host* create_hvh_resolver(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Resolver"), pos, size);
    {
      gcs_component_group* g1              = group_host->new_group(WXOR(L"General"));
      gcs_component_group* resolver_group  = g1->new_category(WXOR(L"General"));
      {
        gcs::toggle(resolver_group, WXOR(L"Enabled"), &config->hvh.resolver);
        gcs::dropdown(resolver_group, WXOR(L"Options"), true)
          ->add_option(WXOR(L"Show resolver status"), &config->hvh.resolver_esp_status)
          ->add_option(WXOR(L"Cheaters only"), &config->hvh.resolver_cheatersonly)
          ->add_option(WXOR(L"Headshot weapons only"), &config->hvh.resolver_headshot_weponly);
      }


      gcs_component_group* g2                    = group_host->new_group(WXOR(L"Manual yaw cycle"));
      gcs_component_group* resolver_manual_group = g2->new_category(WXOR(L"Manual yaw cycle"));

      gcs::toggle(resolver_manual_group, WXOR(L"Enabled"), &config->hvh.resolver_allow_manual_cycle);

      gcs::add_space(resolver_manual_group, &config->hvh.resolver_allow_manual_cycle);

      // Select player.
      {
        gcs::label(resolver_manual_group, WXOR(L"Select player key"))
          ->set_render_on(&config->hvh.resolver_allow_manual_cycle)
          ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&config->hvh.resolver_mc_sp_key);

        gcs::dropdown(resolver_manual_group, WXOR(L"Key control"), false)
          ->add_option(WXOR(L"Single click"), &config->hvh.resolver_mc_sp_single_click)
          ->add_option(WXOR(L"Double click"), &config->hvh.resolver_mc_sp_double_click)
          ->set_render_on(&config->hvh.resolver_allow_manual_cycle);
      }

      gcs::add_space(resolver_manual_group, &config->hvh.resolver_allow_manual_cycle);

      // Cycle up.
      {
        gcs::label(resolver_manual_group, WXOR(L"Cycle key"))
          ->set_render_on(&config->hvh.resolver_allow_manual_cycle)
          ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&config->hvh.resolver_mc_up_key);

        gcs::dropdown(resolver_manual_group, WXOR(L"Key control"), false)
          ->add_option(WXOR(L"Single click"), &config->hvh.resolver_mc_up_single_click)
          ->add_option(WXOR(L"Double click"), &config->hvh.resolver_mc_up_double_click)
          ->set_render_on(&config->hvh.resolver_allow_manual_cycle);
      }

      group_host->update();
    }

    return group_host;
  }

  gcs_component_group_host* create_misc(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Miscellaneous"), pos, size);
    {
      gcs_component_group* general_group          = group_host->new_group(WXOR(L"General"));
      gcs_component_group* movement_general_group = general_group->new_category(WXOR(L"Movement"));
      gcs_component_group* chat_general_group     = general_group->new_category(WXOR(L"Chat"));
      gcs_component_group* mm_general_group       = general_group->new_category(WXOR(L"Lobby"));

      gcs_component_group* exploits_group            = group_host->new_group(WXOR(L"Exploits"));
      gcs_component_group* exploits_crit_hack_group  = exploits_group->new_category(WXOR(L"Crits"));
      gcs_component_group* exploits_double_tap_group = exploits_group->new_category(WXOR(L"Tickbase"));
      gcs_component_group* exploits_fake_lat_group   = exploits_group->new_category(WXOR(L"Fakelat"));
      gcs_component_group* exploits_misc_group       = exploits_group->new_category(WXOR(L"Misc"));

      gcs_component_group* misc_main_group    = group_host->new_group(WXOR(L"Misc"));
      gcs_component_group* misc_general_group = misc_main_group->new_category(WXOR(L"Main"));
      gcs_component_group* misc_privacy_group = misc_main_group->new_category(WXOR(L"Privacy"));

      group_host->update();
      // General.
      {
        // Movement.
        {
          gcs::toggle(movement_general_group, WXOR(L"Bunnyhop"), &config->movement.bunnyhop);
          gcs::toggle(movement_general_group, WXOR(L"Fast stop"), &config->hvh.auto_stop);
          
          gcs::add_space(movement_general_group);
  
          gcs::toggle(movement_general_group, WXOR(L"Auto-strafe"), &config->movement.auto_strafe);
          {
            gcs::toggle(movement_general_group, WXOR(L"Prespeed"), &config->movement.auto_strafe_pre_speed)
              ->set_render_on(&config->movement.auto_strafe);
            
            gcs::slider(movement_general_group, WXOR(L"Retrack speed"), WXOR(L"%"), 1, 100, GCS_SLIDER_TYPE_FLOAT, &config->movement.auto_strafe_retrack)
             ->set_render_on(&config->movement.auto_strafe);

            gcs::dropdown(movement_general_group, WXOR(L"Strafe type"), false)
              ->add_option(WXOR(L"Normal"), &config->movement.auto_strafe_type_normal)
              ->add_option(WXOR(L"Directional (WASD)"), &config->movement.auto_strafe_type_directional)
                ->set_render_on(&config->movement.auto_strafe);
  
            gcs::dropdown(movement_general_group, WXOR(L"Avoid walls"), false)
              ->add_option(WXOR(L"Disabled"), &config->movement.auto_strafe_avoid_walls_disabled)
              ->add_option(WXOR(L"Simple"), &config->movement.auto_strafe_avoid_walls_normal)
              ->add_option(WXOR(L"Advanced"), &config->movement.auto_strafe_avoid_walls_adv)
                ->set_render_on(&config->movement.auto_strafe);
          }

          gcs::add_space(movement_general_group);

          gcs::toggle(movement_general_group, WXOR(L"Peek assist"), &config->hvh.peek_assist)
            ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&config->hvh.peek_assist_hotkey)
            ->set_render_on(&config->hvh.peek_assist);
        }

        // Chat
        {
          chat_format_manager->create_menu_settings(chat_general_group, &config->chat);
        }

        // Match making.
        {
          fast_queue->create_menu_settings(mm_general_group, &config->fq);
          gcs::add_space(mm_general_group);

          // Party networking.
          {
            gcs::toggle(mm_general_group, WXOR(L"Party networking"), &config->misc.party_network);
            gcs::dropdown(mm_general_group, WXOR(L"Flags"), true)
              ->add_option(WXOR(L"Far ESP"), &config->misc.party_network_far_esp)
              ->add_option(WXOR(L"Resolver data"), &config->misc.party_network_resolver)
              ->add_option(WXOR(L"Waypoint marker"), &config->misc.party_network_waypoint_marker)
              ->set_render_on(&config->misc.party_network);
    
            gcs::label(mm_general_group, WXOR(L"Set waypoint marker"))
              ->set_render_on(&config->misc.party_network)
              ->set_render_on(&config->misc.party_network_waypoint_marker)
              ->add_module<gcs_component_hotkey>()
                ->set_hotkey_ptr(&config->misc.place_waypoint_key);
    
            gcs::slider(mm_general_group, WXOR(L"Waypoint expire time"), WXOR(L"s"), 5, 30, GCS_SLIDER_TYPE_FLOAT, &config->misc.waypoint_timeout)
              ->set_render_on(&config->misc.party_network)
              ->set_render_on(&config->misc.party_network_waypoint_marker);
          }
        }
      }

      // Exploits
      {
        // Crit hack
        {
          gcs::toggle(exploits_crit_hack_group, WXOR(L"Enabled"), &config->misc.crithack_enabled)
          ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&config->misc.crithack_hotkey)
              ->set_render_on(&config->misc.crithack_enabled);
          {
            gcs::dropdown(exploits_crit_hack_group, WXOR(L"Preferences"), true)
              ->add_option(WXOR(L"Draw panel"), &config->misc.crithack_draw_panel)
              ->add_option(WXOR(L"Skip natural crits"), &config->misc.crithack_avoid_natural)
              ->add_option(WXOR(L"Melee auto crit"), &config->misc.crithack_melee_auto_crit)
                ->set_render_on(&config->misc.crithack_enabled);
          }
        }

        // Double tap
        {
          gcs::toggle(exploits_double_tap_group, WXOR(L"Double tap"), &config->misc.double_tap_enabled);
          {
            gcs::dropdown(exploits_double_tap_group, WXOR(L"Preferences"), true)
              ->add_option(WXOR(L"Draw panel"), &config->misc.double_tap_draw_panel)
              ->add_option(WXOR(L"Shift on key"), &config->misc.double_tap_shift_on_key)
              ->add_option(WXOR(L"On ground only"), &config->misc.double_tap_on_ground_only)
              ->add_option(WXOR(L"On aimbot only"), &config->misc.double_tap_on_aimbot_only)
              ->add_option(WXOR(L"Passive charging"), &config->misc.double_tap_passive_charging)
              ->add_option(WXOR(L"Always charge max ticks"), &config->misc.double_tap_always_charge_max_ticks)
              ->add_option(WXOR(L"Boost"), &config->misc.double_tap_boost)
                ->set_render_on(&config->misc.double_tap_enabled);
  
            gcs::label(exploits_double_tap_group, WXOR(L"Bar colour"))
              ->set_render_on(&config->misc.double_tap_enabled)
                ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.double_tap_main_colour);
  
            gcs::label(exploits_double_tap_group, WXOR(L"Bar sheen colour"))
              ->set_render_on(&config->misc.double_tap_enabled)
                ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.double_tap_sheen_colour);
  
            gcs::label(exploits_double_tap_group, WXOR(L"Charge key"))
              ->set_render_on(&config->misc.double_tap_enabled)
                ->add_module<gcs_component_hotkey>()
                  ->set_hotkey_ptr(&config->misc.double_tap_charge_hotkey);
  
            gcs::label(exploits_double_tap_group, WXOR(L"Shift key"))
              ->set_render_on(&config->misc.double_tap_enabled)
              ->set_render_on(&config->misc.double_tap_shift_on_key)
                ->add_module<gcs_component_hotkey>()
                  ->set_hotkey_ptr(&config->misc.double_tap_shift_hotkey);
  
            gcs::label(exploits_double_tap_group, WXOR(L"Boost key"))
              ->set_render_on(&config->misc.double_tap_enabled)
              ->set_render_on(&config->misc.double_tap_boost)
                ->add_module<gcs_component_hotkey>()
                  ->set_hotkey_ptr(&config->misc.double_tap_boost_key);

            gcs::slider(exploits_double_tap_group, WXOR(L"Boost ticks"), L"", 2, 22, GCS_SLIDER_TYPE_INT, &config->misc.double_tap_boost_ticks)
              ->set_render_on(&config->misc.double_tap_enabled)
              ->set_render_on(&config->misc.double_tap_boost);
  
            gcs::slider(exploits_double_tap_group, WXOR(L"Passive charge every"), WXOR(L"ms"), 100, 1000, GCS_SLIDER_TYPE_FLOAT, &config->misc.double_tap_passive_charge_every_ms)
              ->set_render_on(&config->misc.double_tap_enabled)
              ->set_render_on(&config->misc.double_tap_passive_charging);
          }
        }

        // Fake latency
        {
          gcs::toggle(exploits_fake_lat_group, WXOR(L"Enabled"), &config->misc.fake_latency_enabled)
            ->add_module<gcs_component_hotkey>()
              ->set_hotkey_ptr(&config->misc.fake_latency_key)
                ->set_render_on(&config->misc.fake_latency_enabled)
                ->set_render_on(&config->misc.fake_latency_use_key);

          gcs::toggle(exploits_fake_lat_group, WXOR(L"Use key"), &config->misc.fake_latency_use_key)
            ->set_render_on(&config->misc.fake_latency_enabled);
  
          gcs::dropdown(exploits_fake_lat_group, WXOR(L"Key control"), false)
            ->add_option(WXOR(L"Held"), &config->misc.fake_latency_key_held )
            ->add_option(WXOR(L"Toggle"), &config->misc.fake_latency_key_toggle)
            ->add_option(WXOR(L"Double click"), &config->misc.fake_latency_key_double_click)
            ->set_render_on(&config->misc.fake_latency_enabled)
            ->set_render_on(&config->misc.fake_latency_use_key);
          {
            gcs::toggle(exploits_fake_lat_group, WXOR(L"Allow: Non lagcomp weapons"), &config->misc.fake_latency_always_on)
              ->set_render_on(&config->misc.fake_latency_enabled);

            gcs::toggle(exploits_fake_lat_group, WXOR(L"Spoof scoreboard ping"), &config->misc.fake_latency_spoof_ms)
              ->set_render_on(&config->misc.fake_latency_enabled);
  
            gcs::slider(exploits_fake_lat_group, WXOR(L"Latency target"), WXOR(L"ms"), 200, 800, GCS_SLIDER_TYPE_FLOAT, &config->misc.fake_latency_target_ms)
              ->set_render_on(&config->misc.fake_latency_enabled);
          }
        }

        // Misc.
        {
          gcs::toggle(exploits_misc_group, WXOR(L"Infinite noisemaker"), &config->misc.infinite_noisemaker);
          gcs::toggle(exploits_misc_group, WXOR(L"Use noisemaker on kill"), &config->misc.infinite_noisemaker_on_kill);

          gcs::add_space(exploits_misc_group);
          gcs::toggle(exploits_misc_group, WXOR(L"Auto ready up"), &config->automation.mvm_auto_ready_up);
          gcs::toggle(exploits_misc_group, WXOR(L"Revive on key"), &config->misc.mvm_revive_use_key)
            ->add_module<gcs_component_hotkey>()
              ->set_hotkey_ptr(&config->misc.mvm_revive_key)
              ->set_render_on(&config->misc.mvm_revive_use_key);

          gcs::dropdown(exploits_misc_group, WXOR(L"MvM Revive response"), false)
            ->add_option(WXOR(L"None"), &config->misc.mvm_revive_none)
            ->add_option(WXOR(L"On medic heal"), &config->misc.mvm_revive_onmedicheal)
            ->add_option(WXOR(L"Always"), &config->misc.mvm_revive_always);
          gcs::add_space(exploits_misc_group);

          gcs::toggle(exploits_misc_group, WXOR(L"Disable backpack expander"), &config->misc.disable_backpack_expander);
          gcs::dropdown(exploits_misc_group, WXOR(L"Remove restrictions/limits"), true)
            ->add_option(WXOR(L"Equip Regions"), &config->misc.unequip_regions)
            ->add_option(WXOR(L"Taunt"), &config->misc.remove_taunt_restrictions)
            ->add_option(WXOR(L"Halloween kart"), &config->misc.remove_kart_restrictions)
            ->add_option(WXOR(L"Halloween cage"), &config->misc.remove_cage_restrictions);
        }
      }

      // Misc
      {
        // Main
        {
          gcs::toggle(misc_general_group, WXOR(L"No interpolation"), &config->misc.force_no_interpolation)
            ->set_help_text(WXOR(L"Disables all client-side interpolation affects.\nThis is entirely optional, having it on or off will not affect accuracy."));

          gcs::add_space(misc_general_group);
          gcs::toggle(misc_general_group, WXOR(L"Pure bypass"), &config->misc.pure_bypass);
          gcs::toggle(misc_general_group, WXOR(L"Hide aimbot/antiaim from demos"), &config->misc.demosystem_hide_angles);  
          gcs::toggle(misc_general_group, WXOR(L"Reveal scoreboard information"), &config->misc.reveal_scoreboard);

          gcs::add_space(misc_general_group);
          gcs::toggle(misc_general_group, WXOR(L"Censor bot names"), &config->misc.cheater_detection_censor_bot_names)
            ->set_help_text(WXOR(L"Replaces the bot's in-game name with 'Bot'."));

          gcs::toggle(misc_general_group, WXOR(L"Disable known actor alias"), &config->misc.cheater_detection_disable_known_actor_alias);

          gcs::add_space(misc_general_group);
          gcs::toggle(misc_general_group, WXOR(L"Reveal spy models"), &config->misc.reveal_spy_models);
          gcs::dropdown(misc_general_group, WXOR(L"Flags"), true)
            ->add_option(WXOR(L"Include localplayer"), &config->misc.reveal_spy_models_include_localplayer)
            ->add_option(WXOR(L"Remove cloak effect"), &config->misc.reveal_spy_models_remove_cloak)
              ->set_render_on(&config->misc.reveal_spy_models);
  
          gcs::dropdown(misc_general_group, WXOR(L"Cloak effect flags"), false)
            ->add_option(WXOR(L"Always remove"), &config->misc.reveal_spy_models_remove_clock_always)
            ->add_option(WXOR(L"Dont remove if cloak blinking"), &config->misc.reveal_spy_models_remove_clock_ignore_flicker)
              ->set_render_on(&config->misc.reveal_spy_models)
              ->set_render_on(&config->misc.reveal_spy_models_remove_cloak);

          gcs::add_space(misc_general_group, &config->misc.reveal_spy_models);
          gcs::dropdown(misc_general_group, WXOR(L"Disable functionality"), true)
            ->add_option(WXOR(L"Player gibs"), &config->misc.disable_gibs)
            ->add_option(WXOR(L"Player taunts"), &config->misc.disable_taunts)
            ->add_option(WXOR(L"Team pushback"), &config->misc.nopush);

          gcs::add_space(misc_general_group);
          gcs::toggle(misc_general_group, WXOR(L"No idle kick"), &config->misc.no_idle_kick);
          gcs::toggle(misc_general_group, WXOR(L"Auto choose class"), &config->misc.no_idle_kick_auto_pick_class)
            ->set_render_on(&config->misc.no_idle_kick);
          gcs::dropdown(misc_general_group, WXOR(L"Class"), false)
            ->add_option(WXOR(L"Random"), &config->misc.no_idle_kick_auto_pick_class_type[0])
            ->add_option(WXOR(L"Scout"), &config->misc.no_idle_kick_auto_pick_class_type[1])
            ->add_option(WXOR(L"Soldier"), &config->misc.no_idle_kick_auto_pick_class_type[3])
            ->add_option(WXOR(L"Pyro"), &config->misc.no_idle_kick_auto_pick_class_type[7])
            ->add_option(WXOR(L"Demoman"), &config->misc.no_idle_kick_auto_pick_class_type[4])
            ->add_option(WXOR(L"Heavy"), &config->misc.no_idle_kick_auto_pick_class_type[6])
            ->add_option(WXOR(L"Engineer"), &config->misc.no_idle_kick_auto_pick_class_type[9])
            ->add_option(WXOR(L"Medic"), &config->misc.no_idle_kick_auto_pick_class_type[5])
            ->add_option(WXOR(L"Sniper"), &config->misc.no_idle_kick_auto_pick_class_type[2])
            ->add_option(WXOR(L"Spy"), &config->misc.no_idle_kick_auto_pick_class_type[8])
              ->set_render_on(&config->misc.no_idle_kick_auto_pick_class)
              ->set_render_on(&config->misc.no_idle_kick);

          gcs::add_space(misc_general_group);
          gcs::toggle(misc_general_group, WXOR(L"MvM: Giant weapon sounds"), &config->misc.mvm_giant_weapon_sounds);
          gcs::dropdown(misc_general_group, WXOR(L"Block sounds"), true)
            ->add_option(WXOR(L"Footstep sounds"), &config->misc.disable_footsteps)
            ->add_option(WXOR(L"Noisemaker sounds"), &config->misc.block_noise_maker_sounds)
            ->add_option(WXOR(L"Frying pan sounds"), &config->misc.block_frying_pan_sounds);
        }

        // Privacy
        {
          privacy_mode->create_menu_settings(misc_privacy_group, &config->privacy_mode);

          gcs::add_space(misc_privacy_group, &config->privacy_mode.enabled);

          gcs::toggle(misc_privacy_group, WXOR(L"Hide player cosmetics"), &config->misc.remove_cosmetics);
          gcs::dropdown(misc_privacy_group, WXOR(L"Include cosmetics from"), true)
            ->add_option(WXOR(L"Localplayer"), &config->misc.remove_cosmetics_localplayer)
            ->add_option(WXOR(L"Friends"), &config->misc.remove_cosmetics_friends)
            ->set_render_on(&config->misc.remove_cosmetics);
        }
      }
    }

    return group_host;
  }

  gcs_component_group_host* create_misc_protection(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Compatibility"), pos, size);
    {
      

      gcs_component_group* general        = group_host->new_group(WXOR(L"Quick Settings"));
      gcs_component_group* rage_settings  = general->new_category(WXOR(L"Test"));
      {
        gcs::label(rage_settings, WXOR(L"Rage features"));
        gcs::add_space(rage_settings);

        gcs::toggle(rage_settings, WXOR(L"Bunnyhop"), &config->movement.bunnyhop);
        gcs::toggle(rage_settings, WXOR(L"Auto strafe"), &config->movement.auto_strafe);
    
        gcs::toggle(rage_settings, WXOR(L"Fakelag"), &config->hvh.fakelag);
        {

          gcs::toggle(rage_settings, WXOR(L"Anti-backtrack"), &config->hvh.fakelag_anti_backtrack);
          gcs::toggle(rage_settings, WXOR(L"Peek fakelag"), &config->hvh.fakelag_peek);
          gcs::toggle(rage_settings, WXOR(L"Desync animations"), &config->hvh.fakelag_on_sequence_changed);

          gcs::dropdown(rage_settings, WXOR(L"Break jump"), false)
            ->add_option(WXOR(L"Disabled"), &config->hvh.break_animations_disabled)
            ->add_option(WXOR(L"Constant"), &config->hvh.break_animations_constant)
            ->add_option(WXOR(L"First jump"), &config->hvh.break_animations_on_jump)
            ->set_help_text(WXOR(L"Recommended to be set to disabled when playing on community servers."));

          gcs::dropdown(rage_settings, WXOR(L"Fake crouch"), false)
            ->add_option(WXOR(L"Disabled"), &config->hvh.fakeduck_disabled)
            ->add_option(WXOR(L"On crouch"), &config->hvh.fakeduck_onduck)
            ->add_option(WXOR(L"On key"), &config->hvh.fakeduck_on_key)
            ->add_option(WXOR(L"Always"), &config->hvh.fakeduck_always)
               ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&config->hvh.fakeduck_key)
            ->set_render_on(&config->hvh.fakeduck_on_key);
        }

        gcs::add_space(rage_settings);
        gcs::label(rage_settings, WXOR(L"Unsafe for community servers."));
        gcs::add_space(rage_settings);
        {
          gcs::toggle(rage_settings, WXOR(L"Anti-aim"), &config->hvh.antiaim);
  
          gcs::toggle(rage_settings, WXOR(L"Infinite noisemaker"), &config->misc.infinite_noisemaker)
            ->set_help_text(WXOR(L"This is detectable on certain community servers."));
  
          gcs::toggle(rage_settings, WXOR(L"Crit-hack"), &config->misc.crithack_enabled)
            ->set_help_text(WXOR(L"This modifies command numbers and will either trigger a plugin to override your random seed or ban you."));

          gcs::toggle(rage_settings, WXOR(L"Double tap"), &config->misc.double_tap_enabled)
            ->set_help_text(WXOR(L"Double tap is not detectable itself, but it can trigger other detections by mistake.\nAlso some plugins reduce the value of sv_maxusrcmdprocessticks thus reducing effectiveness."));
        }
      }

      gcs_component_group* protection          = group_host->new_group(WXOR(L"Protection Settings"));
      gcs_component_group* protection_settings = protection->new_category(WXOR(L"Test2"));
      {
        gcs::add_space(protection_settings);
        gcs::label(protection_settings, WXOR(L"Community server protection."));
        gcs::add_space(protection_settings);
        {
          gcs::toggle(protection_settings, WXOR(L"Allow command number mods"), &config->acm.allow_command_number_manip)
            ->set_help_text(WXOR(L"If the server has an anti-cheat installed and you toggle this, you'll get banned instantly."));

          gcs::toggle(protection_settings, WXOR(L"Allow unclamped angles"), &config->acm.allow_unclamped_angles)
            ->set_help_text(WXOR(L"If the server has an anti-cheat installed, and your angles go out of bounds. You'll get instantly banned."));
        }
      }

      group_host->update();

    }
    return group_host;
  }

  bool setup() override {
    if(global->settings == nullptr){
      global->settings = new c_settings;

      // Set some default values for heavy, sniper etc
      {
        for(u32 i = 0; i < 9; i++){
          for(u32 j = 0; j < 3; j++){
            if(i + 1 == TF_CLASS_SNIPER)
              global->settings->aimbot[i][j].priority_hitbox[0] = true;
            else
              global->settings->aimbot[i][j].priority_hitbox[1] = true;
          }
        }
      }

      load_config();
    }

    vec3i menu_pos = render->screen_size / 2;
    gcs_component_window* window = create_window(this, WXOR(L"RijiN For Team Fortress 2 (64-bit)"), menu_pos);
    {

      assert(window != nullptr);
      window->add_hook(GCS_HOOK_SHOULD_ENABLE, [](gcs_component_base* c, void* p){
        bool key_pressed = c->input() & GCS_IN_INSERT_CLICK || c->input() & GCS_IN_F3_CLICK;

        if(key_pressed)
          global->menu_open = !global->menu_open;

        if(key_pressed || global->menu_open)
          utils::lock_mouse(global->menu_open);

        return global->menu_open;
      });

      vec3i canvas_pos;
      vec3i canvas_size;
      gcs_component_select_sidebar* sidebar = create_sidebar(window, canvas_pos, canvas_size);
      {
        sidebar->add_entry(WXOR(L"Aimbot"))
          ->add_entry(create_aimbot_main(window, canvas_pos, canvas_size))
          ->add_entry(create_aimbot_melee(window, canvas_pos, canvas_size));
        sidebar->add_entry(create_triggerbot(window, canvas_pos, canvas_size));
        sidebar->add_entry(WXOR(L"Visuals"))
          ->add_entry(create_visuals_main(window, canvas_pos, canvas_size))
          ->add_entry(create_visuals_esp(window, canvas_pos, canvas_size))
          ->add_entry(create_visuals_chams(window, canvas_pos, canvas_size))
          ->add_entry(create_visuals_glow(window, canvas_pos, canvas_size));
          //->add_entry(create_visuals_other(window, canvas_pos, canvas_size));
        sidebar->add_entry(WXOR(L"Hack vs Hack"))
          ->add_entry(create_hvh_antiaim(window, canvas_pos, canvas_size))
          ->add_entry(create_hvh_fakelag(window, canvas_pos, canvas_size))
          ->add_entry(create_hvh_resolver(window, canvas_pos, canvas_size));

        sidebar->add_entry(WXOR(L"Miscellaneous"))
          ->add_entry(create_misc(window, canvas_pos, canvas_size))
          ->add_entry(create_misc_protection(window, canvas_pos, canvas_size));

        sidebar->add_entry(WXOR(L"Automations"))
          ->add_entry(create_demo_automation(window, canvas_pos, canvas_size))
          ->add_entry(create_pyro_automation(window, canvas_pos, canvas_size))
          ->add_entry(create_medic_automation(window, canvas_pos, canvas_size))
          ->add_entry(create_misc_automation(window, canvas_pos, canvas_size));

        sidebar->add_entry(create_player_list(window, canvas_pos, canvas_size));
        sidebar->add_entry(WXOR(L"Events"))
          ->add_entry(create_notifications_list(window, canvas_pos, canvas_size))
          ->add_entry(create_cheater_detection_list(window, canvas_pos, canvas_size))
          ->add_entry(create_connections_list(window, canvas_pos, canvas_size))
          ->add_entry(create_votes_list(window, canvas_pos, canvas_size));

        add_cloud_configs_selection(sidebar);
      }
    }

    return true;
  }

  u32 get_game_hash() override{
    return HASH("TF2");
  }

  u32 get_config_version() override{
    return SETTINGS_VERSION;
  }

  void* get_config_pointer() override{
    return config;
  }

  i32 get_config_size() override{
    return sizeof(c_settings);
  }
};