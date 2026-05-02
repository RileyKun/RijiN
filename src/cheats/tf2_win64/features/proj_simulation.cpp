#include "../link.h"

CLASS_ALLOC(c_projectile_simulation, proj_simulate);


s_projectile_simulation_context::s_projectile_simulation_context(c_base_weapon* wep, vec3 aim_angle, bool trace_check, bool disable_random_impulse, bool disable_random_velocity, bool use_point_method, vec3 end_point){
  reset();

  proj_simulate->init_vphysics();
  if(wep == nullptr)
    return;

  c_base_player* owner = wep->get_owner();
  if(owner == nullptr)
    return;

  if(wep->is_flamethrower() || wep->is_dragons_fury())
    return;

  if(!wep->get_projectile_size(proj_min, proj_max))
    return;

  // Remove beggars spread if possible. Otherwise it's impossible to see in projectile camera, trajectory lines, etc...
  if(wep->is_beggars())
    accuracy->apply_spread_to_angle(&aim_angle);
  
  wep->setup_random_seed(command_manager->get_seed());

  proj_angle          = aim_angle;
  proj_speed          = wep->get_projectile_speed();
  proj_gravity        = wep->get_projectile_gravity();
  is_physics_toss     = !proj_simulate->is_vphysics_weapon(wep);

  trace_filter        = proj_simulate->get_trace_filter(wep);
  use_traces          = trace_check;
  drag_coefficient    = 1.0f;

  // This fucking sucks. Clearly, needs to be re-done.
  max_predicted_ticks = math::time_to_ticks((wep->is_grenade_launcher()) ? 2.0f : 15.0f);
  if(wep->is_loose_cannon())
    max_predicted_ticks = math::time_to_ticks(1.f);

  if(proj_simulate->get_angle_impulse(wep, &angle_impulse)){
    if(disable_random_impulse){
      if(wep->is_ball_bat())
        angle_impulse = vec3(0.f, 0.f, 0.f);
      else if(wep->is_grenade_launcher() || wep->is_sticky_launcher() || wep->is_loose_cannon())
        angle_impulse = vec3(600.f, -1200.f, 0.f);
    }
  }

  proj_simulate->get_drag_basis(wep, &drag_basis, &drag_ang_basis);

  vec3 weapon_offset;
  if(wep->is_ball_bat())
    weapon_offset = vec3(0.f, 0.f, 0.f);
  else{
    if(!wep->get_weapon_offset(weapon_offset)){
      DBG("[-] Failing projectile simulation context setup (get_weapon_offset) invalid\n");
      return;
    }
  }

  vec3 shoot_pos;
  vec3 eye_angles;

  if(is_physics_toss){
    vec3 old_angle = owner->last_viewangle();
    owner->last_viewangle() = aim_angle;
    wep->get_projectile_fire_setup(owner, weapon_offset, &shoot_pos, &eye_angles);
    owner->last_viewangle() = old_angle;

    proj_angle = eye_angles;
    math::angle_2_vector(eye_angles, &fwd, &right, &up);
  }
  else if(wep->is_ball_bat()){ 
    eye_angles = aim_angle;
    proj_angle = aim_angle;
    math::angle_2_vector(eye_angles, &fwd, &right, &up);

    // Rebuilt from CTFBat_Wood::GetBallDynamics
    shoot_pos = owner->get_abs_origin() + vec3(owner->model_scale(), owner->model_scale(), owner->model_scale()) * (vec3(0.f, 0.f, 50.f) + fwd * 32.f);
  }
  else{
    // This is accurate for every vphysics projectile with the exception of sandman balls.
    eye_angles = aim_angle;
    proj_angle = aim_angle;

    math::angle_2_vector(eye_angles, &fwd, &right, &up);
    vec3 src = owner->shoot_pos() + (fwd * weapon_offset.x) + (right * weapon_offset.y) + (up * weapon_offset.z);

    // This is how the game setups the shoot offset.
    s_trace tr =  global->trace->ray_obb(owner->shoot_pos(), src, vec3(-8.f, -8.f, -8.f), vec3(8.f, 8.f, 8.f), mask_solid_brushonly, TR_CUSTOM_FILTER_NONE);
    if(tr.started_solid){ // We wouldn't be able to shoot with the weapon...
      DBG("[-] Failing projectile simulation context setup (VPhysics start position started in solid)\n");
      return;
    }

    shoot_pos = tr.end;
  }

  start_pos        = shoot_pos;
  predicted_origin = shoot_pos;
  highest_origin   = shoot_pos;

  if(!wep->get_vphysics_velocity(&predicted_velocity, eye_angles, disable_random_velocity))
    predicted_velocity = (fwd * proj_speed);

  // If used in aimbot, it should be the point that isn't gravity corrected and such. Otherwise it'll always fail or get fail-positive vis checks.
  if(use_point_method){
    end_pos            = end_point;
    wants_end_position = true;
    dir_to_target      = (end_pos - start_pos);
    dist_to_end        = dir_to_target.length_sqr();
    trace_filter       = TR_CUSTOM_FILTER_HIT_TEAM_NO_PLAYERS;

    // Rather than modifying the position and create undefined issues. We'll just error out.
    //s_trace test_tr = global->trace->ray_obb(end_pos, end_pos, proj_min, proj_max, mask_solid, TRACE_EVERYTHING, nullptr, TR_CUSTOM_FILTER_HIT_TEAM_NO_PLAYERS);
    //if(test_tr.started_solid){
    //  DBG("[!] (Will simulate anyways) projectile trajectory constructor: (point method, the end position is in a solid, this is likely not an issue, it could be)\n");
    //}
  }

  if(!is_physics_toss)
    proj_simulate->setup_vphysics_parameters(start_pos, proj_angle, predicted_velocity, angle_impulse, 1.f, drag_basis, drag_ang_basis, true);

  start_velocity = predicted_velocity;
  valid = true;
  setup = true;
}

void c_projectile_simulation::init_vphysics(){
  if(env != nullptr || object != nullptr)
    return;

  DBG("[+] c_projectile_simulation::init_vphysics - INIT\n");
  env = global->physics->create_env();
  if(env == nullptr){
    assert(false);
    return;
  }

  // I wrote this comment while setting up this function, before I had a chance to dump the sizes of throwable objects.
  // We need a function that can override the size, in the event that throwable weapons have different sizes, which I am pretty sure they do.
  vec3 mins = vec3(-2.f, -2.f, -2.f);
  vec3 maxs = vec3(2.f, 2.f, 2.f);

  DBG("[+] c_projectile_simulation::init_vphysics - getting collide object\n");
  void* obj_col = global->physics_collision->bbox_to_collide(mins, maxs);
  assert(obj_col != nullptr);

  DBG("[+] c_projectile_simulation::init_vphysics - creating physics object\n");
  s_object_params params;
  params.setup();
  object = env->create_physics_object(obj_col, vec3(0.f, 0.f, 0.F), vec3(0.f, 0.f, 0.f), &params);
  if(object == nullptr){
    assert(false);
    return;
  }

  DBG("[+] c_projectile_simulation::init_vphysics - wake up\n");
  object->wake();
  DBG("[+] c_projectile_simulation::init_vphysics - OK\n");
}

void c_projectile_simulation::setup_vphysics_parameters(c_base_weapon* weapon){
  if(weapon == nullptr)
    return;

  s_projectile_simulation_context ctx(weapon, global->engine->get_view_angles(), false);

  bool has_limit = weapon->is_grenade_launcher() || weapon->is_loose_cannon() || weapon->is_loch_and_load() || weapon->is_sticky_launcher();

  setup_vphysics_parameters(ctx.start_pos, ctx.proj_angle, ctx.predicted_velocity, ctx.angle_impulse, ctx.drag_coefficient, ctx.drag_basis, ctx.drag_ang_basis, has_limit);
}

void c_projectile_simulation::setup_vphysics_parameters(vec3 pos, vec3 angle, vec3 vel, vec3 angle_impulse, float drag, vec3 drag_basis,vec3 drag_ang_basis, bool has_velocity_limit){
  if(!is_vphysics_setup()){
    DBG("[-] c_projectile_simulation::setup_vphysics_parameters - init_vphysics needs to be called\n");
    return;
  }

  object->set_position(pos, angle);
  object->set_velocity(&vel, &angle_impulse);
  object->set_drag_coefficient(&drag);

  object->drag_basis     = drag_basis;
  object->drag_ang_basis = drag_ang_basis;

  float gravity = -800.f; 
  static bool init_env = false;
  if(!init_env){
    static s_physics_performance perf;
    perf.init();

    env->set_performance_settings(&perf);
    env->set_air_density(2.f);
    env->set_gravity(gravity);
    init_env = true;
  }
}

void c_projectile_simulation::simulate_vphysics(s_projectile_simulation_context* ctx){
  if(ctx == nullptr)
    return;

  if(!is_vphysics_setup())
    return;

  // This will simulate the object forward. It's collision checks are disabled.
  env->simulate(globaldata->interval_per_tick);

  // Copy data from object.
  object->get_position(&ctx->predicted_origin, &ctx->proj_angle);
  object->get_velocity(&ctx->predicted_velocity, &ctx->angle_impulse);
}

bool c_projectile_simulation::predict(s_projectile_simulation_context* ctx, i32 ticks){
  if(ctx == nullptr)
    return false;

  if(ticks < 1)
    return false;

  for(i32 i = 1; i < ticks; i++){
    if(!predict_tick(ctx))
      break;
  }

  return ctx->valid;
}

bool c_projectile_simulation::predict_tick(s_projectile_simulation_context* ctx){
  if(ctx == nullptr){
    DBG("[-] ERROR: predict_tick context was nullptr.\n");
    return false;
  }

  if(!ctx->valid){
    if(ctx->wants_end_position){
      DBG("[-] ERROR: predict_tick context was invalid. Current prediction tick: %i\n", ctx->predicted_ticks);
    }
    return false;
  }

  ctx->predicted_ticks++;
  vec3 current_origin = ctx->predicted_origin;

  move(ctx);
  apply_gravity(ctx);

  if(current_origin.z < ctx->predicted_origin.z)
    ctx->highest_origin = ctx->predicted_origin;
  
  if(ctx->use_traces){
    ctx->tr = global->trace->ray_obb(current_origin, ctx->predicted_origin, ctx->proj_min, ctx->proj_max, mask_solid, TRACE_EVERYTHING, nullptr, ctx->trace_filter);
    if(ctx->tr.started_solid){
      ctx->valid = false;
      return false;
    }
    else if(ctx->tr.fraction < 1.0f){
      ctx->predicted_origin = ctx->tr.end;
      ctx->valid = !ctx->wants_end_position;
      return false;
    }
    else if(ctx->wants_end_position){
      vec3 cur_dir = (ctx->predicted_origin - ctx->start_pos);
      if(cur_dir.dot(ctx->dir_to_target) >= ctx->dist_to_end){
        ctx->valid = (ctx->tr.fraction >= 1.f);
        return false;
      }
    }
  #if defined(SHOW_DEBUG_LINES) && defined(DEV_MODE)
    render_debug->draw_line(current_origin, ctx->predicted_origin, ctx->use_traces ? colour(255, 0, 0, 255) : colour(255, 0, 255, 255), true, globaldata->interval_per_tick * 2.f);
  #endif
  }

  if(ctx->predicted_ticks >= ctx->max_predicted_ticks)
    return false;
  
  return true;
}

void c_projectile_simulation::move(s_projectile_simulation_context* ctx){
  if(ctx->is_physics_toss){
    if(ctx->_single_step){
      for(i32 i = 0; i <= 2; i++){
        ctx->predicted_origin[i] += ctx->predicted_velocity[i] * 10.f;
      }
    }
    else{
      for(i32 i = 0; i <= 2; i++){
        if(i == 2 && ctx->proj_gravity != 0.0f) // This projectile has no gravity.
          continue;
  
        ctx->predicted_origin[i] += ctx->predicted_velocity[i] * globaldata->interval_per_tick;
      }
    }
  }
  else
    simulate_vphysics(ctx);
}

void c_projectile_simulation::apply_gravity(s_projectile_simulation_context* ctx){
  if(ctx->is_physics_toss){
    if(ctx->proj_gravity == 0.0f)
      return;

    float height = ctx->predicted_velocity.z - (ctx->proj_gravity * global->sv_gravity->flt) * globaldata->interval_per_tick;
    ctx->predicted_origin.z += ((ctx->predicted_velocity.z + height) * 0.5f) * globaldata->interval_per_tick;
    ctx->predicted_velocity.z = height;
  }

  clamp_velocity(ctx);
}

void c_projectile_simulation::clamp_velocity(s_projectile_simulation_context* ctx){
  if(ctx == nullptr)
    return;

  if(!ctx->is_physics_toss)
    return;
  
  // Valve are pasters and set values in the velocity rather than scaling it down.
  for(i32 i = 0; i <= 2; i++){
    if(ctx->predicted_velocity[i] > global->sv_maxvelocity->flt)
      ctx->predicted_velocity[i] = global->sv_maxvelocity->flt;
    else if(ctx->predicted_velocity[i] < -global->sv_maxvelocity->flt)
      ctx->predicted_velocity[i] = -global->sv_maxvelocity->flt;
  }
}

bool c_projectile_simulation::should_draw_trajectory_visualizer(){
  if(global->engine->is_playing_demo())
    return false;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return false;

  if(!localplayer->is_valid())
    return false;

  if(localplayer->is_taunting() || localplayer->is_bonked())
    return false;

  c_base_weapon* wep = localplayer->get_weapon();
  if(wep == nullptr)
    return false;

  if(wep->is_flamethrower() || wep->is_wrangler() || wep->is_dragons_fury())
    return false;

  if(!wep->is_projectile_weapon() && !wep->is_ball_bat())
    return false;

  #if !defined(DEV_MODE)
    if(wep->is_ball_bat())
      return false;
  #endif

  return true;
}

bool c_projectile_simulation::get_angle_impulse(c_base_weapon* weapon, vec3* impulse){
  if(weapon == nullptr || impulse == nullptr)
    return false;

  // If it's not a VPhysics weapon then we'll just zero the vector out. Even thought it's never used.
  if(!is_vphysics_weapon(weapon)){
    *impulse = vec3(0.f, 0.f, 0.f);
    return false;
  }

  *impulse = vec3(600.f, math::random_int(-1200, 1200), 0.f);
  if(weapon->is_loch_and_load())
    *impulse = vec3(0.f, 0.f, 0.f);
  else if(weapon->is_cleaver())
    *impulse = vec3(0.f, 500.f, 0.f);
  else if(weapon->is_throwables())
    *impulse = vec3(300.f, 0.f, 0.f);
  else if(weapon->is_ball_bat())
    *impulse = vec3(0.f, math::random_float(0.f, 100.f), 0.f);

  // Returning means we 'wrote' actual data to the impulse parameter.
  return true;
}

bool c_projectile_simulation::get_drag_basis(c_base_weapon* weapon, vec3* drag_basis, vec3* drag_ang_basis){
  if(weapon == nullptr || drag_basis == nullptr || drag_ang_basis == nullptr)
    return false;

  // Default value for everything else.
  *drag_basis     = vec3(0.003902f, 0.009962f, 0.009962f);
  *drag_ang_basis = vec3(0.003618f, 0.001514f, 0.001514f);

  if(weapon->is_loose_cannon()){
    *drag_basis     = vec3(0.020971f, 0.019420f, 0.020971f);
    *drag_ang_basis = vec3(0.012997f, 0.013496f, 0.013714f);
  }
  else if(weapon->is_sticky_launcher()){
    *drag_basis     = vec3(0.007491f, 0.007491f, 0.007306f);
    *drag_ang_basis = vec3(0.002777f, 0.002842f, 0.002812f);
  }
  else if(weapon->is_cleaver()){
    *drag_basis     = vec3(0.022287f, 0.005208f, 0.110697f);
    *drag_ang_basis = vec3(0.013982f, 0.043243f, 0.003465f);
  }
  else if(weapon->is_mad_milk_jar()){
    *drag_basis     = vec3(0.005514f, 0.002313f, 0.005558f);
    *drag_ang_basis = vec3(0.000684f, 0.001439f, 0.000680f);
  }
  else if(weapon->is_jarate_jar()){
    *drag_basis     = vec3(0.005127f, 0.002925f, 0.004337f);
    *drag_ang_basis = vec3(0.000641f, 0.001350f, 0.000717f);
  }
  else if(weapon->is_gas_jar()){
    *drag_basis     = vec3(0.026360f, 0.021780f, 0.058978f);
    *drag_ang_basis = vec3(0.035050f, 0.031199f, 0.022922f);
  }
  else if(weapon->is_sandman()){
    *drag_basis     = vec3(0.006645f, 0.006581f, 0.006710f);
    *drag_ang_basis = vec3(0.002233f, 0.002246f, 0.002206f);
  }
  else if(weapon->is_wrap_assassin()){
    *drag_basis     = vec3(0.010867f, 0.010873f, 0.010804f);
    *drag_ang_basis = vec3(0.002081f, 0.002162f, 0.002069f);
  }

  return true;
}

i32 c_projectile_simulation::get_trace_filter(c_base_weapon* weapon){
  if(weapon == nullptr)
    return TR_CUSTOM_FILTER_NONE;

  if(weapon->is_crossbow())
    return TR_CUSTOM_FILTER_HIT_TEAM;
  else if(weapon->is_rescue_ranger())
    return TR_CUSTOM_FILTER_HIT_TEAM_NO_PLAYERS;

  return TR_CUSTOM_FILTER_NONE;
}

bool c_projectile_simulation::is_vphysics_weapon(c_base_weapon* weapon){
  if(weapon == nullptr)
    return false;

  return weapon->is_grenade_launcher() || weapon->is_loose_cannon() || weapon->is_sticky_launcher() || weapon->is_throwables() || weapon->is_cleaver() || weapon->is_ball_bat();
}