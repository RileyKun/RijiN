#pragma once

class c_physics_env;
class c_physics_collision;
class c_physics_object;

// Called 'objectparams_t' located in vphysics_interface.h
struct s_object_params{
  vec3*   mass_center_override;
  float   mass;
  float   inertia;
  float   damping;
  float   rot_damping;
  float   rot_inertia_limit;
  i8*     name;
  void*   game_data;
  float   volume;
  float   drag_coefficient;
  bool    enable_collisions;

  // Copying how the game setups it.
  // Example located in physics_shared.cpp (Look for: g_PhysDefaultObjectParams)
  ALWAYSINLINE void init(){
    mass_center_override = nullptr;
    mass                 = 1.f;
    inertia              = 1.f;
    damping              = 0.1f;
    rot_damping          = 0.1f;
    rot_inertia_limit    = 0.05f;
    game_data            = nullptr;
    volume               = 0.f;
    drag_coefficient     = 1.f;
    enable_collisions    = true; // Say, if actually does what it says and we can actually easily turn all traces it'll perform that'll be great.

    // I don't want any beast crashing cause the string has a stack pointer in its place.
    static i8 name_hack[32];
    wsprintfA(name_hack, XOR("DEFAULT"));
    name      = name_hack;
  }

  // Setup for actual use in RIJIN
  ALWAYSINLINE void setup(){
    init();
    damping           = 0.f;
    inertia           = 0.f;
    rot_damping       = 0.f;
    rot_inertia_limit = 0.f;
    enable_collisions = false;
  }
};

// Struct inside 'vphysics.h' called physics_performanceparams_t
struct s_physics_performance{
  i32 max_collisions_per_obj_timestep;
  i32 max_collisions_checks_per_timestep;
  float max_velocity;
  float max_ang_velocity;
  float look_ahead_time_obj_vs_world;
  float look_ahead_time_obj_vs_obj;
  float min_friction_mass;
  float max_friction_mass;  

  // NOTE: If this only has to be called once, then XOR32 the numbers to make this harder to reverse.
  // As stated above the struct, you can find all of the information related to this by searching the TF2 source code.
  ALWAYSINLINE void init(){
    max_collisions_per_obj_timestep    = 6;
    max_collisions_checks_per_timestep = 250;
    max_velocity                       = 2000.f;
    max_ang_velocity                   = 3600.f;
    look_ahead_time_obj_vs_world       = 1.f;
    look_ahead_time_obj_vs_obj         = 0.5f;
    min_friction_mass                  = 10.f;
    max_friction_mass                  = 2500.f;
  }
};

class c_physics_interface{
public:
  ALWAYSINLINE c_physics_env* create_env(){
    return utils::internal_virtual_call<c_physics_env*>(base_interface->create_env_index, this);
  }

  ALWAYSINLINE void destroy_env(c_physics_env* env){
    assert(env != nullptr);
    utils::internal_virtual_call<void, c_physics_env*>(base_interface->destroy_env_index, this, env);
  }
};

class c_physics_collision_interface{
public:
  void* bbox_to_collide(vec3& mins, vec3& maxs){
    return utils::internal_virtual_call<void*, vec3&, vec3&>(base_interface->bbox_to_collide_index, this, mins, maxs);
  }
};

class c_physics_object{
public:
 
  // Might need it idk.
  ALWAYSINLINE bool is_collision_enabled(){
    return utils::internal_virtual_call<bool>(base_interface->is_collision_enabled_index, this);
  }

  // 12
  ALWAYSINLINE void enable_collisions(bool enable){
    utils::internal_virtual_call<void, bool>(base_interface->enable_collisions_index, this, enable);
  }

  ALWAYSINLINE void enable_gravity(bool enable){
    utils::internal_virtual_call<void, bool>(base_interface->enable_gravity_index, this, enable);
  }

  ALWAYSINLINE void enable_drag(bool enable){
    utils::internal_virtual_call<void, bool>(base_interface->enable_drag_index, this, enable);
  }

  ALWAYSINLINE void wake(){
    utils::internal_virtual_call<void>(base_interface->wake_index, this);
  }

  ALWAYSINLINE void sleep(){
    utils::internal_virtual_call<void>(base_interface->sleep_index, this);
  }

  // NOTE: If for some reason TF2 has different drag coefficients for the angle then add another parameter and replace the second paramater of the virtual call that one!!
  ALWAYSINLINE void set_drag_coefficient(float* drag){
    utils::internal_virtual_call<void, float*, float*>(base_interface->set_drag_coefficient_index, this, drag, drag);
  }

  ALWAYSINLINE void set_position(vec3& pos, vec3& angle){
    // The last parameter is called 'isTeleport'
    utils::internal_virtual_call<void, vec3&, vec3&, bool>(base_interface->vphy_set_position_index, this, pos, angle, true);
  }

  ALWAYSINLINE void set_velocity(vec3* vel, vec3* ang_vel){
    utils::internal_virtual_call<void, vec3*, vec3*>(base_interface->vphy_set_velocity_index, this, vel, ang_vel);
  }

  ALWAYSINLINE void get_position(vec3* pos, vec3* angles){
    utils::internal_virtual_call<void, vec3*, vec3*>(base_interface->vphy_get_position_index, this, pos, angles);
  }

  ALWAYSINLINE void get_velocity(vec3* vel, vec3* ang_vel){
    utils::internal_virtual_call<void, vec3*, vec3*>(base_interface->vphy_get_velocity_index, this, vel, ang_vel);
  }

  void* pad;
  void* game_data;
  void* object;
  void* collide;
  void* shadow;
  vec3 drag_basis;
  vec3 drag_ang_basis;
};

class c_physics_env{
public:
  
  ALWAYSINLINE void set_gravity(float gravity){
    vec3 v = vec3(0.f, 0.f, gravity);
    utils::internal_virtual_call<void, vec3&>(base_interface->vphy_set_gravity_index, this, v);
  }

  ALWAYSINLINE void set_air_density(float den){
    utils::internal_virtual_call<void, float>(base_interface->set_air_density_index, this, den);
  }

  ALWAYSINLINE c_physics_object* create_physics_object(void* phy_col, vec3 pos, vec3 angle, s_object_params* params){
    return utils::internal_virtual_call<c_physics_object*, void*, i32, vec3&, vec3&, s_object_params*>(base_interface->create_physics_object_index, this, phy_col, 0, pos, angle, params);
  }

  ALWAYSINLINE void set_performance_settings(s_physics_performance* settings){
    assert(settings != nullptr);
    utils::internal_virtual_call<void, s_physics_performance*>(base_interface->set_performance_settings_index, this, settings);
  }

  ALWAYSINLINE void simulate(float delta_time){
    utils::internal_virtual_call<void, float>(base_interface->vphy_simulate_index, this, delta_time);
  }
};