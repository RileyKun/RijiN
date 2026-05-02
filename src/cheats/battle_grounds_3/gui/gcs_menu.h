#pragma once

#include "../link.h"

class c_global;

class gcs_menu : public gcs_scene{
public:
  vec3i menu_size     = vec3i(600, 500);
  i32 select_width    = menu_size.x / 5;

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
          ->add_option(WXOR(L"Enemies"), &config->aimbot.target_enemies)
          ->add_option(WXOR(L"Friends"), &config->aimbot.target_friends);

        gcs::dropdown(target_group, WXOR(L"Algorithm"), false)
          ->add_option(WXOR(L"Closest to FOV"), &config->aimbot.target_method_closest_to_fov)
          ->add_option(WXOR(L"Distance"), &config->aimbot.target_method_distance);

        gcs::dropdown(target_group, WXOR(L"Priority hitbox"), false)
          ->add_option(WXOR(L"Head"), &config->aimbot.priority_hitbox[2])
          ->add_option(WXOR(L"Pelvis"), &config->aimbot.priority_hitbox[0])
          ->add_option(WXOR(L"Upper Chest"), &config->aimbot.priority_hitbox[1]);

        gcs::dropdown(target_group, WXOR(L"Hitscan"), true)
          ->add_option(WXOR(L"Head"), &config->aimbot.hitbox[2])
          ->add_option(WXOR(L"Pelvis"), &config->aimbot.hitbox[1])
          ->add_option(WXOR(L"Upper Chest"), &config->aimbot.hitbox[1]);
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

      if(config_idx == PLAYER_ESP_SETTING_FRIEND){
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
      gcs_component_group* player_enemy_group   = player_esp_group->new_category(WXOR(L"Enemy"));
      gcs_component_group* player_friend_group  = player_esp_group->new_category(WXOR(L"Friend"));
      gcs_component_group* player_colours_group = player_esp_group->new_category(WXOR(L"Colours"));

      gcs_component_group* world_esp_group      = group_host->new_group(WXOR(L"World esp"));
      gcs_component_group* world_entities_group = world_esp_group->new_category(WXOR(L"Entities"));
      gcs_component_group* world_colours_group  = world_esp_group->new_category(WXOR(L"Colours"));

      // Player ESP
      {
        create_player_esp_tabs(player_enemy_group, PLAYER_ESP_SETTING_ENEMY);
        create_player_esp_tabs(player_friend_group, PLAYER_ESP_SETTING_FRIEND);
        // Colours
        {
          // Enemy
          {
            gcs::label(player_colours_group, WXOR(L"Enemy"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_esp_enemy_colour);
          }

          // Aimbot colour
          gcs::toggle(player_colours_group, WXOR(L"Aimbot target"), &config->visual.player_esp_aimbot_target)
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.player_esp_aimbot_target_colour);

          gcs::label(player_colours_group, WXOR(L"Friends"))
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.player_esp_friend_colour);
        }
      }

      // World ESP
      {
        gcs::toggle(world_entities_group, WXOR(L"Enabled"), &config->world_esp.enabled);

        // Options
        {
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
          gcs::label(world_colours_group, WXOR(L"Unassigned"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_esp_unassigned_colour);

          gcs::label(world_colours_group, WXOR(L"Active grenade"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_esp_active_grenade_colour);

          gcs::label(world_colours_group, WXOR(L"Dropped weapons"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_esp_dropped_weapon_colour);

          gcs::label(world_colours_group, WXOR(L"Objective"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_esp_objective_colour);
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

        create_player_chams_tabs(player_chams_enemy_group, PLAYER_ESP_SETTING_ENEMY);
        create_player_chams_tabs(player_chams_friend_group, PLAYER_ESP_SETTING_FRIEND);

        // Colours
        {
           // Enemy / friendly based.
           {
             gcs::label(player_chams_colours_group, WXOR(L"Enemy"))
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.player_chams_enemy_colour);

             gcs::label(player_chams_colours_group, WXOR(L"Overlay Enemy"))
               ->add_module<gcs_component_colour_picker>()
               ->set_output_colour_ptr(config->visual.player_chams_overlay_enemy_colour);
           }

           // Aimbot colour
           gcs::toggle(player_chams_colours_group, WXOR(L"Aimbot target"), &config->visual.player_chams_aimbot_target)
             ->add_module<gcs_component_colour_picker>()
             ->set_output_colour_ptr(config->visual.player_chams_aimbot_target_colour);

           gcs::label(player_chams_colours_group, WXOR(L"Friends"))
             ->add_module<gcs_component_colour_picker>()
             ->set_output_colour_ptr(config->visual.player_chams_friend_colour);

           gcs::label(player_chams_colours_group, WXOR(L"Overlay Friends"))
             ->add_module<gcs_component_colour_picker>()
             ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_colour);
        }
      }

      // World chams
      {
        // Entities
        {
          gcs::toggle(world_chams_entities_group, WXOR(L"Enabled"), &config->world_chams.enabled);

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

          gcs::label(world_chams_colours_group, WXOR(L"Unassigned"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_chams_unassigned_colour);

          gcs::label(world_chams_colours_group, WXOR(L"Active grenade"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_chams_active_grenade_colour);

          gcs::label(world_chams_colours_group, WXOR(L"Dropped weapons"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_chams_dropped_weapon_colour);

          gcs::label(world_chams_colours_group, WXOR(L"Objective"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_chams_objective_colour);

          // Overlays
          gcs::label(world_chams_colours_group, WXOR(L"Overlay unassigned"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_chams_overlay_unassigned_colour);

          gcs::label(world_chams_colours_group, WXOR(L"Overlay Active grenade"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_chams_overlay_active_grenade_colour);

          gcs::label(world_chams_colours_group, WXOR(L"Overlay Dropped weapons"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_chams_overlay_dropped_weapon_colour);

          gcs::label(world_chams_colours_group, WXOR(L"Overlay Ammo"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_chams_overlay_objective_colour);

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
            ->add_option(WXOR(L"Friends"), &config->visual.player_glow_render_steamfriends)
            ->set_render_on(&config->visual.glow_enabled)
            ->set_render_on(&config->visual.player_glow_enabled);
        }

        // World
        gcs::toggle(entities_glow_group, WXOR(L"World"), &config->world_glow.enabled)
           ->set_render_on(&config->visual.glow_enabled);

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
            gcs::label(colours_glow_group, WXOR(L"Enemy"))
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.player_glow_enemy_colour);

            gcs::label(colours_glow_group, WXOR(L"Friends"))
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.player_glow_friend_colour);

            gcs::toggle(colours_glow_group, WXOR(L"Aimbot target"), &config->visual.player_glow_aimbot_target)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.player_glow_aimbot_target_colour);
          }

          gcs::label(colours_glow_group, L""); // A little space to make easier for people to understand that above is for players and below is for world stuff.

          // World colours
          {
            gcs::label(colours_glow_group, WXOR(L"Active grenade"))
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_glow_active_grenade_colour);

            gcs::label(colours_glow_group, WXOR(L"Dropped weapon"))
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_glow_dropped_weapon_colour);

            gcs::label(colours_glow_group, WXOR(L"Ammo"))
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_glow_objective_colour);
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
        gcs::toggle(general_group, WXOR(L"Speedhack"), &config->misc.speedhack);
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

  void add_cloud_configs_selection(gcs_component_base* comp){
    if(!cloud::enabled)
      return;

    assert(comp != nullptr);

    u32 padding = 4;
    u32 top_offset = 0;

    // define the config slot dummy booleans
    static bool config_slot[5];
    {
      // Set all but first value to false
      for(u32 i = 0; i < sizeof(config_slot) / sizeof(bool); i++)
        config_slot[i] = (i == 0);
    }

    // create dropdown
    gcs_component_dropdown* select_config = gcs::dropdown(comp, WXOR(L"Config"), false)
      ->add_option(WXOR(L"Default"), &config_slot[0])
      ->add_option(WXOR(L"Config 1"), &config_slot[1])
      ->add_option(WXOR(L"Config 2"), &config_slot[2])
      ->add_option(WXOR(L"Config 3"), &config_slot[3])
      ->add_option(WXOR(L"Config 4"), &config_slot[4]);

    // draw buttons
    {
      u32 button_width = (comp->size().x / 2) - (padding);

      gcs_component_button* load_config = comp->add<gcs_component_button>();
      {
        assert(load_config != nullptr);

        load_config->info()->disable_auto_positioning();
        load_config->info()->enable_override_auto_positioning();
        load_config->set_name(WXOR(L"Load"));
        load_config->set_param(&select_config->last_selected_index);
        load_config->set_size(vec3i(button_width - (padding / 2), 16));
        load_config->set_pos(vec3i(padding, comp->size().y - (load_config->size().y + padding + top_offset)));

        load_config->add_hook(GCS_HOOK_CLICK, [](gcs_component_base* c, void* p){
          assert(p != nullptr);

          s_cloud_thread_data* thread_data = malloc(sizeof(s_cloud_thread_data));
          {
            thread_data->id     = *(u32*)p;
            thread_data->extra  = c;
          }

          utils::create_worker([](s_cloud_thread_data* data){
            assert(data != nullptr);
            assert(data->extra != nullptr);

            gcs_component_base* comp = data->extra;

            comp->set_name(WXOR(L"..."));
            {
              if(cloud::download(data->id, config, sizeof(c_settings), SETTINGS_VERSION, true))
                comp->set_name(WXOR(L"Done"));
              else
                comp->set_name(WXOR(L"Fail"));

              Sleep(XOR32(1000));
            }
            comp->set_name(WXOR(L"Load"));

            free(data);

            return 0;
          }, thread_data);

          return false;
        });
      }

      gcs_component_button* save_config = comp->add<gcs_component_button>();
      {
        assert(save_config != nullptr);

        save_config->info()->disable_auto_positioning();
        save_config->info()->enable_override_auto_positioning();
        save_config->set_name(WXOR(L"Save"));
        save_config->set_param(&select_config->last_selected_index);
        save_config->set_size(vec3i(button_width - ((padding / 2)), 16));
        save_config->set_pos(vec3i(button_width + padding + (padding / 2), comp->size().y - (save_config->size().y + padding + top_offset)));

        save_config->add_hook(GCS_HOOK_CLICK, [](gcs_component_base* c, void* p){
          assert(p != nullptr);

          s_cloud_thread_data* thread_data = malloc(sizeof(s_cloud_thread_data));
          {
            thread_data->id     = *(u32*)p;
            thread_data->extra  = c;
          }

          utils::create_worker([](s_cloud_thread_data* data){
            assert(data != nullptr);
            assert(data->extra != nullptr);

            gcs_component_base* comp = data->extra;

            comp->set_name(WXOR(L"..."));
            {
              if(cloud::upload(data->id, config, sizeof(c_settings), true))
                comp->set_name(WXOR(L"Done"));
              else
                comp->set_name(WXOR(L"Fail"));

              Sleep(XOR32(1000));
            }
            comp->set_name(WXOR(L"Save"));

            free(data);

            return 0;
          }, thread_data);

          return false;
        });
      }

      top_offset += load_config->size().y + padding;
    }

    // set selection dropdown
    {
      select_config->info()->disable_auto_positioning();
      select_config->info()->enable_override_auto_positioning();
      select_config->set_full_wide(true);
      select_config->set_size(vec3i(comp->size().x - (padding * 2), 16));
      select_config->set_pos(vec3i(padding, comp->size().y - (select_config->size().y + padding + top_offset)));
      top_offset += select_config->size().y + padding;
    }
  }

  bool setup() override {
    // cloud configs
    {
      cloud::enabled = true;

      if(cloud::enabled && !input_system->held(VK_HOME))
        cloud::download(0, config, XOR32(sizeof(c_settings)), XOR32(SETTINGS_VERSION), true);
    }

    vec3i menu_pos = render->screen_size / 2;

    gcs_component_window* window = gcs::window(this, WXOR(L"RijiN For Double Action: Boogaloo"), menu_pos, menu_size);
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

      gcs_component_select_sidebar* sidebar = window->add<gcs_component_select_sidebar>();
      {
        assert(sidebar != nullptr);

        sidebar->set_pos(vec3i());
        sidebar->set_size(vec3i(select_width, menu_size.y));

        vec3i canvas_pos = vec3i(select_width, 0);
        vec3i canvas_size = vec3i(menu_size.x - select_width, menu_size.y);
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
};