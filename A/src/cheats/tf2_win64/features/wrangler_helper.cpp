#include "../link.h"

CLASS_ALLOC(c_wrangler_helper, wrangler_helper);


void c_wrangler_helper::createmove(bool pre_prediction){
  if(sentry_index == -1){
    should_freeze = false;
    return;
  }

  if(pre_prediction && should_freeze){
    global->current_cmd->buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT | IN_JUMP);
    global->current_cmd->move.x = global->current_cmd->move.y = 0.f;
  }
}

void c_wrangler_helper::frame_stage_notify(u32 stage){
  if(stage != FRAME_NET_UPDATE_POSTDATAUPDATE_END)
    return;

  detect_sentry();
}

void c_wrangler_helper::detect_sentry(){
  sentry_index = -1;
  for(u32 i = globaldata->max_clients + 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* entity = global->entity_list->get_entity(i);
    if(entity == nullptr)
      continue;

    if(entity->is_dormant())
      continue;

    if(!entity->is_sentry())
      continue;

    c_base_entity* owner = entity->get_building_owner();
    if(owner == nullptr)
      continue;

    if(owner != global->localplayer)
      continue;

    sentry_index = (i32)i;
    break;
  }
}

void c_wrangler_helper::render_view(s_view_setup* view){
  should_freeze = false;
  is_overriding_view = false;

  if(!config->visual.override_view_with_sentry)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return;

  if(!wep->is_wrangler())
    return;

  static c_key_control key(&config->visual.override_view_with_sentry_key, &config->visual.override_view_with_sentry_key_held, &config->visual.override_view_with_sentry_key_toggle, &config->visual.override_view_with_sentry_key_double_click);
  if(config->visual.override_view_with_sentry_use_key && !key.is_toggled())
    return;

  c_base_entity* sentry = get_active_sentry();
  if(sentry == nullptr)
    return;

  if(sentry->disabled() || sentry->plasma_disabled() || sentry->building() || sentry->placing() || sentry->sapped())
    return;

  vec3 angles    = global->engine->get_view_angles();
  vec3 shoot_pos = localplayer->shoot_pos();
  vec3 fwd;
  math::angle_2_vector(angles, &fwd, nullptr, nullptr);
  s_trace tr = global->trace->ray(shoot_pos, shoot_pos + (fwd * 8912.f), mask_bullet);

  view->origin   = sentry->get_sentry_shoot_pos();
  view->angles   = math::calc_view_angle(view->origin, tr.end);
  should_freeze = true;
  is_overriding_view = true;
}