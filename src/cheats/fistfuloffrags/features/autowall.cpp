#include "../link.h"

CLASS_ALLOC(c_autowall, autowall);

EXPORT void __cdecl util_impacttrace(s_trace* trace, i32 damage_type, i8* custom_impact_name){
  if(autowall->is_simulating){
    if(!autowall->dump.dumped && trace->hit_entity != nullptr && trace->hit_entity->is_player() && raytrace_aimbot->should_target_entity(trace->hit_entity)){
      autowall->dump.dumped       = true;
      autowall->dump.damage       = *(float*)((u32)__builtin_frame_address(1) - (u32)0x14);
      autowall->dump.hit_entity   = trace->hit_entity;
      autowall->dump.hitgroup     = trace->hit_group;
    }

    return;
  }

  utils::call_cdecl<void, s_trace*, i32, i8*>(gen_internal->decrypt_asset(global->util_impacttrace_trp), trace, damage_type, custom_impact_name);
}