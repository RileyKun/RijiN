#include "../link.h"

CLASS_ALLOC(c_auto_bash, auto_bash);

EXPORT void __fastcall trace_ray_hook(void* ecx, void* edx, s_ray& ray, i32 mask, c_trace_filter* filter, s_trace* tr){
  assert(ecx != nullptr);
  static c_trace_filter new_filter;

  // Fix bugged trace filter on test_swing_collision call
  if(auto_bash->should_dump){
  	{
    	new_filter.type                 = TRACE_EVERYTHING;
    	new_filter.custom_trace_filter  = 0;
    	new_filter.ignore_entity        = utils::localplayer();

    	filter = &new_filter;
  	}
  }

	utils::call_fastcall<void, s_ray&, i32, c_trace_filter*, s_trace*>(gen_internal->decrypt_asset(global->trace_ray_hook_trp), ecx, edx, ray, mask, filter, tr);

  if(auto_bash->should_dump && tr != nullptr && tr->hit_entity != nullptr){
  	c_base_player* pl = tr->hit_entity;
    u32            pt = pl->type();

  	if(pl->is_valid() && (pt & TYPE_IS_ZOMBIE) && !pl->is_invisible()){

      if(config->aimbot.auto_bash_target_special_only && !(pt & TYPE_SPECIAL))
        return;

    	memcpy(&auto_bash->trace_dump, tr, sizeof(s_trace));
    	auto_bash->should_dump = false;
  	}
	}
}