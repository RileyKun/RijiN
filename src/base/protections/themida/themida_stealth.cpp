#include "../../link.h"

#if defined(ANTI_REVERSE_VM_ENGINE)
template<u32 size>
struct STEALTH_AREA{
  u32 start[4];
  u8  buffer[size];
  u32 end[4];
};

#define KB 1000
#if defined(LOADER_CLIENT)
  #define TH_TEXT_SIZE KB * 9000
  #define TH_DATA_SIZE KB * 64
#else
  #define TH_TEXT_SIZE KB * 6000
  #define TH_DATA_SIZE KB * 64
#endif

extern "C" {
  __attribute__((section(".text"))) STEALTH_AREA<TH_TEXT_SIZE> stealth_code_area = {
    {0xA1A2A3A4, 0xA4A3A2A1, 0xB1A1B2A2, 0xB8A8A1A1},
    {0},
    {0xB6B5B3B6, 0xA2B2C2D2, 0xA9A8A2A2, 0xA0A9B9B8}
  };

  __attribute__((section(".data"))) STEALTH_AREA<TH_DATA_SIZE> stealth_data_area = {
    {0xA5A6A7A8, 0xA8A7A6A5, 0xB5A5B6A6, 0xB7A7A5A5},
    {0},
    {0xB8B7B6B5, 0xA6B6C6D6, 0xA7A6A4A4, 0xA4A5B1B2}
  };
}

// trick compiler into referencing code so that its initialized
void we_advise_you_stop(){
  stealth_code_area.buffer[0] = 1;
  stealth_data_area.buffer[0] = 1;
}
#endif