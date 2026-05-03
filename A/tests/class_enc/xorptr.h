#pragma once

#if !defined(DEBUG_MODE)

#define XOR32(x) u32_dec(u32_enc(x, __LINE__), __LINE__)
#define XOR64(x) u64_dec(u64_enc(x, __LINE__), __LINE__)

#if !defined(XORPTR_H)
#define XORPTR_H

consteval u32 u32_enc(u32 ptr, u32 line){

  if(ptr == 0)
    return 0;

  u32 hash = CRC32C(__TIME__, sizeof(__TIME__)) ^ line;
  u32 strength = 8 + (line % 13);

  for(auto i = 0; i < strength; i++)
    ptr ^= (hash*i) % UINT_MAX;

  for(auto i = 0; i < strength; i++)
    ptr ^= UINT_MAX - (i*2);

  return ptr;
}

inline ALWAYSINLINE u32 u32_dec(u32 ptr, u32 line){

  if(ptr == 0)
    return 0;

  u32 hash = CRC32C(__TIME__, sizeof(__TIME__)) ^ line;
  u32 strength = 8 + (line % 13);

  for(auto i = 0; i < strength; i++)
    ptr ^= UINT_MAX - (i*2);

  for(auto i = 0; i < strength; i++)
    ptr ^= (hash*i) % UINT_MAX;

  return ptr;
}

inline __attribute__((always_inline)) u64 u64_dec(u64 ptr, u64 line){

  if(ptr == 0)
    return 0;

  u64 hash = CRC32C(__TIME__, sizeof(__TIME__));
  u64 strength = 8 + (line % 13);

  for(auto i = 0; i < strength; i++)
    ptr ^= UINT_MAX - (i*2);

  for(auto i = 0; i < strength; i++)
    ptr ^= (hash*i) % UINT_MAX;

  return ptr;
}

consteval u64 u64_enc(u64 ptr, u64 line){

  if(ptr == 0)
    return 0;

  u64 hash = CRC32C(__TIME__, sizeof(__TIME__));
  u64 strength = 8 + (line % 13);

  for(auto i = 0; i < strength; i++)
    ptr ^= (hash*i) % UINT_MAX;

  for(auto i = 0; i < strength; i++)
    ptr ^= UINT_MAX - (i*2);

  return ptr;
}

#endif

#else
#define XOR32(x) x
#define XOR64(x) x
#endif