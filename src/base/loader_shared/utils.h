#pragma once

extern i32         error_code;
extern i32         error_line;
extern ul64        error_status_code;
extern std::string error_msg;

namespace utils{
  inline std::string format(std::string format, ... ){
    va_list list;
    va_start( list, format.c_str() );
  
    i8 buffer[4096] = { 0 };
    I(vsnprintf)( buffer, 4096, format.c_str(), list );
  
    va_end( list );
  
    return std::string( buffer );
  }
  
  #define LISTEN_FOR_ERRORS()  utils::listen_for_errors();
  #define SHOW_ERROR(num, exit) utils::error(XOR32(num), XOR32(__LINE__), exit)
  #define SHOW_ERROR_STR(num, str, exit) utils::error(XOR32(num), XOR32(__LINE__), XOR(str), exit)
  #define SHOW_ERROR_STATUS(num, status_code, exit) utils::error(XOR32(num), XOR32(__LINE__), status_code, exit)
  #define SHOW_ERROR_STATUS_STR(num, status_code, str, exit) utils::error(XOR32(num), XOR32(__LINE__), status_code, XOR(str), exit)
  #define SHOW_ERROR_STR_RAW(num, str, exit) utils::error(XOR32(num), XOR32(__LINE__), str, exit)

  inline void listen_for_errors(){
    utils::create_worker([](void* unk){
      while(true){
        if(!error_code){
          I(Sleep)(1);
          continue;
        }

        // Let the client tell us what occured!
        #if !defined(DISABLE_NETWORKING)
          utils::submit_error_code(error_code, error_status_code);
        #endif

        std::string str;

        i8 buf[1024] = {0};
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error_status_code, 0, buf, sizeof(buf), nullptr);

        if(error_line != -1)
          str = utils::format(XOR("Error: %i\nLine: %i\n\n%s"), error_code, error_line, error_msg.c_str());
        else
          str = utils::format(XOR("Error: %i\n\n%s"), error_msg.c_str());

        if(error_status_code)
          str += utils::format(XOR("\n0x%X - %s"), error_status_code, buf);

        I(MessageBoxA)(nullptr, str.c_str(), nullptr, XOR32(MB_ICONERROR) | XOR32(MB_TOPMOST));

        error_code        = 0;
        error_line        = 0;
        error_status_code = 0;
        error_msg         = "";
        I(ExitProcess)(0);
        break;
      }
      return 0;
    }, nullptr);
  }

  inline void internal_error(i32 error_num, i32 line, std::string msg = "", ul64 status_code = 0, bool sexit = true){
    error_code        = error_num;
    error_line        = line;
    error_status_code = status_code;
    error_msg         = msg;

    I(Sleep)(XOR32(10000));
  }

  inline void error(i32 error_num, i32 line, bool sexit){
    internal_error(error_num, line, "", 0, sexit);
  }

  inline void error(i32 error_num, i32 line, std::string msg, bool sexit){
    internal_error(error_num, line, msg, 0, sexit);
  }

  inline void error(i32 error_num, i32 line, u64 status_code, bool sexit){
    internal_error(error_num, line, "", status_code, sexit);
  }

  inline void error(i32 error_num, i32 line, u64 status_code, std::string msg, bool sexit){
    internal_error(error_num, line, msg, status_code, sexit);
  }
};