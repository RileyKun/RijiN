#include "link.h"

#define HASH(str)     fnv1a(str, sizeof(str) - 1)

// FNV1A - Runtime
u32 FNV1A_RT(const i8* buf, uptr len, u32 ret = 0x811C9DC5){
  return len <= 0 ? ret : fnv1a_rt(&buf[1], --len, (ret ^ buf[0]) * 0x01000193);
}

// FNV1A - Immediate Compile time
consteval u32 FNV1A(const i8* buf, uptr len, u32 ret = 0x811C9DC5){
  return len <= 0 ? ret : fnv1a(&buf[1], --len, (ret ^ buf[0]) * 0x01000193);
}

i32 main(i32 argc, const i8 *argv[]) {
  #define TEST_STR "test1234"

  printf("fnv1a32    %X\n", HASH_RT(TEST_STR));
  printf("fnv1a32_ct %X\n", HASH(TEST_STR));
  printf("%X\n", HASH_RT(TEST_STR));
  system("pause");


  return 0;
}
