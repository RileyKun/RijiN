#pragma once

#if !defined(SOURCE_CHEAT)
  #if defined(DEV_MODE)
    #if defined(DEBUG_USE_PRINTF)
      #define DBG(f_, ...) \
      { \
        std::string str = f_; \
        if(str[1] == '!') \
          SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xE); \
        else if(str[1] == '-') \
          SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x0C); \
        else \
          SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x0F); \
        printf(str.c_str(), ##__VA_ARGS__); \
      }
    #else
      #define DBG(...){}
    #endif
  #else
    #define DBG(...){}
  #endif
#else
  #if defined(DEV_MODE)
  #if defined(__x86_64__)
    typedef void (__fastcall* warning_fn)(const i8* text, ...);
  #else
    typedef void (__cdecl* warning_fn)(const i8* text, ...);
  #endif

  // I've done the warning function like this to make debugging less annoying in dev mode.
  static warning_fn get_warning_fn() {
    static uptr tier0 = 0;
    static uptr warning_ptr = 0;
    static warning_fn warning = nullptr;

    if(!warning){
      tier0 = (uptr)GetModuleHandleA("tier0.dll");
      if(tier0){
        warning_ptr = (uptr)GetProcAddress((void*)tier0, "Warning");
        warning = (warning_fn)warning_ptr;
      }
    }

    return warning;
  }

  #define DBG(_f, ...) { \
    get_warning_fn()(_f, ##__VA_ARGS__); \
  }

  #define DBG_STAGING(_f, ...) { \
    get_warning_fn()(_f, ##__VA_ARGS__); \
  }
  #else
    #define DBG(_f, ...) {}
    #define DBG_STAGING(_f, ...) {}
  #endif
#endif

#define DBG_IF_NULL(x, args, ...) { \
  if(x == NULL){ \
    DBG(args, ##__VA_ARGS__); \
  } \
} \


