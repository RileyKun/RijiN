#pragma once

#include "link.h"

class c_global;

class gcs_menu : public gcs_scene{
public:
  vec3i menu_size     = vec3i(600, 500);
  i32 select_width    = menu_size.x / 5;

  gcs_component_group_host* create_aimbot(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);


    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Sample Tab"), pos, size);
    gcs_component_group*      group1     = group_host->new_group(L"Group 1");
    gcs_component_group*      group2     = group_host->new_group(L"Group 2");
    gcs_component_group*      group3     = group_host->new_group(L"Group 3");
    gcs_component_group*      group4     = group_host->new_group(L"Group 4");
    {
    
      gcs_component_group* group1_cat1 = group1->new_category(WXOR(L"Cat 1"));
      gcs_component_group* group1_cat2 = group1->new_category(WXOR(L"Cat 2"));
      gcs_component_group* group1_cat3 = group1->new_category(WXOR(L"Cat 3"));
      gcs_component_group* group1_cat4 = group1->new_category(WXOR(L"Cat 4"));


      {

        static float example_colour[5];

        static u8 hotkey_example = VK_LSHIFT;
        static i32 _slider = 0;
        static bool dropdown_checkboxes_all[4];
        static bool dropdown_checkboxes_single[4];

        static bool checkbox_sample = false;
        static bool checkbox_sample2 = false;
        static bool render_checkbox_sample = false;
        gcs::toggle(group1_cat1, WXOR(L"Checkbox"), &checkbox_sample)
          ->set_help_text(WXOR(L"This is a test\nWow\nAwesome"));

        gcs::toggle(group1_cat1, WXOR(L"Checkbox2"), &render_checkbox_sample)
          ->set_render_on(&checkbox_sample)
          ->set_help_text(WXOR(L"This is a checkbox, it's meant for showing information.\nCan be put on newlines..."));

        gcs::dropdown(group1_cat1, WXOR(L"Multi selection dropdown"), true)
          ->add_option(WXOR(L"Item 1"), &dropdown_checkboxes_all[0])
          ->add_option(WXOR(L"Item 2"), &dropdown_checkboxes_all[1])
          ->add_option(WXOR(L"Item 3"), &dropdown_checkboxes_all[2])
          ->add_option(WXOR(L"Item 4"), &dropdown_checkboxes_all[3])
            ->set_help_text(WXOR(L"1\n2\n3\n4\n5\nThis is how it looks like."));

        gcs::dropdown(group1_cat1, WXOR(L"Single selection dropdown"), false)
          ->add_option(WXOR(L"Item 1"), &dropdown_checkboxes_single[0])
          ->add_option(WXOR(L"Item 2"), &dropdown_checkboxes_single[1])
          ->add_option(WXOR(L"Item 3"), &dropdown_checkboxes_single[2])
          ->add_option(WXOR(L"Item 4"), &dropdown_checkboxes_single[3]);

        gcs::slider(group1_cat1, WXOR(L"Slider"), -100, 100, GCS_SLIDER_TYPE_INT, &_slider);

        gcs::label(group1_cat1, WXOR(L"Label example"));

        gcs::label(group1_cat2, WXOR(L"Hotkey label"))
          ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&hotkey_example);

        gcs::toggle(group1_cat2, WXOR(L"Hotkey checkbox"), &checkbox_sample2)
          ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&hotkey_example);

        gcs::slider(group1_cat2, WXOR(L"Hotkey Slider"), -100, 100, GCS_SLIDER_TYPE_INT, &_slider)
         ->add_module<gcs_component_hotkey>()
            ->set_hotkey_ptr(&hotkey_example);


        gcs::label(group1_cat3, WXOR(L"Colour label"))
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(example_colour)
            ->enable_alpha_bar();

        gcs::toggle(group1_cat3, WXOR(L"Colour checkbox"), &checkbox_sample2)
          ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(example_colour)
            ->enable_alpha_bar();

        gcs::slider(group1_cat3, WXOR(L"Colour Slider"), -100, 100, GCS_SLIDER_TYPE_INT, &_slider)
         ->add_module<gcs_component_colour_picker>()
            ->set_output_colour_ptr(example_colour)
            ->enable_alpha_bar();

      }

      {
        gcs_component_group* group2_cat1 = group2->new_category(WXOR(L"Cat 1"));
        gcs_component_group* group2_cat2 = group2->new_category(WXOR(L"Cat 2"));
        gcs_component_group* group2_cat3 = group2->new_category(WXOR(L"Cat 3"));
        gcs_component_group* group2_cat4 = group2->new_category(WXOR(L"Cat 4"));
  
        gcs_component_group* group3_cat1 = group3->new_category(WXOR(L"Cat 1"));
        gcs_component_group* group3_cat2 = group3->new_category(WXOR(L"Cat 2"));
        gcs_component_group* group3_cat3 = group3->new_category(WXOR(L"Cat 3"));
        gcs_component_group* group3_cat4 = group3->new_category(WXOR(L"Cat 4"));
  
        gcs_component_group* group4_cat1 = group4->new_category(WXOR(L"Cat 1"));
        gcs_component_group* group4_cat2 = group4->new_category(WXOR(L"Cat 2"));
        gcs_component_group* group4_cat3 = group4->new_category(WXOR(L"Cat 3"));
        gcs_component_group* group4_cat4 = group4->new_category(WXOR(L"Cat 4"));
      }

    }

    group_host->update();
    return group_host;
  }

  gcs_component_group_host* create_sub_tab1(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);
    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Sub Tab 1"), pos, size);

    group_host->update();
    return group_host;
  }

  gcs_component_group_host* create_sub_tab2(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);
    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Sub Tab 2"), pos, size);

    group_host->update();
    return group_host;
  }

  gcs_component_group_host* create_sub_tab3(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);
    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Sub Tab 3"), pos, size);

    group_host->update();
    return group_host;
  }

  gcs_component_canvas* create_entity_list(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);
    gcs_component_canvas* main_canvas = gcs::canvas(comp, WXOR(L"Entity List"), pos, size, false);
    
    // All of these positions are set and done. Don't mess with them, unless some magical UI code is changed and somehow breaks them.
    gcs_component_canvas* canvas = gcs::canvas(main_canvas, WXOR(L"Entity List Settings"), vec3i(), size, false);
    {
      canvas->info()->disable_auto_positioning();
  
      gcs_component_group_host* group_host = gcs::group_host(canvas, WXOR(L"Trollers1"), vec3i(), size);
      gcs_component_group* group           = group_host->new_group(L"Trollers2");

      gcs_component_group* group_cat1 = group->new_category(WXOR(L"Settings"));
      gcs_component_group* group_cat2 = group->new_category(WXOR(L"ESP"));
      gcs_component_group* group_cat3 = group->new_category(WXOR(L"Chams"));
      gcs_component_group* group_cat4 = group->new_category(WXOR(L"Glow"));

      gcs::button(group_cat1, L"Test Button 1")
        ->add_hook(GCS_HOOK_CLICK, [](gcs_component_base* c, void* p){
          DBG("[!] click 1\n");
          return true;
        });

      group_host->update();
    }
    
    gcs_component_canvas* canvas2 = gcs::canvas(main_canvas, WXOR(L"Entity List Window"), vec3i(size.x / 2, (2)), vec3i(size.x / 2, size.y - (4)), true);
    canvas2->add<gcs_component_entity_list>()
      ->info()->disable_alpha_vis_fade(); 
    canvas2->add_module<gcs_component_scrollbar>();
    canvas2->info()->disable_alpha_vis_fade();
    {
      canvas2->add_hook(GCS_HOOK_POST_PAINT, [](gcs_component_base* c, void* p){
        colour background_transparent = c->scheme()->grey3;
        background_transparent.a = 255.f * 0.5f;

        render->outlined_rect(c->pos(true), c->size(), background_transparent);

        return true;
      });
    }

    return main_canvas;
  }

  gcs_component_canvas* create_player_list(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    // Create a base canvas. (Otherwise things are fucked)
    gcs_component_canvas* main_canvas = gcs::canvas(comp, WXOR(L"Player List"), pos, size, false);

    // Then create the one we'll draw on, but do not pass the pos.
    gcs_component_canvas* canvas      = gcs::canvas(main_canvas, WXOR(L"Player List Settings"), vec3i(), size, false);

    canvas->info()->disable_auto_positioning();

    gcs_component_group_host* group_host = gcs::group_host(canvas, WXOR(L"Trollers1"), vec3i(), size);
    gcs_component_group*      group      = group_host->new_group(L"Trollers2");

    gcs_component_group* group_cat1 = group->new_category(WXOR(L"Settings"));
    gcs_component_group* group_cat2 = group->new_category(WXOR(L"ESP"));
    {
      // Options here.
    }
    group_host->update();


    gcs_component_canvas* canvas2 = gcs::canvas(main_canvas, WXOR(L"Player List Window"), vec3i(size.x / 2, (2)), vec3i(size.x / 2, size.y - (4)), true);
    canvas2->add<gcs_component_player_list>()
      ->info()->disable_alpha_vis_fade(); 
    canvas2->add_module<gcs_component_scrollbar>();
    canvas2->info()->disable_alpha_vis_fade();
    {
      canvas2->add_hook(GCS_HOOK_POST_PAINT, [](gcs_component_base* c, void* p){
        vec3i pos  = c->pos(true);
        vec3i size = c->size();

        colour background_transparent = c->scheme()->grey3;
        background_transparent.a = 255.f * 0.5f;

        render->outlined_rect(pos, size, background_transparent);

        return true;
      });
    }

    return main_canvas;
  }

  gcs_component_canvas* create_notifications_list(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);
    gcs_component_canvas* tab_canvas   = gcs::canvas(comp, WXOR(L"General"), pos, size, false);
    gcs_component_canvas* draw_canvas  = gcs::canvas(tab_canvas, WXOR(L"List Window"), vec3i((2), (2)), size - vec3i(0, (4)), true);

    draw_canvas->add<gcs_notify_component_history>()
      ->info()->disable_alpha_vis_fade();
    draw_canvas->add_module<gcs_component_scrollbar>();
    draw_canvas->info()->disable_alpha_vis_fade();
    {
      draw_canvas->add_hook(GCS_HOOK_POST_PAINT, [](gcs_component_base* c, void* p){
        colour background_transparent = c->scheme()->grey3;
        background_transparent.a = 255.f * 0.5f;
        render->outlined_rect(c->pos(true), c->size(), background_transparent);
        return true;
      });
    }

    return tab_canvas;
  }

  gcs_component_canvas* create_player_notifications_list(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);
    gcs_component_canvas* tab_canvas   = gcs::canvas(comp, WXOR(L"Player"), pos, size, false);
    gcs_component_canvas* draw_canvas  = gcs::canvas(tab_canvas, WXOR(L"List Window"), vec3i((2), (2)), size - vec3i(0, (4)), true);

    logs->player_log = draw_canvas->add<c_gcs_component_log_window>();
    logs->player_log->info()->disable_alpha_vis_fade();

    draw_canvas->add_module<gcs_component_scrollbar>();
    draw_canvas->info()->disable_alpha_vis_fade();
    {
      draw_canvas->add_hook(GCS_HOOK_POST_PAINT, [](gcs_component_base* c, void* p){
        colour background_transparent = c->scheme()->grey3;
        background_transparent.a = 255.f * 0.5f;
        render->outlined_rect(c->pos(true), c->size(), background_transparent);
        return true;
      });
    }

    return tab_canvas;
  }

  gcs_component_canvas* create_connections_notifications_list(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);
    gcs_component_canvas* tab_canvas   = gcs::canvas(comp, WXOR(L"Connections"), pos, size, false);
    gcs_component_canvas* draw_canvas  = gcs::canvas(tab_canvas, WXOR(L"List Window"), vec3i((2), (2)), size - vec3i(0, (4)), true);

    logs->server_log = draw_canvas->add<c_gcs_component_log_window>();
    logs->server_log->info()->disable_alpha_vis_fade();

    draw_canvas->add_module<gcs_component_scrollbar>();
    draw_canvas->info()->disable_alpha_vis_fade();
    {
      draw_canvas->add_hook(GCS_HOOK_POST_PAINT, [](gcs_component_base* c, void* p){
        colour background_transparent = c->scheme()->grey3;
        background_transparent.a = 255.f * 0.5f;
        render->outlined_rect(c->pos(true), c->size(), background_transparent);
        return true;
      });
    }

    return tab_canvas;
  }

  void add_cloud_configs_selection(gcs_component_base* comp){
    //if(!cloud::enabled)
    //  return;

    assert(comp != nullptr);

    u32 padding = render->rescale(4);
    u32 top_offset = render->rescale(60);

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
        load_config->set_size(vec3i(button_width - (padding / 2), render->rescale(16)));
        load_config->set_pos(vec3i(padding, comp->size().y - (load_config->size().y + padding + top_offset)));

        load_config->add_hook(GCS_HOOK_CLICK, [](gcs_component_base* c, void* p){
          assert(p != nullptr);

          //s_cloud_thread_data* thread_data = malloc(sizeof(s_cloud_thread_data));
          //{
          //  thread_data->id     = *(u32*)p;
          //  thread_data->extra  = c;
          //}

          //utils::create_worker([](s_cloud_thread_data* data){
          //  assert(data != nullptr);
          //  assert(data->extra != nullptr);
//
          //  gcs_component_base* comp = data->extra;
//
          //  comp->set_name(WXOR(L"..."));
          //  {
          //    if(cloud::download(data->id, config, sizeof(c_settings), SETTINGS_VERSION, true))
          //      comp->set_name(WXOR(L"Done"));
          //    else
          //      comp->set_name(WXOR(L"Fail"));
//
          //    Sleep(XOR32(1000));
          //  }
          //  comp->set_name(WXOR(L"Load"));
//
          //  free(data);
//
          //  return 0;
          //}, thread_data);

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
        save_config->set_size(vec3i(button_width - ((padding / 2)), render->rescale(16)));
        save_config->set_pos(vec3i(button_width + padding + (padding / 2), comp->size().y - (save_config->size().y + padding + top_offset)));

        save_config->add_hook(GCS_HOOK_CLICK, [](gcs_component_base* c, void* p){
          assert(p != nullptr);

          //s_cloud_thread_data* thread_data = malloc(sizeof(s_cloud_thread_data));
          //{
          //  thread_data->id     = *(u32*)p;
          //  thread_data->extra  = c;
          //}
//
          //utils::create_worker([](s_cloud_thread_data* data){
          //  assert(data != nullptr);
          //  assert(data->extra != nullptr);
//
          //  gcs_component_base* comp = data->extra;
//
          //  comp->set_name(WXOR(L"..."));
          //  {
          //    if(cloud::upload(data->id, config, sizeof(c_settings), true))
          //      comp->set_name(WXOR(L"Done"));
          //    else
          //      comp->set_name(WXOR(L"Fail"));
//
          //    Sleep(XOR32(1000));
          //  }
          //  comp->set_name(WXOR(L"Save"));
//
          //  free(data);
//
          //  return 0;
          //}, thread_data);

          return false;
        });
      }

      top_offset = render->rescale(20);

      gcs_component_button* import_from_clipboard = comp->add<gcs_component_button>();
      {
        assert(import_from_clipboard != nullptr);

        import_from_clipboard->info()->disable_auto_positioning();
        import_from_clipboard->info()->enable_override_auto_positioning();
        import_from_clipboard->set_name(WXOR(L"Import clipboard"));
        //import_from_clipboard->set_param(&select_config->last_selected_index);
        import_from_clipboard->set_size(vec3i(button_width * 2, render->rescale(16)));
        import_from_clipboard->set_pos(vec3i(padding, comp->size().y - (import_from_clipboard->size().y + padding + top_offset)));

        import_from_clipboard->add_hook(GCS_HOOK_CLICK, [](gcs_component_base* c, void* p){


          return false;
        });

        top_offset += import_from_clipboard->size().y + padding;
      }

      gcs_component_button* export_from_clipboard = comp->add<gcs_component_button>();
      {
        assert(export_from_clipboard != nullptr);

        export_from_clipboard->info()->disable_auto_positioning();
        export_from_clipboard->info()->enable_override_auto_positioning();
        export_from_clipboard->set_name(WXOR(L"Export clipboard"));
        //export_from_clipboard->set_param(&select_config->last_selected_index);
        export_from_clipboard->set_size(vec3i(button_width * 2, render->rescale(16)));
        export_from_clipboard->set_pos(vec3i(padding, comp->size().y - (export_from_clipboard->size().y + padding + top_offset)));

        export_from_clipboard->add_hook(GCS_HOOK_CLICK, [](gcs_component_base* c, void* p){


          return false;
        });
        top_offset += export_from_clipboard->size().y + padding;
      }
    }

    top_offset = 0;


    // set selection dropdown
    {
      select_config->info()->disable_auto_positioning();
      select_config->info()->enable_override_auto_positioning();
      select_config->set_full_wide(true);
      select_config->set_size(vec3i(comp->size().x - (padding * 2), render->rescale(16)));
      select_config->set_pos(vec3i(padding, comp->size().y - (select_config->size().y + padding + top_offset)));
    }

  }

  bool setup() override {

    DBG("[!] We called menu setup\n");
    vec3i menu_pos = render->screen_size / 2;

    gcs_component_window* window = gcs::window(this, WXOR(L"RijiN - GFX D3D9"), menu_pos, render->rescale(menu_size));
    window->wants_icon = true;
    {
      i32   scaled_select_width = render->rescale(select_width);
      vec3i scaled_menu_size    = render->rescale(menu_size);

      assert(window != nullptr);

      window->add_hook(GCS_HOOK_SHOULD_ENABLE, [](gcs_component_base* c, void* p){
        return true;
      });

      gcs_component_select_sidebar* sidebar = window->add<gcs_component_select_sidebar>();
      {
        assert(sidebar != nullptr);

        sidebar->set_pos(vec3i());
        sidebar->set_size(vec3i(scaled_select_width, scaled_menu_size.y));

        vec3i canvas_pos = vec3i(scaled_select_width, 0);
        vec3i canvas_size = vec3i(scaled_menu_size.x - scaled_select_width, scaled_menu_size.y);
        sidebar->add_entry(create_aimbot(window, canvas_pos, canvas_size));
        sidebar->add_entry(L"Sub tabs")
          ->add_entry(create_sub_tab1(window, canvas_pos, canvas_size))
          ->add_entry(create_sub_tab2(window, canvas_pos, canvas_size))
          ->add_entry(create_sub_tab3(window, canvas_pos, canvas_size));

        sidebar->add_entry(create_player_list(window, canvas_pos, canvas_size));
        sidebar->add_entry(create_entity_list(window, canvas_pos, canvas_size));

        sidebar->add_entry(L"Notifications")
          ->add_entry(create_notifications_list(window, canvas_pos, canvas_size))
          ->add_entry(create_player_notifications_list(window, canvas_pos, canvas_size))
          ->add_entry(create_connections_notifications_list(window, canvas_pos, canvas_size));
       //sidebar->add_entry(WXOR(L"Visuals"))
       //  ->add_entry(create_visuals_main(window, canvas_pos, canvas_size))
       //  ->add_entry(create_visuals_models(window, canvas_pos, canvas_size));

       //sidebar->add_entry(create_misc(window, canvas_pos, canvas_size));

        add_cloud_configs_selection(sidebar);
      }
    }

    return true;
  }
};