#pragma once

// https://en.wikipedia.org/wiki/Fowler–Noll–Vo hash function
#define HASH_RT(str)   FNV1A_STR_RT(str)
#define HASH(str)      FNV1A_STR_CT(str)
#define WHASH_RT(wstr) FNV1A_WSTR_RT(wstr)
#define WHASH(wstr)    FNV1A_WSTR_CT(wstr)

// Macro to handle the lowercase
#define FNV1A_HANDLE_LOWER_CASE(c) (lowercase ? (c >= 'A' && c <= 'Z' ? (c | (1 << 5)) : c) : c)

// FNV1A_STR_RT - Runtime
constexpr u32 FNV1A_STR_RT(i8* buf, bool lowercase = false, u32 ret = 0x811C9DC5){
  return buf[0] == 0 ? ret : FNV1A_STR_RT(&buf[1], lowercase, (ret ^ FNV1A_HANDLE_LOWER_CASE(buf[0])) * 0x01000193);
}

// FNV1A_STR_CT - Immediate Compile time
consteval u32 FNV1A_STR_CT(i8* buf, bool lowercase = false, u32 ret = 0x811C9DC5){
  return buf[0] == 0 ? ret : FNV1A_STR_CT(&buf[1], lowercase, (ret ^ FNV1A_HANDLE_LOWER_CASE(buf[0])) * 0x01000193);
}

// FNV1A_WSTR_RT - Runtime
constexpr u32 FNV1A_WSTR_RT(wchar_t* buf, bool lowercase = false, u32 ret = 0x811C9DC5){
  return buf[0] == 0 ? ret : FNV1A_WSTR_RT(&buf[1], lowercase, (ret ^ FNV1A_HANDLE_LOWER_CASE(buf[0])) * 0x01000193);
}

// FNV1A_WSTR_CT - Immediate Compile time
consteval u32 FNV1A_WSTR_CT(wchar_t* buf, bool lowercase = false, u32 ret = 0x811C9DC5){
  return buf[0] == 0 ? ret : FNV1A_WSTR_CT(&buf[1], lowercase, (ret ^ FNV1A_HANDLE_LOWER_CASE(buf[0])) * 0x01000193);
}

// FNV1A - Runtime
constexpr u32 FNV1A_RT(i8* buf, uptr len, bool lowercase = false, u32 ret = 0x811C9DC5){
  return len <= 0 ? ret : FNV1A_RT(&buf[1], --len, lowercase, (ret ^ FNV1A_HANDLE_LOWER_CASE(buf[0])) * 0x01000193);
}

// FNV1A - Immediate Compile time
consteval u32 FNV1A_CT(i8* buf, uptr len, bool lowercase = false, u32 ret = 0x811C9DC5){
  return len <= 0 ? ret : FNV1A_CT(&buf[1], --len, lowercase, (ret ^ FNV1A_HANDLE_LOWER_CASE(buf[0])) * 0x01000193);
}