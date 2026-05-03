#pragma once

#if defined(BUILD_FLAG_RTP)

#include "../../math/fnv1a.h"
#include "../../net_fragment.h"
#include "../../net_structs.h"
#include "../../cheat_shared/rebuild_core.h"
#include "../../api/registry.h"
#include "../../api/string_utils.h"

enum rtp_reasons{
  RTP_REASON_UNK = 0,
  RTP_REASON_TIME_CHECK_FAILED = 1,
  RTP_REASON_SUSPENDED_THREAD_RAN = 2,
  RTP_REASON_PROCESS_PAUSED = 3,
  RTP_REASON_SLEEP_BP = 4,
  RTP_REASON_COMMON_BP_PLACEMENTS = 5,
  RTP_REASON_DEBUGGER_ATTACHED = 6,
  RTP_REASON_CONSOLE_WINDOW_PRESENT = 7,
  RTP_REASON_CONSOLE_WINDOW_CHECK_BP_HOOK_PRESENT = 8,
  RTP_REASON_QUERY_VIRUTAL_MEMORY_TAMPERED = 9,
  RTP_REASON_SCYLLA_HIDE_DETECTED = 10,
  RTP_REASON_TITAN_HIDE_DETECTED = 11,
  RTP_REASON_DLL_INJECTION_FROM_DESKTOP = 12,
  RTP_REASON_CREATE_THREAD_BP_OR_HOOKED = 13,
  RTP_REASON_KUSERSHARED_KD_DEBUGGER_ENABLED = 14,
  RTP_REASON_PROTECTION_SYSTEM_TAMPERED = 15,
  RTP_REASON_NTQUERYSYSTEMINFO_TAMPERED = 16,
  RTP_REASON_TEST_SIGNING_CHECK_BYPASSED = 17,
};

#define WANTS_TIME_CHECKS() \
  float _time_check = 0.f; \
  float _time_end   = 0.f; \

#define TIME_CHECK_START() \
  _time_check = math::time(false, true);

#define TIME_CHECK_END(fail_delta) \
  _time_end = math::time(false, true); \
  if(math::abs(_time_check - _time_end) >= (float)fail_delta){ \
    assert(false && "time check fail, make sure delta is big enough"); \
    TRIP_SECURITY_REASON(RTP_REASON_TIME_CHECK_FAILED); \
  }

inline __attribute__((noreturn, naked)) void die() {
  __asm__ __volatile__ (
    "pop %rsp\n\t"
    "pop %rsp\n\t"
    "lea 0x19(%rsp), %rsp\n\t"
    "ret\n\t"
  );
}

#define SAFE_EXIT() \
  if(!IS_IMPORT_BREAKPOINT("kernel32.dll", "ExitProcess") && !IS_IMPORT_HOOKED("kernel32.dll", "ExitProcess")) \
    I(ExitProcess)(0); \
  else{ \
    die(); \
  }

namespace rtp{
  void reload_text_sections();
  void page_check();
  void runtime_protection();
  void pre_launch();
  void debugger_check();

  // This is solely designed to detect tampering, this should be called in the loader and stub, and possibly the vac bypass.
  ALWAYSINLINE inline void send_challenge(){
    c_net_request_challenge msg{};
    i8* guid = reg::read_key_str(XOR("SOFTWARE\\Microsoft\\Cryptography"), XOR("MachineGuid"), HKEY_LOCAL_MACHINE);
    if(guid != nullptr){
      for(i32 i = 0; i < strlen(guid); i++)
        msg.machine_guid[i] = guid[i];

      free(guid);
      guid = nullptr;
    }

    wchar_t* name = utils::get_module_name(utils::get_module_handle(0));
    for(i32 i = 0; i < wcslen(name); i++)
      msg.process_name[i] = (name[i] & XOR32(0xFF));

    ul64 len = XOR32(sizeof(msg.computer_name));
    GetComputerNameA(msg.computer_name, &len);

    msg.steam_active_user = reg::read_key_i32(WXOR(L"Software\\Valve\\Steam\\ActiveProcess"), WXOR(L"ActiveUser"));
    msg.steam_pid         = reg::read_key_i32(WXOR(L"Software\\Valve\\Steam\\ActiveProcess"), WXOR(L"pid"));
    msg.running_appid     = reg::read_key_i32(WXOR(L"Software\\Valve\\Steam\\ActiveProcess"), WXOR(L"RunningAppID"));
    msg.ida_history_count = 0;


    // Fetch IDA history.
    {
      memset(msg.ida_history, 0, sizeof(msg.ida_history));

      s_reg_key_data* keys = reg::get_keys(WXOR(L"Software\\Hex-Rays\\IDA\\History64"), msg.ida_history_count, HKEY_CURRENT_USER);
      if(keys != nullptr && msg.ida_history_count > 0){
        msg.flags |= NET_CHALLENGE_FLAG_IDA_INSTALLED;

        i32 b = 0;
        for(i32 i = 0; i < msg.ida_history_count; i++){
          s_reg_key_data* entry = &keys[i];
          if(entry == nullptr)
            continue;

          strcpy(msg.ida_history[i].buffer, entry->value);
          msg.ida_history[i].buffer_len = strlen(entry->value);

          for(i32 j = 0; j < msg.ida_history[i].buffer_len; j++)
            msg.ida_history[i].buffer[j] ^= STRING_XOR_KEY;

          if(b++ >= XOR32(MAX_CHALLENGE_STRING_TABLE_SIZE))
            break;
        }

        free(keys);
      }
    }

    DBG("\n");
    DBG("[!] process_name: \"%s\"\n", msg.process_name);
    DBG("[!] machine_guid: \"%s\"\n", msg.machine_guid);
    DBG("[!] computer_name: \"%s\"\n", msg.computer_name);
    DBG("[!] steam_active_user: \"%i\"\n", msg.steam_active_user);
    DBG("[!] steam_pid: \"%i\"\n", msg.steam_pid);
    DBG("[!] running_appid: \"%i\"\n", msg.running_appid);
    DBG("[!] frags: \"%i\"\n", msg.flags);
    DBG("\n");

    void* response = msg.transmit(true);
    if(response == nullptr){
      #if defined(LOADER_CLIENT)
        I(ExitProcess)(0);
      #endif
    }
  }

#if defined(LOADER_CLIENT)
  void screenshot(bool* result);
#endif
};

extern volatile u32   rtp_tick_count;
extern volatile float rtp_last_tick_update_time;
extern volatile bool  rtp_transmitted_heartbeat;
#endif