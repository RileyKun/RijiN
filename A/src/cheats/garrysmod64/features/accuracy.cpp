#include "../link.h"

CLASS_ALLOC(c_accuracy, accuracy);

bool c_accuracy::do_spread_dump(){
  c_base_player* localplayer = utils::localplayer();

  if(localplayer == nullptr)
    return false;

  c_base_weapon* wep = localplayer->get_weapon();

  if(wep == nullptr)
    return false;

  // zero spread dump buffer
  memset(&dump, 0, sizeof(s_fire_bullets_info));

  // backup old weapon vars
  i32   next_think_tick     = wep->next_think_tick;
  float next_primary_attack = wep->next_primary_attack;
  float next_attack         = wep->next_attack;
  vec3  punch               = localplayer->punchangle();

  // set random seed
  global->current_cmd->random_seed = math::md5_pseudo_random(global->current_cmd->command_number) & 0xFF;

  is_dumping_spread = true;
  engine_prediction->run();
  // !IMPORTANT! call client prediction update after this! 
  is_dumping_spread = false;

  // restore weapon vars
  wep->next_think_tick     = next_think_tick;
  wep->next_primary_attack = next_primary_attack;
  wep->next_attack         = next_attack;
  localplayer->set_punchangle(punch);

  // return true if the attacker is correctly set
  // (which means we've dumped the spread data)
  return dump.attacker == localplayer;
}

bool c_accuracy::nospread(){
  if(!config->aimbot.no_spread)
    return false;

  c_base_player* localplayer = utils::localplayer();

  if(localplayer == nullptr)
    return false;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return false;

  if(wep->is_melee() || wep->is_harmless_weapon())
    return;

  if(!(global->current_cmd->buttons & IN_ATTACK) || !localplayer->can_fire())
    return false;

  vec3 punchangle = localplayer->punchangle();

  u8 wep_type = wep->scripted_type();

  if(wep_type == weapon_type_m9k || wep_type == weapon_type_unk || wep_type == weapon_type_hl2){
    if(!do_spread_dump())
      return false;

    // set random seed
    global->current_cmd->random_seed = math::md5_pseudo_random(global->current_cmd->command_number) & 0xFF;

    math::set_random_seed(global->current_cmd->random_seed);

    float s_x = math::random_float(-0.5f, 0.5f) + math::random_float(-0.5f, 0.5f);
    float s_y = math::random_float(-0.5f, 0.5f) + math::random_float(-0.5f, 0.5f);

    vec3 fwd, right, up;
    global->current_cmd->view_angles.normalize_angle();
    math::angle_2_vector(global->current_cmd->view_angles, &fwd, &right, &up);

    float r_m = -(dump.spread.x * s_x);
    float u_m = -(dump.spread.y * s_y);

    vec3 new_angle = fwd + right * r_m + up * u_m;
    math::vector_2_angle(&new_angle, &global->current_cmd->view_angles);

    global->info_panel->add_entry(INFO_PANEL_NOSPREAD, WXOR(L"NOSPREAD"), WXOR(L"RUNNING"), INFO_PANEL_RAGE_CLR);
  }
  else if(wep_type == weapon_type_cw || wep_type == weapon_type_fas){
    ILuaInterface* lua = global->lua_shared->GetLuaInterface(LUAINTERFACE_CLIENT);
    if(lua == nullptr)
      return false;

    if(!wep->is_lua_weapon())
      return false;
      

    // set random seed
    global->current_cmd->random_seed = math::md5_pseudo_random(global->current_cmd->command_number) & 0xFF;

    // recalculate cone
    {
      wep->push_entity();

      // LocalPlayer():GetActiveWeapon():CalculateSpread(vel, dt)
      lua->GetField(-1, XOR( "CalculateSpread" ));
      lua->Push(-2);
      lua->PushNumber(localplayer->velocity( ).length());
      lua->PushNumber(globaldata->frame_time);
      lua->Call(3, 0);

      lua->Pop(1);
    }

    // fetch cone
    {
      // LocalPlayer()s
      wep->push_entity();

      // LocalPlayer():GetActiveWeapon().CurCone
      lua->GetField(-1, XOR("CurCone"));
    }

    float cone = (float)lua->GetNumber(-1);

    lua->Pop(2);

    if(wep_type == weapon_type_cw)
      math::lua_random_seed(global->current_cmd->command_number);
    else if(wep_type == weapon_type_fas)
      math::lua_random_seed(globaldata->cur_time);

    const auto sx = -math::lua_random_float(-cone, cone);
    const auto sy = -math::lua_random_float(-cone, cone);

    global->current_cmd->view_angles.x += sx * 25.f;
    global->current_cmd->view_angles.y += sy * 25.f;

    //DBG("%f %f\n", sx * 25.f, sy * 25.f);
  }
  else if(false && wep_type == weapon_type_tfa){ // <---- Is this not supported?
    ILuaInterface* lua = global->lua_shared->GetLuaInterface(LUAINTERFACE_CLIENT);
    if(lua == nullptr)
      return false;

    // set random seed
    global->current_cmd->random_seed = math::md5_pseudo_random(global->current_cmd->command_number) & 0xFF;

    float old_ct = globaldata->cur_time;
    globaldata->cur_time = 0.f;

    // LocalPlayer()
    wep->push_entity();

    // LocalPlayer():GetActiveWeapon():CalculateConeRecoil()
    lua->GetField(-1, XOR("CalculateConeRecoil"));
    lua->Push(-2);
    lua->Call(1, 1);

    float cone = (float)lua->GetNumber(-1);

    lua->Pop(2);

    globaldata->cur_time = old_ct;

    math::set_random_seed(global->current_cmd->random_seed);

    float s_x = math::random_float(-0.5f, 0.5f) + math::random_float(-0.5f, 0.5f);
    float s_y = math::random_float(-0.5f, 0.5f) + math::random_float(-0.5f, 0.5f);

    vec3 fwd, right, up;
    global->current_cmd->view_angles.normalize_angle();
    math::angle_2_vector(global->current_cmd->view_angles, &fwd, &right, &up);

    float r_m = -(cone * s_x);
    float u_m = -(cone * s_y);

    vec3 new_angle = fwd + right * r_m + up * u_m;
    math::vector_2_angle(&new_angle, &global->current_cmd->view_angles);

    global->info_panel->add_entry(INFO_PANEL_NOSPREAD, WXOR(L"NOSPREAD"), WXOR(L"RUNNING"), INFO_PANEL_RAGE_CLR);
  }

  return true;
}

bool c_accuracy::norecoil(){
  if(!config->aimbot.no_recoil)
    return false;

  c_base_player* localplayer = utils::localplayer();

  if(localplayer == nullptr)
    return false;

  c_base_weapon* wep = localplayer->get_weapon();

  if(wep == nullptr)
    return false;

  if(!(global->current_cmd->buttons & IN_ATTACK) || !localplayer->can_fire())
    return false;

  u8 wep_type = wep->scripted_type();

  if(wep_type == weapon_type_cw || wep_type == weapon_type_fas || wep_type == weapon_type_tfa || wep_type == weapon_type_hl2)
    global->current_cmd->view_angles -= localplayer->punchangle();

  global->info_panel->add_entry(INFO_PANEL_NORECOIL, WXOR(L"NORECOIL"), WXOR(L"RUNNING"), INFO_PANEL_RAGE_CLR);
  return true;
}

bool c_accuracy::run(){
  nospread();
  norecoil();

  return true;
}

EXPORT void __fastcall fire_bullets_hook(void* rcx, s_fire_bullets_info* bullet_data){
  if(accuracy->is_dumping_spread && rcx == utils::localplayer()){
    memcpy(&global->bullets_info_dump, bullet_data, sizeof(s_fire_bullets_info));

    if(accuracy->is_dumping_spread){
      memcpy(&accuracy->dump, bullet_data, sizeof(s_fire_bullets_info));

      return;
    }
  }
  
  utils::call_fastcall64<void, s_fire_bullets_info*>(gen_internal->decrypt_asset(global->fire_bullets_hook_trp), rcx, bullet_data);
}