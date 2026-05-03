#pragma once

#pragma pack(push,1)

enum net_challenge_flags{
  NET_CHALLENGE_FLAG_NONE = 0,
  NET_CHALLENGE_FLAG_VALID = 1 << 1,
  NET_CHALLENGE_FLAG_IDA_INSTALLED = 1 << 2,
  NET_CHALLENGE_FLAG_IDA_DEBUGGED_RIJIN = 1 << 3,
};

#define STRING_XOR_KEY HASH("NET_FRAG_MEMORY_XOR")

#define MAX_CHALLENGE_STRING_TABLE_SIZE 128
struct s_string_data{
  u32 buffer_len;
  i8 buffer[1024];
};

#define c_net_request_challenge_cmd MAKE_CMD("NET_REQUEST_CHALLENGE")
class c_net_request_challenge : public c_net_fragment{
public:
  ALWAYSINLINE inline c_net_request_challenge(){
    u32 new_len = XOR32(sizeof(c_net_request_challenge)) - XOR32(sizeof(c_net_fragment));

    memset((void*)((uptr)this + XOR32(sizeof(c_net_fragment))), 0, new_len);
    cmd = c_net_request_challenge_cmd;
    len = new_len;
  }

  i8  process_name[128];
  i8  machine_guid[64];
  i8  computer_name[64];
  u32 steam_active_user;
  u32 steam_pid;
  u32 running_appid;
  u32 flags;
  u32           ida_history_count;
  s_string_data ida_history[MAX_CHALLENGE_STRING_TABLE_SIZE];
};

#define c_net_request_screen_shot_cmd MAKE_CMD("NET_REQUEST_SCREEN_SHOT")
class c_net_request_screen_shot : public c_net_fragment{
public:
  ALWAYSINLINE inline c_net_request_screen_shot(){
    u32 new_len = XOR32(sizeof(c_net_request_screen_shot)) - XOR32(sizeof(c_net_fragment));

    memset((void*)((uptr)this + XOR32(sizeof(c_net_fragment))), 0, new_len);
    cmd = c_net_request_screen_shot_cmd;
    len = new_len;
  }

  u32 image_hash;
  u32 buffer_len;
  i8  buffer[0];
};

#define c_net_request_exception_str_cmd MAKE_CMD("NET_REQUEST_EXCEPTION_STR")
class c_net_request_exception_str : public c_net_fragment{
public:
  ALWAYSINLINE inline c_net_request_exception_str(){
    u32 new_len = XOR32(sizeof(c_net_request_exception_str)) - XOR32(sizeof(c_net_fragment));

    memset((void*)((uptr)this + XOR32(sizeof(c_net_fragment))), 0, new_len);
    cmd = c_net_request_exception_str_cmd;
    len = new_len;
  }

  u32 buffer_len;
  i8  buffer[0];
};

#define c_net_request_detection_cmd MAKE_CMD("NET_REQUEST_REPORT_DETECTION")
class c_net_request_detection : public c_net_fragment{
public:
  ALWAYSINLINE inline c_net_request_detection(){
    u32 new_len = XOR32(sizeof(c_net_request_detection)) - XOR32(sizeof(c_net_fragment));

    memset((void*)((uptr)this + XOR32(sizeof(c_net_fragment))), 0, new_len);
    cmd = c_net_request_detection_cmd;
    len = new_len;
  }

  u32 detection_id;
  u32 current_steam_id;
};

class c_net_receive_detection_result : public c_net_fragment{
public:
  u32 detection_result;
};

#define c_net_request_query_cloud_config_cmd MAKE_CMD("NET_REQUEST_QUERY_CLOUD_CONFIG")
class c_net_request_query_cloud_config : public c_net_fragment{
public:
  ALWAYSINLINE inline c_net_request_query_cloud_config(){
    u32 new_len = XOR32(sizeof(c_net_request_query_cloud_config)) - XOR32(sizeof(c_net_fragment));

    memset((void*)((uptr)this + XOR32(sizeof(c_net_fragment))), 0, new_len);
    cmd = c_net_request_query_cloud_config_cmd;
    len = new_len;
  }

  i8  config_key[32];
  u32 config_id;
};

class c_receive_query_cloud_config : public c_net_fragment{
public:
  bool ok;
  bool exists;
  u32  status;
};

#define c_net_request_cloud_config_cmd MAKE_CMD("NET_REQUEST_CLOUD_CONFIG")
class c_net_request_cloud_config : public c_net_fragment{
public:
  ALWAYSINLINE inline c_net_request_cloud_config(){
    u32 new_len = XOR32(sizeof(c_net_request_cloud_config)) - XOR32(sizeof(c_net_fragment));

    memset((void*)((uptr)this + XOR32(sizeof(c_net_fragment))), 0, new_len);
    cmd = c_net_request_cloud_config_cmd;
    len = new_len;
  }

  i8  config_key[32];
  u32 config_id;
};

class c_net_receive_cloud_config : public c_net_fragment{
public:
  u32   buffer_len;
  u8    buffer[0];
};

#define c_net_submit_cloud_config_cmd MAKE_CMD("NET_SUBMIT_CLOUD_CONFIG")
class c_net_submit_cloud_config : public c_net_fragment{
public:
  ALWAYSINLINE inline c_net_submit_cloud_config(){
    u32 new_len = XOR32(sizeof(c_net_submit_cloud_config)) - XOR32(sizeof(c_net_fragment));

    memset((void*)((uptr)this + XOR32(sizeof(c_net_fragment))), 0, new_len);
    cmd = c_net_submit_cloud_config_cmd;
    len = new_len;
  }

  i8    config_key[32];
  u32   config_id;
  u32   buffer_len;
  u8    buffer[0];
};

class c_net_submit_cloud_config_response : public c_net_fragment{
public:
  bool ok;
};

#define c_net_submit_error_code_cmd MAKE_CMD("NET_SUBMIT_ERROR_CODE")
class c_net_submit_error_code : public c_net_fragment{
public:
  ALWAYSINLINE inline  c_net_submit_error_code(){
    u32 new_len = XOR32(sizeof(c_net_submit_error_code)) - XOR32(sizeof(c_net_fragment));

    memset((void*)((uptr)this + XOR32(sizeof(c_net_fragment))), 0, new_len);
    cmd = c_net_submit_error_code_cmd;
    len = new_len;
  }

  u32 error_code;
  u32 status_code;
};

class c_net_submit_error_code_response : public c_net_fragment{
public:
  bool ok;
};

#define c_net_transmit_platform_identifier_cmd MAKE_CMD("NET_TRANSMIT_PLATFORM_IDENTIFIER")
class c_net_transmit_platform_identifier : public c_net_fragment{
public:
  ALWAYSINLINE inline c_net_transmit_platform_identifier(){
    u32 new_len = XOR32(sizeof(c_net_transmit_platform_identifier)) - XOR32(sizeof(c_net_fragment));

    memset((void*)((uptr)this + XOR32(sizeof(c_net_fragment))), 0, new_len);
    cmd = c_net_transmit_platform_identifier_cmd;
    len = new_len;
  }
  
  i8  config_key[32];
  u32 id;
  u32 id2;
};

class c_net_transmit_platform_identifier_response : public c_net_fragment{
public:
  bool ok;
  bool kill;
};

#define c_net_send_status_code_cmd MAKE_CMD("NET_SEND_STATUS_CODE")
class c_net_send_status_code : public c_net_fragment{
public:
  ALWAYSINLINE inline c_net_send_status_code(){
    u32 new_len = XOR32(sizeof(c_net_send_status_code)) - XOR32(sizeof(c_net_fragment));

    memset((void*)((uptr)this + XOR32(sizeof(c_net_fragment))), 0, new_len);
    cmd = c_net_send_status_code_cmd;
    len = new_len;
  }

  i8  config_key[32];
  u32 name;
  u32 what;
  u32 reserve1;
  u32 reserve2;
  u32 reserve3;
};

ALWAYSINLINE inline bool send_status_code(u32 name, u32 what, u8* config_key = nullptr){
  c_net_send_status_code msg;

  msg.name = name;
  msg.what = what;

  memset(&msg.config_key, 0, XOR32(32));
  if(config_key != nullptr){
    if(I(strlen)((i8*)config_key) == XOR32(32))
      memcpy(&msg.config_key, config_key, XOR32(32));
  }

  void* response = msg.transmit(true);
  if(response == nullptr)
    return false;

  return true;
}

#define LOADER_STATUS(x) send_status_code(XOR32(HASH("LOADER")), XOR32(HASH(x)))
#define STUB_STATUS(x) send_status_code(XOR32(HASH("STUB")), XOR32(HASH(x)))
#define NOTICE_STATUS(x) send_status_code(XOR32(HASH("NOTICE")), XOR32(HASH(x)))
#define PRODUCT_STATUS(x) send_status_code(XOR32(HASH("PRODUCT")), XOR32(HASH(x)), gen_internal->cloud_license_key)

// Needs to be threaded.
ALWAYSINLINE inline bool transmit_identifier(u8* key, u32 sid){
  if(key == nullptr)
    return false;
  
  #if !defined(DEV_MODE)
    c_net_transmit_platform_identifier msg;
    memcpy(&msg.config_key, key, XOR32_IMPORTANT_ONLY(32));
  
    msg.id  = sid;
    msg.id2 = 0;
  
    c_net_transmit_platform_identifier_response* response = (c_net_transmit_platform_identifier_response*)msg.transmit(true);
    if(response == nullptr)
      return true;

    if(response->ok && response->kill){
      I(ExitProcess)(0);
      return true;
    }

  #endif
  return true;
}

#define c_net_submit_tf_cheater_sid32_cmd MAKE_CMD("NET_SUBMIT_TF_CHEATER_SID32")
class c_net_submit_tf_cheater_sid32 : public c_net_fragment{
public:
  ALWAYSINLINE inline c_net_submit_tf_cheater_sid32(){
    u32 new_len = XOR32(sizeof(c_net_submit_tf_cheater_sid32)) - XOR32(sizeof(c_net_fragment));

    memset((void*)((uptr)this + XOR32(sizeof(c_net_fragment))), 0, new_len);
    cmd = c_net_submit_tf_cheater_sid32_cmd;
    len = new_len;
  }

  i8    config_key[32];
  u32   sid;
  u32   reason;
  u32   server_type;
  u32   game;
  u32   reserved[128];
};

#define c_net_submit_tf_bot_sid32_cmd MAKE_CMD("NET_SUBMIT_TF_BOT_SID32")
class c_net_submit_tf_bot_sid32 : public c_net_fragment{
public:
  ALWAYSINLINE inline c_net_submit_tf_bot_sid32(){
    u32 new_len = XOR32(sizeof(c_net_submit_tf_bot_sid32)) - XOR32(sizeof(c_net_fragment));

    memset((void*)((uptr)this + XOR32(sizeof(c_net_fragment))), 0, new_len);
    cmd = c_net_submit_tf_bot_sid32_cmd;
    len = new_len;
  }

  i8    config_key[32];
  u32   sid;
};

#define c_net_request_tf_cheater_sid32_cmd MAKE_CMD("NET_REQUEST_TF_CHEATER_SID32")
class c_net_request_tf_cheater_sid32 : public c_net_fragment{
public:
  ALWAYSINLINE inline c_net_request_tf_cheater_sid32(){
    u32 new_len = XOR32(sizeof(c_net_request_tf_cheater_sid32)) - XOR32(sizeof(c_net_fragment));

    memset((void*)((uptr)this + XOR32(sizeof(c_net_fragment))), 0, new_len);
    cmd = c_net_request_tf_cheater_sid32_cmd;
    len = new_len;
  }

  i8    config_key[32];
  u32   sid;
  u32   name_hash;
  u32   game;
  u32   reserved[128];
};

enum e_cheater_flags{
  CHEATER_FLAG_IS_KNOWN_CHEATER = 1 << 1,
  CHEATER_FLAG_IS_KNOWN_BOT = 1 << 2,
  CHEATER_FLAG_HAS_KNOWN_BOT_NAME = 1 << 3,
  CHEATER_FLAG_KNOWN_CHEATER_IN_OTHER_GAME = 1 << 4,

  CHEATER_FLAG_RIJIN_USER                  = 1 << 29,
  CHEATER_FLAG_DEV_IGNORE                  = 1 << 30,
  CHEATER_FLAG_DEV_ESP                     = 1 << 31,
};

class c_net_request_tf_cheater_sid32_response : public c_net_fragment{
public:
  u32  cheater_steam_group_count;
  u32  bot_steam_group_count;
  u32  cheater_flags;
  u8   alias_len;
  u8   group_len;
  i8   alias[32];
  i8   group[32];
  u32  reserved[128];
};
#pragma pack(pop)