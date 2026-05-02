#include "../link.h"

static c_keyvalues* c_keyvalues::create(i8* name){
  c_keyvalues* kv = utils::call_fastcall64<c_keyvalues*>(global->keyvalues_init, (void*)72);

  if(kv == nullptr){
    assert(kv != nullptr);
    return nullptr;
  }

  if(global->keyvalues_setname == nullptr){
    assert(global->keyvalues_setname != nullptr);
    return nullptr;
  }

  kv = utils::call_fastcall64<c_keyvalues*, i8*>(global->keyvalues_setname, kv, name);

  DBG("[!] Created keyvalue: %s (%p)\n", name, kv);
  return kv;
}

void c_keyvalues::set_int(i8* name, i32 value){
  if(global->keyvalues_setint == nullptr){
    assert(global->keyvalues_setint != nullptr);
    return;
  }

  utils::call_fastcall64<void, u8*, i32>(global->keyvalues_setint, this, name, value);
}

void c_keyvalues::set_bool(i8* name, bool value){
  set_int(name, value ? 1 : 0);
}

void c_keyvalues::set_float(i8* name, float value){
  if(global->keyvalues_setfloat == nullptr){
    assert(global->keyvalues_setfloat != nullptr);
    return;
  }

  utils::call_fastcall64<void, u8*, float>(global->keyvalues_setfloat, this, name, value);
}

void c_keyvalues::set_string(i8* name, i8* value){
  if(global->keyvalues_setstring == nullptr){
    assert(global->keyvalues_setstring != nullptr);
    return;
  }

  utils::call_fastcall64<void, u8*, i8*>(global->keyvalues_setstring, this, name, value);
}

void c_keyvalues::transmit(){
  global->engine->send_keyvalue(this);
}