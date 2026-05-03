#pragma once

namespace utils{
  static std::vector<i8*> processed_table;
  inline bool is_table_processed(i8* name){
    for(u32 i = 0; i < processed_table.size(); i++){
      if(!strcmp(processed_table[i], name))
        return true;
    }
    return false;
  }

  static void write_to_file(i8* file_name, i8* a, ...){
    i8 buf[128];
    {
      va_list list;
      va_start(list, a);
      vsprintf_s(buf, sizeof( buf ), a, list);
      va_end(list);
    }

    FILE* open = NULL;
    fopen_s(&open, file_name, "a");
    if(open){
      fprintf_s(open, "%s\n", buf);
      fclose(open);
    }
  }

  static void replace_text(std::string& str, const std::string from, const std::string to) {
    auto start_pos = str.find(from);
    if (start_pos != std::string::npos)
      str.replace(start_pos, from.length(), to);
  }


  static bool is_number(i8* str){
    if(str == nullptr)
      return false;

    for(u32 i = 0; i < strlen(str); i++){
      if(!(str[i] >= '0' && str[i] <= '9'))
        return false;
    }

    return true;
  }

  static std::string c_happy_str(i8* arg){
    if(arg == nullptr)
      return "";

    std::string new_str;
    for(u32 i = 0; i < strlen(arg); i++){
      if(arg[i] == '[' || arg[i] == ']' || arg[i] == '"' || arg[i] == '\'' || arg[i] == '.' || arg[i] == 32)
        continue;

      new_str += arg[i];
    } 

    // Add a standard of naming when going to other games so our base class can just work without any issues.

    replace_text(new_str, "DT_TFPlayerResource", "DT_PlayerResource");
    replace_text(new_str, "DT_DODPlayerResource", "DT_PlayerResource");
    replace_text(new_str, "DT_CSPlayerResource", "DT_PlayerResource");
    replace_text(new_str, "DT_SDKPlayerResource", "DT_PlayerResource");
    replace_text(new_str, "DT_TerrorPlayerResource", "DT_PlayerResource");

    replace_text(new_str, "DT_TFPlayer", "DT_SPlayer");
    replace_text(new_str, "DT_CSPlayer", "DT_SPlayer");
    replace_text(new_str, "DT_DODPlayer", "DT_SPlayer");
    replace_text(new_str, "DT_HL2MP_Player", "DT_SPlayer");
    replace_text(new_str, "DT_GMOD_Player", "DT_SPlayer");
    replace_text(new_str, "DT_TerrorPlayer", "DT_SPlayer");

    replace_text(new_str, "DT_CSLocalPlayerExclusive", "DT_LocalPlayerExclusive");
    replace_text(new_str, "DT_TFLocalPlayerExclusive", "DT_LocalPlayerExclusive");
    replace_text(new_str, "DT_SDKLocalPlayerExclusive", "DT_LocalPlayerExclusive");
    replace_text(new_str, "DT_HL2MPLocalPlayerExclusive", "DT_LocalPlayerExclusive");
    replace_text(new_str, "DT_FoFLocalPlayerExclusive06", "DT_LocalPlayerExclusive");
    replace_text(new_str, "DT_TerrorLocalPlayerExclusive", "DT_LocalPlayerExclusive");

    replace_text(new_str, "DT_SDKNonLocalPlayerExclusive", "DT_NonLocalPlayerExclusive");
    replace_text(new_str, "DT_HL2MPNonLocalPlayerExclusive", "DT_NonLocalPlayerExclusive");

    return new_str;
  }
};