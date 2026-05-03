#pragma once

class c_base_rijin_source1{
public:
  bool  is_cracked       = false;
  float attack_the_crack = 0.f;

  virtual bool is_valid(){
    if(gen_internal->has_sub_expired())
      return false;

    return true;
  }

  virtual bool on_subscription_expired(){
    PRODUCT_STATUS("PRODUCT_SUB_EXPIRED");
    I(ExitProcess)(0);
    return false;
  }

  virtual bool on_security_violation(u32 code){
    #if !defined(DEV_MODE)
      c_net_request_detection req{};
      req.detection_id     = code;
      req.current_steam_id = 0;
      req.transmit(true);
  
      global_internal->entity_list = ((u32)global_internal->entity_list ^ code) + code % 128;
    #endif

    return true;
  }

  virtual bool on_software_crack_detected(){
    return invoke_software_crack_punishment();
  }

  virtual bool invoke_software_crack_punishment(){
    cookie_block_check_return_val(true);

    return false;
  }

  virtual void on_100ms(){

  }

  virtual void on_1000ms(){

  }

  virtual void on_5000ms(){

  }

  virtual void on_60000ms(){

  }

  virtual void process_timers(){
    const float time = math::time();
    static float ms100   = time + 0.1f;
    static float ms1000  = time + 1.f;
    static float ms5000  = time + 5.f;
    static float ms60000 = time + 60.f;

    if(ms100 <= time){
      on_100ms();
      ms100 = time + 0.1f;
    }

    if(ms1000 <= time){
      on_1000ms();
      ms1000 = time + 1.f;
    }

    if(ms5000 <= time){
      on_5000ms();
      ms5000 = time + 5.f;
    }

    if(ms60000 <= time){
      on_60000ms();
      ms60000 = time + 60.f;
    }
  }

  virtual bool d3d9_input(c_render_d3d9* render_base){
    vec3i screen_size = global_internal->engine->get_screen_size();
    if(screen_size.x != 0 && screen_size.y != 0)
      render->screen_size = screen_size;

    static bool sub_expired = false;
    if(gen_internal->has_sub_expired() && !sub_expired){
      on_subscription_expired();
      sub_expired = true;
    }

    const float time = math::time();
    if(is_cracked && attack_the_crack <= 0.f || time < 0.f){
      attack_the_crack = math::time() + 600.f;
      on_software_crack_detected();
    }
    else if(attack_the_crack <= time){
      invoke_software_crack_punishment();
      attack_the_crack = time + 300.f;
    }
    
    input_system->update();

    process_timers();
    return false;
  }

  virtual bool d3d9_hook_game_specific(c_render_d3d9* render_base){
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  virtual bool d3d9_hook_ui(c_render_d3d9* render_base){
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  virtual bool on_engine_frame(){
    static i32 last_frame_count = 0;
    if(last_frame_count == global_internal->global_data->frame_count)
      return true;

    last_frame_count = global_internal->global_data->frame_count;

    global_internal->engine_tool->get_world_to_screen_matrix_for_view(global_internal->view_setup, &global_internal->view_matrix);
    memcpy(&global_internal->view_matrix_untouched, &global_internal->view_matrix, sizeof(matrix4x4));

    cookie_block_check_return_val_ok(false);

    DBG("[!] rijin is cracked... Forever in sadness.\n");
    is_cracked = true;

    return false;
  }

};