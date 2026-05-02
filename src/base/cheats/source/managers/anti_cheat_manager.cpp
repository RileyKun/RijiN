#include "../../../link.h"

void c_base_anti_cheat_manager::pre_create_move(c_user_cmd* cmd){
  if(cmd == nullptr)
    return;
  
  // Update untouched_cmd.
  memcpy(&untouched_cmd, cmd, sizeof(c_user_cmd));
}

// Call at the bottom of the create_move hook after everything else.
void c_base_anti_cheat_manager::post_create_move(c_user_cmd* cmd, bool post_movement_fix){
  if(cmd == nullptr)
    return;

  cookie_block_check_return();

  // If it's not allowed then clamp the angles.
  if(!should_allow_unclamped_angles() || utils::is_match_making_server())
    cmd->view_angles.x = math::clamp(math::normalize_angle(cmd->view_angles.x), -89.f, 89.f);

  cmd->view_angles.y = math::clamp(math::normalize_angle(cmd->view_angles.y), -180.f, 180.f);
  cmd->view_angles.z = 0.f;

  if(post_movement_fix){
    for(u32 i = 0; i <= 1; i++) // Movement fix and other various tampering with it could cause detections due to it being out of bounds.
      cmd->move[i] = math::clamp(cmd->move[i], -get_maximum_movement_size(), get_maximum_movement_size());
    
    cmd->move.z = math::clamp(cmd->move.z, -320.f, 320.f);
    
    // Rebuild the buttons to ensure they match with the forward and side move.
    {
      cmd->buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);
      if(cmd->move.x > 0.f)
        cmd->buttons |= IN_FORWARD;
      else if(cmd->move.x < 0.f)
        cmd->buttons |= IN_BACK;
    
      if(cmd->move.y > 0.f)
        cmd->buttons |= IN_MOVERIGHT;
      else if(cmd->move.y < 0.f)
        cmd->buttons |= IN_MOVELEFT;
    }

    return;
  }


  bool silent_aim = is_silent_aim_active(cmd);

  // The command manager should handle this as well, but it'll be unaware of what this is doing since this function should be called afterwards...
  if(!should_allow_command_number_changes()){
    if(untouched_cmd.command_number != cmd->command_number){
      DBG("[-] c_base_anti_cheat_manager::post_create_move disallowing the change of command_number!\n");
      cmd->command_number = untouched_cmd.command_number;
    }
  }

  // If we can't back track then disallow the change of tick count.
  if(!can_back_track()){
    if(untouched_cmd.tick_count != cmd->tick_count){
      DBG("[-] c_base_anti_cheat_manager::post_create_move disallowing the change of tick_count. Really, this should be done in interp manager just a heads up\n");
      cmd->tick_count = untouched_cmd.tick_count;
    }
  }

  // See if we're going to repeat angles, if so then hold our aim for an extra tick.
  if(!is_anti_aim_allowed() && !has_to_angle_repeat()){
    if(silent_aim || silent_aim_last_tick){
      if(is_angle_repeat(cmd) == ACM_TRIGGER_WITHOUT_SET){
        DBG("[+] c_base_anti_cheat_manager::post_create_move: angle repeat detected, holding last tick to prevent detection.\n");
        cmd->view_angles = history_cmd[ACM_LAST_TICK].view_angles;
        on_angle_modified(cmd);
      }
    }
  }


  // Certain anti-cheats detect single input presses on attack buttons, this will detect this and automatically prevent it from occuring.
  counter_single_tick_presses(cmd, IN_ATTACK);
  counter_single_tick_presses(cmd, IN_ATTACK2);
  counter_single_tick_presses(cmd, IN_ATTACK3);
  counter_single_tick_presses(cmd, IN_JUMP);
  counter_single_tick_presses(cmd, IN_RELOAD);

  // Always match the random seed with the command number.
  cmd->random_seed = math::md5_pseudo_random(cmd->command_number) & INT_MAX;

  for(u32 i = (ACM_MAX_COUNT - 1); i > 0; i--)
    memcpy(&history_cmd[i], &history_cmd[i - 1], sizeof(c_user_cmd));
  
  memcpy(&history_cmd[ACM_LAST_TICK], cmd, sizeof(c_user_cmd));

  silent_aim_last_tick = silent_aim;
}

bool c_base_anti_cheat_manager::on_process_print_hook(c_svc_print* msg){
  if(msg == nullptr)
    return false;

  assert(msg->text != nullptr);

  DBG("[!] c_base_anti_cheat_manager::on_process_print_hook: %s\n", msg->text);
  return false;
}

bool c_base_anti_cheat_manager::on_process_query_convar_hook(c_clc_respond_cvar_value* msg){
  if(msg == nullptr)
    return false;

  DBG("[!] c_base_anti_cheat_manager::on_process_query_convar_hook: %i - %s -> %s - %i\n", msg->cookie, msg->name, msg->val, msg->query_status);
  return false;
} 

void c_base_anti_cheat_manager::on_angle_modified(c_user_cmd* cmd){
  if(cmd == nullptr)
    return;

  if(!can_hide_angle_snaps()){
    DBG("[-] c_base_anti_cheat_manager::on_angle_modified: Weapon doesn't support serverside silent aim.\n");
    return;
  }

  global_internal->base_packet_manager->disregard_next_frame_unchoke();
  global_internal->base_packet_manager->set_choke(true);
  DBG("[+] c_base_anti_cheat_manager::on_angle_modified: Adjusting choke packets to prevent angle leakage if needed.\n");
}