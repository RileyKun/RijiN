#pragma once

class c_party_networking{
public:

  c_party_chat_fragment* read_party_chat_packet(i8* str){
    if(str == nullptr)
      return nullptr;

    u32 len = strlen(str);
    if(len > 0 && (len / 2) < XOR32(sizeof(c_party_chat_fragment)))
      return nullptr;

    c_party_chat_fragment* header = (c_party_chat_fragment*)(utils::hex_to_bytes(str));
    if(header == nullptr)
      return nullptr;

    if(!header->valid()){
      DBG("[-] Received invalid party chat packet!\n");
      free(header);
      return nullptr;
    }

    if(header->overflowed()){
      DBG("[-] Received a valid party chat packet! But the data was overflowed\n");
      free(header);
      return nullptr;
    }

    return header;
  }

  bool should_transmit(){
    if(!config->misc.party_network)
      return false;

    if(global->client_state->signon_state() != SIGNONSTATE_FULL)
      return false;

    ctf_party* party = utils::get_party();
    if(party == nullptr)
      return false;

    // Not enough members in this party.
    if(party->member_count < 2)
      return false;

    return true;
  }

  bool on_party_chat_message(i8* text, i32 type){
    if(text == nullptr)
      return false;

    // We're getting send party chat failure.
    if(type == XOR32(1002))
      global->party_chat_rate_limited = math::time() + 1.f;

    c_party_chat_fragment* header = read_party_chat_packet(text);
    if(header == nullptr)
      return false;

    if(!header->is_in_same_server() || header->sender == global->localplayer_index){
      free(header);
      return true;
    }

    if(header->sender > globaldata->max_clients){
      free(header);
      return true;
    }

    if(!config->misc.party_network){
      free(header);
      return true;
    }

    switch(header->type){
      default: break;
      case PARTY_CHAT_PACKET_TYPE_ENTITY_UPDATE:
      {
        if(header->len == XOR32(sizeof(c_party_chat_packet_entity_update)))
          dormant_manager->read_party_player_data((c_party_chat_packet_entity_update*)header);

        break;
      }
      case PARTY_CHAT_PACKET_TYPE_RESOLVER:
      {
        if(header->len == XOR32(sizeof(c_party_chat_packet_resolver)))
          resolver->read_resolver_data((c_party_chat_packet_resolver*)header);

        break;
      }
      case PARTY_CHAT_PACKET_TYPE_WAYPOINT_MARKERS:
      {
        if(header->len == XOR32(sizeof(c_party_chat_packet_waypoint_markers)))
          read_marker_packet((c_party_chat_packet_waypoint_markers*)header);

        break;
      }
    }

    free(header);
    return true;
  }

  void create_marker_packet(){
    if(!should_transmit())
      return;

    if(!config->misc.party_network_waypoint_marker)
      return;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return;

    if(!localplayer->is_valid())
      return;

    static bool key_held = false;
    if(!input_system->held(config->misc.place_waypoint_key)){
      key_held = false;
      return;
    }

    if(key_held)
      return;

    key_held = true;

    c_party_chat_packet_waypoint_markers waypoint_marker;

    vec3 angles = global->engine->get_view_angles();
    if(!freecam->get_shoot_pos(waypoint_marker.pos))
      return;

    vec3 fwd;
    math::angle_2_vector(angles, &fwd, nullptr, nullptr);

    s_trace tr = global->trace->ray(waypoint_marker.pos, waypoint_marker.pos + (fwd * 8912.f), mask_bullet);
    waypoint_marker.pos = tr.end;

    if(waypoint_marker.transmit()){
      utils::create_tf2_marker(global->engine->get_local_player(), waypoint_marker.pos, localplayer->info().name, config->misc.waypoint_timeout);
      DBG("[!] Sent marker\n");
    }
  }

  void read_marker_packet(c_party_chat_packet_waypoint_markers* marker){
    if(marker == nullptr)
      return;

    if(!config->misc.party_network_waypoint_marker)
      return;

    s_player_info info;
    if(global->engine->get_player_info(marker->sender, &info))
      utils::create_tf2_marker(marker->sender, marker->pos, info.name, config->misc.waypoint_timeout);
  }

};

CLASS_EXTERN(c_party_networking, party_network);