#include "link.h"

u32 decrypt_asset(u32 asset){
  asset ^= *(u32*)(0x7FFE0330);

  return asset;
}

gen_internal->decrypt_asset(testblala)

i32 main(i32 argc, const i8 *argv[]) {
  printf("cookie: %X\n", *(u32*)(0x7FFE0330));

#if defined(__x86_64__)
  __PEB* peb = (__PEB*)__readgsqword( 0x60 );
#else
  __PEB* peb = (__PEB*)__readfsdword( 0x30 );
#endif

  printf("peb: %X\n", peb);
  printf("peb: %X\n", peb->ProcessHeap);

  system("pause");

  return 0;
}
