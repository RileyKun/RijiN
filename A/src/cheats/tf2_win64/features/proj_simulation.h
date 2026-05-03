#pragma once
#define AIR_DENSITY 2.0f

//#define SHOW_DEBUG_LINES
struct s_projectile_simulation_context{
  vec3 start_pos; // Where the simulation first started.
  vec3 start_velocity; // What the velocity was when it first started.
  vec3 predicted_origin; // Current predicted position.
  vec3 predicted_velocity; // Current speed of our projectile.
  vec3 highest_origin; // The highest origin recorded before we began falling down again.
  vec3 end_pos; // Used to test if a position is visible.
  bool wants_end_position; // True if its a position test not an entity one.
  float dist_to_end; // The distance to the end.
  vec3  dir_to_target;

  float proj_speed; // Speed of the projectile.
  float proj_gravity; // Gravity of the projectile.

  float drag_coefficient;
  vec3  angle_impulse; // The angle impulse of the projectile.
  vec3  drag_basis;
  vec3  drag_ang_basis;

  vec3  proj_angle;
  vec3  fwd;
  vec3  right;
  vec3  up;

  i32 current_random_seed; // Our current random seed.

  bool is_physics_toss; // If true then we use PhysicsToss simulation otherwise we use a very basic VPhysics simulation.
  bool use_traces; // If true we'll perform traces to ensure visibility of the next tick's position.
  bool valid; // Is this prediction valid?
  bool setup; // Has the prediction be setup successfully?
  i32 trace_filter; // The trace filter we use.
  s_trace tr;

  i32 predicted_ticks; // The amount of ticks simulated.
  i32 max_predicted_ticks; // The max amount of ticks to be simulated before failing.

  vec3 proj_min;
  vec3 proj_max;

  bool  _single_step;

  s_projectile_simulation_context(){
    reset();
  }


  s_projectile_simulation_context(c_base_weapon* wep, vec3 aim_angle, bool trace_check = true, bool disable_random_impulse = false, bool disable_random_velocity = false, bool use_point_method = false, vec3 end_point = vec3());

  bool is_gravity(){
    return proj_gravity > 0.0f || !is_physics_toss;
  }

  void single_step(){
    if(is_gravity())
      _single_step = false;
    else
      _single_step = true;
  }

  bool is_single_step(){
    return _single_step;
  }

  ALWAYSINLINE void reset(){
    memset(this, 0, sizeof(*this));
  }
};

class c_projectile_simulation{
public:
  void init_vphysics();
  void setup_vphysics_parameters(c_base_weapon* weapon);
  void setup_vphysics_parameters(vec3 pos, vec3 angle, vec3 vel, vec3 angle_impulse, float drag, vec3 drag_basis, vec3 drag_ang_basis, bool has_velocity_limit = true);
  void simulate_vphysics(s_projectile_simulation_context* ctx);

  bool predict(s_projectile_simulation_context* ctx, i32 ticks);
  bool predict_tick(s_projectile_simulation_context* ctx);
  void move(s_projectile_simulation_context* ctx);
  void apply_gravity(s_projectile_simulation_context* ctx);
  void clamp_velocity(s_projectile_simulation_context* ctx);
  bool should_draw_trajectory_visualizer();

  void setup_constant_lines(c_base_player* localplayer){
    if(localplayer == nullptr)
      return;
    
    if(localplayer->is_taunting() || localplayer->is_bonked())
      return;
    
    c_base_weapon* wep = localplayer->get_weapon();
    if(wep == nullptr)
      return;

    if(wep->is_wrangler())
      return;

    if(!config->visual.projectile_trajectory_visualizer)
      return;

    if(!config->visual.projectile_trajectory_constant_line && !config->visual.projectile_trajectory_impact_camera)
      return;

    bool  charge_time_needs_reset  = false;
    float old_charge_time          = -1.f;

    // If we're using the auto sticky spam feature. Lets trick the simulation to use a sticky charge value at this set 
    if(wep->is_sticky_launcher() && !wep->is_blast_jumper_weapon()){
      if(config->automation.auto_sticky_spam){
        if(!config->automation.auto_sticky_spam_use_key || input_system->held(config->automation.auto_sticky_spam_key)){
          float max_charge     = wep->get_sticky_launcher_max_charge() * (float)math::clamp(config->automation.auto_sticky_spam_at_charge, 1, 100) / 100.f;
          float current_charge = (globaldata->cur_time - wep->charge_begin_time());

          // If the charge time is less than our wanted max charge then force the charge to be at that exact time.
          if(wep->charge_begin_time() <= 0.f || wep->charge_begin_time() > 0.f && current_charge < max_charge){
            old_charge_time         = wep->charge_begin_time();
            charge_time_needs_reset = true;
            wep->charge_begin_time()  = globaldata->cur_time - max_charge;
          }
        }
      }
    }

    static s_pred_track_array track;
    memset(&track, 0, sizeof(s_pred_track_array));

    vec3 angles = global->engine->get_view_angles();
    if(freecam->is_active())
      angles = freecam->freeze_angle;

    // We also disable the random velocity / direction changes because customers will be unable to understand why valve decided to make the velocity and direction random...
    s_projectile_simulation_context ctx(wep, angles, true, !config->visual.projectile_trajectory_show_random_impulse, !config->visual.projectile_trajectory_show_random_velocity);
    for(u32 i = 0; i < math::time_to_ticks(60.f); i++){
      if(!predict_tick(&ctx))
        break;

      if(!track.add(ctx.predicted_origin))
        break;
    }

    if(!ctx.setup){
      render_cam->reset_position();
      global->proj_track_array.size    = 0;
      global->proj_track_radius_points = 0;
      return;
    }

    // Copy over the track array now.
    memcpy(&global->proj_track_array, &track, sizeof(s_pred_track_array));

    float blast_radius = wep->get_blast_radius();
    if(blast_radius > 0.0f && config->visual.projectile_trajectory_constant_splash_circle){
      u32 points = 512;
      utils::get_radius_points(ctx.predicted_origin, blast_radius, global->proj_track_radius, &points, ctx.tr.fraction < 1.f ? ctx.tr.plane.normal : vec3(0.f, 1.f, 0.f));
      global->proj_track_radius_points = points;
    }
    else
      global->proj_track_radius_points = 0;
    
    if(charge_time_needs_reset)
      wep->charge_begin_time() = old_charge_time;

    // Calculate best camera position and angle.
    {
      c_base_player* owner = wep->get_owner();
      if(owner == nullptr)
        return;

      // Set the current impact position so we can visualize it with in the camera.
      render_cam->set_impact_position(ctx.predicted_origin);

      vec3 angle_to_owner = math::calc_view_angle(ctx.predicted_origin, owner->obb_center());
      vec3 fwd_to_owner;
      math::angle_2_vector(angle_to_owner, &fwd_to_owner, nullptr, nullptr);

      // We wanna be above looking down at it at an angle.
      s_trace tr = global->trace->ray(ctx.predicted_origin, fwd_to_owner * 64.f + ctx.predicted_origin + vec3(0.f, 0.f, 64.f), mask_bullet);
      render_cam->set_camera_position(tr.end);

      vec3 ang = math::calc_view_angle(tr.end, ctx.predicted_origin);
      render_cam->set_camera_angles(ang);
    }

    global->proj_track_angles  = math::clamp_angles(ctx.proj_angle);
    global->proj_track_obb_min = ctx.proj_min;
    global->proj_track_obb_max = ctx.proj_max;
  }

  void draw_projectile_trajectory_lines(){
    if(!should_draw_trajectory_visualizer())
      return;
    
    if(global->proj_track_array.size <= 0 || global->proj_track_array.size >= MAX_PRED_TRACK_ENTRIES)
      return;

    if(!config->visual.projectile_trajectory_visualizer || !config->visual.projectile_trajectory_constant_line)
      return;

    vec3 last_origin;
    for(i32 i = 0; i < global->proj_track_array.size; i++){

      if(i)
        render_debug->draw_static_line(last_origin, global->proj_track_array.entries[i].pos, flt_array2clr(config->visual.projectile_trajectory_constant_line_colour), config->visual.projectile_trajectory_constant_outline);

      last_origin = global->proj_track_array.entries[i].pos;
    }

    if(global->proj_track_radius_points > 0 && config->visual.projectile_trajectory_constant_splash_circle){
        
      vec3 last_radius_origin;
      for(i32 i = 0; i < global->proj_track_radius_points; i++){

        if(i){
          render_debug->draw_static_line(last_radius_origin, global->proj_track_radius[i], flt_array2clr(config->visual.projectile_trajectory_constant_line_colour), config->visual.projectile_trajectory_constant_outline);
        }

        last_radius_origin = global->proj_track_radius[i];
      }
    }

    if(config->visual.projectile_trajectory_constant_box)
      render_debug->draw_static_3dbox(last_origin, global->proj_track_obb_min, global->proj_track_obb_max, global->proj_track_angles, flt_array2clr(config->visual.projectile_trajectory_constant_line_colour), rgba(0, 0, 0, 255));
  }

  void draw_flight_lines(c_base_weapon* wep){
    if(wep == nullptr)
      return;

    if(!should_draw_trajectory_visualizer())
      return;

    if(wep->is_throwables()){
      if(global->animation_fire_time <= globaldata->cur_time){
        if(!(math::abs(global->animation_fire_time - globaldata->cur_time) > globaldata->interval_per_tick && math::abs(global->animation_fire_time - globaldata->cur_time) <= globaldata->interval_per_tick * 2.f))
          return;
      }
    }

    if(!config->visual.projectile_trajectory_visualizer)
      return;

    if(!config->visual.projectile_trajectory_dynamic_line)
      return;

    static i32 last_drawn_tick = 0;
    if(last_drawn_tick == globaldata->tick_count)
      return;

    last_drawn_tick = globaldata->tick_count;

    float latency = utils::get_latency();
    s_projectile_simulation_context ctx(wep, global->current_cmd->view_angles, true, !config->visual.projectile_trajectory_show_random_impulse, !config->visual.projectile_trajectory_show_random_velocity);  
    vec3 prev_origin;
    for(u32 i = 0; i <= math::time_to_ticks(5.f); i++){
      if(i)
        render_debug->draw_line(prev_origin, ctx.predicted_origin, flt_array2clr(config->visual.projectile_trajectory_dynamic_line_colour), config->visual.projectile_trajectory_dynamic_outline, latency + math::ticks_to_time(i + 1));

      prev_origin = ctx.predicted_origin;
      if(!predict_tick(&ctx))
        break;
    }

    float blast_radius = wep->get_blast_radius();
    if(config->visual.projectile_trajectory_dynamic_splash_circle && blast_radius > 0.0f && ctx.setup){
      u32 size = 512;
      vec3 points[512];
      utils::get_radius_points(ctx.predicted_origin, blast_radius, points, &size, ctx.tr.fraction < 1.f ? ctx.tr.plane.normal : vec3(0.f, 1.f, 0.f));
      if(size > 0){
        for(u32 i = 0; i < size; i++){
          if(i)
            render_debug->draw_line(prev_origin, points[i], flt_array2clr(config->visual.projectile_trajectory_dynamic_line_colour), config->visual.projectile_trajectory_dynamic_outline, math::ticks_to_time(ctx.predicted_ticks));

          prev_origin = points[i];
        }
      }
    }

    if(config->visual.projectile_trajectory_dynamic_box)
      render_debug->draw_3dbox(ctx.predicted_origin, ctx.proj_min, ctx.proj_max, ctx.proj_angle, flt_array2clr(config->visual.projectile_trajectory_dynamic_line_colour), rgba(0, 0, 0, 255), latency + math::ticks_to_time(ctx.predicted_ticks));
  }

  bool get_angle_impulse(c_base_weapon* weapon, vec3* impulse);
  bool get_drag_basis(c_base_weapon* weapon, vec3* drag_basis, vec3* drag_ang_basis);
  i32  get_trace_filter(c_base_weapon* weapon);
  bool is_vphysics_weapon(c_base_weapon* weapon);

  bool is_vphysics_setup(){
    return env != nullptr && object != nullptr;
  }
private:
  c_physics_env*    env;
  c_physics_object* object;
};

CLASS_EXTERN(c_projectile_simulation, proj_simulate);