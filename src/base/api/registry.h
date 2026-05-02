#pragma once


struct s_reg_key_data{
  i8 name[1024];
  i8 value[1024];
};

namespace reg{
 
  inline wchar_t* read_key_str(std::wstring dir, std::wstring key_name, u32 region = HKEY_CURRENT_USER, uptr* error = nullptr){
    if(dir.empty() || key_name.empty())
      return nullptr;

    HKEY key;
    uptr result = I(RegOpenKeyExW)(region, dir.c_str(), 0, XOR32(KEY_READ), &key);
    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      return nullptr;
    }

    u32 size     = XOR32(0x512);
    wchar_t* str = malloc(size);
    if(str == nullptr){
      if(error != nullptr)
        *error = 0x8;

      I(RegCloseKey)(key);
    }

    I(memset)(str, 0, size);

    result = I(RegQueryValueExW)(key, key_name.c_str(), nullptr, nullptr, (i8*)str, &size);
    I(RegCloseKey)(key);

    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      I(free)(str);
      return nullptr;
    }

    return str;
  }

  inline i8* read_key_str(std::string dir, std::string key_name, u32 region = HKEY_CURRENT_USER, uptr* error = nullptr){
    if(dir.empty() || key_name.empty())
      return nullptr;

    HKEY key;
    uptr result = I(RegOpenKeyExA)(region, dir.c_str(), 0, XOR32(KEY_READ), &key);
    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      return nullptr;
    }

    u32 size     = XOR32(0x512);
    i8* str = malloc(size);
    if(str == nullptr){
      if(error != nullptr)
        *error = 0x8;

      I(RegCloseKey)(key);
    }

    I(memset)(str, 0, size);

    result = I(RegQueryValueExA)(key, key_name.c_str(), nullptr, nullptr, (i8*)str, &size);
    I(RegCloseKey)(key);

    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      I(free)(str);
      return nullptr;
    }

    return str;
  }

  inline i32 read_key_i32(std::wstring dir, std::wstring key_name, u32 region = HKEY_CURRENT_USER, uptr* error = nullptr){
    if(dir.empty() || key_name.empty())
      return 0;

    HKEY key;
    uptr result = I(RegOpenKeyExW)(region, dir.c_str(), 0, XOR32(KEY_READ), &key);
    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      return 0;
    }

    u32 size = XOR32(sizeof(i32));
    i32 val  = 0;

    result = I(RegQueryValueExW)(key, key_name.c_str(), nullptr, nullptr, (i8*)&val, &size);
    I(RegCloseKey)(key);

    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      return 0;
    }

    return val;
  }

  // Issue: float precision is not 100% accurate.
  inline float read_key_float(std::wstring dir, std::wstring key_name, u32 region = HKEY_CURRENT_USER, uptr* error = nullptr){
    if(dir.empty() || key_name.empty())
      return 0.f;

    HKEY key;
    uptr result = I(RegOpenKeyExW)(region, dir.c_str(), 0, XOR32(KEY_READ), &key);
    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      return 0.f;
    }

    u32   size   = XOR32(sizeof(float));
    float val  = 0.f;
    u32   type = 0;

    result = I(RegQueryValueExW)(key, key_name.c_str(), nullptr, &type, reinterpret_cast<i8*>(&val), &size);
    I(RegCloseKey)(key);

    if(type != REG_BINARY || size != XOR32(sizeof(float))) {
      if(error != nullptr)
        *error = ERROR_INVALID_DATA;

      return 0.f;
    }

    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      return 0.f;
    }

    return val;
  }

  inline i64 read_key_i64(std::wstring dir, std::wstring key_name, u32 region = HKEY_CURRENT_USER, uptr* error = nullptr){
    if(dir.empty() || key_name.empty())
      return 0;

    HKEY key;
    uptr result = I(RegOpenKeyExW)(region, dir.c_str(), 0, XOR32(KEY_READ), &key);
    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      return 0;
    }

    u32 size = XOR32(sizeof(i64));
    i64 val  = 0;

    result = I(RegQueryValueExW)(key, key_name.c_str(), nullptr, nullptr, (i8*)&val, &size);
    I(RegCloseKey)(key);

    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      return 0;
    }

    return val;
  }

  inline bool write_key_i32(std::wstring dir, std::wstring key_name, i32 data, u32 region = HKEY_CURRENT_USER, uptr* error = nullptr){
    if(dir.empty() || key_name.empty())
      return false;


    HKEY key;
    uptr result = I(RegCreateKeyExW)(region, dir.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &key, nullptr);
    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      return false;
    }

    result = I(RegSetValueExW)(key, key_name.c_str(), 0, REG_DWORD, (i8*)&data, sizeof(i32));
    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      I(RegCloseKey)(key);
      return false;
    }

    I(RegCloseKey)(key);
    return true;
  }

  inline bool write_key_float(std::wstring dir, std::wstring key_name, float data, u32 region = HKEY_CURRENT_USER, uptr* error = nullptr){
    if(dir.empty() || key_name.empty())
      return false;

    HKEY key;
    uptr result = I(RegCreateKeyExW)(region, dir.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &key, nullptr);
    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      return false;
    }

    result = I(RegSetValueExW)(key, key_name.c_str(), 0, REG_BINARY, (i8*)&data, sizeof(float));
    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      I(RegCloseKey)(key);
      return false;
    }

    I(RegCloseKey)(key);
    return true;
  }

  inline bool write_key_str(std::wstring dir, std::wstring key_name, std::wstring data, u32 region = HKEY_CURRENT_USER, uptr* error = nullptr){
    if(dir.empty() || key_name.empty())
      return false;

    HKEY key;
    uptr result = I(RegCreateKeyExW)(region, dir.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &key, nullptr);
    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      return false;
    }

    u32 total_size = (data.length() + 1) * sizeof(wchar_t);

    result = I(RegSetValueExW)(key, key_name.c_str(), 0, REG_SZ, (const u8*)data.c_str(), total_size);
    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      I(RegCloseKey)(key);
      return false;
    }

    I(RegCloseKey)(key);
    return true;
  }

  inline s_reg_key_data* get_keys(std::wstring path, u32& size, i32 region = HKEY_CLASSES_ROOT, uptr* error = nullptr){
    HKEY key;
    uptr result = I(RegOpenKeyExW)(region, path.c_str(), 0, KEY_READ, &key);
    if(result != ERROR_SUCCESS){
      if(error != nullptr)
        *error = result;

      return nullptr;
    }

    result = RegQueryInfoKeyA(key, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &size, nullptr, nullptr, nullptr, nullptr);
    if(result != ERROR_SUCCESS || size <= 0){
      if(error != nullptr)
        *error = result;
      
      I(RegCloseKey)(key);
      return nullptr;
    }

    static s_reg_key_data key_data;
    I(memset)(&key_data, 0, sizeof(s_reg_key_data));

    s_reg_key_data* table = I(malloc)(sizeof(s_reg_key_data) * size);

    if(table == nullptr){
      if(error != nullptr)
        *error = 0x8;

      I(RegCloseKey)(key);
      return nullptr;
    }

    u32 written_count = 0;
    for(u32 i = 0; i < size; i++){ 
      u32 max_name_size  = sizeof(key_data.name);
      u32 max_value_size = sizeof(key_data.value);
      u32 data_type      = 0;

      result = I(RegEnumValueA)(key, i, key_data.name, &max_name_size, nullptr, &data_type, (u8*)&key_data.value, &max_value_size);
      if(result != ERROR_SUCCESS){
        if(error != nullptr)
          *error = result;

        break;  
      }

      // REG_DWORD
      if(data_type == XOR32(4)){
        i32 value       = 0;
        max_value_size  = XOR32(4);
        if(I(RegEnumValueA)(key, i, key_data.name, &max_name_size, nullptr, &data_type, (i8*)&value, &max_value_size) == ERROR_SUCCESS)
          I(wsprintfA)(key_data.value, XOR("%i"), value);
      }

      I(memcpy)(&table[i], &key_data, sizeof(s_reg_key_data));
      written_count++;
    }

    I(RegCloseKey)(key);

    // If there's no entries then just free the table and return nullptr.
    if(written_count <= 0){
      I(free)(table);
      return nullptr;
    }

    return table;
  }

  inline LONG delete_registry_tree(HKEY key_root, std::wstring sub_key) {
    HKEY key;
    LONG result = RegOpenKeyExW(key_root, sub_key.c_str(), 0, KEY_READ | KEY_WRITE, &key);
    if (result != ERROR_SUCCESS)
      return result;

    wchar_t sub_key_name[256];
    DWORD sub_key_name_size = sizeof(sub_key_name) / sizeof(sub_key_name[0]);
    while (RegEnumKeyExW(key, 0, sub_key_name, &sub_key_name_size, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
      result = delete_registry_tree(key, std::wstring(sub_key_name));
      if (result != ERROR_SUCCESS) {
        RegCloseKey(key);
        return result;
      }
      sub_key_name_size = sizeof(sub_key_name) / sizeof(sub_key_name[0]); // Reset buffer size
    }

    // Close the key handle and delete the key itself
    RegCloseKey(key);
    result = RegDeleteKeyW(key_root, sub_key.c_str());
    return result;
  }
};