#pragma once

enum e_fast_queue_regions{
  fast_queue_region_unknown = 0,
  fast_queue_region_eu = 1,
  fast_queue_region_asia_me = 2,
};

class c_fast_queue_settings{
public:
  union{
    struct{
      bool enabled                 = true;
      u32  max_ping                = 200;

      // EU
      bool block_netherlands       = false;
      bool block_frankfurt_germany = false;
      bool block_heathrow_uk       = false;
      bool block_madrid_spain      = false;
      bool block_paris_france      = false;
      bool block_luxembourg        = false;
      bool block_stockholm_sweden  = false;
      bool block_warsaw_poland     = false;
      bool block_sofia_bulgaria    = false;
      bool block_helsinki_finland  = false;
      bool block_vienna_austria    = false;
      bool block_milan_italy       = false;

      // Asia / Middle east.
      bool block_mumbai_india    = false;
      bool block_chennai_india   = false;
      bool block_dubai_uae       = false;
      bool block_hong_kong       = false;
      bool block_shanghai_china  = false;
      bool block_guangzhou_china = false;
      bool block_tianjin_china   = false;
      bool block_tokyo_japan     = false;
      bool block_singapore       = false;
      bool block_seoul_skorea    = false;

      // USA
      bool block_us_texas          = false;
      bool block_us_new_york       = false;
      bool block_us_atlanta        = false;
      bool block_us_washington     = false;
      bool block_us_chicago        = false;
      bool block_us_california     = false;
      bool block_us_oklahoma       = false;
      bool block_us_new_orleans    = false;
      bool block_us_block_virginia = false;
      bool block_us_florida        = false;
      bool block_us_boston         = false;
      bool block_us_seattle_wshton = false;

      // South america
      bool block_brazil    = false;
      bool block_peru      = false;
      bool block_chile     = false;
      bool block_argentina = false;

      // Africa / Australia
      bool block_africa    = false;
      bool block_australia = false;
    };
    PAD(SETTINGS_DATA_PAD);
  };
};

class c_base_fast_queue{
public:

  virtual c_fast_queue_settings* get_settings(){
    FUNCTION_OVERRIDE_NEEDED;
    return nullptr;
  }

  virtual u32  think(u32 ping, u32 pop_id);
  virtual i8*  get_pop_name(u32 pop_id);

  virtual u32 get_whitelist_ping(){
    return (u32)math::random_int(5, 20);
  }

  virtual u32 get_blacklist_ping(){
    return (u32)math::random_int(700, 1250);
  }

  virtual void create_menu_settings(gcs_component_group* group, c_fast_queue_settings* settings);
};