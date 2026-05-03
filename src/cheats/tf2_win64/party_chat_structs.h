#pragma once

#define PARTY_CHAT_PACKET_HEADER 0x1776
#define PARTY_CHAT_PACKET_MAX_SIZE 124

enum party_chat_packet_types{
  PARTY_CHAT_PACKET_TYPE_INVALID = 0,
  PARTY_CHAT_PACKET_TYPE_TEST,
  PARTY_CHAT_PACKET_TYPE_ENTITY_UPDATE,
  PARTY_CHAT_PACKET_TYPE_RESOLVER,
  PARTY_CHAT_PACKET_TYPE_WAYPOINT_MARKERS,
};

class c_party_chat_fragment{
public:
  u16 magic;
  u32 server;
  u8  type;
  u8  sender;
  u32 len;

  c_party_chat_fragment(){
    magic  = XOR32(PARTY_CHAT_PACKET_HEADER);
    server = get_current_server();
  }

  ALWAYSINLINE u32 get_current_server(){
    i8* ip = global->engine->get_ip();
    if(ip == nullptr)
      return 0;

    return HASH_RT(ip);
  }

  bool is_in_same_server(){
    return this != nullptr && server == get_current_server();
  }

  bool valid(){
    if(this == nullptr)
      return false;

    return magic == PARTY_CHAT_PACKET_HEADER && len > 0 && sender > 0 && sender <= MAXPLAYERS && type > 0;
  }

  bool overflowed(){
    return len > PARTY_CHAT_PACKET_MAX_SIZE;
  }

  bool transmit(){
    if(this == nullptr)
      return false;
    
    sender = (u8)global->engine->get_local_player();
    if(!valid()){
      DBG("[-] c_party_chat_fragment invalid\n");
      return false;
    }

    if(overflowed()){
      DBG("[-] c_party_chat_fragment data overflowed %i/%i\n", len, PARTY_CHAT_PACKET_MAX_SIZE);
      return false;
    }

    if(global->party_chat_rate_limited > math::time()){
      DBG("[-] c_party_chat_fragment rate limited.\n");
      return false;
    }

    std::string str = utils::bytes_to_hex((u8*)this, len);

    i8 buf[IDEAL_MAX_BUF_SIZE];
    formatA(buf, XOR("tf_party_chat \"%s\""), str.c_str());
    global->engine->client_cmd(buf);

    return true;
  }
};

class c_party_chat_packet_test : public c_party_chat_fragment{
public:
  c_party_chat_packet_test(){
    type = PARTY_CHAT_PACKET_TYPE_TEST;
    len  = XOR32(sizeof(c_party_chat_packet_test));
  }
  i8 buf[114];
};

struct entity_update_data{
  u8   index;
  vec3 pos;
};

class c_party_chat_packet_entity_update : public c_party_chat_fragment{
public:
  c_party_chat_packet_entity_update(){
    type = PARTY_CHAT_PACKET_TYPE_ENTITY_UPDATE;
    len  = XOR32(sizeof(c_party_chat_packet_entity_update));
  }

  entity_update_data data[6];
};

class c_party_chat_packet_resolver : public c_party_chat_fragment{
public:
  c_party_chat_packet_resolver(){
    type = PARTY_CHAT_PACKET_TYPE_RESOLVER;
    len = XOR32(sizeof(c_party_chat_packet_resolver));
  }

  i32  index;
  i32  resolver_pitch_cycle;
  i32  resolver_yaw_cycle;

  vec3 real_ang;
  vec3 network_ang;
  i32  response;
};

class c_party_chat_packet_waypoint_markers : public c_party_chat_fragment{
public:
  c_party_chat_packet_waypoint_markers(){
    type = PARTY_CHAT_PACKET_TYPE_WAYPOINT_MARKERS;
    len  = XOR32(sizeof(c_party_chat_packet_waypoint_markers));
  }

  vec3 pos;
};