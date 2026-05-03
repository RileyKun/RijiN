#pragma once

class c_esp {
public:
  float _calc1;
  float _calc3;
  
  float _fov;
  
  ow_local* _local;
  ow_local _local_data;
  ow_object _objects[MAX_OBJECTS];
  int _object_count;
  
  void run();
  void run_glow(CONTEXT* ctx);
  bool w2s(Vector world, vec3i& screen);
  bool calc_bounding_box(Vector pos, vec3 min, vec3 max, vec3i& size_out, vec3i& pos_out);
  float calc_nameplate_correction(i32 hero_id, float nameplate_y);
};

extern c_esp* g_esp;