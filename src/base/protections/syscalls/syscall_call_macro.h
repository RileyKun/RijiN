#pragma once
#include "in_memory_init.h"
#include "syscalls.h"

template<std::uint32_t Hash>
static ALWAYSINLINE u32 syscall_present_impl(){
  ::jm::detail::exports_directory exports(static_cast<const char*>(::jm::detail::ntdll_base()));
  for(auto i = exports.size();; --i){
    if(::jm::hash(exports.name(i)) == Hash)
      return *reinterpret_cast<const std::int32_t*>(exports.address(i) + 4);
  }
  return 0;
}

#define SYSCALLS_PRESENT

#define GET_SYSCALL_ID(x) (syscall_present_impl<::jm::hash(#x)>())
#define SYSCALL(x) INLINE_SYSCALL_MANUAL(x, GET_SYSCALL_ID(x))