#pragma once

#define TICKBASE_CORRECTION_MAX_TB_RECORDS 66
struct s_tb_record{
  bool  filled;                   // Is the record filled?
  i32   command_number;           // Command number at which clock correction should take place
  i32   choked_commands;          // Amount of commands to be batched after command_number (command_number + choked_commands)
  i32   corrected_tickbase;       // The corrected tickbase
  i32   last_corrected_tickbase;  // Last corrected command number in this batch
};

class c_tickbase_correction{
public:
  bool        filled;
  s_tb_record records[TICKBASE_CORRECTION_MAX_TB_RECORDS];
  i32         latest_record_index;
  i32         processing_tick;

  ALWAYSINLINE void reset(){
    if(!filled)
      return;

    memset(this, 0, sizeof(*this));
    DBG("[!] c_tickbase_correction: reset\n");
  }

  s_tb_record* get_record(i32 index){
    if(!filled)
      return nullptr;

    index = latest_record_index - index;

    while(index < 0)
      index = TICKBASE_CORRECTION_MAX_TB_RECORDS - math::abs(index);

    if(index > 0)
      index = math::clamp(index % TICKBASE_CORRECTION_MAX_TB_RECORDS, 0, TICKBASE_CORRECTION_MAX_TB_RECORDS - 1);
    else
      index = math::biggest(index, 0);

    s_tb_record* record = &records[index];

    if(!record->filled)
      return nullptr;

    return record;
  }

  s_tb_record* find_record(i32 command_number){
    for(u32 i = 0; i < TICKBASE_CORRECTION_MAX_TB_RECORDS; i++){
      s_tb_record* tb_record = get_record(i);

      if(tb_record == nullptr)
        break;

      // Are we outside of the region?
      if(command_number < tb_record->command_number || command_number > tb_record->command_number + tb_record->choked_commands)
        break;

      return tb_record;
    }

    return nullptr;
  }

  void add_record(i32 tickbase, i32 command_number, i32 choked_commands){
    s_tb_record record;
    {
      record.filled                   = true;
      record.command_number           = command_number;
      record.choked_commands          = choked_commands;
      record.corrected_tickbase       = tickbase;
      record.last_corrected_tickbase  = 0;

      if(!adjust_player_time_base(&record, record.corrected_tickbase)){
        DBG("[!] c_tickbase_correction: no correction needed on %i+%i\n", command_number, choked_commands);
        return;
      }
    }

    latest_record_index = (latest_record_index + 1) % TICKBASE_CORRECTION_MAX_TB_RECORDS;
    memcpy(&records[latest_record_index], &record, sizeof(s_tb_record));

    filled = true;
  }

  bool adjust_player_time_base(s_tb_record* tb_record, i32& tickbase){
    s_global_data old_global_data;
    memcpy(&old_global_data, global->server_global_data, sizeof(s_global_data));

    // Predict the servers gpGlobals->tickcount (if its not in sync with our tickcount)
    if(globaldata->tick_count != global->server_global_data->tick_count){
      global->server_global_data->tick_count = global->client_state->server_tick() + math::time_to_ticks(global->engine->get_latency(false) + global->engine->get_latency(true));
      global->current_cmd->tick_count = global->server_global_data->tick_count;
      DBG("[!] adjust_player_time_base: server_tick_count: %i (%i)\n", global->server_global_data->tick_count, global->server_global_data->tick_count - globaldata->tick_count);
    }


    // Set remaining ticks to simulate
    global->server_global_data->max_clients          = globaldata->max_clients;
    global->server_global_data->sim_ticks_this_frame = 0;

    // Create fake c_base_player and get offset to m_nTickBase
    static void*  c_base_player = malloc(0x2300);
    i32*          m_ntickbase   = (u32*)((u32)c_base_player + 0x2058);

    // Write our tickbase to fake tickbase
    *m_ntickbase = tickbase;

    // Call AdjustPlayerTimeBase
    STACK_CHECK_START;
    utils::call_thiscall<void, i32>(gen_internal->decrypt_asset(global->server_adjust_player_time_base_hook_trp), c_base_player, tb_record->choked_commands);
    STACK_CHECK_END;

    // Was the tickbase adjusted?
    bool tickbase_adjusted = tickbase != *m_ntickbase;

    if(tickbase_adjusted)
      DBG("[!] adjust_player_time_base: tickbase_adjusted: %i\n", *m_ntickbase - tickbase);

    // Write the new corrected tickbase
    tickbase = *m_ntickbase;

    memcpy(global->server_global_data, &old_global_data, sizeof(s_global_data));

    return tickbase_adjusted;
  }
};

CLASS_EXTERN(c_tickbase_correction, tickbase_correction);