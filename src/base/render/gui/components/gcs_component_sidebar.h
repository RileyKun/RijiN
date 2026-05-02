#pragma once

struct gcs_sidebar_entry{
  std::wstring                     name;
  std::vector<gcs_component_base*> sub_list;

  ALWAYSINLINE gcs_sidebar_entry* add_entry(gcs_component_base* comp){
    assert(comp != nullptr);

    sub_list.push_back(comp);

    comp->disable();

    return this;
  }
};

class gcs_component_select_sidebar : public gcs_component_base{
private:
  std::vector<gcs_sidebar_entry>    list;
  gcs_component_base*               selected_comp;
  gcs_sidebar_entry*                selected_list;
  i32                               font_size = 18;
public:

  ALWAYSINLINE void set_active_comp(gcs_sidebar_entry* entry, gcs_component_base* new_comp){
    if(new_comp != selected_comp)
      emit_click_sound();

    selected_comp = new_comp;
    selected_list = entry;

    for(gcs_sidebar_entry entry : list){
      for(gcs_component_base* comp : entry.sub_list){
        assert(comp != nullptr);

        if(comp != nullptr)
          comp == selected_comp ? comp->enable() : comp->disable();
      }
    }
  }

  ALWAYSINLINE gcs_sidebar_entry* add_entry(gcs_component_base* comp){
    assert(comp != nullptr);

    gcs_sidebar_entry new_entry;
    {
      new_entry.sub_list.push_back(comp);
    }

    list.push_back(new_entry);

    comp->disable();

    if(list.size() == 1)
      set_active_comp(&list[0], comp);

    return &list.back();
  }

  ALWAYSINLINE gcs_sidebar_entry* add_entry(std::wstring name){
    assert(!name.empty());

    gcs_sidebar_entry new_entry;
    {
      new_entry.name = name;
    }

    list.push_back(new_entry);

    return &list.back();
  }

  ALWAYSINLINE gcs_component_base* get_active_comp(){
    return selected_comp;
  }

  void set_font_size(i32 new_size){
    font_size = new_size;
  }

  ALWAYSINLINE void setup() override{
    info()->always_run_think = true;
  }

  void think(vec3i pos, vec3i size) override{
  }

  void draw(vec3i pos, vec3i size){
    gcs_colour_scheme* _scheme = scheme();
    gcs_scene_info* _scene = scene()->info();

    // back graphics
    colour alpha_background = scheme()->background;
    alpha_background.a = 255 * 0.40f;
    render->filled_rect(pos, size, alpha_background);

    // draw options
    {
      i32 height_offset = 0;
      i32 bar_height    = render->rescale(30);

      for(gcs_sidebar_entry& entry : list){
        assert(entry.sub_list.size() > 0);

        vec3i start  = pos + vec3i(0, height_offset);
        vec3i bounds = vec3i(size.x, bar_height);
        if(entry.sub_list.size() > 1){
          assert(!entry.name.empty());

          if(selected_list == &entry){
            colour alpha_col = scheme()->main;
            alpha_col.a = 255 * 0.25f;
            render->filled_rect(start, bounds, alpha_col);

            i32 count = 0;
            for(gcs_component_base* comp : entry.sub_list){
              assert(comp != nullptr);
              count++;

              i32 sub_bar_height  = render->rescale(30);
              vec3i sub_start     = pos + vec3i(0, height_offset + bounds.y);
              vec3i sub_bounds    = vec3i(size.x, sub_bar_height);

              if(selected_comp != comp){
                if(is_in_rect(sub_start, sub_bounds)){
                  render->filled_rect(sub_start, sub_bounds, alpha_background * 1.10f);

                  if(input() & GCS_IN_M1_CLICK)
                    set_active_comp(&entry, comp);
                }
              }
              else
                render->filled_rect(sub_start, sub_bounds, _scheme->background * 1.10f);

              // draw tree lines
              {
                colour line_col = selected_comp == comp ? scheme()->main : scheme()->text;
                line_col.a *= 0.50f;

                if(count != entry.sub_list.size())
                  render->filled_rect(sub_start + vec3i(sub_bounds.x / 14, 0), vec3i(1, sub_bounds.y), line_col);
                else
                  render->filled_rect(sub_start + vec3i(sub_bounds.x / 14, 0), vec3i(1, sub_bounds.y / 2), line_col);

                render->filled_rect(sub_start + vec3i(sub_bounds.x / 14, sub_bounds.y / 2), vec3i(4, 1), line_col);
              }

              _scene->font->draw(sub_start + vec3i(sub_bounds.x / 6, sub_bounds.y / 2), font_size, comp->name(), _scheme->text, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);

              height_offset += sub_bounds.y;
            }
          }
          else if(is_in_rect(start, bounds)){
            if(input() & GCS_IN_M1_CLICK)
              set_active_comp(&entry, entry.sub_list[0]);

            render->filled_rect(start, bounds, _scheme->background * 1.2f);
          }

          // Draw name
          _scene->font->draw(start + vec3i(bounds.x / 10, bounds.y / 2), font_size, entry.name, _scheme->text, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);
        }
        else{
          gcs_component_base* comp = entry.sub_list[0];

          if(comp == get_active_comp()){
            colour alpha_col = scheme()->main;
            alpha_col.a = 255 * 0.25f;
            render->filled_rect(start, bounds, alpha_col);
          }
          else if(is_in_rect(start, bounds)){
            if(input() & GCS_IN_M1_CLICK)
              set_active_comp(&entry, comp);

            render->filled_rect(start, bounds, _scheme->background * 1.2f);
          }

          // Draw name
          _scene->font->draw(start + vec3i(bounds.x / 10, bounds.y / 2), font_size, comp->name(), _scheme->text, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);
        }


        height_offset += bounds.y;
      }
    }
  }
};