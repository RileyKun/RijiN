#include "../link.h"

CLASS_ALLOC(c_world_esp, world_esp);


void c_world_esp::run(){
  if(!config->world_esp.enabled)
    return;

  if(!global->engine->is_in_game())
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  for(i32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
    s_entity_info* info = entity_info->get_data(i);
    if(info == nullptr)
      continue;

    if(info->model_ptr == nullptr)
      continue;

    c_base_entity* entity = global->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    if(entity->is_dormant())
      continue;

    float dist = (float)info->meter_dist;
    config->world_esp.world_esp_render_dist = math::clamp(config->world_esp.world_esp_render_dist, 10.f, 100.f);
    if(dist >= config->world_esp.world_esp_render_dist)
      continue;

    c_entity_data* data = entity->data();
    if(data == nullptr)
      continue;

    if(!data->has_transform)
      continue;

    u32 type = get_object_index(entity);
    if(type == OBJECT_TYPE_INVALID)
      continue;

    assert(type >= 0);

    if(!should_render_entity(entity, type))
      continue;

    if(!should_draw(localplayer, entity))
      continue;

    if(!should_draw_extra(localplayer, type, WORLD_ESP_RENDER_TYPE_TEXT))
      continue;

    s_box_data box_data = math::calculate_box_region(info->obb_min, info->obb_max, data->transform);
    if(!box_data.valid)
      continue;

    // box thinning
    {
      box_data.size.x = math::biggest(box_data.size.x, 8);
      box_data.size.y = math::biggest(box_data.size.y, 8);
    }

    c_esp_data<32> ex_data;

    // Name
    if(config->world_esp.name[type])
      ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255, 255, 255), global->esp_font_large, 12, get_object_name(entity, type));

    //Flags
    if(config->world_esp.distance[type]){
      static wchar_t dist_buf[17]; // TODO: CACHE ME!!!
      wsprintfW(dist_buf, WXOR(L"[ %im ]"), info->meter_dist);

      ex_data.add(ESP_TEXT_TYPE_BOTTOM, rgb(255, 255, 255), global->esp_font_small, 10, dist_buf);
    }

    u32 esp_box_flags = ESP_FLAG_TEXT_OUTLINES;
    {
      if(config->world_esp.box[type])
        esp_box_flags |= ESP_FLAG_BOX;

      if(config->world_esp.box_outlines[type])
        esp_box_flags |= ESP_FLAG_OUTLINES;
    }

    render->draw_esp_box(box_data.pos, box_data.size, get_draw_colour(entity), esp_box_flags, &ex_data);
  }

}

u32 c_world_esp::get_object_index(c_base_entity* entity){
  if(entity == nullptr)
    return OBJECT_TYPE_INVALID;

  s_client_class* cc = entity->get_client_class();
  if(cc == nullptr)
    return OBJECT_TYPE_INVALID;

  if(entity->is_active_grenade())
    return OBJECT_TYPE_GRENADE;

  if(entity->is_objective())
    return OBJECT_TYPE_OBJECTIVE;

  if(entity->is_dropped_weapon())
    return OBJECT_TYPE_DROPPED_WEAPON;

  if(entity->is_dropped_item())
    return OBJECT_TYPE_ITEMS;


  return OBJECT_TYPE_INVALID;
}

bool c_world_esp::should_render_entity(c_base_entity* entity, u32 type){
  if(entity == nullptr)
    return false;

  if(type >= 6){
    assert(false && "object_enabled overflow");
    return false;
  }

  return config->world_esp.object_enabled[type];
}

std::wstring c_world_esp::get_object_name(c_base_entity* entity, u32 type){
  if(entity == nullptr)
    return L"";

  switch(type){
    default: return L"";
    case OBJECT_TYPE_DROPPED_WEAPON: return entity->get_dropped_weapon_name();
    case OBJECT_TYPE_ITEMS: return entity->get_dropped_item_name();
    case OBJECT_TYPE_GRENADE: return entity->get_grenade_name();
    case OBJECT_TYPE_OBJECTIVE: return WXOR(L"Objective");
  }

  return L"";
}

bool c_world_esp::should_draw(c_base_player* localplayer, c_base_entity* entity){
  if(localplayer == nullptr || entity == nullptr)
    return false;

  if(entity->owner_handle() != -1 && !entity->is_active_grenade())
    return false;

  return true;
}

bool c_world_esp::should_draw_extra(c_base_entity* entity, u32 type, u32 render_type){
  if(entity == nullptr)
    return false;

  s_client_class* cc = entity->get_client_class();
  if(cc == nullptr)
    return false;

  return true;
}

CFLAG_O0 colour c_world_esp::get_draw_colour(c_base_entity* entity, i32 colour_type){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return rgba(255, 255, 255, 255);

  colour draw_col                 = rgba(255, 255, 255, 255);
  switch(colour_type){
    default: break;
    case WORLD_ESP_RENDER_TYPE_TEXT:
    {
      if(entity->is_health_item())
        draw_col = flt_array2clr(config->visual.world_esp_health_items_colour);
      else if(entity->is_active_grenade())
        draw_col = flt_array2clr(config->visual.world_esp_grenade_colour);
      else if(entity->is_dropped_weapon())
        draw_col = flt_array2clr(config->visual.world_esp_dropped_weapon_colour);
      else if(entity->is_dropped_item())
        draw_col = flt_array2clr(config->visual.world_esp_dropped_items_colour);

      if(entity->is_objective())
        draw_col = flt_array2clr(config->visual.world_esp_objective_colour);

      draw_col.w = 255;
      break;
    }
    case WORLD_ESP_RENDER_TYPE_CHAMS:
    {
      if(entity->is_health_item())
        draw_col = flt_array2clr(config->visual.world_chams_health_items_colour);
      else if(entity->is_active_grenade())
        draw_col = flt_array2clr(config->visual.world_chams_grenade_colour);
      else if(entity->is_dropped_weapon())
        draw_col = flt_array2clr(config->visual.world_chams_dropped_weapon_colour);
      else if(entity->is_dropped_item())
        draw_col = flt_array2clr(config->visual.world_chams_dropped_items_colour);

      if(entity->is_objective())
        draw_col = flt_array2clr(config->visual.world_chams_objective_colour);

      break;
    }
    case WORLD_ESP_RENDER_TYPE_CHAMS_OVERLAY:
    {
      if(entity->is_health_item())
        draw_col = flt_array2clr(config->visual.world_chams_overlay_health_items_colour);
      else if(entity->is_active_grenade())
        draw_col = flt_array2clr(config->visual.world_chams_overlay_grenade_colour);
      else if(entity->is_dropped_weapon())
        draw_col = flt_array2clr(config->visual.world_chams_overlay_dropped_weapon_colour);
      else if(entity->is_dropped_item())
        draw_col = flt_array2clr(config->visual.world_chams_overlay_dropped_items_colour);

      if(entity->is_objective())
        draw_col = flt_array2clr(config->visual.world_chams_overlay_objective_colour);

      break;
    }
    case WORLD_ESP_RENDER_TYPE_GLOW:
    {
      if(entity->is_health_item())
        draw_col = flt_array2clr(config->visual.world_glow_health_items_colour);
      else if(entity->is_active_grenade())
        draw_col = flt_array2clr(config->visual.world_glow_grenade_colour);
      else if(entity->is_dropped_weapon())
        draw_col = flt_array2clr(config->visual.world_glow_dropped_weapon_colour);
      else if(entity->is_dropped_item())
        draw_col = flt_array2clr(config->visual.world_glow_dropped_items_colour);

      if(entity->is_objective())
        draw_col = flt_array2clr(config->visual.world_glow_objective_colour);

      draw_col.w = 255;
      break;
    }
  }

  return draw_col;
}