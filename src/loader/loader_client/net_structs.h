#pragma once

#pragma pack(push,1)
#define c_net_request_license_cmd MAKE_CMD("NET_REQUEST_LICENSE")
class c_net_request_license : public c_net_fragment {
public:
  ALWAYSINLINE inline c_net_request_license(){
    u32 new_len = XOR32(sizeof(c_net_request_license)) - XOR32(sizeof(c_net_fragment));
        
    memset((uptr)this + XOR32(sizeof(c_net_fragment)), 0, new_len);
    cmd = c_net_request_license_cmd;
    len = new_len;
  } 
  
  u32 client_version;
  i8 username[32];
  i8 password[256];
  
  // hardware info
  u32 hwid_mac_address_hash;
  u32 hwid_arp_mac_address_hash;
  i8  hwid_cpu_brand_name[48];

  // steamid logging
  s_steam_id_list steam_id_list;
};

class c_net_receive_license : public c_net_fragment{
public:
  i8 key[32];
  u32 license_expire_seconds = 0;
  u32 objects_size = 0;
  license_object objects[0];
};

#define c_net_request_image_resource_cmd MAKE_CMD("NET_REQUEST_IMAGE_RESOURCE")
class c_net_request_image_resource : public c_net_fragment{
public:
  ALWAYSINLINE inline c_net_request_image_resource(){
    u32 new_len = XOR32(sizeof(c_net_request_image_resource)) - XOR32(sizeof(c_net_fragment));

    memset((uptr)this + XOR32(sizeof(c_net_fragment)), 0, new_len);
    cmd = c_net_request_image_resource_cmd;
    len = new_len;
  }

  i8  key[32];
  u32 object_pos;
};

class c_net_receive_image_resource : public c_net_fragment{
public:
  u32 len;
  u8 data[0];
};

#define c_net_request_stub_cmd MAKE_CMD("NET_RECEIVE_STUB")
class c_net_request_stub : public c_net_fragment{
public:
  ALWAYSINLINE inline c_net_request_stub(){
    u32 new_len = XOR32(sizeof(c_net_request_stub)) - XOR32(sizeof(c_net_fragment));
        
    memset((uptr)this + XOR32(sizeof(c_net_fragment)), 0, new_len);
    cmd = c_net_request_stub_cmd;
    len = new_len;
  } 
  
  i8 key[32];
};

class c_net_receive_stub : public c_net_fragment{
public:
  u32 len;
  u8 data[0];
};
#pragma pack(pop)