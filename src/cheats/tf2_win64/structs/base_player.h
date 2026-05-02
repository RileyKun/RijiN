#pragma once

class c_player_data;
class c_base_player : public c_base_entity{
public:

  c_player_data* data();

  ALWAYSINLINE i32& buttons(){
    gen_read_offset(i32, "DT_LocalPlayerExclusive_m_hConstraintEntity", -0xC);
  }

  ALWAYSINLINE i32& player_class(){
    gen_read(i32, "DT_SPlayerClassShared_m_iClass");
  }

  ALWAYSINLINE i32 grappling_hook_target_handle(){
    gen_read(i32, "DT_SPlayer_m_hGrapplingHookTarget");
  }

  ALWAYSINLINE float& gravity(){
    gen_read_offset(float, "DT_LocalPlayerExclusive_m_vecBaseVelocity", 12);
  }

  ALWAYSINLINE i32& player_cond(){
    gen_read(i32, "DT_SPlayerShared_m_nPlayerCond");
  }

  ALWAYSINLINE i32& player_cond_ex(){
    gen_read(i32, "DT_SPlayerShared_m_nPlayerCondEx");
  }

  ALWAYSINLINE i32& player_cond_ex2(){
    gen_read(i32, "DT_SPlayerShared_m_nPlayerCondEx2");
  }

  ALWAYSINLINE i32& player_cond_ex3(){
    gen_read(i32, "DT_SPlayerShared_m_nPlayerCondEx3");
  }

  ALWAYSINLINE i32& player_cond_ex4(){
    gen_read(i32, "DT_SPlayerShared_m_nPlayerCondEx4");
  }

  ALWAYSINLINE i32 healers(){
    gen_read(i32, "DT_SPlayerShared_m_nNumHealers");
  }

  ALWAYSINLINE bool deadringer_deployed(){
    gen_read(float, "DT_SPlayerShared_m_bFeignDeathReady");
  }

  ALWAYSINLINE float cloak_meter(){
    gen_read(float, "DT_SPlayerShared_m_flCloakMeter");
  }

  ALWAYSINLINE float rage_meter(){
    gen_read(float, "DT_SPlayerSharedLocal_m_flRageMeter");
  }

  ALWAYSINLINE bool rage_draining(){
    gen_read(bool, "DT_SPlayerSharedLocal_m_bRageDraining");
  }

  ALWAYSINLINE bool is_jumping(){
    gen_read(bool, "DT_SPlayerShared_m_bJumping");
  }

  ALWAYSINLINE bool is_valid(bool check_dormant = true){
    return check_dormant ? !is_dormant() && is_alive() : is_alive();
  }

  ALWAYSINLINE s_shared* shared(){
    gen_read_pointer(s_shared*, "DT_SPlayer_m_Shared");
  }

  NEVERINLINE bool is_alive(){
    if(is_sentry_buster())
      return health() > 1 && !is_taunting();
    else if(has_condition(TF_COND_HALLOWEEN_GHOST_MODE))
      return false;

    return is_base_alive();
  }

  NEVERINLINE bool is_cloaked(bool logic_check = true){
    return logic_check ? (has_condition(TF_COND_STEALTHED) || has_condition(TF_COND_STEALTHED_USER_BUFF)) && !is_cloak_blinking() : (has_condition(TF_COND_STEALTHED) || has_condition(TF_COND_STEALTHED_USER_BUFF));
  }

  NEVERINLINE bool is_cloak_blinking(){
    return has_condition(TF_COND_BURNING) || has_condition(TF_COND_URINE) || has_condition(TF_COND_STEALTHED_BLINK) || has_condition(TF_COND_BLEEDING);
  }

  ALWAYSINLINE bool is_disguised(){
    return has_condition(TF_COND_DISGUISED);
  }

  NEVERINLINE bool is_ubercharged(){
    return has_condition(TF_COND_INVULNERABLE) || has_condition(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED) || has_condition(TF_COND_INVULNERABLE_CARD_EFFECT) || has_condition(TF_COND_INVULNERABLE_USER_BUFF);
  }

  ALWAYSINLINE bool is_ubercharge_hidden(){
    return has_condition(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED);
  }

  NEVERINLINE bool is_taunting(){
    return has_condition(TF_COND_TAUNTING) || has_condition(TF_COND_HALLOWEEN_THRILLER);
  }

  ALWAYSINLINE bool is_quickfix_uber(){
    return has_condition(TF_COND_MEGAHEAL);
  }

  ALWAYSINLINE bool is_over_healed(){
    return has_condition(TF_COND_HEALTH_OVERHEALED);
  }

  ALWAYSINLINE bool is_bonked(bool predicted = false){
    return has_condition(TF_COND_PHASE, predicted);
  }

  ALWAYSINLINE bool is_scoped(){
    return has_condition(TF_COND_ZOOMED);
  }

  NEVERINLINE bool is_on_fire(){
    return has_condition(TF_COND_BURNING) || has_condition(TF_COND_BURNING_PYRO);
  }

  ALWAYSINLINE bool is_bullet_resist(){
    return has_condition(TF_COND_MEDIGUN_SMALL_BULLET_RESIST);
  }

  ALWAYSINLINE bool is_bullet_charge(bool predicted = false){
    return has_condition(TF_COND_MEDIGUN_UBER_BULLET_RESIST, predicted);
  }

  ALWAYSINLINE bool is_blast_resist(){
    return has_condition(TF_COND_MEDIGUN_SMALL_BLAST_RESIST);
  }

  ALWAYSINLINE bool is_blast_charge(bool predicted = false){
    return has_condition(TF_COND_MEDIGUN_UBER_BLAST_RESIST, predicted);
  }

  ALWAYSINLINE bool is_fire_resist(){
    return has_condition(TF_COND_MEDIGUN_SMALL_FIRE_RESIST);
  }

  ALWAYSINLINE bool is_fire_charge(bool predicted = false){
    return has_condition(TF_COND_MEDIGUN_UBER_FIRE_RESIST, predicted);
  }

  NEVERINLINE bool is_vulnerable(){
    return has_condition(TF_COND_BURNING) || has_condition(TF_COND_HEALING_DEBUFF) || has_condition(TF_COND_BLEEDING) || has_condition(TF_COND_MARKEDFORDEATH) || has_condition(TF_COND_URINE);
  }

  ALWAYSINLINE bool is_player_class(i32 c){
    return c == player_class();
  }

  NEVERINLINE float get_health_percent(){
    if(health() <= 1 || max_health() <= 1)
      return 0.f;

    return ((float)health() / (float)max_health()) * 100.f;
  }

  ALWAYSINLINE c_score_data* get_round_score_data(){
    gen_read_pointer_other(shared(), c_score_data*, "DT_SPlayerSharedLocal_m_RoundScoreData");
  }

  //Inside C_TFPlayer Constructor
  //*(this + 0x1D88) = sub_104CD120(this);
  //\x89\x81\x00\x00\x00\x00\x51
  ALWAYSINLINE s_animation_state* get_animation_state(){
    return *(s_animation_state**)((uptr)this + 0x2358);
  }

  bool is_sentry_buster();
  i32 get_ammo_count(i32 ammo_type);
  i32 get_loaded_ammo();
  bool is_crit_boosted();
  float get_max_speed();
  float get_condition_prediction_time(u32 cond);
  bool has_condition(u32 cond, bool predicted = false);
  void add_condition(u32 cond, float duration = PERMANENT_CONDITION);
  void remove_condition(u32 cond);
  bool can_move_during_taunt();
  bool can_fire();
  bool can_fire_secondary(bool all_weapons_required = false, bool no_button_check = false);
  bool throwable_should_fire(bool holding_fire_button_check = false);
  bool should_medigun_fire();
  bool should_secondary_pda_fire();
  bool will_fire_this_tick();
  bool is_player_blocked();
  void set_blocked_state(bool blocked);
  bool can_headshot();
  bool can_spy_headshot();
  float get_sniper_charge_damage(c_base_player* target, bool crit);
  bool can_huntsman_kill(c_base_player* target);
  i32 get_max_buffed_health();
  float get_air_speed_cap();
  bool is_carrying_objective();
  bool is_localplayer();
  bool has_razer_back();
  bool is_dueling(bool cache = true);
  c_base_player* get_spectate_target();
  bool is_saxton_hale();
  i32  get_heal_arrow_heal_amount();
  void* get_equip_wearable_for_loadout_slot(i32 slot);
  bool has_canteen_equipped();
};