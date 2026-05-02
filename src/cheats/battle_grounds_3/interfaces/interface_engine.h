#pragma once

class c_interface_engine{
public:
  ALWAYSINLINE vec3i get_screen_size(){
    i32 w = 0, h = 0;
    utils::virtual_call<5, void, i32*, i32*>(this, &w, &h);

    return vec3i(w, h);
  }

  ALWAYSINLINE void server_cmd(i8* cmd, bool reliable){
    if(cmd == nullptr)
      return;

    utils::virtual_call<6, void, i8*, bool>(this, cmd, reliable);
  }

  ALWAYSINLINE bool get_player_info(i32 entity_id, s_player_info* info){
    if(info == nullptr)
      return false;

    return utils::virtual_call<8, bool, i32, s_player_info*>(this, entity_id, info);
  }

  ALWAYSINLINE i32 get_player_for_user_id(i32 entity_id){
    return utils::virtual_call<9, i32, i32>(this, entity_id);
  }

  ALWAYSINLINE i32 get_local_player(){
    return utils::virtual_call<12, i32>(this);
  }

  ALWAYSINLINE vec3 get_view_angles(){
    vec3 ang;
    utils::virtual_call<19, void, vec3*>(this, &ang);

    return ang;
  }

  ALWAYSINLINE void set_view_angles(vec3 ang){
    utils::virtual_call<20, void, vec3*>(this, &ang);
  }

  // search for "VGui_PreRender" in client.dll look at the function in the sourcesdk for ref.
  ALWAYSINLINE bool is_taking_screenshot(){
    return utils::virtual_call<85, bool>(this);
  }

  // DEPRECIATED: USE CLIENTSTATE->SIGNONSTATE INSTEAD AND CHECK AGAINST SIGNONSTATE_FULL
  // search for "fov %f\n" in client.dll
  ALWAYSINLINE bool is_in_game(){
    return utils::virtual_call<26, bool>(this);
  }

  // How to find:
  // Search for "Client TempEnts" in client.dll
  // And the function below it is the offset from engine
  // (*(void (**)(void))(*(_DWORD *)dword_10C46968 + 224))();
  ALWAYSINLINE void fire_events(){
    utils::virtual_call<56, void>(this);
  }

  ALWAYSINLINE void client_cmd(i8* cmd){
    utils::virtual_call<106, void, i8*>(this, cmd);
  }

  ALWAYSINLINE void send_keyvalue(void* kv){
    DBG("Sending keyvalue: 0x%p\n", kv);
    utils::virtual_call<127, void, void*>(this, kv);
  }

  ALWAYSINLINE s_net_channel* get_net_channel(){
    return utils::virtual_call<72, s_net_channel*>(this);
  }

  ALWAYSINLINE bool is_playing_demo(){
    return utils::virtual_call<76, bool>(this);
  }

  ALWAYSINLINE i8* get_level_name(){
    return utils::virtual_call<51, i8*>(this);
  }

  ALWAYSINLINE float get_latency(bool outgoing, bool correct_for_fake_latency = false){
    s_net_channel* net = get_net_channel();

    if(net == nullptr)
      return 0.f;

    return utils::virtual_call<9, float, i32>(net, outgoing ? 0 : 1);
  }

  ALWAYSINLINE float get_avg_latency(bool outgoing){
    s_net_channel* net = get_net_channel();

    if(net == nullptr)
      return 0.f;

    return utils::virtual_call<10, float, i32>(net, outgoing ? 0 : 1);
  }

  ALWAYSINLINE float get_avg_packets(bool outgoing){
    s_net_channel* net = get_net_channel();
    if(net == nullptr)
      return 0.f;

    return utils::virtual_call<14, float, i32>(net, outgoing ? 0 : 1);
  }

  ALWAYSINLINE i8* get_ip(){
    s_net_channel* net = get_net_channel();
    if(net == nullptr)
      return nullptr;

    return utils::virtual_call<1, i8*>(net);
  }

  //ALWAYSINLINE c_achievement_mgr* get_achievement_mgr(){
  //  return utils::virtual_call<114, c_achievement_mgr*>(this);
  //}
};