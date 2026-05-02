#pragma once

#define ENTITY_LIST_OPTION_SIZE 18
class gcs_component_entity_list : public gcs_component_base{

  void setup() override{
    set_size(vec3i(info()->size.x, ENTITY_LIST_OPTION_SIZE));
  }

  void think(vec3i pos, vec3i size) override{
  }

  void pre_run() override{
  }

  void post_run() override{
  }

  vec3i size() override{
    if(entity_list_data->get_list_count() <= 0)
      return info()->size;

    return vec3i(info()->size.x, (ENTITY_LIST_OPTION_SIZE + (info()->auto_positioning_padding.y / 2)) * entity_list_data->get_list_count());
  }

  void draw(vec3i pos, vec3i size){
    vec3i   option_size = vec3i(size.x, ENTITY_LIST_OPTION_SIZE);
    i32     padding     = info()->auto_positioning_padding.y;
    colour  text_col    = scheme()->text;

    for(i32 i = 0; i < entity_list_data->get_max_entries(); i++){
      s_entity_list_data* entry = entity_list_data->get_data_from_pos(i);
      if(!entry->is_valid()) // This entry isn't setup so ignore it.
        continue;

      colour  r           = text_col * 0.75f;
      bool    _is_in_rect = is_in_rect(pos, option_size);

      // Highlight selected index.
      if(entity_list_data->selected_index == i && entity_list_data->selected_item.class_name_hash){
        r = scheme()->main;
        render->filled_rect(pos, option_size, r * 0.5f);
      }
      else if(entry->entity_data.render){
        r = scheme()->main;
        render->filled_rect(pos, option_size, r * 0.75f);
      }

      if(_is_in_rect){
        r = scheme()->main;
        render->filled_rect(pos, option_size, r * 0.25f);

        // Mouse1 Click
        if(input() & GCS_IN_M1_CLICK){
          memcpy(&entity_list_data->selected_item, entry, sizeof(s_entity_list_data));
          entity_list_data->reset_selected_item();
          entity_list_data->selected_item  = *entry;
          entity_list_data->selected_index = i;
        }
      }

      colour tag_col = rgba(188, 188, 188, text_col.w);
      static wchar_t buf[1024];
      static wchar_t tag[1024];

      memset(tag, 0, sizeof(tag));
      {
        if(entry->entity_data.aimbot_entity){
          tag_col = rgba(255, 32, 32, text_col.w);
          wsprintfW(tag, WXOR(L"🎯"));
        }
      }

      vec3i tag_size;
      if(wcslen(tag) > 0){
        tag_size = font()->draw(pos + vec3i(padding / 2, ENTITY_LIST_OPTION_SIZE / 2), _is_in_rect ? ENTITY_LIST_OPTION_SIZE : ENTITY_LIST_OPTION_SIZE - 2, tag, tag_col, TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_CENTER);
      }

      
      wsprintfW(buf, WXOR(L"%ls"), entry->class_name);
      font()->draw(pos + vec3i(tag_size.x + padding, ENTITY_LIST_OPTION_SIZE / 2), _is_in_rect ? ENTITY_LIST_OPTION_SIZE - 2 : ENTITY_LIST_OPTION_SIZE - 4, buf, text_col, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);
      render->outlined_rect(pos, option_size, r);
      
      

      pos.y += ENTITY_LIST_OPTION_SIZE + (padding / 2);
    }
  }
};