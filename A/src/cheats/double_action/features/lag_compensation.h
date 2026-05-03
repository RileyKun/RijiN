#pragma once

#define LC_MAX_RECORDS 66
#define LC_MAX_ENTRIES 128

struct s_lc_record{
  bool      filled;
  bool      broken;
  i32       index;

  vec3      origin;
  vec3      velocity;
  vec3      obb_center;
  vec3      obb_mins;
  vec3      obb_maxs;
  vec3      viewangles;
  float     simulation_time;
  i32       entity_flags;
  i32       bone_count;
  matrix3x4 bone_matrix[128];
  i32       server_tick;

  ALWAYSINLINE bool get_hitbox_pos(c_base_player* player, i32 hitbox, vec3& output_pos){
    s_studio_bbox* bbox = player->get_studio_bbox(hitbox);

    if(bbox == nullptr)
      return false;

    output_pos = math::bbox_pos(bbox, bone_matrix, player->model_scale);
    return true;
  }

  // Returns how far the record is behind
  ALWAYSINLINE float time_delta(){

    // !! READ !!

    // !! READ !!

    // I've added the extra parameter in get_latency so the cheat compiles. Port the code from TF2 when doing fake latency.
    float in  = global->engine->get_latency(false, false);
    float out = global->engine->get_latency(true, false);

    float target_fake_latency = 0.f; // fake_latency->get_target_latency();
    float correct             = out + in + math::ticks_to_time(math::time_to_ticks(math::get_lerp()));
          correct             = math::clamp(correct, 0.f, 1.f);

    //DBG("target_fake_latency: %f, correct: %f, in: %f, out: %f, total: %f\n", target_fake_latency, correct, in, out, (in + out) - target_fake_latency);
    i32 server_tick = (global->client_state->server_tick + 1) + math::biggest(math::time_to_ticks((in + out) - target_fake_latency), 0);

    return math::abs(correct - (math::ticks_to_time(server_tick) - simulation_time));
  }

  ALWAYSINLINE bool valid(){
    if(broken)
      return false;

    // The first tick at the start of the track should always be valid
    //float delay = math::time_to_ticks(global->cl_interp->value.flt);
    {
      // TODO: HANDLE ME!!!
      //if(index == 0)
        //return true;
    }

    return time_delta() < (0.200f - math::ticks_to_time(1));
  }
};

struct s_lc_data{
  bool        filled;
  s_lc_record records[LC_MAX_RECORDS];
  i32         max_records; // records to parse
  i32         latest_record_index;

  ALWAYSINLINE void reset(){
    memset(this, 0, sizeof(*this));
  }

  ALWAYSINLINE bool valid(){
    return max_records > 0;
  }

  ALWAYSINLINE void think(){
    max_records = math::smallest(math::time_to_ticks(1.f), LC_MAX_RECORDS);
  }

  // fetches a record from the current position downwards
  // so for example get_record(1) = records[last_record_index-1]
  // because records are created forwards and history is backwards
  ALWAYSINLINE s_lc_record* get_record(i32 index){
    if(!filled || max_records == 0)
      return nullptr;

    index = latest_record_index - index;

    while(index < 0)
      index = max_records - math::abs(index);

    if(index > 0)
      index = math::clamp(index % max_records, 0, max_records - 1);
    else
      index = math::biggest(index, 0);

    s_lc_record* record = &records[index];

    if(!record->filled)
      return nullptr;

    return record;
  }

  ALWAYSINLINE bool is_record_broken(i32 index){
    s_lc_record* record1 = get_record(index);
    s_lc_record* record2 = get_record(index + 1);

    if(record1 == nullptr || record2 == nullptr)
      return false;

    return (record1->origin - record2->origin).length_sqr_2d() > 4096;
  }

  ALWAYSINLINE void add_record(s_lc_record record){
    latest_record_index = (latest_record_index + 1) % max_records;

    record.filled       = true;
    record.broken       = false;
    memcpy(&records[latest_record_index], &record, sizeof(s_lc_record));

    filled = true;

    // Everytime we add a record, we need to let all our records know what position they are
    for(u32 i = 0; i < max_records; i++){
      s_lc_record* record = get_record(i);

      if(record == nullptr)
        break;

      record->index = i;
    }
  }

  // Checks if the players simulation time has increased
  ALWAYSINLINE bool needs_update(c_base_player* player){
    if(player == nullptr)
      return false;

    s_lc_record* latest_record = get_record(0);

    if(latest_record == nullptr)
      return true;

    return player->simulation_time > latest_record->simulation_time;
  }
};

class c_lag_compensation{
public:
  s_lc_data* lc_table;
  bool init  = false;
  void setup(){
    if(lc_table == nullptr)
      lc_table = malloc(XOR32(sizeof(s_lc_data) * LC_MAX_ENTRIES));

    assert(lc_table != nullptr);
    init = true;
  }

  void unload(){
    if(lc_table != nullptr){
      free(lc_table);
      lc_table = nullptr;
    }

    init = false;
  }

  s_lc_data* get_data(c_base_player* player){
    if(player == nullptr)
      return nullptr;

    if(!player->is_valid() || !player->is_alive())
      return nullptr;

    i32 index = player->get_index() - 1;

    if(index < 0 || index >= LC_MAX_ENTRIES)
      return nullptr;

    return &lc_table[index];
  }

  void createmove_run(){
    assert(init == true);
    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr)
      return;

    for(i32 i = 1; i <= globaldata->max_clients; i++){
      c_base_player* player = global->entity_list->get_entity(i);

      if(player == nullptr)
        continue;

      if(!player->is_valid() || !player->is_alive())
        continue;

      s_studio_hdr* hdr = player->get_studio_hdr();
      if(hdr == nullptr)
        continue;

      s_lc_data* lc = get_data(player);

      if(lc == nullptr)
        continue;

      // run think for lag compensation entry
      lc->think();

      if(!lc->needs_update(player))
        continue;

      s_lc_record new_record;
      {
        // bones related
        {
          if(!player->setup_bones((matrix3x4*)&new_record.bone_matrix, true)){
            DBG("[!] lag_compensation: setup bones failed for %i\n", i);
            continue;
          }

          new_record.bone_count = hdr->num_bones;
        }

        new_record.origin           = player->origin();
        new_record.velocity         = player->velocity();
        new_record.obb_center       = player->obb_center();
        new_record.obb_mins         = player->obb_mins();
        new_record.obb_maxs         = player->obb_maxs();
        new_record.viewangles       = player->viewangles();
        new_record.simulation_time  = player->simulation_time;
        new_record.entity_flags     = player->entity_flags;
        new_record.server_tick      = global->client_state->server_tick;

        //call_entity_prediction_required(player);

        //bullet_tracers->add(new_record.origin, new_record.origin + vec3(0, 0, 10), rgb(255,255, 0), 1.f);
      }
      lc->add_record(new_record);

      // If teleport distance check fails on record 0, all records after 0 are going to be broken
      if(lc->is_record_broken(0)){
        for(u32 i = 1; i < lc->max_records; i++){
          s_lc_record* record = lc->get_record(i);

          // end of track
          if(record == nullptr)
            break;

          record->broken = true;
        }
      }
    }
  }

  // return true = dont call original setupbones
  bool base_animating_setupbones_run(c_base_player* player, matrix3x4* matrix, i32 max_bones, i32 bone_mask, float current_time){
    if(player == nullptr || matrix == nullptr)
      return false;

    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr)
      return false;

    if(!localplayer->is_alive())
      return false;

    // Should localplayer be ignored from setup bone optimizations? -Rud
    if(localplayer == player)
      return false;

    s_lc_data* lc = get_data(player);

    if(lc == nullptr)
      return false;

    s_lc_record* record = lc->get_record(0);

    if(record == nullptr)
      return false;

    // Only give them the cached bone set if they ask for the bones the player has
    if(max_bones != record->bone_count)
      return false;

    memcpy(matrix, &record->bone_matrix, sizeof(matrix3x4) * max_bones);

    return true;
  }

  // Handles disable interpolation logic for entities, since we dont want interpolation running at all when lag compensation is in play
  void frame_stage_notify_run(u32 stage){
    if(!global->valid())
      return;

    if(stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END){
      createmove_run();
      return;
    }

    if(stage != FRAME_RENDER_START)
      return;

    i32 local_index = global->engine->get_local_player();
    for(i32 i = 1; i <= globaldata->max_clients; i++){
      // DT: If we start to have missing issues after this change then just enforce disable interp again for local
      //if(i == local_index /*&& !double_tap->should_shift && !double_tap->is_shift_tick()*/)
      //  continue;

      c_base_entity* entity = global->entity_list->get_entity( i );

      if(entity == nullptr)
        continue;

      if(entity->is_dormant())
        continue;

      if(!entity->is_player())
        continue;

      entity->disable_interpolation();

      i32 count = 0;
      for(c_base_entity* attachment = entity->first_move_child(); attachment != nullptr; attachment = attachment->next_move_peer()){
        if(attachment == nullptr || count++ >= 32)
          break;

        if(attachment->should_draw())
          attachment->disable_interpolation();
      }
    }
  }

  void call_entity_prediction_required(c_base_player* player);
};

CLASS_EXTERN(c_lag_compensation, lag_compensation);