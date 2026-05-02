
#include "link.h"

static i32 find_and_zero_string(const wchar_t* file_path, i8* target_str, bool override_start_check = false) {
  const u32 len = strlen(target_str);
  if (len <= 1)
    return 0;

  HANDLE file_handle = CreateFileW(file_path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  if (file_handle == INVALID_HANDLE_VALUE) {
    printf("ERROR: %ls either doesn't exist, or is in use.\n", file_path);
    return 1;
  }

  u32 file_size = GetFileSize(file_handle, 0);
  if (file_size == INVALID_FILE_SIZE || file_size == 0) {
    CloseHandle(file_handle);
    printf("ERROR: %ls Unable to fetch file size.\n", file_path);
    return 2;
  }

  HANDLE mapping = CreateFileMappingW(file_handle, 0, PAGE_READWRITE, 0, 0, 0);
  if (!mapping) {
    CloseHandle(file_handle);
    printf("ERROR: %ls Unable to create file mapping.\n", file_path);
    return 3;
  }

  u8* bytes = (u8*)MapViewOfFile(mapping, FILE_MAP_WRITE, 0, 0, 0);
  if (bytes == nullptr) {
    CloseHandle(mapping);
    CloseHandle(file_handle);
    printf("ERROR: %ls Unable to map view of file.\n", file_path);
    return 4;
  }

  // Locate .text section
  u32 text_section_start = 0;
  u32 text_section_end = 0;

  IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)bytes;
  if (dos->e_magic == IMAGE_DOS_SIGNATURE) {
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(bytes + dos->e_lfanew);
    if (nt->Signature == IMAGE_NT_SIGNATURE) {
      IMAGE_FILE_HEADER* file_header = &nt->FileHeader;
      IMAGE_SECTION_HEADER* sections = (IMAGE_SECTION_HEADER*)((u8*)&nt->OptionalHeader + file_header->SizeOfOptionalHeader);

      for (u32 i = 0; i < file_header->NumberOfSections; i++) {
        IMAGE_SECTION_HEADER* s = &sections[i];
        if (*(u32*)s->Name == *(u32*)".text") {
          text_section_start = s->PointerToRawData;
          text_section_end = text_section_start + s->SizeOfRawData;
          break;
        }
      }
    }
  }

  for (u32 i = 1; i <= file_size - (len - 1); i++) {
    if (i >= text_section_start && i < text_section_end)
      continue;

    if ((bytes[i - 1] == 0 && !override_start_check) ||
        (bytes[i - 1] == 0xFF && !override_start_check) ||
        override_start_check) {
      if (memcmp(&bytes[i], target_str, len) == 0) {

        for (u32 j = 0; j < len; j++)
          bytes[i + j] = 0x00;

        printf("[!] Detected unwanted string \"%s\" at %X\n", target_str, i);

      }
    }
  }

  UnmapViewOfFile(bytes);
  CloseHandle(mapping);
  CloseHandle(file_handle);
  return 0;
}


i32 main(){
  wchar_t* base_dll = L"";
  wchar_t* cmd_line = GetCommandLineW();
  if(cmd_line == nullptr)
    return 0;

  u32 len = wcslen(cmd_line);
  std::wstring module = L"";
  bool fetch = false;
  for(u32 i = 0; i < len; i++){
    if(cmd_line[i] == L'-'){
      fetch = true;
      continue;
    }

    if(!fetch)
      continue;

    module += cmd_line[i];
  }

  if(module.empty())
    return;

  printf("Scanning for unwanted strings: %ls\n", module.c_str());
  base_dll = module.c_str();
  find_and_zero_string(base_dll, "basic_string::_S_construct null not valid", true);
  find_and_zero_string(base_dll, "basic_string::_S_create", true);
  find_and_zero_string(base_dll, "basic_string::append");
  find_and_zero_string(base_dll, "basic_string::assign");
  find_and_zero_string(base_dll, "basic_string: construction from null is not valid");
  find_and_zero_string(base_dll, "basic_string::_M_create");
  find_and_zero_string(base_dll, "basic_string::substr");
  find_and_zero_string(base_dll, "vector::_M_realloc_append", true);
  find_and_zero_string(base_dll, "basic_string::_M_replace");
  find_and_zero_string(base_dll, "basic_string::insert");
  find_and_zero_string(base_dll, "basic_string::compare");
  find_and_zero_string(base_dll, "basic_string::at: __n (which is \%zu) >= this->size() (which is \%zu)");
  find_and_zero_string(base_dll, "basic_string::_M_replace_aux");
  find_and_zero_string(base_dll, "%s: __pos (which is \%zu) > this->size() (which is \%zu)");
  find_and_zero_string(base_dll, "std::allocator");
  find_and_zero_string(base_dll, "std::basic_string");
  find_and_zero_string(base_dll, "basic_string::replace");
  find_and_zero_string(base_dll, "basic_string::erase");
  find_and_zero_string(base_dll, "std::string");
  find_and_zero_string(base_dll, "std::istream");
  find_and_zero_string(base_dll, "std::bad_alloc");
  find_and_zero_string(base_dll, "std::bad_array_new_length");
  find_and_zero_string(base_dll, "std::bad_cast");
  find_and_zero_string(base_dll, "std::bad_typeid");
  find_and_zero_string(base_dll, "__gnu_cxx::__concurrence_lock_error");
  find_and_zero_string(base_dll, "std::exception");
  find_and_zero_string(base_dll, "std::bad_exception");
  find_and_zero_string(base_dll, "GCC: (Rev2, Built by MSYS2 project) 14.2.0");
  find_and_zero_string(base_dll, "St9type_info");
  find_and_zero_string(base_dll, "St9exception");
  find_and_zero_string(base_dll, "St9bad_alloc");
  find_and_zero_string(base_dll, "St8bad_cast");
  find_and_zero_string(base_dll, "St20bad_array_new_length");
  find_and_zero_string(base_dll, "St16invalid_argument");
  find_and_zero_string(base_dll, "St15underflow_error");
  find_and_zero_string(base_dll, "St14overflow_error");
  find_and_zero_string(base_dll, "St13runtime_error");
  find_and_zero_string(base_dll, "St13bad_exception");
  find_and_zero_string(base_dll, "St12system_error");
  find_and_zero_string(base_dll, "St12out_of_range");
  find_and_zero_string(base_dll, "St12length_error");
  find_and_zero_string(base_dll, "St12domain_error");
  find_and_zero_string(base_dll, "St11range_error");
  find_and_zero_string(base_dll, "St11logic_error");
  find_and_zero_string(base_dll, "St10bad_typeid");
  find_and_zero_string(base_dll, "NSt3_V214error_category");
  find_and_zero_string(base_dll, "basic_string::copy");
  find_and_zero_string(base_dll, "basic_string::basic_string");
  find_and_zero_string(base_dll, "basic_string::resize");
  find_and_zero_string(base_dll, "string::string");
  find_and_zero_string(base_dll, "N9__gnu_cxx29__concurrence_broadcast_errorE");
  find_and_zero_string(base_dll, "N9__gnu_cxx26__concurrence_unlock_errorE");
  find_and_zero_string(base_dll, "N9__gnu_cxx24__concurrence_wait_errorE");
  find_and_zero_string(base_dll, "N9__gnu_cxx24__concurrence_lock_errorE");
  find_and_zero_string(base_dll, "*N12_GLOBAL__N_122generic_error_categoryE");
  find_and_zero_string(base_dll, "*N12_GLOBAL__N_121system_error_categoryE");
  find_and_zero_string(base_dll, "N10__cxxabiv120__si_class_type_infoE");
  find_and_zero_string(base_dll, "N10__cxxabiv119__foreign_exceptionE");
  find_and_zero_string(base_dll, "N10__cxxabiv117__class_type_infoE");
  find_and_zero_string(base_dll, "N10__cxxabiv115__forced_unwindE");
  find_and_zero_string(base_dll, "Mingw-w64 runtime failure:");
  find_and_zero_string(base_dll, "Address %p has no image-section");
  find_and_zero_string(base_dll, "  VirtualQuery failed for %d bytes at address %p");
  find_and_zero_string(base_dll, "  VirtualProtect failed with code 0x%x");
  find_and_zero_string(base_dll, "__gnu_cxx::__concurrence_unlock_error");
  find_and_zero_string(base_dll, "__gnu_cxx::__concurrence_broadcast_error");
  find_and_zero_string(base_dll, "not enough space for format expansion (Please submit full bug report at https://gcc.gnu.org/bugs/):    ");
  find_and_zero_string(base_dll, "terminate called recursively\n");
  find_and_zero_string(base_dll, "terminate called without an active exception\\n");
  find_and_zero_string(base_dll, "terminate called after throwing an instance of '");
  find_and_zero_string(base_dll, "  what():  ");
  find_and_zero_string(base_dll, "random_device: rdrand failed");
  find_and_zero_string(base_dll, "random_device: rand_s failed");
  find_and_zero_string(base_dll, "random_device: rdseed failed");
  find_and_zero_string(base_dll, "random_device::random_device(const");
  find_and_zero_string(base_dll, "device not available");
  find_and_zero_string(base_dll, "random_device could not be read");

  find_and_zero_string(base_dll, "__gnu_cxx::__concurrence_wait_error");
  find_and_zero_string(base_dll, "not enough space for format expansion (Please submit full bug report at https://gcc.gnu.org/bugs/):\n    ");
  //find_and_zero_string(base_dll, "0123456789"); Bad idea, break shit.
  find_and_zero_string(base_dll, "terminate called recursively\n");
  find_and_zero_string(base_dll, "terminate called without an active exception\n");
  find_and_zero_string(base_dll, "generic");
  find_and_zero_string(base_dll, "system");
  find_and_zero_string(base_dll, "(anonymous namespace)");
  find_and_zero_string(base_dll, "auto");
  find_and_zero_string(base_dll, "decltype(auto)");
  find_and_zero_string(base_dll, "decltype(nullptr)");
  find_and_zero_string(base_dll, "string literal");
  find_and_zero_string(base_dll, "std");
  find_and_zero_string(base_dll, "[abi:");
  find_and_zero_string(base_dll, "{default arg#");
  find_and_zero_string(base_dll, "}::");
  find_and_zero_string(base_dll, "auto:");
  find_and_zero_string(base_dll, "template parameter object for ");
  find_and_zero_string(base_dll, "initializer for module ");
  find_and_zero_string(base_dll, "vtable for ");
  find_and_zero_string(base_dll, "VTT for ");
  find_and_zero_string(base_dll, "construction vtable for ");
  find_and_zero_string(base_dll, "-in-");
  find_and_zero_string(base_dll, "typeinfo for ");
  find_and_zero_string(base_dll, "typeinfo name for ");
  find_and_zero_string(base_dll, "typeinfo fn for ");
  find_and_zero_string(base_dll, "non-virtual thunk to ");
  find_and_zero_string(base_dll, "virtual thunk to ");
  find_and_zero_string(base_dll, "covariant return thunk to ");
  find_and_zero_string(base_dll, "java Class for ");
  find_and_zero_string(base_dll, "guard variable for ");
  find_and_zero_string(base_dll, "TLS init function for ");
  find_and_zero_string(base_dll, "TLS wrapper function for ");
  find_and_zero_string(base_dll, "reference temporary #");

  find_and_zero_string(base_dll, "hidden alias for ");
  find_and_zero_string(base_dll, "transaction clone for ");
  find_and_zero_string(base_dll, "non-transaction clone for ");
  find_and_zero_string(base_dll, "operator");
  find_and_zero_string(base_dll, "operator ");
  find_and_zero_string(base_dll, " : ");
  find_and_zero_string(base_dll, "new ");
  find_and_zero_string(base_dll, "ull");
  find_and_zero_string(base_dll, "false");
  find_and_zero_string(base_dll, "true");
  find_and_zero_string(base_dll, "java resource ");
  find_and_zero_string(base_dll, "decltype (");
  find_and_zero_string(base_dll, "basic_iostream");
  find_and_zero_string(base_dll, "::basic_iostream<char, std::char_traits<char> >");
  find_and_zero_string(base_dll, "::iostream");
  find_and_zero_string(base_dll, "basic_ostream");
  find_and_zero_string(base_dll, "::basic_ostream<char, std::char_traits<char> >");
  find_and_zero_string(base_dll, "::ostream");
  find_and_zero_string(base_dll, "basic_istream");
  find_and_zero_string(base_dll, "::basic_istream<char, std::char_traits<char> >");
  find_and_zero_string(base_dll, "std::allocator<char> >");
  find_and_zero_string(base_dll, "<char, std::char_traits<char>, std::allocator<char> >");
  find_and_zero_string(base_dll, "basic_string");
  find_and_zero_string(base_dll, "allocator");
  find_and_zero_string(base_dll, "decltype(nullptr)");
  find_and_zero_string(base_dll, "string literal");
  find_and_zero_string(base_dll, "this");
  find_and_zero_string(base_dll, "{parm#");
  find_and_zero_string(base_dll, "global constructors keyed to ");
  find_and_zero_string(base_dll, "global destructors keyed to ");
  find_and_zero_string(base_dll, "{lambda");
  find_and_zero_string(base_dll, ")#");
  find_and_zero_string(base_dll, "{unnamed type#");
  find_and_zero_string(base_dll, " [clone ");
  find_and_zero_string(base_dll, "[friend]");
  find_and_zero_string(base_dll, "typename");
  find_and_zero_string(base_dll, "template");
  find_and_zero_string(base_dll, " class");
  find_and_zero_string(base_dll, " requires ");
  find_and_zero_string(base_dll, " restrict");
  find_and_zero_string(base_dll, " volatile");
  find_and_zero_string(base_dll, " const");
  find_and_zero_string(base_dll, " transaction_safe");
  find_and_zero_string(base_dll, " noexcept");
  find_and_zero_string(base_dll, " throw");
  find_and_zero_string(base_dll, "&&");
  find_and_zero_string(base_dll, " _Complex");
  find_and_zero_string(base_dll, " _Imaginary");
  find_and_zero_string(base_dll, "::*");
  find_and_zero_string(base_dll, " __vector(");
  find_and_zero_string(base_dll, "  Unknown pseudo relocation protocol version \%d.");
  find_and_zero_string(base_dll, "  Unknown pseudo relocation bit size \%d.");
  find_and_zero_string(base_dll, "\%d bit pseudo relocation at \%p out of range, targeting \%p, yieldi");
  find_and_zero_string(base_dll, "ng the value \%p.");
  find_and_zero_string(base_dll, "*** buffer overflow detected ***: terminated");
  find_and_zero_string(base_dll, "*** stack smashing detected ***: terminated");

  find_and_zero_string(base_dll, "C\%p \%lu V=\%0X w=\%ld \%s\n");
  find_and_zero_string(base_dll, "C\%p \%lu \%s\n");
  find_and_zero_string(base_dll, "Error cleaning up spin_keys for thread \%lu.");
  find_and_zero_string(base_dll, "\%p not found?!?!");
  find_and_zero_string(base_dll, " once \%p is \%ld");
  find_and_zero_string(base_dll, "T\%p \%lu \%s");
  find_and_zero_string(base_dll, "T\%p \%lu V=\%0X H=\%p \%s");
  find_and_zero_string(base_dll, "Assertion failed: (\%s), file \%s, line \%d\n");
  find_and_zero_string(base_dll, "(((rwlock_t *)*rwl)->valid == LIFE_RWLOCK) && (((rwlock_t *)*rwl)->busy > 0)");
  find_and_zero_string(base_dll, "../mingw-w64/mingw-w64-libraries/winpthreads/src/rwlock.c");
  find_and_zero_string(base_dll, "RWL\%p \%lu \%s");
  find_and_zero_string(base_dll, "RWL\%p \%lu V=\%0X B=\%d r=\%ld w=\%ld L=\%p \%s");
  find_and_zero_string(base_dll, "co_await");
  find_and_zero_string(base_dll, "_matherr(): %s in %s(%g, %g)  (retval=%g)");
  find_and_zero_string(base_dll, "The result is too small to be represented (UNDERFLOW)");
  find_and_zero_string(base_dll, "Total loss of significance (TLOSS)");
  find_and_zero_string(base_dll, "Partial loss of significance (PLOSS)");
  find_and_zero_string(base_dll, "Overflow range error (OVERFLOW)");
  find_and_zero_string(base_dll, "Argument singularity (SIGN)");
  find_and_zero_string(base_dll, "Argument domain error (DOMAIN)");
  find_and_zero_string(base_dll, "std::bfloat16_t");
  find_and_zero_string(base_dll, "std::basic_iostream<char, std::char_traits<char> >");
  find_and_zero_string(base_dll, "std::basic_istream<char, std::char_traits<char> >");
  find_and_zero_string(base_dll, "std::basic_ostream<char, std::char_traits<char> >");
  find_and_zero_string(base_dll, "std::iostream");
  find_and_zero_string(base_dll, "std::ostream");
  find_and_zero_string(base_dll, "bad_function_call");
  find_and_zero_string(base_dll, "Unknown error");
  find_and_zero_string(base_dll, "GCC: (Rev8, Built by MSYS2 project) 15.1.0");
  find_and_zero_string(base_dll, "GCC: (Rev7, Built by MSYS2 project) 15.1.0");

  find_and_zero_string(base_dll, "rdseed", true);
  find_and_zero_string(base_dll, "rdrand", true);
  find_and_zero_string(base_dll, "rdrnd" , true);
  find_and_zero_string(base_dll, "rand_s", true);

  find_and_zero_string(base_dll, "decimal128", true);

  find_and_zero_string(base_dll, "signed char");
  find_and_zero_string(base_dll, "bool");
  find_and_zero_string(base_dll, "boolean");
  find_and_zero_string(base_dll, "char");
  find_and_zero_string(base_dll, "byte");
  find_and_zero_string(base_dll, "double");
  find_and_zero_string(base_dll, "long double");
  find_and_zero_string(base_dll, "float");
  find_and_zero_string(base_dll, "__float128");
  find_and_zero_string(base_dll, "unsigned char");
  find_and_zero_string(base_dll, "int");
  find_and_zero_string(base_dll, "unsigned int");
  find_and_zero_string(base_dll, "unsigned");
  find_and_zero_string(base_dll, "long");
  find_and_zero_string(base_dll, "unsigned long");
  find_and_zero_string(base_dll, "__int128");
  find_and_zero_string(base_dll, "unsigned __int128");
  find_and_zero_string(base_dll, "short");
  find_and_zero_string(base_dll, "unsigned short");
  find_and_zero_string(base_dll, "void");
  find_and_zero_string(base_dll, "wchar_t");
  find_and_zero_string(base_dll, "long long");
  find_and_zero_string(base_dll, "unsigned long long");
  find_and_zero_string(base_dll, "decimal32");
  find_and_zero_string(base_dll, "decimal64");
  find_and_zero_string(base_dll, "char8_t");
  find_and_zero_string(base_dll, "char16_t");
  find_and_zero_string(base_dll, "char32_t");
  find_and_zero_string(base_dll, "_Float");
  find_and_zero_string(base_dll, "}::");
  find_and_zero_string(base_dll, "auto:");
  find_and_zero_string(base_dll, " for ");
  find_and_zero_string(base_dll, "{parm#");
  find_and_zero_string(base_dll, "::*");
  find_and_zero_string(base_dll, "_GLOBAL_");
  find_and_zero_string(base_dll, "(...");
  find_and_zero_string(base_dll, "...)");
  find_and_zero_string(base_dll, "alignof");
  find_and_zero_string(base_dll, "const_cast");
  find_and_zero_string(base_dll, "[...]=");
  find_and_zero_string(base_dll, "delete[] ");
  find_and_zero_string(base_dll, "dynamic_cast");
  find_and_zero_string(base_dll, "delete ");
  find_and_zero_string(base_dll, "noexcept");
  find_and_zero_string(base_dll, "new[]");
  find_and_zero_string(base_dll, "reinterpret_cast");
  find_and_zero_string(base_dll, "sizeof...");
  find_and_zero_string(base_dll, "static_cast");
  find_and_zero_string(base_dll, "sizeof");
  find_and_zero_string(base_dll, "throw");
  find_and_zero_string(base_dll, "alignof ");
  find_and_zero_string(base_dll, "St17bad_function_call");
  find_and_zero_string(base_dll, "Infinity");
  find_and_zero_string(base_dll, "NaN");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  find_and_zero_string(base_dll, "");
  // Possible problematic.
  find_and_zero_string(base_dll, "GLIBCXX_TUNABLES");
  find_and_zero_string(base_dll, "obj_size");
  find_and_zero_string(base_dll, "obj_count");
  find_and_zero_string(base_dll, "glibcxx.eh_pool");

  find_and_zero_string(base_dll, "tf2_win64_sse2.dll");
  find_and_zero_string(base_dll, "loader.exe");
  find_and_zero_string(base_dll, "stub.dll");
  find_and_zero_string(base_dll, "vac_bypass.dll");
  find_and_zero_string(base_dll, "l4d2_sse2.dll");
  find_and_zero_string(base_dll, "gmod_sse2.dll");
  find_and_zero_string(base_dll, "fof_sse2.dll");
  find_and_zero_string(base_dll, "dab_sse2.dll");
  printf("\n\nFinished trimming strings\n");
  Sleep(5000);
  return 0;
}