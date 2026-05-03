#include "link.h"

CLASS_ALLOC(c_global, global);

void c_global::handle_creds(bool* should_save_logins, bool* attempt_auto_login){
  if(should_save_logins == nullptr || attempt_auto_login == nullptr)
    return;

  #if defined(DEV_MODE)
    memset(&up, 0, sizeof(up_data));
    up.username_len = 10;
    up.password_len = 10;
    I(strcpy)(up.username, XOR("dev"));
    I(strcpy)(up.password, XOR("Unaired Clique Judiciary Italicize9 Halt"));
    return;
  #endif

  if(utils::load_up()){
    *attempt_auto_login = false;
    return;
  }

  uptr result = 0;
  if(!utils::prompt_up(should_save_logins, &result)){
    prompt_fail(result);
    return;
  }

  std::string invalid_reason = "";
  if(up.username_len < LOGIN_MIN_USERNAME_SIZE)
    invalid_reason = XOR("Username is too short");
  else if(up.username_len > LOGIN_MAX_USERNAME_SIZE)
    invalid_reason = XOR("Username is too large");
  else if(up.password_len < LOGIN_MIN_PASSWORD_SIZE)
    invalid_reason = XOR("Password is too small");
  else if(up.password_len > LOGIN_MAX_PASSWORD_SIZE)
    invalid_reason = XOR("Password is too large");

  if(!invalid_reason.empty()){
    show_loading_page(WXOR(L"Received invalid login details"));
    I(MessageBoxA)(nullptr, invalid_reason.c_str(), "", MB_ICONERROR | MB_TOPMOST);
    I(ExitProcess)(1);
  }
}

void c_global::prompt_fail(uptr result){
  if(result == ERROR_CANCELLED){
    I(ExitProcess)(0);
    return;
  }

  switch(result){
    default:{
      SHOW_ERROR_STATUS_STR(ERR_FAILED_TO_CREATE_LOGIN_PROMPT, result, "Your windows install is preventing the loader from opening a login prompt.", true);
      break;
    }
    case ERROR_ACCESS_DENIED:{
      SHOW_ERROR_STR(ERR_FAILED_TO_CREATE_LOGIN_PROMPT, "The loader is unable to create a login prompt due to system restrictions.\n - Anti-virus\n - Loader not running as administrator.", true);
      break;
    }
    case 0xFFFF:{
      SHOW_ERROR_STR(ERR_FAILED_TO_CREATE_LOGIN_PROMPT, "Windows is setup to block our login prompt window.", true);
      break;
    }
  }

  I(ExitProcess)(1000);
}