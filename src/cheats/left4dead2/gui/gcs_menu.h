#pragma once

#include "../link.h"

class c_global;

class gcs_menu : public gcs_base_menu{
public:
  gcs_component_group_host* create_aimbot(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Aimbot"), pos, size);
    {
      gcs_component_group* aimbot_group     = group_host->new_group(WXOR(L"Aimbot"));
      gcs_component_group* target_group     = group_host->new_group(WXOR(L"Targets"));
      gcs_component_group* other_group      = group_host->new_group(WXOR(L"Other"));
      gcs_component_group* auto_bash_group  = group_host->new_group(WXOR(L"Auto bash"));
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
          ->add_option(WXOR(L"Normal infected"), &config->aimbot.target_zombies)
          ->add_option(WXOR(L"Special infected"), &config->aimbot.target_special_infected)
          ->add_option(WXOR(L"Survivors"), &config->aimbot.target_survivors)
          ->add_option(WXOR(L"Friends"), &config->aimbot.target_friends);

        gcs::dropdown(target_group, WXOR(L"Algorithm"), false)
          ->add_option(WXOR(L"Smart shot"), &config->aimbot.target_method_smart_shot)
          ->add_option(WXOR(L"Closest to FOV"), &config->aimbot.target_method_closest_to_fov)
          ->add_option(WXOR(L"Distance"), &config->aimbot.target_method_distance);

        gcs::dropdown(target_group, WXOR(L"Ignore flags"), true)
          ->add_option(WXOR(L"Idle infected"), &config->aimbot.ignore_idle_zombies)
          ->add_option(WXOR(L"Burning infected"), &config->aimbot.ignore_burning_zombies)
          ->add_option(WXOR(L"Wandering witch"), &config->aimbot.ignore_wandering_zombies);
      }

      // Other
      {
        gcs::toggle(other_group, WXOR(L"No spread"), &config->aimbot.no_spread);
        gcs::toggle(other_group, WXOR(L"No recoil"), &config->aimbot.no_recoil);
        gcs::toggle(other_group, WXOR(L"Avoid shooting team-mates"), &config->aimbot.avoid_teammates);

        //gcs::toggle(other_group, WXOR(L"Auto wall"), &config->aimbot.autowall_enabled);
        //gcs::slider(other_group, WXOR(L"Minimum damage"), WXOR(L"%"), 1, 90, GCS_SLIDER_TYPE_FLOAT, &config->aimbot.autowall_min_dmg)
        //  ->set_render_on(&config->aimbot.autowall_enabled);
      }

      // HINT-HEADER: Auto bash
      {
        gcs::toggle(auto_bash_group, WXOR(L"Enabled"), &config->aimbot.auto_bash_enabled);
        gcs::slider(auto_bash_group, WXOR(L"Maximum FOV"), WXOR(L"°"), 1, 180, GCS_SLIDER_TYPE_INT, &config->aimbot.auto_bash_max_fov)
          ->set_render_on(&config->aimbot.auto_bash_enabled);
          
        gcs::dropdown(auto_bash_group, WXOR(L"Mode"), false)
          ->add_option(WXOR(L"Smart"), &config->aimbot.auto_bash_mode_smart)
          ->add_option(WXOR(L"Aggressive"), &config->aimbot.auto_bash_mode_aggressive)
            ->set_render_on(&config->aimbot.auto_bash_enabled);
        gcs::dropdown(auto_bash_group, WXOR(L"Target"), false)
          ->add_option(WXOR(L"Special only"), &config->aimbot.auto_bash_target_special_only)
          ->add_option(WXOR(L"Special & Infected"), &config->aimbot.auto_bash_target_special_and_infected)
            ->set_render_on(&config->aimbot.auto_bash_enabled);
      }
    }

    return group_host;
  }

  gcs_component_group_host* create_player_esp_tabs(gcs_component_base* comp, i32 config_idx){
    assert(comp != nullptr);
    assert(config_idx >= 0 && config_idx <= 3);

    {
      c_player_esp_settings* esp_settings = utils::get_player_esp_settings(config_idx);
      gcs::toggle(comp, WXOR(L"Enabled"), &esp_settings->enabled);

      if(config_idx == 2){
        gcs::toggle(comp, WXOR(L"Render friend text"), &esp_settings->friend_tag)
          ->set_render_on(&esp_settings->enabled);

        gcs::toggle(comp, WXOR(L"Show localplayer"), &esp_settings->show_localplayer)
          ->set_render_on(&esp_settings->enabled);
      }

      gcs::dropdown(comp, WXOR(L"Components"), true)
        ->add_option(WXOR(L"Name"), &esp_settings->name)
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

  #define RIJIN_L4D2_FLAT_CHAMS_ONLY
  gcs_component_group_host* create_player_chams_tabs(gcs_component_base* comp, i32 config_idx){
    assert(comp != nullptr);
    assert(config_idx >= 0 && config_idx <= 2);

    {
      c_player_chams_settings* chams_settings = utils::get_player_chams_settings(config_idx);
      gcs::toggle(comp, WXOR(L"Enabled"), &chams_settings->enabled);
      if(config_idx == 2){
        gcs::toggle(comp, WXOR(L"Show localplayer"), &chams_settings->show_localplayer)
          ->set_render_on(&chams_settings->enabled);
      }

      gcs::toggle(comp, WXOR(L"Ignore z"), &chams_settings->ignore_z)
        ->set_render_on(&chams_settings->enabled);

      #if !defined(RIJIN_L4D2_FLAT_CHAMS_ONLY)
        gcs::dropdown(comp, WXOR(L"Base material"), false)
        ->add_option(WXOR(L"None"), &chams_settings->base_material_none)
        ->add_option(WXOR(L"Shaded"), &chams_settings->base_material_shaded)
        ->add_option(WXOR(L"Flat"), &chams_settings->base_material_flat)
        ->set_render_on(&chams_settings->enabled);

        gcs::dropdown(comp, WXOR(L"Overlay material"), false)
          ->add_option(WXOR(L"None"), &chams_settings->overlay_material_none)
          ->add_option(WXOR(L"Flat"), &chams_settings->overlay_material_flat)
          ->add_option(WXOR(L"Fresnel"), &chams_settings->overlay_material_fresnel)
          ->set_render_on(&chams_settings->enabled);

      #elif defined(RIJIN_L4D2_FLAT_CHAMS_ONLY)
        gcs::dropdown(comp, WXOR(L"Base material"), false)
          ->add_option(WXOR(L"None"), &chams_settings->base_material_none)
          ->add_option(WXOR(L"Flat"), &chams_settings->base_material_flat)
          ->set_render_on(&chams_settings->enabled);
      #endif
    }

    return nullptr;
  }

  void create_world_esp_options(gcs_component_base* comp, std::wstring name, i32 type){
    assert(comp != nullptr);
    assert(type >= 0);
    gcs::toggle(comp, name, &config->world_esp.object_enabled[type]);
    gcs::dropdown(comp, WXOR(L"Components"), true)
      ->add_option(WXOR(L"Box"), &config->world_esp.box[type])
      ->add_option(WXOR(L"Box Outline"), &config->world_esp.box_outlines[type])
      ->add_option(WXOR(L"Name"), &config->world_esp.name[type])
      ->add_option(WXOR(L"Distance"), &config->world_esp.distance[type])
      ->set_render_on(&config->world_esp.object_enabled[type])
      ->set_render_on(&config->world_esp.enabled);
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

      create_player_esp_tabs(player_team_group, 0);
      create_player_esp_tabs(player_enemy_group, 1);
      create_player_esp_tabs(player_friend_group, 2);

      // Player Colours
      {
        gcs::label(player_colours_group, WXOR(L"Friendly player colour"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.box_player_colour_friendly);

        gcs::label(player_colours_group, WXOR(L"Enemy player colour"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.box_player_colour_enemy);

        gcs::label(player_colours_group, WXOR(L"Friend colour"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.player_esp_friend_colour);

        gcs::label(player_colours_group, WXOR(L"Infected colour"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.box_colour_infected);

        gcs::label(player_colours_group, WXOR(L"Hunter colour"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.box_colour_hunter);

        gcs::label(player_colours_group, WXOR(L"Jockey colour"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.box_colour_jockey);

        gcs::label(player_colours_group, WXOR(L"Charger colour"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.box_colour_charger);

        gcs::label(player_colours_group, WXOR(L"Smoker colour"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.box_colour_smoker);

        gcs::label(player_colours_group, WXOR(L"Boomer colour"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.box_colour_boomer);

        gcs::label(player_colours_group, WXOR(L"Spitter colour"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.box_colour_spitter);

        gcs::label(player_colours_group, WXOR(L"Tank colour"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.box_colour_tank);

        gcs::label(player_colours_group, WXOR(L"Witch colour"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.box_colour_witch);
      }

      gcs_component_group* world_esp_group      = group_host->new_group(WXOR(L"World esp"));
      gcs_component_group* world_esp_main       = world_esp_group->new_category(WXOR(L"Main"));
      gcs_component_group* world_colours_group  = world_esp_group->new_category(WXOR(L"Colours"));

      {
        // Main
        {
          gcs::toggle(world_esp_main, WXOR(L"Enabled"), &config->world_esp.enabled);
          gcs::slider(world_esp_main, WXOR(L"Max render distance"), WXOR(L"m"), 10, 100, GCS_SLIDER_TYPE_FLOAT, &config->world_esp.world_esp_render_dist)
            ->set_render_on(&config->world_esp.enabled);

          create_world_esp_options(world_esp_main, WXOR(L"Grenade"), 0);
          create_world_esp_options(world_esp_main, WXOR(L"Dropped weapons"), 1);
          create_world_esp_options(world_esp_main, WXOR(L"World items"), 2);
          create_world_esp_options(world_esp_main, WXOR(L"Objectives"), 3);
        }

        // World colours
        {
          gcs::label(world_colours_group, WXOR(L"Grenade"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_esp_grenade_colour);

          gcs::label(world_colours_group, WXOR(L"Dropped weapon"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_esp_dropped_weapon_colour);

          gcs::label(world_colours_group, WXOR(L"World items"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_esp_dropped_items_colour);

          gcs::label(world_colours_group, WXOR(L"Health items"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.world_esp_health_items_colour);

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
        gcsf::add_space(other_group);

        gcs::toggle(other_group, WXOR(L"Override FOV"), &config->visual.override_fov);
        gcs::slider(other_group, WXOR(L"FOV"), WXOR(L"°"), 75, 140, GCS_SLIDER_TYPE_FLOAT, &config->visual.override_fov_val)
            ->set_render_on(&config->visual.override_fov);

       gcsf::add_space(other_group);
       gcs::toggle(other_group, WXOR(L"Thirdperson"), &config->tp.thirdperson)
          ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&config->tp.thirdperson_key)
          ->set_render_on(&config->tp.thirdperson)
          ->set_render_on(&config->tp.thirdperson_usekey);
        {
          gcs::toggle(other_group, WXOR(L"Camera collision"), &config->tp.thirdperson_collision)
            ->set_render_on(&config->tp.thirdperson);

          gcs::toggle(other_group, WXOR(L"Use key"), &config->tp.thirdperson_usekey)
            ->set_render_on(&config->tp.thirdperson);

          gcs::dropdown(other_group, WXOR(L"Key control"), false)
            ->add_option(WXOR(L"Held"), &config->tp.thirdperson_key_held)
            ->add_option(WXOR(L"Toggle"), &config->tp.thirdperson_key_toggle)
            ->add_option(WXOR(L"Double click"), &config->tp.thirdperson_key_double_click)
            ->set_render_on(&config->tp.thirdperson)
            ->set_render_on(&config->tp.thirdperson_usekey);

          gcs::slider(other_group, WXOR(L"Max distance"), WXOR(L"hu"), 30, 400, GCS_SLIDER_TYPE_INT, &config->tp.thirdperson_distance)
            ->set_render_on(&config->tp.thirdperson);

          gcs::slider(other_group, WXOR(L"Up offset"), WXOR(L"hu"), -64, 64, GCS_SLIDER_TYPE_INT, &config->tp.thirdperson_up_offset)
            ->set_render_on(&config->tp.thirdperson);

          gcs::toggle(other_group, WXOR(L"Right offset"), &config->tp.thirdperson_right_offset_enabled)
            ->set_render_on(&config->tp.thirdperson)
            ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&config->tp.thirdperson_right_offset_key)
            ->set_render_on(&config->tp.thirdperson_right_offset_enabled)
            ->set_render_on(&config->tp.thirdperson_right_offset_use_key);

          gcs::toggle(other_group, WXOR(L"Use key"), &config->tp.thirdperson_right_offset_use_key)
            ->set_render_on(&config->tp.thirdperson)
            ->set_render_on(&config->tp.thirdperson_right_offset_enabled);

            gcs::dropdown(other_group, WXOR(L"Key control"), false)
            ->add_option(WXOR(L"Held"), &config->tp.thirdperson_right_offset_held)
            ->add_option(WXOR(L"Toggle"), &config->tp.thirdperson_right_offset_toggle)
            ->add_option(WXOR(L"Double click"), &config->tp.thirdperson_right_offset_double_click)
            ->set_render_on(&config->tp.thirdperson)
            ->set_render_on(&config->tp.thirdperson_right_offset_enabled);

          gcs::label(other_group, WXOR(L"Invert offset key"))
            ->set_render_on(&config->tp.thirdperson)
            ->set_render_on(&config->tp.thirdperson_right_offset_enabled)
              ->add_module<gcs_component_hotkey>()
                ->set_hotkey_ptr(&config->tp.thirdperson_right_offset_invert_key);

          gcs::dropdown(other_group, WXOR(L"Invert key control"), false)
            ->add_option(WXOR(L"Held"), &config->tp.thirdperson_right_offset_invert_held)
            ->add_option(WXOR(L"Toggle"), &config->tp.thirdperson_right_offset_invert_toggle)
            ->add_option(WXOR(L"Double click"), &config->tp.thirdperson_right_offset_invert_double_click)
            ->set_render_on(&config->tp.thirdperson)
            ->set_render_on(&config->tp.thirdperson_right_offset_enabled);

          gcs::slider(other_group, WXOR(L"Offset"), WXOR(L"hu"), -64, 64, GCS_SLIDER_TYPE_INT, &config->tp.thirdperson_right_offset)
            ->set_render_on(&config->tp.thirdperson)
            ->set_render_on(&config->tp.thirdperson_right_offset_enabled);
        }

        gcsf::add_space(other_group);
        // HINT-HEADER: Viewmodel
        {

          gcs::toggle(other_group, WXOR(L"Offset"), &config->vm.offset_enabled);
          {
            gcs::slider(other_group, WXOR(L"Forward"), WXOR(L"hu"), -60, 60, GCS_SLIDER_TYPE_INT, &config->vm.offset_x)
              ->set_render_on(&config->vm.offset_enabled);
            gcs::slider(other_group, WXOR(L"Right"), WXOR(L"hu"), -60, 60, GCS_SLIDER_TYPE_INT, &config->vm.offset_y)
              ->set_render_on(&config->vm.offset_enabled);
            gcs::slider(other_group, WXOR(L"Up"), WXOR(L"hu"), -60, 60, GCS_SLIDER_TYPE_INT, &config->vm.offset_z)
              ->set_render_on(&config->vm.offset_enabled);

            gcs::slider(other_group, WXOR(L"Pitch"), WXOR(L"°"), -89, 89, GCS_SLIDER_TYPE_INT, &config->vm.pitch)
              ->set_render_on(&config->vm.offset_enabled);
            gcs::slider(other_group, WXOR(L"Yaw"), WXOR(L"°"), -180, 180, GCS_SLIDER_TYPE_INT, &config->vm.yaw)
              ->set_render_on(&config->vm.offset_enabled);
            gcs::slider(other_group, WXOR(L"Roll"), WXOR(L"°"), -180, 180, GCS_SLIDER_TYPE_INT, &config->vm.roll)
              ->set_render_on(&config->vm.offset_enabled);
          }

          gcsf::add_space(other_group, &config->vm.offset_enabled);

          //gcs::toggle(other_group, WXOR(L"Weapon sway"), &config->vm.sway_enabled);
          //{
          //  gcs::slider(other_group, WXOR(L"Scale"), L"", 1, 10, GCS_SLIDER_TYPE_FLOAT, &config->vm.sway_amount)
          //    ->set_render_on(&config->vm.sway_enabled);
          //}

          gcsf::add_space(other_group, &config->vm.sway_enabled);

          gcs::toggle(other_group, WXOR(L"Show silent aim"), &config->vm.silent_aim);
        }

        //
      }
      group_host->update();
    }

    return group_host;
  }

  gcs_component_group_host* create_visuals_models(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Chams/Glow"), pos, size);
    {
      assert(group_host != nullptr);

      gcs_component_group* player_chams_group   = group_host->new_group(WXOR(L"Player chams"));
      gcs_component_group* player_team_group    = player_chams_group->new_category(WXOR(L"Team"));
      gcs_component_group* player_enemy_group   = player_chams_group->new_category(WXOR(L"Enemy"));
      gcs_component_group* player_friend_group   = player_chams_group->new_category(WXOR(L"Friend"));
      gcs_component_group* player_colours_group = player_chams_group->new_category(WXOR(L"Colours"));

      gcs_component_group* world_chams_group          = group_host->new_group(WXOR(L"World chams"));
      gcs_component_group* world_chams_entities_group = world_chams_group->new_category(WXOR(L"Entities"));
      gcs_component_group* world_chams_settings_group = world_chams_group->new_category(WXOR(L"Settings"));
      gcs_component_group* world_chams_colours_group  = world_chams_group->new_category(WXOR(L"Colours"));

      gcs_component_group* glow_group          = group_host->new_group(WXOR(L"Glow"));
      gcs_component_group* entities_glow_group = glow_group->new_category(WXOR(L"Entities"));
      gcs_component_group* settings_glow_group = glow_group->new_category(WXOR(L"Settings"));
      gcs_component_group* colours_glow_group  = glow_group->new_category(WXOR(L"Colours"));

      create_player_chams_tabs(player_team_group, 0);
      create_player_chams_tabs(player_enemy_group, 1);
      create_player_chams_tabs(player_friend_group, 2);

      // Player Colours
      {
        // Base colors
        {
          gcs::label(player_colours_group, WXOR(L"Friendly player"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_player_colour_friendly);

          gcs::label(player_colours_group, WXOR(L"Enemy player"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_player_colour_enemy);

          gcs::label(player_colours_group, WXOR(L"Friend colour"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.player_chams_friend_colour);

          gcs::label(player_colours_group, WXOR(L"Infected"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_colour_infected);

          gcs::label(player_colours_group, WXOR(L"Hunter"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_colour_hunter);

          gcs::label(player_colours_group, WXOR(L"Jockey"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_colour_jockey);

          gcs::label(player_colours_group, WXOR(L"Charger"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_colour_charger);

          gcs::label(player_colours_group, WXOR(L"Smoker"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_colour_smoker);

          gcs::label(player_colours_group, WXOR(L"Boomer"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_colour_boomer);

          gcs::label(player_colours_group, WXOR(L"Spitter"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_colour_spitter);

          gcs::label(player_colours_group, WXOR(L"Tank"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_colour_tank);

          gcs::label(player_colours_group, WXOR(L"Witch"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_colour_witch);
        }

        #if !defined(RIJIN_L4D2_FLAT_CHAMS_ONLY)
        // Overlays
        {
          gcs::label(player_colours_group, WXOR(L"Overlay Friendly player"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_overlay_player_colour_friendly);

          gcs::label(player_colours_group, WXOR(L"Overlay Enemy player"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_overlay_player_colour_enemy);

          gcs::label(player_colours_group, WXOR(L"Overlay Friend colour"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_colour);

          gcs::label(player_colours_group, WXOR(L"Overlay Infected"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_overlay_colour_infected);

          gcs::label(player_colours_group, WXOR(L"Overlay Hunter"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_overlay_colour_hunter);

          gcs::label(player_colours_group, WXOR(L"Overlay Jockey"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_overlay_colour_jockey);

          gcs::label(player_colours_group, WXOR(L"Overlay Charger"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_overlay_colour_charger);

          gcs::label(player_colours_group, WXOR(L"Overlay Smoker"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_overlay_colour_smoker);

          gcs::label(player_colours_group, WXOR(L"Overlay Boomer"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_overlay_colour_boomer);

          gcs::label(player_colours_group, WXOR(L"Overlay Spitter"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_overlay_colour_spitter);

          gcs::label(player_colours_group, WXOR(L"Overlay Tank"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_overlay_colour_tank);

          gcs::label(player_colours_group, WXOR(L"Overlay Witch"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->visual.chams_overlay_colour_witch);
        }
        #endif
      }

      // World chams
      {
        // Entities
        {
          gcs::toggle(world_chams_entities_group, WXOR(L"Enabled"), &config->world_chams.enabled);
          gcs::slider(world_chams_entities_group, WXOR(L"Max render distance"), WXOR(L"m"), 10, 100, GCS_SLIDER_TYPE_FLOAT, &config->world_chams.world_chams_render_dist)
            ->set_render_on(&config->world_chams.enabled);

          gcs::toggle(world_chams_entities_group, WXOR(L"Grenade"), &config->world_chams.object_enabled[0])
            ->set_render_on(&config->world_chams.enabled);

          gcs::toggle(world_chams_entities_group, WXOR(L"Dropped weapons"), &config->world_chams.object_enabled[1])
            ->set_render_on(&config->world_chams.enabled);

          gcs::toggle(world_chams_entities_group, WXOR(L"World items"), &config->world_chams.object_enabled[2])
            ->set_render_on(&config->world_chams.enabled);

          gcs::toggle(world_chams_entities_group, WXOR(L"Objectives"), &config->world_chams.object_enabled[3])
            ->set_render_on(&config->world_chams.enabled);
        }

        // Settings
        {
          gcs::toggle(world_chams_settings_group, WXOR(L"Ignore z"), &config->world_chams.ignore_z);

          #if !defined(RIJIN_L4D2_FLAT_CHAMS_ONLY)
            gcs::dropdown(world_chams_settings_group, WXOR(L"Base material"), false)
              ->add_option(WXOR(L"None"), &config->world_chams.base_material_none)
              ->add_option(WXOR(L"Shaded"), &config->world_chams.base_material_shaded)
              ->add_option(WXOR(L"Flat"), &config->world_chams.base_material_flat);

            gcs::dropdown(world_chams_settings_group, WXOR(L"Overlay material"), false)
              ->add_option(WXOR(L"None"), &config->world_chams.overlay_material_none)
              ->add_option(WXOR(L"Flat"), &config->world_chams.overlay_material_flat)
              ->add_option(WXOR(L"Fresnel"), &config->world_chams.overlay_material_fresnel);

          #elif defined(RIJIN_L4D2_FLAT_CHAMS_ONLY)
            gcs::dropdown(world_chams_settings_group, WXOR(L"Base material"), false)
              ->add_option(WXOR(L"None"), &config->world_chams.base_material_none)
              ->add_option(WXOR(L"Flat"), &config->world_chams.base_material_flat);
          #endif
        }

        // World colours
        {
          // Base
          {
            gcs::label(world_chams_colours_group, WXOR(L"Grenade"))
              ->add_module<gcs_component_colour_picker>()
              ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_grenade_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Dropped weapon"))
              ->add_module<gcs_component_colour_picker>()
              ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_dropped_weapon_colour);

            gcs::label(world_chams_colours_group, WXOR(L"World items"))
              ->add_module<gcs_component_colour_picker>()
              ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_dropped_items_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Health items"))
              ->add_module<gcs_component_colour_picker>()
              ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_health_items_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Objective"))
              ->add_module<gcs_component_colour_picker>()
              ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_objective_colour);
          }

          #if !defined(RIJIN_L4D2_FLAT_CHAMS_ONLY)
          // Overlays
          {
            gcs::label(world_chams_colours_group, WXOR(L"Overlay Grenade"))
              ->add_module<gcs_component_colour_picker>()
              ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_grenade_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Overlay Dropped weapon"))
              ->add_module<gcs_component_colour_picker>()
              ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_dropped_weapon_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Overlay World items"))
              ->add_module<gcs_component_colour_picker>()
              ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_dropped_items_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Overlay Health items"))
              ->add_module<gcs_component_colour_picker>()
              ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_health_items_colour);

            gcs::label(world_chams_colours_group, WXOR(L"Overlay Objective"))
              ->add_module<gcs_component_colour_picker>()
              ->enable_alpha_bar()
                ->set_output_colour_ptr(config->visual.world_chams_overlay_objective_colour);
          }
          #endif
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
            ->add_option(WXOR(L"Localplayer"), &config->visual.player_glow_render_localplayer)
            ->set_render_on(&config->visual.glow_enabled)
            ->set_render_on(&config->visual.player_glow_enabled);
        }

        // World
        gcs::toggle(entities_glow_group, WXOR(L"World"), &config->world_glow.enabled)
           ->set_render_on(&config->visual.glow_enabled);

        gcs::slider(entities_glow_group, WXOR(L"Max render distance"), WXOR(L"m"), 10, 100, GCS_SLIDER_TYPE_FLOAT, &config->world_glow.world_glow_render_dist)
          ->set_render_on(&config->visual.glow_enabled)
          ->set_render_on(&config->world_glow.enabled);

        gcs::toggle(entities_glow_group, WXOR(L"Grenade"), &config->world_glow.object_enabled[0])
          ->set_render_on(&config->world_glow.enabled)
          ->set_render_on(&config->visual.glow_enabled);

        gcs::toggle(entities_glow_group, WXOR(L"Dropped weapons"), &config->world_glow.object_enabled[1])
          ->set_render_on(&config->world_glow.enabled)
          ->set_render_on(&config->visual.glow_enabled);

        gcs::toggle(entities_glow_group, WXOR(L"World items"), &config->world_glow.object_enabled[2])
          ->set_render_on(&config->world_glow.enabled)
          ->set_render_on(&config->visual.glow_enabled);

        gcs::toggle(entities_glow_group, WXOR(L"Objective"), &config->world_glow.object_enabled[3])
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

          // Player
          {
            gcs::label(colours_glow_group, WXOR(L"Friendly player"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.glow_player_colour_friendly);

            gcs::label(colours_glow_group, WXOR(L"Enemy player"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.glow_player_colour_enemy);

            gcs::label(colours_glow_group, WXOR(L"Friend colour"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.player_glow_friend_colour);

            gcs::label(colours_glow_group, WXOR(L"Infected"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.glow_colour_infected);

            gcs::label(colours_glow_group, WXOR(L"Hunter"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.glow_colour_hunter);

            gcs::label(colours_glow_group, WXOR(L"Jockey"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.glow_colour_jockey);

            gcs::label(colours_glow_group, WXOR(L"Charger"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.glow_colour_charger);

            gcs::label(colours_glow_group, WXOR(L"Smoker"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.glow_colour_smoker);

            gcs::label(colours_glow_group, WXOR(L"Boomer"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.glow_colour_boomer);

            gcs::label(colours_glow_group, WXOR(L"Spitter"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.glow_colour_spitter);

            gcs::label(colours_glow_group, WXOR(L"Tank"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.glow_colour_tank);

            gcs::label(colours_glow_group, WXOR(L"Witch"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.glow_colour_witch);
          }

          gcs::label(colours_glow_group, L"");

          // World
          {
            gcs::label(colours_glow_group, WXOR(L"Grenade"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_grenade_colour);

            gcs::label(colours_glow_group, WXOR(L"Dropped weapons"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_dropped_weapon_colour);

            gcs::label(colours_glow_group, WXOR(L"World items"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_dropped_items_colour);

            gcs::label(colours_glow_group, WXOR(L"Health items"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_health_items_colour);

            gcs::label(colours_glow_group, WXOR(L"Objective"))
              ->add_module<gcs_component_colour_picker>()
                ->set_output_colour_ptr(config->visual.world_glow_objective_colour);
          }

        }

      }
      group_host->update();
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
        gcs::toggle(general_group, WXOR(L"Bunnyhop"), &config->misc.bhop);
        gcs::toggle(general_group, WXOR(L"Auto strafe"), &config->misc.auto_strafe);
        {
          gcs::dropdown(general_group, WXOR(L"Strafe type"), false)
            ->add_option(WXOR(L"Normal"), &config->misc.auto_strafe_type_normal)
            ->add_option(WXOR(L"Directional (WASD)"), &config->misc.auto_strafe_type_directional)
              ->set_render_on(&config->misc.auto_strafe);
          gcs::slider(general_group, WXOR(L"Retrack speed"), WXOR(L"%"), 1, 100, GCS_SLIDER_TYPE_FLOAT, &config->misc.auto_strafe_retrack)
            ->set_render_on(&config->misc.auto_strafe)
            ->set_render_on(&config->misc.auto_strafe_type_directional);
        }
        gcs::toggle(general_group, WXOR(L"Auto pistol"), &config->misc.auto_pistol);
      }

      // Other
      {
        //gcs::toggle(other_group, WXOR(L"Teleport"), &config->misc.teleport)
        //  ->add_module<gcs_component_hotkey>()
        //    ->set_hotkey_ptr(&config->misc.teleport_key)
        //      ->set_render_on(&config->misc.teleport_use_key)
        //      ->set_render_on(&config->misc.teleport);
//
        //gcs::toggle(other_group, WXOR(L"Use key"), &config->misc.teleport_use_key)
        //  ->set_render_on(&config->misc.teleport);

        gcs::toggle(other_group, WXOR(L"Break tickbase"), &config->misc.break_tickbase)
          ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&config->misc.break_tickbase_key);

        gcs::slider(other_group, WXOR(L"Amount"), L"", 10, 5000, GCS_SLIDER_TYPE_INT, &config->misc.break_tickbase_amount)
          ->set_render_on(&config->misc.break_tickbase);

        gcs::toggle(other_group, WXOR(L"Speedhack"), &config->misc.speedhack)
          ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&config->misc.speedhack_key)
              ->set_render_on(&config->misc.speedhack_use_key)
              ->set_render_on(&config->misc.speedhack);

        gcs::toggle(other_group, WXOR(L"Use key"), &config->misc.speedhack_use_key)
          ->set_render_on(&config->misc.speedhack);

        gcs::slider(other_group, WXOR(L"Speedhack amount"), L"x", 2, 21, GCS_SLIDER_TYPE_INT, &config->misc.speedhack_commands)
          ->set_render_on(&config->misc.speedhack);

        gcs::toggle(other_group, WXOR(L"No interpolation"), &config->misc.force_no_interpolation);

#if defined(STAGING_MODE) || defined(DEV_MODE)
        gcs::toggle(other_group, WXOR(L"Godmode exploit"), &config->misc.godmode_exploit)
            ->add_module<gcs_component_hotkey>()
              ->set_hotkey_ptr(&config->misc.godmode_exploit_key)
                ->set_render_on(&config->misc.godmode_exploit);
#endif
      }
    }

    return group_host;
  }

  gcs_component_canvas* create_player_list(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    auto objects = gcs::fetch_playerlist_settings_comp(comp, WXOR(L"Player list"), WXOR(L"Player list settings"), pos, size);
    gcs_component_canvas*     main_canvas = objects.second;
    gcs_component_group_host* group_host  = objects.first;
    {
      if(!global->playerlist_data.valid)
        global->playerlist_data.init();

      gcs_component_group* group      = group_host->new_group(L"Settings");
      gcs_component_group* settings   = group->new_category(WXOR(L"Settings"));
      {
        gcs::toggle(settings, WXOR(L"Friend"), &global->playerlist_data.mark_as_friend);
        gcs::toggle(settings, WXOR(L"Ignore on aimbot"), &global->playerlist_data.ignore_player);
        gcs::dropdown(settings, WXOR(L"Flags"), false)
          ->add_option(WXOR(L"Auto"), &global->playerlist_data.mark_as_auto)
          ->add_option(WXOR(L"Legit"), &global->playerlist_data.mark_as_legit)
          ->add_option(WXOR(L"Cheater"), &global->playerlist_data.mark_as_cheater);

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
    load_config();

    vec3i menu_pos = render->screen_size / 2;

    gcs_component_window* window = create_window(this, WXOR(L"RijiN For Left 4 Dead 2"), menu_pos);
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
    return HASH("L4D2");
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