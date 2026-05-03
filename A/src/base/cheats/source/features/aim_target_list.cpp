#include "../../../link.h"
#include "aim_target_list.h"

void c_base_target_list::compute_optimized_list(){

  i32 count = 0;
  for(i32 i = 1; i <= global_internal->entity_list->get_highest_index(); i++){
    if(!is_valid_target_entity(i))
      continue;

    optimized_list[count] = i;
    count++;
    if(count >= 128)
      break;
  }

  optimized_table_count = count;
}

// Call ideally before lag compensation.
void c_base_target_list::frame_stage_notify(u32 stage){
  if(stage != FRAME_NET_UPDATE_POSTDATAUPDATE_END)
    return;

  compute_active_spectators();
  compute_optimized_list();

  if(!optimized_table_count)
    return;

  cookie_block_check_return();

  // Things are active, lets try and collect thread data.
  if(thread_flags != 0){
    collect_thread_data();
    return;
  }

  if(!is_local_ready())
    return;

  prepare_thread_data();
  for(i32 i = 0; i < optimized_table_count; i++)
    on_collection_iteration(optimized_list[i]);
  
  // Can return false because there's no entities to be sorted.
  if(!post_thread_data())
    return;

  allow_thread_run = true;

  static bool thread_created = false;
  if(thread_created)
    return;

  thread_created = true;

  // Lets avoid overrides in here.
  utils::create_worker([](c_base_target_list* target_list){
    while(true){
      if(global_internal->unloading)
        break;

      if(!target_list->allow_thread_run){
        Sleep(1);
        continue;
      }

      target_list->allow_thread_run = false;
      target_list->thread_flags |= THREAD_DATA_STATUS_SORTING;

      // Correct values with priority before sorting.
      if(target_list->needs_priority_calc){
        for(i32 i = 0; i < target_list->thread_table_count - 1; i++){
          if(!target_list->thread_closest_fov[i].p)
            continue;

          target_list->thread_closest_fov[i].val = (target_list->thread_smallest_fov - target_list->thread_closest_fov[i].p);
        }

        for(i32 i = 0; i < target_list->thread_table_count - 1; i++){
          if(!target_list->thread_closest_dist[i].p)
            continue;

          target_list->thread_closest_dist[i].val = (target_list->thread_smallest_dist - target_list->thread_closest_dist[i].p);
        }
      }

      target_list->q_sort(target_list->thread_closest_fov, 0, target_list->thread_table_count - 1);
      target_list->q_sort(target_list->thread_closest_dist, 0, target_list->thread_table_count - 1);

      target_list->thread_flags |= THREAD_DATA_STATUS_SORTED;
      Sleep(1);
    }
    return 0;
  }, this);
}

void c_base_target_list::collect_thread_data(){
  if(!(thread_flags & THREAD_DATA_STATUS_SORTED))
    return;

  memcpy(&game_closest_dist, &thread_closest_dist, sizeof(game_closest_dist));
  memcpy(&game_closest_fov, &thread_closest_fov, sizeof(game_closest_fov));

  game_table_count = thread_table_count;
  thread_flags     = 0;
  if(is_sorted_table_valid() && is_weapon_supported()){
    best_target_index = game_closest_fov[0].index;
    if(get_wanted_list_type() == TARGET_LIST_SORT_DIST)
       best_target_index = game_closest_dist[0].index;  
  }
  else 
    best_target_index = -1;
}

i32 c_base_target_list::get_sorted_target_index(i32 index, bool always_use_dist){
  if(!is_sorted_table_valid())
    return index;

  if(index < 0 || index > game_table_count || index >= MAX_TARGETS){
    assert(index >= 0 && index < MAX_TARGETS);
    return index;
  }

  if(get_wanted_list_type() == TARGET_LIST_SORT_DIST)
    return game_closest_dist[index].index;
  
  if(always_use_dist)
    return game_closest_dist[index].index;

  return game_closest_fov[index].index;
}