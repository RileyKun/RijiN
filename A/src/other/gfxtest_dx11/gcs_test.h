#pragma once

class gcs_test : public gcs_scene{
public:
  vec3i menu_size     = vec3i(600, 500);
  i32 select_width    = menu_size.x / 5;

  gcs_component_group_host* create_misc(gcs_component_base* comp, vec3i pos, vec3i size){
    assert(comp != nullptr);

    gcs_component_group_host* group_host = gcs::group_host(comp, WXOR(L"Miscellaneous"), pos, size);
    {
      gcs_component_group* general_group = group_host->new_group(WXOR(L"General"));
      gcs_component_group* other_group   = group_host->new_group(WXOR(L"Other"));
      group_host->update();

      // General
      {
        static bool test1 = false;
        static bool test2 = true;
        gcs::dropdown(general_group, XOR(L"Ignore flags"), true)
          ->add_option(XOR(L"Ignore healed robots"), &test1)
          ->add_option(XOR(L"Ignore sentry busters"), &test2);
      }

      // Other
      {
        static bool test1 = false;
        static float col[4];
        gcs::toggle(other_group, WXOR(L"Test"), &test1)
          ->add_module<gcs_component_colour_picker>()
            ->enable_alpha_bar()
            ->set_output_colour_ptr(col);

        static float black;
        gcs::slider(other_group, WXOR(L"Test slider"), WXOR(L"%"), 1, 100, GCS_SLIDER_TYPE_FLOAT, &black);
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
              //if(cloud::download(data->id, config, sizeof(c_settings), SETTINGS_VERSION, true))
              //  comp->set_name(WXOR(L"Done"));
              //else
              //  comp->set_name(WXOR(L"Fail"));

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
              //if(cloud::upload(data->id, config, sizeof(c_settings), true))
              //  comp->set_name(WXOR(L"Done"));
              //else
              //  comp->set_name(WXOR(L"Fail"));

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
    /*{
      cloud::enabled = true;

      if(cloud::enabled && !input_system->held(VK_HOME)){
        utils::create_worker([](void* p){
          cloud::download(0, config, sizeof(c_settings), SETTINGS_VERSION, true);

          return 0;
        }, nullptr);
      }
    }*/

    vec3i menu_pos = render->screen_size / 2;

    gcs_component_window* window = gcs::window(this, WXOR(L"RijiN For Overwatch"), menu_pos, menu_size);
    {
      assert(window != nullptr);

      window->add_hook(GCS_HOOK_SHOULD_ENABLE, [](gcs_component_base* c, void* p){
        return true;
      });

      gcs_component_select_sidebar* sidebar = window->add<gcs_component_select_sidebar>();
      {
        assert(sidebar != nullptr);

        sidebar->set_pos(vec3i());
        sidebar->set_size(vec3i(select_width, menu_size.y));

        vec3i canvas_pos = vec3i(select_width, 0);
        vec3i canvas_size = vec3i(menu_size.x - select_width, menu_size.y);

        sidebar->add_entry(create_misc(window, canvas_pos, canvas_size));

        add_cloud_configs_selection(sidebar);
      }
    }

    return true;
  }
};