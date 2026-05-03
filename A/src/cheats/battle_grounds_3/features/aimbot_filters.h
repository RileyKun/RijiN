#pragma once

class c_aimbot_filter_base{
public:
  // base data
  c_base_player*  localplayer       = nullptr;
  c_base_weapon*  localplayer_wep   = nullptr;
  c_base_player*  target            = nullptr;
  c_base_entity*  target_ent        = nullptr;
  float           max_range         = 0.f;

  // custom
  s_lc_record*    lc_record         = nullptr;
  vec3            aim_point;

  c_aimbot_filter_base(){
    localplayer = utils::localplayer();
    if(localplayer != nullptr)
      localplayer_wep = localplayer->get_weapon();
  }

  virtual void reset(){
    memset(this, 0, sizeof(*this));
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

class c_aimbot_filter_melee_fov : public c_aimbot_filter_base{
public:
  float closest_fov = 0.f;

  void reset() override{
    memset(this, 0, sizeof(*this));
  }

  bool handle( s_lc_record* record, c_base_entity* p ) override;
  bool store( s_lc_record* record, c_base_entity* p ) override;
};
