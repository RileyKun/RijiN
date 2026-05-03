#pragma once
#define SETTINGS_VERSION 4

#define CONFIG_RGBA(name, r, g, b, a) float name[4] = {r,g,b,a};
#define CONFIG_RGB(name, r, g, b) CONFIG_RGBA(name, r, g, b, 255);

enum player_esp_settings_type{
  PLAYER_ESP_SETTING_TYPE_RED_TEAM = 0,
  PLAYER_ESP_SETTING_TYPE_BLU_TEAM = 1,
  PLAYER_ESP_SETTING_TYPE_FRIEND = 2,
};

enum world_esp_settings_type{
  WORLD_ESP_SETTING_TYPE_RED_TEAM = 0,
  WORLD_ESP_SETTING_TYPE_BLU_TEAM = 1,
  WORLD_ESP_SETTING_TYPE_UNASSIGNED_TEAM = 2,
  WORLD_ESP_SETTING_TYPE_BACKTRACK = 3,
  WORLD_ESP_SETTING_TYPE_FAKEANGLE = 4,
};

enum aimbot_weapon_settings_type{
  AIMBOT_WEAPON_SETTING_HITSCAN = 0,
  AIMBOT_WEAPON_SETTING_PROJ = 1,
  AIMBOT_WEAPON_SETTING_MEDIGUN = 2,
  AIMBOT_WEAPON_SETTING_MELEE = 3,
  AIMBOT_WEAPON_SETTING_MAX,
};

class c_aimbot_settings{
public:
  union{
    struct{
      // aimbot
      bool  enabled                         = false;
      u8    aim_hotkey                      = VK_XBUTTON2;
      bool  aim_mode_automatic              = false;
      bool  aim_mode_on_attack              = false;
      bool  aim_mode_on_key                 = true;
      i32   max_fov                         = 180;
      float smoothing                       = 0;
      bool  position_adjust_history         = true;
      bool  position_adjust_accurate_unlag  = true;

      // target
      bool  target_friendlies               = false;
      bool  target_enemies                  = true;
      bool  target_cloaked_spies            = true;
      bool  target_friends                  = false;
      bool  target_method_closest_to_fov    = true;
      bool  target_method_distance          = false;
      bool  priority_hitbox[17];
      bool  hitbox[17];

      // other
      bool  unused3                         = true;
      bool  sniper_auto_scope               = true; // NO LONGER USED
      bool  sniper_wait_for_charge          = false;
      bool  sniper_wait_for_headshot        = true;
      bool  sniper_body_aim_if_lethal       = true;
      bool  no_spread                       = false;
      bool  spread_manipulation             = true;
      float spread_manipulation_hit_chance  = 8.f;

      bool auto_medigun_friendsonly                         = false;
      bool auto_medigun_preferfriends                       = false;
      bool auto_medigun_preferfriends_except_on_key         = false;
      u8   auto_medigun_preferfriends_except_on_key_hotkey  = VK_XBUTTON2;
      bool auto_medigun_preferall                           = true;

      bool mvm_ignore_healed                = false;
      bool mvm_ignore_sentrybuster          = false;
      bool spy_ambassador_wait_for_spread   = true;

      bool target_sentries              = true;
      bool target_dispenser             = false;
      bool target_teleporter            = false;
      bool target_sticky                = true;
      bool target_boss                  = false;

      bool minigun_auto_rev             = false;

      bool unused01                     = true;
      i32  unused02                     = 180;
      bool spy_ambassador_body_aim_if_lethal = true;

      bool silent_aim_none        = false;
      bool silent_aim_clientside  = true;
      bool silent_aim_serverside  = false;

      float double_donk_above_ms = 0.2f;

      bool heal_arrow_enabled     = true;
      i32  heal_arrow_max_fov     = 180;
      bool heal_arrow_any         = false;
      bool heal_arrow_friendsonly = true;

      float projectile_prediction_min_hit_chance    = 25.f;
      float projectile_prediction_max_travel_time   = 1000.f;
      bool  unused1         = true;
      CONFIG_RGB(unused2, 255, 255, 255);
      bool sniper_body_aim_if_crit_immune           = true;
      bool auto_shoot                               = true;

      bool unused04                                 = false;
      u8   unused03                                 = VK_XBUTTON2;

      bool heal_arrow_use_key                       = false;
      u8   heal_arrow_key                           = VK_XBUTTON2;

      bool sniper_scoped_only                       = false;

      bool force_body_aim_on_key                    = false;
      u8   force_body_aim_key                       = 0x58;

      bool sniper_ignore_vaccinator                 = false;

      bool  point_scale_enabled                     = false;
      float point_scale                             = 0.01f;
      bool  spy_ignore_razer_back                   = false;

      bool loose_cannon_target_ubercharged          = false;
      bool loose_cannon_target_bonked               = false;
      bool splash_assist                            = false;
      bool predict_evasion                          = true;
      bool obb_auto                                 = true;
      bool obb_head                                 = false;
      bool obb_body                                 = false;
      bool obb_feet                                 = false;

      bool auto_release                             = true;

      bool  enable_uber_sharing                     = false;
      float uber_sharing_time                       = 2.5f;

      bool  splash_bot = false;
      float unused0001  = 8.f;

      bool double_tap_aim_point                   = false;
      bool double_tap_aim_point_pelvis            = true;
      bool double_tap_aim_point_stomach           = false;

      float splash_blast_radius_size              = 100.f;

      bool aimbot_only_enabled     = false;
      bool aimbot_only_use_key     = false;
      u8   aimbot_only_key         = VK_XBUTTON2;
      bool aimbot_only_prioritized = false;
      bool aimbot_only_cheaters    = false;
      bool aimbot_only_bots        = false;

      bool auto_reload_beggars        = false;
      bool predict_server_random_seed = false;
      bool manipulation_disabled      = false;

      bool max_targets_enabled        = false;
      bool max_targets_manual         = true;
      bool max_targets_auto           = false;
      i32  max_targets                = 1;

      bool disable_on_spectated       = false;

      float auto_heal_next_target_delay = 0.f;
      float splash_bot_max_points       = 120.f;
      bool splash_bot_fallback          = true;
      bool splash_bot_prefer            = false;

      bool loose_cannon_lethal          = false;
      bool ignore_cheaters              = false;

      float unused12                    = 0.75f;
      bool  __splash_reserved           = true; // do not remove or use.
      bool swing_prediction             = false;
      bool heal_arrow_priority_team     = false;
      
      bool heal_arrow_priority_team_inview = true;
      bool heal_arrow_priority_team_any    = false;

      bool splash_bot_use_key          = false;
      u8   splash_bot_key              = VK_XBUTTON2;

      bool splash_bot_key_held         = true;
      bool splash_bot_key_toggle       = false;
      bool splash_bot_key_double_click = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_triggerbot_settings{
public:
  union {
    struct{
      bool enabled        = false;
      bool use_key        = false;
      u8   key            = 0x4;

      bool key_held         = true;
      bool key_toggle       = false;
      bool key_double_click = false;

      i32  delay            = 200;
      i32  fire_for_time    = 100;


      bool hitbox_ignore_limbs = false;
      bool hitbox_ignore_body  = false;
      bool hitbox_ignore_head  = false;

      bool shoot_cloaked    = false;
      bool shoot_disguised  = true;
      bool shoot_bonked     = false;
      bool shoot_uber       = false;
      bool shoot_friends    = false;
      bool shoot_sentry     = false;
      bool shoot_dispenser  = false;
      bool shoot_teleporter = false;
      bool shoot_boss       = false;
      bool shoot_sticky     = false;

      bool ignore_player_class[10] = {false, false, false, false, false, false, false, false, false};

      bool show_trigger_bot_traces = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_player_esp_settings{
public:
  union{
    struct{
      bool enabled                   = true;
      bool render_cloaked_spies      = true;
      bool friend_tag                = true;

      bool no_box                    = true;
      bool box                       = false;
      bool box_outlines              = false;
      bool name                      = true;
      bool weapon_name               = true;
      bool distance                  = true;
      bool healthbar                 = true;
      i32  health_bar_fractions      = 0;

      bool ubercharge_bar            = true;

      bool cheater_bot_alert         = true;
      bool flags                     = true;
      bool flag_slow_down            = true;
      bool flag_zoomed               = true;
      bool flag_cloaked              = true;
      bool flag_bonked               = true;
      bool flag_disguised            = true;
      bool flag_ubercharged          = true;
      bool flag_ubercharged_hidden   = true;
      bool flag_taunting             = true;
      bool flag_crit_boosted         = true;
      bool flag_damage_buff          = true;
      bool flag_demo_charge          = true;
      bool flag_crit_cola            = true;
      bool flag_on_fire              = true;
      bool flag_jarated              = true;
      bool flag_bleeding             = true;
      bool flag_defense_buff         = true;
      bool flag_speed_heal_buff      = true;
      bool flag_mad_milked           = true;
      bool flag_quick_fix_uber       = true;
      bool flag_hitmans_buff         = true;
      bool flag_bullet_resist_charge = true;
      bool flag_bullet_resist        = true;
      bool flag_blast_resist_charge  = true;
      bool flag_blast_resist         = true;
      bool flag_fire_resist_charge   = true;
      bool flag_fire_resist          = true;
      bool flag_medigun_debuff       = true;
      bool flag_healing_debuff       = true;
      bool flag_show_healing         = true;

      bool dormant_enabled           = true;
      bool dormant_render_text       = false;

      bool flag_has_objective        = true;
      bool flag_sniper_charge        = true;
      bool flag_show_duels           = true;
      bool dormant_color_gray_out    = true;
      bool dormant_color_team        = false;
      bool dormant_color_none        = false;
      bool flag_aimbot_target        = true;

      bool health_text               = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_world_esp_settings{
public:
  union{
    struct{
      bool team_enabled               = true;
      bool object_enabled[9]          = {true, true, true, true, true, true, true, true, true};
      i32  health_bar_fractions       = 0;
      bool box[9]                     = {false, false, false, false, false, false, false, false, false};
      bool box_outlines[9]            = {true, true, true, true, true, true, true, true, true};
      bool name[9]                    = {true, true, true, true, true, true, true, true, true};
      bool flags[9]                   = {true, true, true, true, true, true, true, true, true};
      bool distance[9]                = {true, true, true, true, true, true, true, true, true};
      bool healthbar[9]               = {true, true, true, true, true, true, true, true, true};
      bool sentry_level               = true;
      bool sentry_activestate         = true;
      bool sentry_controlled          = true;
      bool sentry_ammostatus          = true;
      bool sentry_buildingstate       = true;

      bool dispenser_level            = true;
      bool dispenser_activestate      = true;
      bool dispenser_buildingstate    = true;

      bool teleporter_level           = true;
      bool teleporter_activestate     = true;
      bool teleporter_buildingstate   = true;

      bool objective_returntime       = true;
      bool objective_status           = true;

      bool projectile_rocket          = true;
      bool projectile_sticky          = true;
      bool projectile_pipebomb        = true;
      bool projectile_arrow           = true;
      bool projectile_flare           = true;
      bool projectile_throwables      = true;
      bool projectile_spells          = true;
      bool projectile_flag_criticals  = true;

      bool ent_bombs                  = true;
      bool ent_lunchables             = true;
      bool ent_boss                   = true;
      bool ent_powerup                = true;
      bool ent_spellbooks             = true;
      bool ent_money                  = true;
      bool ent_revive_marker          = true;

      bool dormant_enabled[9]         = {true, true, true, true, true, true, true, true, true};
      bool dormant_render_text[9]     = {false, false, false, false, false, false, false, false, false};
      bool ent_halloween_pickup       = true;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_player_chams_settings{
public:
  union{
    struct{
      bool enabled                   = true;
      bool render_cloaked_spies      = true;

      bool ignore_z                  = true;
      bool base_material_none        = false;
      bool base_material_shaded      = false;
      bool base_material_flat        = true;

      bool overlay_material_none     = false;
      bool overlay_material_fresnel  = false;

      bool overlay_material_flat      = false;
      bool overlay_material_wireframe = false;
      bool base_material_selfillum    = false;
      bool base_material_frensel      = false;
      bool overlay_material_selfillum = false;
      bool overlay_material_shaded    = false;
      bool base_material_wireframe    = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_world_chams_settings{
public:
  union{
    struct{
      bool team_enabled = true;

      bool object_enabled[9]          = {true, true, true, true, true, true, true, true, true};
      bool ignore_z                   = true;
      bool base_material_none         = false;
      bool base_material_shaded       = false;
      bool base_material_flat         = true;
      float base_alpha                = 15.f;

      bool overlay_material_none      = false;
      bool overlay_material_fresnel   = false;
      float overlay_alpha             = 15.f;

      bool ent_spellbooks             = true;
      bool ent_lunchables             = true;
      bool ent_powerup                = true;
      bool ent_boss                   = true;
      bool ent_bombs                  = true;
      bool ent_money                  = true;
      bool ent_revive_marker          = true;
      bool ent_projectile_arrow       = true;
      bool ent_projectile_throwables  = true;
      bool ent_projectile_sticky      = true;
      bool ent_projectile_pipebomb    = true;
      bool ent_projectile_rocket      = true;
      bool ent_projectile_flare       = true;
      bool ent_projectile_spells      = true;

      bool overlay_material_flat      = false;
      bool overlay_material_wireframe = false;
      bool base_material_selfillum    = false;
      bool base_material_frensel      = false;
      bool overlay_material_selfillum = false;
      bool overlay_material_shaded    = false;
      bool base_material_wireframe    = false;

      bool ent_halloween_pickup       = true;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_visual_settings{
public:
  union{
    struct{
      bool cheap_text_rendering                 = false;
      bool render_medic_mode                    = false;
      bool render_engineer_mode                 = false;

      bool player_esp_use_team_colours            = true;
      bool player_esp_use_enemyteam_colours       = false;

      bool player_esp_friends_use_single_colour   = true;
      bool player_esp_friends_use_mult_colour     = false;

      bool world_esp_use_team_colours             = true;
      bool world_esp_use_enemyteam_colours        = false;

      bool player_chams_use_team_colours          = true;
      bool player_chams_use_enemyteam_colours     = false;

      bool player_chams_friends_use_single_colour = true;
      bool player_chams_friends_use_mult_colour   = false;

      bool world_chams_use_team_colours           = true;
      bool world_chams_use_enemyteam_colours      = false;

      bool player_glow_use_team_colours           = true;
      bool player_glow_use_enemyteam_colours      = false;

      bool player_glow_friends_use_single_colour  = true;
      bool player_glow_friends_use_mult_colour    = false;

      bool world_glow_use_team_colours            = true;
      bool world_glow_use_enemyteam_colours       = false;

      bool bullet_tracers_enabled               = false;
      bool tracer_beam_effect                   = true;
      bool tracer_raygun_effect                 = false;
      bool tracer_machina_effect                = false;
      bool tracer_lightning_effect              = false;
      bool tracer_meramus_effect                = false;
      bool tracer_distortion_effect             = false;

      bool disable_overlay_effects_enabled      = false;
      bool disable_overlay_invuln               = false;
      bool disable_overlay_milk                 = false;
      bool disable_overlay_jarate               = false;
      bool disable_overlay_bleed                = false;
      bool disable_overlay_stealth              = false;
      bool disable_overlay_bonked               = false;
      bool disable_overlay_gas                  = false;
      bool disable_overlay_onfire               = false;

      i32  custom_fov_slider                    = 90;
      bool no_zoom_enabled                      = false;
      bool no_zoom_fov                          = false;

      CONFIG_RGB(menu_foreground_colour, 25, 118, 210);
      CONFIG_RGB(menu_background_colour, 41, 44, 51);

      bool viewmodel_offset                     = false;
      i32 viewmodel_offset_x                    = 0;
      i32 viewmodel_offset_y                    = 0;
      i32 viewmodel_offset_z                    = 0;
      bool viewmodel_silentaim                  = false;

      // ragdoll effects
      bool ragdoll_effects                              = false;
      bool ragdoll_effects_show_friendly                = false;
      bool ragdoll_effects_show_enemy                   = true;
      bool ragdoll_effects_show_all                     = false;
      bool ragdoll_effects_disable_death_animations     = false;
      bool ragdoll_effects_disable_game_spawned_effects = false;
      bool ragdoll_effect_strong_burning                = false;
      bool ragdoll_effect_strong_electrocuted           = false;
      bool ragdoll_effect_weak_dissolve                 = false;
      bool ragdoll_effect_weak_golden                   = true;
      bool ragdoll_effect_weak_frozen                   = false;
      bool ragdoll_effect_weak_ash                      = false;
      bool ragdoll_effect_model_none                    = false;
      bool ragdoll_effect_particle_none                 = false;

      bool no_visual_recoil       = true;
      bool no_viewmodel_bob       = false;

      bool unused0                  = true;
      bool unused1                  = false;
      bool unused2                  = false;
      bool unused3                  = false;
      bool unused4                  = false;

      // Glow player settings
      bool player_glow_enabled                = true;
      bool player_glow_render_friendlies      = true;
      bool player_glow_render_enemies         = true;
      bool player_glow_render_cloaked_spies   = true;
      bool player_glow_render_steamfriends    = true;

      bool world_glow_enabled       = true;
      bool glow_object_enabled[9]   = {true, true, true, true, true, true, true, true, true};

      bool world_glow_render_friendlies          = true;
      bool world_glow_render_enemies             = true;

      bool world_glow_ent_spellbooks              = true;
      bool world_glow_ent_lunchables              = true;
      bool world_glow_ent_powerup                 = true;
      bool world_glow_ent_boss                    = true;
      bool world_glow_ent_bombs                   = true;
      bool world_glow_ent_money                   = true;
      bool world_glow_ent_revive_marker           = true;

      bool world_glow_ent_projectile_arrow        = true;
      bool world_glow_ent_projectile_throwables   = true;
      bool world_glow_ent_projectile_sticky       = true;
      bool world_glow_ent_projectile_pipebomb     = true;
      bool world_glow_ent_projectile_rocket       = true;
      bool world_glow_ent_projectile_flare        = true;
      bool world_glow_ent_projectile_spells       = true;


      // Glow settings
      bool glow_stencil       = true;
      bool glow_blur          = false;
      i32  glow_thickness     = 1;

      bool show_hitboxes_on_hit                  = false;

      CONFIG_RGBA(show_hitboxes_on_hit_colour, 255, 255, 255, 0);

      // Player ESP
      CONFIG_RGB(player_esp_red_team_colour, 255, 74, 74);
      CONFIG_RGB(player_esp_blu_team_colour, 124, 144, 255);
      CONFIG_RGB(player_esp_friend_colour, 96, 255, 100);

      CONFIG_RGB(player_esp_friend_red_team_colour, 255, 55, 25);
      CONFIG_RGB(player_esp_friend_blu_team_colour, 0, 220, 255);
      CONFIG_RGB(player_esp_friend_enemy_colour, 255, 150, 0);
      CONFIG_RGB(player_esp_friend_team_colour, 0, 255, 125);

      CONFIG_RGB(player_esp_enemy_colour, 255, 125, 30);
      CONFIG_RGB(player_esp_team_colour, 0, 255, 70);

      // World ESP
      CONFIG_RGB(world_esp_red_team_colour, 255, 74, 74);
      CONFIG_RGB(world_esp_blu_team_colour, 124, 144, 255);
      CONFIG_RGB(world_esp_unassigned_team_colour, 255, 255, 255);
      CONFIG_RGB(world_esp_enemy_colour, 255, 125, 30);
      CONFIG_RGB(world_esp_team_colour, 0, 255, 70);
      CONFIG_RGB(world_esp_health_pack_colour, 0, 255, 100);
      CONFIG_RGB(world_esp_ammo_pack_colour, 188, 188, 200);
      CONFIG_RGB(world_esp_boss_colour, 148, 50, 200);
      CONFIG_RGB(world_esp_bomb_colour, 235, 20, 0);
      CONFIG_RGB(world_esp_money_colour, 80, 255, 0);
      CONFIG_RGB(world_esp_lunchables_colour, 0, 255, 100);
      CONFIG_RGB(world_esp_powerup_colour, 255, 255, 0);
      CONFIG_RGB(world_esp_spellbook_normal_colour, 100, 255, 100);
      CONFIG_RGB(world_esp_spellbook_rare_colour, 140, 50, 180);

      // Player chams
      CONFIG_RGBA(player_chams_red_team_colour, 204, 80, 80, 127);
      CONFIG_RGBA(player_chams_blu_team_colour, 119, 148, 212, 127);
      CONFIG_RGBA(player_chams_friend_colour, 96, 255, 100, 127);

      CONFIG_RGBA(player_chams_friend_red_team_colour, 255, 55, 25, 127);
      CONFIG_RGBA(player_chams_friend_blu_team_colour, 0, 220, 255, 127);
      CONFIG_RGBA(player_chams_friend_enemy_colour, 255, 150, 0, 127);
      CONFIG_RGBA(player_chams_friend_team_colour, 0, 255, 125, 127);

      CONFIG_RGBA(player_chams_enemy_colour, 255, 125, 30, 127);
      CONFIG_RGBA(player_chams_team_colour, 0, 255, 70, 127);

      // World chams
      CONFIG_RGBA(world_chams_red_team_colour, 204, 80, 80, 127);
      CONFIG_RGBA(world_chams_blu_team_colour, 119, 148, 212, 127);
      CONFIG_RGBA(world_chams_enemy_colour, 255, 125, 30, 127);
      CONFIG_RGBA(world_chams_team_colour, 0, 255, 70, 127);
      CONFIG_RGBA(world_chams_unassigned_team_colour, 255, 255, 255, 127);
      CONFIG_RGBA(world_chams_health_pack_colour, 0, 255, 100, 127);
      CONFIG_RGBA(world_chams_ammo_pack_colour, 188, 188, 200, 127);
      CONFIG_RGBA(world_chams_boss_colour, 148, 50, 200, 127);
      CONFIG_RGBA(world_chams_bomb_colour, 235, 20, 0, 127);
      CONFIG_RGBA(world_chams_money_colour, 80, 255, 0, 127);
      CONFIG_RGBA(world_chams_lunchables_colour, 0, 255, 100, 127);
      CONFIG_RGBA(world_chams_powerup_colour, 255, 255, 0, 127);
      CONFIG_RGBA(world_chams_spellbook_normal_colour, 100, 255, 100, 127);
      CONFIG_RGBA(world_chams_spellbook_rare_colour, 140, 50, 180, 127);

      // Player glow
      CONFIG_RGB(player_glow_red_team_colour, 232, 60, 60);
      CONFIG_RGB(player_glow_blu_team_colour, 75, 180, 232);
      CONFIG_RGB(player_glow_friend_colour, 100, 255, 125);

      CONFIG_RGB(player_glow_friend_red_team_colour, 255, 55, 25);
      CONFIG_RGB(player_glow_friend_blu_team_colour, 0, 220, 255);
      CONFIG_RGB(player_glow_friend_enemy_colour, 255, 150, 0);
      CONFIG_RGB(player_glow_friend_team_colour, 0, 255, 125);

      CONFIG_RGB(player_glow_enemy_colour, 255, 160, 60);
      CONFIG_RGB(player_glow_team_colour, 20, 255, 90);

      // World glow
      CONFIG_RGB(world_glow_red_team_colour, 232, 60, 60);
      CONFIG_RGB(world_glow_blu_team_colour, 75, 180, 232);
      CONFIG_RGB(world_glow_enemy_colour, 255, 160, 60);
      CONFIG_RGB(world_glow_team_colour, 20, 255, 90);
      CONFIG_RGB(world_glow_unassigned_team_colour, 255, 255, 255);
      CONFIG_RGB(world_glow_health_pack_colour, 0, 255, 100);
      CONFIG_RGB(world_glow_ammo_pack_colour, 188, 188, 200);
      CONFIG_RGB(world_glow_boss_colour, 148, 50, 200);
      CONFIG_RGB(world_glow_bomb_colour, 235, 20, 0);
      CONFIG_RGB(world_glow_money_colour, 80, 255, 0);
      CONFIG_RGB(world_glow_lunchables_colour, 0, 255, 100);
      CONFIG_RGB(world_glow_powerup_colour, 255, 255, 0);
      CONFIG_RGB(world_glow_spellbook_normal_colour, 100, 255, 100);
      CONFIG_RGB(world_glow_spellbook_rare_colour, 140, 50, 180);

      CONFIG_RGB(bullet_beam_tracer_colour, 66, 173, 245);

      CONFIG_RGBA(player_chams_overlay_red_team_colour, 212, 103, 103, 85);
      CONFIG_RGBA(player_chams_overlay_blu_team_colour, 121, 170, 255, 85);
      CONFIG_RGBA(player_chams_overlay_enemy_colour, 255, 160, 60, 85);
      CONFIG_RGBA(player_chams_overlay_team_colour, 20, 255, 90, 85);

      CONFIG_RGBA(player_chams_overlay_friend_colour, 100, 255, 125, 85);
      CONFIG_RGBA(player_chams_overlay_friend_enemy_colour, 255, 150, 0, 85);
      CONFIG_RGBA(player_chams_overlay_friend_team_colour, 0, 255, 125, 85);
      CONFIG_RGBA(player_chams_overlay_friend_red_team_colour, 255, 55, 25, 85);
      CONFIG_RGBA(player_chams_overlay_friend_blu_team_colour, 0, 220, 255, 85);

      CONFIG_RGBA(world_chams_overlay_red_team_colour, 212, 103, 103, 85);
      CONFIG_RGBA(world_chams_overlay_blu_team_colour, 121, 170, 255, 85);
      CONFIG_RGBA(world_chams_overlay_enemy_colour, 255, 125, 30, 85);
      CONFIG_RGBA(world_chams_overlay_team_colour, 0, 255, 70, 85);
      CONFIG_RGBA(world_chams_overlay_unassigned_team_colour, 255, 255, 255, 85);
      CONFIG_RGBA(world_chams_overlay_health_pack_colour, 0, 255, 100, 85);
      CONFIG_RGBA(world_chams_overlay_ammo_pack_colour, 188, 188, 200, 85);
      CONFIG_RGBA(world_chams_overlay_boss_colour, 148, 50, 200, 85);
      CONFIG_RGBA(world_chams_overlay_bomb_colour, 235, 20, 0, 85);
      CONFIG_RGBA(world_chams_overlay_money_colour, 80, 255, 0, 85);
      CONFIG_RGBA(world_chams_overlay_lunchables_colour, 0, 255, 100, 85);
      CONFIG_RGBA(world_chams_overlay_powerup_colour, 255, 255, 0, 85);
      CONFIG_RGBA(world_chams_overlay_spellbook_normal_colour, 100, 255, 100, 85);
      CONFIG_RGBA(world_chams_overlay_spellbook_rare_colour, 140, 50, 180, 85);

      bool player_esp_aimbot_target           = false;
      bool world_esp_aimbot_target            = false;

      bool player_chams_aimbot_target         = false;
      bool world_chams_aimbot_target          = false;

      bool player_chams_overlay_aimbot_target = false;
      bool world_chams_overlay_aimbot_target  = false;

      bool player_glow_aimbot_target          = true;
      bool world_glow_aimbot_target           = true;

      CONFIG_RGB(player_esp_aimbot_target_colour, 10, 255, 10);
      CONFIG_RGB(world_esp_aimbot_target_colour, 10, 255, 10);

      CONFIG_RGB(player_chams_aimbot_target_colour, 32, 255, 32);
      CONFIG_RGB(world_chams_aimbot_target_colour, 32, 255, 32);

      CONFIG_RGB(player_chams_overlay_aimbot_target_colour, 32, 255, 32);
      CONFIG_RGB(world_chams_overlay_aimbot_target_colour, 32, 255, 32);

      CONFIG_RGB(player_glow_aimbot_target_colour, 64, 255, 64);
      CONFIG_RGB(world_glow_aimbot_target_colour, 64, 255, 64);

      bool draw_crosshair_at_aim_point        = false;
      bool tracer_bignasty_effect             = false;

      bool show_spectators       = true;
      bool show_information      = true;
      bool use_steam_nicknames   = false;

      bool show_icons_thru_walls             = false;
      bool show_vaccinator_icons_thru_walls  = false;
      bool show_mannpower_icons_thru_walls   = false;
      bool show_both_team_icons              = true;
      bool show_enemy_team_icons             = false;
      bool show_friendly_team_icons          = false;

      bool sniper_los_enabled                = false;
      bool sniper_los_friendly_only          = false;
      bool sniper_los_enemy_only             = false;
      bool sniper_los_both                   = true;
      bool sniper_los_ignore_z               = false;

      bool no_fog                            = false;
      bool control_farz                      = false;
      i32  farz_value                        = 0;

      bool disable_overlay_water             = false;
      i32  tracer_beam_effect_life_time      = 15;

      bool player_esp_include_localplayer    = false;
      bool player_chams_include_localplayer  = true;
      bool player_glow_include_localplayer   = true;

      bool combat_text_enabled               = false;
      bool combat_text_force_show_spies      = true;
      bool combat_text_show_thru_walls       = true;
      bool combat_text_show_team_mate_dmg    = true;
      bool combat_text_show_dmg_friends_only = false;

      bool projectile_prediction_show_path  = true;
      CONFIG_RGB(projectile_prediction_show_path_col, 255, 255, 255);

      i32 viewmodel_angle_pitch  = 0;
      i32 viewmodel_angle_yaw    = 0;
      i32 viewmodel_angle_roll   = 0;

      bool tracer_lines         = false;
      i32 tracer_line_life_time = 15;
      CONFIG_RGBA(tracer_line_colour, 255, 255, 255, 32);

      bool projectile_trajectory_visualizer       = false;
      bool projectile_trajectory_constant_line    = false;
      bool projectile_trajectory_constant_outline = false;
      CONFIG_RGBA(projectile_trajectory_constant_line_colour, 255, 255, 255, 32);

      bool projectile_trajectory_dynamic_line    = false;
      bool projectile_trajectory_dynamic_outline = false;
      CONFIG_RGBA(projectile_trajectory_dynamic_line_colour, 255, 255, 255, 32);

      bool tracer_line_outline                   = false;
      bool projectile_trajectory_impact_camera   = false;

      CONFIG_RGB(double_tap_main_colour, 60, 64, 198);
      CONFIG_RGB(double_tap_sheen_colour, 15, 188, 249);

      bool  enable_exposure                      = false;
      float exposure_min                         = 0.f;
      float exposure_max                         = 1.f;

      bool  enable_bloom                         = false;
      float bloom_amount                         = 0.f;

      bool projectile_trajectory_show_random_impulse   = false;
      bool projectile_trajectory_show_random_velocity  = false;
      bool bullet_tracers_include_sentry               = true;
      bool projectile_trajectory_constant_box          = false;
      bool projectile_trajectory_dynamic_box           = false;

      bool  unused93                        = false;
      float unused931                       = 5.f;

      bool hand_chams_enabled = false;
      bool hand_chams_removal = false;

      bool hand_chams_base_material_none        = false;
      bool hand_chams_base_material_shaded      = true;
      bool hand_chams_base_material_flat        = false;
      bool hand_chams_base_material_selfillum   = false;
      bool hand_chams_base_material_fresnel     = false;
      bool hand_chams_base_material_wireframe   = false;

      bool hand_chams_overlay_material_none      = false;
      bool hand_chams_overlay_material_shaded    = false;
      bool hand_chams_overlay_material_flat      = false;
      bool hand_chams_overlay_material_selfillum = false;
      bool hand_chams_overlay_material_fresnel   = true;
      bool hand_chams_overlay_material_wireframe = false;

      CONFIG_RGBA(hand_chams_base_material_colour, 0, 0, 0, 255);
      CONFIG_RGBA(hand_chams_overlay_material_colour, 190, 255, 200, 255);

      bool weapon_chams_enabled = false;

      bool weapon_chams_base_material_none        = false;
      bool weapon_chams_base_material_shaded      = true;
      bool weapon_chams_base_material_flat        = false;
      bool weapon_chams_base_material_selfillum   = false;
      bool weapon_chams_base_material_fresnel     = false;
      bool weapon_chams_base_material_wireframe   = false;

      bool weapon_chams_overlay_material_none      = false;
      bool weapon_chams_overlay_material_shaded    = false;
      bool weapon_chams_overlay_material_flat      = false;
      bool weapon_chams_overlay_material_selfillum = false;
      bool weapon_chams_overlay_material_fresnel   = true;
      bool weapon_chams_overlay_material_wireframe = false;

      CONFIG_RGBA(weapon_chams_base_material_colour, 0, 0, 0, 255);
      CONFIG_RGBA(weapon_chams_overlay_material_colour, 170, 190, 230, 255);

      bool  static_prop_enabled = false;
      float static_prop_alpha = 100.f;

      bool  door_transparency_enabled = false;
      float door_transparency_value = 100.f;

      bool world_glow_ent_halloween_pickup = true;

      bool use_alt_vaccinator_shields      = false;

      float glow_blur_bloom_value = 5.f;

      bool unused0001    = false;
      i32  unused0002    = 5;
      u8   unused0003    = VK_XBUTTON2;
      bool unused0004    = true;
      bool unused0005    = false;
      bool unused0006    = false;
      bool show_spectators_localplayer = false;

      bool health_glow_enabled    = false;
      bool health_glow_players    = true;
      bool health_glow_sentries   = true;
      bool health_glow_dispenser  = true;
      bool health_glow_teleporter = true;

      bool health_glow_show_friendly    = true;
      bool health_glow_show_localplayer = true;
      bool health_glow_show_enemy       = true;
      bool health_glow_show_friends     = true;

      bool projectile_prediction_show_path_outline = true;
      bool projectile_prediction_show_path_ticks   = false;

      bool show_estimated_aim_target     = false;
      bool estimated_aim_target_circle   = true;
      bool estimated_aim_target_triangle = false;
      bool estimated_aim_target_arrow    = false;
      i32  estimated_aim_target_size     = 12;

      CONFIG_RGBA(estimated_aim_target_colour, 255, 255, 0, 255);

      bool projectile_trajectory_constant_splash_circle = false;
      bool projectile_trajectory_dynamic_splash_circle  = false;

      CONFIG_RGBA(backtrack_chams_colour, 255, 255, 255, 45);
      CONFIG_RGBA(backtrack_chams_overlay_colour, 255, 255, 255, 45);
      bool backtrack_glow_enabled = true;
      CONFIG_RGBA(backtrack_glow_colour, 255, 255, 255, 180);
      bool backtrack_glow_render_cloaked_spies = true;

      CONFIG_RGBA(fake_angle_chams_colour, 255, 64, 0, 45);
      CONFIG_RGBA(fake_angle_chams_overlay_colour, 255, 64, 0, 45);
      bool fake_angle_glow_enabled = true;
      CONFIG_RGBA(fake_angle_glow_colour, 255, 32, 32, 180);

      bool  aimbot_fov_circle_enabled                   = true;
      bool  aimbot_fov_circle_outlines                  = true;
      CONFIG_RGBA(aimbot_fov_circle_colour, 255, 255, 255, 255);

      bool hide_sentry_with_wrangler_out                = false;
      bool override_view_with_sentry                    = false;
      bool override_view_with_sentry_use_key            = false;
      u8   override_view_with_sentry_key                = VK_XBUTTON2;
      bool override_view_with_sentry_key_held           = true;
      bool override_view_with_sentry_key_toggle         = false;
      bool override_view_with_sentry_key_double_click   = false;

    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_hvh_settings{
public:
  union{
    struct{
      bool break_animations_disabled = true;
      bool break_animations_constant = false;
      bool break_animations_on_jump = false;

      bool fakelag                 = false;
      i32  fakelag_maxchoke        = 12;
      bool fakelag_method_interval = false;
      bool fakelag_method_movement = true;
      u8   fakelag_key             = VK_MENU;
      bool fakelag_usekey          = false;
      bool fakelag_key_held        = true;
      bool fakelag_key_toggle      = false;
      bool fakelag_anti_backtrack  = false;

      bool antiaim                              = false;
      bool antiaim_pitch                        = false;
      bool antiaim_yaw                          = true;
      bool antiaim_headshotdanger               = true;

      bool pitch_fake_none                      = false;
      bool pitch_fake_up                        = true;
      bool pitch_fake_down                      = false;

      bool pitch_real_up                        = true;
      bool pitch_real_zero                      = false;
      bool pitch_real_down                      = false;

      bool yaw_method_real_rotate               = true;
      bool yaw_method_real_spin                 = false;
      bool yaw_method_real_rotate_dynamic       = false;

      bool yaw_method_fake_rotate               = true;
      bool yaw_method_fake_spin                 = false;
      bool yaw_method_fake_rotate_dynamic       = false;

      float rotate_fake_ang                      = 90.f;
      float rotate_fake_ang2                     = -90.f;
      float rotate_real_ang                      = -90.f;
      float rotate_real_ang2                     = 90.f;

      i32  spin_fake_speed                      = 12;
      i32  spin_real_speed                      = 12;
      bool spin_fake_invert                     = false;
      bool spin_real_invert                     = false;

      float  yaw_rotate_dynamic_update_rate = 750.f;

      bool resolver                             = true;
      bool resolver_esp_status                  = true;
      bool resolver_cheatersonly                = true;

      bool yaw_rotate_real_target_method_view         = false;
      bool yaw_rotate_real_target_method_closetargets = false;
      bool yaw_rotate_real_target_method_snipers      = true;

      bool yaw_rotate_fake_target_method_view         = false;
      bool yaw_rotate_fake_target_method_closetargets = false;
      bool yaw_rotate_fake_target_method_snipers      = true;

      u8   antiaim_key        = 0x54;
      bool antiaim_usekey     = false;
      bool antiaim_key_held   = true;
      bool antiaim_key_toggle = false;

      bool antiaim_cycle_enabled         = false;
      u8   antiaim_cycle_key             = 0x6;
      bool antiaim_cycle_swap_settings   = true;

      bool antiaim_cycle_invert_fake_angle  = false;
      bool antiaim_cycle_invert_real_angle  = false;
      bool antiaim_cycle_invert_both_angles = false;
      bool antiaim_pitch_disabled           = false;
      bool antiaim_pitch_auto               = true;

      bool peek_assist                      = false;
      u8   peek_assist_hotkey               = VK_MENU;

      bool resolver_headshot_weponly        = true;

      bool antiaim_legit                    = false;
      bool antiaim_rage                     = true;

      bool yaw_auto_edge_enabled            = false;
      bool yaw_auto_edge_fake               = false;
      bool yaw_auto_edge_real               = true;

      bool yaw_auto_edge_target_method_view         = true;
      bool yaw_auto_edge_target_method_closetargets = false;
      bool yaw_auto_edge_target_method_snipers      = false;

      bool fakelag_peek                        = false;

      bool fakeduck_disabled                   = true;
      bool fakeduck_onduck                     = false;
      bool fakeduck_always                     = false;

      bool antiaim_headshotdanger_any          = true;
      bool antiaim_headshotdanger_cheatersonly = false;
      bool antiaim_headshotdanger_vischeck     = false;

      bool antiaim_pitch_static                = true;
      bool antiaim_pitch_dynamic               = false;

      bool antiaim_pitch_dynamic_real_down     = false;
      bool antiaim_pitch_dynamic_real_up       = false;

      bool fakelag_adaptive                    = false;
      bool fakelag_on_sequence_changed         = false;
      bool fakeduck_on_key                     = false;
      u8   fakeduck_key                        = 0x11;
      bool auto_stop                           = false;
      bool fakelag_headshotdanger              = true;
      bool fakelag_headshotdanger_any          = false;
      bool fakelag_headshotdanger_cheatersonly = true;

      bool antiaim_key_double_click             = false;
      bool fakelag_key_double_click             = false;
      bool fakelag_headshotdanger_vischeck     = false;

      bool resolver_allow_manual_cycle         = false;
      u8   resolver_mc_sp_key                  = VK_SHIFT;
      u8   resolver_mc_up_key                  = VK_TAB;

      bool resolver_mc_sp_single_click         = false;
      bool resolver_mc_sp_double_click         = true;

      bool resolver_mc_up_single_click         = true;
      bool resolver_mc_up_double_click         = false;

    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_misc_settings{
public:
  union{
    struct{
      // no idea if this is how we should do settings.. but I can't put it in the union...
      bool pure_bypass                        = true;
      bool reveal_scoreboard                  = false;
      bool remove_taunt_restrictions          = false;
      bool reveal_spy_models                  = false;

      bool unused02                       = true;
      i32  unused_max_ping                = 200;
      bool unused_block_eu                = false;
      bool unused_block_asia              = false;
      bool unused_block_au                = false;
      bool unused_block_southus           = false;
      bool unused_block_northus_east      = false;
      bool unused_block_northus_west      = false;
      bool unused_block_africa            = false;

      bool auto_hop                           = false;
      bool auto_strafe                        = false;
      bool crithack_enabled                   = false;
      bool crithack_draw_panel                = true;
      bool crithack_avoid_natural             = true;
      bool crithack_melee_auto_crit           = true;
      u8   crithack_hotkey                    = VK_SHIFT;
      bool double_tap_enabled                 = false;
      u8   double_tap_charge_hotkey           = VK_CAPITAL;
      u8   double_tap_shift_hotkey            = VK_CAPITAL;
      bool double_tap_on_ground_only          = false;
      bool double_tap_draw_panel              = true;
      bool double_tap_shift_on_key            = false;
      bool nopush                             = true;
      bool remove_cosmetics                   = false;
      bool disable_gibs                       = true;
      bool disable_footsteps                  = false;

      bool auto_vote_enabled                  = false;
      bool auto_vote_castyes                  = true;
      bool auto_vote_castno                   = true;

      bool auto_vote_autocast_disabled        = true;
      bool auto_vote_autocast_random          = false;
      bool auto_vote_autocast_botsonly        = false;

      bool no_idle_kick                       = true;
      bool auto_retry_teambalance             = true;
      bool auto_mute_bots                     = true;
      bool auto_rev_jump                      = false;
      bool anti_backstab                      = false;
      bool remove_cosmetics_localplayer       = false;
      bool remove_cosmetics_friends           = false;

      bool infinite_noisemaker                = false;
      bool mvm_revive_none                    = false;
      bool mvm_revive_onmedicheal             = true;
      bool mvm_revive_always                  = false;

      bool block_unused1291                   = false;

      bool double_tap_on_aimbot_only          = false;

      bool unused_block_south_west            = false;
      bool unused_block_peru                  = false;
      bool unused_block_brazil                = false;
      bool unused_block_japan                 = false;
      bool unused_block_hongkong              = false;
      bool unused_block_eu_east               = false;
      bool unused_block_middle_east           = false;
      bool unused_block_india                 = false;

      bool auto_record_hvh_matches            = false;
      bool block_noise_maker_sounds           = true;
      bool block_frying_pan_sounds            = false;


      bool remove_cage_restrictions               = false;
      bool remove_kart_restrictions               = false;
      bool auto_vote_autocast_cheatersonly        = false;

      bool party_network                          = false;
      bool party_network_far_esp                  = true;
      bool party_network_resolver                 = true;
      bool party_network_waypoint_marker          = false;
      u8   place_waypoint_key                     = 0x5A;
      float waypoint_timeout                      = 10.f;

      bool double_tap_passive_charging            = false;
      float double_tap_passive_charge_every_ms    = 500.f;

      bool unk00;
      bool unk02;
      bool unk0232;
      bool unk03;

      bool auto_vote_autocast_defensive           = false;

      bool  fake_latency_enabled                  = false;
      float fake_latency_target_ms                = 200.f;

      bool double_tap_boost                       = false;
      u8   double_tap_boost_key                   = VK_SHIFT;

      bool disable_taunts                         = false;

      bool fake_latency_use_key                   = false;
      u8   fake_latency_key                       = VK_XBUTTON2;

      bool infinite_noisemaker_on_kill            = false;
      bool disable_backpack_expander              = false;
      bool no_recoil                              = true;

      bool demosystem_format_ds_dir               = false;
      bool demosystem_hide_angles                 = false;

      bool fake_latency_key_held                  = true;
      bool fake_latency_key_toggle                = false;
      bool fake_latency_key_double_click          = false;

      bool broadcast_my_party                            = false;
      bool reveal_spy_models_include_localplayer         = false;
      bool no_idle_kick_auto_pick_class                  = false;
      bool mvm_giant_weapon_sounds                       = false;
      bool unused1                                       = false;
      bool auto_strafe_prespeed                          = true;
      bool anti_backstab_ignore_friends                  = false;
      bool reveal_spy_models_remove_cloak                = true;
      bool reveal_spy_models_remove_clock_always         = true;
      bool reveal_spy_models_remove_clock_ignore_flicker = false;

      bool unused302                                     = true;
      bool cheater_detection_disable_known_actor_alias   = false;
      bool cheater_detection_censor_bot_names            = true;
      bool unequip_regions                               = false;

      bool no_idle_kick_auto_pick_class_type[10] = {true, false, false, false, false, false, false, false, false, false};

      bool fake_latency_always_on                        = false;

      bool mvm_revive_use_key = false;
      u8   mvm_revive_key     = VK_SHIFT;

      bool fake_latency_spoof_ms = false;
      bool force_no_interpolation = false;

      u32  double_tap_boost_ticks = 1;
      bool double_tap_always_charge_max_ticks = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_automation_settings{
public:
  union{
    struct{
      bool auto_vaccinator                        = true;
      bool auto_vaccinator_fullcontrol            = true;
      bool auto_vaccinator_passive                = false;
      i32  auto_vaccinator_sensitivity_bullet     = 0;
      i32  auto_vaccinator_sensitivity_blast      = 0;
      i32  auto_vaccinator_sensitivity_fire       = 0;
      bool auto_vaccinator_disallow_resist_bullet = false;
      bool auto_vaccinator_disallow_resist_blast  = false;
      bool auto_vaccinator_disallow_resist_fire   = false;
      bool medic_uber_active_charge               = true;
      bool medic_auto_uber                        = true;

      bool auto_sticky_detonate                   = true;
      bool auto_sticky_target_friend              = false;
      bool auto_sticky_target_cloaked             = true;
      bool auto_sticky_target_sentry              = true;
      bool auto_sticky_target_dispenser           = false;
      bool auto_sticky_target_teleporter          = false;
      bool auto_sticky_target_sticky              = true;
      bool auto_sticky_target_boss                = false;

      bool auto_reflect                           = false;
      bool auto_reflect_disable_dragonsfury       = false;
      bool auto_reflect_disable_ubered            = false;

      i32  auto_reflect_ammo_required             = 0;
      bool auto_reflect_burning_players           = false;
      bool auto_reflect_burning_players_ifhurt    = false;
      bool auto_reflect_enemy_players             = false;
      bool auto_reflect_redirect_towards_players  = true;

      bool auto_disguise                          = true;
      bool removeme1                              = false;
      bool auto_vaccinator_passive_resist_none    = true;
      bool auto_vaccinator_passive_resist_bullet  = false;
      bool auto_vaccinator_passive_resist_blast   = false;
      bool auto_vaccinator_passive_resist_fire    = false;

      bool auto_reflect_enemy_players_meleeonly   = false;
      bool auto_reflect_enemy_players_none        = false;
      bool auto_reflect_burning_players_none      = false;

      bool auto_vaccinator_friendsonly            = false;
      bool auto_uber_friendsonly                  = false;
      bool medic_uber_active_charge_friendsonly   = false;

      bool demo_charge_bot                        = false;
      bool medic_follow_demo_charge               = false;

      bool auto_rocket_jumper                     = false;
      u8   auto_rocket_jumper_key                 = 0x45;

      bool trigger_bot                            = false;
      bool trigger_bot_use_key                    = false;
      u8   trigger_bot_key                        = 0x4;

      i32  trigger_bot_delay                      = 200;
      i32  trigger_bot_fire_for_time              = 100;

      bool trigger_bot_hitbox_auto                = true;
      bool trigger_bot_hitbox_head                = false;
      bool trigger_bot_hitbox_body                = false;
      bool trigger_bot_hitbox_any                 = false;

      bool trigger_bot_shoot_cloaked    = false;
      bool trigger_bot_shoot_friends    = false;
      bool trigger_bot_shoot_sentry     = false;
      bool trigger_bot_shoot_dispenser  = false;
      bool trigger_bot_shoot_teleporter = false;
      bool trigger_bot_shoot_boss       = false;
      bool trigger_bot_shoot_sticky     = false;

      bool auto_detonator            = true;
      bool auto_detonator_friends    = false;
      bool auto_detonator_cloaked    = true;
      bool auto_detonator_sentry     = false;
      bool auto_detonator_dispenser  = false;
      bool auto_detonator_teleporter = false;
      bool auto_detonator_boss       = false;
      bool auto_detonator_sticky     = false;

      bool auto_reflect_redirect_towards              = true;
      bool auto_reflect_redirect_towards_sentry       = true;
      bool auto_reflect_redirect_towards_dispenser    = true;
      bool auto_reflect_redirect_towards_teleporter   = true;
      bool auto_reflect_redirect_towards_projectiles  = true;
      bool auto_reflect_redirect_towards_boss         = false;
      bool auto_reflect_redirect_ignore_friends       = true;
      bool auto_reflect_redirect_ignore_cloaked       = true;
      bool auto_reflect_redirect_healbolt_teammates   = true;

      bool auto_reflect_target_rockets     = true;
      bool auto_reflect_target_flares      = true;
      bool auto_reflect_target_pills       = true;
      bool auto_reflect_target_stickies    = true;
      bool auto_reflect_target_arrows      = true;
      bool auto_reflect_target_throwables  = true;
      bool auto_reflect_target_flameball   = true;
      bool auto_reflect_target_mechorb     = true;

      bool auto_reflect_use_key            = false;
      u8   auto_reflect_key                = 0x4;

      bool auto_vaccinator_react_friends = false;
      bool auto_vaccinator_react_bonked  = false;
      bool auto_vaccinator_react_cloaked = false;

      bool backtrack_to_crosshair        = false;

      bool auto_sapper                   = true;
      bool auto_sapper_target_robots     = true;
      bool auto_sapper_target_sentry     = true;
      bool auto_sapper_target_dispenser  = true;
      bool auto_sapper_target_teleporter = true;
      i32  auto_sapper_min_robots        = 1;

      i32 auto_sticky_min_bomb_count     = 1;

      bool auto_vaccinator_use_key       = false;
      u8   auto_vaccinator_key           = 0x4;

      bool auto_vaccinator_manualcharge_resist_none   = false;
      bool auto_vaccinator_manualcharge_resist_bullet = true;
      bool auto_vaccinator_manualcharge_resist_blast  = false;
      bool auto_vaccinator_manualcharge_resist_fire   = false;
      bool medic_uber_active_charge_resist_bullet     = true;
      bool medic_uber_active_charge_resist_blast      = false;
      bool medic_uber_active_charge_resist_fire       = false;

      bool  auto_uber_react_friends                   = false;
      bool  auto_uber_react_bonked                    = false;
      bool  auto_uber_react_cloaked                   = false;
      float auto_uber_visible_enemy_threat_multiplier = 50.f;
      float auto_uber_nearby_enemy_threat_multiplier  = 50.f;

      bool medic_auto_kritz            = true;
      bool auto_kritz_friendsonly      = false;

      bool auto_kritz_react_friends    = false;
      bool auto_kritz_react_bonked     = false;
      bool auto_kritz_react_cloaked    = false;
      bool auto_kritz_deploy_near_tank = false;
      bool auto_call_medic             = false;

      bool auto_sticky_spam            = false;
      bool auto_sticky_spam_use_key    = false;
      u8   auto_sticky_spam_key        = VK_XBUTTON2;
      i32  auto_sticky_spam_at_charge  = 0;

      bool auto_shield                                      = false;
      bool auto_shield_auto_deploy                          = false;
      i32  auto_shield_deploy_at_count                      = 0;

      bool auto_soldier_whip                                = true;

      i32  auto_reflect_fov                                 = 180;
      bool demo_charge_bot_use_key                          = false;
      u8   demo_charge_bot_key                              = 0x4;
      bool auto_vaccinator_manualcharge_resist_cycle_option = true;
      bool auto_reflect_disable_forcing_crits               = true;
      bool auto_sticky_prevent_self_dmg                     = false;
      bool auto_reflect_target_mechbolts                    = true;
      bool auto_reflect_heal_bolts                          = true;

      bool auto_repair                      = false;
      i32  auto_repair_max_fov              = 180;
      bool auto_repair_use_key              = false;
      u8   auto_repair_key                  = VK_XBUTTON2;

      bool auto_conserve_hlh_ammo           = false;
      i32  auto_conserve_hlh_range_mult     = 0;
      bool auto_vaccinator_key_held         = true;
      bool auto_vaccinator_key_toggle       = false;
      bool auto_vaccinator_key_double_click = false;
      float auto_disguise_delay             = 100.f;
      float auto_sticky_blast_radius        = 100.f;

      bool automatic_heal_arrow              = false;
      bool unused34                          = true;
      i32  unused                            = 100;
      bool automatic_heal_arrow_friends_only = false;
      bool auto_sticky_detonate_use_key      = false;
      bool auto_sticky_detonate_key_held         = true;
      bool auto_sticky_detonate_key_toggle       = false;
      bool auto_sticky_detonate_key_double_click = false;
      u8   auto_sticky_detonate_key             = VK_XBUTTON2;
      bool auto_repair_rescue_ranger_enabled    = false;

      bool auto_sticky_spam_auto_release_near_entity = false;
      bool auto_sticky_spam_set_charge_release_at    = true;

      bool auto_reflect_aimbot = true;

      bool auto_rezoom             = false;
      bool auto_rezoom_when_moving = false;
      bool auto_rezoom_on_aimbot   = false;

      bool mvm_auto_ready_up       = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_settings{
  u32 version = SETTINGS_VERSION;
public:

  c_aimbot_settings aimbot[9][4]; // [0] = hitscan, [1] = projectile, [2] = medigun, [3] = melee
  c_player_esp_settings player_esp[3];
  c_world_esp_settings world_esp[3];
  c_player_chams_settings player_chams[6];
  c_world_chams_settings world_chams[6];
  c_visual_settings visual;
  c_hvh_settings hvh;
  c_misc_settings misc;
  c_automation_settings automation;
  c_triggerbot_settings triggerbot;

  // BASE FEATURES (With no overrides)
  c_misc_privacy_mode_settings             privacy_mode;
  c_movement_settings                      movement;
  c_third_person_settings                  tp;
  c_anti_cheat_manager_settings            acm;
  c_dispatch_user_message_feature_settings dum;
  c_fast_queue_settings                    fq;
  c_freecam_settings                       freecam;
  c_viewmodel_offset_settings              vm;
  c_chat_format_settings                   chat;
  RESERVE_SETTINGS(16); // Decrease if you add a new setting, and ensure it's in a union and is paddded by SETTINGS_DATA_PAD!

  vec3i double_tap_panel_pos;
  vec3i crit_hack_panel_pos;
  vec3i fake_latency_panel_pos;
  vec3i information_panel_pos;
  vec3i spectator_panel_pos;
};