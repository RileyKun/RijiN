#pragma once

#if defined(INTERNAL_CHEAT)
  #define NO_SIZE_PARA
  #define func_a wvsprintfA
  #define func_w wvsprintfW

  #define IDEAL_MAX_BUF_SIZE 1024
  #define IDEAL_MIN_BUF_SIZE 1024
  #define IDEAL_WORKING_BUF_SIZE 1024
#else
  #define func_a vsnprintf
  #define func_w vswprintf

  #define IDEAL_MAX_BUF_SIZE 1024
  #define IDEAL_MIN_BUF_SIZE 128
  #define IDEAL_WORKING_BUF_SIZE 8192

#endif

inline bool valid_parameters(void* buf, u32 size, void* fmt){
  if(buf == nullptr || fmt == nullptr || !size)
    return false;

  return true;
}

static bool format_a(i8* buf, u32 size, const i8* fmt, ...) {
  if(!valid_parameters(buf, size, fmt))
    return false;

  va_list args;
  va_start(args, fmt);
  #if !defined(NO_SIZE_PARA)
    func_a(buf, size, fmt, args);
  #else
    func_a(buf, fmt, args);
  #endif
  va_end(args);
  buf[size - 1] = '\0';

  return true;
}

static bool format_w(wchar_t* buf, u32 size, const wchar_t* fmt, ...) {
  if(!valid_parameters(buf, size, fmt))
    return false;

  va_list args;
  va_start(args, fmt);
  #if !defined(NO_SIZE_PARA)
    func_w(buf, size, fmt, args);
  #else
    func_w(buf, fmt, args);
  #endif
  va_end(args);
  buf[size - 1] = L'\0';

  return true;
}

static bool format_append_a(i8* buf, u32 size, const i8* fmt, ...){
  if(!valid_parameters(buf, size, fmt))
    return false;

  u32 buf_len = strlen(buf);
  if(buf_len >= size)
    return false;

  static i8 working_buf[IDEAL_WORKING_BUF_SIZE];
  memset(working_buf, 0, sizeof(working_buf));

  va_list args;
  va_start(args, fmt);
  #if !defined(NO_SIZE_PARA)
    func_a(working_buf, size, fmt, args);
  #else
    func_a(working_buf, fmt, args);
  #endif
  va_end(args);

  working_buf[IDEAL_WORKING_BUF_SIZE - 1] = '\0';
  const u32 working_buf_len = strlen(working_buf);

  i32 i = 0;
  while(buf_len < size && i < working_buf_len){
    buf[buf_len] = working_buf[i];
    buf_len++;
    i++;
  }

  buf[size - 1] = '\0';
  return true;
}

static bool format_append_w(wchar_t* buf, u32 size, const wchar_t* fmt, ...){
  if(!valid_parameters(buf, size, fmt))
    return false;

  u32 buf_len = wcslen(buf);
  if(buf_len >= size)
    return false;

  static wchar_t working_buf[IDEAL_WORKING_BUF_SIZE];
  memset(working_buf, 0, sizeof(working_buf));

  va_list args;
  va_start(args, fmt);
  #if !defined(NO_SIZE_PARA)
    func_w(working_buf, size, fmt, args);
  #else
    func_w(working_buf, fmt, args);
  #endif
  va_end(args);

  working_buf[IDEAL_WORKING_BUF_SIZE - 1] = '\0';
  const u32 working_buf_len = wcslen(working_buf);

  i32 i = 0;
  while(buf_len < size && i < working_buf_len){
    buf[buf_len] = working_buf[i];
    buf_len++;
    i++;
  }

  buf[size - 1] = L'\0';
  return true;
}

#define formatA(buf, fmt, ...) \
  (void)format_a(buf, sizeof(buf), fmt, ##__VA_ARGS__);

#define formatW(buf, fmt, ...) \
  (void)format_w(buf, sizeof(buf), fmt, ##__VA_ARGS__);

#define appendA(buf, fmt, ...) \
  (void)format_append_a(buf, sizeof(buf), fmt, ##__VA_ARGS__);

#define appendW(buf, fmt, ...) \
  (void)format_append_w(buf, sizeof(buf), fmt, ##__VA_ARGS__);






