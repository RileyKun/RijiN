#pragma once

#include "link.h"

class c_global;

class gcs_menu : public gcs_scene{
public:
  vec3i menu_size     = vec3i(600, 500);
  i32 select_width    = menu_size.x / 5;

  void handle_aimbot_class(gcs_component_group_host* group_host, c_aimbot_settings* aimbot_settings, u8 current_class, vec3i pos, vec3i size){
    assert(group_host != nullptr);
    assert(aimbot_settings != nullptr);
    assert(current_class >= 0 && current_class < HERO_MAX);

    // Calculate the current class name for aimbot
    std::string name = XOR("Aimbot - ") + (std::string)g_hero_data[current_class].name;

    gcs_component_group* aimbot_group = group_host->new_group(utils::str2wstr(name));
    gcs_component_group* smoothing_group  = group_host->new_group(WXOR(L"Smoothing"));
    gcs_component_group* trig_group  = group_host->new_group(WXOR(L"Trigger bot"));
    gcs_component_group* other_group  = group_host->new_group(WXOR(L"Other"));
    group_host->update();

    // Aimbot
    {
      gcs::toggle(aimbot_group, WXOR(L"Enabled"), &aimbot_settings->enabled)
        ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&aimbot_settings->aim_hotkey)
            ->set_render_on(&aimbot_settings->enabled);

      gcs::toggle(aimbot_group, WXOR(L"Sticky aim"), &aimbot_settings->sticky_aim)
        ->set_render_on(&aimbot_settings->enabled);

      gcs::dropdown(aimbot_group, WXOR(L"Hitboxes"), true)
        ->add_option(WXOR(L"Head"), &aimbot_settings->aim_hitbox_head)
        ->add_option(WXOR(L"Body"), &aimbot_settings->aim_hitbox_body)
        ->add_option(WXOR(L"Limbs"), &aimbot_settings->aim_hitbox_limbs)
          ->set_render_on(&aimbot_settings->enabled);

      gcs::slider(aimbot_group, WXOR(L"Field of view"), WXOR(L"deg"), 0, 100, GCS_SLIDER_TYPE_FLOAT_PRECISE, &aimbot_settings->aim_fov)
        ->set_render_on(&aimbot_settings->enabled);
      
      gcs::slider(aimbot_group, WXOR(L"Dynamic FOV"), WXOR(L"%"), 0, 1000, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->aim_dynamic_fov)
        ->set_render_on(&aimbot_settings->enabled);
      gcs::slider(aimbot_group, WXOR(L"Random Aimpoint"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->aim_random_point)
        ->set_render_on(&aimbot_settings->enabled);
      gcs::slider(aimbot_group, WXOR(L"Wait time"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->aim_wait_time)
        ->set_render_on(&aimbot_settings->enabled);
      gcs::slider(aimbot_group, WXOR(L"Flick time"), WXOR(L"ms"), 0, 1000, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->aim_autofire_time)
       ->set_render_on(&aimbot_settings->enabled);
    }
    
    // Smoothing
    {
      gcs::toggle(smoothing_group, WXOR(L"Use constant"), &aimbot_settings->aim_use_const);
        
      gcs::slider(smoothing_group, WXOR(L"Smooth factor"), WXOR(L""), 0, 50, GCS_SLIDER_TYPE_FLOAT_PRECISE, &aimbot_settings->aim_smooth);
      
      gcs::slider(smoothing_group, WXOR(L"Const factor"), WXOR(L""), 0, 10, GCS_SLIDER_TYPE_FLOAT_PRECISE, &aimbot_settings->aim_smooth_const)
        ->set_render_on(&aimbot_settings->aim_use_const);
        
      gcs::slider(smoothing_group, WXOR(L"Smooth correction"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->aim_smooth_correction);
      
      gcs::slider(smoothing_group, WXOR(L"Smooth accel"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->aim_smooth_accel);
      
      gcs::slider(smoothing_group, WXOR(L"Sine variation"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->aim_smooth_sine);
    }
    
    // Trigger
    {
      gcs::toggle(trig_group, WXOR(L"Enabled"), &aimbot_settings->trig_enabled)
        ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&aimbot_settings->trig_hotkey)
            ->set_render_on(&aimbot_settings->trig_enabled);
        
        gcs::toggle(trig_group, WXOR(L"Backtrack"), &aimbot_settings->trig_backtrack)
          ->set_render_on(&aimbot_settings->trig_enabled);
        gcs::toggle(trig_group, WXOR(L"Forwardtrack"), &aimbot_settings->trig_forwardtrack)
          ->set_render_on(&aimbot_settings->trig_enabled);
        
        gcs::toggle(trig_group, WXOR(L"Shoot objects"), &aimbot_settings->trig_shoot_objects)
          ->set_render_on(&aimbot_settings->trig_enabled);
        
        gcs::slider(trig_group, WXOR(L"Min charge"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->trig_min_charge)
          ->set_render_on(&aimbot_settings->trig_enabled);
        
        gcs::dropdown(trig_group, WXOR(L"Hitboxes"), true)
        ->add_option(WXOR(L"Head"), &aimbot_settings->trig_hitbox_head)
        ->add_option(WXOR(L"Body"), &aimbot_settings->trig_hitbox_body)
        ->add_option(WXOR(L"Limbs"), &aimbot_settings->trig_hitbox_limbs)
          ->set_render_on(&aimbot_settings->trig_enabled);
    }

    // Other
    {
      if(current_class == HERO_ROADHOG)
        gcs::slider(other_group, WXOR(L"Hook smoothing"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->roadhook_hooking_smoothing);

      if(current_class == HERO_ZEN)
        gcs::slider(other_group, WXOR(L"Auto orbs"), WXOR(L"orbs"), 1, 5, GCS_SLIDER_TYPE_INT, &aimbot_settings->zen_wait_orbs)
          ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&aimbot_settings->zen_wait_orbs_hotkey);

      gcs::toggle(other_group, WXOR(L"Position adjustment"), &aimbot_settings->position_enabled);
      
      gcs::slider(other_group, WXOR(L"Back amount"), WXOR(L"frames"), 0, 16, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->position_backtrack)
        ->set_render_on(&aimbot_settings->position_enabled);
      
      gcs::slider(other_group, WXOR(L"Forward amount"), WXOR(L"frames"), 0, 16, GCS_SLIDER_TYPE_FLOAT, &aimbot_settings->position_forwardtrack)
        ->set_render_on(&aimbot_settings->position_enabled);
      
      gcs::toggle(other_group, WXOR(L"Ghost aim"), &aimbot_settings->ghost_aim);
      
      gcs::slider(other_group, WXOR(L"Slow sens factor"), WXOR(L"%"), 0, 100, GCS_SLIDER_TYPE_FLOAT_PRECISE, &aimbot_settings->ghost_aim_amount)
        ->set_render_on(&aimbot_settings->ghost_aim);
      
      gcs::toggle(other_group, WXOR(L"Rage Mode"), &config->aimbot[0].rage_enabled)
        ->add_module<gcs_component_hotkey>()
          ->set_hotkey_ptr(&config->aimbot[0].rage_hotkey)
            ->set_render_on(&config->aimbot[0].rage_enabled);

      gcs::toggle(other_group, WXOR(L"Anti-Aim"), &config->aimbot[0].rage_anti_aim_enabled)
        ->set_render_on(&config->aimbot[0].rage_enabled);
      gcs::toggle(other_group, WXOR(L"Anti-Aim key"), &config->aimbot[0].rage_anti_aim_on_key)
        ->set_render_on(&config->aimbot[0].rage_enabled)
        ->set_render_on(&config->aimbot[0].rage_anti_aim_enabled)
          ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&config->aimbot[0].rage_anti_aim_hotkey)
            ->set_render_on(&config->aimbot[0].rage_anti_aim_on_key);

      gcs::dropdown(other_group, WXOR(L"Pitch"), false)
        ->add_option(WXOR(L"Disabled"), &config->aimbot[0].rage_anti_aim_pitch_disabled)
        ->add_option(WXOR(L"Down"), &config->aimbot[0].rage_anti_aim_pitch_down)
        ->add_option(WXOR(L"Up"), &config->aimbot[0].rage_anti_aim_pitch_up)
        ->add_option(WXOR(L"Jitter"), &config->aimbot[0].rage_anti_aim_pitch_jitter)
        ->set_render_on(&config->aimbot[0].rage_enabled)
        ->set_render_on(&config->aimbot[0].rage_anti_aim_enabled);

      gcs::dropdown(other_group, WXOR(L"Yaw"), false)
        ->add_option(WXOR(L"Disabled"), &config->aimbot[0].rage_anti_aim_yaw_disabled)
        ->add_option(WXOR(L"Backwards"), &config->aimbot[0].rage_anti_aim_yaw_backwards)
        ->add_option(WXOR(L"Spin"), &config->aimbot[0].rage_anti_aim_yaw_spin)
        ->add_option(WXOR(L"Jitter"), &config->aimbot[0].rage_anti_aim_yaw_jitter)
        ->add_option(WXOR(L"360 Jitter"), &config->aimbot[0].rage_anti_aim_yaw_360_jitter)
        ->add_option(WXOR(L"WeedBoss"), &config->aimbot[0].rage_anti_aim_yaw_weed_boss)
        ->set_render_on(&config->aimbot[0].rage_enabled)
        ->set_render_on(&config->aimbot[0].rage_anti_aim_enabled);
    }
  }

  gcs_component_base* create_aimbot(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_canvas* root_panel = gcs::canvas(comp, WXOR(L"Aimbot"), pos, size, false);
    {
      assert(root_panel != nullptr);

      root_panel->add_hook(GCS_HOOK_POST_PAINT, [](gcs_component_base* c, void* p){
        if(g_ow->_gm != 0 && g_esp->_local != nullptr && g_esp->_local->object.hero != 0)
          return true;

        vec3i text_size = c->font()->draw(c->pos() + (c->size() / 2), 20, WXOR(L"Please select a class in-game."), c->scheme()->text, TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_CENTER);
        c->font()->draw(c->pos() + (c->size() / 2) - vec3i(0, text_size.y), 45, WXOR(L"!"), c->scheme()->warning, TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_TOP);

        return true;
      });

      for(u32 i = 1; i < HERO_MAX; i++){
        c_aimbot_settings* aimbot_settings = &config->aimbot[i];

        assert(aimbot_settings != nullptr);
        assert(i >= 0 && i < HERO_MAX);

        gcs_component_group_host* group_host = gcs::group_host(root_panel, WXOR(L"Aimbot"), vec3i(), size);
        {
          assert(group_host != nullptr);

          group_host->info()->param_buffer = i;
          group_host->add_hook(GCS_HOOK_SHOULD_ENABLE, [](gcs_component_base* c, void* p){
            return g_ow->_gm != 0 && g_esp->_local != nullptr && g_esp->_local->object.hero != 0 && c->info()->param_buffer == g_esp->_local->object.hero;
          });

          handle_aimbot_class(group_host, aimbot_settings, i, pos, size);
        }
      }
    }

    return root_panel;
  }

  gcs_component_group_host* create_player_esp_tabs(gcs_component_base* comp, i32 config_idx){
    assert(comp != nullptr);
    assert(config_idx >= 0 && config_idx <= 2);

    {
      c_esp_settings* esp_settings = &config->esp[config_idx];
      gcs::toggle(comp, WXOR(L"Enabled"), &esp_settings->enabled);

      gcs::dropdown(comp, WXOR(L"Components"), true)
        ->add_option(WXOR(L"Hero name"), &esp_settings->name)
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

  gcs_component_group_host* create_visuals(gcs_component_base* comp, vec3i pos, vec3i size){
   assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Visuals"), pos, size);
    {
      assert(group_host != nullptr);

      gcs_component_group* player_esp_group     = group_host->new_group(WXOR(L"Player esp"));
      gcs_component_group* player_enemy_group   = player_esp_group->new_category(WXOR(L"Enemy"));
      gcs_component_group* player_team_group    = player_esp_group->new_category(WXOR(L"Team"));
      gcs_component_group* player_colours_group = player_esp_group->new_category(WXOR(L"Colours"));

      create_player_esp_tabs(player_enemy_group, 0);
      create_player_esp_tabs(player_team_group, 1);

      // Colours
      {
        gcs::label(player_colours_group, WXOR(L"Enemy colour"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->esp[0].box_colour);

        gcs::label(player_colours_group, WXOR(L"Friendly colour"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->esp[1].box_colour);
      }

      gcs_component_group* other_group = group_host->new_group(WXOR(L"Other"));
      {
        gcs::label(other_group, WXOR(L"Menu Foreground"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.menu_foreground_colour);

        gcs::label(other_group, WXOR(L"Menu Background"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(config->visual.menu_background_colour);

        gcs::dropdown(other_group, WXOR(L"Enemy Glow"), false)
          ->add_option(WXOR(L"Disabled"), &config->esp[0].glow_disabled)
          ->add_option(WXOR(L"Custom Colour"), &config->esp[0].glow_custom_color)
          ->add_option(WXOR(L"Health Based"), &config->esp[0].glow_health_based);

        gcs::label(other_group, WXOR(L"Glow colour"))
          ->set_render_on(&config->esp[0].glow_custom_color)
            ->add_module<gcs_component_colour_picker>()
              ->enable_alpha_bar()
              ->set_output_colour_ptr(config->esp[0].glow_health_rgb);
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
        gcs::toggle(general_group, WXOR(L"Bunny hop"), &config->misc.bhop);
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

          c->set_name(WXOR(L"Load"));
          if(!cloud::download(*(u32*)p, config, XOR32(sizeof(c_settings)), XOR32(SETTINGS_VERSION), true))
            c->set_name(WXOR(L"Load fail"));

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

          c->set_name(WXOR(L"Save"));
          if(!cloud::upload(*(u32*)p, config, XOR32(sizeof(c_settings)), true))
            c->set_name(WXOR(L"Save fail"));

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

    gcs_component_window* window = gcs::window(this, WXOR(L"RijiN For Overwatch"), menu_pos, menu_size);
    {
      assert(window != nullptr);

      window->add_hook(GCS_HOOK_SHOULD_ENABLE, [](gcs_component_base* c, void* p){
        bool key_pressed = c->input() & GCS_IN_INSERT_CLICK || c->input() & GCS_IN_F3_CLICK;

        if(key_pressed)
          global->menu_open = !global->menu_open;

        return global->menu_open;
      });

      gcs_component_select_sidebar* sidebar = window->add<gcs_component_select_sidebar>();
      {
        assert(sidebar != nullptr);

        sidebar->set_pos(vec3i());
        sidebar->set_size(vec3i(select_width, menu_size.y));

        vec3i canvas_pos = vec3i(select_width, 0);
        vec3i canvas_size = vec3i(menu_size.x - select_width, menu_size.y);

        if(global->access_level >= 2)
          sidebar->add_entry(create_aimbot(window, canvas_pos, canvas_size));

        sidebar->add_entry(create_visuals(window, canvas_pos, canvas_size));

        if(global->access_level >= 2)
          sidebar->add_entry(create_misc(window, canvas_pos, canvas_size));

        add_cloud_configs_selection(sidebar);
      }
    }

    return true;
  }
};