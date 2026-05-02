#include "link.h"

CLASS_ALLOC(c_hwid, hwid);


bool c_hwid::is_invalid_network_driver(i8* desc) {
    if (desc == nullptr)
        return true;

    desc = str_utils::tolower(desc);
    if (   str_utils::contains(desc, XOR("bluetooth"))
        || str_utils::contains(desc, XOR("miniport"))
        || str_utils::contains(desc, XOR("hamachi"))
        || str_utils::contains(desc, XOR("virtual"))
        || str_utils::contains(desc, XOR("private internet"))
        || str_utils::contains(desc, XOR("guard"))
        || str_utils::contains(desc, XOR("vpn"))
        || str_utils::contains(desc, XOR("test"))
        || str_utils::contains(desc, XOR("loopback"))
        || str_utils::contains(desc, XOR("vbox"))
        || str_utils::contains(desc, XOR("vmware"))
        || str_utils::contains(desc, XOR("hyper-v"))
        || str_utils::contains(desc, XOR("wsl"))
        || str_utils::contains(desc, XOR("tap"))
        || str_utils::contains(desc, XOR("tun"))
        || str_utils::contains(desc, XOR("aws"))
        || str_utils::contains(desc, XOR("azure"))
        || str_utils::contains(desc, XOR("gcp"))
        || str_utils::contains(desc, XOR("google cloud"))
        || str_utils::contains(desc, XOR("oracle cloud"))
        || str_utils::contains(desc, XOR("isdn"))
        || str_utils::contains(desc, XOR("ppp"))
        || str_utils::contains(desc, XOR("team"))
        || str_utils::contains(desc, XOR("bond"))
        || str_utils::contains(desc, XOR("bridge")))
        return true;

    return false;
}

bool c_hwid::get_mac_address(s_mac_address_info& info){
  u32 size = 0;
  u32 result = I(GetAdaptersInfo)(nullptr, &size);
  if(result != XOR32(ERROR_BUFFER_OVERFLOW)){
    SHOW_ERROR_STATUS_STR(ERR_GET_ARP_GET_ADAPTER_FAILED, result, "Pre setup failed.", true);
    return false;
  }

  if(!size){
    SHOW_ERROR(ERR_GET_ARP_NULL_SIZE, true);
    return false;
  }

  IP_ADAPTER_INFO* adapter = I(VirtualAlloc)(nullptr, size, XOR32(MEM_COMMIT), XOR32(PAGE_READWRITE));
  if(adapter == nullptr){
    SHOW_ERROR_STATUS(ERR_GET_ARP_ALLOC_FAILED, I(GetLastError)(), true);
    return false;
  }

  // Actually get the adapter data now.
  result = I(GetAdaptersInfo)(adapter, &size);
  if(result != ERROR_SUCCESS){
    SHOW_ERROR_STATUS(ERR_GET_ARP_GET_ADAPTER_FAILED, result, true);
    return false;
  }

  IP_ADAPTER_INFO* ethernet = adapter;
  IP_ADAPTER_INFO* wireless = adapter;
  IP_ADAPTER_INFO* other = adapter;

  // Try and fetch the ethernet drivers only.
  while(ethernet != nullptr){
    if(is_bad_mac_address(other)
      || is_invalid_network_driver(ethernet->Description)
      || is_invalid_network_driver_type(ethernet->Type, ethernet->AddressLength)
      || ethernet->Type != XOR32(MIB_IF_TYPE_ETHERNET)){
      ethernet = ethernet->Next;
      continue;
    }

    info.hash = FNV1A_RT(&ethernet->Address[0], ethernet->AddressLength);
    DBG("[!] get_mac_address (MIB_IF_TYPE_ETHERNET): \"%s\"\n", ethernet->Description);
    DBG("[!] get_mac_address (MIB_IF_TYPE_ETHERNET): %01X-%01X-%01X-%01X-%01X-%01X (0x%08X)\n", ethernet->Address[0], ethernet->Address[1], ethernet->Address[2], ethernet->Address[3], ethernet->Address[4], ethernet->Address[5], info.hash);
    if(info.hash != 0)
      return true;

    ethernet = ethernet->Next;
  }

  // Try non wireless then next.
  while(wireless != nullptr){
    if(is_bad_mac_address(other)
      || is_invalid_network_driver(wireless->Description)
      || is_invalid_network_driver_type(wireless->Type, wireless->AddressLength)
      || wireless->Type != XOR32(IF_TYPE_IEEE80211)){
      wireless = wireless->Next;
      continue;
    }

    info.hash = FNV1A_RT(&wireless->Address[0], wireless->AddressLength);
    DBG("[!] get_mac_address (MIB_IF_TYPE_ETHERNET): \"%s\"\n", wireless->Description);
    DBG("[!] get_mac_address (MIB_IF_TYPE_ETHERNET): %01X-%01X-%01X-%01X-%01X-%01X (0x%08X)\n", wireless->Address[0], wireless->Address[1], wireless->Address[2], wireless->Address[3], wireless->Address[4], wireless->Address[5], info.hash);
    if(info.hash != 0)
      return true;

    wireless = wireless->Next;
  }

  // Other than try and fetch one driver, but no type x->type != check failure.
  while(other != nullptr){
    if(is_bad_mac_address(other)
      || is_invalid_network_driver(other->Description)
      || is_invalid_network_driver_type(other->Type, other->AddressLength)){
      other = other->Next;
      continue;
    }

    info.hash = FNV1A_RT(&other->Address[0], other->AddressLength);
    DBG("[!] get_mac_address (%i): \"%s\"\n", other->Type, other->Description);
    DBG("[!] get_mac_address (%i): %01X-%01X-%01X-%01X-%01X-%01X (0x%08X)\n", other->Type, other->Address[0], other->Address[1], other->Address[2], other->Address[3], other->Address[4], other->Address[5], info.hash);
    if(info.hash != 0)
      return true;

    other = other->Next;
  }

  SHOW_ERROR_STR(ERR_GET_MAC_ADDRESS_FAILED, "Pre setup failed (2).", true);
  return false;
}

bool c_hwid::get_arp_mac_address(s_arp_mac_address_info& info){
  u32 size = 0;
  u32 result = I(GetAdaptersInfo)(nullptr, &size);
  if(result != XOR32(ERROR_BUFFER_OVERFLOW)){
    SHOW_ERROR_STATUS_STR(ERR_GET_ARP_GET_ADAPTER_FAILED, result, "Pre setup failed. (3)", true);
    return false;
  }

  if(!size){
    SHOW_ERROR(ERR_GET_ARP_NULL_SIZE, true);
    return false;
  }

  IP_ADAPTER_INFO* adapter = I(VirtualAlloc)(nullptr, size, XOR32(MEM_COMMIT), XOR32(PAGE_READWRITE));
  if(adapter == nullptr){
    SHOW_ERROR_STATUS(ERR_GET_ARP_ALLOC_FAILED, I(GetLastError)(), true);
    return false;
  }

  // Actually get the adapter data now.
  result = I(GetAdaptersInfo)(adapter, &size);
  if(result != ERROR_SUCCESS){
    SHOW_ERROR_STATUS(ERR_GET_ARP_GET_ADAPTER_FAILED, result, true);
    return false;
  }

  u32 in_addr = 0;
  while(adapter != nullptr){

    // We don't do this for mac addresses, but it has to be done here to try and resolve a bug with networks.
    bool is_valid_dhcp = I(strlen)(adapter->DhcpServer.IpAddress.String) != 0;
    if(!adapter->DhcpEnabled) // Assume this was manually configured by the user.
      is_valid_dhcp = true; 

    // The usual checks, avoid trying to mess with VPN, VM, and other 'fake' network drivers, we want the real deal.
    if(is_invalid_network_driver(adapter->Description) || is_invalid_network_driver_type(adapter->Type, XOR32(6)) && !is_valid_dhcp){
      DBG("[!] ARP Ignored network device: %s\n", adapter->Description);
      adapter = adapter->Next;
      continue;
    }

    in_addr = I(inet_addr)(adapter->GatewayList.IpAddress.String);
    if(in_addr == 0){
      DBG("[-] get_arp_mac_address: bad data next \"%s\" (%s): \"%s\"\n", adapter->Description, adapter->DhcpServer.IpAddress.String, adapter->GatewayList.IpAddress.String);
      adapter = adapter->Next;
      continue;
    }

    u32 mac_len = XOR32(sizeof(info.data));
    result  = I(SendARP)(in_addr, 0, &info.data[0], &mac_len);

    // Damn we failed!!
    if(result != XOR32(NO_ERROR) || mac_len == 0){
      adapter = adapter->Next;
      continue;
    }

    DBG("[+] get_arp_mac_address: router ip: \"%s\" (%s): \"%s\"\n", adapter->Description, adapter->DhcpServer.IpAddress.String, adapter->GatewayList.IpAddress.String);

    str_utils::trim((i8*)&info.data[0]); // Trim any spaces from the front and end of the string.
    info.hash = FNV1A_RT(&info.data[0], XOR32(sizeof(info.data)));

    DBG("[!] get_arp_mac_address: %01X-%01X-%01X-%01X-%01X-%01X (0x%08X)\n", info.data[0], info.data[1], info.data[2], info.data[3], info.data[4], info.data[5], info.hash);

    return true;
  }

  if(in_addr == 0){
    SHOW_ERROR_STR(ERR_GET_ARP_NO_ROUTER_FOUND, "Current network is not supported.", true);
    return false;
  }

  SHOW_ERROR_STATUS_STR(ERR_GET_ARP_PING_FAILED, result, "The network you are connected to is unsupported by RijiN.\nPlease connect to an ethernet or wifi network.", true);

  return false;
}