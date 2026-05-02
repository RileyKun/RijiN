#pragma once

#include "themida/themida_stealth.h"
#include "lazy_importer.h"
#include "xorstr.h"
#include "xorptr.h"

#if defined(__x86_64__) && !defined(INTERNAL_CHEAT)
  #include "syscalls/syscall_call_macro.h"

  #if defined(BUILD_FLAG_RTP)
    #include "rtp/rtp.h"
  #endif
#endif