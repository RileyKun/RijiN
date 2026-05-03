#include "../../../link.h"

c_base_global* global_internal = nullptr;

void c_base_global::update_look_at_trace(c_user_cmd* cmd){
  if(cmd == nullptr || speedhack_tick || global_data->tick_count == last_update_lookat_tick)
    return;

  c_internal_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  vec3 shoot_pos = localplayer->offset_shoot_pos();

  vec3 fwd;
  math::angle_2_vector(cmd->view_angles, &fwd, nullptr, nullptr);

  // Setup filter
  c_base_generic_trace_filter filter;
  filter.ignore_entity = localplayer;

  assert(base_trace != nullptr);
  lookat_target           = base_trace->internal_ray(filter, shoot_pos, shoot_pos + (fwd * 8912.f), mask_bullet);
  lookat_target_entindex  = lookat_target.hit_entity != nullptr ? lookat_target.hit_entity->get_index() : -1;
  last_update_lookat_tick = global_data->tick_count;
}

c_internal_base_player* c_base_global::update_local_data(){
  localplayer = utils::localplayer();

  localplayer_team  = localplayer != nullptr ? localplayer->team() : 0;
  localplayer_index = localplayer != nullptr ? localplayer->get_index() : -1;

  localplayer_weapon_index = -1;
  if(localplayer != nullptr){
    c_internal_base_entity* wep = (c_internal_base_entity*)localplayer->get_weapon();
    if(wep != nullptr)
      localplayer_weapon_index = wep->get_index();
  }

  return localplayer;
}