#pragma once

class c_target_list : public c_base_target_list{
public:
  bool is_valid_target_entity(i32 index) override;
  void compute_active_spectators() override;
  bool on_collection_iteration(i32 index) override;
  bool is_local_ready() override;
  i32 get_wanted_list_type() override;
  bool is_weapon_supported() override;
  i32 get_loop_end_index() override;
};

CLASS_EXTERN(c_target_list, target_list);