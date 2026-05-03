#pragma once

#include "../link.h"

class c_global;

class gcs_menu : public gcs_base_menu{
public:
  void create_aimbot_main_layer(gcs_component_group* group, i32 type){
    assert(type >= 0 && type < AIMBOT_SETTING_COUNT);

    c_aimbot_settings* aimbot_settings = &config->aimbot[type];
    if(aimbot_settings == nullptr)
      return;

    gcs::toggle(group, WXOR(L"Enabled"), &aimbot_settings->enabled)
      ->add_module<gcs_component_hotkey>()
        ->set_hotkey_ptr(&aimbot_settings->aim_hotkey)
          ->set_render_on(&aimbot_settings->aim_mode_on_key)
          ->set_render_on(&aimbot_settings->enabled);

    gcs::toggle(group, WXOR(L"Auto shoot"), &aimbot_settings->auto_shoot)
      ->set_render_on(&aimbot_settings->aim_mode_on_key)
      ->set_render_on(&aimbot_settings->enabled);

    gcs::dropdown(group, WXOR(L"Aim mode"), false)
      ->add_option(WXOR(L"Automatic"), &aimbot_settings->aim_mode_automatic)
      ->add_option(WXOR(L"On attack"), &aimbot_settings->aim_mode_on_attack)
      ->add_option(WXOR(L"On key"), &aimbot_settings->aim_mode_on_key)
        ->set_render_on(&aimbot_settings->enabled);

    gcs::slider(group, WXOR(L"Maximum FOV"), WXOR(L"°"), 1, 180, GCS_SLIDER_TYPE_INT, &aimbot_settings->max_fov)
      ->set_render_on(&aimbot_settings->enabled);

    gcs::slider(group, WXOR(L"Smoothing"), WXOR(L"%"), 0, 99, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->smoothing)
      ->set_render_on(&aimbot_settings->enabled)
      ->set_render_on(&aimbot_settings->silent_aim_none);

    gcs::dropdown(group, WXOR(L"Silent aim"), false)
      ->add_option(WXOR(L"Disabled"), &aimbot_settings->silent_aim_none)
      ->add_option(WXOR(L"Clientside"), &aimbot_settings->silent_aim_clientside)
      ->add_option(WXOR(L"Serverside"), &aimbot_settings->silent_aim_serverside)
        ->set_render_on(&aimbot_settings->enabled);

    gcs::dropdown(group, WXOR(L"Position adjustment"), true)
      ->add_option(WXOR(L"History"), &aimbot_settings->position_adjust_history)
      ->add_option(WXOR(L"Accurate unlag"), &aimbot_settings->position_adjust_accurate_unlag)
      ->set_render_on(&aimbot_settings->enabled);
  }

  void create_aimbot_target_layer(gcs_component_group* group, i32 type){
    assert(type >= 0 && type < AIMBOT_SETTING_COUNT);

    c_aimbot_settings* aimbot_settings = &config->aimbot[type];
    if(aimbot_settings == nullptr)
      return;

    gcs::dropdown(group, WXOR(L"Aim at"), true)
      ->add_option(WXOR(L"Friendlies"), &aimbot_settings->target_friendlies)
      ->add_option(WXOR(L"Enemies"), &aimbot_settings->target_enemies)
      ->add_option(WXOR(L"Friends"), &aimbot_settings->target_friends);

    gcs::dropdown(group, WXOR(L"Ignore flags"), true)
      ->add_option(WXOR(L"Spawn protected"), &aimbot_settings->ignore_spawn_protected)
      ->add_option(WXOR(L"Players through smoke"), &aimbot_settings->ignore_players_through_smoke);

    gcs::toggle(group, WXOR(L"Next shot"), &aimbot_settings->next_shot);

    gcs::dropdown(group, WXOR(L"Algorithm"), false)
      ->add_option(WXOR(L"Closest to FOV"), &aimbot_settings->target_method_closest_to_fov)
      ->add_option(WXOR(L"Distance"), &aimbot_settings->target_method_distance);

    gcs::dropdown(group, WXOR(L"Priority hitbox"), false)
      ->add_option(WXOR(L"Closest to crosshair"), &aimbot_settings->closest_hitbox_to_crosshair)
      ->add_option(WXOR(L"Head"), &aimbot_settings->priority_hitbox[12])
      ->add_option(WXOR(L"Pelvis"), &aimbot_settings->priority_hitbox[0])
      ->add_option(WXOR(L"Upper Chest"), &aimbot_settings->priority_hitbox[10]);

    gcs::dropdown(group, WXOR(L"Hitscan"), true)
      ->add_option(WXOR(L"Head"), &aimbot_settings->hitbox[12])
      ->add_option(WXOR(L"Pelvis"), &aimbot_settings->hitbox[0])
      ->add_option(WXOR(L"Upper Chest"), &aimbot_settings->hitbox[10]);
  }

  void create_aimbot_other_layer(gcs_component_group* group, i32 type){
    assert(type >= 0 && type < AIMBOT_SETTING_COUNT);

    c_aimbot_settings* aimbot_settings = &config->aimbot[type];
    if(aimbot_settings == nullptr)
      return;

    gcs::toggle(group, WXOR(L"No spread"), &aimbot_settings->no_spread);
    {
      gcs::dropdown(group, WXOR(L"Mode"), false)
        ->add_option(WXOR(L"Rage"), &aimbot_settings->no_spread_rage)
        ->add_option(WXOR(L"Legit"), &aimbot_settings->no_spread_legit)
          ->set_render_on(&aimbot_settings->no_spread);
    }
    gcs::toggle(group, WXOR(L"No recoil"), &aimbot_settings->no_recoil);
    {
      gcs::dropdown(group, WXOR(L"Mode"), false)
        ->add_option(WXOR(L"Rage"), &aimbot_settings->no_recoil_rage)
        ->add_option(WXOR(L"Legit"), &aimbot_settings->no_recoil_legit)
        ->set_render_on(&aimbot_settings->no_recoil);

      gcs::toggle(group, WXOR(L"No visual recoil inherit"), &aimbot_settings->no_recoil_visual_inherit)
        ->set_render_on(&aimbot_settings->no_recoil)
        ->set_render_on(&aimbot_settings->no_recoil_legit)
        ->set_help_text(WXOR(L"View recoil will mimic these settings. That's what this option does."));

      gcs::toggle(group, WXOR(L"Pitch"), &aimbot_settings->no_recoil_legit_pitch_enabled)
        ->set_render_on(&aimbot_settings->no_recoil)
        ->set_render_on(&aimbot_settings->no_recoil_legit);

      gcs::slider(group, WXOR(L"Reduce by"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->no_recoil_legit_pitch)
        ->set_render_on(&aimbot_settings->no_recoil)
        ->set_render_on(&aimbot_settings->no_recoil_legit)
        ->set_render_on(&aimbot_settings->no_recoil_legit_pitch_enabled);

      gcs::toggle(group, WXOR(L"Yaw"), &aimbot_settings->no_recoil_legit_yaw_enabled)
        ->set_render_on(&aimbot_settings->no_recoil)
        ->set_render_on(&aimbot_settings->no_recoil_legit);

      gcs::slider(group, WXOR(L"Reduce by"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->no_recoil_legit_yaw)
        ->set_render_on(&aimbot_settings->no_recoil)
        ->set_render_on(&aimbot_settings->no_recoil_legit)
        ->set_render_on(&aimbot_settings->no_recoil_legit_yaw_enabled);
    }

    gcs::toggle(group, WXOR(L"Auto wall"), &aimbot_settings->autowall_enabled);
    gcs::slider(group, WXOR(L"Minimum damage"), WXOR(L"%"), 1, 90, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->autowall_min_dmg)
      ->set_render_on(&aimbot_settings->autowall_enabled);
  }

  void create_triggerbot_layer(gcs_component_group* group, i32 type){
    assert(type >= 0 && type < AIMBOT_SETTING_COUNT);

    c_triggerbot_settings* triggerbot_settings = &config->triggerbot[type];
    if(triggerbot_settings == nullptr)
      return;

    gcs::toggle(group, WXOR(L"Enabled"), &triggerbot_settings->enabled)
      ->add_module<gcs_component_hotkey>()
      ->set_hotkey_ptr(&triggerbot_settings->trigger_hotkey)
      ->set_render_on(&triggerbot_settings->trigger_mode_on_key)
      ->set_render_on(&triggerbot_settings->enabled);

    gcs::toggle(group, WXOR(L"On key"), &triggerbot_settings->trigger_mode_on_key)
      ->set_render_on(&triggerbot_settings->enabled);

    gcs::dropdown(group, WXOR(L"Ignore flags"), true)
      ->add_option(WXOR(L"Spawn protected"), &triggerbot_settings->ignore_spawn_protected)
      ->add_option(WXOR(L"Players through smoke"), &triggerbot_settings->ignore_players_through_smoke)
        ->set_render_on(&triggerbot_settings->enabled);

    gcs::dropdown(group, WXOR(L"Shoot at"), true)
      ->add_option(WXOR(L"Friendlies"), &triggerbot_settings->target_friendlies)
      ->add_option(WXOR(L"Enemies"), &triggerbot_settings->target_enemies)
      ->add_option(WXOR(L"Friends"), &triggerbot_settings->target_friends)
        ->set_render_on(&triggerbot_settings->enabled);

    gcs::dropdown(group, WXOR(L"Hitbox"), false)
      ->add_option(WXOR(L"Head only"), &triggerbot_settings->target_head)
      ->add_option(WXOR(L"Body only"), &triggerbot_settings->target_body)
      ->add_option(WXOR(L"Head and Body"), &triggerbot_settings->target_head_and_body)
      ->add_option(WXOR(L"Everything"), &triggerbot_settings->target_everything)
        ->set_render_on(&triggerbot_settings->enabled)
        ->set_help_text(WXOR(L"* Everything option will make the triggerbot shoot inaccurate hitboxes like arms and legs."));

    gcs::dropdown(group, WXOR(L"Nospread"), false)
      ->add_option(WXOR(L"Use aimbot settings"), &triggerbot_settings->nospread_use_aimbot_settings)
      ->add_option(WXOR(L"Disable"), &triggerbot_settings->nospread_off)
      ->add_option(WXOR(L"Enable"), &triggerbot_settings->nospread_rage_mode)
      ->add_option(WXOR(L"Spread trigger mode"), &triggerbot_settings->nospread_legit_mode)
        ->set_render_on(&triggerbot_settings->enabled);

    gcs::dropdown(group, WXOR(L"Spread trigger type"), false)
      ->add_option(WXOR(L"Rage"), &triggerbot_settings->spread_mode_rage)
      ->add_option(WXOR(L"Legit"), &triggerbot_settings->spread_mode_legit)
        ->set_render_on(&triggerbot_settings->enabled)
        ->set_render_on(&triggerbot_settings->nospread_legit_mode)
        ->set_help_text(WXOR(L"Rage: Will scan for a command number that lands on the selected hitbox.\nLegit: Will not force any command numbers."));

    gcs::slider(group, WXOR(L"Fire delay"), WXOR(L"ms"), 0, 1000, GCS_SLIDER_TYPE_FLOAT, &triggerbot_settings->trigger_fire_delay)
      ->set_render_on(&triggerbot_settings->enabled);

    gcs::slider(group, WXOR(L"Fire for"), WXOR(L"ms"), 0, 1000, GCS_SLIDER_TYPE_FLOAT, &triggerbot_settings->trigger_fire_for_time)
      ->set_render_on(&triggerbot_settings->enabled);
  }

  gcs_component_group_host* create_aimbot(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Aimbot"), pos, size);
    {
      gcs_component_group* aimbot_group        = group_host->new_group(WXOR(L"Aimbot"));
      gcs_component_group* aimbot_rifle_group  = aimbot_group->new_category(WXOR(L"Rifle"));
      gcs_component_group* aimbot_sniper_group = aimbot_group->new_category(WXOR(L"Sniper"));
      gcs_component_group* aimbot_smg_group    = aimbot_group->new_category(WXOR(L"SMG"));
      gcs_component_group* aimbot_pistol_group = aimbot_group->new_category(WXOR(L"Pistol"));

      gcs_component_group* target_group = group_host->new_group(WXOR(L"Targets"));
      gcs_component_group* target_rifle_group  = target_group->new_category(WXOR(L"Rifle"));
      gcs_component_group* target_sniper_group = target_group->new_category(WXOR(L"Sniper"));
      gcs_component_group* target_smg_group    = target_group->new_category(WXOR(L"SMG"));
      gcs_component_group* target_pistol_group = target_group->new_category(WXOR(L"Pistol"));

      gcs_component_group* other_group  = group_host->new_group(WXOR(L"Other"));
      gcs_component_group* other_rifle_group  = other_group->new_category(WXOR(L"Rifle"));
      gcs_component_group* other_sniper_group = other_group->new_category(WXOR(L"Sniper"));
      gcs_component_group* other_smg_group    = other_group->new_category(WXOR(L"SMG"));
      gcs_component_group* other_pistol_group = other_group->new_category(WXOR(L"Pistol"));

      gcs_component_group* triggerbot_group  = group_host->new_group(WXOR(L"Triggerbot"));
      gcs_component_group* triggerbot_rifle_group  = triggerbot_group->new_category(WXOR(L"Rifle"));
      gcs_component_group* triggerbot_sniper_group = triggerbot_group->new_category(WXOR(L"Sniper"));
      gcs_component_group* triggerbot_smg_group    = triggerbot_group->new_category(WXOR(L"SMG"));
      gcs_component_group* triggerbot_pistol_group = triggerbot_group->new_category(WXOR(L"Pistol"));

      group_host->update();

      create_aimbot_main_layer(aimbot_rifle_group, AIMBOT_SETTING_RIFLE);
      create_aimbot_main_layer(aimbot_sniper_group, AIMBOT_SETTING_SNIPER);
      create_aimbot_main_layer(aimbot_smg_group, AIMBOT_SETTING_SMG);
      create_aimbot_main_layer(aimbot_pistol_group, AIMBOT_SETTING_PISTOL);

      create_aimbot_target_layer(target_rifle_group, AIMBOT_SETTING_RIFLE);
      create_aimbot_target_layer(target_sniper_group, AIMBOT_SETTING_SNIPER);
      create_aimbot_target_layer(target_smg_group, AIMBOT_SETTING_SMG);
      create_aimbot_target_layer(target_pistol_group, AIMBOT_SETTING_PISTOL);

      create_aimbot_other_layer(other_rifle_group, AIMBOT_SETTING_RIFLE);
      create_aimbot_other_layer(other_sniper_group, AIMBOT_SETTING_SNIPER);
      create_aimbot_other_layer(other_smg_group, AIMBOT_SETTING_SMG);
      create_aimbot_other_layer(other_pistol_group, AIMBOT_SETTING_PISTOL);

      create_triggerbot_layer(triggerbot_rifle_group, AIMBOT_SETTING_RIFLE);
      create_triggerbot_layer(triggerbot_sniper_group, AIMBOT_SETTING_SNIPER);
      create_triggerbot_layer(triggerbot_smg_group, AIMBOT_SETTING_SMG);
      create_triggerbot_layer(triggerbot_pistol_group, AIMBOT_SETTING_PISTOL);
    }

    return group_host;
  }

  gcs_component_group_host* create_player_esp_tabs(gcs_component_base* comp, i32 config_idx){
    assert(comp != nullptr);
    assert(config_idx >= 0 && config_idx <= 2);

    {
      c_player_esp_settings* esp_settings = utils::get_player_esp_settings(config_idx);
      gcs::toggle(comp, WXOR(L"Enabled"), &esp_settings->enabled);

      if(config_idx == PLAYER_ESP_SETTING_TYPE_FRIEND){
        gcs::toggle(comp, WXOR(L"Render friend text"), &esp_settings->friend_tag)
          ->set_render_on(&esp_settings->enabled);
      }

      gcs::toggle(comp, WXOR(L"Render spawn protected text"), &esp_settings->spawn_protected_tag)
        ->set_render_on(&esp_settings->enabled);

      gcs::dropdown(comp, WXOR(L"Components"), true)
        ->add_option(WXOR(L"Player name"), &esp_settings->name)
        ->add_option(WXOR(L"Weapon name"), &esp_settings->weapon_name)
        ->add_option(WXOR(L"Distance"), &esp_settings->distance)
        ->set_render_on(&esp_settings->enabled);

      gcs::dropdown(comp, WXOR(L"Box"), false)
        ->add_option(WXOR(L"Disabled"), &esp_settings->no_box)
        ->add_option(WXOR(L"Box"), &esp_settings->box)
        ->add_option(WXOR(L"Box + Outlines"), &esp_settings->box_outlines)
        ->set_render_on(&esp_settings->enabled);

      gcs::dropdown(comp, WXOR(L"Flags"), true)
        ->add_option(WXOR(L"Enabled"), &esp_settings->flags)
        ->add_option(WXOR(L"Has defuser"), &esp_settings->flag_has_defuser)
        ->add_option(WXOR(L"Is defusing"), &esp_settings->flag_is_defusing)
        ->add_option(WXOR(L"Has Night Vision"), &esp_settings->flag_has_night_vision)
        ->add_option(WXOR(L"Is using night vision"), &esp_settings->flag_is_using_night_vision)
        ->add_option(WXOR(L"Armor status"), &esp_settings->flag_has_helmet)
          ->set_render_on(&esp_settings->enabled);

      gcs::toggle(comp, WXOR(L"Health bar"), &esp_settings->health_bar)
        ->set_render_on(&esp_settings->enabled);

      gcs::slider(comp, WXOR(L"Health fractions"), 0, 10, GCS_SLIDER_TYPE_INT, &esp_settings->health_bar_fractions)
        ->set_render_on(&esp_settings->health_bar)
        ->set_render_on(&esp_settings->enabled);

      gcs::toggle(comp, WXOR(L"Armor bar"), &esp_settings->armor_bar)
        ->set_render_on(&esp_settings->enabled);

      gcs::slider(comp, WXOR(L"Armor fractions"), 0, 10, GCS_SLIDER_TYPE_INT, &esp_settings->armor_bar_fractions)
        ->set_render_on(&esp_settings->armor_bar)
        ->set_render_on(&esp_settings->enabled);

      gcs::dropdown(comp, WXOR(L"Dormancy"), true)
        ->add_option(WXOR(L"Show"), &esp_settings->dormant_enabled)
        ->add_option(WXOR(L"Render dormant text"), &esp_settings->dormant_render_text)
        ->set_render_on(&esp_settings->enabled);
    }

    return nullptr;
  }

  gcs_component_group_host* create_player_chams_tabs(gcs_component_base* comp, i32 config_idx){
    assert(comp != nullptr);
    assert(config_idx >= 0 && config_idx <= 2);

    {
      c_player_chams_settings* chams_settings = utils::get_player_chams_settings(config_idx);
      gcs::toggle(comp, WXOR(L"Enabled"), &chams_settings->enabled);

      gcs::toggle(comp, WXOR(L"Ignore z"), &chams_settings->ignore_z)
        ->set_render_on(&chams_settings->enabled);

      gcs::dropdown(comp, WXOR(L"Base material"), false)
      ->add_option(WXOR(L"None"), &chams_settings->base_material_none)
      ->add_option(WXOR(L"Shaded"), &chams_settings->base_material_shaded)
      ->add_option(WXOR(L"Flat"), &chams_settings->base_material_flat)
      ->set_render_on(&chams_settings->enabled);

      gcs::slider(comp, WXOR(L"Shaded alpha"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &chams_settings->base_alpha)
        ->set_render_on(&chams_settings->enabled)
        ->set_render_on(&chams_settings->base_material_shaded);

      gcs::slider(comp, WXOR(L"Flat alpha"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &chams_settings->base_alpha)
        ->set_render_on(&chams_settings->enabled)
        ->set_render_on(&chams_settings->base_material_flat);

      gcs::dropdown(comp, WXOR(L"Overlay material"), false)
        ->add_option(WXOR(L"None"), &chams_settings->overlay_material_none)
        ->add_option(WXOR(L"Flat"), &chams_settings->overlay_material_flat)
        ->add_option(WXOR(L"Fresnel"), &chams_settings->overlay_material_fresnel)
        ->set_render_on(&chams_settings->enabled);

      gcs::slider(comp, WXOR(L"Flat alpha"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &chams_settings->overlay_alpha)
        ->set_render_on(&chams_settings->enabled)
        ->set_render_on(&chams_settings->overlay_material_flat);

      gcs::slider(comp, WXOR(L"Fresnel alpha"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &chams_settings->overlay_alpha)
         ->set_render_on(&chams_settings->enabled)
         ->set_render_on(&chams_settings->overlay_material_fresnel);
    }

    return nullptr;
  }

  gcs_component_group_host* create_visuals_main(gcs_component_base* comp, vec3i pos, vec3i size){
   assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"ESP/Main"), pos, size);
    {
      assert(group_host != nullptr);

      gcs_component_group* player_esp_group     = group_host->new_group(WXOR(L"Player esp"));
      gcs_component_group* player_team_group    = player_esp_group->new_category(WXOR(L"Team"));
      gcs_component_group* player_enemy_group   = player_esp_group->new_category(WXOR(L"Enemy"));
      gcs_component_group* player_friend_group  = player_esp_group->new_category(WXOR(L"Friend"));
      gcs_component_group* player_colours_group = player_esp_group->new_category(WXOR(L"Colours"));

      gcs_component_group* world_esp_group      = group_host->new_group(WXOR(L"World esp"));
      gcs_component_group* world_entities_group = world_esp_group->new_category(WXOR(L"Entities"));
      gcs_component_group* world_colours_group  = world_esp_group->new_category(WXOR(L"Colours"));

      // Player ESP
      {
        create_player_esp_tabs(player_enemy_group, PLAYER_ESP_SETTING_TYPE_ALLIED_TEAM);
        create_player_esp_tabs(player_team_group, PLAYER_ESP_SETTING_TYPE_AXIS_TEAM);
        create_player_esp_tabs(player_friend_group, PLAYER_ESP_SETTING_TYPE_FRIEND);
        // Colours
        {

          gcs::dropdown(player_colours_group, WXOR(L"Player scheme"), false)
            ->add_option(WXOR(L"Team based"), &config->visual.player_esp_use_team_colours)
            ->add_option(WXOR(L"Enemy / friendly"), &config->visual.player_esp_use_enemyteam_colours);

          // Team based
          {
            gcs::label(player_colours_group, WXOR(L"T team"))
              ->set_render_on(&config->visual.player_esp_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_esp_allied_team_colour);

            gcs::label(player_colours_group, WXOR(L"CT team"))
              ->set_render_on(&config->visual.player_esp_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_esp_axis_team_colour);
          }

          // Enemy / Friendly based.
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

          // Aimbot colour
          gcs::toggle(player_colours_group, WXOR(L"Aimbot target"), &config->visual.player_esp_aimbot_target)
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.player_esp_aimbot_target_colour);

          gcs::label(player_colours_group, WXOR(L"Spawn protected text"))
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.player_spawn_protected_text_colour);

          gcs::dropdown(player_colours_group, WXOR(L"Friend scheme"), false)
            ->add_option(WXOR(L"Single colour"), &config->visual.player_esp_friends_use_single_colour)
            ->add_option(WXOR(L"Use player scheme"), &config->visual.player_esp_friends_use_mult_colour);

          gcs::label(player_colours_group, WXOR(L"Friends"))
            ->set_render_on(&config->visual.player_esp_friends_use_single_colour)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.player_esp_friend_colour);

          // Team based.
          {
             gcs::label(player_colours_group, WXOR(L"T team"))
               ->set_render_on(&config->visual.player_esp_friends_use_mult_colour)
               ->set_render_on(&config->visual.player_esp_use_team_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.player_esp_friend_allied_team_colour);

             gcs::label(player_colours_group, WXOR(L"CT team"))
               ->set_render_on(&config->visual.player_esp_friends_use_mult_colour)
               ->set_render_on(&config->visual.player_esp_use_team_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.player_esp_friend_axis_team_colour);
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

      // World ESP
      {
        gcs::toggle(world_entities_group, WXOR(L"Enabled"), &config->world_esp.enabled);

        // Options
        {
          gcs::dropdown(world_entities_group, WXOR(L"Show"), true)
            ->add_option(WXOR(L"Enemies"), &config->world_esp.render_enemies)
            ->add_option(WXOR(L"Team"), &config->world_esp.render_friendies)
              ->set_render_on(&config->world_esp.enabled);

          for(u32 i = 0; i <= 2; i++){
            std::wstring str = (i == 0) ? WXOR(L"Grenade") : WXOR(L"Dropped weapons");
            if(i == 2)
              str = WXOR(L"Objective");

            gcs::toggle(world_entities_group, str, &config->world_esp.object_enabled[i])
              ->set_render_on(&config->world_esp.enabled);

            gcs::dropdown(world_entities_group, WXOR(L"Components"), true)
              ->add_option(WXOR(L"Box"), &config->world_esp.box[i])
              ->add_option(WXOR(L"Box outline"), &config->world_esp.box_outlines[i])
              ->add_option(WXOR(L"Name"), &config->world_esp.name[i])
              ->add_option(WXOR(L"Distance"), &config->world_esp.distance[i])
                ->set_render_on(&config->world_esp.enabled)
                ->set_render_on(&config->world_esp.object_enabled[i]);
          }
        }

        // Colours
        {
          gcs::dropdown(world_colours_group, WXOR(L"Scheme"), false)
            ->add_option(WXOR(L"Team based"), &config->visual.world_esp_use_team_colours)
            ->add_option(WXOR(L"Enemy / friendly"), &config->visual.world_esp_use_enemyteam_colours);

          // Team based
          {
            gcs::label(world_colours_group, WXOR(L"T team"))
              ->set_render_on(&config->visual.world_esp_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_esp_allied_team_colour);

            gcs::label(world_colours_group, WXOR(L"CT team"))
              ->set_render_on(&config->visual.world_esp_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_esp_axis_team_colour);
          }

          // Enemy / Friendly based.
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

          gcs::label(world_colours_group, WXOR(L"Unassigned"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_esp_unassigned_colour);

           // Aimbot colour
          gcs::toggle(world_colours_group, WXOR(L"Aimbot target"), &config->visual.world_esp_aimbot_target)
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.world_esp_aimbot_target_colour);
        }
      }

      gcs_component_group* other_group = group_host->new_group(WXOR(L"Other"));
      {
          gcs::label(other_group, WXOR(L"Menu Foreground"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.menu_foreground_colour);

          gcs::label(other_group, WXOR(L"Menu Background"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.menu_background_colour);

          gcs::toggle(other_group, WXOR(L"Backtrack chams"), &config->visual.backtrack_chams)
          ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.backtrack_chams_colour)
              ->enable_alpha_bar()
                ->set_render_on(&config->visual.backtrack_chams);

          gcs::toggle(other_group, WXOR(L"No visual recoil"), &config->visual.no_visual_recoil);
          gcs::toggle(other_group, WXOR(L"No flash"), &config->visual.no_flash);
          gcs::toggle(other_group, WXOR(L"Show information panel"), &config->visual.show_information);

          thirdperson->create_menu_settings(other_group, &config->tp);
          gcs::add_space(other_group);
          freecam->create_menu_settings(other_group, &config->freecam);
      }
      group_host->update();
    }

    return group_host;
  }

  gcs_component_group_host* create_visuals_models(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Chams/Glow"), pos, size);
    {
      gcs_component_group* player_chams_group         = group_host->new_group(WXOR(L"Player chams"));
      gcs_component_group* player_chams_team_group    = player_chams_group->new_category(WXOR(L"Team"));
      gcs_component_group* player_chams_enemy_group   = player_chams_group->new_category(WXOR(L"Enemy"));
      gcs_component_group* player_chams_friend_group  = player_chams_group->new_category(WXOR(L"Friend"));
      gcs_component_group* player_chams_colours_group = player_chams_group->new_category(WXOR(L"Colours"));

      gcs_component_group* world_chams_group          = group_host->new_group(WXOR(L"World chams"));
      gcs_component_group* world_chams_entities_group = world_chams_group->new_category(WXOR(L"Entities"));
      gcs_component_group* world_chams_settings_group = world_chams_group->new_category(WXOR(L"Settings"));
      gcs_component_group* world_chams_colours_group  = world_chams_group->new_category(WXOR(L"Colours"));

      gcs_component_group* glow_group          = group_host->new_group(WXOR(L"Glow"));
      gcs_component_group* entities_glow_group = glow_group->new_category(WXOR(L"Entities"));
      gcs_component_group* settings_glow_group = glow_group->new_category(WXOR(L"Settings"));
      gcs_component_group* colours_glow_group  = glow_group->new_category(WXOR(L"Colours"));

      group_host->update();
      // Player chams
      {

        create_player_chams_tabs(player_chams_enemy_group, PLAYER_ESP_SETTING_TYPE_ALLIED_TEAM);
        create_player_chams_tabs(player_chams_team_group, PLAYER_ESP_SETTING_TYPE_AXIS_TEAM);
        create_player_chams_tabs(player_chams_friend_group, PLAYER_ESP_SETTING_TYPE_FRIEND);

        // Colours
        {
          gcs::dropdown(player_chams_colours_group, WXOR(L"Scheme"), false)
             ->add_option(WXOR(L"Team based"), &config->visual.player_chams_use_team_colours)
             ->add_option(WXOR(L"Enemy / friendly"), &config->visual.player_chams_use_enemyteam_colours);

           // Team based.
           {
                gcs::label(player_chams_colours_group, WXOR(L"T team"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_allied_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"CT team"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_axis_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay T team"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_allied_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay CT team"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_axis_team_colour);
           }

           // Enemy / friendly based.
           {
                gcs::label(player_chams_colours_group, WXOR(L"Enemy"))
                  ->set_render_on(&config->visual.player_chams_use_enemyteam_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_enemy_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Team"))
                  ->set_render_on(&config->visual.player_chams_use_enemyteam_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay Enemy"))
                  ->set_render_on(&config->visual.player_chams_use_enemyteam_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_enemy_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay Team"))
                  ->set_render_on(&config->visual.player_chams_use_enemyteam_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_team_colour);
           }

           // Aimbot colour
           gcs::toggle(player_chams_colours_group, WXOR(L"Aimbot target"), &config->visual.player_chams_aimbot_target)
             ->add_module<gcs_component_colour_picker>()
             ->set_output_colour_ptr(config->visual.player_chams_aimbot_target_colour);

           gcs::dropdown(player_chams_colours_group, WXOR(L"Friend scheme"), false)
               ->add_option(WXOR(L"Single colour"), &config->visual.player_chams_friends_use_single_colour)
               ->add_option(WXOR(L"Use player scheme"), &config->visual.player_chams_friends_use_mult_colour);

           gcs::label(player_chams_colours_group, WXOR(L"Friends"))
               ->set_render_on(&config->visual.player_chams_friends_use_single_colour)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.player_chams_friend_colour);

           gcs::label(player_chams_colours_group, WXOR(L"Overlay Friends"))
               ->set_render_on(&config->visual.player_chams_friends_use_single_colour)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_colour);

           // Team based.
           {
                gcs::label(player_chams_colours_group, WXOR(L"T team"))
                  ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_friend_allied_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"CT team"))
                  ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_friend_axis_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay T team"))
                  ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_allied_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay CT team"))
                  ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_axis_team_colour);
           }

           // Enemy / friendly based.
           {
                gcs::label(player_chams_colours_group, WXOR(L"Enemy"))
                  ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                  ->set_render_on(&config->visual.player_chams_use_enemyteam_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_friend_enemy_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Team"))
                  ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                  ->set_render_on(&config->visual.player_chams_use_enemyteam_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_friend_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay Enemy"))
                  ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                  ->set_render_on(&config->visual.player_chams_use_enemyteam_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_enemy_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay Team"))
                  ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                  ->set_render_on(&config->visual.player_chams_use_enemyteam_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_team_colour);
           }
        }
      }

      // World chams
      {
        // Entities
        {
          gcs::toggle(world_chams_entities_group, WXOR(L"Enabled"), &config->world_chams.enabled);
          gcs::dropdown(world_chams_entities_group, WXOR(L"Show"), true)
            ->add_option(WXOR(L"Enemies"), &config->world_chams.render_enemies)
            ->add_option(WXOR(L"Team"), &config->world_chams.render_friendies)
            ->set_render_on(&config->world_chams.enabled);

          gcs::toggle(world_chams_entities_group, WXOR(L"Grenade"), &config->world_chams.object_enabled[0])
            ->set_render_on(&config->world_chams.enabled);

          gcs::toggle(world_chams_entities_group, WXOR(L"Dropped weapons"), &config->world_chams.object_enabled[1])
            ->set_render_on(&config->world_chams.enabled);

          gcs::toggle(world_chams_entities_group, WXOR(L"Objective"), &config->world_chams.object_enabled[2])
            ->set_render_on(&config->world_chams.enabled);
        }

        // Settings
        {
          gcs::toggle(world_chams_settings_group, WXOR(L"Ignore z"), &config->world_chams.ignore_z);

          gcs::dropdown(world_chams_settings_group, WXOR(L"Base material"), false)
            ->add_option(WXOR(L"None"), &config->world_chams.base_material_none)
            ->add_option(WXOR(L"Shaded"), &config->world_chams.base_material_shaded)
            ->add_option(WXOR(L"Flat"), &config->world_chams.base_material_flat);

          gcs::slider(world_chams_settings_group, WXOR(L"Shaded alpha"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &config->world_chams.base_alpha)
            ->set_render_on(&config->world_chams.base_material_shaded);

          gcs::slider(world_chams_settings_group, WXOR(L"Flat alpha"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &config->world_chams.base_alpha)
            ->set_render_on(&config->world_chams.base_material_flat);

          gcs::dropdown(world_chams_settings_group, WXOR(L"Overlay material"), false)
            ->add_option(WXOR(L"None"), &config->world_chams.overlay_material_none)
            ->add_option(WXOR(L"Flat"), &config->world_chams.overlay_material_flat)
            ->add_option(WXOR(L"Fresnel"), &config->world_chams.overlay_material_fresnel);

          gcs::slider(world_chams_settings_group, WXOR(L"Flat alpha"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &config->world_chams.overlay_alpha)
            ->set_render_on(&config->world_chams.overlay_material_flat);

          gcs::slider(world_chams_settings_group, WXOR(L"Fresnel alpha"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &config->world_chams.overlay_alpha)
             ->set_render_on(&config->world_chams.overlay_material_fresnel);
        }

        // Colours
        {
          gcs::dropdown(world_chams_colours_group, WXOR(L"Scheme"), false)
            ->add_option(WXOR(L"Team based"), &config->visual.world_chams_use_team_colours)
            ->add_option(WXOR(L"Enemy / friendly"), &config->visual.world_chams_use_enemyteam_colours);

          // Team based
          {
            gcs::label(world_chams_colours_group, WXOR(L"T team"))
              ->set_render_on(&config->visual.world_chams_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_chams_allied_team_colour);

            gcs::label(world_chams_colours_group, WXOR(L"CT team"))
              ->set_render_on(&config->visual.world_chams_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_chams_axis_team_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Overlay T team"))
              ->set_render_on(&config->visual.world_chams_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_allied_team_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Overlay CT team"))
              ->set_render_on(&config->visual.world_chams_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_axis_team_colour);
          }

          // Enemy / Friendly based.
          {
            gcs::label(world_chams_colours_group, WXOR(L"Enemy"))
              ->set_render_on(&config->visual.world_chams_use_enemyteam_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_chams_enemy_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Team"))
              ->set_render_on(&config->visual.world_chams_use_enemyteam_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_chams_team_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Overlay enemy"))
              ->set_render_on(&config->visual.world_chams_use_enemyteam_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_enemy_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Overlay team"))
              ->set_render_on(&config->visual.world_chams_use_enemyteam_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_team_colour);
          }

          gcs::label(world_chams_colours_group, WXOR(L"Unassigned"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_chams_unassigned_colour);

          gcs::label(world_chams_colours_group, WXOR(L"Overlay unassigned"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_chams_overlay_unassigned_colour);

          // Aimbot colour
          gcs::toggle(world_chams_colours_group, WXOR(L"Aimbot target"), &config->visual.world_chams_aimbot_target)
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.world_chams_aimbot_target_colour);
        }
      }

      // Glow
      {
        gcs::toggle(entities_glow_group, WXOR(L"Enabled"), &config->visual.glow_enabled);

        // Players
        gcs::toggle(entities_glow_group, WXOR(L"Players"), &config->visual.player_glow_enabled)
           ->set_render_on(&config->visual.glow_enabled);
        {
          gcs::dropdown(entities_glow_group, WXOR(L"Show"), true)
            ->add_option(WXOR(L"Enemies"), &config->visual.player_glow_render_enemies)
            ->add_option(WXOR(L"Team"), &config->visual.player_glow_render_friendlies)
            ->add_option(WXOR(L"Friends"), &config->visual.player_glow_render_steamfriends)
            ->set_render_on(&config->visual.glow_enabled)
            ->set_render_on(&config->visual.player_glow_enabled);
        }

        // World
        gcs::toggle(entities_glow_group, WXOR(L"World"), &config->world_glow.enabled)
           ->set_render_on(&config->visual.glow_enabled);
        {
          gcs::dropdown(entities_glow_group, WXOR(L"Show"), true)
            ->add_option(WXOR(L"Enemies"), &config->world_glow.render_enemies)
            ->add_option(WXOR(L"Team"), &config->world_glow.render_friendlies)
            ->set_render_on(&config->visual.glow_enabled)
            ->set_render_on(&config->world_glow.enabled);
        }

        gcs::toggle(entities_glow_group, WXOR(L"Grenade"), &config->world_glow.object_enabled[0])
          ->set_render_on(&config->world_glow.enabled)
          ->set_render_on(&config->visual.glow_enabled);

        gcs::toggle(entities_glow_group, WXOR(L"Dropped weapons"), &config->world_glow.object_enabled[1])
          ->set_render_on(&config->world_glow.enabled)
          ->set_render_on(&config->visual.glow_enabled);

        gcs::toggle(entities_glow_group, WXOR(L"Objective"), &config->world_glow.object_enabled[2])
          ->set_render_on(&config->world_glow.enabled)
          ->set_render_on(&config->visual.glow_enabled);


        // Settings
        {
          gcs::dropdown(settings_glow_group, WXOR(L"Styles"), true)
            ->add_option(WXOR(L"Stencil"), &config->visual.glow_stencil)
            ->add_option(WXOR(L"Blur"), &config->visual.glow_blur);

          gcs::slider(settings_glow_group, WXOR(L"Stencil thickness"), L"", 1, 4, GCS_SLIDER_TYPE_INT, &config->visual.glow_thickness)
            ->set_render_on(&config->visual.glow_stencil);
        }

        // Colours
        {
           // Player colours
          {
            gcs::dropdown(colours_glow_group, WXOR(L"Player scheme"), false)
              ->add_option(WXOR(L"Team based"), &config->visual.player_glow_use_team_colours)
              ->add_option(WXOR(L"Enemy / friendly"), &config->visual.player_glow_use_enemyteam_colours);

            // Team based.
            {
             gcs::label(colours_glow_group, WXOR(L"T team"))
               ->set_render_on(&config->visual.player_glow_use_team_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.player_glow_allied_team_colour);
             gcs::label(colours_glow_group, WXOR(L"CT team"))
               ->set_render_on(&config->visual.player_glow_use_team_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.player_glow_axis_team_colour);
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

            gcs::toggle(colours_glow_group, WXOR(L"Aimbot target"), &config->visual.player_glow_aimbot_target)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.player_glow_aimbot_target_colour);

            gcs::dropdown(colours_glow_group, WXOR(L"Friend scheme"), false)
              ->add_option(WXOR(L"Single colour"), &config->visual.player_glow_friends_use_single_colour)
              ->add_option(WXOR(L"Use player scheme"), &config->visual.player_glow_friends_use_mult_colour);
            gcs::label(colours_glow_group, WXOR(L"Friends"))
              ->set_render_on(&config->visual.player_glow_friends_use_single_colour)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.player_glow_friend_colour);

            // Team based.
            {
              gcs::label(colours_glow_group, WXOR(L"T team"))
                ->set_render_on(&config->visual.player_glow_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_glow_friend_allied_team_colour);
              gcs::label(colours_glow_group, WXOR(L"CT team"))
                ->set_render_on(&config->visual.player_glow_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_glow_friend_axis_team_colour);
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

          gcs::label(colours_glow_group, L""); // A little space to make easier for people to understand that above is for players and below is for world stuff.

          // World colours
          {
            gcs::dropdown(colours_glow_group, WXOR(L"World scheme"), false)
              ->add_option(WXOR(L"Team based"), &config->visual.world_glow_use_team_colours)
              ->add_option(WXOR(L"Enemy / friendly"), &config->visual.world_glow_use_enemyteam_colours);

            // Team based.
            {
             gcs::label(colours_glow_group, WXOR(L"T team"))
               ->set_render_on(&config->visual.world_glow_use_team_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.world_glow_allied_team_colour);
             gcs::label(colours_glow_group, WXOR(L"CT team"))
               ->set_render_on(&config->visual.world_glow_use_team_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.world_glow_axis_team_colour);
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

            gcs::label(colours_glow_group, WXOR(L"Unassigned"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_unassigned_team_colour);

            // Aimbot colour
            gcs::toggle(colours_glow_group, WXOR(L"Aimbot target"), &config->visual.world_glow_aimbot_target)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_glow_aimbot_target_colour);
          }
        }
      }
    }
    return group_host;
  }

  gcs_component_group_host* create_misc(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Miscellaneous"), pos, size);
    {
      gcs_component_group* general_group = group_host->new_group(WXOR(L"General"));
      gcs_component_group* other_group   = group_host->new_group(WXOR(L"Other"));
      group_host->update();

      // General
      {
        movement->create_menu_settings(general_group, &config->movement);
        gcs::toggle(general_group, WXOR(L"Auto pistol"), &config->misc.auto_pistol);
      }

      // Other
      {
        gcs::toggle(other_group, WXOR(L"No interpolation"), &config->misc.force_no_interpolation)
          ->set_help_text(WXOR(L"Disables all client-side interpolation affects.\nThis is entirely optional, having it on or off will not affect accuracy."));

        gcs::toggle(other_group, WXOR(L"Pure bypass"), &config->misc.pure_bypass);
        gcs::toggle(other_group, WXOR(L"Block server chat messages"), &config->misc.block_server_chat_messages);

        privacy_mode->create_menu_settings(other_group, &config->privacy_mode);

        gcs::dropdown(other_group, WXOR(L"Chat tags"), true)
          ->add_option(WXOR(L"Enabled"), &config->misc.chat_tags)
          ->add_option(WXOR(L"Localplayer"), &config->misc.chat_tags_localplayer)
          ->add_option(WXOR(L"Friends"), &config->misc.chat_tags_friends)
          ->add_option(WXOR(L"Cheaters"), &config->misc.chat_tags_cheaters);
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

  bool setup() override {
    // cloud configs
    load_config();

    vec3i menu_pos = render->screen_size / 2;
    gcs_component_window* window = create_window(this, WXOR(L"RijiN For Counter Strike: Source (x64)"), menu_pos);
    {
      assert(window != nullptr);
      window->add_hook(GCS_HOOK_SHOULD_ENABLE, [](gcs_component_base* c, void* p){
        bool key_pressed = c->input() & GCS_IN_INSERT_CLICK || c->input() & GCS_IN_F3_CLICK;

        if(key_pressed){
          global->menu_open = !global->menu_open;
          if(key_pressed || global->menu_open)
            utils::lock_mouse(global->menu_open);
        }

        return global->menu_open;
      });

      vec3i canvas_pos;
      vec3i canvas_size;
      gcs_component_select_sidebar* sidebar = create_sidebar(window, canvas_pos, canvas_size);
      {
        sidebar->add_entry(create_aimbot(window, canvas_pos, canvas_size));

        sidebar->add_entry(WXOR(L"Visuals"))
          ->add_entry(create_visuals_main(window, canvas_pos, canvas_size))
          ->add_entry(create_visuals_models(window, canvas_pos, canvas_size));

        sidebar->add_entry(create_misc(window, canvas_pos, canvas_size));
        sidebar->add_entry(create_player_list(window, canvas_pos, canvas_size));

        add_cloud_configs_selection(sidebar);
      }
    }

    return true;
  }

  u32 get_game_hash() override{
    return HASH("CSS");
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