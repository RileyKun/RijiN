#include "../link.h"

CLASS_ALLOC(c_autowall, autowall);

EXPORT void __fastcall util_impacttrace(s_trace* trace, i32 damage_type, i8* custom_impact_name){
  if(autowall->is_simulating){
    if(trace != nullptr){
      if(trace->hit_entity != nullptr){
        c_base_entity* hit_ent = (c_base_entity*)trace->hit_entity;
        if(!autowall->dump.dumped && hit_ent->is_player() && raytrace_aimbot->should_target_entity(trace->hit_entity)){
          void* rbp = __builtin_frame_address(1);

          autowall->dump.dumped       = true;
          autowall->dump.damage       = *(float*)((uptr)rbp + 0x300);
          autowall->dump.hit_entity   = trace->hit_entity;
          autowall->dump.hitgroup     = trace->hit_group;
        }
      }
    }

    return;
  }

  utils::call_fastcall64_raw<void, s_trace*, i32, i8*>(gen_internal->decrypt_asset(global->util_impacttrace_trp), trace, damage_type, custom_impact_name);
}