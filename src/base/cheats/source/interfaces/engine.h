#pragma once

class c_achievement_mgr;
class c_interface_engine{
public:
  ALWAYSINLINE vec3i get_screen_size(){
    i32 w = 0, h = 0;
    utils::internal_virtual_call<void, i32*, i32*>(base_interface->get_screen_size_index, this, &w, &h);

    static i32 s_w = 0;
    static i32 s_h = 0;

    if(w >= 640)
      s_w = w;

    if(h >= 480)
      s_h = h;

    return vec3i(s_w, s_h);
  }

  ALWAYSINLINE void server_cmd(i8* cmd, bool reliable){
    assert(cmd != nullptr);
    utils::internal_virtual_call<void, i8*, bool>(base_interface->server_cmd_index, this, cmd, reliable);
  }

  NEVERINLINE bool get_player_info(i32 entity_id, s_player_info* info){
    if(entity_id < 0){
      DBG("[!] get_player_info trying to access info from %i\n", entity_id);
      return false;
    }

    assert(info != nullptr);

    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<bool, i32, s_player_info*>(base_interface->get_player_info_index, this, entity_id, info);
    STACK_CHECK_END;
    return r;
  }

  ALWAYSINLINE i32 get_player_for_user_id(i32 entity_id){
    assert(entity_id >= 0);

    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<i32, i32>(base_interface->get_player_for_user_id_index, this, entity_id);
    STACK_CHECK_END;
    return r;
  }

  ALWAYSINLINE i32 get_local_player(){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<i32>(base_interface->get_local_player_index, this);
    STACK_CHECK_END;
    return r;
  }

  ALWAYSINLINE vec3 get_view_angles(){
    vec3 ang;
    STACK_CHECK_START;
    utils::internal_virtual_call<void, vec3*>(base_interface->get_view_angles_index, this, &ang);
    STACK_CHECK_END;
    return ang;
  }

  ALWAYSINLINE void set_view_angles(vec3 ang){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, vec3*>(base_interface->set_view_angles_index, this, &ang);
    STACK_CHECK_END;
  }

  ALWAYSINLINE bool is_taking_screenshot(){
    return utils::internal_virtual_call<bool>(base_interface->is_taking_screenshot_index, this);
  }

  ALWAYSINLINE bool is_in_game(){
    return utils::virtual_call<26, bool>(this);
  }

  // How to find:
  // Search for "Client TempEnts" in client.dll
  // And the function below it is the offset from engine
  // (*(void (**)(void))(*(_DWORD *)dword_10C46968 + 224))();
  ALWAYSINLINE void fire_events(){
    STACK_CHECK_START;
    utils::internal_virtual_call<void>(base_interface->engine_fire_events_index, this);
    STACK_CHECK_END;
  }

  ALWAYSINLINE void client_cmd(i8* cmd){
    assert(cmd != nullptr);

    STACK_CHECK_START;
    utils::internal_virtual_call<void, i8*>(base_interface->client_cmd_index, this, cmd);
    STACK_CHECK_END;
  }

  ALWAYSINLINE void send_keyvalue(void* kv){
    assert(kv != nullptr);

    STACK_CHECK_START;
    utils::internal_virtual_call<void, void*>(base_interface->send_keyvalue_index, this, kv);
    STACK_CHECK_END;
  }

  ALWAYSINLINE c_net_channel* get_net_channel(){
    return utils::internal_virtual_call<c_net_channel*>(base_interface->get_net_channel_index, this);
  }

  ALWAYSINLINE bool is_playing_demo(){
    return utils::internal_virtual_call<bool>(base_interface->is_playing_demo_index, this);
  }

  ALWAYSINLINE bool is_recording_demo(){
    return utils::internal_virtual_call<bool>(base_interface->is_recording_demo_index, this);
  }

  ALWAYSINLINE i8* get_level_name(){
    return utils::internal_virtual_call<i8*>(base_interface->get_level_name_index, this);
  }

  ALWAYSINLINE float get_latency(bool outgoing, bool old){
    DBG("[-] WARNING: get_latency(bool, bool) is outdated! If you want to correct for correct_for_fake_latency you need to pass the get_raw_target_latency through the second parameter.\n");
    return get_latency(outgoing, 0.f);
  }

  ALWAYSINLINE float get_latency(bool outgoing, float correct_for_fake_latency = 0.f){
    c_net_channel* net = get_net_channel();
  
    if(net == nullptr)
      return 0.f;
  
    float time = utils::internal_virtual_call<float, i32>(base_interface->get_latency_index, net, outgoing ? 0 : 1);
  
    if(correct_for_fake_latency > 0.0f && !outgoing) // Make sure you use 'get_raw_target_latency'
      time = math::abs(correct_for_fake_latency - time);
  
    return time;
  }

  ALWAYSINLINE float get_avg_latency(bool outgoing){
    c_net_channel* net = get_net_channel();

    if(net == nullptr)
      return 0.f;

    return utils::internal_virtual_call<float, i32>(base_interface->get_avg_latency_index, net, outgoing ? 0 : 1);
  }

  ALWAYSINLINE float get_avg_packets(bool outgoing){
    c_net_channel* net = get_net_channel();
    if(net == nullptr)
      return 0.f;

    return utils::internal_virtual_call<float, i32>(base_interface->get_avg_packets_index, net, outgoing ? 0 : 1);
  }

  ALWAYSINLINE i8* get_ip(){
    c_net_channel* net = get_net_channel();
    if(net == nullptr)
      return nullptr;

    return utils::internal_virtual_call<i8*>(base_interface->get_ip_index, net);
  }

  ALWAYSINLINE i32 send_datagram(s_bf_write* p){
    c_net_channel* net = get_net_channel();

    if(net == nullptr)
      return 0;

    return utils::internal_virtual_call<i32, s_bf_write*>(base_interface->send_datagram_index, net, p);
  }

  ALWAYSINLINE c_achievement_mgr* get_achievement_mgr(){
    return utils::internal_virtual_call<c_achievement_mgr*>(base_interface->get_achievement_mgr_index, this);
  }
};