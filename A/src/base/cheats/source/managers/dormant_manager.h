#pragma once

#define DORMANT_EXPIRE_TIME 1.f


class c_base_player_dormant_data{
public:
  float dormant_timeout;
  float dormant_death_timeout;
};

class c_base_dormant_manager{
public:

  virtual void update(i32 index, vec3 pos = vec3(), bool set_origin = false){
    FUNCTION_OVERRIDE_NEEDED;
  }

  // Call in SetDormant hook
  virtual void on_dormant(c_internal_base_entity* entity, bool dormant){
    FUNCTION_OVERRIDE_NEEDED;
  }

  // Call when killed or spawns in.
  virtual void reset(i32 index){
    FUNCTION_OVERRIDE_NEEDED;
  }

 
  // Call in startsound hook
  virtual void process_entity_sounds(start_sound_params params){
    if(!params.from_server)
      return;

    if(params.sound_source <= 0)
      return;

    if(params.pos == vec3())
      return;

    update(params.sound_source, params.pos, true);
  }
};