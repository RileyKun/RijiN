#pragma once
#define CREATE_BASE_INTERFACES() if(base_interface == nullptr) base_interface = new c_base_interface; base_interface->initialize();

class c_base_interface{
public:
  void initialize(){
    DBG("[!] c_base_interface::initialize start\n");
    write_user_cmd_delta_to_buffer_index      = GET_PKG("write_user_cmd_delta_to_buffer");
    get_entity_index                          = GET_PKG("get_entity");
    get_entity_handle_index                   = GET_PKG("get_entity_handle");
    get_highest_index_index                   = GET_PKG("get_highest_index");
    set_mouse_input_enabled_index             = GET_PKG("set_mouse_input_enabled");
    panel_get_name_index                      = GET_PKG("panel_get_name");
    force_material_override_index             = GET_PKG("force_material_override");
    get_model_name_index                      = GET_PKG("get_model_name");
    get_studio_hdr_index                      = GET_PKG("get_studio_hdr");
    get_world_to_screen_matrix_for_view_index = GET_PKG("get_world_to_screen_matrix_for_view");
    set_blend_index                           = GET_PKG("set_blend");
    get_blend_index                           = GET_PKG("get_blend");
    set_colour_modulation_index               = GET_PKG("set_colour_modulation");
    get_colour_modulation_index               = GET_PKG("get_colour_modulation");
    draw_beam_index                           = GET_PKG("draw_beam");
    create_beam_points_index                  = GET_PKG("create_beam_points");

    create_env_index                          = GET_PKG("create_env");
    destroy_env_index                         = GET_PKG("destroy_env");
    bbox_to_collide_index                     = GET_PKG("bbox_to_collide");
    is_collision_enabled_index                = GET_PKG("is_collision_enabled");
    enable_collisions_index                   = GET_PKG("enable_collisions");
    enable_gravity_index                      = GET_PKG("enable_gravity");
    enable_drag_index                         = GET_PKG("enable_drag");
    wake_index                                = GET_PKG("wake");
    sleep_index                               = GET_PKG("sleep");
    set_drag_coefficient_index                = GET_PKG("set_drag_coefficient");
    vphy_set_position_index                   = GET_PKG("vphy_set_position");
    vphy_set_velocity_index                   = GET_PKG("vphy_set_velocity");
    vphy_get_position_index                   = GET_PKG("vphy_get_position");
    vphy_get_velocity_index                   = GET_PKG("vphy_get_velocity");
    vphy_set_gravity_index                    = GET_PKG("vphy_set_gravity");
    set_air_density_index                     = GET_PKG("set_air_density");
    create_physics_object_index               = GET_PKG("create_physics_object");
    set_performance_settings_index            = GET_PKG("set_performance_settings");
    vphy_simulate_index                       = GET_PKG("vphy_simulate");

    is_logged_in_index                        = GET_PKG("is_logged_in");
    get_steam_id_index                        = GET_PKG("get_steam_id");

    // This is supposed to be zero, so...
    //get_persona_name_index                    = fetch("get_persona_name"); 
    get_persona_name_other_index              = GET_PKG("get_persona_name_other");
    game_overlay_to_web_page_index            = GET_PKG("game_overlay_to_web_page");

    // material system
    create_material_index                       = GET_PKG("create_material");                      
    find_material_index                         = GET_PKG("find_material");                        
    find_texture_index                          = GET_PKG("find_texture");                         
    create_named_render_target_texture_ex_index = GET_PKG("create_named_render_target_texture_ex");
    get_render_context_index                    = GET_PKG("get_render_context");                   
    get_back_buffer_format_index                = GET_PKG("get_back_buffer_format"); 
    
    // material
    //mat_get_name_index                        = fetch("mat_get_name"); // This is zero so..                
    get_texture_group_name_index              = GET_PKG("get_texture_group_name");       
    mat_find_var_index                        = GET_PKG("mat_find_var");                 
    mat_increment_reference_count_index       = GET_PKG("mat_increment_reference_count");
    mat_alpha_modulate_index                  = GET_PKG("mat_alpha_modulate");           
    mat_colour_modulate_index                 = GET_PKG("mat_colour_modulate");          
    set_material_var_flag_index               = GET_PKG("set_material_var_flag");        
    get_material_var_flag_index               = GET_PKG("get_material_var_flag");        
    is_error_material_index                   = GET_PKG("is_error_material");            
    get_alpha_modulate_index                  = GET_PKG("get_alpha_modulate");           

    // texture
    texture_get_actual_width_index            = GET_PKG("texture_get_actual_width");
    texture_get_actual_height_index           = GET_PKG("texture_get_actual_height");
    texture_increment_reference_count_index   = GET_PKG("texture_increment_reference_count");

    // Render context
    rc_release_index                          = GET_PKG("rc_release");                    
    rc_depth_range_index                      = GET_PKG("rc_depth_range");                
    set_stencil_enable_index                  = GET_PKG("set_stencil_enable");            
    set_stencil_fail_operation_index          = GET_PKG("set_stencil_fail_operation");    
    set_stencil_zfail_operation_index         = GET_PKG("set_stencil_zfail_operation");   
    set_stencil_pass_operation_index          = GET_PKG("set_stencil_pass_operation");    
    set_stencil_compare_function_index        = GET_PKG("set_stencil_compare_function");  
    set_stencil_reference_value_index         = GET_PKG("set_stencil_reference_value");   
    set_stencil_test_mask_index               = GET_PKG("set_stencil_test_mask");         
    set_stencil_write_mask_index              = GET_PKG("set_stencil_write_mask");        
    push_render_target_and_viewport_index     = GET_PKG("push_render_target_and_viewport");
    pop_render_target_and_viewport_index      = GET_PKG("pop_render_target_and_viewport");
    set_render_target_index                   = GET_PKG("set_render_target");
    rc_viewport_index                         = GET_PKG("rc_viewport");
    clear_colour_4ub_index                    = GET_PKG("clear_colour_4ub");
    clear_buffers_index                       = GET_PKG("clear_buffers");
    draw_screen_space_rectangle_index         = GET_PKG("draw_screen_space_rectangle");
    
    // material vars  
    mv_set_float_value_index                  = GET_PKG("mv_set_float_value");  
    mv_set_int_value_index                    = GET_PKG("mv_set_int_value");    
    mv_set_string_value_index                 = GET_PKG("mv_set_string_value"); 
    mv_get_string_value_index                 = GET_PKG("mv_get_string_value"); 
    mv_set_2d_index                           = GET_PKG("mv_set_2d");           
    mv_set_3d_index                           = GET_PKG("mv_set_3d");           
    mv_set_4d_index                           = GET_PKG("mv_set_4d");

    // game event
    ge_get_name_index                         = GET_PKG("ge_get_name");  
    ge_get_bool_index                         = GET_PKG("ge_get_bool");  
    ge_get_int_index                          = GET_PKG("ge_get_int");   
    ge_get_float_index                        = GET_PKG("ge_get_float"); 
    ge_get_string_index                       = GET_PKG("ge_get_string");
    ge_set_bool_index                         = GET_PKG("ge_set_bool");  
    ge_set_int_index                          = GET_PKG("ge_set_int");   
    ge_set_float_index                        = GET_PKG("ge_set_float"); 
    ge_set_string_index                       = GET_PKG("ge_set_string");
    
    net_msg_get_group_index                   = GET_PKG("net_msg_get_group");
    net_msg_get_name_index                    = GET_PKG("net_msg_get_name");

    run_command_index                         = GET_PKG("run_command");

    add_listener_index                        = GET_PKG("add_listener");            
    remove_listener_index                     = GET_PKG("remove_listener");         
    find_listener_index                       = GET_PKG("find_listener");           
    fire_event_client_side_index              = GET_PKG("fire_event_client_side");  
    create_event_index                        = GET_PKG("create_event");

    get_screen_size_index                     = GET_PKG("get_screen_size");       
    server_cmd_index                          = GET_PKG("server_cmd");            
    get_player_info_index                     = GET_PKG("get_player_info");       
    get_player_for_user_id_index              = GET_PKG("get_player_for_user_id");
    get_local_player_index                    = GET_PKG("get_local_player");      
    set_view_angles_index                     = GET_PKG("set_view_angles");
    get_view_angles_index                     = GET_PKG("get_view_angles");     
    is_taking_screenshot_index                = GET_PKG("is_taking_screenshot");  
    is_in_game_index                          = GET_PKG("is_in_game");            
    engine_fire_events_index                  = GET_PKG("engine_fire_events");    
    client_cmd_index                          = GET_PKG("client_cmd");            
    send_keyvalue_index                       = GET_PKG("send_keyvalue");         
    get_net_channel_index                     = GET_PKG("get_net_channel");       
    is_playing_demo_index                     = GET_PKG("is_playing_demo");       
    is_recording_demo_index                   = GET_PKG("is_recording_demo");     
    get_level_name_index                      = GET_PKG("get_level_name");        
    get_latency_index                         = GET_PKG("get_latency");           
    get_avg_latency_index                     = GET_PKG("get_avg_latency");       
    get_avg_packets_index                     = GET_PKG("get_avg_packets");       
    get_ip_index                              = GET_PKG("get_ip");                
    send_datagram_index                       = GET_PKG("send_datagram");         
    get_achievement_mgr_index                 = GET_PKG("get_achievement_mgr");

    chat_print_index                          = GET_PKG("chat_print");        
    sfx_table_get_name_index                  = GET_PKG_NER("sfx_table_get_name");
    achievement_get_id_index                  = GET_PKG_NER("achievement_get_id");
    get_achievement_by_index_index            = GET_PKG_NER("get_achievement_by_index");
    get_achievement_count_index               = GET_PKG("get_achievement_count");   
    award_achievement_index                   = GET_PKG("award_achievement");

    client_renderable_unknown_index           = GET_PKG_NER("client_renderable_unknown");
    client_unknown_get_base_entity_index      = GET_PKG("client_unknown_get_base_entity"); 

    get_commands_index                        = GET_PKG("get_commands");
    find_command_index                        = GET_PKG("find_command");
    cvar_find_var_index                       = GET_PKG("cvar_find_var");
    cvar_set_value_str_index                  = GET_PKG("cvar_set_value_str");
    cvar_set_value_int_index                  = GET_PKG("cvar_set_value_int");

    engine_ray_trace_index                    = GET_PKG("engine_ray_trace");
    engine_ray_to_entity_index                = GET_PKG("engine_ray_to_entity");

    has_friend_index                          = GET_PKG("has_friend");

#if defined(SOURCE_2018)
    set_stencil_state_index = GET_PKG("set_stencil_state");
#endif
    DBG("[!] c_base_interface::initialize end\n");
  }

  bool is_csgo_engine(){
#if defined(SOURCE_2018)
    return true;
#else
    return false;
#endif
  }

  #if defined(CHECK_FOR_INVALID)
  ALWAYSINLINE u32 get_vindex(i8* str){
    u32 hash = HASH_RT(str);

    u32 result = gen_internal->get_pkg(hash);
    if(result == 0){
      //i8 error_msg[1024];
      //wsprintfA(error_msg, XOR("VTABLE: %s is zero\n"), str);
      //MessageBoxA(nullptr, error_msg, "", 0);
    }

    return result;
  }
  #else
  ALWAYSINLINE u32 get_vindex(u32 hash){
    return (u32)gen_internal->get_pkg(hash);
  }
  #endif

  u32 engine_ray_trace_index               = 0;
  u32 engine_ray_to_entity_index           = 0;
  u32 get_commands_index                   = 0;
  u32 find_command_index                   = 0;
  u32 cvar_find_var_index                  = 0;
  u32 cvar_set_value_str_index             = 0;
  u32 cvar_set_value_int_index             = 0;

  u32 client_renderable_unknown_index      = 0;
  u32 client_unknown_get_base_entity_index = 0;

  u32 chat_print_index                 = 0;
  u32 sfx_table_get_name_index         = 0;
  u32 achievement_get_id_index         = 0;
  u32 get_achievement_by_index_index   = 0;
  u32 get_achievement_count_index      = 0;
  u32 award_achievement_index          = 0;

  u32 get_screen_size_index        = 0;
  u32 server_cmd_index             = 0;
  u32 get_player_info_index        = 0;
  u32 get_player_for_user_id_index = 0;
  u32 get_local_player_index       = 0;
  u32 set_view_angles_index        = 0;
  u32 get_view_angles_index        = 0;
  u32 is_taking_screenshot_index   = 0;
  u32 is_in_game_index             = 0;
  u32 engine_fire_events_index     = 0;
  u32 client_cmd_index             = 0;
  u32 send_keyvalue_index          = 0;
  u32 get_net_channel_index        = 0;
  u32 is_playing_demo_index        = 0;
  u32 is_recording_demo_index      = 0;
  u32 get_level_name_index         = 0;
  u32 get_latency_index            = 0;
  u32 get_avg_latency_index        = 0;
  u32 get_avg_packets_index        = 0;
  u32 get_ip_index                 = 0;
  u32 send_datagram_index          = 0;
  u32 get_achievement_mgr_index    = 0;

  u32 add_listener_index                          = 0;
  u32 remove_listener_index                       = 0;
  u32 find_listener_index                         = 0;
  u32 fire_event_client_side_index                = 0;
  u32 create_event_index                          = 0;

  u32 run_command_index                           = 0;
  u32 net_msg_get_group_index                     = 0;
  u32 net_msg_get_name_index                      = 0;
  u32 ge_get_name_index                           = 0;
  u32 ge_get_bool_index                           = 0;
  u32 ge_get_int_index                            = 0;
  u32 ge_get_float_index                          = 0;
  u32 ge_get_string_index                         = 0;
  u32 ge_set_bool_index                           = 0;
  u32 ge_set_int_index                            = 0;
  u32 ge_set_float_index                          = 0;
  u32 ge_set_string_index                         = 0;
  
  u32 create_material_index                       = 0;
  u32 find_material_index                         = 0;
  u32 find_texture_index                          = 0;
  u32 create_named_render_target_texture_ex_index = 0;
  u32 get_render_context_index                    = 0;
  u32 get_back_buffer_format_index                = 0;


  u32 mat_get_name_index                    = 0;
  u32 get_texture_group_name_index          = 0;
  u32 mat_find_var_index                    = 0;
  u32 mat_increment_reference_count_index   = 0;
  u32 mat_alpha_modulate_index              = 0;
  u32 mat_colour_modulate_index             = 0;
  u32 set_material_var_flag_index           = 0;
  u32 get_material_var_flag_index           = 0;
  u32 is_error_material_index               = 0;
  u32 get_alpha_modulate_index              = 0;

  // material vars.
  u32 mv_set_float_value_index  = 0;
  u32 mv_set_int_value_index    = 0;
  u32 mv_set_string_value_index = 0;
  u32 mv_get_string_value_index = 0;
  u32 mv_set_2d_index           = 0;
  u32 mv_set_3d_index           = 0;
  u32 mv_set_4d_index           = 0;

  // render context
  u32 rc_release_index                      = 0;
  u32 rc_depth_range_index                  = 0;
  u32 set_stencil_enable_index              = 0;
  u32 set_stencil_fail_operation_index      = 0;
  u32 set_stencil_zfail_operation_index     = 0;
  u32 set_stencil_pass_operation_index      = 0;
  u32 set_stencil_compare_function_index    = 0;
  u32 set_stencil_reference_value_index     = 0;
  u32 set_stencil_test_mask_index           = 0;
  u32 set_stencil_write_mask_index          = 0;
  u32 push_render_target_and_viewport_index = 0;
  u32 pop_render_target_and_viewport_index  = 0;
  u32 set_render_target_index               = 0;
  u32 rc_viewport_index                     = 0;
  u32 clear_colour_4ub_index                = 0;
  u32 clear_buffers_index                   = 0;
  u32 draw_screen_space_rectangle_index     = 0;

  u32 texture_get_actual_width_index          = 0;
  u32 texture_get_actual_height_index         = 0;
  u32 texture_increment_reference_count_index = 0;

  u32 write_user_cmd_delta_to_buffer_index      = 0;
  u32 get_entity_index                          = 0;
  u32 get_entity_handle_index                   = 0;
  u32 get_highest_index_index                   = 0; // This is dumb.
  u32 set_mouse_input_enabled_index             = 0;
  u32 panel_get_name_index                      = 0;
  u32 force_material_override_index             = 0;
  u32 get_model_name_index                      = 0;
  u32 get_studio_hdr_index                      = 0;
  u32 get_world_to_screen_matrix_for_view_index = 0;
  u32 set_blend_index                           = 0;
  u32 get_blend_index                           = 0;
  u32 set_colour_modulation_index               = 0;
  u32 get_colour_modulation_index               = 0;

  // viewport panel (None of these are literally set, so that's why we are crashing when calling this functions, shucks!)
  u32 vpp_get_name_index           = 0;
  u32 vpp_set_data_index           = 0;
  u32 vpp_reset_index              = 0;
  u32 vpp_update_index             = 0;
  u32 vpp_needs_update_index       = 0;
  u32 vpp_has_input_elements_index = 0;
  u32 vpp_show_panel_index         = 0;

  // viewport
  u32 vp_show_background_index     = 0;
  u32 vp_find_panel_by_name_index  = 0;
  u32 draw_beam_index              = 0;
  u32 create_beam_points_index     = 0;

  // vphysics
  u32 create_env_index               = 0;
  u32 destroy_env_index              = 0;
  u32 bbox_to_collide_index          = 0;
  u32 is_collision_enabled_index     = 0;
  u32 enable_collisions_index        = 0;
  u32 enable_gravity_index           = 0;
  u32 enable_drag_index              = 0;
  u32 wake_index                     = 0;
  u32 sleep_index                    = 0;
  u32 set_drag_coefficient_index     = 0;
  u32 vphy_set_position_index        = 0;
  u32 vphy_set_velocity_index        = 0;
  u32 vphy_get_position_index        = 0;
  u32 vphy_get_velocity_index        = 0;
  u32 vphy_set_gravity_index         = 0;
  u32 set_air_density_index          = 0;
  u32 create_physics_object_index    = 0;
  u32 set_performance_settings_index = 0;
  u32 vphy_simulate_index            = 0;
  u32 is_logged_in_index             = 0;
  u32 get_steam_id_index             = 0;
  u32 get_persona_name_index         = 0;
  u32 get_persona_name_other_index   = 0;
  u32 game_overlay_to_web_page_index = 0;
  u32 has_friend_index               = 0;

#if defined(SOURCE_2018)
  u32 set_stencil_state_index        = 0;
#endif
};

CLASS_EXTERN(c_base_interface, base_interface);