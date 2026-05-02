#include "../link.h"

static c_keyvalues* c_keyvalues::create(i8* name){
  c_keyvalues* kv = utils::call_cdecl<c_keyvalues*, i32>(global->keyvalues_init, 32);

  if(kv == nullptr){
    assert(kv != nullptr);
    return nullptr;
  }

  if(global->keyvalues_setname == nullptr){
    assert(global->keyvalues_setname != nullptr);
    return nullptr;
  }

  // setname rebuilt
  *(u32*)(kv + 0x4)  = 0;
  *(u32*)(kv + 0x8)  = 0;
  *(u32*)(kv + 0xC)  = 0;
  *(u32*)(kv + 0x10) = XOR32(0x10000);
  *(u32*)(kv + 0x14) = 0;
  *(u32*)(kv + 0x18) = 0;
  *(u32*)(kv + 0x1C) = 0;

  u32 symbol_name = **(u32**)(global->keyvalues_setname + 0x2);
  *(u32*)(kv) = utils::call_cdecl<i32, u8*, bool>((void*)symbol_name, name, true);

  return kv;
}

void c_keyvalues::set_int(i8* name, i32 value){
  if(global->keyvalues_setint == nullptr){
    assert(global->keyvalues_setint != nullptr);
    return;
  }

  utils::call_thiscall<void, u8*, i32>(global->keyvalues_setint, this, name, value);
}

void c_keyvalues::set_bool(i8* name, bool value){
  set_int(name, value ? 1 : 0);
}

void c_keyvalues::set_float(i8* name, float value){
  if(global->keyvalues_setfloat == nullptr){
    assert(global->keyvalues_setfloat != nullptr);
    return;
  }

  utils::call_thiscall<void, u8*, float>(global->keyvalues_setfloat, this, name, value);
}

void c_keyvalues::set_string(i8* name, i8* value){
  if(global->keyvalues_setstring == nullptr){
    assert(global->keyvalues_setstring != nullptr);
    return;
  }

  utils::call_thiscall<void, u8*, i8*>(global->keyvalues_setstring, this, name, value);
}

void c_keyvalues::transmit(){
  global->engine->send_keyvalue(this);
}