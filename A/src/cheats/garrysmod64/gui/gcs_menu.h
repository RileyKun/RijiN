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
            ->set_render_on(&config->aimbot.enabled)
            ->set_help_text(WXOR(L"Note: Serverside is patched for most weapons due to sv_maxusrcmdprocessticks_holdaim.\nWe recommend using not using silent aim if you wish to look more legit."));

        gcs::dropdown(aimbot_group, WXOR(L"Position adjustment"), true)
          ->add_option(WXOR(L"History"), &config->aimbot.position_adjust_history)
          ->add_option(WXOR(L"Accurate unlag"), &config->aimbot.position_adjust_accurate_unlag)
          ->set_render_on(&config->aimbot.enabled);
      }

      // Targets
      {     
        gcs::dropdown(target_group, WXOR(L"Ignore Flags"), true)
          ->add_option(WXOR(L"Team"), &config->aimbot.ignore_team_mates)
          ->add_option(WXOR(L"Friends"), &config->aimbot.ignore_friends)
          ->add_option(WXOR(L"Demi-god"), &config->aimbot.ignore_demi_god)
          ->add_option(WXOR(L"Noclipping"), &config->aimbot.ignore_noclipping)
          ->add_option(WXOR(L"In vehicle"), &config->aimbot.ignore_vehicle)
          ->add_option(WXOR(L"Godmode"), &config->aimbot.ignore_god_mode)
          ->add_option(WXOR(L"Staff"), &config->aimbot.ignore_staff);

        gcs::dropdown(target_group, WXOR(L"Algorithm"), false)
          ->add_option(WXOR(L"Closest to FOV"), &config->aimbot.target_method_closest_to_fov)
          ->add_option(WXOR(L"Distance"), &config->aimbot.target_method_distance);

        gcs::dropdown(target_group, WXOR(L"Priority hitbox"), false)
          ->add_option(WXOR(L"Head"), &config->aimbot.priority_hitbox[1])
          ->add_option(WXOR(L"Chest"), &config->aimbot.priority_hitbox[2])
          ->add_option(WXOR(L"Stomach"), &config->aimbot.priority_hitbox[3]);

        gcs::dropdown(target_group, WXOR(L"Hitscan"), true)
          ->add_option(WXOR(L"Head"), &config->aimbot.hitbox[1])
          ->add_option(WXOR(L"Chest"), &config->aimbot.hitbox[2])
          ->add_option(WXOR(L"Stomach"), &config->aimbot.hitbox[3]);
      }

      // Other
      {
        gcs::toggle(other_group, WXOR(L"No spread"), &config->aimbot.no_spread);
        gcs::toggle(other_group, WXOR(L"No recoil"), &config->aimbot.no_recoil);

        gcs::toggle(other_group, WXOR(L"Auto wall"), &config->aimbot.autowall_enabled);
        gcs::slider(other_group, WXOR(L"Minimum damage"), WXOR(L"%"), 1, 90, GCS_SLIDER_TYPE_FLOAT, &config->aimbot.autowall_min_dmg)
          ->set_render_on(&config->aimbot.autowall_enabled);

        gcs::toggle(other_group, WXOR(L"Trace through vehicles"), &config->aimbot.trace_filter_skip_vehicle)
          ->set_help_text(WXOR(L"If the player is inside a vehicle it will ignore the vehicle entity.\nThis won't guarantee damage towards the player though with your weapon."));
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
      gcs::slider(comp, WXOR(L"Render dist"), WXOR(L"m"), 1, 1000, GCS_SLIDER_TYPE_INT, &esp_settings->max_distance)
        ->set_render_on(&esp_settings->enabled);

      if(config_idx == PLAYER_ESP_SETTING_TYPE_FRIEND){
        gcs::toggle(comp, WXOR(L"Render friend text"), &esp_settings->friend_tag)
          ->set_render_on(&esp_settings->enabled);

        gcs::toggle(comp, WXOR(L"Show localplayer"), &esp_settings->show_localplayer)
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

      gcs::toggle(comp, WXOR(L"Flags"), &esp_settings->flags)
      ->set_render_on(&esp_settings->enabled);
      {
        gcs::dropdown(comp, WXOR(L"Flags list"), true)
          ->add_option(WXOR(L"God mode"), &esp_settings->flag_god_mode)
          ->add_option(WXOR(L"Armor"), &esp_settings->flag_armor)
          ->add_option(WXOR(L"Noclipped"), &esp_settings->flag_noclipped)
          ->add_option(WXOR(L"In vehicle"), &esp_settings->flag_in_vehicle)
          ->add_option(WXOR(L"On fire"), &esp_settings->flag_on_fire)
          ->add_option(WXOR(L"Frozen"), &esp_settings->flag_frozen)
          ->add_option(WXOR(L"Grabbed"), &esp_settings->flag_grabbed)
          ->add_option(WXOR(L"Demi-god"), &esp_settings->flag_demi_god)
          ->add_option(WXOR(L"Staff"), &esp_settings->flag_staff)
          ->add_option(WXOR(L"Hit detection"), &esp_settings->flag_hitboxes)
            ->set_render_on(&esp_settings->enabled)
            ->set_render_on(&esp_settings->flags);
      }
      
      gcs::toggle(comp, WXOR(L"Health bar"), &esp_settings->health_bar)
        ->set_render_on(&esp_settings->enabled);

      gcs::slider(comp, WXOR(L"Health fractions"), 0, 10, GCS_SLIDER_TYPE_INT, &esp_settings->health_bar_fractions)
        ->set_render_on(&esp_settings->health_bar)
        ->set_render_on(&esp_settings->enabled);

      gcs::toggle(comp, WXOR(L"Armor bar"), &esp_settings->armor_bar)
        ->set_render_on(&esp_settings->enabled);

      gcs::slider(comp, WXOR(L"Armor fractions"), 0, 10, GCS_SLIDER_TYPE_INT, &esp_settings->armor_bar_fractions)
        ->set_render_on(&esp_settings->health_bar)
        ->set_render_on(&esp_settings->enabled);

      gcs::toggle(comp, WXOR(L"Cheater alerts"), &esp_settings->cheater_alert_tag)
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
      gcs::slider(comp, WXOR(L"Render dist"), WXOR(L"m"), 1, 1000, GCS_SLIDER_TYPE_INT, &chams_settings->max_distance)
        ->set_render_on(&chams_settings->enabled);

      if(config_idx == PLAYER_ESP_SETTING_TYPE_FRIEND){
        gcs::toggle(comp, WXOR(L"Show localplayer"), &chams_settings->show_localplayer)
          ->set_render_on(&chams_settings->enabled);
      }

      gcs::toggle(comp, WXOR(L"Ignore z"), &chams_settings->ignore_z)
        ->set_render_on(&chams_settings->enabled);

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
    }

    return nullptr;
  }

  gcs_component_group_host* create_visuals_main(gcs_component_base* comp, vec3i pos, vec3i size){
   assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"ESP/Main"), pos, size);
    {
      assert(group_host != nullptr);

      gcs_component_group* player_esp_group     = group_host->new_group(WXOR(L"Player esp"));
      gcs_component_group* player_enemy_group   = player_esp_group->new_category(WXOR(L"Any"));
      gcs_component_group* player_friend_group  = player_esp_group->new_category(WXOR(L"Friend"));
      gcs_component_group* player_colours_group = player_esp_group->new_category(WXOR(L"Colours"));

      gcs_component_group* world_esp_group      = group_host->new_group(WXOR(L"World esp"));
      gcs_component_group* world_entities_group = world_esp_group->new_category(WXOR(L"Entities"));
      gcs_component_group* world_colours_group  = world_esp_group->new_category(WXOR(L"Colours"));

      // Player ESP
      {
        create_player_esp_tabs(player_enemy_group, PLAYER_ESP_SETTING_TYPE_NORMAL);
        create_player_esp_tabs(player_friend_group, PLAYER_ESP_SETTING_TYPE_FRIEND);
        // Colours
        {

          gcs::label(player_colours_group, WXOR(L"Player"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.player_esp_colour);

          gcs::label(player_colours_group, WXOR(L"Friends"))
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.player_esp_friend_colour);

          // Aimbot colour
          gcs::toggle(player_colours_group, WXOR(L"Aimbot target"), &config->visual.player_esp_aimbot_target)
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.player_esp_aimbot_target_colour);
        }
      }

      // World ESP
      {
        gcs::toggle(world_entities_group, WXOR(L"Enabled"), &config->world_esp.object_enabled);
        gcs::slider(world_entities_group, WXOR(L"Render dist"), WXOR(L"m"), 1, 1000, GCS_SLIDER_TYPE_INT, &config->world_esp.max_distance)
            ->set_render_on(&config->world_esp.object_enabled);
        gcs::dropdown(world_entities_group, WXOR(L"Components"), true)
          ->add_option(WXOR(L"Box"), &config->world_esp.box)
          ->add_option(WXOR(L"Box outline"), &config->world_esp.box_outlines)
          ->add_option(WXOR(L"Name"), &config->world_esp.name)
          ->add_option(WXOR(L"Distance"), &config->world_esp.distance)
            ->set_render_on(&config->world_esp.object_enabled);

        {
          gcs::label(world_colours_group, WXOR(L"Box"))
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->world_esp.esp_colour);

          gcs::toggle(world_colours_group, WXOR(L"Aimbot target"), &config->world_esp.aimbot_target)
            ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->world_esp.esp_aimbot_colour)
              ->set_render_on(&config->world_esp.aimbot_target);
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

        gcs::add_space(other_group);
        gcs::toggle(other_group, WXOR(L"Cheap text rendering"), &config->visual.cheap_text_rendering);
        gcs::toggle(other_group, WXOR(L"No visual recoil"), &config->visual.no_visual_recoil);
        gcs::toggle(other_group, WXOR(L"Show information panel"), &config->visual.show_information);

        gcs::add_space(other_group);
        gcs::toggle(other_group, WXOR(L"FOV circle"), &config->visual.aimbot_fov_circle_enabled);
        gcs::toggle(other_group, WXOR(L"FOV circle outlines"), &config->visual.aimbot_fov_circle_outlines)
          ->set_render_on(&config->visual.aimbot_fov_circle_enabled);

        gcs::label(other_group, WXOR(L"FOV circle colour"))
          ->set_render_on(&config->visual.aimbot_fov_circle_enabled)
            ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.aimbot_fov_circle_colour);

        gcs::add_space(other_group);
        gcs::toggle(other_group, WXOR(L"Target visual prediction"), &config->visual.aimbot_target_visual_pred)
          ->set_help_text(WXOR(L"This feature predicts the next aimbot target and highlights the player in advance so you know who you're targetting.\nThis will mainly affect the esp and cham colours for \"aimbot target\" options\n\nNOTICE: THIS MAY REDUCE PERFORMANCE!"));

        gcs::add_space(other_group);
        gcs::toggle(other_group, WXOR(L"Thirdperson"), &config->visual.thirdperson)
          ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&config->visual.thirdperson_key)
          ->set_render_on(&config->visual.thirdperson)
          ->set_render_on(&config->visual.thirdperson_usekey);
        {
          gcs::toggle(other_group, WXOR(L"Use key"), &config->visual.thirdperson_usekey)
            ->set_render_on(&config->visual.thirdperson);

          gcs::dropdown(other_group, WXOR(L"Key control"), false)
            ->add_option(WXOR(L"Held"), &config->visual.thirdperson_key_held)
            ->add_option(WXOR(L"Toggle"), &config->visual.thirdperson_key_toggle)
            ->add_option(WXOR(L"Double click"), &config->visual.thirdperson_key_double_click)
            ->set_render_on(&config->visual.thirdperson)
            ->set_render_on(&config->visual.thirdperson_usekey);

          gcs::slider(other_group, WXOR(L"Max distance"), WXOR(L"hu"), 30, 400, GCS_SLIDER_TYPE_INT, &config->visual.thirdperson_distance)
            ->set_render_on(&config->visual.thirdperson);

          gcs::slider(other_group, WXOR(L"Up offset"), WXOR(L"hu"), -64, 64, GCS_SLIDER_TYPE_INT, &config->visual.thirdperson_up_offset)
            ->set_render_on(&config->visual.thirdperson);

          gcs::toggle(other_group, WXOR(L"Right offset"), &config->visual.thirdperson_right_offset_enabled)
            ->set_render_on(&config->visual.thirdperson)
            ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&config->visual.thirdperson_right_offset_key)
            ->set_render_on(&config->visual.thirdperson_right_offset_enabled)
            ->set_render_on(&config->visual.thirdperson_right_offset_use_key);

          gcs::toggle(other_group, WXOR(L"Use key"), &config->visual.thirdperson_right_offset_use_key)
            ->set_render_on(&config->visual.thirdperson)
            ->set_render_on(&config->visual.thirdperson_right_offset_enabled);

            gcs::dropdown(other_group, WXOR(L"Key control"), false)
            ->add_option(WXOR(L"Held"), &config->visual.thirdperson_right_offset_held)
            ->add_option(WXOR(L"Toggle"), &config->visual.thirdperson_right_offset_toggle)
            ->add_option(WXOR(L"Double click"), &config->visual.thirdperson_right_offset_double_click)
            ->set_render_on(&config->visual.thirdperson)
            ->set_render_on(&config->visual.thirdperson_right_offset_enabled);

          gcs::label(other_group, WXOR(L"Invert offset key"))
            ->set_render_on(&config->visual.thirdperson)
            ->set_render_on(&config->visual.thirdperson_right_offset_enabled)
              ->add_module<gcs_component_hotkey>()
                ->set_hotkey_ptr(&config->visual.thirdperson_right_offset_invert_key);

          gcs::dropdown(other_group, WXOR(L"Invert key control"), false)
            ->add_option(WXOR(L"Held"), &config->visual.thirdperson_right_offset_invert_held)
            ->add_option(WXOR(L"Toggle"), &config->visual.thirdperson_right_offset_invert_toggle)
            ->add_option(WXOR(L"Double click"), &config->visual.thirdperson_right_offset_invert_double_click)
            ->set_render_on(&config->visual.thirdperson)
            ->set_render_on(&config->visual.thirdperson_right_offset_enabled);

          gcs::slider(other_group, WXOR(L"Offset"), WXOR(L"hu"), -64, 64, GCS_SLIDER_TYPE_INT, &config->visual.thirdperson_right_offset)
            ->set_render_on(&config->visual.thirdperson)
            ->set_render_on(&config->visual.thirdperson_right_offset_enabled);
        }


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
      gcs_component_group* player_chams_enemy_group   = player_chams_group->new_category(WXOR(L"Any"));
      gcs_component_group* player_chams_friend_group  = player_chams_group->new_category(WXOR(L"Friend"));
      gcs_component_group* player_chams_colours_group = player_chams_group->new_category(WXOR(L"Colours"));

      gcs_component_group* world_chams_group          = group_host->new_group(WXOR(L"World chams"));
      gcs_component_group* world_chams_settings_group = world_chams_group->new_category(WXOR(L"Settings"));
      gcs_component_group* world_chams_colours_group  = world_chams_group->new_category(WXOR(L"Colours"));

      gcs_component_group* glow_group          = group_host->new_group(WXOR(L"Glow"));
      gcs_component_group* entities_glow_group = glow_group->new_category(WXOR(L"Entities"));
      gcs_component_group* settings_glow_group = glow_group->new_category(WXOR(L"Settings"));
      gcs_component_group* colours_glow_group  = glow_group->new_category(WXOR(L"Colours"));

      group_host->update();
      // Player chams
      {

        create_player_chams_tabs(player_chams_enemy_group, PLAYER_ESP_SETTING_TYPE_NORMAL);
        create_player_chams_tabs(player_chams_friend_group,PLAYER_ESP_SETTING_TYPE_FRIEND);

        // Colours
        {
      
          gcs::label(player_chams_colours_group, WXOR(L"Player"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
            ->set_output_colour_ptr(config->visual.player_chams_colour);

          gcs::label(player_chams_colours_group, WXOR(L"Overlay player"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
            ->set_output_colour_ptr(config->visual.player_chams_overlay_colour);

          gcs::label(player_chams_colours_group, WXOR(L"Friends"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
            ->set_output_colour_ptr(config->visual.player_chams_friend_colour);

          gcs::label(player_chams_colours_group, WXOR(L"Overlay Friends"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
            ->set_output_colour_ptr(config->visual.player_chams_overlay_friend_colour);
           
          // Aimbot colour
          gcs::toggle(player_chams_colours_group, WXOR(L"Aimbot target"), &config->visual.player_chams_aimbot_target)
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
            ->set_output_colour_ptr(config->visual.player_chams_aimbot_target_colour);

        }
      }

      // World chams
      {
        gcs::toggle(world_chams_settings_group, WXOR(L"Ignore z"), &config->world_chams.ignore_z);
        gcs::slider(world_chams_settings_group, WXOR(L"Render dist"), WXOR(L"m"), 1, 1000, GCS_SLIDER_TYPE_INT, &config->world_chams.max_distance);
          gcs::dropdown(world_chams_settings_group, WXOR(L"Base material"), false)
            ->add_option(WXOR(L"None"), &config->world_chams.base_material_none)
            ->add_option(WXOR(L"Shaded"), &config->world_chams.base_material_shaded)
            ->add_option(WXOR(L"Flat"), &config->world_chams.base_material_flat);

          gcs::dropdown(world_chams_settings_group, WXOR(L"Overlay material"), false)
            ->add_option(WXOR(L"None"), &config->world_chams.overlay_material_none)
            ->add_option(WXOR(L"Flat"), &config->world_chams.overlay_material_flat)
            ->add_option(WXOR(L"Fresnel"), &config->world_chams.overlay_material_fresnel);

          gcs::label(world_chams_colours_group, WXOR(L"Base colour"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->world_chams.chams_colour_base);

          gcs::label(world_chams_colours_group, WXOR(L"Overlay colour"))
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
              ->set_output_colour_ptr(config->world_chams.chams_colour_overlay);

          gcs::toggle(world_chams_colours_group, WXOR(L"Aimbot target base"), &config->world_chams.aimbot_target)
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
            ->set_output_colour_ptr(config->world_chams.chams_aimbot_base_colour)
              ->set_render_on(&config->world_chams.aimbot_target);

          gcs::toggle(world_chams_colours_group, WXOR(L"Aimbot target overlay"), &config->world_chams.aimbot_target_overlay)
            ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
            ->set_output_colour_ptr(config->world_chams.chams_aimbot_overlay_colour)
              ->set_render_on(&config->world_chams.aimbot_target_overlay);
      }

      // Glow
      {
        gcs::toggle(entities_glow_group, WXOR(L"Enabled"), &config->visual.glow_enabled);

        // Players
        gcs::toggle(entities_glow_group, WXOR(L"Players"), &config->visual.player_glow_enabled)
           ->set_render_on(&config->visual.glow_enabled);

        gcs::slider(entities_glow_group, WXOR(L"Player render dist"), WXOR(L"m"), 1, 1000, GCS_SLIDER_TYPE_INT, &config->visual.player_glow_max_distance)
          ->set_render_on(&config->visual.player_glow_enabled)
          ->set_render_on(&config->visual.glow_enabled);
        {
          gcs::dropdown(entities_glow_group, WXOR(L"Show"), true)
            ->add_option(WXOR(L"Friends"), &config->visual.player_glow_render_steamfriends)
            ->add_option(WXOR(L"Localplayer"), &config->visual.player_glow_render_localplayer)
            ->set_render_on(&config->visual.glow_enabled)
            ->set_render_on(&config->visual.player_glow_enabled);
        }

        // World settings?! L0L
        {
          gcs::slider(entities_glow_group, WXOR(L"World render dist"), WXOR(L"m"), 1, 1000, GCS_SLIDER_TYPE_INT, &config->world_glow.max_distance)
            ->set_render_on(&config->visual.glow_enabled);
        }

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

            gcs::label(colours_glow_group, WXOR(L"Player"))
              ->set_render_on(&config->visual.player_glow_use_team_colours)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.player_glow_colour);
       
            gcs::label(colours_glow_group, WXOR(L"Friends"))
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.player_glow_friend_colour);

            gcs::toggle(colours_glow_group, WXOR(L"Aimbot target"), &config->visual.player_glow_aimbot_target)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->visual.player_glow_aimbot_target_colour)
                ->set_render_on(&config->visual.player_glow_aimbot_target);

            gcs::add_space(colours_glow_group);
          }

          // World colours
          {
            gcs::label(colours_glow_group, WXOR(L"World entities"))
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->world_glow.glow_colour);

            gcs::toggle(colours_glow_group, WXOR(L"Aimbot target"), &config->world_glow.aimbot_target)
              ->add_module<gcs_component_colour_picker>()
              ->set_output_colour_ptr(config->world_glow.glow_aimbot_colour)
                ->set_render_on(&config->world_glow.aimbot_target);
          }
        }
      }
    }
    return group_host;
  }

  gcs_component_group_host* create_hvh_antiaim(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Anti-aim"), pos, size);
    {
      gcs_component_group* g1                  = group_host->new_group(L"General");

      gcs_component_group* antiaim_group_main = g1->new_category(WXOR(L"General"));
      {
        gcs::dropdown(antiaim_group_main, WXOR(L"Style"), false)
          ->add_option(WXOR(L"Legit"), &config->hvh.antiaim_legit)
          ->add_option(WXOR(L"Rage"), &config->hvh.antiaim_rage)
          ->set_help_text(WXOR(L"Legit style will not stop anti-cheat detections.\nSo use carefully."));

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
          //gcs::dropdown(antiaim_group_pitch, WXOR(L"Fake"), false)
          //  ->add_option(WXOR(L"None"), &config->hvh.pitch_fake_none)
          //  ->add_option(WXOR(L"Up"), &config->hvh.pitch_fake_up)
          //  ->add_option(WXOR(L"Down"), &config->hvh.pitch_fake_down)
          //  ->set_render_on(&config->hvh.antiaim_pitch)
          //  ->set_render_on(&config->hvh.antiaim_rage)
          //  ->set_render_on(&config->hvh.antiaim_pitch_static);

          gcs::dropdown(antiaim_group_pitch, WXOR(L"Angle"), false)
            ->add_option(WXOR(L"Up"), &config->hvh.pitch_real_up)
            ->add_option(WXOR(L"Zero"), &config->hvh.pitch_real_zero)
            ->add_option(WXOR(L"Down"), &config->hvh.pitch_real_down)
            ->set_render_on(&config->hvh.antiaim_pitch)
            ->set_render_on(&config->hvh.antiaim_rage)
            ->set_render_on(&config->hvh.antiaim_pitch_static);
           
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
            ->set_render_on(&config->hvh.yaw_auto_edge_enabled)
            ->set_render_on(&config->hvh.antiaim_yaw);
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

      gcs_component_group* g2 = group_host->new_group(WXOR(L"Misc"));
      {
        gcs_component_group* fakelag_misc = g2->new_category(WXOR(L"Misc"));
        {
          gcs::toggle(fakelag_misc, WXOR(L"Anti-backtrack"), &config->hvh.fakelag_anti_backtrack);
          gcs::toggle(fakelag_misc, WXOR(L"Peek fakelag"), &config->hvh.fakelag_peek);

          gcs::add_space(fakelag_misc);

          gcs::dropdown(fakelag_misc, WXOR(L"Break jump"), false)
            ->add_option(WXOR(L"Disabled"), &config->hvh.break_animations_disabled)
            ->add_option(WXOR(L"Constant"), &config->hvh.break_animations_constant)
            ->add_option(WXOR(L"First jump"), &config->hvh.break_animations_on_jump);

          #if defined(DEV_MODE)
          gcs::dropdown(fakelag_misc, WXOR(L"Fake crouch"), false)
            ->add_option(WXOR(L"Disabled"), &config->hvh.fakeduck_disabled)
            ->add_option(WXOR(L"On crouch"), &config->hvh.fakeduck_onduck)
            ->add_option(WXOR(L"On key"), &config->hvh.fakeduck_on_key)
            ->add_option(WXOR(L"Always"), &config->hvh.fakeduck_always)
               ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&config->hvh.fakeduck_key)
            ->set_render_on(&config->hvh.fakeduck_on_key);
          #endif

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
          ->add_option(WXOR(L"Cheaters only"), &config->hvh.resolver_cheatersonly);
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
      gcs_component_group* general_group = group_host->new_group(WXOR(L"General"));
      gcs_component_group* other_group   = group_host->new_group(WXOR(L"Protections"));
      group_host->update();

      // General
      {
        gcs::toggle(general_group, WXOR(L"Bunny hop"), &config->misc.bhop);
        gcs::toggle(general_group, WXOR(L"Auto strafe"), &config->misc.auto_strafe);
        {
          gcs::toggle(general_group, WXOR(L"Prespeed"), &config->misc.auto_strafe_prespeed)
            ->set_render_on(&config->misc.auto_strafe);

          gcs::dropdown(general_group, WXOR(L"Strafe type"), false)
            ->add_option(WXOR(L"Normal"), &config->misc.auto_strafe_type_normal)
            ->add_option(WXOR(L"Directional (WASD)"), &config->misc.auto_strafe_type_directional)
              ->set_render_on(&config->misc.auto_strafe);
          gcs::dropdown(general_group, WXOR(L"Avoid walls"), false)
            ->add_option(WXOR(L"Disabled"), &config->misc.auto_strafe_avoid_walls_disabled)
            ->add_option(WXOR(L"Simple"), &config->misc.auto_strafe_avoid_walls_normal)
            ->add_option(WXOR(L"Advanced"), &config->misc.auto_strafe_avoid_walls_adv)
              ->set_render_on(&config->misc.auto_strafe);

          gcs::slider(general_group, WXOR(L"Retrack speed"), WXOR(L"%"), 1, 100, GCS_SLIDER_TYPE_FLOAT, &config->misc.auto_strafe_retrack)
            ->set_render_on(&config->misc.auto_strafe)
            ->set_render_on(&config->misc.auto_strafe_type_directional);
        }

        gcs::toggle(general_group, WXOR(L"Free camera"), &config->misc.freecam)
          ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&config->misc.freecam_key)
          ->set_render_on(&config->misc.freecam);
        {
          gcs::slider(general_group, WXOR(L"Speed"), L"", 5, 15, GCS_SLIDER_TYPE_INT, &config->misc.freecam_speed)
            ->set_render_on(&config->misc.freecam);

          gcs::dropdown(general_group, WXOR(L"Key control"), false)
            ->add_option(WXOR(L"Held"), &config->misc.freecam_key_held)
            ->add_option(WXOR(L"Toggle"), &config->misc.freecam_key_toggle)
            ->add_option(WXOR(L"Double click"), &config->misc.freecam_key_double_click)
            ->set_render_on(&config->misc.freecam);
        }

        gcs::toggle(general_group, WXOR(L"Auto pistol"), &config->misc.auto_pistol)
          ->set_help_text(WXOR(L"Will make semi-auto weapons fire automatically."));
       
        gcs::toggle(general_group, WXOR(L"Custom server integrations"), &config->misc.custom_server_integrations);

        gcs::toggle(general_group, WXOR(L"Setupbones optimisations"), &config->misc.setupbones_optimisations)
          ->set_help_text(WXOR(L"Greatly optimises how and when the players rig bones are set up\n\nWILL RESULT IN MORE FPS, BUT THIS MAY MAKE PLAYERS APPEAR LAGGY!"));
      }

      // Other
      {
        gcs::toggle(other_group, WXOR(L"No silent aim"), &config->misc.no_silent_aim)
          ->set_help_text(WXOR(L"Silent aimbot is a rage feature. Disabling it can reduce the chances of getting detected by anti-cheats."));

        gcs::toggle(other_group, WXOR(L"No movement fix"), &config->misc.no_movement_fix)
          ->set_help_text(WXOR(L"Recommended against anti-cheats that can detect this feature."));

        gcs::toggle(other_group, WXOR(L"Auto disable HvH"), &config->misc.auto_disable_hvh_on_leave)
          ->set_help_text(WXOR(L"When you leave or join a server it'll automatically disable hvh features."));

        gcs::dropdown(other_group, WXOR(L"Anti-screengrab"), false)
          ->add_option(WXOR(L"Always"), &config->misc.anti_screen_grab_always)
          ->add_option(WXOR(L"Ignore 2D camera"), &config->misc.anti_screen_grab_dont_capture_2dcam)
          ->add_option(WXOR(L"Disabled"), &config->misc.anti_screen_grab_disabled)
          ->set_help_text(WXOR(L"Ignore 2D camera: This will disable Start2D related visuals. This can result in failing screen-grab checks.\nDisabled: This will disable anti-screen-grab along with features that require it."));
      
        gcs::toggle(other_group, WXOR(L"Voicerecord protect"), &config->misc.voicerecord_protect)
          ->set_help_text(WXOR(L"This feature works by blocking voice packets unless X, B OR N key is held.\nSome servers have plugins that prompt you to enable automatic enabling of +voicerecord\nThis can be used maliciously to enable your microphone on demand.\nThis feature will prevent this from happening while allowing you to use plugins like radios etc."));
      
        gcs::toggle(other_group, WXOR(L"Demo protect"), &config->misc.demo_protect)
          ->set_help_text(WXOR(L"This feature enables various demo protection features, Some servers require you to run plugins that record and send demos.\nThis will overwrite the packets in the demos to hide aimbot and anti aim."));
      }
    }

    return group_host;
  }

  gcs_component_canvas* create_entity_list(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);
    auto objects = gcs::fetch_playerlist_settings_comp(comp, WXOR(L"Entity list"), WXOR(L"Entity list settings"), pos, size);
    gcs_component_canvas*     main_canvas = objects.second;
    gcs_component_group_host* group_host  = objects.first;
    {
      gcs_component_group* group    = group_host->new_group(L"Entity Settings");
      gcs_component_group* settings = group->new_category(WXOR(L"Settings"));
      {
        gcs::toggle(settings, WXOR(L"Render"), &entity_list_data->selected_item.entity_data.render);

        gcs::toggle(settings, WXOR(L"Use custom settings"), &entity_list_data->selected_item.entity_data.use_custom_settings)
          ->set_render_on(&entity_list_data->selected_item.entity_data.render);

        gcs::toggle(settings, WXOR(L"Aimbot through this entity"), &entity_list_data->selected_item.entity_data.filter_from_aimbot_vis_check);
        gcs::toggle(settings, WXOR(L"Aimbot this entity"), &entity_list_data->selected_item.entity_data.aimbot_entity);
      }
      gcs_component_group* esp_group = group->new_category(WXOR(L"ESP"));
      {
        gcs::toggle(esp_group, WXOR(L"Enabled"), &entity_list_data->selected_item.entity_data.esp_settings.object_enabled);
        gcs::slider(esp_group, WXOR(L"Render dist"), WXOR(L"m"), 1, 1000, GCS_SLIDER_TYPE_INT, &entity_list_data->selected_item.entity_data.esp_settings.max_distance)
          ->set_render_on(&entity_list_data->selected_item.entity_data.esp_settings.object_enabled);

        gcs::dropdown(esp_group, WXOR(L"Components"), true)
          ->add_option(WXOR(L"Box"), &entity_list_data->selected_item.entity_data.esp_settings.box)
          ->add_option(WXOR(L"Box outline"), &entity_list_data->selected_item.entity_data.esp_settings.box_outlines)
          ->add_option(WXOR(L"Name"), &entity_list_data->selected_item.entity_data.esp_settings.name)
          ->add_option(WXOR(L"Distance"), &entity_list_data->selected_item.entity_data.esp_settings.distance)
             ->set_render_on(&entity_list_data->selected_item.entity_data.esp_settings.object_enabled);

        gcs::label(esp_group, WXOR(L"Box Colour"))
          ->add_module<gcs_component_colour_picker>()
          ->set_output_colour_ptr(entity_list_data->selected_item.entity_data.esp_settings.esp_colour);

        gcs::toggle(esp_group, WXOR(L"Aimbot target"), &entity_list_data->selected_item.entity_data.esp_settings.aimbot_target)
          ->add_module<gcs_component_colour_picker>()
          ->set_output_colour_ptr(entity_list_data->selected_item.entity_data.esp_settings.esp_aimbot_colour)
            ->set_render_on(&entity_list_data->selected_item.entity_data.esp_settings.aimbot_target);
      }
      gcs_component_group* chams_group = group->new_category(WXOR(L"Chams"));
      {
        gcs::toggle(chams_group, WXOR(L"Enabled"), &entity_list_data->selected_item.entity_data.chams_settings.object_enabled);
        gcs::slider(chams_group, WXOR(L"Render dist"), WXOR(L"m"), 1, 1000, GCS_SLIDER_TYPE_INT, &entity_list_data->selected_item.entity_data.chams_settings.max_distance)
          ->set_render_on(&entity_list_data->selected_item.entity_data.chams_settings.object_enabled);
          
        gcs::toggle(chams_group, WXOR(L"Ignore z"), &entity_list_data->selected_item.entity_data.chams_settings.ignore_z);
        gcs::dropdown(chams_group, WXOR(L"Base material"), false)
          ->add_option(WXOR(L"None"), &entity_list_data->selected_item.entity_data.chams_settings.base_material_none)
          ->add_option(WXOR(L"Shaded"), &entity_list_data->selected_item.entity_data.chams_settings.base_material_shaded)
          ->add_option(WXOR(L"Flat"), &entity_list_data->selected_item.entity_data.chams_settings.base_material_flat);

        gcs::dropdown(chams_group, WXOR(L"Overlay material"), false)
          ->add_option(WXOR(L"None"), &entity_list_data->selected_item.entity_data.chams_settings.overlay_material_none)
          ->add_option(WXOR(L"Flat"), &entity_list_data->selected_item.entity_data.chams_settings.overlay_material_flat)
          ->add_option(WXOR(L"Fresnel"), &entity_list_data->selected_item.entity_data.chams_settings.overlay_material_fresnel);

        gcs::label(chams_group, WXOR(L"Base colour"))
          ->add_module<gcs_component_colour_picker>()
          ->enable_alpha_bar()
            ->set_output_colour_ptr(entity_list_data->selected_item.entity_data.chams_settings.chams_colour_base);

        gcs::label(chams_group, WXOR(L"Overlay colour"))
          ->add_module<gcs_component_colour_picker>()
          ->enable_alpha_bar()
            ->set_output_colour_ptr(entity_list_data->selected_item.entity_data.chams_settings.chams_colour_overlay);

        gcs::toggle(chams_group, WXOR(L"Aimbot target Base"), &entity_list_data->selected_item.entity_data.chams_settings.aimbot_target)
          ->add_module<gcs_component_colour_picker>()
          ->set_output_colour_ptr(entity_list_data->selected_item.entity_data.chams_settings.chams_aimbot_base_colour)
            ->set_render_on(&entity_list_data->selected_item.entity_data.chams_settings.aimbot_target);

        gcs::toggle(chams_group, WXOR(L"Aimbot target Overlay"), &entity_list_data->selected_item.entity_data.chams_settings.aimbot_target)
          ->add_module<gcs_component_colour_picker>()
          ->set_output_colour_ptr(entity_list_data->selected_item.entity_data.chams_settings.chams_aimbot_overlay_colour)
            ->set_render_on(&entity_list_data->selected_item.entity_data.chams_settings.aimbot_target);
      }
      gcs_component_group* glow_group = group->new_category(WXOR(L"Glow"));
      {
        gcs::toggle(glow_group, WXOR(L"Enabled"), &entity_list_data->selected_item.entity_data.glow_settings.object_enabled);
        gcs::slider(glow_group, WXOR(L"Render dist"), WXOR(L"m"), 1, 1000, GCS_SLIDER_TYPE_INT, &entity_list_data->selected_item.entity_data.glow_settings.max_distance)
          ->set_render_on(&entity_list_data->selected_item.entity_data.glow_settings.object_enabled);

        gcs::label(glow_group, WXOR(L"Glow colour"))
          ->add_module<gcs_component_colour_picker>()
          ->set_output_colour_ptr(entity_list_data->selected_item.entity_data.glow_settings.glow_colour);

        gcs::toggle(glow_group, WXOR(L"Aimbot target"), &entity_list_data->selected_item.entity_data.glow_settings.aimbot_target)
          ->add_module<gcs_component_colour_picker>()
          ->set_output_colour_ptr(entity_list_data->selected_item.entity_data.glow_settings.glow_aimbot_colour)
            ->set_render_on(&entity_list_data->selected_item.entity_data.glow_settings.aimbot_target);
      }
  
      group_host->update();
    }
    
    gcs_component_canvas* canvas2 = gcs::fetch_playerlist_window_comp<gcs_component_entity_list>(main_canvas, WXOR(L"Entity List Window"), size);
    {
      canvas2->add_hook(GCS_HOOK_POST_PAINT, [](gcs_component_base* c, void* p){
        colour background_transparent = c->scheme()->grey3;
        background_transparent.a = 255.f * 0.5f;

        render->outlined_rect(c->pos(true), c->size(), background_transparent);

        entity_list_data->sync_selected_item_entity_data();
        return true;
      });
    }

    return main_canvas;
  }

  gcs_component_canvas* create_player_list(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);
    auto objects = gcs::fetch_playerlist_settings_comp(comp, WXOR(L"Player list"), WXOR(L"Player list settings"), pos, size);
    gcs_component_canvas*     main_canvas = objects.second;
    gcs_component_group_host* group_host  = objects.first;
    {
      if(!global->playerlist_data.valid)
        global->playerlist_data.init();

      gcs_component_group* group      = group_host->new_group(WXOR(L"Settings"));
      gcs_component_group* settings = group->new_category(WXOR(L"Settings"));
      {
        gcs::toggle(settings, WXOR(L"Friend"), &global->playerlist_data.mark_as_friend);
        gcs::toggle(settings, WXOR(L"Ignore on aimbot"), &global->playerlist_data.ignore_player);
        gcs::dropdown(settings, WXOR(L"Flags"), false)
          ->add_option(WXOR(L"Auto"), &global->playerlist_data.mark_as_auto)
          ->add_option(WXOR(L"Cheater"), &global->playerlist_data.mark_as_cheater);

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
      gcs_component_group* colour_settings = group->new_category(WXOR(L"Colours"));
      {
        gcs::toggle(colour_settings, WXOR(L"ESP"), &global->playerlist_data.custom_esp_colour)
          ->add_module<gcs_component_colour_picker>()
          ->set_output_colour_ptr(global->playerlist_data.esp_colour)
            ->set_render_on(&global->playerlist_data.custom_esp_colour);

        gcs::toggle(colour_settings, WXOR(L"Base Chams"), &global->playerlist_data.custom_chams_base_colour)
          ->add_module<gcs_component_colour_picker>()
          ->set_output_colour_ptr(global->playerlist_data.chams_colour)
            ->set_render_on(&global->playerlist_data.custom_chams_base_colour);

        gcs::toggle(colour_settings, WXOR(L"Overlay Chams"), &global->playerlist_data.custom_chams_overlay_colour)
          ->add_module<gcs_component_colour_picker>()
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

    gcs_component_window* window = create_window(this, WXOR(L"RijiN For Garry's Mod (64 bit)"), menu_pos);

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

        sidebar->add_entry(WXOR(L"Hack vs Hack"))
          ->add_entry(create_hvh_antiaim(window, canvas_pos, canvas_size))
          ->add_entry(create_hvh_fakelag(window, canvas_pos, canvas_size))
          ->add_entry(create_hvh_resolver(window, canvas_pos, canvas_size));
        sidebar->add_entry(create_misc(window, canvas_pos, canvas_size));
        sidebar->add_entry(create_player_list(window, canvas_pos, canvas_size));
        sidebar->add_entry(create_entity_list(window, canvas_pos, canvas_size));

        add_cloud_configs_selection(sidebar);
      }
    }

    return true;
  }

  u32 get_game_hash() override{
    return HASH("GMOD");
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