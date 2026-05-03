#include "link.h"
#include "RCC.h"

void print_bytes(u8* str, u8* bytes, uptr bytes_len){
  printf("%s: ", str);

  for(i32 i = 0; i < bytes_len; i++)
    printf("%X ", bytes[i]);

  printf("\n");
}

i32 main(i32 argc, const i8 *argv[]) {
  u8 str[] = "This is a bass test dub!!boss!!!";
  u8 key1[] = "e5a66d4f14c2d2fbea0857a54b4a22aa";
  u8 key2[] = "e5a66d4f14c2d2fbea0857a54b4a22aa";
  u8 key3[] = "e5a66d4f14c2d2fbea0857a54b4a22aa";

  print_bytes("key", key1, sizeof(key1));

  u32 recursions = 100000;
  c_rcc::encrypt(str, sizeof(str), key1, sizeof(key1), recursions);

  print_bytes("str", str, sizeof(str));
  print_bytes("key1", key1, sizeof(key1));

  c_rcc::decrypt(str, sizeof(str), key2, sizeof(key2), recursions);

  printf("dec: \"%s\"\n", str);
  print_bytes("key2", key2, sizeof(key2));

  system("pause");

  return 0;
}
