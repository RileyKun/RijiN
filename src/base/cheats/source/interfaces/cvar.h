#pragma once

typedef void (*change_callback1_fn)();
class c_cvar{
public:
  ALWAYSINLINE void set_value(std::string str){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, const i8*>(base_interface->cvar_set_value_str_index, this, str.c_str());
    STACK_CHECK_END;
  }

  ALWAYSINLINE void set_value(i32 val){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, i32>(base_interface->cvar_set_value_int_index, this, val);
    STACK_CHECK_END;
  }

  // Probably should avoid this, but, it's only for GMOD. If other games end up having more different cvar layouts then this can be moved to cheats themself.
  #if defined(GMOD_CHEAT)
    uptr*    rtti;
    c_cvar*  next;
    bool     registered;
    PAD(0x4);
    i8*      name;
    i8*      help_string;
    i32      flags;
    PAD(0xC);
    c_cvar*  parent;
    u8*      default_value;
    i8*      str;
    u32      str_len;
    PAD(0x8);
    
    float flt(){
      return convert::str_to_float(str);
    }
  
    i32 val(){
      return convert::str_to_i32(str);
    }
  
    i32      has_min;
    float    min_val;
    i32      has_max;
    float    max_val;
    void*    change_callbacks;
  #else
    uptr*               rtti;
    c_cvar*             next;
    bool                registered;
    i8*                 name;
    i8*                 help_string;
    iptr                flags;
    change_callback1_fn change_callbacks_v1;
    c_cvar*             parent;
    i8*                 default_value;
  
    // values
    i8*                 str;
    i32                 str_len;
    float               flt;
    i32                 val;
  
    i32                 has_min;
    float               min_val;
    i32                 has_max;
    float               max_val;
      #if defined(TF2_CHEAT) 
      PAD(0x11)
      #endif
    void*               change_callbacks;
  #endif
};

class c_interface_cvar{
public:
  ALWAYSINLINE c_cvar* get_commands(){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<c_cvar*>(base_interface->get_commands_index, this);
    STACK_CHECK_END;
    return r;
  }

  ALWAYSINLINE c_cvar* find_command(std::string name){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<c_cvar*, const i8*>(base_interface->find_command_index, this, name.c_str());
    STACK_CHECK_END;
    return r;
  }

  ALWAYSINLINE c_cvar* find_var(std::string name){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<c_cvar*, const i8*>(base_interface->cvar_find_var_index, this, name.c_str());
    STACK_CHECK_END;
    return r;
  }

  ALWAYSINLINE void enforce_var(std::string name, std::string value){
    c_cvar* var = find_var(name);
    if(var == nullptr){
      DBG("[-] c_interface_cvar::enforce_var: %s not found.\n", name.c_str());
      return;
    }

    var->set_value(value);
    DBG("[+] c_interface_cvar::enforce_var: %s -> %s\n", name.c_str(), value.c_str());
  }

  #if defined(DEV_MODE)
  // Only allowed in dev mode.
  ALWAYSINLINE void clear_flags_and_set(std::string name, std::string value = ""){
    c_cvar* var = find_var(name);
    if(var == nullptr)
      return;

    iptr o_flags = var->flags;
    var->flags &= ~FCVAR_NOT_CONNECTED;
    var->flags &= ~FCVAR_HIDDEN;
    var->flags &= ~FCVAR_DEVELOPMENTONLY;

    if(!value.empty())
      var->set_value(value.c_str());
  }

  #endif

  ALWAYSINLINE void dont_record(std::string name){
    c_cvar* var = find_var(name);
    if(var == nullptr){
      var = find_command(name);
      if(var == nullptr){
        DBG("[-] c_interface_cvar::dont_record: Unable to find cvar or command %s\n", name.c_str());
        return;
      }
    }

    if(!(var->flags & FCVAR_DONTRECORD))
      var->flags |= FCVAR_DONTRECORD;

    DBG("[+] c_interface_cvar::dont_record: %s added FCVAR_DONTRECORD\n", name.c_str());
  }

  ALWAYSINLINE bool is_enabled(std::string name){
    c_cvar* var = find_var(name);
    if(var == nullptr)
      return false;

    #if defined(GMOD_CHEAT)
      return val() == 1;
    #else
    return var->val == 1;
    #endif
  }

  // Settings that can break certain chams.
  ALWAYSINLINE bool is_chams_broken(){
    return !is_enabled(XOR("mat_bumpmap")) 
        || !is_enabled(XOR("mat_managedtextures")) 
        || is_enabled(XOR("mat_forcemanagedtextureintohardware"))
        || !is_enabled(XOR("r_drawbatchdecals"));
  }

  // It doesn't matter if somethings are game only in here.
  ALWAYSINLINE void setup(){
    cookie_block_check_return();

    // Client networking
    enforce_var(XOR("rate"), XOR("1048576"));
    enforce_var(XOR("cl_cmdrate"), XOR("66"));
    enforce_var(XOR("cl_updaterate"), XOR("66"));
    enforce_var(XOR("cl_interp_ratio"), XOR("1"));
    enforce_var(XOR("cl_interp"), XOR("0.0"));
    enforce_var(XOR("cl_smooth"), XOR("0"));
    enforce_var(XOR("cl_pred_optimize"), XOR("2"));

    // Server networking
    enforce_var(XOR("sv_lan"), XOR("1"));
    enforce_var(XOR("sv_maxcmdrate"), XOR("66"));
    enforce_var(XOR("sv_maxupdaterate"), XOR("66"));
    enforce_var(XOR("sv_client_min_interp_ratio"), XOR("-1"));
    enforce_var(XOR("sv_maxrate"), XOR("0"));
    enforce_var(XOR("sv_minrate"), XOR("80000"));

    // Ideal for performance.
    enforce_var(XOR("mat_queue_mode"), XOR("2"));
    enforce_var(XOR("r_threaded_particles"), XOR("1"));
    enforce_var(XOR("r_threaded_renderables"), XOR("1"));

    // Sometimes if 1 it'll cause an engine error: Engine hunk overflow.
    enforce_var(XOR("r_hunkalloclightmaps"), XOR("0")); 

    // So chams can be all happy.
    enforce_var(XOR("mat_bumpmap"), XOR("1"));
    enforce_var(XOR("mat_managedtextures"), XOR("1"));
    enforce_var(XOR("mat_forcemanagedtextureintohardware"), XOR("0"));
    enforce_var(XOR("r_drawbatchdecals"), XOR("1"));

    #if defined(DEV_MODE)
      clear_flags_and_set(XOR("developer"), XOR("1"));
      clear_flags_and_set(XOR("net_showevents"), XOR("0"));
    #endif

    // TF2
#if defined(TF2_CHEAT)
    enforce_var(XOR("tf_spec_xray_disable"), XOR("1"));
    enforce_var(XOR("tf_enable_glows_after_respawn"), XOR("0"));
#endif
  }
};