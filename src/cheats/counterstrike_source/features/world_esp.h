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

    if(entity->is_active_grenade())
      return OBJECT_TYPE_GRENADE;
    else if(entity->is_dropped_weapon())
      return OBJECT_TYPE_DROPPED_WEAPON;
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
      case OBJECT_TYPE_OBJECTIVE: return L"Objective";
    }

    return L"";
  }

  bool should_draw(c_base_player* localplayer, c_base_entity* entity){
    if(localplayer == nullptr || entity == nullptr)
      return false;

    if(entity->is_dropped_weapon())
      return entity->owner_handle() == -1;

    if(!config->world_esp.render_enemies && localplayer->team() != entity->team() && entity->team() >= 2 && entity->team() <= 3)
      return false;

    if(!config->world_esp.render_friendies && localplayer->team() == entity->team())
      return false;

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
      case WORLD_ESP_RENDER_TYPE_TEXT:
      {

        if(config->visual.world_esp_use_team_colours){
          if(entity->team() == 2)
            draw_col = flt_array2clr(config->visual.world_esp_allied_team_colour);
          else if(entity->team() == 3)
            draw_col = flt_array2clr(config->visual.world_esp_axis_team_colour);
          else
            draw_col = flt_array2clr(config->visual.world_esp_unassigned_colour);
        }
        else if(config->visual.world_esp_use_enemyteam_colours){
          if(localplayer->team() != entity->team() && entity->team() >= 2 && entity->team() <= 3)
            draw_col = flt_array2clr(config->visual.world_esp_enemy_colour);
          else if(localplayer->team() == entity->team())
            draw_col = flt_array2clr(config->visual.world_esp_team_colour);
          else
            draw_col = flt_array2clr(config->visual.world_esp_unassigned_colour);
        }

        // TYPE LOGIC HERE
        if(type == OBJECT_TYPE_DROPPED_WEAPON)
          draw_col = flt_array2clr(config->visual.world_esp_unassigned_colour);

        if(config->visual.world_esp_aimbot_target){
          if(global->aimbot_target_index == entity->get_index())
            draw_col = flt_array2clr(config->visual.world_esp_aimbot_target_colour);
        }

        break;
      }
      case WORLD_ESP_RENDER_TYPE_CHAMS:
      {
        // Team colour.
        if(config->visual.world_chams_use_team_colours){
          if(entity->team() == 3)
            draw_col = flt_array2clr(config->visual.world_chams_axis_team_colour);
          else if(entity->team() == 2)
            draw_col = flt_array2clr(config->visual.world_chams_allied_team_colour);
          else
            draw_col = flt_array2clr(config->visual.world_chams_unassigned_colour);
        }
        else if(config->visual.world_chams_use_enemyteam_colours){ // Enemy and friendly.
          if(localplayer->team() != entity->team() && entity->team() >= 2 && entity->team() <= 3)
            draw_col = flt_array2clr(config->visual.world_chams_enemy_colour);
          else if(localplayer->team() == entity->team())
            draw_col = flt_array2clr(config->visual.world_chams_team_colour);
          else
            draw_col = flt_array2clr(config->visual.world_chams_unassigned_colour);
        }

        // TYPE LOGIC HERE
        if(type == OBJECT_TYPE_DROPPED_WEAPON)
          draw_col = flt_array2clr(config->visual.world_chams_unassigned_colour);

        if(config->visual.world_chams_aimbot_target){
          if(global->aimbot_target_index == entity->get_index())
            draw_col = flt_array2clr(config->visual.world_chams_aimbot_target_colour);
        }

        break;
      }
      case WORLD_ESP_RENDER_TYPE_CHAMS_OVERLAY:
      {
        // Team colour.
        if(config->visual.world_chams_use_team_colours){
          if(entity->team() == 3)
            draw_col = flt_array2clr(config->visual.world_chams_overlay_axis_team_colour);
          else if(entity->team() == 2)
            draw_col = flt_array2clr(config->visual.world_chams_overlay_allied_team_colour);
          else
            draw_col = flt_array2clr(config->visual.world_chams_overlay_unassigned_colour);
        }
        else if(config->visual.world_chams_use_enemyteam_colours){ // Enemy and friendly.
          if(localplayer->team() != entity->team() && entity->team() >= 2 && entity->team() <= 3)
            draw_col = flt_array2clr(config->visual.world_chams_overlay_enemy_colour);
          else if(localplayer->team() == entity->team())
            draw_col = flt_array2clr(config->visual.world_chams_overlay_team_colour);
          else
            draw_col = flt_array2clr(config->visual.world_chams_overlay_unassigned_colour);
        }

        // TYPE LOGIC HERE
        if(type == OBJECT_TYPE_DROPPED_WEAPON)
          draw_col = flt_array2clr(config->visual.world_chams_overlay_unassigned_colour);

        if(config->visual.world_chams_overlay_aimbot_target){
          if(global->aimbot_target_index == entity->get_index())
            draw_col = flt_array2clr(config->visual.world_chams_overlay_aimbot_target_colour);
        }

        break;
      }
      case WORLD_ESP_RENDER_TYPE_GLOW:
      {
        // Team colour.
        if(config->visual.world_glow_use_team_colours){
          if(entity->team() == 3)
            draw_col = flt_array2clr(config->visual.world_glow_axis_team_colour);
          else if(entity->team() == 2)
            draw_col = flt_array2clr(config->visual.world_glow_allied_team_colour);
          else
            draw_col = flt_array2clr(config->visual.world_glow_unassigned_team_colour);
        }
        else if(config->visual.world_glow_use_enemyteam_colours){ // Enemy and friendly.
          if(localplayer->team() != entity->team() && entity->team() >= 2 && entity->team() <= 3)
            draw_col = flt_array2clr(config->visual.world_glow_enemy_colour);
          else if(localplayer->team() == entity->team())
            draw_col = flt_array2clr(config->visual.world_glow_team_colour);
          else
            draw_col = flt_array2clr(config->visual.world_glow_unassigned_team_colour);
        }

        // TYPE LOGIC HERE
        if(type == OBJECT_TYPE_DROPPED_WEAPON)
          draw_col = flt_array2clr(config->visual.world_glow_unassigned_team_colour);

        if(config->visual.world_glow_aimbot_target){
          if(global->aimbot_target_index == entity->get_index())
            draw_col = flt_array2clr(config->visual.world_glow_aimbot_target_colour);
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

      uptr* model = entity->get_model();
      if(model == nullptr)
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

      s_box_data box_data = math::calculate_box_region(entity->obb_mins(), entity->obb_maxs(), entity->rgfl_coordinate_frame());
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