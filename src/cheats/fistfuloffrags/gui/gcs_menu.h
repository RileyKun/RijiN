#pragma once

#include "../link.h"

class c_global;

class gcs_menu : public gcs_base_menu{
public:
  gcs_component_group_host* create_aimbot(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Aimbot"), pos, size);
    {
      gcs_component_group* aimbot_group  = group_host->new_group(WXOR(L"Aimbot"));
      gcs_component_group* target_group = group_host->new_group(WXOR(L"Targets"));
      gcs_component_group* other_group = group_host->new_group(WXOR(L"Other"));
      group_host->update();

      // Aimbot
      {
        gcs::toggle(aimbot_group, WXOR(L"Enabled"), &config->aimbot.enabled)
          ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&config->aimbot.aim_hotkey)
              ->set_render_on(&config->aimbot.aim_mode_on_key)
              ->set_render_on(&config->aimbot.enabled);

        gcs::toggle(aimbot_group, WXOR(L"Auto shoot"), &config->aimbot.auto_shoot)
          ->set_render_on(&config->aimbot.aim_mode_on_key)
          ->set_render_on(&config->aimbot.enabled);

        gcs::dropdown(aimbot_group, WXOR(L"Aim mode"), false)
          ->add_option(WXOR(L"Automatic"), &config->aimbot.aim_mode_automatic)
          ->add_option(WXOR(L"On attack"), &config->aimbot.aim_mode_on_attack)
          ->add_option(WXOR(L"On key"), &config->aimbot.aim_mode_on_key)
            ->set_render_on(&config->aimbot.enabled);

        gcs::slider(aimbot_group, WXOR(L"Maximum FOV"), WXOR(L"°"), 1, 180, GCS_SLIDER_TYPE_INT, &config->aimbot.max_fov)
          ->set_render_on(&config->aimbot.enabled);

        gcs::slider(aimbot_group, WXOR(L"Smoothing"), WXOR(L"%"), 0, 99, GCS_SLIDER_TYPE_FLOAT, &config->aimbot.smoothing)
          ->set_render_on(&config->aimbot.enabled)
          ->set_render_on(&config->aimbot.silent_aim_none);

        gcs::dropdown(aimbot_group, WXOR(L"Silent aim"), false)
          ->add_option(WXOR(L"Disabled"), &config->aimbot.silent_aim_none)
          ->add_option(WXOR(L"Clientside"), &config->aimbot.silent_aim_clientside)
          ->add_option(WXOR(L"Serverside"), &config->aimbot.silent_aim_serverside)
            ->set_render_on(&config->aimbot.enabled);

        gcs::dropdown(aimbot_group, WXOR(L"Position adjustment"), true)
          ->add_option(WXOR(L"History"), &config->aimbot.position_adjust_history)
          ->add_option(WXOR(L"Accurate unlag"), &config->aimbot.position_adjust_accurate_unlag)
          ->set_render_on(&config->aimbot.enabled);
      }

      // Targets
      {
        gcs::dropdown(target_group, WXOR(L"Aim at"), true)
          ->add_option(WXOR(L"Friendlies"), &config->aimbot.target_friendlies)
          ->add_option(WXOR(L"Enemies"), &config->aimbot.target_enemies)
          ->add_option(WXOR(L"Friends"), &config->aimbot.target_friends);

        gcs::dropdown(target_group, WXOR(L"Algorithm"), false)
          ->add_option(WXOR(L"Closest to FOV"), &config->aimbot.target_method_closest_to_fov)
          ->add_option(WXOR(L"Distance"), &config->aimbot.target_method_distance);

        gcs::dropdown(target_group, WXOR(L"Priority hitbox"), false)
          ->add_option(WXOR(L"Head"), &config->aimbot.priority_hitbox[0])
          ->add_option(WXOR(L"Pelvis"), &config->aimbot.priority_hitbox[16])
          ->add_option(WXOR(L"Upper Chest"), &config->aimbot.priority_hitbox[17]);

        gcs::dropdown(target_group, WXOR(L"Hitscan"), true)
          ->add_option(WXOR(L"Head"), &config->aimbot.hitbox[0])
          ->add_option(WXOR(L"Pelvis"), &config->aimbot.hitbox[16])
          ->add_option(WXOR(L"Upper Chest"), &config->aimbot.hitbox[17]);
      }

      // Other
      {
        gcs::toggle(other_group, WXOR(L"No spread"), &config->aimbot.no_spread);
        gcs::toggle(other_group, WXOR(L"No recoil"), &config->aimbot.no_recoil);

        //gcs::toggle(other_group, WXOR(L"Auto wall"), &config->aimbot.autowall_enabled);
        //gcs::slider(other_group, WXOR(L"Minimum damage"), WXOR(L"%"), 1, 90, GCS_SLIDER_TYPE_FLOAT, &config->aimbot.autowall_min_dmg)
        //  ->set_render_on(&config->aimbot.autowall_enabled);
      }

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

      gcs::toggle(comp, WXOR(L"Health bar"), &esp_settings->health_bar)
        ->set_render_on(&esp_settings->enabled);

      gcs::slider(comp, WXOR(L"Health fractions"), 0, 10, GCS_SLIDER_TYPE_INT, &esp_settings->health_bar_fractions)
        ->set_render_on(&esp_settings->health_bar)
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
            gcs::label(player_colours_group, WXOR(L"Vigilantes"))
              ->set_render_on(&config->visual.player_esp_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_esp_vigil_team_colour);

            gcs::label(player_colours_group, WXOR(L"Desperados"))
              ->set_render_on(&config->visual.player_esp_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_esp_desper_team_colour);

            gcs::label(player_colours_group, WXOR(L"Bandidos"))
              ->set_render_on(&config->visual.player_esp_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_esp_bandio_team_colour);

            gcs::label(player_colours_group, WXOR(L"Rangers"))
              ->set_render_on(&config->visual.player_esp_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_esp_rangers_team_colour);

            gcs::label(player_colours_group, WXOR(L"Unassigned"))
              ->set_render_on(&config->visual.player_esp_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_esp_unassigned_colour);
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

          gcs::dropdown(player_colours_group, WXOR(L"Friend scheme"), false)
            ->add_option(WXOR(L"Single colour"), &config->visual.player_esp_friends_use_single_colour)
            ->add_option(WXOR(L"Use player scheme"), &config->visual.player_esp_friends_use_mult_colour);

          gcs::label(player_colours_group, WXOR(L"Friends"))
            ->set_render_on(&config->visual.player_esp_friends_use_single_colour)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.player_esp_friend_colour);

          // Team based.
          {
             gcs::label(player_colours_group, WXOR(L"Vigilantes"))
               ->set_render_on(&config->visual.player_esp_friends_use_mult_colour)
               ->set_render_on(&config->visual.player_esp_use_team_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.player_esp_friend_vigil_team_colour);

             gcs::label(player_colours_group, WXOR(L"Desperados"))
               ->set_render_on(&config->visual.player_esp_friends_use_mult_colour)
               ->set_render_on(&config->visual.player_esp_use_team_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.player_esp_friend_desper_team_colour);

             gcs::label(player_colours_group, WXOR(L"Bandidos"))
               ->set_render_on(&config->visual.player_esp_friends_use_mult_colour)
               ->set_render_on(&config->visual.player_esp_use_team_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.player_esp_friend_bandio_team_colour);

             gcs::label(player_colours_group, WXOR(L"Rangers"))
               ->set_render_on(&config->visual.player_esp_friends_use_mult_colour)
               ->set_render_on(&config->visual.player_esp_use_team_colours)
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.player_esp_friend_rangers_team_colour);
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

          for(u32 i = 0; i <= 5; i++){
            std::wstring str = (i == 0) ? WXOR(L"Dynamite") : WXOR(L"Dropped weapons");
            if(i == 2)
              str = WXOR(L"Objective");
            else if(i == 3)
              str = WXOR(L"Whiskey");
            else if(i == 4)
              str = WXOR(L"Weapon crate");
            else if(i == 5)
              str = WXOR(L"Horse");

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
            gcs::label(world_colours_group, WXOR(L"Vigilantes"))
              ->set_render_on(&config->visual.world_esp_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_esp_vigil_team_colour);

            gcs::label(world_colours_group, WXOR(L"Desperados"))
              ->set_render_on(&config->visual.world_esp_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_esp_desper_team_colour);

            gcs::label(world_colours_group, WXOR(L"Bandidos"))
              ->set_render_on(&config->visual.world_esp_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_esp_bandio_team_colour);

            gcs::label(world_colours_group, WXOR(L"Rangers"))
              ->set_render_on(&config->visual.world_esp_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_esp_rangers_team_colour);
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

          gcs::label(world_colours_group, WXOR(L"Whiskey"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_esp_whiskey_colour);

          gcs::label(world_colours_group, WXOR(L"Dynamite"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_esp_dynamite_colour);

          gcs::label(world_colours_group, WXOR(L"Horse"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_esp_horse_colour);

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

        gcs::toggle(other_group, WXOR(L"No visual recoil"), &config->visual.no_visual_recoil);
        gcs::toggle(other_group, WXOR(L"No drunk effects"), &config->visual.no_drunk_effects);
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
                gcs::label(player_chams_colours_group, WXOR(L"Vigilantes"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_vigil_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Desperados"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_desper_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Bandidos"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_bandio_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Rangers"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_rangers_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Unassigned"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_unassigned_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay vigilantes"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_vigil_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay desperados"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_desper_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay bandidos"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_bandio_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay rangers"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_rangers_team_colour);

                gcs::label(player_chams_colours_group, WXOR(L"Overlay unassigned"))
                  ->set_render_on(&config->visual.player_chams_use_team_colours)
                  ->add_module<gcs_component_colour_picker>()
                  ->set_output_colour_ptr(config->visual.player_chams_overlay_unassigned_colour);
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
              gcs::label(player_chams_colours_group, WXOR(L"Vigilantes"))
                ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_chams_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_chams_friend_vigil_team_colour);

              gcs::label(player_chams_colours_group, WXOR(L"Desperados"))
                ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_chams_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_chams_friend_desper_team_colour);

              gcs::label(player_chams_colours_group, WXOR(L"Bandidos"))
                ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_chams_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_chams_friend_bandio_team_colour);

              gcs::label(player_chams_colours_group, WXOR(L"Rangers"))
                ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_chams_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_chams_friend_rangers_team_colour);

              gcs::label(player_chams_colours_group, WXOR(L"Overlay vigilantes"))
                ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_chams_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_vigil_team_colour);

              gcs::label(player_chams_colours_group, WXOR(L"Overlay desperados"))
                ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_chams_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_desper_team_colour);

              gcs::label(player_chams_colours_group, WXOR(L"Overlay bandidos"))
                ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_chams_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_bandio_team_colour);

              gcs::label(player_chams_colours_group, WXOR(L"Overlay rangers"))
                ->set_render_on(&config->visual.player_chams_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_chams_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_rangers_team_colour);
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

          gcs::toggle(world_chams_entities_group, WXOR(L"Dynamite"), &config->world_chams.object_enabled[0])
            ->set_render_on(&config->world_chams.enabled);

          gcs::toggle(world_chams_entities_group, WXOR(L"Dropped weapons"), &config->world_chams.object_enabled[1])
            ->set_render_on(&config->world_chams.enabled);

          gcs::toggle(world_chams_entities_group, WXOR(L"Objective"), &config->world_chams.object_enabled[2])
            ->set_render_on(&config->world_chams.enabled);

          gcs::toggle(world_chams_entities_group, WXOR(L"Whiskey"), &config->world_chams.object_enabled[3])
            ->set_render_on(&config->world_chams.enabled);

          gcs::toggle(world_chams_entities_group, WXOR(L"Weapon crate"), &config->world_chams.object_enabled[4])
            ->set_render_on(&config->world_chams.enabled);

          gcs::toggle(world_chams_entities_group, WXOR(L"Horse"), &config->world_chams.object_enabled[5])
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
            gcs::label(world_chams_colours_group, WXOR(L"Vigilantes"))
              ->set_render_on(&config->visual.world_chams_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_chams_vigil_team_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Desperados"))
              ->set_render_on(&config->visual.world_chams_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_chams_desper_team_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Bandidos"))
              ->set_render_on(&config->visual.world_chams_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_chams_bandio_team_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Rangers"))
              ->set_render_on(&config->visual.world_chams_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_chams_rangers_team_colour);


            gcs::label(world_chams_colours_group, WXOR(L"Overlay vigilantes"))
              ->set_render_on(&config->visual.world_chams_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_vigil_team_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Overlay vesperados"))
              ->set_render_on(&config->visual.world_chams_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_desper_team_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Overlay bandidos"))
              ->set_render_on(&config->visual.world_chams_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_bandio_team_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Overlay rangers"))
              ->set_render_on(&config->visual.world_chams_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_rangers_team_colour);
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

          gcs::label(world_chams_colours_group, WXOR(L"Whiskey"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_chams_whiskey_colour);

          gcs::label(world_chams_colours_group, WXOR(L"Overlay whiskey"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_chams_overlay_whiskey_colour);

          gcs::label(world_chams_colours_group, WXOR(L"Dynamite"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_chams_dynamite_colour);

          gcs::label(world_chams_colours_group, WXOR(L"Overlay Dynamite"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_chams_overlay_dynamite_colour);

          gcs::label(world_chams_colours_group, WXOR(L"Horse"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_chams_horse_colour);

          gcs::label(world_chams_colours_group, WXOR(L"Overlay Horse"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_chams_overlay_horse_colour);

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

        gcs::toggle(entities_glow_group, WXOR(L"Dynamite"), &config->world_glow.object_enabled[0])
          ->set_render_on(&config->world_glow.enabled)
          ->set_render_on(&config->visual.glow_enabled);

        gcs::toggle(entities_glow_group, WXOR(L"Dropped weapons"), &config->world_glow.object_enabled[1])
          ->set_render_on(&config->world_glow.enabled)
          ->set_render_on(&config->visual.glow_enabled);

        gcs::toggle(entities_glow_group, WXOR(L"Objective"), &config->world_glow.object_enabled[2])
          ->set_render_on(&config->world_glow.enabled)
          ->set_render_on(&config->visual.glow_enabled);

        gcs::toggle(entities_glow_group, WXOR(L"Whiskey"), &config->world_glow.object_enabled[3])
          ->set_render_on(&config->world_glow.enabled)
          ->set_render_on(&config->visual.glow_enabled);

        gcs::toggle(entities_glow_group, WXOR(L"Weapon crate"), &config->world_glow.object_enabled[4])
          ->set_render_on(&config->world_glow.enabled)
          ->set_render_on(&config->visual.glow_enabled);

        gcs::toggle(entities_glow_group, WXOR(L"Horse"), &config->world_glow.object_enabled[5])
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
              gcs::label(colours_glow_group, WXOR(L"Vigilantes"))
                ->set_render_on(&config->visual.player_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_glow_vigil_team_colour);

              gcs::label(colours_glow_group, WXOR(L"Desperados"))
                ->set_render_on(&config->visual.player_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_glow_desper_team_colour);

              gcs::label(colours_glow_group, WXOR(L"Bandidos"))
                ->set_render_on(&config->visual.player_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_glow_bandio_team_colour);

              gcs::label(colours_glow_group, WXOR(L"Rangers"))
                ->set_render_on(&config->visual.player_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_glow_rangers_team_colour);

              gcs::label(colours_glow_group, WXOR(L"Unassigned"))
                ->set_render_on(&config->visual.player_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_glow_unassigned_colour);
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
              gcs::label(colours_glow_group, WXOR(L"Vigilantes"))
                ->set_render_on(&config->visual.player_glow_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_glow_friend_vigil_team_colour);

              gcs::label(colours_glow_group, WXOR(L"Desperados"))
                ->set_render_on(&config->visual.player_glow_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_glow_friend_desper_team_colour);

              gcs::label(colours_glow_group, WXOR(L"Bandidos"))
                ->set_render_on(&config->visual.player_glow_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_glow_friend_bandio_team_colour);

              gcs::label(colours_glow_group, WXOR(L"Rangers"))
                ->set_render_on(&config->visual.player_glow_friends_use_mult_colour)
                ->set_render_on(&config->visual.player_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_glow_friend_rangers_team_colour);
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
              gcs::label(colours_glow_group, WXOR(L"Vigilantes"))
                ->set_render_on(&config->visual.world_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_vigil_team_colour);

              gcs::label(colours_glow_group, WXOR(L"Desperados"))
                ->set_render_on(&config->visual.world_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_desper_team_colour);

              gcs::label(colours_glow_group, WXOR(L"Bandidos"))
                ->set_render_on(&config->visual.world_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_bandio_team_colour);

              gcs::label(colours_glow_group, WXOR(L"Rangers"))
                ->set_render_on(&config->visual.world_glow_use_team_colours)
                ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_rangers_team_colour);
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

            gcs::label(colours_glow_group, WXOR(L"Whiskey"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_whiskey_colour);

            gcs::label(colours_glow_group, WXOR(L"Dynamite"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_dynamite_colour);

            gcs::label(colours_glow_group, WXOR(L"Horse"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_horse_colour);

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
      ///gcs_component_group* other_group   = group_host->new_group(WXOR(L"Other"));
      group_host->update();

      // General
      {
        gcs::toggle(general_group, WXOR(L"Bunny hop"), &config->misc.bhop);
        gcs::toggle(general_group, WXOR(L"Auto pistol"), &config->misc.auto_pistol);

        #if defined(DEV_MODE)
        gcs::toggle(general_group, WXOR(L"Server crasher"), &config->misc.server_crasher)
          ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&config->misc.server_crasher_key)
            ->set_render_on(&config->misc.server_crasher);
        #endif
      }

      // Other
      {

      }
    }

    return group_host;
  }

  bool setup() override {
    load_config();
    vec3i menu_pos = render->screen_size / 2;

    gcs_component_window* window = create_window(this, WXOR(L"RijiN For Fistful of Frags"), menu_pos);
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
        assert(sidebar != nullptr);
        sidebar->add_entry(create_aimbot(window, canvas_pos, canvas_size));
        sidebar->add_entry(WXOR(L"Visuals"))
          ->add_entry(create_visuals_main(window, canvas_pos, canvas_size))
          ->add_entry(create_visuals_models(window, canvas_pos, canvas_size));

        sidebar->add_entry(create_misc(window, canvas_pos, canvas_size));

        add_cloud_configs_selection(sidebar);
      }
    }

    return true;
  }

  u32 get_game_hash() override{
    return HASH("FOF");
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