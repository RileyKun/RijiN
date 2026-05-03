#pragma once

#define PLAYER_LIST_DATA_VERSION HASH("VERSION1");

#pragma pack(push,1)
class c_base_player_list_data{
public:
  c_base_player_list_data(){
    init();
  }

  virtual void init(){
    version = PLAYER_LIST_DATA_VERSION;

    mark_as_auto       = true;
    mark_as_cheater    = false;
    mark_as_friend     = false;
    mark_as_bot        = false;
    mark_as_suspicious = false;
    mark_as_legit      = false;

    resolver_auto      = true;
    resolver_disallow  = false;
    resolver_force     = false;

    resolver_pitch_auto = true;
    resolver_pitch_up   = false;
    resolver_pitch_down = false;
    resolver_pitch_zero = false;

    resolver_yaw_auto    = true;
    resolver_yaw_neg90   = false;
    resolver_yaw_pos90   = false;
    resolver_yaw_neg45   = false;
    resolver_yaw_pos45   = false;
    resolver_yaw_lookat  = false;
    resolver_yaw_180away = false;

    ignore_player        = false;


    // We want to ensure the data is initilized no black colours allowed.
    {
      esp_colour[0] = 96;
      esp_colour[1] = 255;
      esp_colour[2] = 164;
      esp_colour[3] = 255;
    }

    // Base Chams colour
    {
      chams_colour[0] = 96;
      chams_colour[1] = 255;
      chams_colour[2] = 164;
      chams_colour[3] = 32;
    }

    // Overlay Chams colour
    {
      chams_overlay_colour[0] = 96;
      chams_overlay_colour[1] = 255;
      chams_overlay_colour[2] = 164;
      chams_overlay_colour[3] = 32;
    }

    // Glow colour
    {
      glow_colour[0] = 96;
      glow_colour[1] = 255;
      glow_colour[2] = 164;
      glow_colour[3] = 255;
    }

    priority_mode = false;
    priority      = 0;
    valid         = true;
  }

  virtual u32 get_size(){
    return sizeof(c_base_player_data);
  }

  u32 version;

  i8   alias[64];
  bool mark_as_friend;
  bool mark_as_auto;
  bool mark_as_legit;
  bool mark_as_cheater;
  bool mark_as_bot;
  bool mark_as_suspicious;

  bool resolver_auto;
  bool resolver_disallow;
  bool resolver_force;

  bool resolver_pitch_auto;
  bool resolver_pitch_up;
  bool resolver_pitch_down;
  bool resolver_pitch_zero;

  bool resolver_yaw_auto;
  bool resolver_yaw_neg90;
  bool resolver_yaw_pos90;
  bool resolver_yaw_neg45;
  bool resolver_yaw_pos45;
  bool resolver_yaw_lookat;
  bool resolver_yaw_180away;

  bool ignore_player;
  bool custom_esp_colour;
  bool custom_chams_base_colour;
  bool custom_chams_overlay_colour;
  bool custom_glow_colour;

  float esp_colour[4];
  float chams_colour[4];
  float chams_overlay_colour[4];
  float glow_colour[4];

  bool priority_mode;
  i32  priority;
  bool valid;
};
#pragma pack(pop)

class c_base_player_list_data_manager{
public:
  ALWAYSINLINE inline void alloc_table(u32 size){
    assert(size >= sizeof(c_base_player_data)); // Incorrect size.
    table = malloc(MAXPLAYERS * size);
  }


  c_base_player_list_data* table = nullptr;
};