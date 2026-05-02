#pragma once

struct s_box_data{
  bool valid = false;
  vec3i pos;
  vec3i size;
};

namespace math{
  inline bool world2screen(vec3 world, vec3i& screen){
    if(global->view_setup == nullptr)
      return false;

    float w = global->view_matrix[ 3 ][ 0 ] * world.x + global->view_matrix[ 3 ][ 1 ] * world.y + global->view_matrix[ 3 ][ 2 ] * world.z + global->view_matrix[ 3 ][ 3 ];

    screen = vec3i(
      (render->screen_size.x / 2) + ( 0.5f * ( ( global->view_matrix[ 0 ][ 0 ] * world.x + global->view_matrix[ 0 ][ 1 ] * world.y + global->view_matrix[ 0 ][ 2 ] * world.z + global->view_matrix[ 0 ][ 3 ] ) * ( 1.f / w ) ) * render->screen_size.x + 0.5f ),
      (render->screen_size.y / 2) - ( 0.5f * ( ( global->view_matrix[ 1 ][ 0 ] * world.x + global->view_matrix[ 1 ][ 1 ] * world.y + global->view_matrix[ 1 ][ 2 ] * world.z + global->view_matrix[ 1 ][ 3 ] ) * ( 1.f / w ) ) * render->screen_size.y + 0.5f )
    );

    return w > 0.01f;
  }

  inline s_box_data calculate_box_region(vec3 min, vec3 max, matrix3x4 tran_frame){
    s_box_data data{};

    vec3i screen_boxes[8];
    vec3 points[] ={
      vec3( min.x, min.y, min.z ),
      vec3( min.x, max.y, min.z ),
      vec3( max.x, max.y, min.z ),
      vec3( max.x, min.y, min.z ),
      vec3( max.x, max.y, max.z ),
      vec3( min.x, max.y, max.z ),
      vec3( min.x, min.y, max.z ),
      vec3( max.x, min.y, max.z )
    };

    for(u8 i = 0; i <= 7; i++)
      if(!world2screen(math::transform(points[i], tran_frame), screen_boxes[i]))
        return data;

    vec3i box_array[ ] = {
      screen_boxes[ 3 ], // Front left
      screen_boxes[ 5 ], // Back right
      screen_boxes[ 0 ], // Back left
      screen_boxes[ 4 ], // Front right
      screen_boxes[ 2 ], // Front right
      screen_boxes[ 1 ], // Back right
      screen_boxes[ 6 ], // Back left
      screen_boxes[ 7 ]  // Front Left
    };

    i32   left    = screen_boxes[ 3 ].x,
          bottom  = screen_boxes[ 3 ].y,
          right   = screen_boxes[ 3 ].x,
          top     = screen_boxes[ 3 ].y;

    for (u8 i = 0; i <= 7; i++){
      if (left > box_array[i].x)
        left = box_array[i].x;

      if (bottom < box_array[i].y)
        bottom = box_array[i].y;

      if (right < box_array[i].x)
        right = box_array[i].x;

      if (top > box_array[i].y)
        top = box_array[i].y;
    }

    data.valid  = true;
    data.pos    = vec3i(left, top);
    data.size   = vec3i(right - left, bottom - top);

    return data;
  }


  inline float get_lerp(){
    if( global->cl_interp_ratio == nullptr )
      return 0.f;

    float min_ratio = math::biggest(1.f, global->sv_client_min_interp_ratio->flt());
    float ratio     = math::clamp(global->cl_interp_ratio->flt(), min_ratio, global->sv_client_max_interp_ratio->flt());

    // At this point if this is above 200ms or cl_interp is above 200ms we can't adjust for lerp anymore. Clamp it and hope for the best!
    float lerp = math::smallest(ratio / global->cl_updaterate->flt(), 1.0f);
    float interp = math::smallest(global->cl_interp->flt(), 1.0f);

    return interp > lerp ? interp : lerp;
  }


  inline i32 time_to_ticks(float time){
    return (i32)(0.5f + (float)time / globaldata->interval_per_tick);
  }

  inline float ticks_to_time(i32 ticks){
    return globaldata->interval_per_tick * ticks;
  }


  inline vec3 bbox_pos(s_studio_bbox* box, matrix3x4* matrix, float model_scale){
    if(box == nullptr){
      assert(false);
      return vec3();
    }

    assert(model_scale > 0.0f);
    vec3 mins = box->mins * model_scale;
    vec3 maxs = box->maxs * model_scale;
    return math::transform((mins + maxs) * 0.5f, matrix[box->bone]);
  }

  inline void vector_2_angle(vec3* fwd, vec3* ang){
    if(fwd == nullptr){
      assert(fwd != nullptr);
      return;
    }

    if(ang == nullptr){
      assert(ang != nullptr);
      return;
    }

    if(fwd->x == 0.f && fwd->y == 0.f){
      ang->x = fwd->z > 0.f ? -90.f : 90.f;
      ang->y = 0.f;
    }
    else{
      ang->x = math::rad_2_deg(math::atan2(-fwd->z, fwd->length_2d()));
      ang->y = math::rad_2_deg(math::atan2(fwd->y, fwd->x));
    }
  }

  inline void angle_2_vector(vec3 ang, vec3* fwd, vec3* right, vec3* up){
    float sp, sy, sr, cp, cy, cr;

    ang.normalize_angle();
    sin_cos(deg_2_rad(ang.x), &sp, &cp);
    sin_cos(deg_2_rad(ang.y), &sy, &cy);
    sin_cos(deg_2_rad(ang.z), &sr, &cr);

    if ( fwd != nullptr ) {
      fwd->x = cp * cy;
      fwd->y = cp * sy;
      fwd->z = -sp;
    }

    if ( right != nullptr ) {
      right->x = -1 * sr * sp * cy + -1 * cr * -sy;
      right->y = -1 * sr * sp * sy + -1 * cr * cy;
      right->z = -1 * sr * cp;
    }

    if ( up != nullptr ) {
      up->x = cr * sp * cy + -sr * -sy;
      up->y = cr * sp * sy + -sr * cy;
      up->z = cr * cp;
    }
  }

  inline vec3 calc_view_angle(vec3 v1, vec3 v2){
    vec3 ret;

    vec3 angle_delta = (v2 - v1);
    angle_delta.normalize();
    vector_2_angle(&angle_delta, &ret);

    ret.normalize_angle();

    return ret;
  }

  inline float get_fov_delta(vec3 view_angles, vec3 start, vec3 end){
    vec3 fwd;
    angle_2_vector(view_angles, &fwd, nullptr, nullptr);

    vec3 dir = (end - start);
    dir.normalize();

    return math::abs(biggest(normalize_angle(rad_2_deg(acos(fwd.dot(dir)))), 0.f));
  }

  inline bool rebase_movement(s_user_cmd* cmd, s_user_cmd original_cmd){
    if(cmd == nullptr){
      assert(cmd != nullptr);
      return false;
    }

    if(cmd->view_angles == original_cmd.view_angles)
      return false;

    vec3 move = vec3(cmd->move.x, cmd->move.y, 0.f);

    float move_len = move.normalize();
    if(move_len == 0.f)
      return false;

    // Convert the move direction to an angle
    vec3 move_angle;
    vector_2_angle(&move, &move_angle);

    // Rebase the move direction to compensate for our changed view angle
    float delta = (cmd->view_angles.y - original_cmd.view_angles.y);
    move_angle.y += delta;

    // Convert the move angle back to a vector direction
    angle_2_vector(move_angle, &move_angle, nullptr, nullptr);
    move_angle *= move_len;

    // TODO: check if on a ladder or noclipping and return true
    if(false){
      if(cmd->view_angles.x >= 45.f && original_cmd.view_angles.x < 45.f && math::abs(delta) <= 65.f)
        move_angle.x *= -1.f;

      cmd->move.x = clamp(move_angle.x, -10000.f, 10000.f);
      cmd->move.y = clamp(move_angle.y, -10000.f, 10000.f);
      //cmd->move.z = 0.f;

      // Recalculate buttons
      {
        cmd->buttons &= ~(IN_FORWARD|IN_BACK|IN_MOVELEFT|IN_MOVERIGHT);

        if(cmd->move.x > 200.f)
          cmd->buttons |= IN_FORWARD;
        else if(cmd->move.x < -200.f)
          cmd->buttons |= IN_BACK;

        if(cmd->move.y > 200.f)
          cmd->buttons |= IN_MOVERIGHT;
        else if(cmd->move.y < -200.f)
          cmd->buttons |= IN_MOVELEFT;
      }
    }
    else{

      float pitch = math::abs(cmd->view_angles.x);
      if(pitch > 90.f && pitch != 271.f && pitch <= 360.f)
        move_angle.x *= -1.f;

      cmd->move.x = clamp(move_angle.x, -10000.f, 10000.f);
      cmd->move.y = clamp(move_angle.y, -10000.f, 10000.f);
      //cmd->move.z = 0.f;

      // Recalculate buttons
      {
        cmd->buttons &= ~(IN_FORWARD|IN_BACK|IN_MOVELEFT|IN_MOVERIGHT);

        if(cmd->move.x > 0.f)
          cmd->buttons |= IN_FORWARD;
        else if(cmd->move.x < 0.f)
          cmd->buttons |= IN_BACK;

        if(cmd->move.y > 0.f)
          cmd->buttons |= IN_MOVERIGHT;
        else if(cmd->move.y < 0.f)
          cmd->buttons |= IN_MOVELEFT;
      }
    }

    return true;
  }

/*
  inline u32 md5_pseudo_random(u32 seed){
    assert(global->md5_pseudorandom != nullptr);
    return utils::call_cdecl<u32, u32>(global->md5_pseudorandom, seed);
  }
*/

  inline u32 md5_pseudo_random(u32 seed){
    assert(global->md5_pseudorandom != nullptr);
    return utils::call_fastcall64_raw<u32, u32>(global->md5_pseudorandom, seed);
  }

  inline void set_random_seed(u32 seed){
    static void* random_seed_ptr = utils::get_proc_address(utils::get_module_handle(HASH("vstdlib.dll")), HASH("RandomSeed"));
    utils::call_fastcall64_raw<void, u32>(random_seed_ptr, seed);
  }

  inline i32 random_int(i32 min, i32 max){
    static void* random_int_ptr = utils::get_proc_address(utils::get_module_handle(HASH("vstdlib.dll")), HASH("RandomInt"));
    return utils::call_fastcall64_raw<i32, i32, i32>(random_int_ptr, min, max);
  }

  inline float random_float(float min, float max){
    static void* random_float_ptr = utils::get_proc_address(utils::get_module_handle(HASH("vstdlib.dll")), HASH("RandomFloat"));
    return utils::call_fastcall64_raw<float, float, float>(random_float_ptr, min, max);
  }

  inline float remap_val_clamped(float val, float A, float B, float C, float D){
    if ( A == B )
      return val >= B ? D : C;

    float cval = (val - A) / (B - A);
    cval = math::clamp(cval, 0.0f, 1.0f);

    return C + (D - C) * cval;
  }

  inline float simple_spline(float value){
    float value_squared = value * value;
    return (3 * value_squared - 2 * value_squared * value);
  }

  inline float simple_spline_remap_val_clamped(float val, float A, float B, float C, float D){
    if (A == B)
      return val >= B ? D : C;

    float cval = (val - A) / (B - A);
    cval = math::clamp(cval, 0.0f, 1.0f);
    return C + (D - C) * simple_spline(cval);
  }

  inline void matrix_angles(matrix3x4& matrix, vec3& angles){
    vec3 forward;
    vec3 left;

    for(i32 i = 0; i <= 2; i++){
      forward[i] = matrix[i][0];
      left[i]    = matrix[i][1];
    }

    float up = matrix[2][2];

    float dist = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
    if(dist > 0.001f){
      angles.y = rad_2_deg(atan2(forward[1], forward[0]));
      angles.x = rad_2_deg(atan2(-forward[2], dist));
      angles.z = rad_2_deg(atan2(left[2], up));
    }
    else{
      angles.y = rad_2_deg(atan2(-left[0], left[1]));
      angles.x = rad_2_deg(atan2(-forward[2], dist));
      angles.z = 0.f;
    }
  }

  inline void matrix_angles(matrix3x4& matrix, vec3& angles, vec3& pos){
    pos.x = matrix[0][3];
    pos.y = matrix[1][3];
    pos.z = matrix[2][3];
    math::matrix_angles(matrix, angles);
  }

  inline bool lua_random_seed(float seed){
    ILuaInterface* lua = global->lua_shared->GetLuaInterface(LUAINTERFACE_CLIENT);
    if(lua == nullptr)
      return false;

    lua->PushSpecial(SPECIAL_GLOB);
    lua->GetField(-1, XOR("math"));
    lua->GetField(-1, XOR("randomseed"));
    lua->PushNumber(seed);
    lua->Call(1, 0);
    lua->Pop(2);

    return true;
  }

  inline float lua_random_float(float l, float h){
    ILuaInterface* lua = global->lua_shared->GetLuaInterface(LUAINTERFACE_CLIENT);
    if(lua == nullptr)
      return 0.f;

    lua->PushSpecial(SPECIAL_GLOB);
    lua->GetField(-1, XOR("math"));
    lua->GetField(-1, XOR("Rand"));
    lua->PushNumber(l);
    lua->PushNumber(h);
    lua->Call(2, 1);

    float ret = (float)lua->GetNumber(-1);

    lua->Pop(3);

    return ret;
  }

  inline bool get_hitbox_pos(matrix3x4* bone_matrix, c_base_entity* entity, i32 hitbox, vec3& output_pos){
    s_studio_bbox* bbox = entity->get_studio_bbox(hitbox);

    if(bbox == nullptr)
      return false;

    output_pos = math::bbox_pos(bbox, bone_matrix, entity->model_scale);
    return true;
  }

  inline bool get_hitbox_pos_from_hg(matrix3x4* bone_matrix, c_base_entity* entity, i32 hitgroup, vec3& output_pos){
    s_studio_bbox* bbox = entity->get_studio_bbox_from_hg(hitgroup);

    if(bbox == nullptr)
      return false;

    output_pos = math::bbox_pos(bbox, bone_matrix, entity->model_scale);
    return true;
  }
}