#pragma once

enum EH_LOCATION{
  EH_CL_MOVE_SP_LOCATION_ZEROING = 1,
  EH_CL_MOVE_SP_LOCATION_SET_CHOKE,
};

class c_base_packet_manager{
private:
#if defined(__x86_64__)
  bool  send_packet = true;
#else
  bool* send_packet = nullptr;
#endif
public:
  bool  force_next_send_packet   = false;
  bool  should_force_send_packet = false;
  bool  is_locked                = false;

  #if defined(DEV_MODE) && defined(__x86_64__)
  virtual void unload(HANDLE thread){
    if(!global_internal->unloading)
      return;

    CONTEXT ctx{};
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    GetThreadContext(thread, &ctx);
    if(ctx.Dr0 != 0){
      ctx.Dr0 = 0;
      ctx.Dr7 = 0;
      SetThreadContext(thread, &ctx);
    }

    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    GetThreadContext(thread, &ctx);
    if(ctx.Dr1 != 0){
      ctx.Dr1 = 0;
      ctx.Dr7 = 0;
      SetThreadContext(thread, &ctx);
    }
  }
  #endif
  virtual void reset(){
    // Do not use.
    #if defined(__x86_64__)
    send_packet              = true;
    #else
    send_packet              = nullptr;
    #endif
    force_next_send_packet   = false;
    should_force_send_packet = false;
    is_locked                = false;
  }

  #if defined(__x86_64__)
  virtual void compute_send_packet_val(bool final_tick){
    send_packet = final_tick;
    if(utils::is_listen_server()){
      static c_cvar* host_limitlocal = global_internal->cvar->find_var(XOR("host_limitlocal"));
      if(host_limitlocal != nullptr)
        host_limitlocal->set_value(1);
    }
  }

  virtual void set_breakpoint(HANDLE thread, u32 type){
    #if defined(DEV_MODE)
    unload(thread);
    #endif

    assert(global_internal->cl_move_test_dil_addr != nullptr);
    assert(global_internal->cl_move_send_packet_false_addr != nullptr);

    switch(type){
      default: break;
      case EH_CL_MOVE_SP_LOCATION_ZEROING:
      {
        CONTEXT ctx{};
        ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        GetThreadContext(thread, &ctx);
        if(ctx.Dr0 == 0){
          ctx.Dr0 = global_internal->cl_move_test_dil_addr;
          ctx.Dr7 = (1 << 0); // L0BPDR0 Enable Flag
          SetThreadContext(thread, &ctx);
        }
        break;
      }
      case EH_CL_MOVE_SP_LOCATION_SET_CHOKE:
      {
        CONTEXT ctx{};
        ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        GetThreadContext(thread, &ctx);
        if(ctx.Dr1 == 0){
          ctx.Dr1 = global_internal->cl_move_test_dil_addr;
          ctx.Dr7 = (1 << 0); // L0BPDR0 Enable Flag
          SetThreadContext(thread, &ctx);
        }
        break;
      }
    }
  }

  virtual void handle_eh(CONTEXT* context_frame, u32 type){
    // Clear breakpoint.

    switch(type){
      default: break;
      case EH_CL_MOVE_SP_LOCATION_ZEROING:
      {
        context_frame->Dr0 = 0;
        context_frame->Dr7 = 0;

        DBG("[!] NEEDS CHOKE\n");
        set_breakpoint(GetCurrentThread(), EH_CL_MOVE_SP_LOCATION_SET_CHOKE);
        send_packet = false;
        break;
      }
      case EH_CL_MOVE_SP_LOCATION_SET_CHOKE:
      {
        context_frame->Dr1 = 0;
        context_frame->Dr7 = 0;
        assert(context_frame != nullptr);
        if(send_packet)
          context_frame->EFlags &= ~0x40;
        else
          context_frame->EFlags |= 0x40;

        set_breakpoint(GetCurrentThread(), EH_CL_MOVE_SP_LOCATION_ZEROING);
        context_frame->Rip += 3; // Skip the test dil, dil instruction.
        break;
      }
    }


    context_frame->EFlags |= (1 << 16); // RF Flag
    utils::nt_continue(context_frame, false);
  }
  #endif

  virtual void post_send_packet_init(bool* sp = nullptr){
    #if !defined(__x86_64__)
      send_packet = sp;
    #endif
    if(force_next_send_packet){
      should_force_send_packet  = true;
      force_next_send_packet    = false;
    }
  }

  virtual void post_create_move_prediction(){
    if(!should_force_send_packet)
      return;

    set_choke(false);
    should_force_send_packet = false;
  }

  bool set_choke(bool choke){
    #if !defined(__x86_64__)
    if(send_packet == nullptr)
      return false;
    #endif

    if(is_locked){
      DBG_STAGING("[!] set_choke locked would have done %i\n", choke);
      #if defined(__x86_64__)
        return send_packet;
      #else
        return *send_packet;
      #endif
    }

    #if defined(__x86_64__)
      return (send_packet = !(!should_force_send_packet && allow_choking(choke) && choke));
    #else
      return (*send_packet = !(!should_force_send_packet && allow_choking(choke) && choke));
    #endif
  }

  void lock(bool locked){
    DBG_STAGING("[!] locking packet changes : %i to %i\n", is_locked, locked);
    is_locked = locked;
  }

  void force_send_packet(bool next_tick){
    if(is_locked){
      DBG_STAGING("[!] force packet blocked due to being locked (%i)\n", next_tick);
      return;
    }

    if(next_tick)
      force_next_send_packet = true;
    else{
      should_force_send_packet = true;
      set_choke(false);
    }
  }

  virtual i32 get_max_choke_ticks(){
    DBG("[-] c_base_packet_manager::get_max_choke_ticks has no override so defaulting to 14\n");
    return 14;
  }

  virtual bool allow_choking(bool predicted = false){
    FUNCTION_OVERRIDE_NEEDED;
    return false; 
  }

  ALWAYSINLINE bool choking(){
    #if defined(__x86_64__)
      return !should_force_send_packet && !send_packet;
    #else
      return !should_force_send_packet && !*send_packet;
    #endif
  }

  virtual void on_input(){
    if(choking())
      force_send_packet(true);
    else{
      force_send_packet(false);
      force_send_packet(true);
    }
  }

  virtual bool get_sendpacket_value(){
    return send_packet;
  }

  // Lets say, we're doing serverside silent, but something changed angle, and thus we'll leak an aimbot angle.
  virtual void disregard_next_frame_unchoke(){
    force_next_send_packet   = false;
    should_force_send_packet = false;
  }
};