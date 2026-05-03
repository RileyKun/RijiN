#pragma once

struct gcs_component_group_category{
  std::wstring        name;
  gcs_component_base* comp;
};

class gcs_component_group : public gcs_component_base{
public:
  std::vector<gcs_component_group_category> categories;
  gcs_component_base* selected_category = nullptr;
  vec3i category_padding = vec3i(0, 18);
  vec3i draw_offset;

  gcs_component_base* new_category(std::wstring category_name){
    gcs_component_group_category entry;
    {
      entry.name  = category_name;
      entry.comp  = add<gcs_component_base>();
    }

    if(categories.empty())
      set_active_comp(entry.comp);

    categories.push_back(entry);

    return entry.comp;
  }

  ALWAYSINLINE void set_active_comp(gcs_component_base* new_comp){
    if(new_comp != selected_category)
      emit_click_sound();

    selected_category = new_comp;
  }

  void setup() override{
    category_padding = render->rescale(vec3i(0, 18));
    add_hook(GCS_HOOK_POST_PAINT, [](gcs_component_base* c, void* p){
      gcs_component_group* group = (gcs_component_group*)c;

      colour background_transparent = group->scheme()->grey3;
      background_transparent.a = 255.f * 0.5f;

      // draw category selection
      if(group->categories.size() > 1){
        vec3i category_pos = group->pos(true) + vec3i(1,1) - vec3i(0, group->draw_offset.y);
        vec3i category_size = vec3i(group->size().x, group->draw_offset.y) - vec3i(2,1);

        render->push_clip_region(category_pos, category_size);
        {
          colour cat_background = group->scheme()->background;
          cat_background.a = 255 * 0.40f;

          render->filled_rect(category_pos, category_size, cat_background);

          // render options
          {
            vec3i opt_pos       = category_pos;
            vec3i opt_size      = vec3i(category_size.x / group->categories.size(), category_size.y);
            i32   current_opt   = 0;

            for(gcs_component_group_category group_category : group->categories){
              bool at_end_opt = current_opt >= group->categories.size() - 1;

              if(!at_end_opt)
                render->filled_rect(opt_pos + vec3i(opt_size.x, 0), vec3i(1, category_size.y), background_transparent);

              bool is_in_opt = group->is_in_rect(opt_pos, opt_size, true);

              if(is_in_opt && input_system->pressed(VK_LBUTTON))
                group->set_active_comp(group_category.comp);

              if(group_category.comp == group->selected_category){
                vec3i opt_padding = vec3i(1, 1);
                vec3i fix_opt_size = opt_size;

                if(at_end_opt)
                  fix_opt_size.x = ((category_pos.x + category_size.x) - (opt_pos.x + opt_padding.x)) + 1;

                render->filled_rect(opt_pos + opt_padding, fix_opt_size - (opt_padding * 2), group->scheme()->main * 0.75f);
                render->outlined_rect(opt_pos + opt_padding, fix_opt_size - (opt_padding * 2), group->scheme()->main);
              }
              else if(is_in_opt)
                render->filled_rect(opt_pos, opt_size, group->scheme()->background);

              render->push_clip_region(opt_pos + vec3i(2,2), opt_size - vec3i(4,4));
              {
                group->font()->draw(opt_pos + (opt_size / 2), 14, group_category.name.c_str(), group->scheme()->text, TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_CENTER);
              }
              render->pop_clip_region();

              opt_pos.x += opt_size.x + 1;
              current_opt++;
            }
          }
        }
        render->pop_clip_region();

        render->filled_rect(group->pos(true) + vec3i(1, 0), vec3i(category_size.x, 1), background_transparent);
      }

      // draw group box
      {
        vec3i size = group->size();

        if(group->module() != nullptr)
          size.x += group->module()->size().x;

        render->outlined_rect(group->pos(true) - group->draw_offset, size + group->draw_offset, background_transparent);

        if(group->categories.size() <= 1)
          group->font()->draw(group->pos(true) - group->draw_offset + vec3i(group->push_align_offset() / 2, 0), 16, group->name().c_str(), group->scheme()->text, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);
        else
          group->font()->draw(group->pos(true) + vec3i(0, size.y) + vec3i(group->push_align_offset() / 2, 0), 16, group->name().c_str(), group->scheme()->text, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);
      }

      return false;
    });

    info()->clip_pos_offset  = vec3i(1,1);
    info()->clip_size_offset = info()->clip_pos_offset * -2;
    info()->always_run_think = true;
  }

  void think(vec3i pos, vec3i size) override{
    if(categories.empty())
      return;

    for(gcs_component_group_category group_category : categories){
      assert(group_category.comp != nullptr);

      if(group_category.comp != nullptr)
        group_category.comp == selected_category ? group_category.comp->enable() : group_category.comp->disable();
    }
  }

  bool update(){
    if(categories.empty()){
      add_module<gcs_component_scrollbar>();
      return false;
    }

    // adjust size and position
    if(categories.size() > 1){
      set_pos(pos(false) + category_padding);
      set_size(size() - category_padding);

      draw_offset = category_padding;
    }

    info()->disable_auto_positioning();

    for(gcs_component_group_category group_category : categories){
      assert(group_category.comp != nullptr);

      group_category.comp->info()->clip_pos_offset = vec3i(1,1);
      group_category.comp->info()->clip_size_offset = info()->clip_pos_offset * -2;
      group_category.comp->info()->enable_auto_positioning();

      group_category.comp->set_pos(vec3i());
      group_category.comp->set_size(size());

      group_category.comp->add_module<gcs_component_scrollbar>();
    }

    return true;
  }
};