#pragma once

#if defined(INTERNAL_CHEAT)
namespace exceptions{
  static LONG handle_exception(EXCEPTION_RECORD* record, CONTEXT* registers, i8* game_name);
  static void write_register(i8* buf, i8* register_name, void* p){
    MODULEENTRY32 entry = {};
    if(utils::get_module_info_from_address_space((uptr)p, &entry)){
      format_append_a(buf, IDEAL_WORKING_BUF_SIZE, XOR("%s: %s+%X\n"), register_name, entry.szModule, (uptr)p - (uptr)entry.modBaseAddr);
    }
    else{
      format_append_a(buf, IDEAL_WORKING_BUF_SIZE, XOR("%s: %p\n"), register_name, p);
    }
  }

  static void show_stack(i8* buf, void* bp, void* sp, const bool leaf){
    if(bp == nullptr)
      return;

    if(!utils::is_memory_valid(sp, MEMORY_READ))
      return;

    format_append_a(buf, IDEAL_WORKING_BUF_SIZE, XOR("\nCALLSTACK:\n"));
    for(i32 i = 1; i < 128; i++){

      if(utils::is_memory_valid(bp, MEMORY_READ)                      ||
         utils::is_memory_valid((uptr)bp + sizeof(uptr), MEMORY_READ) ||
         utils::is_memory_valid((uptr)bp + (sizeof(uptr) * 2), MEMORY_READ)){

        uptr* ret_addr_ptr = (uptr*)((uptr)bp + sizeof(uptr));
        if(*ret_addr_ptr != 0){
          write_register(buf, XOR("BP-"), (void*)*ret_addr_ptr);

          sp = (bp + (sizeof(uptr) * 2));
          bp = (bp + (sizeof(uptr)));
          continue;
        }
      }


      if(utils::is_memory_valid(sp, MEMORY_READ)                      ||
         utils::is_memory_valid((uptr)sp + sizeof(uptr), MEMORY_READ)){
         uptr* ret_addr_ptr = (uptr*)sp;
         if(*ret_addr_ptr != 0){
           write_register(buf, XOR("SP-"), (void*)*ret_addr_ptr);

           sp += sizeof(uptr);
           continue;
         }
      }


      break;
    }
  }

  static void write_exception(EXCEPTION_RECORD* record, CONTEXT* registers, i8* game_name){
    if(record == nullptr || registers == nullptr)
      return;

    static i8 buf[IDEAL_WORKING_BUF_SIZE] = {};
    appendA(buf, XOR("RijiN has caught an exception in %s - %s\nCode: %X\n\n"), game_name, XOR(BUILD_VERSION_STR), record->ExceptionCode);

    appendA(buf, XOR("CFlags: %X, EFlags: %X\n"), registers->ContextFlags, registers->EFlags);

    #if defined(__x86_64__)
      write_register(buf, XOR("RIP"), registers->Rip);
      write_register(buf, XOR("RAX"), registers->Rax);
      write_register(buf, XOR("RCX"), registers->Rcx);
      write_register(buf, XOR("RDX"), registers->Rdx);
      write_register(buf, XOR("RBX"), registers->Rbx);
      write_register(buf, XOR("RSP"), registers->Rsp);
      write_register(buf, XOR("RBP"), registers->Rbp);
      write_register(buf, XOR("RSI"), registers->Rsi);
      write_register(buf, XOR("RDI"), registers->Rdi);

      write_register(buf, XOR("\nR8"), registers->R8);
      write_register(buf, XOR("R9"), registers->R9);
      write_register(buf, XOR("R10"), registers->R10);
      write_register(buf, XOR("R11"), registers->R11);
      write_register(buf, XOR("R12"), registers->R12);
      write_register(buf, XOR("R13"), registers->R13);
      write_register(buf, XOR("R14"), registers->R14);
      write_register(buf, XOR("R15"), registers->R15);

      show_stack(buf, registers->Rbp, registers->Rsp, true);
    #else
      write_register(buf, XOR("EIP"), registers->Eip);
      write_register(buf, XOR("EAX"), registers->Eax);
      write_register(buf, XOR("ECX"), registers->Ecx);
      write_register(buf, XOR("EDX"), registers->Edx);
      write_register(buf, XOR("EBX"), registers->Ebx);
      write_register(buf, XOR("ESP"), registers->Esp);
      write_register(buf, XOR("EBP"), registers->Ebp);
      write_register(buf, XOR("ESI"), registers->Esi);
      write_register(buf, XOR("EDI"), registers->Edi);

      show_stack(buf, registers->Ebp, registers->Esp, true);
    #endif

    // This is totally "Safe".
    static c_net_request_exception_str* ex_msg = (c_net_request_exception_str*)malloc(sizeof(c_net_request_exception_str) + IDEAL_WORKING_BUF_SIZE);

    ex_msg->magic  = XOR32(NET_FRAGMENT_MAGIC);
    ex_msg->cmd    = XOR32(c_net_request_exception_str_cmd);
    ex_msg->status = XOR32(NET_FRAGMENT_OK);
    ex_msg->len    = XOR32(sizeof(c_net_request_exception_str)) + IDEAL_WORKING_BUF_SIZE - XOR32(sizeof(c_net_fragment));

    formatA(ex_msg->buffer, XOR("%s"), buf);
    ex_msg->buffer_len = strlen(buf);
    ex_msg->transmit(true);

  #if defined(INTERNAL_CHEAT) && !defined(VAC_BYPASS_MODULE)
    MessageBoxA(nullptr, buf, nullptr, XOR32(MB_ICONERROR) | XOR32(MB_SETFOREGROUND));
  #endif
  }

  static bool from_handler(_EXCEPTION_POINTERS* ex_info, i8* game_name){
    if(ex_info == nullptr)
      return true;

    if(ex_info->ExceptionRecord == nullptr || ex_info->ContextRecord == nullptr)
      return true;

    return handle_exception(ex_info->ExceptionRecord, ex_info->ContextRecord, game_name)  == EXCEPTION_CONTINUE_SEARCH;
  }

  static bool from_eh(EXCEPTION_RECORD* record, CONTEXT* registers, i8* game_name){
    if(record == nullptr || registers == nullptr)
      return true;

    return handle_exception(record, registers, game_name) == EXCEPTION_CONTINUE_SEARCH;
  }

  static LONG handle_exception(EXCEPTION_RECORD* record, CONTEXT* registers, i8* game_name){
    ul64 code = record->ExceptionCode;
    switch(code){
      default: break;
      case 0x406D1388: // thread naming
      case 0x40010006: // DBG_CONTROL_C
      case 0xE06D7363: // C++ exception
      case 0x4001000A: // Thread suspended
      case EXCEPTION_BREAKPOINT:
      case EXCEPTION_INT_DIVIDE_BY_ZERO:
        return EXCEPTION_CONTINUE_SEARCH;
    }

    write_exception(record, registers, game_name);

    // Always let the process try and handle their own damn exceptions.
    return EXCEPTION_CONTINUE_SEARCH;
  }

};
#endif


