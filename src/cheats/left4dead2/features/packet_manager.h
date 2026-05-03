#pragma once

class c_packet_manager : public c_base_packet_manager{
public:

  i32 get_max_choke_ticks() override;
  bool allow_choking(bool predicted = false) override;
};

CLASS_EXTERN(c_packet_manager, packet_manager);