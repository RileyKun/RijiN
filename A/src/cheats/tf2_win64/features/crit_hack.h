#pragma once

// TODO:
// Find crit damage by reversing value of observed crit chance when its networked to us
// WORKING POC:
/*
  float observed        = 0.111798f;
  int   ranged_damage   = 3101;

  // find the correct ranged crit random
  for(int i = 1; i < ranged_damage; i++){
    float normalized_damage = (float)i / 3.f;
    float ratio             = normalized_damage + ((float)ranged_damage - (float)i);
    float observed_sim      = normalized_damage / ratio;

    //if(observed_sim == observed)
        printf("dmg: %i %f\n", i, observed_sim);
  }
*/

#define CRIT_HACK_MAX_CMDS 33
#define CRIT_HACK_BACKUP_START XOR32(0xE94)/*m_flCritTokenBucket*/
#define CRIT_HACK_BACKUP_END XOR32(0xFC4 + sizeof(float))/*m_flLastRapidFireCritCheckTime*/
//#define CRIT_HACK_DEBUG

struct s_cmd_list{
  i32   list[CRIT_HACK_MAX_CMDS];
  i32   cur_cmd;
  i32   mask_xor;
  bool  nospread        = false;
  bool  filled          = false;
  bool  thread_running  = false;

  void reset(){
    i32 old_mask_xor = mask_xor;

    memset(this, 0, sizeof(*this));

    mask_xor = old_mask_xor;
  }

  i32 get_entry(i32 seed){
    for(u32 i = 0; i < CRIT_HACK_MAX_CMDS; i++)
      if((math::md5_pseudo_random(list[i]) & 0x7FFFFFFF) == seed)
        return i;

    return -1;
  }

  i32& current_cmd() {
    return list[ cur_cmd % CRIT_HACK_MAX_CMDS ];
  }

  i32 next_cmd() {
    return list[ ( cur_cmd + 1 ) % CRIT_HACK_MAX_CMDS ];
  }

  bool set_next_current_cmd() {
    cur_cmd = ( cur_cmd + 1 ) % CRIT_HACK_MAX_CMDS;

    return cur_cmd == 0;
  }
};

struct s_crit_thread_data{
  i32               mask_xor;
  c_good_seed_data* seed_data;
  s_cmd_list*       force_list;
  s_cmd_list*       skip_list;
  bool              nospread;
  bool*             calc_taken;
};

struct s_crit_bucket_backup{
  i32   player_class;
  i32   item_def_index;
  i32   crit_checks;
  i32   crit_seed_requests;
  float crit_token_bucket;
};

struct s_crit_damage_thread_data{
  float crit_chance;
  bool  is_melee;
  bool  run_thread;
};

struct s_tfstat_damage_ranged_crit_random{
  void reset(){
    memset(this, 0, sizeof(*this));
  }

  float last_sync_time      = 0.f;
  i32   last_synced_damage  = 0;
  bool  was_desynced        = false;
};

class c_crit_hack{
public:
  bool                                can_crit              = false;
  bool                                crit_banned           = false;
  bool                                needs_bucket_resync   = false;
  float                               crit_chance_correct   = 0.f;
  float                               bucket_add_per_shot   = 0.f;
  float                               damage_correct        = 0.f;
  i32                                 force_prediction_seed = 0;
  s_cmd_list                          force_cmds[4096];
  s_cmd_list                          skip_cmds[4096];
  s_crit_bucket_backup                bucket_backup[7];
  s_tfstat_damage_ranged_crit_random  tfstat_damage_ranged_crit_random;

  void reset(){
    memset(this, 0, sizeof(*this));
  }

  void reset_bucket_backup();
  void resync_bucket();
  void backup_crit_bucket();

  void reset_damages();
  void update_damages();
  void find_tfstat_damage_ranged_crit_random(float observed_crit_chance);

  i32 get_weapon_mask_xor();

  void calculate_can_crit();
  void fix_shared_attack_bug();
  void fix_primary_attack_bug(bool pre_prediction);

  float calculate_observed_crit_chance(bool is_melee, i32 crit_dmg_simulate = 0, i32 normal_damage_simulate = 0);

  void can_fire_random_critical_shot_run(c_base_weapon* ecx, float crit_chance);

  s_cmd_list* get_cmd_list(bool crit);
  
  // calculates the crit cost based on the requests and checks made
  float get_crit_cost(i32 requests, i32 checks, bool melee, bool use_rapid_fire_crits);
  bool get_crit_counts(i32* available_crits, i32* potential_crits, float* remaining_bucket = nullptr, float* remaining_cost = nullptr);
  void update_damage_for_unban();
  void fire_event_run(c_game_event* event);
  void fill_crit_seed_list();
  bool force(bool should_crit);
  void createmove_run();
};

CLASS_EXTERN(c_crit_hack, crit_hack);