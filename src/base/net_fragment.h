#pragma once

// WinInet
#include <wininet.h>
#include "enc.h"

// June 17, 2025.
// Pretty sure its because the cheats have a different way on how things are included.
// Without this we miss certain things for it to compile.
#if !defined(INTERNAL_CHEAT)
  #include "cheat_shared/rebuild_core.h"
  #include "assert.h"
#endif

// #define NET_FRAG_DEBUG
#define CMD_PREFIX "d3fffdef8062daf1654cd70097eb9219"
#define MAKE_CMD(x) HASH(CMD_PREFIX x CMD_PREFIX)

#define NET_FRAGMENT_MAGIC 0x9B0808
#define NET_USER_AGENT XOR("68BEF823F340F49A07CEA2F34308D1D5")

#if defined(DEV_MODE) || defined(BUILD_FLAG_TALK_TO_LOCALHOST)
#define REMOTE_HOST "127.0.0.1"
#define REMOTE_PORT 43244
#elif defined(STAGING_MODE)
#define REMOTE_HOST "loader_staging.rijin.solutions"
#define REMOTE_PORT 443
#else
#if defined(LOADER_CLIENT) || defined(LOADER_STUB)
#define REMOTE_HOST "loader_init.rijin.solutions"
#define REMOTE_PORT 443
#else
#define REMOTE_HOST "loader2.rijin.solutions"
#define REMOTE_PORT 443
#endif
#endif

#define NET_FRAGMENT_MAX_TIMEOUT 60000 * 2

enum net_fragment_status {
  NET_FRAGMENT_ERROR = 0,
  NET_FRAGMENT_OK = 1,
  NET_FRAGMENT_DATA_DIFFERS = 2,
};

#define NET_FRAGMENT_SECURITY_FLAG_CT  (1 << ((i32)(__TIME__[0] + __TIME__[1] + __TIME__[2] + __TIME__[3] + __TIME__[4]) % 16))
#define NET_FRAGMENT_SECURITY_FLAG_GEN (1 << 30)

#define NET_FRAGMENT_KILL_PROCESS (1 << 30)

extern volatile u32 NET_FRAGMENT_SECURITY_FLAG;
extern volatile u32 NET_FRAGMENT_EXTRA_FLAG;

template <int mod, int reason>
ALWAYSINLINE inline volatile void trip_security() {
  volatile u32* p = &NET_FRAGMENT_SECURITY_FLAG;
  volatile u32* e = &NET_FRAGMENT_EXTRA_FLAG;
  *(u8*)((uptr)e + XOR32(1)) = (u8)reason;
  if constexpr(mod == 0)
  {
    *p |= (1 << 28);
  }
  else if constexpr(mod == 1) {
    *p &= ~NET_FRAGMENT_SECURITY_FLAG_GEN;
  }
  else if constexpr(mod == 2) {
    *p *= 2;
  }
  else if constexpr(mod == 3){
    *p -= XOR32_IMPORTANT_ONLY(200);
  }
  else if constexpr(mod == 4){
    *p = XOR32_IMPORTANT_ONLY(32);
  }
  else{
    *p = 0;
  }
}

#define TRIP_SECURITY() trip_security<__LINE__ % 5, 0>()
#define TRIP_SECURITY_REASON(r) trip_security<__LINE__ % 5, r>()

class c_net_fragment {
public:
  u32 magic;
  u32 cmd;
  u32 status;
  u32 caller;
  u32 owner;
  u32 regression;
  u32 security;
  u32 extra_flags;
  u32 len;
  u8  nonce[NONCE_SIZE];
  u32 checksum;

  ALWAYSINLINE inline void encrypt(){
    if(!len)
      return;

    write_nonce(nonce);

    u8* data = (u8*)((uptr)this + sizeof(c_net_fragment));
    encrypt_buffer(data, len, nonce);
    checksum = FNV1A_BINARY(data, len);
  }

  ALWAYSINLINE inline CFLAG_O0 bool decrypt(){
    if(!valid()){
      assert(false && "bad decrypt call lol");
      return false;
    }

    u8* data = (u8*)((uptr)this + sizeof(c_net_fragment));
    if(FNV1A_BINARY(data, len) != checksum){
      assert(false && "decrypt checksum error");
      return false;
    }

    decrypt_buffer(data, len, nonce);
    return true;
  }

  ALWAYSINLINE void compute_regression(){
    security    = NET_FRAGMENT_SECURITY_FLAG;
    extra_flags = NET_FRAGMENT_EXTRA_FLAG;

    #if defined(LOADER_CLIENT)
      caller = XOR32(HASH("LOADER"));
    #elif defined(LOADER_STUB)
      caller = XOR32(HASH("STUB"));
    #elif defined(INTERNAL_CHEAT)
      caller = XOR32_IMPORTANT_ONLY(HASH("INTERNAL"));
    #else
      caller = XOR32(HASH("PRODUCT"));
    #endif

    #if defined(INTERNAL_CHEAT)
      create_server_iden(rijin_username_hash, "RIJIN_USERNAME");
      owner = rijin_username_hash_gen;
    #else
      owner = XOR32(HASH("GENERIC_BUT_NOT_EMPTY")) ^ XOR32(__COUNTER__);
    #endif

    /*
      The server will conduct a challenged request, it'll know what these values are. If too many failures occur a security lock will occur.
      Failures should be uncommon, but the idea is to punish lazy zeroing or message modifications.
    */
    u8 reg[32];
    write_nonce(reg);
    regression = HASH_RT(reg);
  }
  
  ALWAYSINLINE c_net_fragment(){
    memset(this, 0, XOR32(sizeof(c_net_fragment)));
    magic = XOR32(NET_FRAGMENT_MAGIC);
  }
  
  ALWAYSINLINE bool release(){
    free(this);
    return false;
  }
  
  ALWAYSINLINE bool valid(){
    return this != nullptr && magic == XOR32(NET_FRAGMENT_MAGIC) && len > 0 && status == NET_FRAGMENT_OK;
  }
  
  ALWAYSINLINE inline c_net_fragment* transmit(bool ignore_error = false, bool* did_time_out = nullptr,  bool dont_close = false);
};

// FRAGMENT MESSAGES
#pragma pack(push, 1)
  #define c_net_receive_info_cmd MAKE_CMD("NET_REQUEST_INFO")
  class c_net_receive_info : public c_net_fragment {
  public:
    u32 timestamp = 0;
    u32 loader_hash = 0;
    bool update_mode = false;
  };

  #define c_net_receive_error_message_cmd MAKE_CMD("NET_ERROR_MESSAGE")
  class c_net_receive_error_message : public c_net_fragment{
  public:
    bool exit;
    u32  str_len;
    i8   data[0];
  };
#pragma pack(pop)

ALWAYSINLINE inline c_net_fragment* transmit_fragment_internal(c_net_fragment* frag, bool ignore_error = false, bool* did_time_out = nullptr, bool dont_close = false){
  frag->compute_regression();

  #if defined(NET_FRAG_DEBUG)
  DBG("[!] attempting to transmit cmd %p\n", frag->cmd);
  #endif

  // We do this because some processes wont have wininet initialized so we need to do it like this
  uptr wininet_module = utils::get_module_handle(HASH("wininet.dll"));
  while(!wininet_module) // June 16th, 2025: This was being called every time we executed this function. Only do it if we can't find the module.
    wininet_module = (uptr)I(LoadLibraryA)(XOR("WININET.DLL"));


  static decltype(&InternetOpenA)       internet_open           = (decltype(&InternetOpenA))utils::get_proc_address(wininet_module, HASH("InternetOpenA"));
  static decltype(&InternetConnectA)    internet_connect        = (decltype(&InternetConnectA))utils::get_proc_address(wininet_module, HASH("InternetConnectA"));
  static decltype(&HttpOpenRequestA)    internet_open_request   = (decltype(&HttpOpenRequestA))utils::get_proc_address(wininet_module, HASH("HttpOpenRequestA"));
  static decltype(&InternetCloseHandle) internet_close_handle   = (decltype(&InternetCloseHandle))utils::get_proc_address(wininet_module, HASH("InternetCloseHandle"));
  static decltype(&InternetSetOptionA)  internet_set_option     = (decltype(&InternetSetOptionA))utils::get_proc_address(wininet_module, HASH("InternetSetOptionA"));
  static decltype(&HttpQueryInfoA)      http_query_info         = (decltype(&HttpQueryInfoA))utils::get_proc_address(wininet_module, HASH("HttpQueryInfoA"));
  static decltype(&HttpSendRequestA)    http_send_request       = (decltype(&HttpSendRequestA))utils::get_proc_address(wininet_module, HASH("HttpSendRequestA"));
  static decltype(&InternetReadFile)    internet_read_file      = (decltype(&InternetReadFile))utils::get_proc_address(wininet_module, HASH("InternetReadFile"));

  void* internet_handle = internet_open(NET_USER_AGENT, XOR32(INTERNET_OPEN_TYPE_DIRECT), nullptr, nullptr, 0);

  if(internet_handle == nullptr){
    DBG("[-] internet_handle == nullptr\n");
    return nullptr;
  }

  std::string ip   = XOR(REMOTE_HOST);
  u32         port = XOR32(REMOTE_PORT);

  void* connection_handle = internet_connect(internet_handle, ip.c_str(), port, nullptr, nullptr, XOR32(INTERNET_SERVICE_HTTP), XOR32(INTERNET_FLAG_NO_CACHE_WRITE) | XOR32(INTERNET_FLAG_DONT_CACHE) | XOR32(INTERNET_FLAG_RELOAD), 0);

  if(connection_handle == nullptr){
    DBG("[-] connection_handle invalid (%p)\n", connection_handle);
    internet_close_handle(internet_handle);
    return nullptr;
  }

  u32 flags = XOR32(INTERNET_FLAG_NO_CACHE_WRITE) | XOR32(INTERNET_FLAG_DONT_CACHE) | XOR32(INTERNET_FLAG_RELOAD);

  if(port == XOR32(443))
    flags |= XOR32(INTERNET_FLAG_SECURE);

  void* request_handle = internet_open_request(connection_handle, XOR("GET"), XOR(""), nullptr, nullptr, nullptr, flags, 0 );

  if(request_handle == nullptr){
    DBG("[-] request_handle invalid (%p)\n", request_handle);
    internet_close_handle(request_handle);
    internet_close_handle(connection_handle);
    internet_close_handle(internet_handle);
    return nullptr;
  }

  const ul64 timeout = XOR32(NET_FRAGMENT_MAX_TIMEOUT);

  internet_set_option(nullptr, XOR32(INTERNET_OPTION_END_BROWSER_SESSION), nullptr, 0);

  internet_set_option(request_handle, XOR32(INTERNET_OPTION_RECEIVE_TIMEOUT),      &timeout, XOR32(sizeof(ul64)));
  internet_set_option(request_handle, XOR32(INTERNET_OPTION_SEND_TIMEOUT),         &timeout, XOR32(sizeof(ul64)));
  internet_set_option(request_handle, XOR32(INTERNET_OPTION_CONNECT_TIMEOUT),      &timeout, XOR32(sizeof(ul64)));
  internet_set_option(request_handle, XOR32(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT), &timeout, XOR32(sizeof(ul64)));
  internet_set_option(request_handle, XOR32(INTERNET_OPTION_DATA_SEND_TIMEOUT),    &timeout, XOR32(sizeof(ul64)));

  if(did_time_out != nullptr)
    *did_time_out = false;

  frag->encrypt();
  bool request_sent = http_send_request(request_handle, nullptr, 0, frag, frag->len + XOR32(sizeof(c_net_fragment)));

  if(!request_sent){
    if(did_time_out != nullptr)
      *did_time_out = true;

    DBG("[-] request timed out %p\n", GetLastError());
    internet_close_handle(request_handle);
    internet_close_handle(connection_handle);
    internet_close_handle(internet_handle);
    return nullptr;
  }

  u32 buffer_len = 0;
  u32 buffer_len_size = XOR32(sizeof(buffer_len));

  if(!http_query_info(request_handle, XOR32(HTTP_QUERY_CONTENT_LENGTH) | XOR32(HTTP_QUERY_FLAG_NUMBER), &buffer_len, &buffer_len_size, nullptr)){
    assert_private(false && "http_query_info failed");
    return nullptr;
  }

  if(buffer_len < sizeof(c_net_fragment)){
    if(did_time_out != nullptr)
      *did_time_out = true;

    DBG("[-] buffer_len too small\n");
    internet_close_handle(request_handle);
    internet_close_handle(connection_handle);
    internet_close_handle(internet_handle);
    return nullptr;
  }

  #if defined(NET_FRAG_DEBUG)
  DBG("[!] expecting buffer len %i\n", buffer_len);
  #endif

  u64 current_position = 0;
  c_net_fragment* fragment = I(malloc)(buffer_len);
  if(fragment == nullptr) // We would want an error code to popup rather than our process crashing.
    return nullptr;

  memset(fragment, 0, buffer_len);

  u32  data_size = 0;
  bool result;
  do{
    result = internet_read_file(request_handle, (i8*)((u64)fragment + (u32)current_position), buffer_len - current_position, &data_size);
    if(result){
      if(!data_size)
        break;

      current_position += data_size;
    }
    else{
      assert_private(false && "internet_read_file failed");
      break;
    }

  } while(data_size > 0 && current_position < buffer_len);

  if(current_position != buffer_len){
    DBG("[!] transmit_fragment_internal: invalid fragment(current_position: %i, buffer_len: %i)\n", current_position, buffer_len);
    I(free)(fragment);
    return nullptr;
  }

#if defined(INTERNAL_CHEAT)
    if(fragment->extra_flags & NET_FRAGMENT_KILL_PROCESS){
      I(ExitProcess)(0);
      return nullptr;
    }
#endif


  if(!fragment->valid()){
    I(free)(fragment);
    return nullptr;
  }

  fragment->decrypt();

#if defined(NET_FRAG_DEBUG)
  DBG("[!] OK - fragment->magic = %p\n", fragment->magic);
  DBG("[!] OK - fragment->cmd = %p\n", fragment->cmd);
  DBG("[!] OK - fragment->status = %i\n", fragment->status);
  DBG("[!] OK - fragment->regression = %i\n", fragment->regression);
  DBG("[!] OK - fragment->security = %i\n", fragment->security);
  DBG("[!] OK - fragment->extra_flags = %i\n", fragment->extra_flags);
  DBG("[!] OK - fragment->len = %i\n", fragment->len);
#endif

  if(fragment->cmd == c_net_receive_error_message_cmd){
    c_net_receive_error_message* error_msg = (c_net_receive_error_message*)fragment;

    DBG("[-] fragment error \"%s\"", (const i8*)&error_msg->data);
#if !defined(INTERNAL_CHEAT)
    if(!ignore_error){
      I(MessageBoxA)(nullptr, (const i8*)&error_msg->data, nullptr, MB_ICONERROR);

      if(error_msg->exit && !dont_close)
        I(exit)(0);
    }
#endif

    I(free)(fragment);
    fragment = nullptr;
  }

  internet_close_handle(request_handle);
  internet_close_handle(connection_handle);
  internet_close_handle(internet_handle);

  return fragment;
}

ALWAYSINLINE inline c_net_fragment* c_net_fragment::transmit(bool ignore_error, bool* did_time_out, bool dont_close) {
  return transmit_fragment_internal(this, ignore_error, did_time_out, dont_close);
}

