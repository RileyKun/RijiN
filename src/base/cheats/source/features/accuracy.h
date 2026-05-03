#pragma once


class c_good_seed_data{
public:
  bool valid;
  bool is_shotgun;
  bool is_hitscan;
  bool is_grenade_launcher;
  bool is_loch_load;
  bool wants_speed;
};

class c_base_accuracy{
public:
  i32 best_command;

  void on_disconnect(){
    best_command = 0;
  }

  void on_command_found(i32 seed){
    best_command = seed;
  }

  virtual bool has_custom_server_random_seed(){
    return false;
  }

  virtual void set_prediction_random_seed(i32 seed){
    FUNCTION_OVERRIDE_NEEDED;
  }

  virtual bool is_seed_predicting(){
    if(!has_custom_server_random_seed())
      return false;

    return false;
  }

  virtual bool set_command(i32 command_number){
    on_command_found(command_number);
    return true;
  }

  virtual i32 get_seed(){
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  virtual i32 get_command_number_step(){
    return 8912;
  }

  virtual bool get_best_bullet_with_seed(vec3 current_angle, i32* bullet){
    FUNCTION_OVERRIDE_NEEDED;
    return false; 
  }

  virtual bool get_spread(vec3* out, u32& math_type, i32 seed = -1){
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  virtual bool is_nospread_active(){
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  virtual bool should_modify_command_number(c_internal_base_weapon* weapon){
    if(has_custom_server_random_seed())
      return false;

    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  virtual bool setup_good_seed_data(c_good_seed_data* data, c_internal_base_weapon* weapon){
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  virtual bool is_good_seed(i32 seed, c_good_seed_data* data){
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  virtual bool should_force_command_number(){
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  virtual bool calculate_command_number(c_user_cmd* cmd, c_internal_base_weapon* weapon){
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

  // Return true to break the loop.
  virtual bool on_received_command_number(i32 command_number, c_internal_base_weapon* weapon){
    FUNCTION_OVERRIDE_NEEDED;
    return true;
  }

  virtual void apply_spread_to_angle(vec3* angle, bool add = false, i32 seed_override = -1){
    FUNCTION_OVERRIDE_NEEDED;
  }

  bool is_good_seed(i32 seed, c_internal_base_weapon* weapon, bool speed = false){
    if(weapon == nullptr)
      return false;

    if(!should_modify_command_number(weapon))
      return false;

    static c_good_seed_data* data = (c_good_seed_data*)malloc(sizeof(c_good_seed_data));

    if(!setup_good_seed_data(data, weapon))
      return false;

    data->wants_speed = speed;
    return is_good_seed(seed, data);
  }

  bool find_command(c_internal_base_weapon* weapon){
    best_command++;
    for(i32 i = best_command; i < best_command + get_command_number_step(); i++){
      if(on_received_command_number(i, weapon))
        return true;
    }

    best_command += get_command_number_step();
    DBG("[-] c_base_accuracy::find_command - Unable to find special command number\n");
    return false;
  }
  // This needs a better name.
  virtual void init_runtime(c_user_cmd* cmd){
    if(cmd == nullptr)
      return;

    if(cmd->command_number == 0)
      return;

    if(!best_command)
      best_command = cmd->command_number + 1;
  }

  //pre_prediction : true - Call before aimbot is ran.
  //pre_prediction : false - Call when aimbot is targeting or firing.
  virtual bool create_move_run(c_user_cmd* cmd, bool pre_prediction){
    FUNCTION_OVERRIDE_NEEDED;
    return false;
  }

};

