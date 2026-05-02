#pragma once

class c_base_command_manager{
public:
  bool        is_locked           = false;
  i32         current_command     = 0;
  i32         current_seed        = 0;
  c_user_cmd* current_cmd         = nullptr;
  i32         last_command_number = 0;

  virtual void reset(){
    last_command_number = 0;
    is_locked           = false;
  }

  void set_prediction_random_seed(const i32 seed){
    if(global_internal->prediction_random_seed == nullptr)
      return;

    if(*global_internal->prediction_random_seed != -1)
      *global_internal->prediction_random_seed = seed;
  }

  virtual bool set_user_cmd(c_user_cmd* cmd, u32 command_number){
    if(cmd == nullptr){
      DBG("[-] c_base_command_manager::set_user_cmd the cmd parameter is nullptr\n");
      return false;
    }

    if(is_locked){
      DBG("[-] c_base_command_manager::set_user_cmd blocked because its locked until next create_move call\n");
      return false;
    }

    cmd->command_number = current_command = command_number;
    cmd->random_seed    = current_seed    = math::md5_pseudo_random(command_number) & INT_MAX;
    set_prediction_random_seed(cmd->random_seed);
    return true;
  }

  // After the double tap is_charging if it exists.
  virtual void pre_create_move(c_user_cmd* cmd){
    is_locked = false;
    if(cmd == nullptr)
      return;

    current_cmd     = cmd;
    current_command = cmd->command_number;
    current_seed    = cmd->random_seed = math::md5_pseudo_random(cmd->command_number) & INT_MAX;
  }

  virtual bool set_command(u32 command_number){
    if(is_locked){
      DBG("[-] c_base_command_manager::set_command not allowed to update command_number until next create_move call\n");
      return false;
    }

    return set_user_cmd(current_cmd, command_number);
  }


  virtual void create_move(c_user_cmd* cmd, bool post){
    if(cmd == nullptr)
      return;

    if(cmd->command_number != current_command || cmd->random_seed != current_seed){
      DBG("[-] c_base_command_manager::create_move_prediction: command_number or random_seed was illegally changed! Enforcing values.\n");
    }

    cmd->command_number = current_command;
    cmd->random_seed    = current_seed;
    set_prediction_random_seed(current_seed);
    if(post)
      last_command_number = current_command;
  }

  virtual i32 get_previous_command(){
    return last_command_number;
  }

  void lock(){
    is_locked = true;
  }

  i32 get_seed(){
    return current_seed;
  }
};