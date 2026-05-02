#pragma once

#define FILE_MAP_MOD 32768

struct s_file_map_data{
  u32    hash;
  HANDLE handle;
};

class c_file_map{
public:

  ALWAYSINLINE inline u32 get_kernel_cookie(){
    KUSER_SHARED_DATA* data = (KUSER_SHARED_DATA*)(XOR32(0x7FFE0000));
    return data->Cookie;
  }

  NEVERINLINE bool create(u32 hash){
    if(is_active(hash))
      return false;

    i8 buf[IDEAL_MIN_BUF_SIZE];
    formatA(buf, XOR("%X"), hash + (get_kernel_cookie() % XOR32(FILE_MAP_MOD)));
    HANDLE handle = I(CreateFileMappingA)(INVALID_HANDLE_VALUE, nullptr, XOR32(PAGE_READWRITE), 0, XOR32(4), buf);
    if(!IS_HANDLE_VALID(handle)){
      ul64 error_code = I(GetLastError)();
      DBG("[!] c_file_map::create - error_code: %X\n", error_code);
      return false;
    }

    table[count].hash   = hash;
    table[count].handle = handle;

    count++;
    if(count >= 32)
      count = 0;

    return true;
  }

  NEVERINLINE bool destroy(u32 hash){
    for(u32 i = 0; i < 32; i++){
      if(!IS_HANDLE_VALID(table[i].handle))
        continue;

      if(table[i].hash != hash)
        continue;

      I(CloseHandle)(table[i].handle);

      table[i].hash   = 0;
      table[i].handle = nullptr;
      break;
    }

    return true;
  }

  NEVERINLINE bool is_active(u32 hash){
    i8 buf[IDEAL_MIN_BUF_SIZE];
    formatA(buf, XOR("%X"), hash + (get_kernel_cookie() % XOR32(FILE_MAP_MOD)));
    HANDLE handle = I(OpenFileMappingA)(XOR32(FILE_MAP_ALL_ACCESS), false, buf);
    if(!IS_HANDLE_VALID(handle))
      return false;

    I(CloseHandle)(handle);
    return true;
  }

  u32             count;
  s_file_map_data table[32];
};

CLASS_EXTERN(c_file_map, file_map);