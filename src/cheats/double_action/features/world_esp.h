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

  u32 get_object_index(c_base_entity* entity){
    if(entity == nullptr)
      return OBJECT_TYPE_INVALID;

    s_client_class* cc = entity->get_client_class();
    if(cc == nullptr)
      return OBJECT_TYPE_INVALID;

    if(entity->is_dropped_weapon())
      return OBJECT_TYPE_DROPPED_WEAPON;
    else if(entity->is_active_grenade())
      return OBJECT_TYPE_GRENADE;
    else if(entity->is_objective())
      return OBJECT_TYPE_OBJECTIVE;

    return OBJECT_TYPE_INVALID; // TEMP MAKE IT OBJECT_TYPE_INVALID
  }

  bool should_render_entity(c_base_entity* entity, u32 type){
    if(entity == nullptr || type == OBJECT_TYPE_INVALID)
      return false;

    return config->world_esp.object_enabled[type];
  }

  std::wstring get_object_name(c_base_entity* entity, u32 type){
    if(entity == nullptr)
      return L"";

    switch(type){
      default: return L"";
      case OBJECT_TYPE_GRENADE: return entity->get_grenade_name();
      case OBJECT_TYPE_DROPPED_WEAPON: return entity->get_weapon_name();
      case OBJECT_TYPE_OBJECTIVE: return entity->get_objective_name();
    }

    return L"";
  }

  bool should_draw(c_base_player* localplayer, c_base_entity* entity){
    if(localplayer == nullptr || entity == nullptr)
      return false;

    if(entity->is_dropped_weapon())
      return entity->owner_handle == -1;

    return true;
  }

  bool should_draw_extra(c_base_entity* entity, u32 type, u32 render_type){
    if(entity == nullptr)
      return false;

    s_client_class* cc = entity->get_client_class();
    if(cc == nullptr)
      return false;


    return true;
  }

  colour get_draw_colour(c_base_entity* entity, i32 type, i32 colour_type = WORLD_ESP_RENDER_TYPE_TEXT){
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return rgb(255, 255, 255);

    colour draw_col                 = rgb(255,255,255);
    switch(colour_type){
      default: break;
      case WORLD_ESP_RENDER_TYPE_TEXT:
      {
        switch(type)
        {
          default: break;
          case OBJECT_TYPE_GRENADE:
          {
            draw_col = flt_array2clr(config->visual.world_esp_active_grenade_colour);
            break;
          }
          case OBJECT_TYPE_DROPPED_WEAPON:
          {
            draw_col = flt_array2clr(config->visual.world_esp_dropped_weapon_colour);
            break;
          }
          case OBJECT_TYPE_OBJECTIVE:
          {
            draw_col = flt_array2clr(config->visual.world_esp_objective_colour);
            break;
          }
        }
        break;
      }
      case WORLD_ESP_RENDER_TYPE_CHAMS:
      {
        switch(type)
        {
          default: break;
          case OBJECT_TYPE_GRENADE:
          {
            draw_col = flt_array2clr(config->visual.world_chams_active_grenade_colour);
            break;
          }
          case OBJECT_TYPE_DROPPED_WEAPON:
          {
            draw_col = flt_array2clr(config->visual.world_chams_dropped_weapon_colour);
            break;
          }
          case OBJECT_TYPE_OBJECTIVE:
          {
            draw_col = flt_array2clr(config->visual.world_chams_objective_colour);
            break;
          }
        }
        break;
      }
      case WORLD_ESP_RENDER_TYPE_CHAMS_OVERLAY:
      {
        switch(type)
        {
          default: break;
          case OBJECT_TYPE_GRENADE:
          {
            draw_col = flt_array2clr(config->visual.world_chams_overlay_active_grenade_colour);
            break;
          }
          case OBJECT_TYPE_DROPPED_WEAPON:
          {
            draw_col = flt_array2clr(config->visual.world_chams_overlay_dropped_weapon_colour);
            break;
          }
          case OBJECT_TYPE_OBJECTIVE:
          {
            draw_col = flt_array2clr(config->visual.world_chams_overlay_objective_colour);
            break;
          }
        }
        break;
      }
      case WORLD_ESP_RENDER_TYPE_GLOW:
      {
        switch(type)
        {
          default: break;
          case OBJECT_TYPE_GRENADE:
          {
            draw_col = flt_array2clr(config->visual.world_glow_active_grenade_colour);
            break;
          }
          case OBJECT_TYPE_DROPPED_WEAPON:
          {
            draw_col = flt_array2clr(config->visual.world_glow_dropped_weapon_colour);
            break;
          }
          case OBJECT_TYPE_OBJECTIVE:
          {
            draw_col = flt_array2clr(config->visual.world_glow_objective_colour);
            break;
          }
        }
        break;
      }
    }

    return draw_col;
  }

  void run(){
    if(!config->world_esp.enabled)
      return;

    if(!global->engine->is_in_game())
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    for(i32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
      c_base_entity* entity = global->entity_list->get_entity(i);
      if(entity == nullptr)
        continue;

      if(entity->is_dormant())
        continue;

      if(entity->origin().distance_meter(localplayer->origin()) > 32)
        continue;

      uptr* model = entity->get_model();
      if(model == nullptr)
        continue;

      u32 type = get_object_index(entity);
      if(type == OBJECT_TYPE_INVALID && !input_system->held(VK_HOME))
        continue;

      assert(type >= 0);
      if(!should_render_entity(entity, type) && !input_system->held(VK_HOME))
        continue;

      if(!should_draw(localplayer, entity))
        continue;

      if(!should_draw_extra(localplayer, type, WORLD_ESP_RENDER_TYPE_TEXT))
        continue;

      s_box_data box_data = math::calculate_box_region(entity->obb_mins(), entity->obb_maxs(), entity->rgfl_coordinate_frame);
      if(!box_data.valid)
        continue;

      // box thinning
      {
        box_data.size.x = math::biggest(box_data.size.x, 8);
        box_data.size.y = math::biggest(box_data.size.y, 8);
      }

      c_esp_data<32> ex_data;

      // There is no entity in this game that requires a health bar.
      if(type == OBJECT_TYPE_INVALID){ // !!! DEBUG !!!
        static i8 model_name[256];
        wsprintfA(model_name, "%s %s", entity->get_client_class()->name, global->model_info->get_model_name(model));

        static wchar_t buf[256];
        convert::str2wstr(model_name, buf);
        ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 255, 255), global->esp_font_large, 12, buf);
      }

      // Name
      if(config->world_esp.name[type])
        ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 255, 255), global->esp_font_large, 12, get_object_name(entity, type));

      //Flags
      if(config->world_esp.distance[type]){
        static wchar_t dist_buf[17]; // TODO: CACHE ME!!!
        wsprintfW(dist_buf, L"[ %im ]", localplayer->origin().distance_meter(entity->origin()));

        ex_data.add(ESP_TEXT_TYPE_BOTTOM, rgb(255, 255, 255), global->esp_font_small, 10, dist_buf);
      }


      u32 esp_box_flags = ESP_FLAG_TEXT_OUTLINES;
      {
        if(config->world_esp.box[type])
          esp_box_flags |= ESP_FLAG_BOX;

        if(config->world_esp.box_outlines[type])
          esp_box_flags |= ESP_FLAG_OUTLINES;
      }

      render->draw_esp_box(box_data.pos, box_data.size, get_draw_colour(entity, type), esp_box_flags, &ex_data);
    }

  }
};

CLASS_EXTERN(c_world_esp, world_esp);