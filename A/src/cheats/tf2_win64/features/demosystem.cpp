#include "../link.h"

void c_demosystem::createmove(){
  if(!config->misc.auto_record_hvh_matches || global->engine->is_playing_demo())
    return;

  if(global->engine->is_recording_demo() && !global->cheat_recorded){
    DBG("[!] Not recording with the cheat's demo system!\n");
    apply_settings();
    global->engine->client_cmd(XOR("ds_stop; stop"));
  }
}

void c_demosystem::begin(i8* name){
  if(!config->misc.auto_record_hvh_matches)
    return;

  if(global->engine->is_recording_demo())
    return;

  if(name != nullptr){
     global->ds_prefix->set_value(name);
  }
  else
    global->ds_prefix->set_value(XOR("rijin"));

  apply_settings();
  global->engine->client_cmd(XOR("ds_record"));
  DBG("[!] Recording demo..\n");
  global->cheat_recorded = true;
}

void c_demosystem::end(){
  if(!config->misc.auto_record_hvh_matches)
    return;

  if(global->engine->is_playing_demo() || !global->engine->is_recording_demo())
    return;

  apply_settings();
  global->engine->client_cmd(XOR("ds_stop; stop"));
  DBG("[!] Stopped recording demo!\n");
  global->cheat_recorded = false;
}

void c_demosystem::apply_settings(){
  if(!config->misc.auto_record_hvh_matches)
    return;

  assert(global->ds_enable     != nullptr);
  assert(global->ds_notify     != nullptr);
  assert(global->ds_log        != nullptr);
  assert(global->ds_sound      != nullptr);
  assert(global->ds_autodelete != nullptr);
  assert(global->ds_dir        != nullptr);
  assert(global->ds_prefix     != nullptr);
  assert(global->ds_min_streak != nullptr);


  global->ds_enable->set_value(0);
  global->ds_notify->set_value(0);
  global->ds_log->set_value(0);
  global->ds_sound->set_value(0);
  global->ds_autodelete->set_value(0);
  global->ds_min_streak->set_value(100);

  std::string match_name = get_match_name();

  format_dir(true);
}

void c_demosystem::format_dir(bool bypass_check){
  if(!config->misc.demosystem_format_ds_dir && !bypass_check)
    return;

  i8 buf[IDEAL_MIN_BUF_SIZE];
  formatA(buf, XOR("demos/%s"), get_match_name().c_str());
  global->ds_dir->set_value(buf);
  DBG("[!] Setting ds_dir to %s\n", buf);
}

void c_demosystem::record_player_event(c_base_player* player, i8* info){
  if(!config->misc.auto_record_hvh_matches)
    return;

  if(global->engine->is_playing_demo() || !global->engine->is_recording_demo())
    return;

  if(player == nullptr)
    return;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return;

  if(player->is_steam_friend())
    return;

  c_player_data* data = player->data();
  if(data == nullptr)
    return;

  if(!cheat_detection->is_cheating(player->get_index()))
    return;

  i8 buf[IDEAL_MIN_BUF_SIZE];
  formatA(buf, XOR("ds_mark \"[%s] %s\""), player->info().name, info);
  global->engine->client_cmd(buf);
}

std::string c_demosystem::get_match_name(){
  if(utils::is_community_server())
    return XOR("community");
  else if(utils::is_match_making_server())
    return XOR("matchmaking");
  else if(utils::is_listen_server())
    return XOR("listenserver");

  return XOR("unknown");
}

CLASS_ALLOC(c_demosystem, demosystem);