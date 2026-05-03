#pragma once

class c_interface_cvar;
class c_base_interpolation_manager{
public:

  virtual c_base_interpolation_manager(){
    reset();
  }

  virtual void reset(){
    push_entity    = 0;
    set_tick_count = 0;
    interp_time    = 0.f;
    interp_ticks   = 0;
    org_tickcount  = 0;
    collected      = false;
  }

  virtual void update();
  virtual bool should_update_interp(){
    if(!utils::is_in_game())
      return true;

    c_internal_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return true;

    return (localplayer->team() <= 1 || global_internal->global_data->max_clients == 1);
  }

  virtual bool is_interp_command(u32 hash);

  // There can be lag compensated entities that we do not process. And thus should be handled.
  virtual bool is_handled_by_lag_comp(c_internal_base_entity* entity){
    if(entity == nullptr)
      return false;

    if(entity->get_index() > global_internal->global_data->max_clients)
      return false;

    return true;
  }

  virtual void set_tick_count_with_interp(u32 tick, i32 entity_index, bool backtrack){
    c_internal_base_entity* entity = global_internal->entity_list->get_entity(entity_index);
    handle_set_tick_count_with_interp(entity, tick, entity_index, backtrack);
  }

  virtual void set_tick_count_with_interp(float time, i32 entity_index, bool backtrack){
    set_tick_count_with_interp((u32)math::time_to_ticks(time), entity_index, backtrack);
  }

  virtual void handle_set_tick_count_with_interp(c_internal_base_entity* entity, u32 tick, i32 entity_index, bool backtrack){
    if(entity == nullptr)
      return;

    // If this is on, then we need to compensate for lerp.
    // Edit: Backtrack uses non interpolated positions.
    // So, please don't remove this.
    if(wants_no_interp(entity))
      tick += interp_ticks;
    else{
      if(!backtrack && is_handled_by_lag_comp(entity))
        return;

      tick += interp_ticks;
    }

    set_tick_count = tick;
    push_entity    = entity_index;
  }

  virtual void pre_create_move(c_user_cmd* cmd){
    assert(cmd != nullptr);

    if(!collected)
      update();

    set_tick_count = 0;
    push_entity    = -1;

    org_tickcount  = cmd->tick_count;
  }

  virtual void post_create_move(c_user_cmd* cmd){
    if(cmd->tick_count != org_tickcount){
      DBG("[-] Illegal tickcount change! Restoring...\n");
      cmd->tick_count = org_tickcount;
    }
  
    if(push_entity != -1 && set_tick_count > 0)
      cmd->tick_count = set_tick_count;
  
    push_entity    = -1;
    set_tick_count = 0;
  }

  virtual bool wants_no_interp(c_internal_base_entity* entity){
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  virtual bool must_have_interp();

  float get_interp(){
    return interp_time;
  }

  float get_lerp();

private:
  i32   push_entity;
  u32   set_tick_count;
  float interp_time;
  u32   interp_ticks;
  u32   org_tickcount;
  bool  collected;
};