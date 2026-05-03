#include "link.h"

bool setup(void* device){
  if(gen_internal->loaded)
    return true;

  GLOBAL_SETUP();
  RENDER_SETUP();
  RENDER_DEBUG_SETUP();
  CREATE_NOTIFICATION_SYSTEM();
  CREATE_BASE_INTERFACES();

  notify->setup_fonts();
  render_debug->register_ws2(&math::world2screen);

  // Setup trampoline addresses
  {
    global->d3d9_hook_trp                                           = GET_PKG("d3d9_hook");
    //global->d3d9_reset_trp                                          = GET_PKG("d3d9_reset");
    global->create_move_hook_trp                                    = GET_PKG("create_move_hook");
    global->override_view_hook_trp                                  = GET_PKG("override_view_hook");
    global->engine_paint_hook_trp                                   = GET_PKG("engine_paint_hook");
    global->paint_traverse_hook_trp                                 = GET_PKG("paint_traverse_hook");
    global->player_resource_get_team_trp                            = GET_PKG("player_resource_get_team_hook");
    global->player_panel_get_team_trp                               = GET_PKG("player_panel_get_team_hook");
    global->ctfplayershared_incondition_trp                         = GET_PKG("ctfplayershared_incondition_hook");
    //global->get_direct_ping_to_pop_hook_trp                         = GET_PKG("get_direct_ping_to_pop_hook");
    global->get_direct_ping_to_data_center_hook_trp                 = GET_PKG("get_direct_ping_to_data_center_hook");
    global->check_for_pure_server_whitelist_hook_trp                = GET_PKG("check_for_pure_server_whitelist_hook");
    global->ctfplayer_geteffectiveinvislevel_trp                    = GET_PKG("ctfplayer_geteffectiveinvislevel_hook");
    global->ctfplayer_isplayerclass_hook_trp                        = GET_PKG("ctfplayer_isplayerclass_hook");
    global->frame_stage_notify_hook_trp                             = GET_PKG("frame_stage_notify_hook");
    global->float_decode_hook_trp                                   = GET_PKG("float_decode_hook");
    global->int_decode_hook_trp                                     = GET_PKG("int_decode_hook");
    global->medigun_itempostframe_hook_trp                          = GET_PKG("medigun_itempostframe_hook");
    global->ctfplayer_avoidplayers_hook_trp                         = GET_PKG("ctfplayer_avoidplayers_hook");
    global->ctfwearable_shoulddraw_hook_trp                         = GET_PKG("ctfwearable_shoulddraw_hook");
    global->run_command_hook_trp                                    = GET_PKG("run_command_hook");
    global->post_think_hook_trp                                     = GET_PKG("post_think_hook");
    global->fire_event_hook_trp                                     = GET_PKG("fire_event_hook");
    global->util_tracer_hook_trp                                    = GET_PKG("util_particle_tracer_hook");
    global->ctfplayer_firebullet_hook_trp                           = GET_PKG("ctfplayer_firebullet_hook");
    global->cl_move_hook_trp                                        = GET_PKG("cl_move_hook");
    global->usermessages_dispatch_user_message_hook_trp             = GET_PKG("usermessages_dispatch_user_message_hook");
    global->update_step_sound_hook_trp                              = GET_PKG("update_step_sound_hook");
    global->get_tracer_type_hook_trp                                = GET_PKG("get_tracer_type_hook");
    global->cviewrender_setscreenoverlaymaterial_hook_trp           = GET_PKG("cviewrender_setscreenoverlaymaterial_hook");
    global->ctfplayer_createplayergibs_hook_trp                     = GET_PKG("ctfplayer_createplayergibs_hook");
    global->base_animating_setupbones_hook_trp                      = GET_PKG("base_animating_setupbones_hook");
    global->set_dormant_hook_trp                                    = GET_PKG("set_dormant_hook");
    global->get_outer_abs_velocity_hook_trp                         = GET_PKG("get_outer_abs_velocity_hook");
    global->hud_chatline_insert_and_colorize_text_hook_trp          = GET_PKG("hud_chatline_insert_and_colorize_text_hook");
 
    global->calc_is_attack_critical_hook_trp                        = GET_PKG("calc_is_attack_critical_hook");
    global->minigun_weapon_sound_update_hook_trp                    = GET_PKG("minigun_weapon_sound_update_hook");
    global->emit_sound_hook_trp                                     = GET_PKG("emit_sound_hook");
    global->cinput_get_user_cmd_hook_trp                            = GET_PKG("cinput_get_user_cmd_hook");
    global->process_packet_hook_trp                                 = GET_PKG("process_packet_hook");
    global->send_datagram_hook_trp                                  = GET_PKG("send_datagram_hook");
    global->buf_addtext_withmarkers_hook_trp                        = GET_PKG("buf_addtext_withmarkers_hook");
    global->cbaseentity_followentity_hook_trp                       = GET_PKG("cbaseentity_followentity_hook");
    global->fx_firebullets_hook_trp                                 = GET_PKG("fx_firebullets_hook");

    global->can_fire_random_critical_shot_hook_trp                  = GET_PKG("can_fire_random_critical_shot_hook");


    global->engine_get_player_info_hook_trp                         = GET_PKG("engine_get_player_info_hook");
    global->is_allowed_to_withdraw_from_crit_bucket_hook_trp        = GET_PKG("is_allowed_to_withdraw_from_crit_bucket_hook");
    global->add_to_crit_bucket_hook_trp                             = GET_PKG("add_to_crit_bucket_hook");
    global->ctfragdoll_create_ragdoll_hook_trp                      = GET_PKG("ctfragdoll_create_ragdoll_hook");
    global->cbaseanimating_update_client_side_animation_hook_trp    = GET_PKG("cbaseanimating_update_client_side_animation_hook");
    global->ctfviewmodel_calcviewmodelview_hook_trp                 = GET_PKG("ctfviewmodel_calcviewmodelview_hook");
    global->cnetchan_sendnetmsg_hook_trp                            = GET_PKG("cnetchan_sendnetmsg_hook");

    global->studio_render_context_drawmodel_hook_trp                = GET_PKG("studio_render_context_drawmodel_hook");
    global->menu_lobbychat_hook_trp                                 = GET_PKG("menu_lobbychat_hook");
    global->cbuf_executecmd_hook_trp                                = GET_PKG("cbuf_executecmd_hook");
    global->cvar_set_value_hook_trp                                 = GET_PKG("cvar_set_value_hook");
    global->voice_assign_channel_hook_trp                           = GET_PKG("voice_assign_channel_hook");
    global->get_local_view_angles_hook_trp                          = GET_PKG("get_local_view_angles_hook");
    global->do_post_screen_space_effects_hook_trp                   = GET_PKG("do_post_screen_space_effects_hook");
    global->s_startsound_hook_trp                                   = GET_PKG("s_startsound_hook");
    global->draw_model_execute_hook_trp                             = GET_PKG("draw_model_execute_hook");
    global->calc_viewmodel_bob_hook_trp                             = GET_PKG("calc_viewmodel_bob_hook");
    global->engine_trace_ray_hook_trp                               = GET_PKG("engine_trace_ray_hook");
    global->cbase_combat_weapon_hook_trp                            = GET_PKG("cbase_combat_weapon_hook");
    global->ctfplayershared_onconditionadded_hook_trp               = GET_PKG("ctfplayershared_onconditionadded_hook");
    global->ctfplayershared_onconditionremoved_hook_trp             = GET_PKG("ctfplayershared_onconditionremoved_hook");
    global->ctfplayeranimstate_aimyaw_hook_trp                      = GET_PKG("ctfplayeranimstate_aimyaw_hook");

    global->cmaterial_uncache_hook_trp                              = GET_PKG("cmaterial_uncache_hook");
    global->chudcrosshair_getdrawposition_hook_trp                  = GET_PKG("chudcrosshair_getdrawposition_hook");
    global->cbaseclientstate_process_print_hook_trp                 = GET_PKG("cbaseclientstate_process_print_hook");
    global->cnewparticle_effect_render_hook_trp                     = GET_PKG("cnewparticle_effect_render_hook");
    global->csniperdot_clientthink_hook_trp                         = GET_PKG("csniperdot_clientthink_hook");
    global->particlesystem_setcontrolpoint_hook_trp                 = GET_PKG("particlesystem_setcontrolpoint_hook");
    //global->is_crit_boosted_hook_trp                                = GET_PKG("is_crit_boosted_hook");
    global->r_setfogvolumestate_hook_trp                            = GET_PKG("r_setfogvolumestate_hook");
    global->get_fog_enable_hook_trp                                 = GET_PKG("get_fog_enable_hook");


    global->cbaseclientstate_process_set_convar_hook_trp            = GET_PKG("cbaseclientstate_process_set_convar_hook");
    global->studio_render_forced_material_override_hook_trp         = GET_PKG("studio_render_forced_material_override_hook");
    global->ctfplayerinventory_getmaxitems_hook_trp                 = GET_PKG("ctfplayerinventory_getmaxitems_hook");

    global->equippable_items_for_slotgenerator_constructor_hook_trp = GET_PKG("equippable_items_for_slotgenerator_constructor_hook");
    global->input_cam_is_thirdperson_hook_trp                       = GET_PKG("input_cam_is_thirdperson_hook");
    global->ccvar_find_command_hook_trp                             = GET_PKG("ccvar_find_command_hook");
    global->crenderview_renderview_hook_trp                         = GET_PKG("crenderview_renderview_hook");
    global->inventorymanager_loadpreset_hook_trp                    = GET_PKG("inventorymanager_loadpreset_hook");
    global->sentry_muzzle_flash_hook_trp                            = GET_PKG("sentry_muzzle_flash_hook");
    global->tracer_callback_hook_trp                                = GET_PKG("tracer_callback_hook");
    global->ctfviewmodel_calcviewmodellag_hook_trp                  = GET_PKG("ctfviewmodel_calcviewmodellag_hook");
    global->ctfweaponbase_getshootsound_hook_trp                    = GET_PKG("ctfweaponbase_getshootsound_hook");


    global->add_resist_shield_hook_trp                              = GET_PKG("add_resist_shield_hook");
    global->inventory_manager_updateinv_equipped_state_hook_trp     = GET_PKG("inventory_manager_updateinv_equipped_state_hook");
    global->generate_equip_region_conflict_mask_hook_trp            = GET_PKG("generate_equip_region_conflict_mask_hook");
    global->maintain_sequence_transition_hook_trp                   = GET_PKG("maintain_sequence_transition_hook");
    global->set_context_think_hook_trp                              = GET_PKG("set_context_think_hook");
    global->is_blacklisted_hook_trp                                 = GET_PKG("is_blacklisted_hook");
    global->create_particle_hook_trp                                = GET_PKG("create_particle_hook");
    global->rtl_dispatch_exception_hook_trp                         = GET_PKG("rtl_dispatch_exception_hook");
    global->cstudiohdr_seqdesc_hook_trp                             = GET_PKG("cstudiohdr_seqdesc_hook");
    global->cbaseentity_teleported_hook_trp                         = GET_PKG("cbaseentity_teleported_hook");
    global->engineclient_ispaused_hook_trp                          = GET_PKG("engineclient_ispaused_hook");
    global->cbaseentity_baseinterpolatepart1_hook_trp               = GET_PKG("cbaseentity_baseinterpolatepart1_hook");

    #if defined(DEV_MODE)
      //global->physics_object_getdragindir_hook_trp                = GET_PKG("physics_object_getdragindir_hook");
      //global->physics_object_getangdir_hook_trp                   = GET_PKG("physics_object_getangdir_hook");
      //global->physics_clamp_velocity_hook_trp                     = GET_PKG("physics_clamp_velocity_hook");
      //global->server_calc_is_attack_critical_hook_trp                 = GET_PKG("server_calc_is_attack_critical_hook");
      //global->server_fire_bullet_hook_trp                             = GET_PKG("server_fire_bullet_hook");
      //global->server_adjust_player_time_base_hook_trp                 = GET_PKG("server_adjust_player_time_base_hook");
    #endif
  }

  // Copy signature addresses and pointers
  {
    global->input                                                 = *(s_input**)GET_PKG("input");
    global->client_state                                          = (s_client_state*)GET_PKG("client_state");

    global->get_team_check_scoreboard_retaddr1                    = GET_PKG("get_team_check_scoreboard_retaddr1");
    global->get_team_status_panel_retaddr1                        = GET_PKG("get_team_status_panel_retaddr1");
    global->ctfplayer_fireevent_retaddr1                          = GET_PKG("ctfplayer_fireevent_retaddr1");
    global->displaydmgfeedback_retaddr1                           = GET_PKG("displaydmgfeedback_retaddr1");
    global->tf2_getmaxspeed                                       = GET_PKG("tf2_getmaxspeed");
    global->ctfplayer_createmove_retaddr1                         = GET_PKG("ctfplayer_createmove_retaddr1");
    global->ctfplayer_createmove_retaddr2                         = GET_PKG("ctfplayer_createmove_retaddr2");
    global->ctfplayer_createmove_retaddr3                         = GET_PKG("ctfplayer_createmove_retaddr3");
    global->ctfwearable_shoulddraw_retaddr1                       = GET_PKG("ctfwearable_shoulddraw_retaddr1");
    global->ctfplayer_shoulddraw_retaddr1                         = GET_PKG("ctfplayer_shoulddraw_retaddr1");
    global->ctfplayershared_addcond                               = GET_PKG("ctfplayershared_addcond");
    global->ctfplayershared_removecond                            = GET_PKG("ctfplayershared_removecond");
    global->shoulddraw_viewmodel_retaddr1                         = GET_PKG("shoulddraw_viewmodel_retaddr1");
    global->ctfplayer_canmoveduringtaunt                          = GET_PKG("ctfplayer_canmoveduringtaunt");
    global->get_client_voice_manager                              = GET_PKG("get_client_voice_manager");
    global->set_player_blocked_state                              = GET_PKG("set_player_blocked_state");
    global->is_player_blocked                                     = GET_PKG("is_player_blocked");
    global->is_crit_boosted                                       = GET_PKG("is_crit_boosted");
    global->medigun_find_and_heal_target_retaddr1                 = GET_PKG("medigun_find_and_heal_target_retaddr1");
    global->get_ammo_count                                        = GET_PKG("get_ammo_count");
    global->is_steam_friend_get_player_info_retaddr1              = GET_PKG("is_steam_friend_get_player_info_retaddr1");

    global->update_player_avatar_get_player_info_retaddr2         = GET_PKG("update_player_avatar_get_player_info_retaddr2");
    global->attrib_hook_value_float                               = GET_PKG("attrib_hook_value_float");
    global->attrib_hook_value_int                                 = GET_PKG("attrib_hook_value_int");
    global->get_class_name                                        = GET_PKG("get_class_name");
    global->look_weapon_info_slot                                 = GET_PKG("look_weapon_info_slot");
    global->file_weapon_info_from_handle                          = GET_PKG("file_weapon_info_from_handle");
    global->steam3client_addr                                     = GET_PKG("steam3client_addr");

    global->hud_element_addr                                      = (void*)(GET_PKG("hud_element_addr"));
    global->get_hud_element_addr                                  = GET_PKG("get_hud_element_addr");
    global->ctfinput_applymouse_retaddr1                          = GET_PKG("ctfinput_applymouse_retaddr1");
    global->get_projectile_fire_setup                             = GET_PKG("get_projectile_fire_setup");

    global->ctrace_filter_simple                                  = GET_PKG("ctrace_filter_simple");
    global->dispatch_particle_effect                              = GET_PKG("dispatch_particle_effect");

    assert(global->hud_element_addr != nullptr);
    STACK_CHECK_START;
    global->hud_chat                                              = utils::call_fastcall64<void*, i8*>(global->get_hud_element_addr, global->hud_element_addr, XOR("CHudChat"));
    STACK_CHECK_END;

    global->get_max_buffed_health_addr                            = GET_PKG("get_max_buffed_health_addr");
    global->chatprintf_retaddr1                                   = GET_PKG("chatprintf_retaddr1");
    global->gamerules_addr                                        = GET_PKG("gamerules_addr");
    global->shared_object_cache_addr                              = GET_PKG("shared_object_cache_addr"); 
    global->find_base_type_cache_addr                             = GET_PKG("find_base_type_cache_addr");

    global->displaydmgfeedback_retaddr4                           = GET_PKG("displaydmgfeedback_retaddr4");
    global->entity_sphere_query_addr                              = GET_PKG("c_entity_sphere_query");

#if defined(DEV_MODE)
    //global->get_server_animating_addr                             = GET_PKG("get_server_animating_addr");
    //global->draw_server_hitbox_addr                               = GET_PKG("draw_server_hitbox_addr");
#endif

    global->collision_property_calcnearestpoint                   = GET_PKG("collision_property_calcnearestpoint");
    global->ctfplayer_hastheflag                                  = GET_PKG("ctfplayer_hastheflag");
    global->cmd_alias_list                                        = *(void**)(GET_PKG("cmd_alias_list"));
    global->cnewparticle_get_effect_name_addr                     = GET_PKG("cnewparticle_get_effect_name_addr");
    global->csniperdot_clientthink_retaddr1                       = GET_PKG("csniperdot_clientthink_retaddr1");
    //global->is_crit_boosted_material_retaddr1                     = GET_PKG("is_crit_boosted_material_retaddr1");
    //global->update_crit_boost_effect_is_crit_boost_retaddr1       = GET_PKG("update_crit_boost_effect_is_crit_boost_retaddr1");
    global->displaydmgfeedback_isplayerclass_retaddr              = GET_PKG("displaydmgfeedback_isplayerclass_retaddr1");
    global->cnewparticle_effect_render_retaddr                    = GET_PKG("cnewparticle_effect_render_retaddr");

    global->viewport                                              = *(c_interface_viewport**)GET_PKG("viewportpanel_interface_addr");
    global->cdisguisestatus_paint_retaddr                         = GET_PKG("cdisguisestatus_paint_retaddr");
    global->cdisguisestatus_shoulddraw_retaddr                    = GET_PKG("cdisguisestatus_shoulddraw_retaddr");
    global->ctfhudplayerclass_onthink_retaddr                     = GET_PKG("ctfhudplayerclass_onthink_retaddr");
    global->ctfhudplayerclass_update_model_panel_retaddr          = GET_PKG("ctfhudplayerclass_update_model_panel_retaddr");
    global->load_itempreset_incond_retaddr                        = GET_PKG("load_itempreset_incond_retaddr");
    global->updateinvequippedstate_retaddr                        = GET_PKG("updateinvequippedstate_retaddr");
    global->get_spread_angles_addr                                = GET_PKG("get_spread_angles_addr");
    global->ctfpartyclient_func_addr                              = GET_PKG("ctfpartyclient_func_addr");
    global->staticpropmgr_drawstaticprop_retaddr1                 = GET_PKG("staticpropmgr_drawstaticprop_retaddr1");
    global->ctf_firebullet_retaddr1                               = GET_PKG("ctf_firebullet_retaddr1");
    global->iclientfriends_getfriendrelationship_retaddr1         = GET_PKG("iclientfriends_getfriendrelationship_retaddr1");
    global->iclientfriends_getfriendrelationship_retaddr2         = GET_PKG("iclientfriends_getfriendrelationship_retaddr2");
    global->nil_seq_desc_addr                                     = GET_PKG("nil_seq_desc_addr");
    global->get_equip_wearable_for_loadout_slot_addr              = GET_PKG("get_equip_wearable_for_loadout_slot_addr");
    global->setup_bones_base_entity_teleport_retaddr              = GET_PKG("setup_bones_base_entity_teleport_retaddr");
    global->engineclient_ispaused_retaddr                         = GET_PKG("engineclient_ispaused_retaddr");
    global->baseentity_calculate_abs_velocity_addr                = GET_PKG("baseentity_calculate_abs_velocity_addr");
  }

  // Setup interfaces
  {
    global->client          = utils::get_interface(WXOR(L"client.dll"), HASH("VClient017"));
    global->prediction      = utils::get_interface(WXOR(L"client.dll"), HASH("VClientPrediction001"));
    global->entity_list     = utils::get_interface(WXOR(L"client.dll"), HASH("VClientEntityList003"));
    global->engine          = utils::get_interface(WXOR(L"engine.dll"), HASH("VEngineClient014"));
    global->engine_tool     = utils::get_interface(WXOR(L"engine.dll"), HASH("VENGINETOOL003"));
    global->physics           = utils::get_interface(WXOR(L"vphysics.dll"), HASH("VPhysics031"));
    global->physics_collision = utils::get_interface(WXOR(L"vphysics.dll"), HASH("VPhysicsCollision007"));
    global->trace           = utils::get_interface(WXOR(L"engine.dll"), HASH("EngineTraceClient003"));
    global->engine_model    = utils::get_interface(WXOR(L"engine.dll"), HASH("VEngineModel016"));
    global->render_view     = utils::get_interface(WXOR(L"engine.dll"), HASH("VEngineRenderView014"));
    global->model_info      = utils::get_interface(WXOR(L"engine.dll"), HASH("VModelInfoClient006"));
    global->game_manager    = utils::get_interface(WXOR(L"engine.dll"), HASH("GAMEEVENTSMANAGER002"));
    global->material_system = utils::get_interface(WXOR(L"materialsystem.dll"), HASH("VMaterialSystem082"));

    global->cvar            = utils::get_interface(WXOR(L"vstdlib.dll"), HASH("VEngineCvar004"));
    global->panel           = utils::get_interface(WXOR(L"vgui2.dll"), HASH("VGUI_Panel009"));

    global->view_render_beams = *(c_view_render_beams**)GET_PKG("view_render_beams_interface");
    global->steamcontext      = utils::call_fastcall64_raw<void*>(global->steam3client_addr);
  }

  global->init(global->trace);

  // Cvar
  {
    global->cvar->setup();

   // global->cl_interpolate                      = global->cvar->find_var(XOR("cl_interpolate"));
    global->tf_weapon_criticals                 = global->cvar->find_var(XOR("tf_weapon_criticals"));
    global->tf_weapon_criticals_melee           = global->cvar->find_var(XOR("tf_weapon_criticals_melee"));

    global->tf_weapon_criticals_bucket_cap      = global->cvar->find_var(XOR("tf_weapon_criticals_bucket_cap"));
    global->tf_flamethrower_velocity            = global->cvar->find_var(XOR("tf_flamethrower_velocity"));
    global->tf_parachute_maxspeed_xy            = global->cvar->find_var(XOR("tf_parachute_maxspeed_xy"));
    global->tf_parachute_maxspeed_z             = global->cvar->find_var(XOR("tf_parachute_maxspeed_z"));
    global->tf_grapplinghook_move_speed         = global->cvar->find_var(XOR("tf_grapplinghook_move_speed"));
    global->tf_max_charge_speed                 = global->cvar->find_var(XOR("tf_max_charge_speed"));
    global->tf_parachute_aircontrol             = global->cvar->find_var(XOR("tf_parachute_aircontrol"));
    global->tf_halloween_kart_dash_speed        = global->cvar->find_var(XOR("tf_halloween_kart_dash_speed"));
    global->tf_halloween_kart_aircontrol        = global->cvar->find_var(XOR("tf_halloween_kart_aircontrol"));
    global->tf_flamethrower_boxsize             = global->cvar->find_var(XOR("tf_flamethrower_boxsize"));
    global->tf_fireball_speed                   = global->cvar->find_var(XOR("tf_fireball_speed"));

    // For auto-recording matches with cheaters in them.
    global->ds_enable                           = global->cvar->find_var(XOR("ds_enable"));
    global->ds_notify                           = global->cvar->find_var(XOR("ds_notify"));
    global->ds_log                              = global->cvar->find_var(XOR("ds_log"));
    global->ds_sound                            = global->cvar->find_var(XOR("ds_sound"));
    global->ds_autodelete                       = global->cvar->find_var(XOR("ds_autodelete"));
    global->ds_dir                              = global->cvar->find_var(XOR("ds_dir"));
    global->ds_prefix                           = global->cvar->find_var(XOR("ds_prefix"));
    global->ds_min_streak                       = global->cvar->find_var(XOR("ds_min_streak"));
    global->cl_autoreload                       = global->cvar->find_var(XOR("cl_autoreload"));
    global->cl_ragdoll_fade_time                = global->cvar->find_var(XOR("cl_ragdoll_fade_time"));
    global->cl_ragdoll_forcefade                = global->cvar->find_var(XOR("cl_ragdoll_forcefade"));
    global->cl_ragdoll_physics_enable           = global->cvar->find_var(XOR("cl_ragdoll_physics_enable"));

    c_cvar* tf_datacenter_ping_interval = global->cvar->find_var(XOR("tf_datacenter_ping_interval"));
    if(tf_datacenter_ping_interval != nullptr){
      tf_datacenter_ping_interval->set_value(XOR("5"));
    }

    // Make these cvars not recorded by the demosystem. Requested feature.
    {
      global->cvar->dont_record(XOR("volume"));
      global->cvar->dont_record(XOR("+taunt"));
      global->cvar->dont_record(XOR("toggleconsole"));
      global->cvar->dont_record(XOR("open_charinfo_direct"));
      global->cvar->dont_record(XOR("open_charinfo_backpack"));
      global->cvar->dont_record(XOR("changeclass"));
      global->cvar->dont_record(XOR("changeteam"));
      global->cvar->dont_record(XOR("showmapinfo"));
      global->cvar->dont_record(XOR("show_quest_log"));
      global->cvar->dont_record(XOR("+inspect"));
      global->cvar->dont_record(XOR("-inspect"));
      global->cvar->dont_record(XOR("tf_party_chat"));
      global->cvar->dont_record(XOR("tf_party_request_join_user"));
      global->cvar->dont_record(XOR("gameui_activate"));
      global->cvar->dont_record(XOR("gameui_hide"));
    }

#if defined(DEV_MODE)
    global->engine->client_cmd(XOR("sv_cheats 1; fps_max 0; developer 1; mp_teams_unbalance_limit 30; mp_teams_unbalance_limit 1; mp_respawnwavetime 0"));
#endif
  }

  {
    render_debug->setup_fonts();
    player_list_handler->alloc();

    // Classes with base classes since the alloc macro does not allocate it properly.
    target_list = new c_target_list;
    target_list->setup();

    rijin               = new c_rijin_product;
    accuracy            = new c_accuracy;
    packet_manager      = new c_packet_manager;
    interp_manager      = new c_interpolation_manager;
    command_manager     = new c_command_manager;
    dormant_manager     = new c_dormant_manager;
    acm                 = new c_anti_cheat_manager;
    freecam             = new c_freecamera;
    engine_prediction   = new c_engine_prediction;
    seed_prediction     = new c_seed_prediction;
    cheat_detection     = new c_cheat_detection;
    lag_compensation    = new c_lag_compensation;
    privacy_mode        = new c_privacy_mode;
    movement            = new c_movement;
    thirdperson         = new c_thirdperson;
    dispatch_user_msg   = new c_dispatch_user_message_features;
    fast_queue          = new c_fast_queue;
    viewmodel           = new c_viewmodel_offset;
    logs                = new c_log_manager;
    chat_format_manager = new c_chat_format_manager;

    lag_compensation->setup();

    global->base_lag_compensation  = (c_base_lag_compensation*)lag_compensation;
    global->base_cheat_detection   = (c_base_cheat_detection*)cheat_detection;
    global->base_packet_manager    = (c_base_packet_manager*)packet_manager;
    global->base_acm               = (c_base_anti_cheat_manager*)acm;
    global->base_log               = (c_base_log_manager*)logs;
    global->base_dispatch_user_msg = (c_dispatch_user_message_features*)dispatch_user_msg;

    global->cvar_hash_table = new s_pointer_hash_table;
    global->cvar_hash_table->table = malloc(sizeof(s_pointer_hash_data) * 8912);
  }

  // setup GUI
  {
    // Calculate the screen size so the GUI has it
    render->screen_size = global->engine->get_screen_size();

    //Segoe UI
    if(global->gui_font == nullptr){
      global->gui_font  = render->create_font(XOR("Segoe UI"), 14, FW_MEDIUM, CLEARTYPE_QUALITY);
      assert(global->gui_font != nullptr);
    }

    if(global->gui_font_emoji == nullptr){
      global->gui_font_emoji = render->create_font(XOR("Segoe UI Emoji"), 14, FW_MEDIUM, CLEARTYPE_QUALITY);
      assert(global->gui_font_emoji != nullptr);
    }

    if(global->menu == nullptr){
      global->menu  = gcs::add_scene<gcs_menu>();
      assert(global->menu != nullptr);
    }

    if(global->double_tap_panel == nullptr){
      global->double_tap_panel  = gcs::add_scene<gcs_double_tap_panel>();
      assert(global->double_tap_panel != nullptr);
    }

    if(global->crit_hack_panel == nullptr){
      global->crit_hack_panel  = gcs::add_scene<gcs_crit_hack_panel>();
      assert(global->crit_hack_panel != nullptr);
    }

    if(global->spectator_list_panel == nullptr){
      global->spectator_list_panel = gcs::add_scene<gcs_spectator_list_panel>();
      assert(global->spectator_list_panel != nullptr);

      global->spectator_list_panel->init(&config->spectator_panel_pos, &config->visual.show_spectators, &config->visual.show_spectators_localplayer);
    }

    if(global->info_panel == nullptr){
      global->info_panel = gcs::add_scene<gcs_info_panel>();
      assert(global->info_panel != nullptr);
    }

    if(global->proj_impact_camera_panel == nullptr){
      global->proj_impact_camera_panel = gcs::add_scene<gcs_proj_impact_camera_panel>();
      assert(global->proj_impact_camera_panel != nullptr);
    }

    if(global->target_window == nullptr){
      global->target_window = FindWindowW(WXOR(L"Valve001"), WXOR(L"Team Fortress 2 - Direct3D 9 - 64 Bit"));
      assert(global->target_window != nullptr);
    }
  }

  // Setup additional fonts
  {
    if(global->esp_font_large == nullptr)
      global->esp_font_large = render->create_font(XOR("Tahoma"), 12, FW_SEMIBOLD, ANTIALIASED_QUALITY);

    assert(global->esp_font_large != nullptr);

    if(global->esp_font_small == nullptr)
      global->esp_font_small = render->create_font(XOR("Small Fonts"), 8, FW_MEDIUM, DEFAULT_QUALITY);

    assert(global->esp_font_small != nullptr);
  }

  if(!gen_internal->setup()){
    DBG("[!] not loaded waiting.\n");
    return false;
  }

#if !defined(DEV_MODE)
  // reconnect the user if we are in a server
  {
    if(utils::is_in_game())
      global->engine->client_cmd(XOR("retry"));
  }
#endif

  global->memory_manager.setup();

  #if !defined(DEV_MODE)
  #if defined(STAGING_MODE)
    PRODUCT_STATUS("PRODUCT_STAGING_LOADED");
  #else
    PRODUCT_STATUS("PRODUCT_SHIPPING_LOADED");
  #endif
  #else
    PRODUCT_STATUS("PRODUCT_DEV_LOADED");
  #endif

  file_map->create(HASH("RIJIN_PRODUCT"));

  notify->create(NOTIFY_TYPE_SUCCESS, WXOR(L"RijiN has successfully loaded! Press insert, fn+insert, or F3 to open the menu."), colour(255, 255, 255, 255));

  DBG("[!] Loaded.\n");
  return true;
}

EXPORT HRESULT __fastcall d3d9_hook(void* device, RECT* source_rect, RECT* dest_rect, HWND window_override, void* region){
  if(!setup(device))
    return utils::call_fastcall64<HRESULT, RECT*, RECT*, HWND, void*>(gen_internal->decrypt_asset(global->d3d9_hook_trp), device, source_rect, dest_rect, window_override, region);

  kernel_time_expired_close();
  assert(global->engine != nullptr);
  if(render->handle_device_states(device, global->engine->get_screen_size()))
    return utils::call_fastcall64<HRESULT, RECT*, RECT*, HWND, void*>(gen_internal->decrypt_asset(global->d3d9_hook_trp), device, source_rect, dest_rect, window_override, region);

  IDirect3DStateBlock9* state_block = render->begin();
  {
    rijin->d3d9_input(render);
    rijin->d3d9_hook_game_specific(render);

    if(global->client_state != nullptr)
      render->draw_watermark(global->gui_font, !utils::is_in_game() || global->menu_open);

    rijin->d3d9_hook_ui(render);
  }
  render->end(state_block);

  kernel_time_24h_close();

  #if defined(DEV_MODE)
  if(gen_internal->should_unload()){
    global->unloading = true;
    
    utils::lock_mouse(false);

    player_list_handler->dealloc();
    gen_internal->unload();
    global->reset();
    gcs::free_memory_stack();
    lag_compensation->unload();
    render->unload();
    logs->unload();
    file_map->destroy(HASH("RIJIN_PRODUCT"));
  }
  #endif

  return utils::call_fastcall64<HRESULT, RECT*, RECT*, HWND, void*>(gen_internal->decrypt_asset(global->d3d9_hook_trp), device, source_rect, dest_rect, window_override, region);
}

EXPORT HRESULT __fastcall d3d9_reset_hook(void* rcx, void* param, void* fullscreen_display_mode){
  DBG("[!] d3d9 reset\n");

  render->is_device_resetting = true;
  render->on_device_lost();
  HRESULT result = utils::call_fastcall64<HRESULT, void*, void*>(gen_internal->decrypt_asset(global->d3d9_reset_trp), rcx, param, fullscreen_display_mode);
  render->on_device_reset();
  render->is_device_resetting = false;

  if(result == D3D_OK)
    render->needs_to_wait = false;
  
  render->device_reset = result == D3D_OK;
  render->needs_to_wait = result != D3D_OK;
  return result;
}