#pragma once

namespace clipboard{
  static bool set_text(i8* text){
    if(text == nullptr)
      return false;

    if(!I(OpenClipboard)(nullptr))
      return false;

    if(!I(EmptyClipboard)())
      return false;

    const u32 len = strlen(text);
    i8*       handle = (i8*)I(GlobalAlloc)(GMEM_MOVEABLE, len + 1);
    if(handle == nullptr)
      return false;

    memcpy(I(GlobalLock)(handle), text, len);

    bool success = I(SetClipboardData)(CF_TEXT, handle) != NULL;

    I(CloseClipboard)();
    assert(I(GlobalFree)(handle) == NULL);

    return success;
  }

  static bool set_text(wchar_t* text){
    if(text == nullptr)
      return false;

    if(!I(OpenClipboard)(nullptr))
      return false;

    if(!I(EmptyClipboard)())
      return false;

    const u32 len = wcslen(text) * 2; // Copy on a per byte basis, and wchar_t are 2 bytes in size.
    i8*       handle = (i8*)I(GlobalAlloc)(GMEM_MOVEABLE, len + 1);
    if(handle == nullptr)
      return false;

    memcpy(I(GlobalLock)(handle), text, len);

    bool success = I(SetClipboardData)(CF_UNICODETEXT, handle) != NULL;

    I(CloseClipboard)();
    assert(I(GlobalFree)(handle) == NULL);

    return success;
  }

  static bool set_text(std::string& str){
    return set_text(str.c_str());
  }

  static bool set_text(std::wstring& str){
    return set_text(str.c_str());
  }

  static i8* get_text_alloc(){
    if(!I(OpenClipboard)(nullptr))
      return nullptr;

    void* data = I(GetClipboardData)(CF_TEXT);
    if(data == nullptr)
      return nullptr;

    i8* str = (i8*)(I(GlobalLock)(data));
    if(str == nullptr)
      return nullptr;

    I(GlobalUnlock)(data);
    I(CloseClipboard)();

    return str;
  }

  static wchar_t* get_wtext_alloc(){
    if(!I(OpenClipboard)(nullptr))
      return nullptr;

    void* data = I(GetClipboardData)(CF_UNICODETEXT);
    if(data == nullptr)
      return nullptr;

    wchar_t* str = (wchar_t*)(I(GlobalLock)(data));
    if(str == nullptr)
      return nullptr;

    I(GlobalUnlock)(data);
    I(CloseClipboard)();

    return str;
  }
};