#pragma once

class c_aimbot_filter_base{
public:
  // base data
  c_base_player*  localplayer       = nullptr;
  c_base_weapon*  localplayer_wep   = nullptr;
  c_base_player*  target            = nullptr;
  c_base_entity*  target_ent        = nullptr;
  float           max_range         = 0.f;
  float           current_time      = 0.f;
  bool            priority_only     = false;

  // custom
  s_lc_record*    lc_record         = nullptr;
  vec3            aim_point;

  c_aimbot_filter_base(){
    localplayer = utils::localplayer();
    if(localplayer != nullptr)
      localplayer_wep = localplayer->get_weapon();

    current_time = math::time();
  }

  virtual void reset(){
    memset(this, 0, sizeof(*this));
  }

  virtual void reset_filter_values(){
    max_range = 0.f;
    target    = nullptr;
    target_ent = nullptr;
  }

  virtual bool handle_priority(s_lc_record* record, c_base_entity* entity){
    if(entity == nullptr || target_ent == nullptr)
      return true;
    
    c_base_player* target_player  = (c_base_player*)target_ent;
    c_base_player* current_player = (c_base_player*)entity;

    if(target_ent == entity)
      return true;
    
    c_entity_data* target_data  = target_player->data();
    c_entity_data* current_data = current_player->data();
    if(target_data == nullptr || current_data == nullptr)
      return true;

    if(!target_data->friends_id || !current_data->friends_id)
      return true;

    if(priority_only && !current_data->playerlist.priority_mode)
      return false;
    
    // None of them have this enabled so forget about checking anything else.
    if(!target_data->playerlist.priority_mode && !current_data->playerlist.priority_mode)
      return true;
    
    // Even if either one of them is enabled, if the priority is zero then don't bother checking anything.
    if(!target_data->playerlist.priority && !current_data->playerlist.priority)
      return true;
    
    // Target has priority mode enabled while the player being checked does not.
    if(target_data->playerlist.priority_mode && !current_data->playerlist.priority_mode)
      return false;
    
    // Target has more priority over the other player.
    if(target_data->playerlist.priority_mode && target_data->playerlist.priority > current_data->playerlist.priority)
      return false;

    // We're doing this to clear up by 'best of' values like fov/dist checks.
    target_ent = nullptr;
    reset_filter_values();
    priority_only = true;

    return true;
  }

  virtual bool sanity(s_lc_record* record, c_base_entity* entity){
    if(localplayer == nullptr || entity == nullptr)
      return false;

    if(entity->is_dormant())
      return false;

    return true;
  }

  virtual bool handle(s_lc_record* record, c_base_entity* entity){
    return false;
  }

  virtual bool store(s_lc_record* record, c_base_entity* entity){
    return false;
  }

  virtual bool sanity(s_lc_record* record, c_base_player* player){
    if(localplayer == nullptr || player == nullptr)
      return false;

    if(!player->is_valid())
      return false;

    if(!player->is_alive())
      return false;

    return true;
  }

  virtual bool handle(s_lc_record* record, c_base_player* player){
    return false;
  }

  virtual bool store(s_lc_record* record, c_base_player* player){
    return false;
  }
};

class c_aimbot_filter_raytrace_fov : public c_aimbot_filter_base{
public:
  float closest_fov = 0.f;

  void reset() override{
    memset(this, 0, sizeof(*this));
  }

  bool handle( s_lc_record* record, c_base_entity* entity ) override;
  bool store( s_lc_record* record, c_base_entity* entity ) override;
};

class c_aimbot_filter_raytrace_smart_shot : public c_aimbot_filter_base{
public:
  float closest_distance = 0.f;
  float current_distance = 0.f;
  bool  first_target_blacklisted = false;
  bool  was_idle         = false;

  void reset() override{
    memset(this, 0, sizeof(*this));
  }

  bool handle( s_lc_record* record, c_base_entity* entity ) override;
  bool store( s_lc_record* record, c_base_entity* entity ) override;
};

class c_aimbot_filter_raytrace_distance : public c_aimbot_filter_base{
public:
  float closest_distance = 0.f;
  float current_distance = 0.f;

  void reset() override{
    memset(this, 0, sizeof(*this));
  }

  bool handle( s_lc_record* record, c_base_entity* entity ) override;
  bool store( s_lc_record* record, c_base_entity* entity ) override;
};

class c_aimbot_filter_bash_distance : public c_aimbot_filter_base{
public:
  float closest_distance = 0.f;
  float current_distance = 0.f;

  void reset() override{
    memset(this, 0, sizeof(*this));
  }

  bool handle( s_lc_record* record, c_base_entity* entity ) override;
  bool store( s_lc_record* record, c_base_entity* entity ) override;
};

class c_aimbot_filter_melee_fov : public c_aimbot_filter_base{
public:
  float closest_fov = 0.f;

  void reset() override{
    memset(this, 0, sizeof(*this));
  }

  bool handle( s_lc_record* record, c_base_entity* p ) override;
  bool store( s_lc_record* record, c_base_entity* p ) override;
};
