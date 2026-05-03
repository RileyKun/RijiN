#include "../../../link.h"

c_keyvalues* c_keyvalues::create(i8* name){
#if defined(__x86_64__)
  #if defined(TF2_CHEAT)
  	c_keyvalues* kv = utils::call_fastcall64<c_keyvalues*>(global_internal->keyvalues_init, (void*)64);
	
  	if(kv == nullptr){
  	  assert(kv != nullptr);
  	  return nullptr;
  	}
	
  	if(global_internal->keyvalues_setname == nullptr){
  	  assert(global_internal->keyvalues_setname != nullptr);
  	  return nullptr;
  	}
	
  	// setname rebuilt
  	*(u8*)(kv   + 0x20)  = 0;
  	*(uptr*)(kv + 0x30)  = 0;
  	*(uptr*)(kv + 0x28)  = 0;
  	*(uptr*)(kv + 0x38)  = 0;
  	*(uptr*)(kv + 8)     = 0;
  	*(uptr*)(kv + 0x10)  = 0;
  	*(uptr*)(kv + 0x18)  = 0;
  	*(i32*)(kv  + 0x21)  = 0x100;
  	*(i8*)(kv   + 0x23)  = 0;
	
  	uptr* symbol_name = *(uptr**)((uptr)global_internal->keyvalues_setname);
  	*(uptr*)(kv) = utils::call_fastcall64<uptr, bool>(symbol_name, name, true);
	
  	DBG("[!] Created keyvalue: %s (%p)\n", name, kv);
  	return kv;
  #else
  	c_keyvalues* kv = utils::call_fastcall64<c_keyvalues*>(global_internal->keyvalues_init, (void*)72);
	
  	if(kv == nullptr){
  	  assert(kv != nullptr);
  	  return nullptr;
  	}
	
  	if(global_internal->keyvalues_setname == nullptr){
  	  assert(global_internal->keyvalues_setname != nullptr);
  	  return nullptr;
  	}
	
  	kv = utils::call_fastcall64<c_keyvalues*, i8*>(global_internal->keyvalues_setname, kv, name);
	
  	DBG("[!] Created keyvalue: %s (%p)\n", name, kv);
  	return kv;
  #endif
#else
  #if !defined(SOURCE_2018)
    c_keyvalues* kv = utils::call_cdecl<c_keyvalues*, i32>(global_internal->keyvalues_init, 32);
    if(kv == nullptr){
      assert(kv != nullptr);
      return nullptr;
    }

    if(global_internal->keyvalues_setname == nullptr){
      assert(global_internal->keyvalues_setname != nullptr);
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

    u32 symbol_name = **(u32**)(global_internal->keyvalues_setname + 0x2);
    *(u32*)(kv) = utils::call_cdecl<i32, i8*, bool>((void*)symbol_name, name, true);
  #else
    c_keyvalues* kv = utils::call_cdecl<c_keyvalues*, i32>(global_internal->keyvalues_init, 44);
    if(kv == nullptr){
      assert(kv != nullptr);
      return nullptr;
    }

    if(global_internal->keyvalues_setname == nullptr){
      assert(global_internal->keyvalues_setname != nullptr);
      return nullptr;
    }

    utils::call_thiscall<void, i8*, i8*, bool>(global_internal->keyvalues_setname, kv, name, NULL, false);
  #endif
  return kv;
#endif
}

void c_keyvalues::set_int(i8* name, i32 value){
  if(global_internal->keyvalues_setint == nullptr){
    assert(global_internal->keyvalues_setint != nullptr);
    return;
  }

 #if defined(__x86_64__)
  utils::call_fastcall64<void, i8*, i32>(global_internal->keyvalues_setint, this, name, value);
 #else
  utils::call_thiscall<void, i8*, i32>(global_internal->keyvalues_setint, this, name, value);
 #endif
}

void c_keyvalues::set_bool(i8* name, bool value){
  set_int(name, value ? 1 : 0);
}

void c_keyvalues::set_float(i8* name, float value){
  if(global_internal->keyvalues_setfloat == nullptr){
    assert(global_internal->keyvalues_setfloat != nullptr);
    return;
  }

 #if defined(__x86_64__)
  utils::call_fastcall64<void, i8*, float>(global_internal->keyvalues_setfloat, this, name, value);
 #else
  utils::call_thiscall<void, i8*, float>(global_internal->keyvalues_setfloat, this, name, value);
 #endif
}

void c_keyvalues::set_string(i8* name, i8* value){
  if(global_internal->keyvalues_setstring == nullptr){
    assert(global_internal->keyvalues_setstring != nullptr);
    return;
  }

#if defined(__x86_64__)
  utils::call_fastcall64<void, i8*, i8*>(global_internal->keyvalues_setstring, this, name, value);
#else
  utils::call_thiscall<void, i8*, i8*>(global_internal->keyvalues_setstring, this, name, value);
#endif
}

void c_keyvalues::transmit(){
  assert(global_internal->engine != nullptr);
  global_internal->engine->send_keyvalue(this);
}