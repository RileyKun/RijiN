#include "link.h"

bool file::is_invalid_filename(std::string str){
  bool bad_char = false;
  for(u32 i = 0; i < str.length(); i++){
    if(  str[i] == '#' || str[i] == '%' || str[i] == '&'
      || str[i] == '(' || str[i] == ')' || str[i] == '<' 
      || str[i] == '>' || str[i] == '<' || str[i] == '*'
      || str[i] == '?' || str[i] == '/' || str[i] == '\\'
      || str[i] == '$' || str[i] == '~' || str[i] == ':'
      || str[i] == '+' || str[i] == '|' || str[i] == '"' 
      || str[i] == '!' || str[i] == '@')
        return true;
  }

  i8* str_ptr = str.c_str();
  return str_utils::contains(str_ptr, XOR("CON")) || str_utils::contains(str_ptr, XOR("PRN"))
      || str_utils::contains(str_ptr, XOR("AUX")) || str_utils::contains(str_ptr, XOR("NUL"))
      || str_utils::contains(str_ptr, XOR("COM")) || str_utils::contains(str_ptr, XOR("LPT"))
      || str_utils::contains(str_ptr, XOR("NUL"));
}