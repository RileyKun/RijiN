#pragma once

namespace file{
  inline bool is_invalid_filename(std::string str);
  inline bool does_directory_exist(std::string path){
    u32 attr = I(GetFileAttributesA)(path.c_str());
    return (attr != 0xFFFFFFFF) && attr & FILE_ATTRIBUTE_DIRECTORY && !(attr & FILE_ATTRIBUTE_OFFLINE);
  }

  inline bool does_directory_exist(std::wstring path){
    u32 attr = I(GetFileAttributesW)(path.c_str());
    return (attr != 0xFFFFFFFF) && attr & FILE_ATTRIBUTE_DIRECTORY && !(attr & FILE_ATTRIBUTE_OFFLINE);
  }

  inline bool does_file_exist(std::string path){
    u32 attr = I(GetFileAttributesA)(path.c_str());
    return (attr != 0xFFFFFFFF) && !(attr & FILE_ATTRIBUTE_DIRECTORY || attr & FILE_ATTRIBUTE_OFFLINE);
  }

  inline bool does_file_exist(std::wstring path){
    u32 attr = I(GetFileAttributesW)(path.c_str());
    return (attr != 0xFFFFFFFF) && !(attr & FILE_ATTRIBUTE_DIRECTORY || attr & FILE_ATTRIBUTE_OFFLINE);
  }

  inline bool delete_file(std::string path){
    if(!DeleteFileA(path.c_str())){
      if(!SetFileAttributesA(path.c_str(), FILE_ATTRIBUTE_NORMAL))
        return false;

      return DeleteFileA(path.c_str());
    } 

    return true;
  }

  inline bool delete_file(std::wstring path){
    if(!DeleteFileW(path.c_str())){
      if(!SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL))
        return false;

      return DeleteFileW(path.c_str());
    } 

    return true;
  }

  inline bool delete_directory(std::wstring path, bool recur = false, uptr* error = nullptr){  
    if(!recur){
      bool result = I(RemoveDirectoryW)(path.c_str());
      if(error != nullptr)
        *error = I(GetLastError)();

      return result;
    }
    else{
      std::wstring pattern = path + WXOR(L"\\*");

      WIN32_FIND_DATAW data;
      memset(&data, 0, sizeof(WIN32_FIND_DATAW));

      HANDLE handle = I(FindFirstFileW)(pattern.c_str(), &data);
      if(error != nullptr)
        *error = I(GetLastError)();

      if(!IS_HANDLE_VALID(handle))
        return false;

      while(true){
        if(data.cFileName[0] == '.'){
          if(I(FindNextFileW)(handle, &data) == 0)
            break;

          continue;
        }

        std::wstring file_name = data.cFileName;
        std::wstring full_path = path + WXOR(L"\\") + file_name;

        if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && recur){
          DBG("[!] Recusively deleting %ls\n", full_path.c_str());
          if(!delete_directory(full_path, true))
            return false;
        }
        else{ 
          DBG("[!] Deleting file: %ls\n", full_path.c_str());
          delete_file(full_path);
        }

        if(I(FindNextFileW)(handle, &data) == 0)
          break;
      }

      bool result = I(RemoveDirectoryW)(path.c_str());
      if(error != nullptr)
        *error = I(GetLastError)();

      I(FindClose)(handle);
      return result;
    }

    return true;
  }

  inline bool delete_file_type(std::wstring path, std::wstring pattern, bool recur = false, uptr* error = nullptr){
    if(!wcsstr(pattern.c_str(), WXOR(L"."))){
      DBG("[-] delete_file_type: pattern doesn't contain .")
      return false;
    }

    assert(wcsstr(pattern.c_str(), WXOR(L"*")) == 0);

    WIN32_FIND_DATAW data;
    memset(&data, 0, sizeof(WIN32_FIND_DATAW));

    std::wstring r_pattern = (path + WXOR(L"*.*"));

    HANDLE handle = I(FindFirstFileW)(r_pattern.c_str(), &data);
    if(error != nullptr)
      *error = I(GetLastError)();

    if(!IS_HANDLE_VALID(handle)){
      DBG("[!] delete_file_type: handle is not valid.\n");
      return false;
    }
  
    while(true){
      if(data.cFileName[0] == '.'){
        if(I(FindNextFileW)(handle, &data) == 0)
          break;
        
        continue;
      }

      std::wstring file_name = data.cFileName;
      std::wstring full_path = path + WXOR(L"\\") + file_name;

      if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && recur){
        if(!delete_file_type(full_path + WXOR(L"\\"), pattern, recur))
          return false;
      }
      else{ 
        if(!wcsstr(file_name.c_str(), pattern.c_str())){
          if(I(FindNextFileW)(handle, &data) == 0)
            break;

          DBG("[!] %ls != %ls\n", file_name.c_str(), pattern.c_str());
          continue;
        }

        DBG("[!] Deleting file: %ls\n", full_path.c_str());
        delete_file(full_path);
      }

      if(I(FindNextFileW)(handle, &data) == 0)
        break;
    }


    I(FindClose)(handle);
    return true;
  }

  inline bool write_file(std::wstring file_name, void* buffer, u32 len, uptr* error = nullptr){
    void* handle = I(CreateFileW)(file_name.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if(error != nullptr)
      *error = I(GetLastError)();

    if(handle == nullptr)
      return false;

    ul64 wrote_bytes = 0;

    bool result = I(WriteFile)(handle, buffer, len, (ul64*)&wrote_bytes, nullptr);

    if(error != nullptr)
      *error = I(GetLastError)();

    I(CloseHandle)(handle);
    if(!result)
      return false;

    return result;
  }

  inline void* read_file(std::wstring file_name, i32& file_len, uptr* error = nullptr){
    HANDLE handle = I(CreateFileW)(file_name.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if(error != nullptr)
      *error = I(GetLastError)();

    if(!IS_HANDLE_VALID(handle))
      return nullptr;
    
    file_len = I(GetFileSize)(handle, nullptr);
    if(error != nullptr)
      *error = I(GetLastError)();

    if(file_len <= 0){
      I(CloseHandle)(handle);
      return nullptr;
    }

    void* buffer = I(malloc)(file_len);
    if(buffer == nullptr){
      if(error != nullptr)
        *error = 0x8;

      I(CloseHandle)(handle);
      return nullptr;
    }

    I(memset)(buffer, 0, file_len);

    ul64 read_bytes;
    if(!I(ReadFile)(handle, buffer, file_len, &read_bytes, nullptr)){
      if(error != nullptr)
        *error = I(GetLastError)();

      free(buffer);
      I(CloseHandle)(handle);
      return nullptr;
    }

    I(CloseHandle)(handle);

    return buffer;
  }

  inline bool write_text(std::wstring file_name, std::wstring str, bool append = false, uptr* error = nullptr){
    u32 handle_flags = GENERIC_WRITE;
    if(append)
      handle_flags |= FILE_APPEND_DATA;

    HANDLE handle = I(CreateFileW)(file_name.c_str(), handle_flags, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, append ? OPEN_ALWAYS : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if(error != nullptr)
      *error = I(GetLastError)();

    if(!IS_HANDLE_VALID(handle))
      return false;

    if(append){
      I(SetFilePointer)(handle, 0, nullptr, FILE_END);
      if(error != nullptr)
        *error = I(GetLastError)();
    }

    u32  len         = str.length() * 2; // This is a wide char so times it by 2.
    ul64 wrote_bytes = 0;
    bool result = I(WriteFile)(handle, str.c_str(), len, (ul64*)&wrote_bytes, nullptr);

    if(error != nullptr)
      *error = I(GetLastError)();

    I(CloseHandle)(handle);
    if(!result)
      return false;

    return result;
  }

  inline bool write_text(std::wstring file_name, std::string str, bool append = false, uptr* error = nullptr){
    u32 handle_flags = GENERIC_WRITE;
    if(append)
      handle_flags |= FILE_APPEND_DATA;

    HANDLE handle = I(CreateFileW)(file_name.c_str(), handle_flags, 0, nullptr, append ? OPEN_ALWAYS : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if(error != nullptr)
      *error = I(GetLastError)();

    if(!IS_HANDLE_VALID(handle))
      return false;

    if(append){
      I(SetFilePointer)(handle, 0, nullptr, FILE_END);
      if(error != nullptr)
        *error = I(GetLastError)();
    }

    u32  len         = str.length();
    ul64 wrote_bytes = 0;
    bool result = I(WriteFile)(handle, str.c_str(), len, (ul64*)&wrote_bytes, nullptr);

    if(error != nullptr)
      *error = I(GetLastError)();

    I(CloseHandle)(handle);
    if(!result)
      return false;

    return result;
  }
};