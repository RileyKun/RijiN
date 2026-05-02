#pragma once

namespace convert{
inline bool is_number(i8 c);
inline bool is_number(wchar_t c);
inline bool is_number(std::wstring str);
inline bool is_number(std::string str);

inline u32 hex_str_to_u32(i8* hex){
  if(hex[0] == '#')
    hex++;

  u32 value = 0;
  for(int i = 0; hex[i] && i < 8; ++i){
    i8 c = hex[i];
    value <<= 4;
    if(c >= '0' && c <= '9')
      value |= (c - '0');
    else if(c >= 'A' && c <= 'F')
      value |= (c - 'A' + 10);
    else if(c >= 'a' && c <= 'f')
      value |= (c - 'a' + 10);
  }

  return value;
}

inline u32 hex_str_to_rgba_u32(i8* hex){
  if(hex[0] == '#')
    hex++;

  u32 value = 0;
  i32 len = 0;

  for(i32 i = 0; hex[i] && i < 8; ++i){
    i8 c = hex[i];
    value <<= 4;
    if(c >= '0' && c <= '9')
      value |= (c - '0');
    else if(c >= 'A' && c <= 'F')
      value |= (c - 'A' + 10);
    else if(c >= 'a' && c <= 'f')
      value |= (c - 'a' + 10);

    len++;
  }

  if(len == 6){
    u32 r = (value >> 16) & 0xFF;
    u32 g = (value >> 8)  & 0xFF;
    u32 b = (value)       & 0xFF;
    return (255 << 24) | (b << 16) | (g << 8) | r;
  }
  else if(len == 8){
    u32 r = (value >> 24) & 0xFF;
    u32 g = (value >> 16) & 0xFF;
    u32 b = (value >> 8)  & 0xFF;
    u32 a = (value)       & 0xFF;
    return (a << 24) | (b << 16) | (g << 8) | r;
  }

  return 0;
}

inline colour hex_str_to_colour(i8* hex){
  if(hex[0] == '#')
    hex++;

  u32 value = 0;
  i32 len = 0;

  for(i32 i = 0; hex[i] && i < 8; ++i){
    i8 c = hex[i];
    value <<= 4;
    if(c >= '0' && c <= '9')
      value |= (c - '0');
    else if(c >= 'A' && c <= 'F')
      value |= (c - 'A' + 10);
    else if(c >= 'a' && c <= 'f')
      value |= (c - 'a' + 10);

    len++;
  }

  if(len == 6){
    u32 r = (value >> 16) & 0xFF;
    u32 g = (value >> 8)  & 0xFF;
    u32 b = (value)       & 0xFF;
    return colour(r, g, b, 255);
  }
  else if(len == 8){
    u32 r = (value >> 24) & 0xFF;
    u32 g = (value >> 16) & 0xFF;
    u32 b = (value >> 8)  & 0xFF;
    u32 a = (value)       & 0xFF;
    return colour(r, g, b, a);
  }

  return colour(0, 0, 0, 255);
}

inline i32 str_to_i32(i8* str){
  if(str == nullptr)
    return 0;

  i32 out = 0;
  bool is_neg = (str[0] == '-');

  for(i32 i = 0; i < strlen(str); i++){
    if(!is_number(str[i]))
      return 0;

    out = out * 10 + (str[i] - '0');
  }

  if(is_neg)
    out *= -1;

  return out;
}

inline i64 str_to_i64(i8* str){
  if(str == nullptr)
    return 0;

  i32 out = 0;
  bool is_neg = (str[0] == '-');

  for(i32 i = 0; i < strlen(str); i++){
    if(!is_number(str[i]))
      return 0;

    out = out * 10 + (str[i] - '0');
  }

  if(is_neg)
    out *= -1;

  return out;
}

  inline double str_to_double(i8* str){
    if(str == nullptr)
      return 0.0;

    double result = 0.0;
    i32    i      = 0;
    i32    sign   = 1;

    if(str[i] == '-') {
      sign = -1;
      i++;
    } 
    else if(str[i] == '+')
      i++;
    
    while(str[i] >= '0' && str[i] <= '9'){
      result = result * 10 + (str[i] - '0');
      i++;
    }

    if(str[i] == '.'){
      float decimal = 0.1;
      i++;

      while(str[i] >= '0' && str[i] <= '9') {
        result = result + (str[i] - '0') * decimal;
        decimal *= 0.1;
        i++;
      }
    }

    return result * sign;
  }

  inline float str_to_float(i8* str){
    return (float)str_to_double(str);
  }

  inline std::string float_to_str(float value, i32 frac_count = 2){
    std::string result;
    
    // Handle negative values
    if (value < 0) {
        result += '-';
        value = -value;
    }

    i32 int_part = (i32)(value);
    float frac_part = value - int_part;

    std::string int_str;
    while (int_part > 0) {
        int_str = i8('0' + int_part % 10) + int_str;
        int_part /= 10;
    }

    if (int_str.empty()) 
        int_str = "0";
    
    result += int_str;

    if(frac_count < 0)
      frac_count = 0;
    else if(frac_count > 7)
      frac_count = 7;

    // Convert fractional part to string
    if (frac_part > 0) {
      result += '.';
      for (i32 i = 0; i < frac_count; ++i){
          frac_part *= 10;
          i32 digit = (i32)(frac_part);
          result += i8('0' + digit);
          frac_part -= digit;
      }
    }

    return result;
  }

  inline std::string double_to_str(double value, i32 frac_count = 2){
    std::string result;
    
    // Handle negative values
    if (value < 0) {
        result += '-';
        value = -value;
    }

    i32 int_part = (i32)(value);
    double frac_part = value - int_part;

    std::string int_str;
    while (int_part > 0) {
        int_str = i8('0' + int_part % 10) + int_str;
        int_part /= 10;
    }

    if (int_str.empty()) 
        int_str = "0";
    
    result += int_str;

    if(frac_count < 0)
      frac_count = 0;
    else if(frac_count > 15)
      frac_count = 15;
    
    // Convert fractional part to string
    if (frac_part > 0) {
      result += '.';
      for (i32 i = 0; i < frac_count; ++i){
          frac_part *= 10;
          i32 digit = (i32)(frac_part);
          result += i8('0' + digit);
          frac_part -= digit;
      }
    }

    return result;
  }

  // We could probably keep the other functions inside the utils namespace, or just replace them w/e.
  inline std::wstring str2wstr(std::string str){
    return std::wstring(str.begin(), str.end());
  }

  inline std::string wstr2str(std::wstring str){
    return std::string(str.begin(), str.end());
  }

  inline void str2wstr(i8* str, wchar_t* out_wstr, i32 size){
    if(str == nullptr){
      assert(str != nullptr);
      return;
    }

    if(out_wstr == nullptr){
      assert(out_wstr != nullptr);
      return;
    }

    assert(out_wstr != nullptr);
    wsprintfW(out_wstr, XOR(L"%s"), str);
    MultiByteToWideChar(CP_UTF8, 0, str, -1, out_wstr, size);
  }

  inline bool is_number(i8 c){
    return c >= '0' && c <= '9';
  }

  inline bool is_number(wchar_t c){
    return c >= L'0' && c <= L'9';
  }

  inline bool is_number(std::wstring str){
    if(str.empty())
      return false;

    for(u32 i = 0; i < str.length(); i++){
      if(!is_number(str[i]))
        return false;
    }

    return true;
  }

  inline bool is_number(std::string str){
    if(str.empty())
      return false;

    for(u32 i = 0; i < str.length(); i++){
      if(!is_number(str[i]))
        return false;
    }

    return true;
  }

  static bool str_sid3_to_u32(i8* str, u32* out){
    if(str == nullptr || out == nullptr){
      assert(false && "str_sid3_to_u32 nullptr parameter.");
      return false;
    }

    if(str[0] != '['){
      DBG("[-] str_sid3_to_u32 error: \"%s\"\n", str);
      assert(false && "str_sid3_to_u32 expecting [U:1:X] format");
      return false;
    }

    u32 len = strlen(str);
    if(len <= 6)
      return false;

    i32 c = 0;
    i8  num[128];
    memset(num, 0, sizeof(num));
    for(u32 i = 5; i < len; i++){
      if(!is_number(str[i]))
        continue;

      num[c] = str[i];
      c++;
      if(c >= sizeof(num))
        break;
    }


    *out = (u32)convert::str_to_i32(num);
    return c > 0;
  }
};