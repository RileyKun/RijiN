#include "link.h"

#define I(i) i
#define XOR32(i) i
#define DBG printf

i32 main(i32 argc, const i8 *argv[]) {
  u32 size = 0;
  if(I(GetAdaptersInfo)(nullptr, &size) != XOR32(ERROR_BUFFER_OVERFLOW)){
    return 0;
  }

  IP_ADAPTER_INFO* adapter = I(VirtualAlloc)(nullptr, size, XOR32(MEM_COMMIT), XOR32(PAGE_READWRITE));
  if(I(GetAdaptersInfo)(adapter, &size) != ERROR_SUCCESS){
    return 0;
  }

  while(adapter != nullptr){
    if((adapter->Type == XOR32(IF_TYPE_IEEE80211) || adapter->Type == XOR32(MIB_IF_TYPE_ETHERNET)) && adapter->AddressLength == 6){
      DBG("[!] get_mac_address: %s\n", adapter->Description);
      DBG("[!] get_mac_address: %01X-%01X-%01X-%01X-%01X-%01X\n", adapter->Address[0], adapter->Address[1], adapter->Address[2], adapter->Address[3], adapter->Address[4], adapter->Address[5]);
      break;
    }

    adapter = adapter->Next;
  }

  system("pause");

  return 0;
}
