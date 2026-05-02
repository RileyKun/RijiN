#include "../link.h"

c_accuracy* accuracy = nullptr;

void c_accuracy::set_prediction_random_seed(i32 seed){
  *global->prediction_random_seed = seed;
}

bool c_accuracy::is_seed_predicting(){
  return seed_prediction->active() && seed_prediction->is_synced();
}

bool c_accuracy::set_command(i32 command_number){
  command_manager->set_command(command_number);
  return c_base_accuracy::set_command(command_number);
}

i32 c_accuracy::get_seed(){
  return command_manager->get_seed();
}

bool c_accuracy::get_best_bullet_with_seed(vec3 current_angle, i32* bullet){
  if(bullet == nullptr)
    return false;

  *bullet = 0;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  ctf_weapon_info* wep_info = weapon->get_weapon_info();
  if(wep_info == nullptr)
    return false;

  if(wep_info->bullets_per_shot <= 1)
    return false; 

  vec3 shoot_pos           = localplayer->shoot_pos();
  u32 max_bullets_per_shot = math::smallest(wep_info->bullets_per_shot, 9);
  u32 best_traces          = 0;
  u32 best_bullet          = 0;

  i32 seed = get_seed();
  if(is_seed_predicting())
    seed = seed_prediction->get_seed();

  u32  math_type = 0;
  vec3 spread_angle;
  for(u32 bullet = 0; bullet < max_bullets_per_shot; bullet++){

    // Correct angle.
    vec3 corrected_angle = current_angle;
    apply_spread_to_angle(&corrected_angle, false, (seed + bullet) & 255);

    // Test to see how many other bullets hit entities.
    i32 hit_traces = 0;
    for(u32 c = 0; c < max_bullets_per_shot; c++){
      vec3 spread_angle = current_angle;
      apply_spread_to_angle(&spread_angle, true, (seed + bullet) & 255);

      vec3 fwd;
      math::angle_2_vector(corrected_angle, &fwd, nullptr, nullptr);

      s_trace tr = global->trace->ray(shoot_pos, shoot_pos + (fwd * wep_info->range), mask_bullet);
      if(tr.hit_entity == nullptr)
        continue;

      c_base_entity* entity = (c_base_entity*)tr.hit_entity;
      if(!entity->is_player() && !entity->is_sentry() && !entity->is_dispenser() && !entity->is_teleporter())
        continue;

      hit_traces++;
      if(hit_traces > best_traces){
        best_bullet = bullet;
        best_traces = hit_traces;
      }
    }
  }

  *bullet = best_bullet;
  return true;
}

CFLAG_O0 bool c_accuracy::get_spread(vec3* out, u32& math_type, i32 seed){
  if(out == nullptr)
    return false;

  *out = vec3();

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  i32 current_seed = (seed == -1) ? get_seed() : seed;

  if(weapon->is_syringe_gun()){
    weapon->setup_random_seed(current_seed);

    float x = math::random_float(-1.5f, 1.5f);
    float y = math::random_float(-1.5f, 1.5f);

    *out = vec3(x, y, 0.f);
    return true;
  }
  else if(weapon->is_beggars()){
    weapon->setup_random_seed(current_seed);
    vec3 spread;
    vec3 old_view = localplayer->last_viewangle();
    localplayer->last_viewangle() = vec3();
    utils::call_fastcall64<void, vec3&>(global->get_spread_angles_addr, weapon, spread);
    localplayer->last_viewangle() = old_view;
    *out = vec3(spread.x, spread.y, 0.f);
    return true;
  }
  else{
    float spread = weapon->get_spread();
    if(spread <= 0.0f)
      return false;

    i32 _seed = current_seed;
    if(is_seed_predicting())
      _seed = seed_prediction->get_seed();

    math::set_random_seed(_seed & 255);

    float sx = math::random_float(-0.5f, 0.5f) + math::random_float(-0.5f, 0.5f);
    float sy = math::random_float(-0.5f, 0.5f) + math::random_float(-0.5f, 0.5f);
    *out      = vec3(spread * sx, spread * sy, 0.f);
    math_type = 1;
    return true;
  }

  return false;
}

bool c_accuracy::is_nospread_active(){
  if(global->aimbot_settings == nullptr)
    return false;

  if(utils::is_safe_mode_enabled())
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(weapon->is_melee() || weapon->is_harmless_item() || weapon->is_wrangler() || weapon->is_medigun())
    return false;

  if(weapon->is_hitscan_weapon()){
    if(weapon->get_spread() <= 0.0f)
      return false;

    if(weapon->is_perfect_shot() && !weapon->is_minigun())
      return false;

    if(utils::is_match_making_server() && !is_seed_predicting())
      return false;

    // Since seed prediction has no use other than with nospread. If it's not turned off then lets do nospread!
    if(!global->aimbot_settings->no_spread && !global->aimbot_settings->predict_server_random_seed)
      return false;
  }
  else{
    if(!global->aimbot_settings->no_spread)
      return false;
  }

  return true;
}

bool c_accuracy::should_modify_command_number(c_internal_base_weapon* internal_weapon){
  if(internal_weapon == nullptr)
    return false;

  c_base_weapon* weapon = (c_base_weapon*)internal_weapon;

  if(weapon->is_rocket_launcher() || weapon->is_beggars() || weapon->is_syringe_gun() || weapon->is_melee() || weapon->is_harmless_item() || weapon->is_wrangler() || weapon->is_medigun() || weapon->is_crossbow())
    return false;

  if(weapon->is_hitscan_weapon()){
    if(is_seed_predicting())
      return false;

    if(utils::is_match_making_server())
      return false;
  }

  return true;
}

bool c_accuracy::setup_good_seed_data(c_good_seed_data* data, c_internal_base_weapon* internal_weapon){
  if(data == nullptr || internal_weapon == nullptr)
    return false;

  c_base_weapon* weapon = (c_base_weapon*)internal_weapon;

  memset(data, 0, sizeof(c_good_seed_data));
  if(!should_modify_command_number(weapon)){
    data->valid = false;
    return false;
  }

  data->is_shotgun          = weapon->is_shotgun() || weapon->weapon_id() == WPN_Shortstop;
  data->is_hitscan          = weapon->is_hitscan_weapon();
  data->is_grenade_launcher = (weapon->is_grenade_launcher() || weapon->is_sticky_launcher() || weapon->is_loose_cannon() || weapon->is_loch_and_load());
  data->is_loch_load        = weapon->is_loch_and_load();
  data->valid               = true;

  return true;
}

bool c_accuracy::is_good_seed(i32 seed, c_good_seed_data* data){
  if(data == nullptr)
    return true;

  if(!data->valid){
    DBG("[!] c_accuracy::is_good_seed (!data->valid)\n");
    return true;
  }

  if(data->is_shotgun)
    return (seed & 255) == 142;
  else if(data->is_hitscan)
    return (seed & 255) == 39;
  else if(data->is_grenade_launcher){
    math::setup_random_seed(6, seed, false);
    
    // Prefer the lowest amount of spin for the pill.
    if(!data->wants_speed && !data->is_loch_load){
     
      float right = math::random_float(-10.f, 10.f);
      float up    = math::random_float(-10.f, 10.f);
      
      i32 range = 1;
      i32 random_spin = math::random_int(-1200, 1200);
      return random_spin >= -range && random_spin <= range;
    }
    else{
      float right = math::random_float(-10.f, 10.f);
      float up    = math::random_float(-10.f, 10.f);

      return up > 0.0f;  
    }
  }

  DBG("[-] c_accuracy::is_good_seed unhandled return assuming (true) output\n");
  return true;
}

bool c_accuracy::should_force_command_number(){
  if(config->misc.crithack_enabled || !acm->should_allow_command_number_changes())
    return false;

  return true;
}

bool c_accuracy::calculate_command_number(c_user_cmd* cmd, c_internal_base_weapon* weapon){
  if(!should_force_command_number())
    return false;

  init_runtime(cmd);
  if(weapon == nullptr)
    return false;

  if(find_command(weapon))
    return true;

  DBG("[-] calculate_command_number failed: (couldn't find a magic command_number)\n");
  return false;
}

bool c_accuracy::on_received_command_number(i32 command_number, c_internal_base_weapon* weapon){
  i32 seed = math::md5_pseudo_random(command_number) & 0x7FFFFFFF;
  if(!c_base_accuracy::is_good_seed(seed, weapon, false))
    return false;

  set_command(command_manager);
  return true;
}

void c_accuracy::apply_spread_to_angle(vec3* angle, bool add, i32 seed_override){
  if(angle == nullptr)
    return false;

  vec3 spread   = vec3();
  u32  math_type = 0;
  if(!get_spread(&spread, math_type, seed_override))
    return false;

  // Hitscan!
  if(math_type == 1){
    vec3 fwd, right, up;
    math::angle_2_vector(*angle, &fwd, &right, &up);

    float right_mod = add ? spread.x : -spread.x;
    float up_mod    = add ? spread.y : -spread.y;

    vec3 spread_recalculated = fwd + right * right_mod + up * up_mod;
    math::vector_2_angle(&spread_recalculated, angle);
  }
  else{
    if(!add)
      *angle -= spread;
    else
      *angle += spread;
  }

  math::clamp_angles(*angle);
  return true;
}

bool c_accuracy::create_move_run(c_user_cmd* cmd, bool pre_prediction){
  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  c_base_weapon* weapon = localplayer->get_weapon();
  if(weapon == nullptr)
    return false;

  if(pre_prediction)
    return calculate_command_number(cmd, weapon);
  
  if(!is_nospread_active())
    return false;

  // We need these values synced so everything works properly.
  cmd->random_seed = math::md5_pseudo_random(cmd->command_number) & 0x7FFFFFFF;
  set_prediction_random_seed(is_seed_predicting() ? seed_prediction->get_seed() : cmd->random_seed); // Wow the desync! FELON!
  
  apply_spread_to_angle(&cmd->view_angles);

  return true; 
}