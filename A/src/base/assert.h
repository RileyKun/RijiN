#pragma once

#if defined(DEV_MODE)
  EXTERN void assert_internal(i8* exp, i8* file, i8* func, u32 line);

  #define int3() asm("int $3");
  #define assert(exp) {if((exp) != true){ assert_internal(#exp, __FILE__, __PRETTY_FUNCTION__, __LINE__); int3(); }}
  #define assert_private(exp) {if((exp) != true){ assert_internal(#exp, __FILE__, __PRETTY_FUNCTION__, __LINE__); int3(); }}
#elif defined(STAGING_MODE) && defined(INTERNAL_CHEAT)
  EXTERN void assert_internal(i8* exp, i8* file, i8* func, u32 line);

  #define int3() asm("int $3");
  #define assert(exp) {if((exp) != true){ assert_internal(XOR(#exp), "", "", __LINE__); int3(); }}

  #define assert_private(exp) {if((exp) != true){}}
#else
  // TODO: figure out how we can make this terminate the function like a return
  #define assert(exp) {if((exp) != true){}}
  #define assert_private(exp) {if((exp) != true){}}
  #define int3() {}
#endif