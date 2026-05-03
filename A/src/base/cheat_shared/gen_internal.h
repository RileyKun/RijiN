#pragma once

#if defined(GEN_INTERNAL)
#pragma pack(push,1)
struct s_gen_unload_hook{
  u64 ptr;
  u8 len;
  u8 original_asm[GEN_MAX_HOOK_SAMPLE_ASM64];
};

class c_gen_internal{
public:
  bool loaded = false;
  bool beta   = false;

  u8 cloud_license_key[32];

  u32 unload_hooks_count = 0;
  s_gen_unload_hook unload_hooks[GEN_MAX_HOOKS];

  u32 package_count = 0;
  s_gen_pkg packages[GEN_MAX_PKG];

  u32 base_address_start;
  u32 base_address_end;

  u64 base_address_start64;
  u64 base_address_end64;

  ALWAYSINLINE inline bool is_tampered(){
    #if defined(__x86_64__)
      if(base_address_start64 == 0 || base_address_end64 == 0)
        return true;
    #else
      if(base_address_start == 0 ||  base_address_end == 0)
        return true;
    #endif
    return strlen(cloud_license_key) != XOR32(32);
  }

  // Unloading should only be possible on debug builds
  ALWAYSINLINE bool should_unload(){
#if defined(DEV_MODE)
    static bool unload = false;

    if(input_system->held(VK_END) || input_system->held(VK_DELETE))
      unload = true;

    return unload;
#else
    return false;
#endif
  }

  ALWAYSINLINE bool unload(){
#if defined(DEV_MODE)
    if(!loaded)
      return false;

    loaded = false;

    DBG("[!] gen_internal->unload\n");

    for(u32 i = 0; i < unload_hooks_count; i++){
      s_gen_unload_hook hook = unload_hooks[i];

      u32 old_protect = 0;
      VirtualProtect((uptr)hook.ptr, (u32)hook.len, XOR32(PAGE_EXECUTE_READWRITE), &old_protect);
      memcpy((uptr)hook.ptr, hook.original_asm, (u32)hook.len);
      VirtualProtect((uptr)hook.ptr, (u32)hook.len, old_protect, &old_protect);

      DBG("[+] unload hook %p\n", hook.ptr);
    }

    DBG("[+] unloaded %i hooks\n", unload_hooks_count);

    // Sleep our current thread for 250ms to let any hooks on other threads run
    Sleep(XOR32(250));

    return true;
#else
    return false;
#endif
  }

  bool setup(){
    if(loaded)
      return true;

#if defined(INTERNAL_CHEAT)
    // store base_address_start and base_address_end
    {
      base_address_start = get_pkg(HASH("base_address_start"));
      base_address_end   = get_pkg(HASH("base_address_end"));

      #if defined(__x86_64__)
        base_address_start64 = get_pkg(HASH("base_address_start64"));
        base_address_end64   = get_pkg(HASH("base_address_end64"));
      #endif
    }
#endif

    // Open the file mapping for GMC
#if defined(DEV_MODE)
    I(OpenFileMappingA)(XOR32(FILE_MAP_ALL_ACCESS), false, XOR("GMC001"));
#endif

    DBG("[!] gen_internal->setup\n");

    return loaded = true;
  }

  ALWAYSINLINE bool has_sub_expired(){
    kernel_time_expired_val(true);
#if defined(DEV_MODE) || defined(STAGING_MODE)
    return false;
#else
    static float  expire_time = (float)math::biggest(math::smallest(get_pkg(HASH("gen_expire_time")), XOR32(86400)), XOR32(43200));
    static float  time_start  = math::time();
    float         time_now    = math::time();
    float         time_delta  = (time_now - time_start);

    return expire_time <= 0 || time_delta > expire_time;
#endif
  }

  uptr get_pkg(u32 hash);

  CFLAG_O0 ALWAYSINLINE inline void* decrypt_asset(volatile void* asset){
    assert_private(asset != nullptr);
    volatile u32 key = *(volatile u32*)(XOR32_IMPORTANT_ONLY(0x7FFE0330));
    return (void*)((uptr)asset ^ (uptr)key);
  }
};

#pragma pack(pop)

CLASS_EXTERN(c_gen_internal, gen_internal);

#if defined(DEV_MODE)
static uptr get_debug_pkg(u32 hash, u32 line, std::string file, std::string function){
  uptr data = gen_internal->get_pkg(hash);
  if(data != 0)
    return data;

  i8 buf[1024];
  I(wsprintfA)(buf, XOR("NULL package! 0x%X\nLocated at line %i in %s inside %s\n"), hash, line, file.c_str(), function.c_str());
  I(MessageBoxA)(nullptr, buf, "", 0);
  return 0;
}

#define GET_PKG(x) get_debug_pkg(XOR32(HASH(x)), __LINE__, XOR(__FILE__), XOR(__PRETTY_FUNCTION__))
#else
#define GET_PKG(x) gen_internal->get_pkg(XOR32(HASH(x)))
#endif

#define GET_PKG_NER(x) gen_internal->get_pkg(HASH(x))
#endif