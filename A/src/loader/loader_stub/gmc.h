#pragma once

struct s_gmc_entry{
  u32 hash;
  u32 hash_type;
  u64 ptr;
};

#define GMC_CACHE_ENTRIES 1024
#define GMC_CACHE_SIZE (sizeof(s_gmc_entry) * GMC_CACHE_ENTRIES)
class c_gmc{
public:
  s_gmc_entry* cache = nullptr;
  u32          count = 0;

  void create_file_mapping(){
#if defined(DEV_MODE)
    if(!global->gmc_run)
      return;

    void* map_file = I(OpenFileMappingA)(XOR32(FILE_MAP_ALL_ACCESS), false, XOR("GMC001"));

    bool new_cache = map_file == nullptr;
    if(new_cache)
      map_file = I(CreateFileMappingA)(INVALID_HANDLE_VALUE, nullptr, XOR32(PAGE_READWRITE), 0, XOR32(GMC_CACHE_SIZE), XOR("GMC001"));

    cache = (s_gmc_entry*)I(MapViewOfFile)(map_file, XOR32(FILE_MAP_ALL_ACCESS), 0, 0, XOR32(GMC_CACHE_SIZE));

    if(new_cache)
      memset(cache, 0, XOR32(GMC_CACHE_SIZE));
    else
      update_cache_count();
#endif
  }

  void update_cache_count(){
#if defined(DEV_MODE)
    if(!global->gmc_run)
      return;

    if(cache == nullptr)
      create_file_mapping();

    for(count = 0; count < XOR32(GMC_CACHE_ENTRIES) && cache[count].hash != 0; count++)
      continue;

    DBG("[!] GMC COUNT: %i\n", count);
#endif
  }

  void add_entry(u32 hash_type, u32 hash, u64 ptr){
#if defined(DEV_MODE)
    if(!global->gmc_run)
      return;

    if(cache == nullptr)
      create_file_mapping();

    assert(count+1 < GMC_CACHE_ENTRIES);

    cache[count].hash       = hash;
    cache[count].hash_type  = hash_type;
    cache[count].ptr        = ptr;
    count++;
#endif
  }

  s_gmc_entry* find_entry(u32 hash_type, u32 hash){
#if defined(DEV_MODE)
    if(!global->gmc_run)
      return nullptr;

    if(cache == nullptr)
      create_file_mapping();

    for(u32 i = 0; i < count; i++)
      if(cache[i].hash_type == hash_type && cache[i].hash == hash)
        return &cache[i];
#endif

    return nullptr;
  }
};

CLASS_EXTERN(c_gmc, gmc);