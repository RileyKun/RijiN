#include "link.h"

CFLAG_O0 bool utils::load_up(){
  i32 file_len = 0;
  u8* buffer = file::read_file(LOGIN_FILE_NAME, file_len);
  if(buffer == nullptr || file_len != (sizeof(up_data)))
    return false;

  xor_up(buffer, file_len);

  memcpy(&global->up, buffer, sizeof(up_data));
  I(free)(buffer);

  if(!check_up()){
    delete_up();
    return false;
  }

  return true;
}

bool utils::check_up(){
  global->up.username[CREDUI_MAX_USERNAME_LENGTH - 1] = '\0';
  global->up.username[CREDUI_MAX_USERNAME_LENGTH - 1] = '\0';

  if(global->up.username_len >= CREDUI_MAX_USERNAME_LENGTH || global->up.password_len >= CREDUI_MAX_PASSWORD_LENGTH)
    return false;

  u32 u_len = strlen(global->up.username);
  u32 p_len = strlen(global->up.password);
  if(global->up.username_len != u_len || global->up.password_len != p_len)
    return false;

  return true;
}

void utils::save_up(){
  if(!check_up())
    return;

  u8* up_tmp = malloc(XOR32(sizeof(up_data)));
  assert(up_tmp != nullptr);
  memcpy(up_tmp, &global->up, XOR32(sizeof(up_data)));

  xor_up(up_tmp, sizeof(up_data));

  file::write_file(LOGIN_FILE_NAME, up_tmp, sizeof(up_data));
  I(free)(up_tmp);
}

bool utils::delete_up(){
  return file::delete_file(LOGIN_FILE_NAME);
}

bool utils::prompt_up(bool* should_save_credentials = nullptr, uptr* _result){

  std::string caption_str = utils::format(XOR("RijiN"));
  std::wstring caption_text(caption_str.begin(), caption_str.end());

  std::string message_str = utils::format(XOR("Please enter your forum username and password."));
  std::wstring message_text(message_str.begin(), message_str.end());

  CREDUI_INFOW credui = {};
  credui.cbSize = sizeof(CREDUI_INFOW);
  credui.hwndParent = nullptr;
  credui.pszMessageText = message_text.c_str();
  credui.pszCaptionText = caption_text.c_str();
  credui.hbmBanner = nullptr;

  ul64  auth_package     = 0;
  void* out_cred_buffer = nullptr;
  ul64  out_cred_len     = 0;
  BOOL  save             = FALSE;

  i32 result = I(CredUIPromptForWindowsCredentialsW)(&credui, 0, &auth_package,
    nullptr,
    0,
    &out_cred_buffer,
    &out_cred_len,
    &save, 
    CREDUIWIN_CHECKBOX | CREDUIWIN_GENERIC);

  if(_result != nullptr)
    *_result = result;

  // ERROR
  if(result != NO_ERROR)
    return false;

  u32 max_username_len = CREDUI_MAX_USERNAME_LENGTH;
  u32 max_password_len = CREDUI_MAX_PASSWORD_LENGTH;
  static WCHAR username[CREDUI_MAX_USERNAME_LENGTH];
  static WCHAR password[CREDUI_MAX_PASSWORD_LENGTH];

  memset(username, 0, sizeof(username));
  memset(password, 0, sizeof(password));

  bool unpack_result = I(CredUnPackAuthenticationBufferW)(0, out_cred_buffer, out_cred_len, username, &max_username_len, NULL, 0, password, &max_password_len);
  if(!unpack_result){
    if(_result != nullptr)
      *_result = I(GetLastError)();

    return false;
  }

  I(sprintf)(global->up.username, XOR("%ls"), username);
  I(sprintf)(global->up.password, XOR("%ls"), password);

  global->up.username_len = I(strlen)(global->up.username);
  global->up.password_len = I(strlen)(global->up.password);

  if(should_save_credentials != nullptr)
    *should_save_credentials = save;

  return true;
}