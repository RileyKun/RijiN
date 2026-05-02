/*
  Copyright (c) 2021, Kieran Watkins
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. All advertising materials mentioning features or use of this software
     must display the following acknowledgement:
     This product includes software developed by the <organization>.
  4. Neither the name of the <organization> nor the
     names of its contributors may be used to endorse or promote products
     derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ''AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "crc32.h"

#define RCC_MATH_SMALLEST(x, y)         (x > y ? y : x)
#define RCC_FUNC_MALLOC(len)            malloc(len)
#define RCC_FUNC_FREE(ptr)              free(ptr)
#define RCC_FUNC_MEMCPY(dest, src, len) memcpy(dest, src, len)

enum e_rcc_result{
  RCC_SUCCESS = 0,
  RCC_ERROR   = 1
};

class c_rcc{
public:
  // Changes the key used by hashing the key in sections, and replacing the key with those hashes
  // Calling this function again will mutate the hashes and therefor creating a completely new key on every call
  static e_rcc_result mutate_key(u8* key, uptr key_len, u8* output_key_to = nullptr){
    for(u64 i = 0; i < key_len; i += RCC_MATH_SMALLEST(key_len - i, 4)){
      u32 hash = CRC32(key + i, RCC_MATH_SMALLEST(key_len - i, 4));

      // Copy the new hash of the key to the key buffer
      RCC_FUNC_MEMCPY((output_key_to != nullptr ? output_key_to : key) + i, &hash, RCC_MATH_SMALLEST(key_len - i, 4));
    }

    return RCC_SUCCESS;
  }

  // Generates all the mutated keys once before decryption is started
  // We have to do this because in order to decrypt we need to decrypt the key from back to front
  // We cant know the key before the current key unless we know the last key, this goes back until our original key
  static u8* get_mutated_key_table(u8* key, uptr key_len, uptr rounds){
    u8* key_table = RCC_FUNC_MALLOC(rounds * key_len);

    if(key_table == nullptr)
      return nullptr;

    for(u64 i = 0; i < rounds * key_len; i += key_len)
      mutate_key(i == 0 ? key : key_table + (i - key_len), key_len, key_table + i);

    return key_table;
  }

  static e_rcc_result encrypt(u8* buffer, uptr buffer_len, u8* key, uptr key_len, u32 rounds){
    // Cut the key depending on the size of the buffer, we aren't going to use it
    key_len             = RCC_MATH_SMALLEST(key_len, buffer_len);
    u32 last_key_part   = 0;

    mutate_key(key, key_len);
    for(uptr i = 0; i < buffer_len; i++){
      u8 key_byte   = key[i % key_len] > 0 ? key[i % key_len] : 1;
      u8 key_part   = buffer[i] ^ key_byte;

      // Creates a encrypted buffer based on (part[i]^part[i-1])
      // This cannot be done on the first round, making this considered insecure on 1 round encryption
      buffer[i]     = i == 0 ? (buffer[i] ^ key_byte) : (key_part ^ last_key_part);
      last_key_part = key_part;
    }

    while(rounds-- > 0)
      encrypt(buffer, buffer_len, key, key_len, 0);

    return RCC_SUCCESS;
  }

  static e_rcc_result decrypt(u8* buffer, uptr buffer_len, u8* key, uptr key_len, u32& rounds, u8* key_table = nullptr){
    // Cut the key depending on the size of the buffer, we aren't going to use it
    key_len     = RCC_MATH_SMALLEST(key_len, buffer_len);
    u8* old_key = key;

    // If we are decrypting based on rounds, we need to generate a full mutated key table
    if(rounds > 0 && key_table == nullptr)
      if((key_table = get_mutated_key_table(key, key_len, rounds + 1)) == nullptr)
        return RCC_ERROR;

    if(key_table == nullptr)
      mutate_key(key, key_len);
    else
      key = key_table + (key_len * rounds);

    u32 last_key_part = 0;
    for(uptr i = 0; i < buffer_len; i++){
      u8 key_part   = (i == 0) ? buffer[i] : buffer[i] ^ last_key_part;

      // Decrypts the encrypted buffer by ((enc[i] ^ last_key_part) ^ key[i%key_len])
      buffer[i]     = key_part ^ (key[i % key_len] > 0 ? key[i % key_len] : 1);
      last_key_part = key_part;
    }

    if(rounds-- > 0)
      decrypt(buffer, buffer_len, old_key, key_len, rounds, key_table);
    else if(key_table != nullptr)
      RCC_FUNC_FREE(key_table);

    return RCC_SUCCESS;
  }
};