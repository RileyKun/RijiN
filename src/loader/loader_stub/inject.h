#pragma once

#define STUB_PROCESS_TIMEOUT_SECOND 300
#define STUB_PROCESS_TIMEOUT_MS XOR32(STUB_PROCESS_TIMEOUT_SECOND * 1000)
#define STUB_PROCESS_TIMEOUT_SHORT XOR32(10)
#define STUB_PROCESS_TIMEOUT_SHORT_MS XOR32(10000)


#define STUB_WAIT_LOOP(max_time) for(u32 __i = 1; __i <= max_time; __i++)

#define MAX_LFA_CACHES 64
struct s_lfa_cache{
  u64 module_address;
  u32 lfa;
};

#define MAX_EXPORT_CACHE 512
struct s_export_cache{
  u64 module_address;

  IMAGE_DATA_DIRECTORY   data_directory;
  IMAGE_EXPORT_DIRECTORY export_directory;

  void* names;
  void* ordinals;
  void* funcs;
};

class c_procedure_base;
class c_inject{
public:
  u32     process_id = 0;
  HANDLE* process = nullptr;
  void*   process_hwnd = nullptr;
  bool    hv_mode = false;
  bool    x86mode = false;
  s_gen_input* input = nullptr;

  CFLAG_O0 u64 alloc_memory(ul64 len, u32 allocation_type, u32 protection, u64 base );
  CFLAG_O0 bool read_memory(void* address, void* data, ul64 len, u64* status_code = nullptr);
  CFLAG_O0 bool write_memory(void* address, void* data, ul64 len, u64* status_code = nullptr);

  bool read_memory_raw(void* address, void* data, ul64 len);
  bool write_memory_raw(void* address, void* data, ul64 len);

  bool  virtual_protect(void* address, u32 len, u32 protection, u32* old_protection);
  void* open_process(u32 pid, u32* last_error = nullptr);
  u64 get_module_address(u32 name_hash, bool await = true);
  bool x86(u32 module_hash);
  u64 get_export(u64 module_address, u32 export_hash);
  bool fix_page_permissions(c_net_receive_cheat* output_data);
  bool short_hook(c_procedure_base* pb, s_gen_pkg* pkg);
  bool add_hook(c_procedure_base* pb, u64 module_address, u32 target_func_hash, u64 target_address, bool add_module_address = true);
  bool wait_for_cheat_to_execute(u64 loaded_ptr);
  bool resolve_required_imports(c_net_receive_cheat_info* cheat_info);
  bool write_hooks(c_net_receive_cheat* output_data, s_gen_input* input);
  bool clean_dxgi();
  void fetch_dxgi(c_net_receive_cheat_info* cheat_info);
  bool dump_dxgi_offsets(bool x86);
  bool check_game_compatibility(bool x86);
  void auto_start_steam_game(bool x86);
  bool external_loaded();
  bool vac_bypass_loaded(bool* failed);
  void procedures(u32 name_hash);

  bool run();
  bool run_vac_bypass();

  ALWAYSINLINE inline bool is_process_starting_error(u32 last_error){
    return last_error != ERROR_PARTIAL_COPY && last_error != ERROR_INVALID_PARAMETER && last_error != ERROR_ACCESS_DENIED && last_error != ERROR_BAD_LENGTH;
  }

  s_lfa_cache lfa_cache[MAX_LFA_CACHES];
  u32         lfa_cache_count;

  ALWAYSINLINE inline void push_lfa_cache(u64 module_address, u32 lfa){
    for(i32 i = 0; i < MAX_LFA_CACHES; i++){
      if(!lfa_cache[i].module_address){
        lfa_cache[i].module_address = module_address;
        lfa_cache[i].lfa = lfa ^ HASH("lfa_xor");
        lfa_cache_count++;
        break;
      }
    }
  }

  s_export_cache export_cache[MAX_EXPORT_CACHE];
  u32            export_cache_count;


  ALWAYSINLINE inline s_export_cache* fetch_export_cache_entry(u64 module_address){
    if(!module_address || export_cache_count == 0)
      return nullptr;

    for(i32 i = 0; i < export_cache_count; i++)
      if(export_cache[i].module_address == module_address)
        return &export_cache[i];

    return nullptr;
  }

  // Await means, it'll keep trying for a success for a minute before failing.
  bool get_lfa_new(u64 module_address, u32* lfa_new_out, bool await = true){
    if(!module_address || lfa_new_out == nullptr)
      return false;

    *lfa_new_out = 0;
    if(lfa_cache_count > 0){
      for(i32 i = 0; i < lfa_cache_count; i++){
        if(!lfa_cache[i].module_address)
          break;

        if(lfa_cache[i].module_address == module_address){
          DBG("[!] get_lfa_new cache\n");
         *lfa_new_out = lfa_cache[i].lfa ^ HASH("lfa_xor");
          return true;
        }
      }
    }

    DBG("[!] get_lfa_new: %p - %i\n", module_address, await);
    STUB_WAIT_LOOP(STUB_PROCESS_TIMEOUT_MS){
      if(read_memory(module_address + XOR32(0x3C), lfa_new_out, XOR32(sizeof(u32)))){
        push_lfa_cache(module_address, *lfa_new_out);
        return true;
      }

      if(!await)
        return false;

      I(Sleep)(XOR32(1));
    }

    return false;
  }
};

CLASS_EXTERN(c_inject, inject);