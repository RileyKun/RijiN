#pragma once

#define STATUS_SUCCESS 0
#define STATUS_PARTIAL_COPY  0x8000000D
#define STATUS_ACCESS_DENIED 0xC0000022

typedef char               i8;
typedef short              i16;
typedef int                i32;
typedef long long          i64;
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long      ul64;
typedef unsigned long long u64;

#if defined(__x86_64__)
typedef ul64 uword;
typedef u64  uptr;
typedef i64  iptr;
#else
typedef u32 uword;
typedef u32 uptr;
typedef i32 iptr;
#endif