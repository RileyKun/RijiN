#pragma once

#define MENU_IMPORT_EXPORT_BUTTON_OFFSET 40
#define MENU_SAVE_AND_LOAD_BUTTON_OFFSET 20
#define MENU_CONFIG_SELECTION_OFFSET 0
#define MENU_PADDING_SIZE 4

class gcs_base_menu;

struct s_cloud_param_data{
  gcs_base_menu* menu;
  u32*           id;
};

struct s_cloud_thread_data{
  u32                 id;
  gcs_component_base* comp_base;
  gcs_base_menu*      menu;
};

class gcs_base_menu : public gcs_scene{
public:

  virtual vec3i get_menu_size(){
    return render->rescale(menu_size);
  }

  virtual i32 get_select_width(){
    return render->rescale(select_width);
  }

  virtual u32 get_game_hash(){
    return HASH("UNKNOWN");
  }

  virtual u32 get_config_version(){
    return 1;
  }

  virtual void* get_config_pointer(){
    assert(false && "get_config_size is not setup!");
    return nullptr;
  }

  virtual i32 get_config_size(){
    assert(false && "get_config_size is not setup!");
    return 0;
  }

  ALWAYSINLINE inline void load_config(){
    #if defined(CLOUD_CONFIG)
      static bool once = false;
      if(once)
        return;

      if(get_config_pointer() == nullptr || get_config_size() == 0)
        return;

      if(input_system->held(VK_HOME))
        return;

      utils::create_worker([](gcs_base_menu* p){
        cloud::download(0, p->get_config_pointer(), p->get_config_size(), p->get_config_version(), true);
        return 0;
      }, this);

      once = true;
    #endif
  }

  virtual gcs_component_window* create_window(gcs_scene* comp, std::wstring name, vec3i menu_pos){
    gcs_component_window* window = gcs::window(comp, name, menu_pos, get_menu_size());
    window->wants_icon = true;
    return window;
  }

  virtual gcs_component_select_sidebar* create_sidebar(gcs_component_window* comp, vec3i& canvas_pos, vec3i& canvas_size){
    gcs_component_select_sidebar* sidebar = comp->add<gcs_component_select_sidebar>();
    assert(sidebar != nullptr);
    sidebar->set_pos(vec3i());
    sidebar->set_size(vec3i(get_select_width(), get_menu_size().y));

    canvas_pos  = vec3i(get_select_width(), 0);
    canvas_size = vec3i(get_menu_size().x - get_select_width(), get_menu_size().y);

    return sidebar;
  }

  virtual gcs_component_dropdown* create_select_config(gcs_component_base* comp){
    assert(comp != nullptr);

    // define the config slot dummy booleans
    static bool config_slot[5];
    {
      // Set all but first value to false
      for(u32 i = 0; i < sizeof(config_slot) / sizeof(bool); i++)
        config_slot[i] = (i == 0);
    }

    // create dropdown
    return gcs::dropdown(comp, WXOR(L"Config"), false)
             ->add_option(WXOR(L"Default"),  &config_slot[0])
             ->add_option(WXOR(L"Config 1"), &config_slot[1])
             ->add_option(WXOR(L"Config 2"), &config_slot[2])
             ->add_option(WXOR(L"Config 3"), &config_slot[3])
             ->add_option(WXOR(L"Config 4"), &config_slot[4]);
  }

  virtual void add_clipboard_buttons(gcs_component_base* comp, i32& padding, i32& top_offset, i32& button_width){
    #if defined(CLOUD_CONFIG)
    gcs_component_button* import_from_clipboard = comp->add<gcs_component_button>();
    {
      assert(import_from_clipboard != nullptr);

      import_from_clipboard->info()->disable_auto_positioning();
      import_from_clipboard->info()->enable_override_auto_positioning();
      import_from_clipboard->set_param(param_data);
      import_from_clipboard->set_name(WXOR(L"Import clipboard"));
      import_from_clipboard->set_size(vec3i(button_width * 2, render->rescale(16)));
      import_from_clipboard->set_pos(vec3i(padding, comp->size().y - (import_from_clipboard->size().y + padding + top_offset)));

      import_from_clipboard->add_hook(GCS_HOOK_CLICK, [](gcs_component_base* c, void* p){
          s_cloud_param_data*  param_data = (s_cloud_param_data*)p;
          s_cloud_thread_data* thread_data = malloc(sizeof(s_cloud_thread_data));
          {
            thread_data->id         = *param_data->id;
            thread_data->comp_base  = c;
            thread_data->menu       = param_data->menu;
          }

          utils::create_worker([](s_cloud_thread_data* data){
            gcs_base_menu* m         = data->menu;
            gcs_component_base* comp = data->comp_base;

            comp->set_name(WXOR(L"Parsing..."));

            if(cloud::from_clipboard(m->get_config_version(), m->get_game_hash(), m->get_config_pointer(), m->get_config_size()))
              comp->set_name(WXOR(L"Imported!"));
            else
              comp->set_name(WXOR(L"Malformed config"));

            Sleep(1000);
            comp->set_name(WXOR(L"Import clipboard"));

            free(data);
            return 0;
          }, thread_data);

          return false;
      });

      top_offset += import_from_clipboard->size().y + padding;
    }

    gcs_component_button* export_from_clipboard = comp->add<gcs_component_button>();
    {
      assert(export_from_clipboard != nullptr);

      export_from_clipboard->info()->disable_auto_positioning();
      export_from_clipboard->info()->enable_override_auto_positioning();
      export_from_clipboard->set_name(WXOR(L"Export to clipboard"));
      export_from_clipboard->set_param(param_data);
      export_from_clipboard->set_size(vec3i(button_width * 2, render->rescale(16)));
      export_from_clipboard->set_pos(vec3i(padding, comp->size().y - (export_from_clipboard->size().y + padding + top_offset)));

      export_from_clipboard->add_hook(GCS_HOOK_CLICK, [](gcs_component_base* c, void* p){

        s_cloud_param_data*  param_data = (s_cloud_param_data*)p;
        s_cloud_thread_data* thread_data = malloc(sizeof(s_cloud_thread_data));
        {
          thread_data->id         = *param_data->id;
          thread_data->comp_base  = c;
          thread_data->menu       = param_data->menu;
        }

        utils::create_worker([](s_cloud_thread_data* data){
          gcs_base_menu* m         = data->menu;
          gcs_component_base* comp = data->comp_base;
          comp->set_name(WXOR(L"Generating code..."));

          std::string out_str;
          if(cloud::to_clipboard(m->get_config_version(), m->get_game_hash(), m->get_config_pointer(), m->get_config_size(), &out_str))
            comp->set_name(WXOR(L"Copied to clipboard"));
          else
            comp->set_name(WXOR(L"Error"));

          Sleep(1000);
          comp->set_name(WXOR(L"Export to clipboard"));

          free(data);
          return 0;
        }, thread_data);

        return false;
      });
      top_offset += export_from_clipboard->size().y + padding;
    }
    #endif
    top_offset = render->rescale(MENU_SAVE_AND_LOAD_BUTTON_OFFSET);
  }

  virtual void add_config_buttons(gcs_component_base* comp, gcs_component_dropdown* select_config, i32& padding, i32& top_offset, i32& button_width){
    #if defined(CLOUD_CONFIG)

    gcs_component_button* load_config = comp->add<gcs_component_button>();
    {
      assert(load_config != nullptr);

      load_config->info()->disable_auto_positioning();
      load_config->info()->enable_override_auto_positioning();
      load_config->set_name(WXOR(L"Load"));
      load_config->set_param(param_data);
      load_config->set_size(vec3i(button_width - (padding / 2), render->rescale(16)));
      load_config->set_pos(vec3i(padding, comp->size().y - (load_config->size().y + padding + top_offset)));

      load_config->add_hook(GCS_HOOK_CLICK, [](gcs_component_base* c, void* p){
        assert(p != nullptr);

        s_cloud_param_data*  param_data = (s_cloud_param_data*)p;
        s_cloud_thread_data* thread_data = malloc(sizeof(s_cloud_thread_data));
        {
          thread_data->id         = *param_data->id;
          thread_data->comp_base  = c;
          thread_data->menu       = param_data->menu;
        }


        utils::create_worker([](s_cloud_thread_data* data){
          assert(data != nullptr);

          gcs_base_menu* m         = data->menu;
          gcs_component_base* comp = data->comp_base;

          comp->set_name(WXOR(L"..."));
          {
            DBG("[!] 0x%p - %i\n", m->get_config_pointer(), m->get_config_size());
            if(cloud::download(data->id, m->get_config_pointer(), m->get_config_size(), m->get_config_version(), true))
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
      save_config->set_param(param_data);
      save_config->set_size(vec3i(button_width - ((padding / 2)), render->rescale(16)));
      save_config->set_pos(vec3i(button_width + padding + (padding / 2), comp->size().y - (save_config->size().y + padding + top_offset)));

      save_config->add_hook(GCS_HOOK_CLICK, [](gcs_component_base* c, void* p){
        assert(p != nullptr);

        s_cloud_param_data*  param_data = (s_cloud_param_data*)p;
        s_cloud_thread_data* thread_data = malloc(sizeof(s_cloud_thread_data));
        {
          thread_data->id         = *param_data->id;
          thread_data->comp_base  = c;
          thread_data->menu       = param_data->menu;
        }

        utils::create_worker([](s_cloud_thread_data* data){
          assert(data != nullptr);

          gcs_base_menu* m         = data->menu;
          gcs_component_base* comp = data->comp_base;

          comp->set_name(WXOR(L"..."));
          {
            DBG("[!] 0x%p - %i\n", m->get_config_pointer(), m->get_config_size());
            if(cloud::upload(data->id, m->get_config_pointer(), m->get_config_size(), true))
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

    #endif
    top_offset = MENU_CONFIG_SELECTION_OFFSET;
  }

  virtual void post_select_config_setup(gcs_component_base* comp, gcs_component_dropdown* select_config, i32& padding, i32& top_offset){
    select_config->info()->disable_auto_positioning();
    select_config->info()->enable_override_auto_positioning();
    select_config->set_full_wide(true);
    select_config->set_size(vec3i(comp->size().x - (padding * 2), render->rescale(16)));
    select_config->set_pos(vec3i(padding, comp->size().y - (select_config->size().y + padding + top_offset)));
  }

  virtual void add_cloud_configs_selection(gcs_component_base* comp){
    #if defined(CLOUD_CONFIG)
    assert(comp != nullptr);

    i32 padding      = render->rescale(MENU_PADDING_SIZE);
    i32 top_offset   = render->rescale(MENU_IMPORT_EXPORT_BUTTON_OFFSET);
    i32 button_width = (comp->size().x / 2) - (padding);

    // create dropdown
    gcs_component_dropdown* select_config = create_select_config(comp);

    if(param_data == nullptr){
      param_data = malloc(sizeof(s_cloud_param_data));
      assert(param_data != nullptr);

      param_data->id   = &select_config->last_selected_index;
      param_data->menu = this;
    }

    add_clipboard_buttons(comp, padding, top_offset, button_width);
    add_config_buttons(comp, select_config, padding, top_offset, button_width);
    post_select_config_setup(comp, select_config, padding, top_offset);
    #endif
  }

private:
  vec3i menu_size    = vec3i(600, 500);
  i32   select_width = menu_size.x / 5;
  s_cloud_param_data* param_data = nullptr;
};