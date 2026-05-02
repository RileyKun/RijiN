#pragma once


namespace str_utils{

  inline void trim_start(i8* str){
    if(str == nullptr)
      return;

    i8* trim_str = str;

    while(*trim_str && *trim_str == ' ')
      trim_str++;

    if(trim_str != str){
      i8* dest = str;
      while(*trim_str)
        *dest++ = *trim_str++;

      *dest = '\0';
    }
  }

  inline void trim_end(i8* str){
    if(str == nullptr)
      return;

    i8* end = str;

    while(*end)
      end++;

    while(end > str && *(end - 1) == ' ')
      end--;

    *end = '\0';
  }

  inline void trim(i8* str){
    trim_start(str);
    trim_end(str);
  }

  inline void trim_start(wchar_t* str){
    if(str == nullptr)
      return;

    wchar_t* trim_str = str;

    while(*trim_str && *trim_str == L' ')
      trim_str++;

    if(trim_str != str){
      wchar_t* dest = str;
      while(*trim_str)
        *dest++ = *trim_str++;

      *dest = L'\0';
    }
  }

  inline void trim_end(wchar_t* str){
    if(str == nullptr)
      return;

    wchar_t* end = str;

    while(*end)
      end++;

    while(end > str && *(end - 1) == L' ')
      end--;

    *end = L'\0';
  }

  inline void trim(wchar_t* str){
    trim_start(str);
    trim_end(str);
  }

  inline bool equal(std::wstring str1, std::wstring str2){
    if(str1.length() != str2.length())
      return false;

    for(u32 i = 0; i < str1.length(); i++)
      if(str1[i] != str2[i])
        return false;
    

    return true;
  }

  inline bool equal(std::string str1, std::string str2){
    if(str1.length() != str2.length())
      return false;

    for(u32 i = 0; i < str1.length(); i++)
      if(str1[i] != str2[i])
        return false;
    
    return true;
  }

  inline bool contains(std::wstring str1, std::wstring what){
    if(str1.empty() || what.empty() || what.length() > str1.length())
      return false; 

    u32 c = 0;
    for(u32 i = 0; i < str1.length(); i++){
      if(str1[i] != what[c]){
        c = 0;
        continue;
      }

      c++;
      if(c >= what.length())
        break;
    }

    return c == what.length();
  }

  inline bool contains(std::string str1, std::string what){
    if(str1.empty() || what.empty() || what.length() > str1.length())
      return false; 

    u32 c = 0;
    for(u32 i = 0; i < str1.length(); i++){
      if(str1[i] != what[c]){
        c = 0;
        continue;
      }

      c++;
      if(c >= what.length())
        break;
    }

    return c == what.length();
  }

  inline i8* tolower(i8* str){
    for(u32 i = 0; i < strlen(str); i++)
      str[i] = (str[i] >= 'A' && str[i] <= 'Z' ? (str[i] | (1 << 5)) : str[i]);

    return str;
  }

  inline wchar_t* tolower(wchar_t* str){
    for(u32 i = 0; i < wcslen(str); i++)
      str[i] = (str[i] >= L'A' && str[i] <= L'Z' ? (str[i] | (1 << 5)) : str[i]);

    return str;
  }

  inline i32 toupper(const i32 c){
    return (c >= 97 && c <= 122) ? (c - 32) : c;
  }

  inline wchar_t toupper(const wchar_t c){
    return (c >= 97 && c <= 122) ? (c - 32) : c;
  }

  inline i8* toupper(i8* str){
    for(u32 i = 0; i < strlen(str); i++)
      str[i] = toupper(str[i]);

    return str;
  }

  inline wchar_t* toupper(wchar_t* str){
    for(u32 i = 0; i < wcslen(str); i++)
      str[i] = toupper(str[i]);

    return str;
  }

  static std::string str_replace_all(const std::string input, const std::string find, const std::string replace) {
    if (find.empty())
      return input;

    std::string result;
    size_t pos = 0;
    size_t match;

    while ((match = input.find(find, pos)) != std::string::npos) {
      result.append(input, pos, match - pos);
      result.append(replace);
      pos = match + find.length();
    }

    result.append(input, pos, input.length() - pos);
    return result;
  }

  static bool is_hex(i8* str){
    while (*str) {
      if (!((*str >= '0' && *str <= '9') || (*str >= 'A' && *str <= 'F') || (*str >= 'a' && *str <= 'f')))
        return false;

      str++;
    }
    return true;
  }

  static bool is_hex(wchar_t* str){
    while (*str) {
      if (!((*str >= '0' && *str <= '9') || (*str >= 'A' && *str <= 'F') || (*str >= 'a' && *str <= 'f')))
        return false;

      str++;
    }
    return true;
  }

};