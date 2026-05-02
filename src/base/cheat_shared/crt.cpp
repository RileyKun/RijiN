#include "../link.h"

// This file rebuilds some crt related stuff

void __cdecl std::__throw_bad_alloc(){
  assert(false && "__throw_bad_alloc");
  I(ExitProcess)(0);
}

void __cdecl std::__throw_bad_function_call(){
  assert(false && "__throw_bad_function_call");
  I(ExitProcess)(0);
}

void __cdecl std::__throw_length_error(const i8*){
  assert(false && "__throw_length_error");
  I(ExitProcess)(0);
}

void __cdecl std::__throw_logic_error(const i8*){
  assert(false && "__throw_logic_error");
  I(ExitProcess)(0);
}

void __cdecl std::__throw_bad_array_new_length(){
  assert(false && "__throw_bad_array_new_length");
  I(ExitProcess)(0);
}

void __cdecl std::__throw_out_of_range_fmt(const i8*, ...){
  assert(false && "__throw_out_of_range_fmt");
  I(ExitProcess)(0);
}

extern "C" wchar_t* wmemset(wchar_t* s, wchar_t c, size_t n) {
  for (size_t i = 0; i < n; ++i)
    s[i] = c;

  return s;
}

#if defined(RENDERING_MODE_D3D9_NEW)
extern "C" void atexit(void (*func)(void)){

}
#endif