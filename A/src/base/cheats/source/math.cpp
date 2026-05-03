#include "../../link.h"
#include "math.h"

void math::set_random_seed(u32 seed){
  static void* ptr = utils::get_proc_address(utils::get_module_handle(HASH("vstdlib.dll")), HASH("RandomSeed"));
  #if defined(__x86_64__)
    utils::call_fastcall64_raw<void, u32>(ptr, seed);
  #else
    utils::call_cdecl<void, u32>(ptr, seed);
  #endif
}

i32 math::random_int(i32 min, i32 max){
  static void* ptr = utils::get_proc_address(utils::get_module_handle(HASH("vstdlib.dll")), HASH("RandomInt"));
  #if defined(__x86_64__)
    return utils::call_fastcall64_raw<i32, i32, i32>(ptr, min, max);
  #else
    return utils::call_cdecl<i32, i32, i32>(ptr, min, max);
  #endif
}

float math::random_float(float min, float max){
  static void* ptr = utils::get_proc_address(utils::get_module_handle(HASH("vstdlib.dll")), HASH("RandomFloat"));
  #if defined(__x86_64__)
    return utils::call_fastcall64_raw<float, float, float>(ptr, min, max);
  #else
    return utils::call_cdecl<float, float, float>(ptr, min, max);
  #endif
}

void math::setup_random_seed(i32 num, i32 seed, bool bad_call){
  if(num <= 0)
    return;

  cookie_block_check_return();

  if(bad_call){
    assert(false && "Mistake, CBaseWeapon version should be better, check tf_win64!");
    return;
  }

  if(global_internal->prediction_random_seed == nullptr)
    return;
  
  if(*global_internal->prediction_random_seed == -1 && seed == -1){
    DBG("[-] math::setup_random_seed prediction_random_seed needs to be not -1\n");
    return;
  }

  const i32 old_seed = *global_internal->prediction_random_seed;

  if(seed != -1)
    *global_internal->prediction_random_seed = seed;

  // I don't know why, and I'll never know why, but for some reason GCC kept optimizing this string out with random shit
  // This fixed it.
  static i8 buf[32];
  static bool init_buf = false;
  if(!init_buf){
    wsprintfA(buf, XOR("SelectWeightedSequence"));
    init_buf = true;
  }

  math::shared_random_int(buf, 0, 0, 0);

  for(i32 i = 0; i < num; i++)
    math::random_float(0.f, 1.f);

  *global_internal->prediction_random_seed = old_seed;
}

i32 math::shared_random_int(i8* str, i32 min, i32 max, i32 exp){
#if defined(__x86_64__)
  return utils::call_fastcall64_raw<i32, i8*, i32, i32, i32>(global_internal->shared_random_int_addr, str, min, max, exp);
#else
  return utils::call_cdecl<i32, i8*, i32, i32, i32>(global_internal->shared_random_int_addr, str, min, max, exp);
#endif
}