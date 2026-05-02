#pragma once

class c_cvar{
public:

  ALWAYSINLINE void set_value(i8* val){
    utils::virtual_call64<10, void, i8*>(this, val);
  }

  ALWAYSINLINE void set_value(i32 val){
    utils::virtual_call64<12, void, i32>(this, val);
  }

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
};

// !!!! SEEK ME !!!!
/*    
  Due to the ease of nature of anti-cheat development in garry's mod. It's very unwise to perform any mods to cvar flags or values.
  This will be easily detectable by anti-cheats alike.
*/  
class c_interface_cvar{
public:
  c_cvar* find_var(std::string name){
    return utils::virtual_call64<17, c_cvar*, i8*>(this, name.c_str());
  }

  void enforce_var(std::string name, std::string value){
    c_cvar* var = find_var(name);
    if(var == nullptr){
      DBG("[-] enforce_var unknown cvar %s\n", name.c_str());
      return;
    }

    if(var->flags & FCVAR_CHEAT || var->flags & FCVAR_HIDDEN || var->flags & FCVAR_DEVELOPMENTONLY){
      DBG("[-] enforce_var %s (bad flag) (gmod standards) not setting values\n", name.c_str());
      return;
    }

    i8* val = value.c_str();
    var->set_value(val);
  }
};