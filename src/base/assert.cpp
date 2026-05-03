#include "link.h"

#if defined(DEV_MODE) || defined(STAGING_MODE) && defined(INTERNAL_CHEAT)

  static i8 assert_buf[8912]{};
  static i8 assert_last_error_buf[8912]{};
  EXPORT CFLAG_O0 void assert_internal(i8* exp, i8* file, i8* func, u32 line){
    if(exp != nullptr && file != nullptr && func != nullptr){
      format_a(assert_buf, sizeof(assert_buf), XOR("Error\nFile: %s\nFunction: %s\nCode: %s\nLine: %i\n"), file, func, exp, line);
      MessageBoxA(nullptr, assert_buf, XOR("Assertion Failed"), MB_ICONERROR | MB_SETFOREGROUND);
    }
  }
#endif