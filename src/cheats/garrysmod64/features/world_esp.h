#pragma once

enum objects_type{
  OBJECT_TYPE_INVALID = -1,
  OBJECT_TYPE_GRENADE,
  OBJECT_TYPE_DROPPED_WEAPON,
  OBJECT_TYPE_OBJECTIVE,

  OBJECT_TYPE_MAX,
};

enum world_esp_render_type{
  WORLD_ESP_RENDER_TYPE_TEXT = 0,
  WORLD_ESP_RENDER_TYPE_CHAMS,
  WORLD_ESP_RENDER_TYPE_GLOW,
  WORLD_ESP_RENDER_TYPE_CHAMS_OVERLAY,
};

class c_world_esp{
public:

  bool should_render_entity(c_base_entity* entity){
    if(entity == nullptr)
      return false;

    return true;
  }

  std::wstring get_object_name(c_base_entity* entity){
    if(entity == nullptr)
      return L"";

    if(entity->is_npc())
      return L"NPC";

    std::wstring print_name = entity->get_print_name();
    if(print_name.length() > 0)
      return print_name;
    else{
      i8* class_name = entity->get_scripted_class_name();
      if(class_name == nullptr)
        return L"";

      static wchar_t buf[1024];
      convert::str2wstr(class_name, buf, sizeof(buf));
      return buf;
    }
  
    return L"";
  }

  bool should_draw(c_base_player* localplayer, c_base_entity* entity){
    if(localplayer == nullptr || entity == nullptr)
      return false;

    if(entity->owner_handle != -1)
      return false;
    
    return true;
  }

  bool should_draw_extra(c_base_entity* entity, u32 render_type){
    if(entity == nullptr)
      return false;

    s_client_class* cc = entity->get_client_class();
    if(cc == nullptr)
      return false;


    return true;
  }

  colour get_draw_colour(c_base_entity* entity, i32 colour_type = WORLD_ESP_RENDER_TYPE_TEXT){
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return rgb(255, 255, 255);

    colour draw_col                 = rgb(255,255,255);
    switch(colour_type){
      default: break;
      case WORLD_ESP_RENDER_TYPE_TEXT:
      {

        c_world_esp_settings* settings = utils::get_world_esp_settings(entity);
        if(settings == nullptr)
          return draw_col;

        draw_col   = flt_array2clr(settings->esp_colour);

        if(settings->aimbot_target && entity->get_index() == global->aimbot_target_index)
          draw_col = flt_array2clr(settings->esp_aimbot_colour);

        draw_col.w = 255;


        break;
      }
      case WORLD_ESP_RENDER_TYPE_CHAMS:
      {
        c_world_chams_settings* settings = utils::get_world_chams_settings(entity);
        if(settings == nullptr)
          return draw_col;

        draw_col = flt_array2clr(settings->chams_colour_base);
        if(settings->aimbot_target && entity->get_index() == global->aimbot_target_index)
          draw_col = flt_array2clr(settings->chams_aimbot_base_colour);

        break;
      }
      case WORLD_ESP_RENDER_TYPE_CHAMS_OVERLAY:
      {
        c_world_chams_settings* settings = utils::get_world_chams_settings(entity);
        if(settings == nullptr)
          return draw_col;

        draw_col = flt_array2clr(settings->chams_colour_overlay);
        if(settings->aimbot_target_overlay && entity->get_index() == global->aimbot_target_index)
          draw_col = flt_array2clr(settings->chams_aimbot_overlay_colour);

        break;
      }
      case WORLD_ESP_RENDER_TYPE_GLOW:
      {
        c_world_glow_settings* settings = utils::get_world_glow_settings(entity);
        if(settings == nullptr)
          return draw_col;

        draw_col = flt_array2clr(settings->glow_colour);
        if(settings->aimbot_target  && entity->get_index() == global->aimbot_target_index)
          draw_col = flt_array2clr(settings->glow_aimbot_colour);
        else if(config->world_glow.aimbot_target && entity->get_index() == global->aimbot_target_index)
          draw_col = flt_array2clr(config->world_glow.glow_aimbot_colour);
        
        break;
      }

    }
    
    // I removed everything here. Because it's going to have to be redesigned anyways.
    return draw_col;
  }

  void run(){
    if(!global->engine->is_in_game())
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    #if defined(DEV_MODE)
      if(input_system->held(VK_HOME)){
        for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
          c_base_entity* entity = global->entity_list->get_entity(i);
          if(entity == nullptr)
            continue;
    
          if(entity->is_dormant())
            continue;

          if(localplayer->origin().distance_meter(entity->origin()) > 12)
            continue;
    
          i8 buf[256];
          wsprintfA(buf, XOR("%s"), entity->get_client_class()->name);
          render_debug->draw_text(entity->obb_center(), colour(255, 255, 255, 255), buf, 1, globaldata->interval_per_tick * 2.f);
            
          uptr* model = entity->get_model();
          if(model != nullptr){
            i8* model_name = global->model_info->get_model_name(model);
            if(model_name != nullptr){
              wsprintfA(buf, XOR("%s"), model_name);
              render_debug->draw_text(entity->obb_center(), colour(255, 255, 255, 255), buf, 2, globaldata->interval_per_tick * 2.f);
            }
          }
          else
            render_debug->draw_text(entity->obb_center(), colour(255, 255, 255, 255), XOR("NO MODEL"), 2, globaldata->interval_per_tick * 2.f);
          
          i8* script_name = entity->get_scripted_class_name();
          if(script_name != nullptr){
            wsprintfA(buf, XOR("%X"), entity);
            render_debug->draw_text(entity->obb_center(), colour(0, 255, 0, 255), buf, 3, globaldata->interval_per_tick * 2.f);

            wsprintfA(buf, XOR("%s"), script_name);
            render_debug->draw_text(entity->obb_center(), colour(0, 255, 0, 255), buf, 4, globaldata->interval_per_tick * 2.f);
          }

          std::wstring pretty_name = entity->get_print_name();
          if(!pretty_name.empty()){
            wsprintfA(buf, XOR("%ls"), pretty_name.c_str());
            render_debug->draw_text(entity->obb_center(), colour(255, 255, 225, 255), buf, 5, globaldata->interval_per_tick * 2.f);
          }
        }
      }
    #endif

    for(i32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
      c_base_entity* entity = global->entity_list->get_entity(i);
      if(entity == nullptr)
        continue;

      if(entity->is_dormant())
        continue;

      if(!entity->should_draw())
        continue;

      c_entity_data* data = entity->data();
      if(data == nullptr)
        continue;

      if(!data->list_data.render)
        continue;
      
      if(!data->has_transform)
        continue;
         
      c_world_esp_settings* settings = utils::get_world_esp_settings(entity);
      if(settings == nullptr)
        continue;

      if(settings->max_distance > 0 && localplayer->origin().distance_meter(entity->origin()) > settings->max_distance)
        continue;
      
      if(!settings->object_enabled && data->list_data.use_custom_settings)
        continue;
      
      if(!should_render_entity(entity))
        continue;
      
      if(!should_draw(localplayer, entity))
        continue;
      
      if(!should_draw_extra(localplayer, WORLD_ESP_RENDER_TYPE_TEXT))
        continue;
      
      s_box_data box_data = math::calculate_box_region(entity->obb_mins(), entity->obb_maxs(), data->transform);
      if(!box_data.valid)
        continue;
         
      // box thinning
      {
        box_data.size.x = math::biggest(box_data.size.x, 8);
        box_data.size.y = math::biggest(box_data.size.y, 8);
      }

      c_esp_data<32> ex_data;

      // There is no entity in this game that requires a health bar.

      // Name
      if(settings->name)
        ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 255, 255), global->esp_font_large, 12, get_object_name(entity));

      //Flags

      if(settings->distance){
        static wchar_t dist_buf[17]; // TODO: CACHE ME!!!
        wsprintfW(dist_buf, L"[ %im ]", localplayer->origin().distance_meter(entity->origin()));

        ex_data.add(ESP_TEXT_TYPE_BOTTOM, rgb(255, 255, 255), global->esp_font_small, 10, dist_buf);
      }


      u32 esp_box_flags = config->visual.cheap_text_rendering ? 0 : ESP_FLAG_TEXT_OUTLINES;
      {
        if(settings->box) // box
          esp_box_flags |= ESP_FLAG_BOX;

        if(settings->box_outlines) // box outline
          esp_box_flags |= ESP_FLAG_OUTLINES;
      }

      render->draw_esp_box(box_data.pos, box_data.size, get_draw_colour(entity), esp_box_flags, &ex_data);
    }

  }
};

CLASS_EXTERN(c_world_esp, world_esp);