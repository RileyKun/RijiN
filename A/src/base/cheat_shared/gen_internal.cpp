#include "../link.h"

#if defined(GEN_INTERNAL)
EXPORT uptr c_gen_internal::get_pkg(u32 hash){
  VM_ULTRA_FAST_START()
  {
    for(u32 i = 0; i < package_count; i++){
      s_gen_pkg pkg = packages[i];

      if(pkg.hash == hash)
        return (uptr)pkg.ptr;
    }
  }
  VM_ULTRA_FAST_STOP()

  //assert(false);
  return 0;
}

EXPORT CLASS_ALLOC(c_gen_internal, gen_internal);
#endif