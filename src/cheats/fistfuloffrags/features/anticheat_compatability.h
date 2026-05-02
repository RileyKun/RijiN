#pragma once
enum ACP_SERVER_PLUGINS{
  ACP_SCAN_STARTED = 1 << 1,
  ACP_SCAN_FINISHED = 1 << 2,
  ACP_SMAC = 1 << 3,
  ACP_STAC = 1 << 4,
  ACP_LILAC = 1 << 5,
  ACP_OTHER_AC = 1 << 6,
  ACP_ANTI_BACKTRACK = 1 << 7,
};

class c_acp{
public:

  void createmove(bool post_createmove){
    assert(global->current_cmd != nullptr);
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    run_plugin_scan();
    if(post_createmove){
      if(is_active()){
        clamp_viewangles();
        bypass_aimbot_angle_repeat();
        bypass_aimbot_snaps();
        bypass_aimbot_auto_shoot();
      }
    }
  }

  // Used to detect plugins with the "sm plugins" command.
  bool on_server_print(i8* txt){
    if(txt == nullptr)
      return false;

    if(!(flags & ACP_SCAN_STARTED) || flags & ACP_SCAN_FINISHED || !is_scanning())
      return false;

    txt = str_utils::tolower(txt);
    if(str_utils::contains(txt, XOR("to see more, type \"sm plugins"))){
      current_sm_plugins++;

      i8 buf[256];
      wsprintfA(buf, XOR("sm plugins %i%i"), current_sm_plugins, current_sm_plugins);
      global->engine->client_cmd(buf);
    }
    else if(str_utils::contains(txt, XOR("smac")) || str_utils::contains(txt, XOR("sourcemod anti-cheat")))
      flags |= ACP_SMAC;
    else if(str_utils::contains(txt, XOR("steph's anticheat")))
      flags |= ACP_STAC;
    else if(str_utils::contains(txt, XOR("little")) && str_utils::contains(txt, "anti-cheat"))
      flags |= ACP_LILAC;
    else if(str_utils::contains(txt, XOR("anticheat")) || str_utils::contains(txt, XOR("anti-cheat")))
      flags |= ACP_OTHER_AC;
    else if(str_utils::contains(txt, XOR("backtrack")))
      flags |= ACP_ANTI_BACKTRACK;

    return true;
  }

  bool on_cvar_query(s_respond_cvar_value* respond_var){
    if(respond_var == nullptr)
      return true;

    // !! We currently don't need to spoof cvar values until we actually have a feature for it.
    // !! Just need away to get all of the current values of the "replicated" cvar values to us. Some of them aren't given to us.

    /*c_cvar* var = global->cvar->find_var(respond_var->name);
    if(var == nullptr){
      if(true){
        i8 buf[128];
        wsprintfA(buf, XOR("Server queried \"%s\" which is not a valid cvar.\n"), respond_var->name);
        utils::send_con_notification(buf);
      }

      respond_var->query_status = query_cvar_value_not_found;
      return false;
    }

    u32 name_hash = HASH_RT(respond_var->name);
    if(name_hash == HASH("mat_dxlevel") || name_hash == HASH("mp_fadetoblack"))
      return false;

    // Tell the end user about this.
    if(true){
      i8 buf[128];
      wsprintfA(buf, XOR("Server queried \"%s\" spoofing value \"%s\" to \"%s\"!\n"), respond_var->name, respond_var->val, var->default_value);
      utils::send_con_notification(buf);
    }

    respond_var->val = var->default_value;*/
    return false;
  }

  void run_plugin_scan(){
    if(!is_scanning() && scan_time > 0.f){
      on_scan_finish();
      return;
    }

    if(flags & ACP_SCAN_STARTED || flags & ACP_SCAN_FINISHED)
      return;

    DBG("[!] Starting scan.\n");
    flags    |= ACP_SCAN_STARTED;
    scan_time = math::time() + 2.0f;
    current_sm_plugins = 1;

    global->engine->client_cmd(XOR("sm plugins 11"));
  }

  void on_scan_finish(){
    scan_time = -1.f;
    flags    &= ~ACP_SCAN_STARTED;
    flags    |= ACP_SCAN_FINISHED;


    if(flags & ACP_SMAC)
      DBG("[!] Detected SMAC.\n");

    if(flags & ACP_LILAC)
      DBG("[!] Detected Lilac\n");

    if(flags & ACP_OTHER_AC)
      DBG("[!] Detected Other AC\n");

    if(flags & ACP_STAC)
      DBG("[!] Detected STAC\n");

    DBG("[!] Scan finished!\n");
  }

  bool allow_bullet_time(){
    return !(flags & ACP_STAC || flags & ACP_LILAC || is_scanning());
  }

  bool is_active(){
    if(is_scanning())
      return true;

    if(flags & ACP_SMAC || flags & ACP_LILAC || flags & ACP_STAC)
      return true;

    return false;
  }

  ALWAYSINLINE bool is_scanning(){
    return scan_time > math::time();
  }

  ALWAYSINLINE void clamp_viewangles(){
    assert(global->current_cmd != nullptr);
    global->current_cmd->view_angles = math::clamp_angles(global->current_cmd->view_angles);
    global->current_cmd->view_angles.z = 0.f;
  }

  ALWAYSINLINE void bypass_aimbot_auto_shoot(){
    assert(global->current_cmd != nullptr);
    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_valid())
      return;

    if(localplayer->water_level > 2)
      return;

    c_base_weapon* weapon = localplayer->get_weapon();
    if(weapon == nullptr)
      return;

    if(weapon->clip1 == 0)
      return;

    if(weapon->is_grenade())
      return;

    if(!localplayer->can_fire())
      global->current_cmd->buttons |= IN_ATTACK;
  }

  ALWAYSINLINE void bypass_aimbot_angle_repeat(){
    assert(global->current_cmd != nullptr);
    // Avoid "false" angle repeat preventions.
    if(!init_angle){
      for(i32 i = 0; i < 2; i++)
        last_angle[i] = global->current_cmd->view_angles;

      init_angle = true;
    }

    vec3 current_angle = global->current_cmd->view_angles;
    float delta = math::abs(math::normalize_angle((last_angle[0] - current_angle).length()));

    /* This will get triggered with our nospread. (Nothing happens to the accuracy of it)
       Gets around lilac angle repeat detections and I'll assume STAC also.

       current_angle
       last_angle
       last_of_last_angle

       * if current_angle is equal to last_of_last_angle but not equal to last_angle then an angle repeat has happened.
       * The 2 anti-cheats also check if your current_angle and last angle are more than 5 degrees apart to avoid false detections.
       * We do at least 1 degree.
    */
    if(delta >= 1.0f){
      for(int xy = 0; xy <= 1; xy++){
        if((i32)last_angle[1][xy] == (i32)current_angle[xy] && (i32)last_angle[0][xy] != (i32)current_angle[xy]){
          // Apply our last view angle and remove our IN_ATTACK buttons.
          global->current_cmd->view_angles = last_angle[0];
          global->current_cmd->buttons    &= ~IN_ATTACK;
          break;
        }
      }
    }

    last_angle[1] = last_angle[0];
    last_angle[0] = current_angle;
  }

  ALWAYSINLINE void bypass_aimbot_snaps(){
    vec3 target_angle = global->current_cmd->view_angles;
    vec3 angle_delta  = target_angle - step_angle;
    angle_delta.normalize_angle();

    float max_total_step        = 34.f;
    u8    py_arrive             = 0;
    bool  lilac_mode            = flags & ACP_LILAC;
    bool  smac_mode            = flags & ACP_SMAC;

    for(u32 i = 0; i <= 1; i++){
      float max_step = max_total_step;

      if(lilac_mode){
        max_total_step = 10.f;
        max_step = math::abs(angle_delta[i]) - 10.f;
        max_step = math::clamp(max_step, 5.f, max_total_step);
      }

      if(angle_delta[i] > max_step)
        step_angle[i] += max_step;
      else if(angle_delta[i] < -max_step)
        step_angle[i] -= max_step;
      else{
        step_angle[i] = target_angle[i];
        py_arrive++;
      }
    }
    step_angle.z = 0.f;
    step_angle.normalize_angle();

    // If we don't snap two angles we are still stepping
    is_angle_stepping = py_arrive != 2;

    // If we are stepping and we want to fire, dont let us yet
    if(is_angle_stepping && (global->current_cmd->buttons & IN_ATTACK))
      global->current_cmd->buttons &= ~IN_ATTACK;

    // Set our new view angles to the stepped angle set
    global->current_cmd->view_angles = step_angle;

    //clamp_viewangles();
  }

  ALWAYSINLINE void reset(){
    DBG("[!] acp reset\n");
    memset(this, 0, sizeof(*this));
  }

  bool init_angle         = false;
  vec3 current_angle;
  vec3 last_angle[2];
  vec3 step_angle;
  bool is_angle_stepping  = false;

  u32 flags               = 0;
  u32 current_sm_plugins  = 1;
  float scan_time         = -1.f;
};

CLASS_EXTERN(c_acp, acp);