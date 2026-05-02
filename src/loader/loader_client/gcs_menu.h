#pragma once

struct exec_stub_thread_data{
  gcs_component_button*     button      = nullptr;
  u32                       pos         = 0;
};

static u32 __stdcall exec_stub_thread(exec_stub_thread_data* p){
  assert(p != nullptr);

  if(p == nullptr)
    return 0;

  license_object* obj = &global->license_data->objects[p->pos];

  // Freeze our input on the menu
  {
    if(!stub->load(global->license_data, p->pos)){
      global->hide_loading_page();
      SHOW_ERROR_STR(ERR_INJECT_STUB_LOAD_FROM_THREAD_FAILED, "Stub agent could not be loaded", true);
      free(p);
      return 0;
    }

    for(u32 i = 3; i > 0; i--){
      if(utils::get_pid(obj->target_hash) != 0)
        break;

      if(obj->appid != 0)
        global->show_loading_page(convert::str2wstr(utils::format(XOR("Launching \"%s\" (%is)"), obj->name, i)));
      else
        global->show_loading_page(convert::str2wstr(utils::format(XOR("Please launch \"%s\" (%is)"), obj->name, i)));

#if !defined(DEV_MODE)
      I(Sleep)(1000);
#endif
    }

    global->show_loading_page(XOR(L"The loader will now close."));
  }

#if !defined(DEV_MODE)
  I(Sleep)(2500);
#endif

  // Free the memory we allocated before the thread call
  I(free)(p);

  // Exit
  I(exit)(0);

  return 0;
}

class gcs_menu : public gcs_scene{
public:
  vec3i menu_size = render->screen_size;

  bool setup() override {

    gcs_component_canvas* canvas = add<gcs_component_canvas>();
    {
      canvas->add_hook(GCS_HOOK_PRE_PAINT, [](gcs_component_base* c, void* p){
        gcs_colour_scheme* scheme = c->scheme();

        //render->filled_rect(vec3i(0, 0), render->screen_size, rgb(26, 35, 58));
        //render->filled_rect(vec3i(0, 0), render->screen_size, rgba(0,0,0,160));

        float anim_timer = math::fmod(math::time() * 0.25f, 2.f);
        float anim_frac  = math::clamp(anim_timer >= 1.f ? 1.f - math::abs(1.f - anim_timer) : anim_timer, 0.f, 1.f);

        colour low_col  =  rgb(26, 35, 58);
        colour high_col =  low_col * 1.5f;
        colour from = math::lerp(low_col, high_col, anim_frac);
        colour to   = math::lerp(high_col, low_col, anim_frac);

        render->gradient_rect_linear(vec3i(0,0), render->screen_size, vec3i(0, 0), render->screen_size, from, to);
        return true;
      });

      canvas->set_pos(vec3i());
      canvas->set_size(menu_size);
      {
        gcs_component_group_host* group_host = gcs::group_host(canvas, WXOR(L"Main"), vec3i(0, 0), menu_size + vec3i(menu_size.x, 0));
        {
          gcs_component_group* cheat_selection_group = group_host->new_group(WXOR(L""));
          group_host->padding = 1;
          group_host->update();

          static i32 bitmap_buffering_delay = 0;
          for(u32 i = 0; i < global->license_data->objects_size; i++){
            license_object* current_license_object = &global->license_data->objects[i];

            // Dont show invisible packages that arent sent to the client
            if(current_license_object->target_hash == 0 || current_license_object->name_hash == 0)
              continue;

            gcs_component_cheat_select_option* button = gcs::cheat_select_option(cheat_selection_group, convert::str2wstr(current_license_object->name));
            button->set_license_object(current_license_object);
            button->set_pos(vec3i(4, 0));
            button->set_size(vec3i(0, button->size().y - 4));
            button->set_param(i);

            utils::create_worker([](gcs_component_cheat_select_option* e){
              while(e->info()->tick_count < 1){
                // Done to prevent spamming bitmap buffer requests
                bitmap_buffering_delay += XOR32(100);
                I(Sleep)(bitmap_buffering_delay);
              }

              c_net_receive_image_resource* image_rsrc = server->request_image_resource(global->license_data, e->license_obj->pos);

              if(image_rsrc != nullptr){
                global->bitmap_creation_queue.emplace_back(image_rsrc, &e->bitmap_icon_data);
                DBG("[!] Queued bitmap \"%s\"\n", e->license_obj->name);
              }
              else{
                SHOW_ERROR_STR(ERR_FAILED_TO_LOAD_BITMAP, "Failed to stream icon", false);
              }

              return 0;
            }, button);

            if(!current_license_object->disabled)
              button->add_hook(GCS_HOOK_CLICK, [](gcs_component_base* c, void* p){
                exec_stub_thread_data* thread_data = malloc(sizeof(exec_stub_thread_data));

                // Fill the stub data
                {
                  thread_data->button = (gcs_component_button*)c;
                  thread_data->pos    = p;
                }

                if(!utils::create_worker(exec_stub_thread, thread_data)){
                  SHOW_ERROR_STR(ERR_CREATE_WORKER_THREAD_FAILED, "Could not dispatch loader agent", true);
                }

                return false;
              });

            button->add_hook(GCS_HOOK_INPUT_ENABLED, [](gcs_component_base* c, void* p){
              return !global->draw_loading_page;
            });
          }
        }
      }
    }

    return true;
  }
};