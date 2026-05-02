#pragma once

struct s_box_data{
  bool  valid = false;
  vec3i pos;
  vec3i size;
};

namespace math{

  static bool world2screen(const vec3 world, vec3i& screen){
    if(global_internal->view_setup == nullptr)
      return false;

    const vec3i screen_size = render->screen_size;

    float w = global_internal->view_matrix[3][0] * world.x + global_internal->view_matrix[3][1] * world.y + global_internal->view_matrix[3][2] * world.z + global_internal->view_matrix[3][3];
    screen = vec3i(
      global_internal->ws2_position_offset.x + (screen_size.x / 2) + (0.5f * ((global_internal->view_matrix[0][0] * world.x + global_internal->view_matrix[0][1] * world.y + global_internal->view_matrix[0][2] * world.z + global_internal->view_matrix[0][3]) * (1.f / w)) * screen_size.x + 0.5f),
      global_internal->ws2_position_offset.y + (screen_size.y / 2) - (0.5f * ((global_internal->view_matrix[1][0] * world.x + global_internal->view_matrix[1][1] * world.y + global_internal->view_matrix[1][2] * world.z + global_internal->view_matrix[1][3]) * (1.f / w)) * screen_size.y + 0.5f)
    );

    return w > 0.01f;
  }

  static s_box_data calculate_box_region(vec3 min, vec3 max, matrix3x4 tran_frame){
    s_box_data data{};

    vec3i screen_boxes[8];
    vec3 points[] ={
      vec3(min.x, min.y, min.z),
      vec3(min.x, max.y, min.z),
      vec3(max.x, max.y, min.z),
      vec3(max.x, min.y, min.z),
      vec3(max.x, max.y, max.z),
      vec3(min.x, max.y, max.z),
      vec3(min.x, min.y, max.z),
      vec3(max.x, min.y, max.z)
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

  inline i32 time_to_ticks(float time){
    return (i32)(0.5f + (float)time / global_internal->global_data->interval_per_tick);
  }

  inline float ticks_to_time(i32 ticks){
    return global_internal->global_data->interval_per_tick * ticks;
  }

  inline vec3 bbox_pos(s_studio_bbox* box, matrix3x4* matrix, float model_scale){
    if(box == nullptr || matrix == nullptr)
      return vec3();
    
    assert(model_scale > 0.0f);
    vec3 mins = box->mins * model_scale;
    vec3 maxs = box->maxs * model_scale;

    return math::transform((mins + maxs) * 0.5f, matrix[box->bone]);
  }

  inline void vector_2_angle(vec3* fwd, vec3* ang){
    if(fwd == nullptr || ang == nullptr)
      return;
    
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
    if(fwd == nullptr && right == nullptr && up == nullptr)
      return;

    float sp, sy, sr, cp, cy, cr;

    ang.normalize_angle();
    sin_cos(deg_2_rad(ang.x), &sp, &cp);
    sin_cos(deg_2_rad(ang.y), &sy, &cy);
    sin_cos(deg_2_rad(ang.z), &sr, &cr);

    if(fwd != nullptr){
      fwd->x = cp * cy;
      fwd->y = cp * sy;
      fwd->z = -sp;
    }

    if(right != nullptr){
      right->x = -1 * sr * sp * cy + -1 * cr * -sy;
      right->y = -1 * sr * sp * sy + -1 * cr * cy;
      right->z = -1 * sr * cp;
    }

    if(up != nullptr){
      up->x = cr * sp * cy + -sr * -sy;
      up->y = cr * sp * sy + -sr * cy;
      up->z = cr * cp;
    }
  }

  static vec3 calc_view_angle(vec3 v1, vec3 v2){
    vec3 ret;
    vec3 angle_delta = (v2 - v1);
    angle_delta.normalize();
    vector_2_angle(&angle_delta, &ret);

    ret.normalize_angle();

    return ret;
  }

  static float get_fov_delta(vec3 view_angles, vec3 start, vec3 end){
    vec3 fwd;
    angle_2_vector(view_angles, &fwd, nullptr, nullptr);

    vec3 dir = (end - start);
    dir.normalize();

    return math::abs(biggest(normalize_angle(rad_2_deg(acos(fwd.dot(dir)))), 0.f));
  }

  /*
    WARNING: Should be obvious, to only be used for YOU not other players or entities.
    Decided to just use the pixel FOV and utilize a normalized value times by 180 where people can easily understand.
    This means, when a player sees something in view, it'll be in the range of 0.0 - 1.0 and any higher is where it will exceed values above 180.
    While you may assume this will be an issue, I can ensure it won't since it'll help with shooting the closest to 'crosshair'.

    This will also address all of the issues we've with the pixel FOV checks in aimbot_filters.cpp (which have been removed in TF2)
    It's not exactly possible to convert a "pixel FOV" to an angle based FOV.

    The reasons why we also can't fall back to angle based FOV is we could go from 179 pixel FOV-
     and then go down to something like 80 angle FOV and result in cases where we shoot someone
     we are not looking at.
  */
  static float get_pixel_fov_delta(const vec3 end){
    const float max_fov       = 180.f; // Just the value to give a normalized value a related output.
    if(global_internal->game_fov <= 0.f || render->screen_size == vec3i()){
      DBG("[!] global_internal->game_fov(%f is it > 0 ?) or render->screen_size is zero\n", global_internal->game_fov);
      return 10000.f;
    }

    const vec3i screen_center = render->screen_size / 2;

    vec3i screen_pos;
    if(!math::world2screen(end, screen_pos))
      return math::sqrt(screen_center.x * screen_center.x + screen_center.y * screen_center.y);

    const vec3i delta                     = screen_pos - screen_center;
    const float max_radius                = (render->screen_size.x * 0.5f) * (max_fov / global_internal->game_fov);
    const float pixel_distance            = math::sqrt(delta.x * delta.x + delta.y * delta.y);
    const float normalized_pixel_distance = pixel_distance / max_radius;

    // Is NaN
    if(normalized_pixel_distance != normalized_pixel_distance)
      return math::sqrt(screen_center.x * screen_center.x + screen_center.y * screen_center.y);


    return normalized_pixel_distance * max_fov;
  }

  inline bool rebase_movement(c_user_cmd* cmd, c_user_cmd original_cmd, u8 move_type = 0){
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

    // Added: check if on a ladder or noclipping and return true (dec 11th 2024)
    if(move_type == MOVETYPE_LADDER || move_type == MOVETYPE_NOCLIP){
      if(cmd->view_angles.x >= 45.f && original_cmd.view_angles.x < 45.f && math::abs(delta) <= 65.f)
        move_angle.x *= -1.f;

      cmd->move.x = clamp(move_angle.x, -450.f, 450.f);
      cmd->move.y = clamp(move_angle.y, -450.f, 450.f);
 
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

      cmd->move.x = clamp(move_angle.x, -450.f, 450.f);
      cmd->move.y = clamp(move_angle.y, -450.f, 450.f);

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

  inline float remap_val_clamped(float val, float A, float B, float C, float D){
    if(A == B)
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
    if(A == B)
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

  inline vec3 vec_point_from_angle_len(vec3 pos, vec3 angles, float len){
    vec3 fwd;
    math::angle_2_vector(angles, &fwd, nullptr, nullptr);
    return (fwd * len) + pos;
  }

  inline void convert_position_to_ivp(vec3 in, vec3& out){
    const float scale_meter = 0.025400f;
    out.x = (in.x * scale_meter);
    out.y = -(in.z * scale_meter);
    out.z = (in.y * scale_meter);
  }

  inline void convert_angular_impulse_to_ivp(vec3 in, vec3& out){
    out.x = math::deg_2_rad(in.x);
    out.y = -math::deg_2_rad(in.z);
    out.z = math::deg_2_rad(in.y);
  }

  inline void convert_dir_to_ivp(vec3 in, vec3& out){
    out.x = in.x;
    out.y = -in.z;
    out.z = in.y;
  }

  inline void convert_dir_to_hl(vec3 in, vec3& out){
    out.z = -in.y;
    out.y = in.z;
    out.x = in.x;
  }

  inline void convert_position_to_hl(vec3 in, vec3& out){
    out.z = -in.y * 39.37007874015748f;
    out.y = in.z * 39.37007874015748f;
    out.x = in.x * 39.37007874015748f;
  }

  inline void convert_angular_impulse_to_hl(vec3 in, vec3& out){
    out.x = math::rad_2_deg(in.x);
    out.y = math::rad_2_deg(in.z);
    out.z = -math::rad_2_deg(in.y);
  }

  inline u32 md5_pseudo_random(u32 seed){
    #if defined(__x86_64__)
      return utils::call_fastcall64_raw<u32, u32>(global_internal->md5_pseudorandom, seed);
    #else
      return utils::call_cdecl<u32, u32>(global_internal->md5_pseudorandom, seed);
    #endif
  }

  void  set_random_seed(u32 seed);
  i32   random_int(i32 min, i32 max);
  float random_float(float min, float max);
  void  setup_random_seed(i32 num = 6, i32 seed = -1, bool bad_call = true);
  i32   shared_random_int(i8* str, i32 min, i32 max, i32 exp);
};