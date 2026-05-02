#pragma once

class c_cvar{
public:

  ALWAYSINLINE void set_value(i8* val){
    utils::virtual_call<10, void, i8*>(this, val);
  }

  ALWAYSINLINE void set_value(i32 val){
    utils::virtual_call<12, void, i32>(this, val);
  }

  struct cvar_val{
    i8*   str;
    i32   str_len;
    float flt;
    i32   val;
  };

  PAD(0x4);
  c_cvar*  next;
  bool     registered;
  i8*      name;
  i8*      help_string;
  i32      flags;
  void*    callback;
  c_cvar*  parent;
  u8*      default_value;
  cvar_val value;
  i32      has_min;
  float    min_val;
  i32      has_max;
  float    max_val;
  void*    change_callbacks;
};

class c_interface_cvar{
public:
  c_cvar* find_var(std::string name, bool mod = false){
    c_cvar* ret = utils::virtual_call<13, c_cvar*, i8*>(this, name.c_str());

    if(ret != nullptr && mod){
      ret->flags &= ~FCVAR_HIDDEN;
      ret->flags &= ~FCVAR_DEVELOPMENTONLY;
      ret->flags &= ~FCVAR_NOT_CONNECTED;
      ret->flags &= ~FCVAR_CHEAT;
    }

    return ret;
  }

  void enforce_var(std::string name, std::string value, bool clamp = false, float min = 0.f, float max = 0.f, bool mod = false){
    c_cvar* var = find_var(name, mod);
    if(var == nullptr){
      DBG("[-] enforce_var unknown cvar %s\n", name.c_str());
      return;
    }

    if(clamp){
      var->has_min = var->has_max = true;
      var->min_val = min;
      var->max_val = max;
    }

    i8* val = value.c_str();
    var->set_value(val);
  }

  void enforce_var(std::string name, float min, float max, bool mod = false){
    c_cvar* var = find_var(name, mod);
    if(var == nullptr){
      DBG("[-] enforce_var unknown cvar %s\n", name.c_str());
      return;
    }

    var->has_min = var->has_max = true;
    var->min_val = min;
    var->max_val = max;
    var->set_value(var->value.str);
  }
};