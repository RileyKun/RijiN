#pragma once

#define GLOBAL_SETUP() if(global == nullptr) global = new c_global; \
global_internal = (c_base_global*)global; \

class gcs_menu;
class gcs_double_tap_panel;
class gcs_crit_hack_panel;
class gcs_fake_latency_panel;
class gcs_spectator_list_panel;
class gcs_info_panel;
class gcs_proj_impact_camera_panel;
class c_settings;
class c_engine_prediction;
class s_lc_data;

enum aimbot_running_flags{
  AIMBOT_AUTO_REPAIR = 1,
  AIMBOT_MAX
};


// Put data in here that is unsafe
struct s_tf2_data{
  bool  is_crit_boosted;
  bool  is_dueling;
  i32   max_buffed_health;
  bool  condition_active[TF_COND_COUNT];
  float condition_active_time[TF_COND_COUNT];
  i32   ground_entity_index;

  bool use_rapid_fire_crits;

  wchar_t         weapon_name[128];
  i32             weapon_index;
  i32             heal_bolt_amount;
  i32             rta_target_flags;
};

#define MAX_ENTITY_DATA_SPLASH_POINTS 2048
class c_entity_data : public c_base_entity_data, public c_base_player_dormant_data{
public:

  float dormant_received_party_timeout;

  vec3 splash_points[MAX_ENTITY_DATA_SPLASH_POINTS];
  u32  splash_point_count;
  
  float last_splash_point_sim_time;
  vec3  last_splash_point_origin;
};

class c_player_list_data : public c_base_player_list_data{
public:
  void init() override{
    c_base_player_list_data::init();
    ignore_auto_heal = false;
  }

  u32 get_size() override{
    return sizeof(c_player_list_data);
  }

  bool ignore_auto_heal = false;
};

class c_player_data : public c_base_player_data, public c_base_player_dormant_data{
public:
  // cheat detection tf2
  bool  has_submitted_bot_to_network;
  float bot_infraction_reset;
  float bot_gesture_reset_time;
  i32   bot_move_infractions;
  i32   bot_infractions;
  i32   bot_gestures;
  bool  has_bot_hats;
  float human_factor;

  u32 wearables[8];
  i32 wearable_tracked;
  u32 wearable_count;
  i32 wearable_suspect;

  bool high_headshot_ratio;
  bool high_kd;

  i32   hp_healed;
  float hp_healed_timeout;

  i32 old_health;
  i32 new_health;
  i32 syn_health;
  i32 health_before_death;

  // resolver networking
  float last_resolver_data_msg_time;

  float dormant_send_party_timeout;
  float dormant_received_party_timeout;

  bool  party_member;
  float next_party_member_check;

  // This really should be moved to a base class!
  float               last_pred_change;
  float               pred_change;
  float               pred_change_delta;
  s_pred_track_array  pred_track_array;
  bool                pred_mod;
  float               pred_real_time;
  u32                 pred_flags;
  bool                bypass_hit_chance;
  u32                 pre_pred_flags;

  float               pred_last_yaw_delta;

  vec3  splash_points[MAX_ENTITY_DATA_SPLASH_POINTS];
  u32   splash_point_count;
  float last_splash_update_time;

  float last_splash_point_sim_time;
  vec3  last_splash_point_origin;

  float sniper_charge_damage;

  // For the anti-backtrack feature.
  bool seen_localplayer;

  // How much times did they attempt to votekick us or our friend?
  i32 attempted_votekicks;

  s_tf2_data tfdata;
  c_player_list_data playerlist;
  u32                playerlist_hash;
  u32                playerlist_cache_hash;
  bool               playerlist_notify;

  vec3               last_predicted_velocity;

  bool               override_dme_matrix;
  matrix3x4*         dme_matrix;

  float evasion_last_sim_time;
  bool  evasion_first_time;
  vec3  evasion_previous_aoa;
  u32   evasion_triggers;
  float evasion_trigger_reset_time;
};

class c_game_specific_data{
public:
  s_shared*           localplayer_shared        = nullptr; // this should only be used for basic tests (NOT SAFE FOR ACCESS)

  u64 bsendpacket_rdi_rsp = 0;

  // player data related
  c_player_data player_data[MAXPLAYERS]; // EDIT: Now its 101 (07.25.2023) -rud, 32 is the aboluste maximum it can go

  // crithack related
  i32 last_fired_weapon_id;
  i32 damage_to_unban;
  i32 display_damage_to_unban; // Use this for showing values to the user.
  i32 total_round_damage;
  i32 total_round_crit_random_damage;
  i32 total_round_crit_boosted_damage;
  i32 total_round_melee_damage;

  // engine related
  bool            reset_resist_state                      = false;
  i32             resist_uber_state                       = -1;
  i32             resist_predicted                        = -1;
  i32             resist_networked                        = -1;
  float           resist_check_prediction_time            = 0.f;
  bool            resist_reset                            = false;
  bool            auto_vaccinator_reload_held             = false;
  bool            auto_vaccinator_prefer_resist           = -1;
  bool            createmove_force_attack2                = false;
  float           next_vote_kick_time                     = 0.f;
  bool            cant_start_vote_kick                    = false;


  // for the laser dot colour
  i32 laser_dot_team_backup                               = 0;

  // ragdoll effects
  bool block_golden_kill_sound                            = false;
  float next_auto_accept_execute                          = 0.f;

  // synced tickbase we can use for estimations on the server when dt is taking place
  i32 last_sent_tickbase;
  float time_between_shots;

  bool cheat_recorded = false;

  // anti-backtrack feature for fakelag
  bool anti_backtrack_choke;
  bool anti_backtrack_choke_started;

  // peek feature for fakelag
  bool peek_fakelag_choke;
  bool peek_fakelag_choke_started;

  // resolver data
  float next_resolver_broadcast_time;

  // Got rate limited by GC due to sending too many party chat messages.
  float party_chat_rate_limited = -1.f;
  float animation_fire_time = 0.f;

  // prediction track related
  i32   last_shot_predicted_player = 0;
  float last_shot_predicted_player_expire = 0.f;
  bool  skip_trajectory_check = false;
  i32   best_spread_command_number = 0;

  s_pred_track_array  proj_track_array;
  vec3                proj_track_radius[512];
  u32                 proj_track_radius_points;
  vec3                proj_track_angles;
  vec3                proj_track_obb_min;
  vec3                proj_track_obb_max;

  // firing this tick
  bool firing_this_tick;

  // For anti-afk
  float last_movement_time;

  vec3 sentry_end_origin;

  i32 resolver_manual_target = 0;

  c_player_list_data  playerlist_data;

  ALWAYSINLINE void reset(){
    DBG("[!] game data reset\n");

    utils::reset_all_lag_comp_data();
    utils::reset_all_player_data();

    memset(this, 0, XOR32(sizeof(c_game_specific_data)));

    input_system->reset();
  }
};

#define globaldata global->global_data
#define config global->settings
class c_global : public c_base_global, public c_game_specific_data{
public:
  gcs_menu*                      menu                      = nullptr;
  gcs_double_tap_panel*          double_tap_panel          = nullptr;
  gcs_crit_hack_panel*           crit_hack_panel           = nullptr;
  gcs_spectator_list_panel*      spectator_list_panel      = nullptr;
  gcs_info_panel*                info_panel                = nullptr;
  gcs_proj_impact_camera_panel*  proj_impact_camera_panel  = nullptr;
  c_settings*                    settings                  = nullptr;

  u64                            hud_scope_id              = 0;
  c_aimbot_settings*             aimbot_settings           = nullptr;
  bool                           aimbot_settings_filled    = false;
  i32                            current_player_class      = 0;
  s_info                         info[128];

  // Pointers
  s_input*        input                                             = nullptr;
  void*           get_team_check_scoreboard_retaddr1                = nullptr;
  void*           get_team_status_panel_retaddr1                    = nullptr;
  void*           ctfplayer_fireevent_retaddr1                      = nullptr;
  void*           displaydmgfeedback_retaddr1                       = nullptr;
  void*           tf2_getmaxspeed                                   = nullptr;
  void*           ctfplayer_createmove_retaddr1                     = nullptr;
  void*           ctfplayer_createmove_retaddr2                     = nullptr;
  void*           ctfplayer_createmove_retaddr3                     = nullptr;
  void*           ctfwearable_shoulddraw_retaddr1                   = nullptr;
  void*           ctfplayer_shoulddraw_retaddr1                     = nullptr;
  void*           ctfplayershared_addcond                           = nullptr;
  void*           ctfplayershared_removecond                        = nullptr;
  void*           util_particletracer                               = nullptr;
  void*           ctfplayer_canmoveduringtaunt                      = nullptr;
  void*           shoulddraw_viewmodel_retaddr1                     = nullptr;
  void*           get_client_voice_manager                          = nullptr;
  void*           set_player_blocked_state                          = nullptr;
  void*           is_player_blocked                                 = nullptr;
  void*           is_crit_boosted                                   = nullptr;
  void*           medigun_find_and_heal_target_retaddr1             = nullptr;
  void*           get_ammo_count                                    = nullptr;
  void*           is_steam_friend_get_player_info_retaddr1          = nullptr;
  void*           update_player_avatar_get_player_info_retaddr2     = nullptr;
  void*           attrib_hook_value_float                           = nullptr;
  void*           get_class_name                                    = nullptr;
  void*           look_weapon_info_slot                             = nullptr;
  void*           file_weapon_info_from_handle                      = nullptr;
  void*           hud_element_addr                                  = nullptr;
  void*           get_hud_element_addr                              = nullptr;
  void*           get_max_buffed_health_addr                        = nullptr;
  void*           chatprintf_retaddr1                               = nullptr;
  void*           gamerules_addr                                    = nullptr;
  void*           ctfinput_applymouse_retaddr1                      = nullptr;
  void*           shared_object_cache_addr                          = nullptr;
  void*           find_base_type_cache_addr                         = nullptr;
  void*           displaydmgfeedback_retaddr4                       = nullptr;
  void*           entity_sphere_query_addr                          = nullptr;
  void*           get_server_animating_addr                         = nullptr;
  void*           draw_server_hitbox_addr                           = nullptr;
  void*           get_projectile_fire_setup                         = nullptr;
  void*           attrib_hook_value_int                             = nullptr;
  void*           ctrace_filter_simple                              = nullptr;
  void*           dispatch_particle_effect                          = nullptr;
  void*           collision_property_calcnearestpoint               = nullptr;
  void*           ctfplayer_hastheflag                              = nullptr;
  s_cmd_alias*    cmd_alias_list                                    = nullptr;
  void*           cnewparticle_get_effect_name_addr                 = nullptr;
  void*           csniperdot_clientthink_retaddr1                   = nullptr;
  void*           is_crit_boosted_material_retaddr1                 = nullptr;
  void*           update_crit_boost_effect_is_crit_boost_retaddr1   = nullptr;
  void*           displaydmgfeedback_isplayerclass_retaddr          = nullptr;
  void*           cnewparticle_effect_render_retaddr                = nullptr;
  void*           cdisguisestatus_paint_retaddr                     = nullptr;
  void*           cdisguisestatus_shoulddraw_retaddr                = nullptr;
  void*           ctfhudplayerclass_onthink_retaddr                 = nullptr;
  void*           ctfhudplayerclass_update_model_panel_retaddr      = nullptr;
  void*           load_itempreset_incond_retaddr                    = nullptr;
  void*           updateinvequippedstate_retaddr                    = nullptr;
  void*           get_spread_angles_addr                            = nullptr;
  void*           ctfpartyclient_func_addr                          = nullptr;
  void*           staticpropmgr_drawstaticprop_retaddr1             = nullptr;
  void*           ctf_firebullet_retaddr1                           = nullptr;
  void*           iclientfriends_getfriendrelationship_retaddr1     = nullptr;
  void*           iclientfriends_getfriendrelationship_retaddr2     = nullptr;
  void*           nil_seq_desc_addr                                 = nullptr;
  void*           get_equip_wearable_for_loadout_slot_addr          = nullptr;
  void*           setup_bones_base_entity_teleport_retaddr          = nullptr;
  void*           engineclient_ispaused_retaddr                     = nullptr;
  void*           baseentity_calculate_abs_velocity_addr            = nullptr;

  // Interfaces
  c_interface_trace*              trace             = nullptr;

  // Cvars
  c_cvar*                   sv_maxunlag                         = nullptr;
  c_cvar*                   tf_weapon_criticals                 = nullptr;
  c_cvar*                   tf_weapon_criticals_melee           = nullptr;
  c_cvar*                   tf_weapon_criticals_bucket_cap      = nullptr;

  c_cvar*                   tf_flamethrower_velocity            = nullptr;
  c_cvar*                   tf_grapplinghook_move_speed         = nullptr;
  c_cvar*                   tf_max_charge_speed                 = nullptr;
  c_cvar*                   tf_parachute_aircontrol             = nullptr;
  c_cvar*                   tf_halloween_kart_dash_speed        = nullptr;
  c_cvar*                   tf_halloween_kart_aircontrol        = nullptr;
  c_cvar*                   tf_flamethrower_boxsize             = nullptr;
  c_cvar*                   tf_fireball_speed                   = nullptr;

  c_cvar*                   ds_enable                           = nullptr;
  c_cvar*                   ds_notify                           = nullptr;
  c_cvar*                   ds_log                              = nullptr;
  c_cvar*                   ds_sound                            = nullptr;
  c_cvar*                   ds_autodelete                       = nullptr;
  c_cvar*                   ds_dir                              = nullptr;
  c_cvar*                   ds_prefix                           = nullptr;
  c_cvar*                   ds_min_streak                       = nullptr;
  c_cvar*                   tf_parachute_maxspeed_xy            = nullptr;
  c_cvar*                   tf_parachute_maxspeed_z             = nullptr;

  c_cvar*                   cl_autoreload                       = nullptr;
  c_cvar*                   cl_ragdoll_fade_time                = nullptr;
  c_cvar*                   cl_ragdoll_forcefade                = nullptr;
  c_cvar*                   cl_ragdoll_physics_enable           = nullptr;


  // Hook original pointers
  void*                     d3d9_hook_trp                                         = nullptr;
  void*                     d3d9_reset_trp                                        = nullptr;
  void*                     create_move_hook_trp                                  = nullptr;
  void*                     override_view_hook_trp                                = nullptr;
  void*                     engine_paint_hook_trp                                 = nullptr;
  void*                     paint_traverse_hook_trp                               = nullptr;
  void*                     player_resource_get_team_trp                          = nullptr;
  void*                     player_panel_get_team_trp                             = nullptr;
  void*                     ctfplayershared_incondition_trp                       = nullptr;
  void*                     check_for_pure_server_whitelist_hook_trp              = nullptr;
  void*                     get_direct_ping_to_pop_hook_trp                       = nullptr;
  void*                     get_direct_ping_to_data_center_hook_trp               = nullptr;
  void*                     ctfplayer_geteffectiveinvislevel_trp                  = nullptr;
  void*                     ctfplayer_isplayerclass_hook_trp                      = nullptr;
  void*                     frame_stage_notify_hook_trp                           = nullptr;
  void*                     float_decode_hook_trp                                 = nullptr;
  void*                     int_decode_hook_trp                                   = nullptr;
  void*                     medigun_itempostframe_hook_trp                        = nullptr;
  void*                     ctfplayer_avoidplayers_hook_trp                       = nullptr;
  void*                     ctfwearable_shoulddraw_hook_trp                       = nullptr;
  void*                     post_think_hook_trp                                   = nullptr;
  void*                     fire_event_hook_trp                                   = nullptr;
  void*                     util_tracer_hook_trp                                  = nullptr;
  void*                     ctfplayer_firebullet_hook_trp                         = nullptr;
  void*                     cl_move_hook_trp                                      = nullptr;
  void*                     usermessages_dispatch_user_message_hook_trp           = nullptr;
  void*                     update_step_sound_hook_trp                            = nullptr;
  void*                     get_tracer_type_hook_trp                              = nullptr;
  void*                     cviewrender_setscreenoverlaymaterial_hook_trp         = nullptr;
  void*                     ctfplayer_createplayergibs_hook_trp                   = nullptr;
  void*                     base_animating_setupbones_hook_trp                    = nullptr;
  void*                     set_dormant_hook_trp                                  = nullptr;
  void*                     get_outer_abs_velocity_hook_trp                       = nullptr;
  void*                     hud_chatline_insert_and_colorize_text_hook_trp        = nullptr;
  void*                     calc_is_attack_critical_hook_trp                      = nullptr;
  void*                     minigun_weapon_sound_update_hook_trp                  = nullptr;
  void*                     emit_sound_hook_trp                                   = nullptr;
  void*                     cinput_get_user_cmd_hook_trp                          = nullptr;
  void*                     process_packet_hook_trp                               = nullptr;
  void*                     send_datagram_hook_trp                                = nullptr;
  void*                     buf_addtext_withmarkers_hook_trp                      = nullptr;
  void*                     cbaseentity_followentity_hook_trp                     = nullptr;
  void*                     fx_firebullets_hook_trp                               = nullptr;
  void*                     can_fire_random_critical_shot_hook_trp                = nullptr;
  void*                     server_calc_is_attack_critical_hook_trp               = nullptr;
  void*                     server_fire_bullet_hook_trp                           = nullptr;
  void*                     server_adjust_player_time_base_hook_trp               = nullptr;
  void*                     engine_get_player_info_hook_trp                       = nullptr;
  void*                     is_allowed_to_withdraw_from_crit_bucket_hook_trp      = nullptr;
  void*                     add_to_crit_bucket_hook_trp                           = nullptr;
  void*                     ctfragdoll_create_ragdoll_hook_trp                    = nullptr;
  void*                     cbaseanimating_update_client_side_animation_hook_trp  = nullptr;
  void*                     ctfviewmodel_calcviewmodelview_hook_trp               = nullptr;
  void*                     cnetchan_sendnetmsg_hook_trp                          = nullptr;
  void*                     studio_render_context_drawmodel_hook_trp              = nullptr;
  void*                     calc_renderable_world_space_aabb_fast_hook_trp        = nullptr;
  void*                     menu_lobbychat_hook_trp                               = nullptr;
  void*                     cbuf_executecmd_hook_trp                              = nullptr;
  void*                     cvar_set_value_hook_trp                               = nullptr;
  void*                     voice_assign_channel_hook_trp                         = nullptr;
  void*                     get_local_view_angles_hook_trp                        = nullptr;
  void*                     do_post_screen_space_effects_hook_trp                 = nullptr;
  void*                     s_startsound_hook_trp                                 = nullptr;
  void*                     draw_model_execute_hook_trp                           = nullptr;
  void*                     calc_viewmodel_bob_hook_trp                           = nullptr;
  void*                     engine_trace_ray_hook_trp                             = nullptr;
  void*                     cbase_combat_weapon_hook_trp                          = nullptr;
  void*                     ctfplayershared_onconditionadded_hook_trp             = nullptr;
  void*                     ctfplayershared_onconditionremoved_hook_trp           = nullptr;
  void*                     ctfplayeranimstate_aimyaw_hook_trp                    = nullptr;
  void*                     cmaterial_uncache_hook_trp                            = nullptr;
  void*                     chudcrosshair_getdrawposition_hook_trp                = nullptr;
  void*                     cbaseclientstate_process_print_hook_trp               = nullptr;
  void*                     cnewparticle_effect_render_hook_trp                   = nullptr;
  void*                     csniperdot_clientthink_hook_trp                       = nullptr;
  void*                     particlesystem_setcontrolpoint_hook_trp               = nullptr;
  void*                     is_crit_boosted_hook_trp                              = nullptr;
  void*                     r_setfogvolumestate_hook_trp                          = nullptr;
  void*                     get_fog_enable_hook_trp                               = nullptr;
  void*                     cbaseclientstate_process_set_convar_hook_trp          = nullptr;
  void*                     studio_render_forced_material_override_hook_trp       = nullptr;
  void*                     ctfplayerinventory_getmaxitems_hook_trp               = nullptr;
  void*                     client_entity_list_on_add_entity_hook_trp             = nullptr;
  void*                     client_entity_list_on_remove_entity_hook_trp          = nullptr;
  void*                     ccvar_find_command_hook_trp                           = nullptr;
  void*                     crenderview_renderview_hook_trp                       = nullptr;
  void*                     inventorymanager_loadpreset_hook_trp                  = nullptr;
  void*                     sentry_muzzle_flash_hook_trp                          = nullptr;
  void*                     tracer_callback_hook_trp                              = nullptr;
  void*                     ctfviewmodel_calcviewmodellag_hook_trp                = nullptr;
  void*                     ctfweaponbase_getshootsound_hook_trp                  = nullptr;
  void*                     input_cam_is_thirdperson_hook_trp                     = nullptr;

  void*                     get_exposure_range_hook_trp                                    = nullptr;
  void*                     get_bloom_amount_hook_trp                                      = nullptr;
  void*                     add_resist_shield_hook_trp                                     = nullptr;
  void*                     equippable_items_for_slotgenerator_constructor_hook_trp        = nullptr;
  void*                     inventory_manager_updateinv_equipped_state_hook_trp            = nullptr;
  void*                     generate_equip_region_conflict_mask_hook_trp                   = nullptr;
  void*                     maintain_sequence_transition_hook_trp                          = nullptr;
  void*                     set_context_think_hook_trp                                     = nullptr;
  void*                     is_blacklisted_hook_trp                                        = nullptr;
  void*                     create_particle_hook_trp                                       = nullptr;
  void*                     cstudiohdr_seqdesc_hook_trp                                    = nullptr;
  void*                     cbaseentity_teleported_hook_trp                                = nullptr;
  void*                     engineclient_ispaused_hook_trp                                 = nullptr;
  void*                     cbaseentity_baseinterpolatepart1_hook_trp                      = nullptr;

  void*                     physics_object_getdragindir_hook_trp                  = nullptr;
  void*                     physics_object_getangdir_hook_trp                     = nullptr;
  void*                     physics_clamp_velocity_hook_trp                       = nullptr;

  void*                     rtl_dispatch_exception_hook_trp                       = nullptr;

  c_memory_manager memory_manager;
  s_pointer_hash_table* cvar_hash_table;

  ALWAYSINLINE bool valid(){
    return client_state != nullptr && client_state->signon_state() == SIGNONSTATE_FULL && view_setup != nullptr && engine != nullptr && localplayer != nullptr;
  }

  c_internal_base_player* update_local_data() override{
    localplayer = c_base_global::update_local_data();

    c_base_player* player = (c_base_player*)localplayer;

    localplayer_shared     = player != nullptr ? player->shared() : nullptr;
    current_player_class   = player != nullptr ? player->player_class() : current_player_class;

    c_aimbot_settings* as  = utils::get_aimbot_settings();
    if(as != nullptr)
      aimbot_settings      = as;

    aimbot_settings_filled = aimbot_settings != nullptr;
    return localplayer;
  }

  ALWAYSINLINE void on_death(){
    resist_reset = true;
    auto_vaccinator_reload_held   = false;
    auto_vaccinator_prefer_resist = -1;
    aimbot_target_index           = -1;
    aimbot_target_reset_time      = -1.f;
    aimbot_target_type            = -1;
    animation_fire_time           = -1.f;
    memset(&proj_track_array, 0, sizeof(s_pred_track_array));
  }

  ALWAYSINLINE void on_aimbot_target(i32 index, vec3 aim_point, c_base_weapon* weapon, u32 type = 0){
    if(weapon == nullptr)
      return;

    ctf_weapon_info* wep_info = weapon->get_weapon_info();
    if(wep_info == nullptr)
      return;

    aimbot_target_aim_point  = aim_point;
    aimbot_target_index      = index;
    aimbot_target_reset_time = math::time() + (wep_info->time_fire_delay);
    aimbot_target_type       = type;
  }
};

CLASS_EXTERN(c_global, global);