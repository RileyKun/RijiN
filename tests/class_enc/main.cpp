#include "link.h"

class c_test_class{
public:
  i32 test_ptr = 0;
};

ALWAYSINLINE void* enc_test(void* ptr){
  uptr cookie = *(uptr*)XOR32(0x7FFE0000 + 0x330/*Cookie*/);

  return (void*)((uptr)ptr ^ cookie);
}

EXPORT i32 main(i32 argc, const i8 *argv[]) {

  c_test_class* enc_test_class = enc_test(new c_test_class);

  ((c_test_class*)enc_test(enc_test_class))->test_ptr = 1488;
  printf("%i\n", ((c_test_class*)enc_test(enc_test_class))->test_ptr);

  system("pause");

  return 0;
}
