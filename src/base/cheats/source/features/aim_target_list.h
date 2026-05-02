#pragma once

class c_base_target_list_data{
public:
  float val;
  float p;
  i32 index;
};

#define THREAD_DATA_STATUS_COLLECTING (1 << 1)
#define THREAD_DATA_STATUS_SORTING (1 << 2)
#define THREAD_DATA_STATUS_SORTED (1 << 3)

#define MAX_TARGETS 128

enum target_list_sorted_type{
  TARGET_LIST_SORT_FOV = 0,
  TARGET_LIST_SORT_DIST = 1,
};


class c_base_target_list{
public:
  void setup(){
    SYSTEM_INFO info;
    I(GetSystemInfo)(&info);
  
    processor_core_count = math::biggest(info.dwNumberOfProcessors, 2);
    if(processor_core_count > 16)
      processor_core_count = 16;
  }

  virtual bool is_valid_target_entity(i32 index){
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  virtual void compute_active_spectators(){
    FUNCTION_OVERRIDE_NEEDED;
  }

  virtual bool on_collection_iteration(i32 index){
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  virtual bool is_local_game_specific(){
    // Lets say there's a condition in a game that we wanna check for.
    return true;
  }

  virtual bool is_local_ready(){
    if(!is_local_game_specific())
      return false;

    // Literally like, are we valid, alive? etc..
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  virtual i32 get_wanted_list_type(){
    FUNCTION_OVERRIDE_NEEDED;
    return TARGET_LIST_SORT_FOV;
  }

  void compute_optimized_list();
  void frame_stage_notify(u32 stage);
  void collect_thread_data();
  i32  get_sorted_target_index(i32 index, bool always_use_dist = false);

  void write_to_thread_section(i32 index, float fov, float dist, float priority = 0.f){
    if(priority > 0.f){
      thread_closest_dist[thread_table_count].p = priority;
      thread_closest_fov[thread_table_count].p  = priority;
      needs_priority_calc = true;
    }

    if(fov < thread_smallest_fov)
      thread_smallest_fov = fov;

    if(dist < thread_smallest_dist)
      thread_smallest_dist = dist;

    thread_closest_dist[thread_table_count].index = index;
    thread_closest_dist[thread_table_count].val   = dist;

    thread_closest_fov[thread_table_count].index  = index;
    thread_closest_fov[thread_table_count].val    = fov;

    thread_table_count++;
  }

  virtual bool is_weapon_supported(){
    return true;
  }

  virtual i32 get_loop_start_index(){
    return is_sorted_table_valid() ? 0 : 1;
  }

  virtual i32 get_loop_end_index(){
    if(!is_sorted_table_valid())
      return global_internal->entity_list->get_highest_index();

    return game_table_count - 1;
  }

   void prepare_thread_data(){
    thread_flags      |= THREAD_DATA_STATUS_COLLECTING;
    thread_table_count = 0;
  
    memset(thread_closest_dist, 0, sizeof(thread_closest_dist));
    memset(thread_closest_fov, 0, sizeof(thread_closest_fov));
  
    thread_smallest_fov  = 360.f;
    thread_smallest_dist = 8912.f;
    needs_priority_calc  = false;
  }

  bool post_thread_data(){
    if(!thread_table_count){
      thread_flags = 0;
      return false;
    }

    return true;
  }

  ALWAYSINLINE bool is_being_spectated(){
    return _is_being_spectated;
  }

  ALWAYSINLINE bool is_sorted_table_valid(){
    return game_table_count > 0;
  }

  ALWAYSINLINE bool is_optimized_list_filled(){
    return optimized_table_count > 0;
  }

  void swap(c_base_target_list_data& a, c_base_target_list_data& b){
    c_base_target_list_data t = a;
    a = b;
    b = t;
  }

  i32 partition(c_base_target_list_data* array, i32 low, i32 high){
    float pivot = array[high].val;
    i32 i     = low - 1;

    for(i32 j = low; j < high; j++){
      if(array[j].val < pivot){
        i++;
        swap(array[i], array[j]);
      }
    }

    swap(array[i + 1], array[high]);
    return i + 1;
  }

  void q_sort(c_base_target_list_data* array, i32 low, i32 high){
    if(low < high){
      i32 pi = partition(array, low, high);
      q_sort(array, low, pi - 1);
      q_sort(array, pi + 1, high);
    }
  }

  i32   optimized_list[MAX_TARGETS];
  i32   optimized_table_count = 0;

  i32   processor_core_count = 0;
  i32   thread_flags         = 0;
  i32   best_target_index    = -1;

  // This will be sorted lists but they'll be done in a smart way.
  c_base_target_list_data game_closest_dist[MAX_TARGETS];
  c_base_target_list_data game_closest_fov[MAX_TARGETS];
  i32                     game_table_count  = 0;

  // DO NOT ACCESS THIS DATA!
  c_base_target_list_data thread_closest_dist[MAX_TARGETS];
  c_base_target_list_data thread_closest_fov[MAX_TARGETS];
  float thread_smallest_fov;
  float thread_smallest_dist;
  bool  needs_priority_calc;
  i32 thread_table_count = 0;

  // Spectator list.
  bool _is_being_spectated;
  u32  spectator_count;
  bool allow_thread_run;
};
