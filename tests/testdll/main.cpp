#include "link.h"

EXPORT bool WINAPI DllMain(void* dll, u32 reason, void* reverse) {

  MessageBoxA(nullptr, "test", "test", 0);

  //system("pause");

  return true;
}
