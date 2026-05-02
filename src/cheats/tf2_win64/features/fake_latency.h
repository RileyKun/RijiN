#pragma once

#define FAKE_LATENCY_MAX_PACKET_RECORDS 4096
struct s_packet_record{
  bool  filled;
  float time;
  i32   sequence_nr;
  i32   reliable_state;
};

class c_fake_latency{
public:
  bool            filled;
  s_packet_record records[FAKE_LATENCY_MAX_PACKET_RECORDS];
  i32             latest_record_index;
  float           next_sync;
  bool            found_sequence_numbers;

  ALWAYSINLINE void reset(){
    if(!filled)
      return;

    memset(this, 0, sizeof(*this));

    DBG("[!] c_fake_latency: reset\n");
  }

  ALWAYSINLINE s_packet_record* get_record(i32 index){
    if(!filled)
      return nullptr;

    index = latest_record_index - index;

    while(index < 0)
      index = FAKE_LATENCY_MAX_PACKET_RECORDS - math::abs(index);

    if(index > 0)
      index = math::clamp(index % FAKE_LATENCY_MAX_PACKET_RECORDS, 0, FAKE_LATENCY_MAX_PACKET_RECORDS - 1);
    else
      index = math::biggest(index, 0);

    s_packet_record* record = &records[index];

    if(!record->filled)
      return nullptr;

    return record;
  }

  ALWAYSINLINE void add_record(c_net_channel* net_channel){
    assert(net_channel != nullptr);

    latest_record_index = (latest_record_index + 1) % FAKE_LATENCY_MAX_PACKET_RECORDS;

    s_packet_record record;
    {
      record.filled         = true;
      record.time           = math::time(false, true);
      record.sequence_nr    = net_channel->in_sequence_nr;
      record.reliable_state = net_channel->in_reliable_state;
    }
    memcpy(&records[latest_record_index], &record, sizeof(s_packet_record));

    filled = true;
  }

  bool should_run(){
    static bool toggle_fake_latency        = false;
    if(!config->misc.fake_latency_enabled)
      return false;

    if(config->misc.fake_latency_use_key){
      if(!config->misc.fake_latency_key_held && !config->misc.fake_latency_key_toggle && !config->misc.fake_latency_key_double_click)
        config->misc.fake_latency_key_held = true;

      static c_key_control key(&config->misc.fake_latency_key, &config->misc.fake_latency_key_held, &config->misc.fake_latency_key_toggle, &config->misc.fake_latency_key_double_click);
      if(!key.is_toggled())
        return false;
    }

    if(utils::is_listen_server())
      return false;

    if(global->client_state->signon_state() != SIGNONSTATE_FULL)
      return false;

    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr)
      return false;

    if(!localplayer->is_alive())
      return false;

    c_base_weapon* weapon = localplayer->get_weapon();

    if(weapon == nullptr)
      return false;

    if(!config->misc.fake_latency_always_on){
      if(weapon->is_projectile_weapon())
        return false;
    }

    //if(weapon->is_medigun())
      //return false;

    return true;
  }

  float get_raw_target_latency(){
    if(!should_run() || config->misc.fake_latency_target_ms <= 0.f)
      return 0.f;

    return math::clamp(config->misc.fake_latency_target_ms / 1000.f, 0.2f, 0.8f);
  }

  float get_target_latency(){
    if(utils::is_listen_server() || !should_run())
      return 0.f;

    if(config->misc.fake_latency_target_ms <= 0.f)
      return 0.f;

    float target = get_raw_target_latency();

    target -= global->engine->get_latency(true, 0.f); // We already have this ping so subtract it
    target -= interp_manager->get_lerp();                  // Lag compensation adds lerp
    target  = math::clamp(target, 0.f, 1.f);

    return target;
  }

  bool is_synced(){
    if(!should_run() || get_raw_target_latency() <= 0.f || get_target_latency() <= 0.f)
      return true;

    return found_sequence_numbers && math::time(false, true) > next_sync;
  }

  void process_packet_run(c_net_channel* net_channel){
    if(net_channel == nullptr)
      return;

    if(!should_run()){
      next_sync = 0.f;
      return;
    }

    s_packet_record* latest_record = get_record(0);

    // Remove duplicate or lower sequence numbers
    if(latest_record != nullptr && latest_record->sequence_nr >= net_channel->in_sequence_nr)
      return;

    add_record(net_channel);
  }

  bool send_datagram_pre_run(c_net_channel* net_channel, s_packet_record* original_packet_record){
    if(net_channel == nullptr || original_packet_record == nullptr)
      return false;

    if(!should_run())
      return false;

    // Store original sequence number and reliable state
    {
      original_packet_record->sequence_nr    = net_channel->in_sequence_nr;
      original_packet_record->reliable_state = net_channel->in_reliable_state;
    }

    float precise_time   = math::time(false, true);
    float target_latency = get_target_latency();

    if(target_latency <= 0.f)
      return false;

    // Try to find a packet sequence number that correlates with our wish ping
    for(u32 i = 0; i < FAKE_LATENCY_MAX_PACKET_RECORDS; i++){
      s_packet_record* record = get_record(i);

      if(record == nullptr)
        break;

      if(record->time >= precise_time)
        continue;

      float time_delta = math::clamp(precise_time - record->time, 0.f, 1.f);

      // Skip all packet records that wont get us our wish latency time
      if(target_latency > time_delta || time_delta >= 1.f)
        continue;

      // Set the acknowledged sequence number and reliable state to our record packet from x ms ago
      // This will trick the server into calculating our latency as x ms
      net_channel->in_sequence_nr    = record->sequence_nr;
      net_channel->in_reliable_state = record->reliable_state;
      //DBG("net_channel->in_sequence_nr = %i (seq delta: %i, target_latency: %f (%i))\n", record->sequence_nr, (record->sequence_nr - original_packet_record->sequence_nr), target_latency, math::time_to_ticks(target_latency));

      if(next_sync == 0.f)
        next_sync = precise_time + target_latency + global->engine->get_latency(true, false) + interp_manager->get_lerp();

      return (found_sequence_numbers = true);
    }

    if(next_sync > 0.f)
      reset();

    return (found_sequence_numbers = false);
  }

  void send_datagram_post_run(c_net_channel* net_channel, s_packet_record* original_packet_record){
    if(net_channel == nullptr || original_packet_record == nullptr)
      return;

    // Restore the original sequence number and reliable state
    // If we don't do this the engine will not acknowledge packets correctly from here on
    net_channel->in_sequence_nr    = original_packet_record->sequence_nr;
    net_channel->in_reliable_state = original_packet_record->reliable_state;
  }

  void update_information_panel(){
    if(!config->misc.fake_latency_enabled)
      return;

    if(!utils::is_in_game() || global->info_panel == nullptr)
      return;

    float target_latency     = get_target_latency();
    float raw_target_latency = get_raw_target_latency();
    bool  synced             = is_synced();

    if(utils::is_listen_server()){
      global->info_panel->add_entry(INFO_PANEL_FAKE_LATENCY, WXOR(L"NOT POSSIBLE"), INFO_PANEL_WARNING_CLR);
      return;
    }

    wchar_t buf[IDEAL_MIN_BUF_SIZE];
    formatW(buf, WXOR(L"%iMS"), math::smallest((i32)((float)target_latency * 1000.f), 800));
    if(!should_run()){
      global->info_panel->add_entry(INFO_PANEL_FAKE_LATENCY, WXOR(L"INACTIVE"));
    }
    else if(math::time_to_ticks(target_latency) < 1){
      global->info_panel->add_entry(INFO_PANEL_FAKE_LATENCY, WXOR(L"TARGET MS TOO LOW"), globaldata->tick_count % 66 <= 32 ? INFO_PANEL_WARNING_CLR : INFO_PANEL_WARN_RAGE_CLR);
    }
    else if(synced)
      global->info_panel->add_entry(INFO_PANEL_FAKE_LATENCY, buf, rgb(11, 232, 129));
    else{
      global->info_panel->add_entry(INFO_PANEL_FAKE_LATENCY,  WXOR(L"SYNC"), rgb(255, 168, 1));
    }
  }

  void calculate_fake_ping(){
    assert(global->cvar != nullptr);
    static c_cvar* cl_cmdrate = global->cvar->find_var(XOR("cl_cmdrate"));
    if(cl_cmdrate == nullptr)
      return;

    cl_cmdrate->has_min = false;
    cl_cmdrate->has_max = false;
    static c_cvar* sv_maxcmdrate  = global->cvar->find_var(XOR("sv_maxcmdrate"));
    if(!should_run() || !config->misc.fake_latency_spoof_ms){
      cl_cmdrate->set_value(sv_maxcmdrate->val);
      return;
    }

    i32 val = 3;

    float raw_latency = get_raw_target_latency() * 1000.f;
    if(raw_latency >= 550.f)
      val = 1;
    else if(raw_latency >= 350.f)
      val = 2;

    i8 buf[IDEAL_MIN_BUF_SIZE];
    formatA(buf, XOR("%i"), val);
    cl_cmdrate->set_value(buf);
  }
};

CLASS_EXTERN(c_fake_latency, fake_latency);

#define FL_RAW_LATENCY fake_latency->get_raw_target_latency()