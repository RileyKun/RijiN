#pragma once

class c_wrangler_helper{
public:
  void createmove(bool pre_prediction);
  void frame_stage_notify(u32 stage);
  void detect_sentry();
  void render_view(s_view_setup* view);

  c_base_entity* get_active_sentry(){
    return sentry_index > 0 ? global->entity_list->get_entity(sentry_index) : nullptr;
  }

  bool is_overriding_view = false;
  bool should_freeze = false;
  i32  sentry_index  = 0;
};

CLASS_EXTERN(c_wrangler_helper, wrangler_helper);