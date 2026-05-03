#pragma once

#if defined(GEN_INTERNAL_NETWORK) || defined(GEN_INTERNAL)
// define constants
#define GEN_MAX_PKG 512 // Maximum number of instruction packages
#define GEN_MAX_BASE_ADDRESSES 1000 // Maximum number of base addresses
#define GEN_MAX_HOOK_SAMPLE_ASM86 16 // Maximum number of bytes that will be scanned in x86
#define GEN_MAX_HOOK_SAMPLE_ASM64 112 // Maximum number of bytes that will be scanned in x64
#define GEN_HOOK_JMP_SIZE86 5  // E9 00 00 00 00
#define GEN_HOOK_JMP_SIZE64 14 // FF 25 00 00 00 00 00 00 00 00 00 00 00 00
#define GEN_MAX_PAGES 8 // Maximum number of pages that can be allocated to be mapped
#define GEN_MAX_ERASES 64 // Maximum number of function that can be declared for deletion after initialization
#define GEN_MAX_HOOKS 192 // Maximum number of hooks that can be placed

#define GEN_PKG_TYPE_PTR    HASH("GEN_PKG_TYPE_PTR")
#define GEN_PKG_TYPE_IMPORT HASH("GEN_PKG_TYPE_IMPORT")
#define GEN_PKG_TYPE_HOOK   HASH("GEN_PKG_TYPE_HOOK")

#pragma pack(push,1)
struct s_gen_pkg{
  // main
  u32 type;
  u32 hash;
  u32 hash1;
  u64 ptr;

  bool short_hook;
  u64  short_hook_ptr;
  u8   short_hook_asm[GEN_HOOK_JMP_SIZE86];
  u8   short_hook_len;

  // hooks 
  u8 original_asm[GEN_MAX_HOOK_SAMPLE_ASM64];
};

struct s_gen_required_import{
  u32 module_hash;
  u32 name_hash;
};

// gen_input
// This will be used to send to the server with package based instructions informing the server how to do what
struct s_gen_input{
  // main
  u64 base_address = 0;
  u64 hook_trp_base_address = 0;

  // This literally does nothing on the server and here.
  u64 base_address_list[GEN_MAX_BASE_ADDRESSES];
  
  // packages
  u32 package_count = 0;
  s_gen_pkg packages[GEN_MAX_PKG];
  
  // functions
  ALWAYSINLINE s_gen_pkg* add_pkg(s_gen_pkg pkg){
    if(package_count >= GEN_MAX_PKG)
      return nullptr;
    
    I(memcpy)((void*)&packages[package_count], (void*)&pkg, XOR32(sizeof(s_gen_pkg)));
    package_count++;
    
    return &packages[package_count-1];
  }
  
  ALWAYSINLINE s_gen_pkg* add_pkg(u32 type, u32 hash, u32 hash1, u64 ptr){
    s_gen_pkg new_pkg{};
    {
      I(memset)(&new_pkg, 0, XOR32(sizeof(s_gen_pkg)));
      
      new_pkg.type  = type;
      new_pkg.hash  = hash;
      new_pkg.hash1 = hash1;
      new_pkg.ptr   = ptr;
    }
    
    return add_pkg(new_pkg);
  }
  
  ALWAYSINLINE s_gen_pkg* get_pkg(u32 index){
    if(index >= package_count)
      return nullptr;
    
    return &packages[index];
  }
  
  ALWAYSINLINE s_gen_pkg* get_pkg_by_hash(u32 hash){
    for(u32 i = 0; i < package_count; i++){
      s_gen_pkg* pkg = &packages[i];

      if(pkg->hash == hash)
        return pkg;
    }

    return nullptr;
  }

  ALWAYSINLINE bool is_valid(){
    return base_address > 0 && hook_trp_base_address > 0 && package_count > 0;
  }
};

struct s_gen_page_list{
  u64 ptr;
  u32 len;
  u32 new_protection;
};

struct s_gen_hook_list{
  u64 hook_ptr;
  u64 write_trampoline_ptr;
  
  u8 original_asm[GEN_MAX_HOOK_SAMPLE_ASM64];
  u8 jmp_asm[GEN_HOOK_JMP_SIZE64];
  u8 jmp_asm_len;
};

struct s_gen_output{
  u64 entry_point;
  u64 has_loaded_ptr;
  
  // Page remapping
  u32 page_list_count;
  s_gen_page_list page_list[GEN_MAX_PAGES];

  // Hooks
  u32 hook_list_count;
  s_gen_hook_list hook_list[GEN_MAX_HOOKS];
  
  // This section will contain the cheat binary
  u32 data_len;
  i8 data[0];
};
#pragma pack(pop)
#endif