#pragma once

#define LC_MAX_RECORDS 66
#define LC_MAX_ENTRIES 128

class s_lc_record{
public:
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
  matrix3x4 bone_matrix[MAX_BONES];
  i32       server_tick;

  NEVERINLINE bool get_hitbox_pos(c_internal_base_player* player, i32 hitbox, vec3& output_pos){
    s_studio_bbox* bbox = player->get_studio_bbox(hitbox);

    if(bbox == nullptr)
      return false;

    output_pos = math::bbox_pos(bbox, bone_matrix, player->model_scale());
    return true;
  }

  NEVERINLINE bool get_hitgroup_pos(c_internal_base_player* player, i32 hitgroup, vec3& output_pos){
    s_studio_bbox* bbox = player->get_studio_bbox_from_hitgroup(hitgroup);
    if(bbox == nullptr)
      return false;

    output_pos = math::bbox_pos(bbox, bone_matrix, player->model_scale());
    return true;
  }

  NEVERINLINE bool get_hitbox_points(c_internal_base_player* player, i32 hitbox, float scale, vec3* points){
    s_studio_bbox* bbox = player->get_studio_bbox(hitbox);
    if(bbox == nullptr)
      return false;

    return math::get_box_points(bbox->mins * player->model_scale(), bbox->maxs * player->model_scale(), bone_matrix[bbox->bone], scale, points);
  }

  // Returns how far the record is behind
  float time_delta();

  NEVERINLINE bool valid(){
    if(broken)
      return false;

    return time_delta() < (0.200f - math::ticks_to_time(1));
  }
};

struct s_lc_data{
  bool        filled;
  s_lc_record records[LC_MAX_RECORDS];
  i32         max_records; // records to parse
  i32         latest_record_index;

  ALWAYSINLINE void reset(){
    memset(this, 0, sizeof(s_lc_data));
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
  NEVERINLINE s_lc_record* get_record(i32 index){
    if(!filled || max_records == 0)
      return nullptr;

    index = latest_record_index - index;

    while(index < 0)
      index = max_records - math::abs(index);

    if(index > 0)
      index = math::clamp(index % max_records, 0, max_records - 1);
    else
      index = math::biggest(index, 0);

    assert(index >= 0);
    assert(index < LC_MAX_RECORDS);
    s_lc_record* record = &records[index];

    if(!record->filled)
      return nullptr;

    return record;
  }

  NEVERINLINE bool is_record_broken(i32 index){
    s_lc_record* record1 = get_record(index);
    s_lc_record* record2 = get_record(index + 1);

    if(record1 == nullptr || record2 == nullptr)
      return false;

    return (record1->origin - record2->origin).length_sqr_2d() > 4096;
  }

  NEVERINLINE void add_record(s_lc_record record){
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

  NEVERINLINE bool needs_update(c_internal_base_player* player){
    if(player == nullptr)
      return false;

    s_lc_record* latest_record = get_record(0);

    if(latest_record == nullptr)
      return true;

    return math::time_to_ticks(player->simulation_time()) > math::time_to_ticks(latest_record->simulation_time);
  }
};

class c_base_lag_compensation{
public:

  virtual void setup(){
    if(lc_table != nullptr)
      return;

    lc_table = (s_lc_data*)malloc(sizeof(s_lc_data) * LC_MAX_ENTRIES);
    assert(lc_table != nullptr);
  }

  virtual void unload(){
    if(lc_table == nullptr)
      return;

    free(lc_table);
    lc_table = nullptr;
  }

  virtual void frame_stage_notify(u32 stage);

  virtual void call_entity_prediction_required(c_internal_base_player* player){
    // do nothing on base.
  }

  NEVERINLINE s_lc_data* get_data(c_internal_base_player* player){
    if(player == nullptr)
      return nullptr;

    if(!player->is_base_alive() || player->is_dormant())
      return nullptr;

    i32 index = player->get_index() - 1;

    if(index < 0 || index >= LC_MAX_ENTRIES)
      return nullptr;

    return &lc_table[index];
  }

  s_lc_data* lc_table;
  bool       is_setting_up_bones;
};