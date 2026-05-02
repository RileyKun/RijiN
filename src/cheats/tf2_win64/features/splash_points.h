#pragma once

#define SPLASH_MAX_FLOOR_POINTS 12
// During the base point calculation, it'll step by this amount from each step.
// The lower the more points and the higher the less points.
//#define SPLASH_BASE_POINT_STEP 0.75f
#define MAX_SPLASH_GEN_POINTS 65535


#define MAX_POINTS_GENERATED 32.f

struct s_splash_thread_data{
  float max_points;
  float aoa;
  i32   weapon_id;
  i32   slot;
  float step_size;
};

struct s_splash_point_sort{
  vec3  cur_pos;
  vec3* p;
  vec3* o;
  u32   s;
};

class c_splash_point{
public:
  bool      generating_base; // This will cause an issue where splash points won't be updated if it's calculating another weapon's. Oh well.
  i32       weapon_id[3];
  vec3      splash_points[3][MAX_SPLASH_GEN_POINTS];
  i32       splash_point_count[3];
  float     splash_max_points[3];
  bool      generating_points[3];

  float     splash_radius_size[3];
  i32       splash_weapon_id[3];
    
  float     cur_blast_radius[3];
  float     cur_blast_radius_sqr[3];
  float     splash_step_size[3];

  void reset(){
    memset(this, 0, sizeof(c_splash_point));
  }

  float get_max_splash_points(c_base_weapon* wep);
  void  update_weapon_cache(c_base_weapon* wep);
  void  generate_base_points();

  float calculate_radius(c_base_entity* entity, i32 slot);
  bool  get_valid_points_to_entity(c_base_entity* entity, vec3 origin, vec3 obb_center);
  bool  has_hit_bad_entity(c_base_entity* entity);

  bool  is_valid_splash_entity(c_base_entity* entity);
  bool  should_generate(c_base_weapon* wep);
  bool  should_update_entity(c_base_entity* entity);
  bool  does_entity_have_points(c_base_entity* entity);

  bool  update_entity_points(c_base_entity* entity, vec3* points_list, u32 count);
  bool  delete_entity_points(c_base_entity* entity);

  bool sort_splash_points(vec3 cur_pos, vec3* splash_points, u32 size);

  u32 get_surface_type(vec3& plane){
    if(plane.z >= 0.7f)
      return 0; // floor.
    else if(plane.z <= -0.7f)
      return 1; // ceiling.

    return 2; // wall
  }

  u32 get_trace_direction(vec3& start, vec3& end, float delta = 6.f){
    vec3 ang = math::calc_view_angle(start, end);
    float yaw   = math::normalize_angle(ang.y);
    u32 sectors = (u32)(360.f / delta);

    for(u32 i = 0; i < sectors; i++) {
      float min = -180.f + i * delta;
      float max = min + delta;
      if(yaw >= min && yaw < max)
        return i + 1;
    }

    return 0;
  }

  u32 get_trace_dist_sectors(vec3& start, vec3& end){
    return math::clamp(start.distance_meter(end), 0, 16);
  }

  float get_step_size(){
    //#if defined(DEV_MODE)
    //if(!global->aimbot_settings->__splash_reserved){
    //  global->aimbot_settings->splash_bot_step_size = 0.7f;
    //  global->aimbot_settings->__splash_reserved = true;
    //}
//
    //return math::clamp(global->aimbot_settings->splash_bot_step_size, 0.1f, 2.0f);
    //#else
    //return 1.0f;
    //#endif
    return 0.1f;
  }

  void swap(vec3& a, vec3& b) {
    vec3 t = a;
    a = b;
    b = t;
  }

  i32 partition(vec3* array, i32 low, i32 high, const vec3& ref) {
    float pivot = (array[high] - ref).length();
    i32 i = low - 1;
  
    for (i32 j = low; j < high; j++) {
      if((array[j] - ref).length() < pivot) {  
        i++;
        swap(array[i], array[j]);
      }
    }
  
    swap(array[i + 1], array[high]);  
    return i + 1;
  }
  
  void q_sort(vec3* array, i32 low, i32 high, const vec3& ref) {
    if (low < high) {
      i32 pi = partition(array, low, high, ref);
      q_sort(array, low, pi - 1, ref);
      q_sort(array, pi + 1, high, ref);
    }
  }
};

CLASS_EXTERN(c_splash_point, splash_point);