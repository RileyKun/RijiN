#include "../link.h"

void c_game_event_listener::fire_game_event(c_game_event* event){
  if(event == nullptr)
    return;

  DBG("[!] fire_game_event: %p - if you're seeing multiple of these then restart your game!!!\n", global->listeners);
  utils::process_game_event(event);
}

bool c_interface_game_event_manager::add_listener(std::string name){
  if(find_listener(name))
    return true;

  return utils::virtual_call64<3, bool, void*, i8*, bool>(this, &global->listeners, name.c_str(), false);
}


bool c_interface_game_event_manager::find_listener(std::string name){
  return utils::virtual_call64<4, bool, void*, i8*>(this, &global->listeners, name.c_str());
}

void c_interface_game_event_manager::remove_listener(){
  utils::virtual_call64<5, void, void*>(this, &global->listeners);
}
