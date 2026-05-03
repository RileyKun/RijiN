#pragma once

#define EPSILON 0.03125f
#define COMPRESSION_GARBAGE_VALUE 1.507446
#define YAW_DELTA_NOT_ENOUGH 0.3f
#define PRED_CHANGE_VEL_LEN_LOW 0.1f
#define PRED_CHANGE_YAW_DELTA_TOO_HIGH 20.f

enum e_pre_prediction_flags{
  PRE_PREDICTION_APPLY_FRICTION = 1 << 1,
  PRE_PREDICITON_ZERO_VEL = 1 << 2
};

enum e_entity_predict_flags{
  EP_FLAG_PREDICT_YAW_TURN = 1 << 0, // Avoids tangents by predicting movement based on turning estimations
  EP_FLAG_PREDICT_FRICTION = 1 << 1, // Applys friction.
};

enum e_velocity_flags{
  EV_FLAG_NONE = 0,
  EV_FLAG_SCALE_BY_TENS = 1 << 0, // Designed to reduce noise by scaling the player's velocity if it's with their max speed to 10% values. so like 5% -> 0%, 13% -> 10%
};

struct s_entity_predict_ctx{
  bool            cached;
  u32             flags;
  c_base_player*  predicted_entity   = nullptr;  // entity to predict
  bool            has_been_predicted = false;    // has the entity been predicted
  bool            prediction_broken  = false;    // has the prediction been broken?
  u32             predicted_ticks    = 0;        // how many ticks has the entity been predicted
  vec3            predicted_origin;              // the resulting predicted origin
  vec3            predicted_velocity;            // the resulting predicted velocity
  vec3            predicted_obb_mins;            // the resulting predicted object bounds (mins)
  vec3            predicted_obb_maxs;            // the resulting predicted object bounds (maxs)
  u8              predicted_flags;               // predicted entity flags, such as FL_ONGROUND
  float           predicted_surface_friction;    // predicted surface friction
  float           predicted_wish_speed;          // our wish speed (used by air/ground acceleration)
  float           predicted_wish_yaw_turn;       // EP_FLAG_PREDICT_YAW_TURN: the predicted yaw turn per tick
  float           predicted_wish_yaw_turn_delta; // EP_FLAG_PREDICT_YAW_TURN: the predicted yaw turn delta per tick

  s_entity_predict_ctx(){}
  s_entity_predict_ctx(c_base_entity* entity_to_predict, u32 new_flags = EP_FLAG_PREDICT_YAW_TURN);
};

struct s_entity_predict_cache_ctx{
  vec3                 correct_origin;
  float                real_time;
  s_entity_predict_ctx ctx;
};


class c_entity_prediction{
public:
  s_entity_predict_cache_ctx cached_pred_data[MAXPLAYERS];

  bool  calc_frame_velocity(c_base_player* player, vec3* vel_out, u32 tick_offset = 0, u32 flags = EV_FLAG_SCALE_BY_TENS);
  bool  draw_prediction_track();
  float compute_yaw_delta(vec3 cur_vel, vec3 last_vel){
    const float yaw1          = math::normalize_angle(math::rad_2_deg(math::atan2(cur_vel.y, cur_vel.x)));
    const float yaw2          = math::normalize_angle(math::rad_2_deg(math::atan2(last_vel.y, last_vel.x)));
    return math::normalize_angle(yaw1 - yaw2);
  }

  void  clean_yaw_deltas(float& raw_yaw_delta, float& yaw_delta, float& last_raw_yaw_delta);
  void  compute_pred_movement_change(c_base_player* player);
  void  compute_pred_yaw_change(c_base_player* player);
  void  clear_pred_yaw_change(c_base_player* player);
  float calculate_hit_chance(c_base_player* player);
  void  apply_gravity(s_entity_predict_ctx* ctx);

  void handle_parachute_gravity(s_entity_predict_ctx* ctx);
  void apply_friction(s_entity_predict_ctx* ctx);
  void update_on_ground(s_entity_predict_ctx* ctx);

  void fix_velocity(s_entity_predict_ctx* ctx){
    if(ctx == nullptr)
      return;

    //for(u32 i = 0; i < 3; i++)
    //  ctx->predicted_velocity[i] = math::clamp(ctx->predicted_velocity[i], -global->sv_maxvelocity->flt, global->sv_maxvelocity->flt);
  }

  void clip_ground_velocity(s_entity_predict_ctx* ctx);
  bool resolve_stuck(s_entity_predict_ctx* ctx);
  void physical_move(s_entity_predict_ctx* ctx);
  void air_accelerate(s_entity_predict_ctx* ctx, vec3 angle, float fmove, float smove);
  bool predict_tick(s_entity_predict_ctx* ctx);
  bool simple(s_entity_predict_ctx* ctx, u32 ticks_to_predict);

  s_entity_predict_cache_ctx* get_cache(c_base_entity* entity);
  bool is_cached(c_base_entity* entity);
  bool store_cache(s_entity_predict_ctx* ctx, c_base_entity* entity, float real_time, vec3 correct_origin);
  void delete_cache(c_base_entity* entity);
};

CLASS_EXTERN(c_entity_prediction, entity_prediction);