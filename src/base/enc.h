#pragma once
#include <bcrypt.h>

#define ENCRYPTION_KEY XOR("ea4f761c42a8d98ca59572ce4d0f308999025de596b1078aafa1b3a2b9af3014")
#define ENCRYPTION_KEY_SIZE 64
#define NONCE_SIZE 32



/*
  This is by no means a secure encryption, but it's enough to stop people from being able to
  tamper and possibly reverse engineer our networking easily.

  If you want to take the time to make it actually impossible to do anything networking related, then do so.
*/
#define ENC_INLINE ALWAYSINLINE inline CFLAG_O0

#define ENC_VM_START()
#define ENC_VM_STOP()

static u64 s[2];

ALWAYSINLINE inline void rand_seed(){
  u64 time = __rdtsc();
  u64 ptr  = (u64)&time;

  s[0] = time ^ (ptr << 13);
  s[1] = (ptr >> 7) ^ (time << 17);
}

ALWAYSINLINE inline u64 rotl(const u64 x, int k){
  return (x << k) | (x >> (64 - k));
}

ALWAYSINLINE inline u64 xoroshiro128plus(){
  const u64 s0 = s[0];
  u64 s1 = s[1];
  const u64 result = s0 + s1;

  s1 ^= s0;
  s[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14);
  s[1] = rotl(s1, 36);

  return result;
}

ENC_INLINE void write_nonce(u8* nonce){
  ENC_VM_START();
  rand_seed();
  for(i32 i = 0; i < 32; i += 8){
    u64 r = xoroshiro128plus();
    *(u64*)(nonce + i) = r;
  }
  ENC_VM_STOP();
}

ENC_INLINE void encrypt_buffer(u8* buffer, const u32 len, u8* nonce){
  ENC_VM_START();

  u8 der_key[ENCRYPTION_KEY_SIZE];
  for(i32 i = 0; i < XOR32_IMPORTANT_ONLY(ENCRYPTION_KEY_SIZE); i++)
    der_key[i] = ENCRYPTION_KEY[i] ^ nonce[i % NONCE_SIZE];

  for(i32 i = 0; i < len; i++)
    buffer[i] ^= der_key[i % ENCRYPTION_KEY_SIZE];

  ENC_VM_STOP();
}

ENC_INLINE void decrypt_buffer(u8* buffer, const u32 len, u8* nonce){
  ENC_VM_START();

  u8 der_key[ENCRYPTION_KEY_SIZE];
  for(i32 i = 0; i < XOR32_IMPORTANT_ONLY(ENCRYPTION_KEY_SIZE); i++)
    der_key[i] = ENCRYPTION_KEY[i] ^ nonce[i % NONCE_SIZE];

  for(i32 i = len - 1; i >= 0; i--)
    buffer[i] ^= der_key[i % ENCRYPTION_KEY_SIZE];

  ENC_VM_STOP();
}

ENC_INLINE void cookie_xor(u8* buffer, const u32 len){
  ENC_VM_START();
  for(i32 i = 0; i < len; i++){
    buffer[i] ^= *(volatile u32*)(XOR32_IMPORTANT_ONLY(0x7FFE0330));
  }

  ENC_VM_STOP();
}