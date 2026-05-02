#pragma once


#if defined(INTERNAL_CHEAT)
  #define INLINE_OR_STATIC static
#else
  #define INLINE_OR_STATIC ALWAYSINLINE inline
#endif

namespace cpu{

  ALWAYSINLINE inline void cpuid(u32 eax_in, u32 ecx_in, u32* eax, u32* ebx, u32* ecx_out, u32* edx) {
    __asm__ __volatile__ (
      "cpuid"
      : "=a" (*eax), "=b" (*ebx), "=c" (*ecx_out), "=d" (*edx)
      : "a" (eax_in), "c" (ecx_in)
    );
  }

  INLINE_OR_STATIC i8* get_vendor(){
    u32 eax, ebx, ecx, edx;
    cpuid(0x0, 0, &eax, &ebx, &ecx, &edx);

    static i8   data[13];
    static bool first_time = false;
    if(!first_time){
      memset(data, 0, sizeof(data));
      ((u32*)data)[0] = ebx;
      ((u32*)data)[1] = edx;
      ((u32*)data)[2] = ecx;
      data[12] = '\0';
    }

    return (i8*)&data[0];
  }

  INLINE_OR_STATIC i8* get_brand_name(){
    static u32 data[13];
    static bool first_time = false;
    if(!first_time){
      first_time = true;
      memset(data, 0, sizeof(data));
      cpuid(0x80000002, 0, &data[0],  &data[1],  &data[2],  &data[3]);
      cpuid(0x80000003, 0, &data[4],  &data[5],  &data[6],  &data[7]);
      cpuid(0x80000004, 0, &data[8],  &data[9],  &data[10], &data[11]);

      data[12] = '\0';
      str_utils::trim((i8*)&data[0]);
    }

    return (i8*)&data[0];
  }

  INLINE_OR_STATIC u32 get_brand_hash(){
    static u32 hash = 0;
    if(!hash)
      hash = HASH_RT(get_brand_name());

    return hash;
  }

  INLINE_OR_STATIC bool is_amd_cpu(){
    static bool first_time = false;
    static bool is_amd_cpu = false;
    if(!first_time){
      first_time = true;

      const i8* vendor = get_vendor();
      is_amd_cpu = vendor[0] == 'A' && vendor[1] == 'u' && vendor[2] == 't' &&
                   vendor[3] == 'h' && vendor[4] == 'e' && vendor[5] == 'n' &&
                   vendor[6] == 't' && vendor[7] == 'i' && vendor[8] == 'c' &&
                   vendor[9] == 'A' && vendor[10] == 'M' && vendor[11] == 'D';
    }

    return is_amd_cpu;
  }

  INLINE_OR_STATIC u32 get_core_count(bool physical = false){
    i32 type = physical ? 0 : 1;
    static u32 cores[2] = {0, 0};
    if(!cores[type]){
      bool fetch_topology = true;
      u32 eax, ebx, ecx, edx;
      if(is_amd_cpu()){
        cpuid(0x80000008, 0, &eax, &ebx, &ecx, &edx);

        cores[type] = ((eax >> 16) & 0xFF) + 1;

        if(cores[type] <= 1){
          cpuid(0x1, 0, &eax, &ebx, &ecx, &edx);
          cores[type]    = ((ebx >> 16) & 0xFF);
        }

        fetch_topology = cores[type] > 1;
      }

      if(fetch_topology){
        u32 smt_threads = 1;
        u32 total_logical = 1;

        for (u32 level = 0; level < 8; level++){
          u32 eax, ebx, ecx, edx;
          cpuid(0x0B, level, &eax, &ebx, &ecx, &edx);

          u32 level_type = (ecx >> 8) & 0xFF;
          if(ebx == 0 || level_type == 0)
            break;

          if(level_type == 1)
            smt_threads = ebx;
          else if(level_type == 2){
            total_logical = ebx;
            break;
          }
        }

        cores[type] = total_logical;
        if(physical){
          cores[type] /= smt_threads;
        }
      }
    }

    return cores[type];
  }
};