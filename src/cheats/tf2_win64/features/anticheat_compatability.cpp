#include "../link.h"

c_anti_cheat_manager* acm = nullptr;

EXPORT bool __fastcall cbaseclientstate_process_print_hook(void* rcx, c_svc_print* msg){
  if(acm->on_process_print_hook(msg))
    return true;

  return utils::call_fastcall64<bool, c_svc_print*>(gen_internal->decrypt_asset(global->cbaseclientstate_process_print_hook_trp), rcx, msg);
}

EXPORT bool __fastcall cbaseclientstate_process_set_convar_hook(void* rcx, void* msg){
  assert(rcx != nullptr);
  assert(msg != nullptr);

  //c_utlvector<s_net_cvar_data>* data = (c_utlvector<s_net_cvar_data>*)((uptr)msg + (uptr)0x28);
  //if(data->size > 0){
  //  for(i32 i = 0; i < data->size; i++){
  //    i8* name = data->memory[i].name;
  //    i8* val  = data->memory[i].value;
//
  //    acp->update_cvar_value(name, val);
  //    DBG("[!] SetConVar: Got %s being set to %s\n", name, val);
  //  }
  //}

  return utils::call_fastcall64<bool, void*>(gen_internal->decrypt_asset(global->cbaseclientstate_process_set_convar_hook_trp), rcx, msg);
}

bool c_anti_cheat_manager::is_anti_aim_allowed(){
  return antiaim->should_antiaim() && antiaim->antiaim_enabled;
}

bool c_anti_cheat_manager::has_to_angle_repeat(){

  if(global->aimbot_settings != nullptr){
    // View-angle no-spread will jitter the view and cause angle-repeats.
    if(global->aimbot_settings->predict_server_random_seed)
      return true;

    if(utils::is_community_server()){
      // No-spread is only good in TF2 if we can mess with the random seed. Otherwise it's terrible.
      if(global->aimbot_settings->no_spread && config->acm.allow_command_number_manip)
        return true;
    }
  }

  return false;
}

u32 c_anti_cheat_manager::get_play_style(bool cache){
  static u32 cached_result = ACM_UNKNOWN;
  if(!cache || cached_result == ACM_UNKNOWN){
    if(global->aimbot_settings == nullptr)
      return ACM_UNKNOWN;

    if(!global->aimbot_settings->silent_aim_none || global->aimbot_settings->no_spread || global->aimbot_settings->predict_server_random_seed || config->misc.crithack_enabled)
      cached_result = ACM_RAGE;
    else
      cached_result = ACM_SEMI_RAGE;
  }

  return cached_result;
}

bool c_anti_cheat_manager::can_back_track(){
  return true;
}

bool c_anti_cheat_manager::should_allow_unclamped_angles(){
  if(!utils::is_match_making_server() && !config->acm.allow_unclamped_angles)
    return false;

  return antiaim->should_antiaim() && antiaim->antiaim_enabled;
}

bool c_anti_cheat_manager::should_allow_command_number_changes(){
  if(utils::is_community_server() && !config->acm.allow_command_number_manip)
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(config->misc.crithack_enabled)
    return true;

  if(localplayer->is_player_class(TF_CLASS_SOLDIER) || localplayer->is_player_class(TF_CLASS_DEMOMAN)){
    if(global->aimbot_settings != nullptr){
      if(global->aimbot_settings->no_spread)
        return true;
    }
  }

  return false;
}

bool c_anti_cheat_manager::can_hide_angle_snaps(){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(weapon->is_hitscan_weapon())
    return false;

  return true;
}