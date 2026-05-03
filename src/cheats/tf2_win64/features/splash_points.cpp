#include "../link.h"

CLASS_ALLOC(c_splash_point, splash_point);
float c_splash_point::get_max_splash_points(c_base_weapon* wep){
  if(wep == nullptr)
    return 0.f;

  const bool is_vphysics = proj_simulate->is_vphysics_weapon(wep);
  const u32  cores       = math::biggest(cpu::get_core_count(true), 2);

  float base  = is_vphysics ? MAX_POINTS_GENERATED * 0.25f : MAX_POINTS_GENERATED;
  if(cores <= 2)
    base *= 0.5f;

  if(cores <= 8){
    i32 ratio_cores = math::biggest((8 - (i32)cores) + 2, 2);
    base /= (float)ratio_cores;
  }

  return math::biggest(base, 4.f);
}

void c_splash_point::update_weapon_cache(c_base_weapon* wep){
  if(wep == nullptr)
    return;

  if(!wep->is_splash_weapon())
    return;

  i32 slot = wep->get_slot();
  if(slot >= 3)
    return;

  float blast_radius = wep->get_blast_radius();
  if(weapon_id[slot] == wep->weapon_id() && cur_blast_radius[slot] == blast_radius)
    return;

  weapon_id[slot]              = wep->weapon_id();
  cur_blast_radius[slot]       = blast_radius;
  cur_blast_radius_sqr[slot]   = blast_radius * blast_radius;

  DBG("[!] c_splash_point::update_weapon_cache %i - %2.2f - %2.2f\n", weapon_id[slot], cur_blast_radius[slot], cur_blast_radius_sqr[slot]);
}

void c_splash_point::generate_base_points(){
  if(global->aimbot_settings == nullptr)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return;

  if(!wep->is_splash_weapon())
    return;

  i32 slot = wep->get_slot();
  if(slot >= 3)
    return;

  const  float max_points = get_max_splash_points(wep);
  static float aoa        = math::pi() * math::deg_2_rad(25.f);

  if((max_points * 2) >= MAX_SPLASH_GEN_POINTS){
    DBG("[-] max_points too big lol: %f/%f\n", max_points * 2, MAX_SPLASH_GEN_POINTS);
    return;
  }

  update_weapon_cache(wep);
  if(!should_generate(wep)){
      //#define SPLASH_POINT_DEBUG
      #if defined(SPLASH_POINT_DEBUG)
      #if defined(DEV_MODE)
        for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
          if(i == global->localplayer_index)
            continue;
    
          c_base_entity* entity = global->entity_list->get_entity(i);
          if(entity == nullptr)
            continue;
    
          if(entity->is_dormant())
            continue;
    
          if(does_entity_have_points(entity)){
            c_base_player* player = entity->get_player();
            if(player != nullptr){
              c_player_data* data = player->data();
              if(data == nullptr)
                continue;
            
              for(i32 i = 0; i < data->splash_point_count; i++)
                render_debug->draw_3dbox(data->splash_points[i], vec3(-2.f, -2.f, -2.f), vec3(2.f, 2.f, 2.f), vec3(0.f, 0.f, 0.f), colour(255, 0, 0, 32), colour(0, 0, 0, 255), globaldata->interval_per_tick* 2.f);

              break;
            }
            else{
              c_entity_data* data = entity->entity_data();
              if(data == nullptr)
                continue;
    
              for(i32 i = 0; i < data->splash_point_count; i++)
                render_debug->draw_3dbox(data->splash_points[i], vec3(-2.f, -2.f, -2.f), vec3(2.f, 2.f, 2.f), vec3(0.f, 0.f, 0.f), colour(255, 0, 0, 32), colour(0, 0, 0, 255), globaldata->interval_per_tick* 2.f);

              break;
            }
          }
        }
      #endif
      #endif


    return;
  }

  if(generating_points[slot])
    return;

  generating_base = true;
  // Make sure we reset the point to zero.
  splash_point_count[slot] = 0;

  s_splash_thread_data* thread_data = (s_splash_thread_data*)malloc(sizeof(s_splash_thread_data));

  thread_data->max_points = max_points;
  thread_data->aoa        = aoa;
  thread_data->weapon_id  = wep->weapon_id();
  thread_data->slot       = slot;
  thread_data->step_size  = get_step_size();

  if(!utils::create_worker([](s_splash_thread_data* data){
    DBG("[+] Worker thread: Calculating base points for weapon slot: %i\n", data->slot);
    for(float mod = -1.f; mod <= 1.f; mod += 2.f){
      for(float x = 0.f; x < data->max_points; x += data->step_size){
        float p = math::acos(mod * (x / data->max_points));
        float y = data->aoa * x;
        
        // Times this position by the radius of the splash.
        i32 c                  = splash_point->splash_point_count[data->slot];
        splash_point->splash_points[data->slot][c] = vec3(math::sin(p) * math::cos(y), math::sin(p) * math::sin(y), math::cos(p));
  
        splash_point->splash_point_count[data->slot]++;
        if(splash_point->splash_point_count[data->slot] >= MAX_SPLASH_GEN_POINTS){
          DBG("[-] TOO MANY SPLASH POINTS BEING CREATED!!\n")
          break;
        }
      }
    }

    // We're done.
    splash_point->splash_weapon_id[data->slot]   = data->weapon_id;
    splash_point->splash_radius_size[data->slot] = splash_point->cur_blast_radius[data->slot];
    splash_point->splash_max_points[data->slot]  = data->max_points;
    splash_point->splash_step_size[data->slot]   = data->step_size;
    splash_point->generating_points[data->slot]  = false;

    splash_point->generating_base = false;
    free(data);
    data = nullptr;
    return 0;
  }, thread_data))
    free(thread_data);
}

float c_splash_point::calculate_radius(c_base_entity* entity, i32 slot){
  if(entity == nullptr)
    return cur_blast_radius[slot];

  if(cur_blast_radius[slot] <= 0.0f)
    return cur_blast_radius[slot];

  if(!entity->is_player())
    return cur_blast_radius[slot];

  c_base_player* player = entity->get_player();
  if(player == nullptr)
    return false;

  float blast_radius = cur_blast_radius[slot];

  s_lc_data* lc_data = lag_compensation->get_data(player);
  if(lc_data == nullptr)
    return false;

  {
    s_lc_record* cur  = lc_data->get_record(0);
    s_lc_record*  last = lc_data->get_record(1);
    if(cur == nullptr || last == nullptr)
      return blast_radius * 0.5f;

    if(math::abs(cur->simulation_time - last->simulation_time) >= math::ticks_to_time(6))
      blast_radius *= 0.95f;
  }

  float reduce = 0.0f;
  for(i32 i = 1; i < lc_data->max_records; i++){
    s_lc_record* record      = lc_data->get_record(i);
    s_lc_record* last_record = lc_data->get_record(i - 1);
    if(record == nullptr || last_record == nullptr)
      continue;

    vec3 last_vel_ang;
    vec3 cur_vel_ang;

    math::vector_2_angle(&last_record->velocity, &last_vel_ang);
    math::vector_2_angle(&record->velocity, &cur_vel_ang);

    float delta = math::abs(math::normalize_angle(last_vel_ang.y - cur_vel_ang.y));
    if(delta > 15.f)
      reduce += 0.01f;
  }

  return blast_radius * (1.f - math::smallest(reduce, 0.05f));
}

bool c_splash_point::get_valid_points_to_entity(c_base_entity* entity, vec3 origin, vec3 obb_center){
  if(entity == nullptr)
    return false;

  if(!is_valid_splash_entity(entity))
    return false;

  if(!should_update_entity(entity))
    return does_entity_have_points(entity);
  
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return false;

  i32 slot = wep->get_slot();
  if(slot >= 3)
    return false;

  const float blast_radius     = calculate_radius(entity, slot);
  const float blast_radius_sqr = blast_radius * blast_radius;

  const vec3* base_points      = splash_points[slot];
  const u32   base_count       = splash_point_count[slot];

  if(base_count <= 0){
    DBG("[-] No base splash points were calculated! (%i)\n", base_count);
    return false;
  }

  if(!global->aimbot_settings->splash_bot_fallback && !global->aimbot_settings->splash_bot_prefer)
    global->aimbot_settings->splash_bot_fallback = true;

  vec3 vel              = entity->est_velocity();
  c_base_player* player = entity->get_player();
  if(player != nullptr){

    s_lc_data* lc_data = lag_compensation->get_data(player);
    if(lc_data != nullptr){

      vec3 vel_avg;
      u32  samples = 0;

      float last_delta = 0.f;
      for(u32 i = 1; i < lc_data->max_records; i++){
        s_lc_record* last_record = lc_data->get_record(i - 1);
        s_lc_record* record      = lc_data->get_record(i);
        if(record == nullptr)
          continue;

        vec3 vel = last_record->velocity;
        vec3 last_vel_ang;
        vec3 cur_vel_ang;
    
        math::vector_2_angle(&last_record->velocity, &last_vel_ang);
        math::vector_2_angle(&record->velocity, &cur_vel_ang);
    
        float delta = math::abs(math::normalize_angle(last_vel_ang.y - cur_vel_ang.y));

        if(delta > 15.f){
          vel_avg = entity->velocity();
          samples = 0;
        }
   
        last_delta = delta;
        vel_avg += vel;
        samples++;
      }

      if(samples > 0)
        vel = vel_avg / (float)samples;

      // We're going to assume if players are using fakelag lets half the velocity so we can avoid issues caused by fakelag.
      i32 choke_ticks = math::time_to_ticks(math::abs(player->old_simulation_time() - player->simulation_time()));
      if(choke_ticks >= 6)
        vel *= 0.5f;
    }
  }

  if(global->aimbot_settings->splash_bot_prefer){
    vel        = vel * 0.1f;
    origin     = utils::trace_line(entity->origin(), entity->origin() + vel, mask_playersolid, TR_CUSTOM_FILTER_NO_TEAM_BASED_ENTS);
    obb_center = (origin + (entity->obb_mins() + entity->obb_maxs()) * 0.5f);
  }

  vec3 wanted_points[MAX_ENTITY_DATA_SPLASH_POINTS];
  u32  wanted_points_count = 0;

  vec3 old_abs_origin = entity->get_abs_origin();

  // So traces can work.
  if(global->aimbot_settings->splash_bot_fallback)
    entity->set_abs_origin(origin);

  static u32 surface_points_created[61];
  static u32 plane_points_created[3];
  static u32 max_points_per_surface = 4;
  memset(&surface_points_created, 0, sizeof(surface_points_created));
  memset(&plane_points_created, 0, sizeof(plane_points_created));

  vec3 last_point;
  {
    for(u32 i = 0; i < base_count; i++){
      vec3      new_point = obb_center + (base_points[i] * 1024.f);
      const u32 dir       = get_trace_direction(obb_center, new_point);
      if(surface_points_created[dir] >= surface_points_created)
        continue;

      surface_points_created[dir]++;
      s_trace    pts = global->trace->ray(obb_center, new_point, PROJ_TRACE_MASK, TRACE_EVERYTHING, entity, TR_CUSTOM_FILTER_SPLASH_BOT_TEST);
      if(pts.fraction >= 1.f || has_hit_bad_entity(pts.hit_entity))
        continue;

      const u32 plane_type = get_surface_type(pts.plane.normal);
      if(plane_type == 0){
        if(plane_points_created[plane_type] >= 195)
          continue;
      }

      vec3 nearest_point;
      entity->calc_nearest_point(pts.end, &nearest_point);
      if((pts.end - nearest_point).length_sqr() >= blast_radius_sqr)
        continue;

      if(i){
        if(pts.end.distance(last_point) <= 16.f)
          continue;
      }

      if(pts.end.distance(origin) <= 24.f)
        continue;

      last_point = pts.end;
      wanted_points[wanted_points_count] = pts.end;
      wanted_points_count++;
      plane_points_created[plane_type]++;
      if(wanted_points_count >= MAX_ENTITY_DATA_SPLASH_POINTS)
        break;  
    }
  }

  DBG("[!] created %i points out of %i\nFloor: %i\nCeiling: %i\nWalls: %i\n", wanted_points_count, base_count, plane_points_created[0], plane_points_created[1], plane_points_created[2]);

  if(global->aimbot_settings->splash_bot_fallback)
    entity->set_abs_origin(old_abs_origin);

  return update_entity_points(entity, wanted_points, wanted_points_count);
}

bool c_splash_point::has_hit_bad_entity(c_base_entity* entity){
  if(entity == nullptr)
    return true;

  if(entity->is_door())
    return true;

  return false;
}

bool c_splash_point::is_valid_splash_entity(c_base_entity* entity){
  if(entity == nullptr)
    return false;

  if(entity->is_dormant())
    return false;

  return entity->is_player() || entity->is_sentry();
}

bool c_splash_point::should_generate(c_base_weapon* wep){
  if(wep == nullptr)
    return false;

  i32 slot = wep->get_slot();
  if(slot >= 3)
    return false;

  if(cur_blast_radius[slot] <= 0.0f)
    return false;

  return    splash_max_points[slot] != get_max_splash_points(wep)
          || splash_weapon_id[slot] != wep->weapon_id() 
          || splash_radius_size[slot] != wep->get_blast_radius()
          || splash_step_size[slot] != get_step_size();
}

bool c_splash_point::should_update_entity(c_base_entity* entity){
  if(entity == nullptr)
    return false;

  if(!is_valid_splash_entity(entity))
    return false;
  
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  c_base_player* player = entity->get_player();
  if(player != nullptr){
    c_player_data* data = player->data();
    if(data == nullptr)
      return false;

    if(player->team() == global->localplayer_team)
      return false;
    
    if(!player->is_alive())
      return false;
   
    if(!localplayer->is_considered_enemy(player))
      return false;
    
    // Only update every entity update.
    if(data->last_splash_point_sim_time >= player->simulation_time())
      return false;
    
    // Don't bother recalculating the points if the position is the same.
    if(data->last_splash_point_origin == player->origin() && math::abs(data->last_splash_point_sim_time - player->simulation_time()) <= 5.f)
      return false;

    return true;
  }
  else if(entity->is_sentry()){ // We could probably add support for other entities but it's a waste of resources.
    c_entity_data* data = entity->entity_data();
    if(data == nullptr)
      return false;

    // Only update every entity update.
    if(data->last_splash_point_sim_time >= entity->simulation_time())
      return false;

    // Don't bother recalculating the points if the position is the same.
    if(data->last_splash_point_origin == entity->origin())
      return false;

    return true;
  }

  DBG("How often did this occur? %s\n", entity->get_client_class()->name);
  return false;
}

bool c_splash_point::does_entity_have_points(c_base_entity* entity){
  if(entity == nullptr)
    return false;

  c_base_player* player = entity->get_player();
  if(player != nullptr){
    c_player_data* data = player->data();
    if(data == nullptr)
      return false;

    return data->splash_point_count != 0;
  }
  else{
    c_entity_data* data = entity->entity_data();
    if(data == nullptr)
      return false;

    return data->splash_point_count != 0;
  }

  return false;
}

bool c_splash_point::update_entity_points(c_base_entity* entity, vec3* points_list, u32 count){
  if(entity == nullptr)
    return false;

  c_base_player* player = entity->get_player();
  if(player != nullptr){
    c_player_data* data = player->data();
    if(data == nullptr)
      return false;

    data->last_splash_point_sim_time = player->simulation_time();
    data->last_splash_point_origin   = player->origin();
    data->splash_point_count         = count;

    memcpy(&data->splash_points, points_list, sizeof(data->splash_points));
    sort_splash_points(player->origin(), data->splash_points, data->splash_point_count);
    if(count <= 0 || data->splash_point_count <= 0){
      DBG("[-] error splash points for a player (%i) (%i)\n", count, data->splash_point_count);
    }
  }
  else{
    c_entity_data* data = entity->entity_data();
    if(data == nullptr)
      return false;

    data->last_splash_point_sim_time = entity->simulation_time();
    data->last_splash_point_origin   = entity->origin();
    data->splash_point_count         = count;

    memcpy(&data->splash_points, points_list, sizeof(data->splash_points));
    sort_splash_points(entity->origin(), data->splash_points, data->splash_point_count);
    DBG("[!] Updated splash points for an entity (%i) (%i)\n", count, data->splash_point_count);
  }

  return count > 0;
}

bool c_splash_point::delete_entity_points(c_base_entity* entity){
  if(entity == nullptr)
    return false;

  c_base_player* player = entity->get_player();
  if(player != nullptr){
    c_player_data* data = player->data();
    if(data == nullptr)
      return false;

    data->splash_point_count         = 0;
    data->last_splash_point_sim_time = 0.f;
    data->last_splash_point_origin   = vec3();
  }
  else{
    c_entity_data* data = entity->entity_data();
    if(data == nullptr)
      return false;

    data->splash_point_count         = 0;
    data->last_splash_point_sim_time = 0.f;
    data->last_splash_point_origin   = vec3();
  }

  return true;
}

bool c_splash_point::sort_splash_points(vec3 cur_pos, vec3* splash_points, u32 size){
  if(splash_points == nullptr || size == 0)
    return false;

  s_splash_point_sort* thread_data = malloc(sizeof(s_splash_point_sort));
  if(thread_data == nullptr)
    return false;

  thread_data->cur_pos = cur_pos;
  thread_data->o = splash_points;
  thread_data->p = malloc(sizeof(vec3) * size);
  thread_data->s = size;

  memcpy(thread_data->p, thread_data->o, sizeof(vec3) * size);
  if(!utils::create_worker([](s_splash_point_sort* thread_data){
  
    float sort_time = math::time(true, true);
    splash_point->q_sort(thread_data->p, 0, thread_data->s - 1, thread_data->cur_pos);

    memcpy(thread_data->o, thread_data->p, sizeof(vec3) * thread_data->s);

    free(thread_data->p);
    free(thread_data);
    thread_data = nullptr;
    return 0;
  }, thread_data)){
    free(thread_data->p);
    free(thread_data);
  }

  return true;
}