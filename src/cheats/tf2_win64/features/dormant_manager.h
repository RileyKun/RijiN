#pragma once

#define DORMANT_NEXT_PARTY_UPDATE 0.25f;
class c_dormant_manager : public c_base_dormant_manager{
public:
  void update(i32 index, vec3 pos = vec3(), bool set_origin = false) override;
  void on_dormant(c_internal_base_entity* entity, bool dormant) override;
  void reset(i32 index) override;

  void read_party_player_data(c_party_chat_packet_entity_update* entity_update);
  void send_party_player_data(u32 stage);
};

CLASS_EXTERN(c_dormant_manager, dormant_manager);